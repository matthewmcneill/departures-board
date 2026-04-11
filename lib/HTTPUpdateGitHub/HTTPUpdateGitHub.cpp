/*
 * HTTPUpdateGitHub Library (c) 2025-2026 Gadec Software
 *  - based on orignal HTTPUpdate for ESP32 by Markus Sattler
 *  - Added ability to use GitHub token for updating from private repositories
 *  - Added correct handling of redirects
 *  - Added correct checking of MD5 hash from GitHub server
 *  - Removed redundant code
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 * Module: lib/hTTPUpdateGitHub/hTTPUpdateGitHub.cpp
 * Description: Implementation of the GitHub OTA Update handler.
 *
 * Exported Functions/Classes:
 * - HTTPUpdate: [Class implementation]
 *   - handleUpdate: Redirect-aware GitHub binary fetcher.
 *   - runUpdate: ESP32-native flash writing orchestrator.
 *   - getLastErrorString: Diagnostic error string formatting.
 */

#include <hTTPUpdateGitHub.hpp>
#include <Update.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <logger.hpp>
#include <StreamString.h>

#include <esp_partition.h>
#include <esp_ota_ops.h>                // get running partition
#include <md5Utils.hpp>

// Include mbedtls and public key
#include <mbedtls/md.h>
#include <mbedtls/pk.h>
#include <publicKey.hpp>

class MbedTLSOtaVerifier {
private:
    mbedtls_pk_context pk;
    mbedtls_md_context_t md_ctx;
    bool initialized = false;
public:
    MbedTLSOtaVerifier() {
        mbedtls_pk_init(&pk);
        mbedtls_md_init(&md_ctx);
    }
    ~MbedTLSOtaVerifier() {
        mbedtls_pk_free(&pk);
        mbedtls_md_free(&md_ctx);
    }
    bool begin() {
        int ret = mbedtls_pk_parse_public_key(&pk, (const unsigned char*)OtaPublicKey, strlen(OtaPublicKey) + 1);
        if (ret != 0) {
            LOG_ERROR("OTA", "mbedtls_pk_parse_public_key failed");
            return false;
        }
        
        const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
        if (md_info == NULL) return false;
        
        if (mbedtls_md_setup(&md_ctx, md_info, 0) != 0) return false;
        if (mbedtls_md_starts(&md_ctx) != 0) return false;
        
        initialized = true;
        return true;
    }
    void update(const uint8_t* buf, size_t len) {
        if (initialized) mbedtls_md_update(&md_ctx, buf, len);
    }
    bool verify(const uint8_t* sig_buf, size_t sig_len) {
        if (!initialized) return false;
        unsigned char hash[32];
        if (mbedtls_md_finish(&md_ctx, hash) != 0) return false;
        
        int ret = mbedtls_pk_verify(&pk, MBEDTLS_MD_SHA256, hash, sizeof(hash), sig_buf, sig_len);
        if (ret != 0) {
             LOG_ERROR("OTA", "mbedtls_pk_verify failed");
             return false;
        }
        return true;
    }
};

HTTPUpdate::HTTPUpdate(void)
        : _httpClientTimeout(8000)
{
}

HTTPUpdate::HTTPUpdate(int httpClientTimeout)
        : _httpClientTimeout(httpClientTimeout)
{
}

HTTPUpdate::~HTTPUpdate(void)
{
}

int HTTPUpdate::getLastError(void)
{
    return _lastError;
}

String HTTPUpdate::getLastErrorString(void)
{

    if(_lastError == 0) {
        return String(); // no error
    }

    if(_lastError > 0) {
        StreamString error;
        Update.printError(error);
        error.trim();
        return String("Update error: ") + error;
    }

    if(_lastError > -100) {
        return String("HTTP error: ") + HTTPClient::errorToString(_lastError);
    }

    switch(_lastError) {
    case HTTP_UE_TOO_LESS_SPACE: return "Not Enough space";
    case HTTP_UE_SERVER_NOT_REPORT_SIZE: return "Server Did Not Report Size";
    case HTTP_UE_SERVER_FILE_NOT_FOUND: return "File Not Found (404)";
    case HTTP_UE_SERVER_FORBIDDEN: return "Forbidden (403)";
    case HTTP_UE_SERVER_WRONG_HTTP_CODE: return "Wrong HTTP Code";
    case HTTP_UE_SERVER_FAULTY_MD5: return "Wrong MD5";
    case HTTP_UE_BIN_VERIFY_HEADER_FAILED: return "Verify Bin Header Failed";
    case HTTP_UE_BIN_FOR_WRONG_FLASH: return "New Binary Does Not Fit Flash Size";
    case HTTP_UE_NO_PARTITION: return "Partition Could Not be Found";
    }
    return String();
}

bool HTTPUpdate::fetchSignature(WiFiClient& client, const String& uri, const String& token, uint8_t* sigBuf, size_t expectedLen) {
     HTTPClient http;
     int redirectCount = 0;
     const int maxRedirects = 5;
     String url = uri;

     while (redirectCount < maxRedirects) {
        if(!http.begin(client, url)) return false;

        http.useHTTP10(true);
        http.setTimeout(_httpClientTimeout);
        http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);
        http.setUserAgent("ESP32-http-Update");
        http.addHeader("Cache-Control", "no-cache");
        http.addHeader("Accept", "application/octet-stream");
        if (token.length()) http.addHeader("Authorization", "Bearer " + token);
        http.addHeader("X-GitHub-Api-Version", "2022-11-28");

        int code = http.GET();
        int len = http.getSize();

        if (code == HTTP_CODE_MOVED_PERMANENTLY || code == HTTP_CODE_FOUND ||
            code == HTTP_CODE_TEMPORARY_REDIRECT || code == HTTP_CODE_PERMANENT_REDIRECT) {
            String newUrl = http.getLocation();
            http.end();
            if (newUrl.length() == 0) return false;
            url = newUrl;
            redirectCount++;
        } else {
            if (code == HTTP_CODE_OK && len > 0 && len <= expectedLen) {
                WiFiClient * tcp = http.getStreamPtr();
                size_t written = 0;
                unsigned long timeout = millis() + _httpClientTimeout;
                while (written < len && millis() < timeout) {
                    if (tcp->available()) {
                        int r = tcp->readBytes(sigBuf + written, len - written);
                        written += r;
                        timeout = millis() + _httpClientTimeout;
                    } else {
                        delay(1);
                    }
                }
                http.end();
                return written == len;
            }
            http.end();
            return false;
        }
    }
    return false;
}


HTTPUpdateResult HTTPUpdate::handleUpdate(WiFiClient& client, const String& uri, const String& sigUri, const String& token) {
     HTTPUpdateResult ret = HTTP_UPDATE_FAILED;

     uint8_t sigBuf[256];
     memset(sigBuf, 0, sizeof(sigBuf));
     if (!fetchSignature(client, sigUri, token, sigBuf, sizeof(sigBuf))) {
         LOG_ERROR("OTA", "Failed to fetch signature! Aborting bin download.");
         return HTTP_UPDATE_FAILED;
     }
     LOG_DEBUG("OTA", "Signature successfully fetched into RAM.");

     HTTPClient http;
     int redirectCount = 0;
     const int maxRedirects = 5;
     String url = uri;

     while (redirectCount < maxRedirects) {
        if(!http.begin(client, url)) return HTTP_UPDATE_FAILED;

        http.useHTTP10(true);
        http.setTimeout(_httpClientTimeout);
        http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS); 
        http.setUserAgent("ESP32-http-Update");
        http.addHeader("Cache-Control", "no-cache");
        http.addHeader("Accept", "application/octet-stream");
        if (token.length()) http.addHeader("Authorization", "Bearer " + token);
        http.addHeader("X-GitHub-Api-Version", "2022-11-28");

        int code = http.GET();
        int len = http.getSize();

        if (code == HTTP_CODE_MOVED_PERMANENTLY || code == HTTP_CODE_FOUND ||
            code == HTTP_CODE_TEMPORARY_REDIRECT || code == HTTP_CODE_PERMANENT_REDIRECT) {
            String newUrl = http.getLocation();
            http.end();
            if (newUrl.length() == 0) return HTTP_UPDATE_FAILED;
            url = newUrl;
            redirectCount++;
        } else {
            if(code <= 0) {
                _lastError = code;
                http.end();
                return HTTP_UPDATE_FAILED;
            }

            if(code == HTTP_CODE_OK) {
                if(len > 0) {
                    int sketchFreeSpace = ESP.getFreeSketchSpace();
                    if(!sketchFreeSpace){
                        _lastError = HTTP_UE_NO_PARTITION;
                        return HTTP_UPDATE_FAILED;
                    }

                    if(len > sketchFreeSpace) {
                        _lastError = HTTP_UE_TOO_LESS_SPACE;
                        return HTTP_UPDATE_FAILED;
                    }

                    if (_cbStart) _cbStart();

                    WiFiClient * tcp = http.getStreamPtr();
                    delay(200);

                    if(tcp->peek() != 0xE9) {
                        _lastError = HTTP_UE_BIN_VERIFY_HEADER_FAILED;
                        http.end();
                        return HTTP_UPDATE_FAILED;
                    }

                    if(runUpdate(*tcp, len, "", U_FLASH, sigBuf, sizeof(sigBuf))) {
                        ret = HTTP_UPDATE_OK;
                        http.end();
                        if (_cbEnd) _cbEnd();
                        if(_rebootOnUpdate) ESP.restart();
                    } else {
                        ret = HTTP_UPDATE_FAILED;
                    }
                } else {
                    _lastError = HTTP_UE_SERVER_NOT_REPORT_SIZE;
                    ret = HTTP_UPDATE_FAILED;
                }
            } else if (code == HTTP_CODE_NOT_MODIFIED) {
                ret = HTTP_UPDATE_NO_UPDATES;
            } else if (code == HTTP_CODE_NOT_FOUND) {
                _lastError = HTTP_UE_SERVER_FILE_NOT_FOUND;
                ret = HTTP_UPDATE_FAILED;
            } else if (code == HTTP_CODE_FORBIDDEN) {
                _lastError = HTTP_UE_SERVER_FORBIDDEN;
                ret = HTTP_UPDATE_FAILED;
            } else {
                _lastError = HTTP_UE_SERVER_WRONG_HTTP_CODE;
                ret = HTTP_UPDATE_FAILED;
            }

            http.end();
            return ret;
        }
    }
    return HTTP_UPDATE_FAILED;
}

bool HTTPUpdate::runUpdate(Stream& in, uint32_t size, String md5, int command, uint8_t* sigBuf, size_t sigLen)
{
     StreamString error;

     if(!Update.begin(size, command)) {
         _lastError = Update.getError();
         Update.printError(error);
         error.trim();
         LOG_ERROR("OTA", "Update.begin failed! (" + error + ")");
         return false;
     }

     MbedTLSOtaVerifier verifier;
     if (!verifier.begin()) {
         LOG_ERROR("OTA", "Cryptographic Verifier initialization failed!");
         Update.abort();
         return false;
     }

     if (_cbProgress) {
         _cbProgress(0, size);
     }

     size_t written = 0;
     uint8_t buf[1024]; 
     while (written < size) {
         int bytesToRead = (size - written < sizeof(buf)) ? (size - written) : sizeof(buf);
         
         int r = in.readBytes(buf, bytesToRead);
         if (r > 0) {
             verifier.update(buf, r);
             if (Update.write(buf, r) != r) {
                 _lastError = Update.getError();
                 Update.printError(error);
                 error.trim();
                 LOG_ERROR("OTA", "Update.write failed! (" + error + ")");
                 Update.abort();
                 return false;
             }
             written += r;
             if (_cbProgress) _cbProgress(written, size);
         } else {
             LOG_ERROR("OTA", "Stream read timeout or disconnect");
             Update.abort();
             return false;
         }
     }

     if(!verifier.verify(sigBuf, sigLen)) {
         LOG_ERROR("OTA", "Cryptographic signature validation FAILED! Update discarded securely.");
         _lastError = HTTP_UE_SERVER_FAULTY_MD5; // Use generic validation failure state
         Update.abort();
         return false;
     }

     if(!Update.end()) {
         _lastError = Update.getError();
         Update.printError(error);
         error.trim();
         LOG_ERROR("OTA", "Update.end failed! (" + error + ")");
         return false;
     }

     return true;
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_HTTPUPDATE)
HTTPUpdate httpUpdate;
#endif

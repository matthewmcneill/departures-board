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
 * Module: lib/HTTPUpdateGitHub/hTTPUpdateGitHub.cpp
 * Description: Implementation of the GitHub OTA Update handler.
 *
 * Exported Functions/Classes:
 * - class HTTPUpdate: Core class handling the HTTP request, redirects, parsing MD5 headers, and writing to flash.
 *   - HTTPUpdate(): Constructor.
 *   - ~HTTPUpdate(): Destructor.
 *   - rebootOnUpdate(): Configures whether to reboot on update success.
 *   - handleUpdate(): Handles the full OTA update process from a custom URL.
 *   - onStart(), onEnd(), onError(), onProgress(): Registers callback functions for update events.
 *   - getLastError(): Retrieves the last error code encountered.
 *   - getLastErrorString(): Retrieves a human-readable string for the last error.
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

 /**
  * @brief Retrieves the last error code encountered during the update process.
  * @return An integer representing the error state.
  */
 int HTTPUpdate::getLastError(void)
 {
     return _lastError;
 }

 /**
  * @brief Retrieves a human-readable string for the last error encountered.
  * @return A string containing the error description.
  */
 String HTTPUpdate::getLastErrorString(void)
 {

     if(_lastError == 0) {
         return String(); // no error
     }

     // error from Update class
     if(_lastError > 0) {
         StreamString error;
         Update.printError(error);
         error.trim(); // remove line ending
         return String("Update error: ") + error;
     }

     // error from http client
     if(_lastError > -100) {
         return String("HTTP error: ") + HTTPClient::errorToString(_lastError);
     }

     switch(_lastError) {
     case HTTP_UE_TOO_LESS_SPACE:
         return "Not Enough space";
     case HTTP_UE_SERVER_NOT_REPORT_SIZE:
         return "Server Did Not Report Size";
     case HTTP_UE_SERVER_FILE_NOT_FOUND:
         return "File Not Found (404)";
     case HTTP_UE_SERVER_FORBIDDEN:
         return "Forbidden (403)";
     case HTTP_UE_SERVER_WRONG_HTTP_CODE:
         return "Wrong HTTP Code";
     case HTTP_UE_SERVER_FAULTY_MD5:
         return "Wrong MD5";
     case HTTP_UE_BIN_VERIFY_HEADER_FAILED:
         return "Verify Bin Header Failed";
     case HTTP_UE_BIN_FOR_WRONG_FLASH:
         return "New Binary Does Not Fit Flash Size";
     case HTTP_UE_NO_PARTITION:
         return "Partition Could Not be Found";
     }

     return String();
 }

 /**
  * @brief Handles the full OTA update process from a custom URL, such as a GitHub Release asset.
  * @param client The active WiFiClient (or WiFiClientSecure).
  * @param uri The URL path of the binary asset.
  * @param token An optional GitHub Personal Access Token for private repositories.
  * @return The result status of the update process (HTTP_UPDATE_OK, HTTP_UPDATE_FAILED, etc.).
  */
  HTTPUpdateResult HTTPUpdate::handleUpdate(WiFiClient& client, const String& uri = "/", const String& token = "") {

     HTTPUpdateResult ret = HTTP_UPDATE_FAILED;

     HTTPClient http;
     int redirectCount = 0;
     const int maxRedirects = 5;
     String url = uri;
     String md5Hex = "";

     while (redirectCount < maxRedirects) {

        LOG_DEBUG("OTA", "URL: " + url);
        if(!http.begin(client, url))
        {
            return HTTP_UPDATE_FAILED;
        }

        // use HTTP/1.0 for update since the update handler does not support any transfer Encoding
        http.useHTTP10(true);
        http.setTimeout(_httpClientTimeout);
        http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);    // Disable redirects, they don't work anyway
        http.setUserAgent("ESP32-http-Update");
        http.addHeader("Cache-Control", "no-cache");
        // Additional headers for GitHub
        http.addHeader("Accept", "application/octet-stream");
        if (token.length()) http.addHeader("Authorization", "Bearer " + token);
        http.addHeader("X-GitHub-Api-Version", "2022-11-28");

        const char * headerkeys[] = { "x-ms-blob-content-md5" };    // GitHub uses x-ms-blob-content-m5d, not x-md5
        size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);

        // track the MD5 hash
        http.collectHeaders(headerkeys, headerkeyssize);

        int code = http.GET();
        int len = http.getSize();
        LOG_DEBUG("OTA", "HTTP GET result " + String(code));

        if (code == HTTP_CODE_MOVED_PERMANENTLY ||
            code == HTTP_CODE_FOUND ||
            code == HTTP_CODE_TEMPORARY_REDIRECT ||
            code == HTTP_CODE_PERMANENT_REDIRECT) {
            // Handle redirect
            String newUrl = http.getLocation();
            LOG_DEBUG("OTA", "[HTTP] Redirected to: " + newUrl);
            http.end();  // End current request before retrying
            if (newUrl.length() == 0) {
                LOG_ERROR("OTA", "[HTTP] Redirect without Location header!");
                return HTTP_UPDATE_FAILED;
            }
            url = newUrl;
            redirectCount++;
        } else {
            if(code <= 0) {
                _lastError = code;
                LOG_ERROR("OTA", "HTTP error: " + http.errorToString(code));
                http.end();
                return HTTP_UPDATE_FAILED;
            }

            LOG_DEBUG("OTA", "Header read fin.");
            LOG_DEBUG("OTA", "Server header:");
            LOG_DEBUG("OTA", " - code: " + String(code));
            LOG_DEBUG("OTA", " - len: " + String(len));

            if(http.hasHeader("x-ms-blob-content-md5")) {
                LOG_DEBUG("OTA", " - MD5: " + http.header("x-ms-blob-content-md5"));
                // Convert the base64 encoded MD5 back to a hex string
                md5Hex = md5.base64ToHex(String(http.header("x-ms-blob-content-md5")));
                LOG_DEBUG("OTA", " - MD5 Hex: " + md5Hex);
            }

            LOG_DEBUG("OTA", "ESP32 info:");
            LOG_DEBUG("OTA", " - free Space: " + String(ESP.getFreeSketchSpace()));
            LOG_DEBUG("OTA", " - current Sketch Size: " + String(ESP.getSketchSize()));

            switch(code) {
            case HTTP_CODE_OK:  ///< OK (Start Update)
                if(len > 0) {
                    bool startUpdate = true;
                    int sketchFreeSpace = ESP.getFreeSketchSpace();
                    if(!sketchFreeSpace){
                        _lastError = HTTP_UE_NO_PARTITION;
                        return HTTP_UPDATE_FAILED;
                    }

                    if(len > sketchFreeSpace) {
                        LOG_ERROR("OTA", "FreeSketchSpace to low (" + String(sketchFreeSpace) + ") needed: " + String(len));
                        startUpdate = false;
                    }

                    if(!startUpdate) {
                        _lastError = HTTP_UE_TOO_LESS_SPACE;
                        ret = HTTP_UPDATE_FAILED;
                    } else {
                        // Warn main app we're starting up...
                        if (_cbStart) {
                            _cbStart();
                        }

                        WiFiClient * tcp = http.getStreamPtr();
                        delay(200);

                        int command;
                        command = U_FLASH;
                        LOG_DEBUG("OTA", "runUpdate flash...");

                        // check for valid first magic byte
                        if(tcp->peek() != 0xE9) {
                            LOG_ERROR("OTA", "Magic header does not start with 0xE9, starts with " + String(tcp->peek()));
                            _lastError = HTTP_UE_BIN_VERIFY_HEADER_FAILED;
                            http.end();
                            return HTTP_UPDATE_FAILED;
                        }

                        if(runUpdate(*tcp, len, md5Hex, command)) {
                            ret = HTTP_UPDATE_OK;
                            LOG_DEBUG("OTA", "Update ok");
                            http.end();
                            // Warn main app we're all done
                            if (_cbEnd) {
                                _cbEnd();
                            }

                            if(_rebootOnUpdate) {
                                ESP.restart();
                            }

                        } else {
                            ret = HTTP_UPDATE_FAILED;
                            LOG_ERROR("OTA", "Update failed");
                        }
                    }
                } else {
                    _lastError = HTTP_UE_SERVER_NOT_REPORT_SIZE;
                    ret = HTTP_UPDATE_FAILED;
                    LOG_ERROR("OTA", "Content-Length was 0 or wasn't set by Server?!");
                }
                break;
            case HTTP_CODE_NOT_MODIFIED:
                ///< Not Modified (No updates)
                ret = HTTP_UPDATE_NO_UPDATES;
                break;
            case HTTP_CODE_NOT_FOUND:
                _lastError = HTTP_UE_SERVER_FILE_NOT_FOUND;
                ret = HTTP_UPDATE_FAILED;
                break;
            case HTTP_CODE_FORBIDDEN:
                _lastError = HTTP_UE_SERVER_FORBIDDEN;
                ret = HTTP_UPDATE_FAILED;
                break;
            default:
                _lastError = HTTP_UE_SERVER_WRONG_HTTP_CODE;
                ret = HTTP_UPDATE_FAILED;
                LOG_ERROR("OTA", "HTTP Code is (" + String(code) + ")");
                break;
            }

            http.end();
            return ret;
        }
    }
    LOG_ERROR("OTA", "Too many redirects!");
    return HTTP_UPDATE_FAILED;
}

 /**
  * @brief Executes the flash write process using the ESP32 Update framework.
  * @param in The HTTP data stream.
  * @param size The total size of the binary in bytes.
  * @param md5 The expected MD5 hash for the binary.
  * @param command The update command (U_FLASH or U_SPIFFS).
  * @return True if the update flashed successfully.
  */
 bool HTTPUpdate::runUpdate(Stream& in, uint32_t size, String md5, int command)
 {
      StreamString error;

     if (_cbProgress) {
         Update.onProgress(_cbProgress);
     }

     if(!Update.begin(size, command)) {
         _lastError = Update.getError();
         Update.printError(error);
         error.trim(); // remove line ending
         LOG_ERROR("OTA", "Update.begin failed! (" + error + ")");
         return false;
     }

     if (_cbProgress) {
         _cbProgress(0, size);
     }

     if(md5.length()) {
         if(!Update.setMD5(md5.c_str())) {
             _lastError = HTTP_UE_SERVER_FAULTY_MD5;
             LOG_ERROR("OTA", "Update.setMD5 failed! (" + md5 + ")");
             return false;
         }
     }

     if(Update.writeStream(in) != size) {
         _lastError = Update.getError();
         Update.printError(error);
         error.trim(); // remove line ending
         LOG_ERROR("OTA", "Update.writeStream failed! (" + error + ")");
         return false;
     }

     if (_cbProgress) {
         _cbProgress(size, size);
     }

     if(!Update.end()) {
         _lastError = Update.getError();
         Update.printError(error);
         error.trim(); // remove line ending
         LOG_ERROR("OTA", "Update.end failed! (" + error + ")");
         return false;
     }

     return true;
 }

 #if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_HTTPUPDATE)
 HTTPUpdate httpUpdate; // Global OTA update handler instance
 #endif

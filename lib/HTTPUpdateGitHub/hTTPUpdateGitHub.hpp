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
 * Module: lib/hTTPUpdateGitHub/hTTPUpdateGitHub.hpp
 * Description: OTA client for fetching firmware from GitHub releases.
 *
 * Exported Functions/Classes:
 * - HTTPUpdate: [Class] Secure firmware download and flash engine.
 *   - handleUpdate(): High-level entry for redirect-aware binary fetching.
 *   - runUpdate(): Strategic flash writing implementation.
 *   - Notification Callbacks: onStart, onEnd, onError, onProgress.
 */

 #ifndef ___HTTP_UPDATE_H___
 #define ___HTTP_UPDATE_H___

 #include <Arduino.h>
 #include <WiFi.h>
 #include <WiFiClient.h>
 #include <WiFiUdp.h>
 #include <HTTPClient.h>
 #include <Update.h>
 #include <md5Utils.hpp>

 /// note we use HTTP client errors too so we start at 100
 #define HTTP_UE_TOO_LESS_SPACE              (-100)
 #define HTTP_UE_SERVER_NOT_REPORT_SIZE      (-101)
 #define HTTP_UE_SERVER_FILE_NOT_FOUND       (-102)
 #define HTTP_UE_SERVER_FORBIDDEN            (-103)
 #define HTTP_UE_SERVER_WRONG_HTTP_CODE      (-104)
 #define HTTP_UE_SERVER_FAULTY_MD5           (-105)
 #define HTTP_UE_BIN_VERIFY_HEADER_FAILED    (-106)
 #define HTTP_UE_BIN_FOR_WRONG_FLASH         (-107)
 #define HTTP_UE_NO_PARTITION                (-108)

 enum HTTPUpdateResult {
     HTTP_UPDATE_FAILED,
     HTTP_UPDATE_NO_UPDATES,
     HTTP_UPDATE_OK
 };

 typedef HTTPUpdateResult t_httpUpdate_return; // backward compatibility

 using HTTPUpdateStartCB = std::function<void()>;
 using HTTPUpdateRequestCB = std::function<void(HTTPClient*)>;
 using HTTPUpdateEndCB = std::function<void()>;
 using HTTPUpdateErrorCB = std::function<void(int)>;
 using HTTPUpdateProgressCB = std::function<void(int, int)>;

 class HTTPUpdate
 {
 public:
     HTTPUpdate(void);
     HTTPUpdate(int httpClientTimeout);
     ~HTTPUpdate(void);

     void rebootOnUpdate(bool reboot)
     {
         _rebootOnUpdate = reboot;
     }

/**
 * @brief Handles the full OTA update process from a custom URL, such as a GitHub Release asset.
 * @param client The active WiFiClient (or WiFiClientSecure).
 * @param uri The URL path of the binary asset.
 * @param sigUri The URL path of the signature asset.
 * @param token An optional GitHub Personal Access Token for private repositories.
 * @return The result status of the update process (HTTP_UPDATE_OK, HTTP_UPDATE_FAILED, etc.).
 */
     HTTPUpdateResult handleUpdate(WiFiClient& client, const String& uri, const String& sigUri, const String& token);

     // Notification callbacks
     void onStart(HTTPUpdateStartCB cbOnStart)          { _cbStart = cbOnStart; }
     void onEnd(HTTPUpdateEndCB cbOnEnd)                { _cbEnd = cbOnEnd; }
     void onError(HTTPUpdateErrorCB cbOnError)          { _cbError = cbOnError; }
/**
 * @brief Registers a callback function to handle update progress events.
 * @param cbOnProgress The callback function accepting downloaded size and total size.
 */
     void onProgress(HTTPUpdateProgressCB cbOnProgress) { _cbProgress = cbOnProgress; }

/**
 * @brief Retrieves the last error code encountered during the update process.
 * @return An integer representing the error state.
 */
     int getLastError(void);

/**
 * @brief Retrieves a human-readable string for the last error encountered.
 * @return A string containing the error description.
 */
     String getLastErrorString(void);

 protected:

/**
 * @brief Executes the flash write process using the ESP32 Update framework and performs cryptographic verification.
 * @param in The HTTP data stream.
 * @param size The total size of the binary in bytes.
 * @param md5 The expected MD5 hash for the binary.
 * @param command The update command (U_FLASH or U_SPIFFS).
 * @param sigBuf Pointer to downloaded signature buffer
 * @param sigLen Length of the signature
 * @return True if the update flashed and verified successfully.
 */
     bool runUpdate(Stream& in, uint32_t size, String md5, int command, uint8_t* sigBuf, size_t sigLen);
     
     // Helper for fetching signature
     bool fetchSignature(WiFiClient& client, const String& url, const String& token, uint8_t* sigBuf, size_t expectedLen);

     // Set the error and potentially use a CB to notify the application
     void _setLastError(int err) {
         _lastError = err;
         if (_cbError) {
             _cbError(err);
         }
     }
     int _lastError;
     bool _rebootOnUpdate = true;

 private:
     int _httpClientTimeout;
     md5Utils md5;

     // Callbacks
     HTTPUpdateStartCB    _cbStart;
     HTTPUpdateEndCB      _cbEnd;
     HTTPUpdateErrorCB    _cbError;
     HTTPUpdateProgressCB _cbProgress;
 };

 #if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_HTTPUPDATE)
 extern HTTPUpdate httpUpdate;
 #endif

 #endif /* ___HTTP_UPDATE_H___ */

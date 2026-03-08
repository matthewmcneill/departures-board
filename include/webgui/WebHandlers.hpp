/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: webgui/WebHandlers.hpp
 * Description: Contains HTTP endpoint handlers and related logic for the Web GUI.
 *
 * Exported Functions:
 * - sendResponse: Send HTTP responses with text messages.
 * - getContentType: Determine MIME type of files.
 * - handleStreamFile: Stream files from LittleFS to the client.
 * - handleStreamFlashFile: Stream PROGMEM files to the client.
 * - handleSaveKeys: Save API keys from the web form.
 * - handleSaveSettings: Save configuration from the web form.
 * - getFSInfo: Get LittleFS storage statistics.
 * - handleFileList: Send HTML directory listing of LittleFS.
 * - handleCat: Stream requested file to the browser.
 * - handleDelete: Delete requested file from the filesystem.
 * - handleFormatFFS: Format the LittleFS partition.
 * - handleFileUpload: Handle multipart file uploads to LittleFS.
 * - handleNotFound: Fallback handler for missing endpoints.
 * - getResultCodeText: Translate update status codes to strings.
 * - handleInfo: Send JSON system status information.
 * - handleRoot: Serve the root index page.
 * - handleFirmwareInfo: Serve the firmware version JSON.
 * - handleReboot: Trigger device reboot.
 * - handleEraseWiFi: Erase WiFiManager credentials.
 * - handleFactoryReset: Factory reset the device.
 * - handleBrightness: Update OLED brightness.
 * - handleOtaUpdate: Trigger manual OTA firmware check.
 * - handleControl: Sleep / screensaver control endpoint.
 * - handleStationPicker: Proxy requests to National Rail station picker.
 * - updateCurrentWeather: Fetch weather for lat/lon.
 */

/**
 * @brief Send a simple text response to the client (Flash String)
 * @param code HTTP status code
 * @param msg Flash string containing the response message
 */
void sendResponse(int code, const __FlashStringHelper* msg) {
  server.send(code, contentTypeText, msg);
}

/**
 * @brief Send a simple text response to the client (String)
 * @param code HTTP status code
 * @param msg String containing the response message
 */
void sendResponse(int code, String msg) {
  server.send(code, contentTypeText, msg);
}

/**
 * @brief Return the correct MIME type for a file name
 * @param filename Name of the file
 * @return String MIME type (e.g. text/html, image/png)
 */
String getContentType(String filename) {
  if (server.hasArg(F("download"))) {
    return F("application/octet-stream");
  } else if (filename.endsWith(F(".htm"))) {
/**
 * @brief f
 * @param "text/html"
 * @return Return value
 */
    return F("text/html");
  } else if (filename.endsWith(F(".html"))) {
/**
 * @brief f
 * @param "text/html"
 * @return Return value
 */
    return F("text/html");
  } else if (filename.endsWith(F(".css"))) {
/**
 * @brief f
 * @param "text/css"
 * @return Return value
 */
    return F("text/css");
  } else if (filename.endsWith(F(".js"))) {
/**
 * @brief f
 * @param "application/javascript"
 * @return Return value
 */
    return F("application/javascript");
  } else if (filename.endsWith(F(".png"))) {
/**
 * @brief f
 * @param "image/png"
 * @return Return value
 */
    return F("image/png");
  } else if (filename.endsWith(F(".gif"))) {
/**
 * @brief f
 * @param "image/gif"
 * @return Return value
 */
    return F("image/gif");
  } else if (filename.endsWith(F(".jpg"))) {
/**
 * @brief f
 * @param "image/jpeg"
 * @return Return value
 */
    return F("image/jpeg");
  } else if (filename.endsWith(F(".ico"))) {
/**
 * @brief f
 * @param "image/x-icon"
 * @return Return value
 */
    return F("image/x-icon");
  } else if (filename.endsWith(F(".xml"))) {
/**
 * @brief f
 * @param "text/xml"
 * @return Return value
 */
    return F("text/xml");
  } else if (filename.endsWith(F(".pdf"))) {
/**
 * @brief f
 * @param "application/x-pdf"
 * @return Return value
 */
    return F("application/x-pdf");
  } else if (filename.endsWith(F(".zip"))) {
/**
 * @brief f
 * @param "application/x-zip"
 * @return Return value
 */
    return F("application/x-zip");
  } else if (filename.endsWith(F(".json"))) {
/**
 * @brief f
 * @param "application/json"
 * @return Return value
 */
    return F("application/json");
  } else if (filename.endsWith(F(".gz"))) {
/**
 * @brief f
 * @param "application/x-gzip"
 * @return Return value
 */
    return F("application/x-gzip");
  } else if (filename.endsWith(F(".svg"))) {
/**
 * @brief f
 * @param "image/svg+xml"
 * @return Return value
 */
    return F("image/svg+xml");
  } else if (filename.endsWith(F(".webp"))) {
/**
 * @brief f
 * @param "image/webp"
 * @return Return value
 */
    return F("image/webp");
  }
/**
 * @brief f
 * @param "text/plain"
 * @return Return value
 */
  return F("text/plain");
}

/**
 * @brief Stream a file from the LittleFS file system to the client
 * @param filename File path on LittleFS
 * @return true if successful, false if file not found
 */
bool handleStreamFile(String filename) {
  if (LittleFS.exists(filename)) {
    File file = LittleFS.open(filename,"r");
    String contentType = getContentType(filename);
    server.streamFile(file, contentType);
    file.close();
    return true;
  } else return false;
}

/**
 * @brief Stream a file stored in PROGMEM flash to the client
 * @param filename Used to determine MIME type
 * @param filedata Pointer to PROGMEM byte array
 * @param contentLength Size of the file in bytes
 */
void handleStreamFlashFile(String filename, const uint8_t *filedata, size_t contentLength) {

  String contentType = getContentType(filename);
  WiFiClient client = server.client();
  // Send the headers
  client.println(F("HTTP/1.1 200 OK"));
  client.print(F("Content-Type: "));
  client.println(contentType);
  client.print(F("Content-Length: "));
  client.println(contentLength);
  client.println(F("Connection: close"));
  client.println(); // End of headers

  const size_t chunkSize = 512;
  uint8_t buffer[chunkSize];
  size_t sent = 0;

  while (sent < contentLength) {
    size_t toSend = min(chunkSize, contentLength - sent);
    // Copy from PROGMEM to buffer
    for (size_t i=0;i<toSend;i++) {
      buffer[i] = pgm_read_byte(&filedata[sent + i]);
    }
    client.write(buffer, toSend);
    sent += toSend;
  }
}

// Save the API keys POSTed from the keys.htm page
// If an OWM key is passed, this is tested before being committed to the file system. It's not possible
// to check the National Rail or TfL tokens at this point.
/**
 * @brief Endpoint to save API keys POSTed from the keys.htm page
 *        Validates OpenWeatherMap key if provided.
 */
void handleSaveKeys() {
  LOG_INFO("API configuration request received (handleSaveKeys).");
  String newJSON, owmToken, nrToken;
  JsonDocument doc;
  bool result = true;
  String msg = F("API keys saved successfully.");

  if ((server.method() == HTTP_POST) && (server.hasArg("plain"))) {
    newJSON = server.arg("plain");
    // Deserialise to get the OWM API key
    DeserializationError error = deserializeJson(doc, newJSON);
    if (!error) {
      JsonObject settings = doc.as<JsonObject>();
      if (settings[F("owmToken")].is<const char*>()) {
        owmToken = settings[F("owmToken")].as<String>();
        if (owmToken.length()) {
          // Check if this is a valid token...
          if (!currentWeather.updateWeather(owmToken, "51.52", "-0.13")) {
            msg = F("The OpenWeather Map API key is not valid. Please check you have copied your key correctly. It may take up to 30 minutes for a newly created key to become active.\n\nNo changes have been saved.");
            result = false;
          }
        }
      }
      if (result) {
        if (!saveFile(F("/apikeys.json"),newJSON)) {
          msg = F("Failed to save the API keys to the file system (file system corrupt or full?)");
          result = false;
        } else {
          nrToken = settings[F("nrToken")].as<String>();
          if (!nrToken.length()) msg+=F("\n\nNote: Only Tube and Bus Departures will be available without a National Rail token.");
          LOG_INFO("API keys successfully validated and saved.");
        }
      }
    } else {
      msg = F("Invalid JSON format. No changes have been saved.");
      LOG_ERROR("API keys update failed: Invalid JSON format received.");
      result = false;
    }
    if (result) {
      // Load/Update the API Keys in memory
      loadApiKeys();
      // If all location codes are blank we're in the setup process. If not, the keys have been changed so just reboot.
      if (!crsCode[0] && !tubeId[0] && !busAtco[0]) {
        sendResponse(200,msg);
        writeDefaultConfig();
        showSetupCrsHelpScreen();
      } else {
        msg += F("\n\nThe system will now restart.");
        sendResponse(200,msg);
        delay(500);
        ESP.restart();
      }
    } else {
      sendResponse(400,msg);
    }
  } else {
    sendResponse(400,F("Invalid"));
  }
}

/**
 * @brief Endpoint to save configuration settings POSTed from index.htm
 */
void handleSaveSettings() {
  LOG_INFO("Configuration settings update requested (handleSaveSettings).");
  String newJSON;

  if ((server.method() == HTTP_POST) && (server.hasArg("plain"))) {
    newJSON = server.arg("plain");
    saveFile(F("/config.json"),newJSON);
    LOG_INFO("Settings saved to /config.json successfully.");
    if ((!crsCode[0] && !tubeId[0]) || server.hasArg("reboot")) {
      // First time setup or base config change, we need a full reboot
      sendResponse(200,F("Configuration saved. The system will now restart."));
      delay(1000);
      ESP.restart();
    } else {
      sendResponse(200,F("Configuration updated. The system will update shortly."));
      softResetBoard();
    }
  } else {
    // Something went wrong saving the config file
    sendResponse(400,F("The configuration could not be updated. The system will restart."));
    delay(1000);
    ESP.restart();
  }
}

/*
 * Expose the file system via the Web GUI with some basic functions for directory browsing, file reading and deletion.
 */

 /**
 * @brief Return LittleFS storage information formatted as a string
 * @return String containing total and used bytes
 */
String getFSInfo() {
  char info[70];

  sprintf(info,"Total: %d bytes, Used: %d bytes\n",LittleFS.totalBytes(), LittleFS.usedBytes());
  return String(info);
}

/**
 * @brief Endpoint to send a basic HTML directory listing to the browser
 */
void handleFileList() {
  String path;
  if (!server.hasArg("dir")) path="/"; else path = server.arg("dir");
  File root = LittleFS.open(path);

  // Use chunked transfer encoding to stream the HTML out rather than fragmenting the heap
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, contentTypeHtml, "");

  server.sendContent(F("<html><body style=\"font-family:Helvetica,Arial,sans-serif\"><h2>Departures Board File System</h2>"));
  if (!root) {
    server.sendContent(F("<p>Failed to open directory</p>"));
  } else if (!root.isDirectory()) {
    server.sendContent(F("<p>Not a directory</p>"));
  } else {
    server.sendContent(F("<table>"));
    File file = root.openNextFile();
    while (file) {
      server.sendContent(F("<tr><td>"));
      if (file.isDirectory()) {
        server.sendContent("[DIR]</td><td><a href=\"/rmdir?f=" + String(file.path()) + F("\" title=\"Delete\">X</a></td><td><a href=\"/dir?dir=") + String(file.path()) + F("\">") + String(file.name()) + F("</a></td></tr>"));
      } else {
        server.sendContent(String(file.size()) + F("</td><td><a href=\"/del?f=")+ String(file.path()) + F("\" title=\"Delete\">X</a></td><td><a href=\"/cat?f=") + String(file.path()) + F("\">") + String(file.name()) + F("</a></td></tr>"));
      }
      file = root.openNextFile();
    }
  }

  server.sendContent(F("</table><br>"));
  server.sendContent(getFSInfo() + F("<p><a href=\"/upload\">Upload</a> a file</p></body></html>"));
  server.sendContent(""); // End of chunked transmission
}

/**
 * @brief Endpoint to stream a requested file to the browser
 */
void handleCat() {
  String filename;

  if (server.hasArg(F("f"))) {
    handleStreamFile(server.arg("f"));
  } else sendResponse(404,F("Not found"));
}

/**
 * @brief Endpoint to delete a file from the file system
 */
void handleDelete() {
  String filename;

  if (server.hasArg(F("f"))) {
    if (LittleFS.remove(server.arg(F("f")))) {
      // Successfully removed go back to directory listing
      server.sendHeader(F("Location"),F("/dir"));
      server.send(303);
    } else sendResponse(400,F("Failed to delete file"));
  } else sendResponse(404,F("Not found"));

}

/**
 * @brief Endpoint to format the LittleFS file system
 */
void handleFormatFFS() {
  String message;

  if (LittleFS.format()) {
    message=F("File System was successfully formatted\n\n");
    message+=getFSInfo();
  } else message=F("File System could not be formatted!");
  sendResponse(200,message);
}

/**
 * @brief Endpoint to handle multipart file uploads from the browser
 */
void handleFileUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith(F("/"))) {
      filename = "/" + filename;
    }
    fsUploadFile = LittleFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    WiFiClient client = server.client();
    if (fsUploadFile) {
      fsUploadFile.close();
      client.println(F("HTTP/1.1 302 Found"));
      client.println(F("Location: /success"));
      client.println(F("Connection: close"));
    } else {
      client.println(F("HTTP/1.1 500 Could not create file"));
      client.println(F("Connection: close"));
    }
    client.println();
  }
}

/*
 * Web GUI handlers
 */

/**
 * @brief Fallback function for browser requests handling 404s and proxying static assets
 */
void handleNotFound() {
  if ((LittleFS.exists(server.uri())) && (server.method() == HTTP_GET)) handleStreamFile(server.uri());
  else if (server.uri() == F("/keys.htm")) handleStreamFlashFile(server.uri(), keyshtm, sizeof(keyshtm));
  else if (server.uri() == F("/index.htm")) handleStreamFlashFile(server.uri(), indexhtm, sizeof(indexhtm));
  else if (server.uri() == F("/nrelogo.webp")) handleStreamFlashFile(server.uri(), nrelogo, sizeof(nrelogo));
  else if (server.uri() == F("/tfllogo.webp")) handleStreamFlashFile(server.uri(), tfllogo, sizeof(tfllogo));
  else if (server.uri() == F("/btlogo.webp")) handleStreamFlashFile(server.uri(), btlogo, sizeof(btlogo));
  else if (server.uri() == F("/tube.webp")) handleStreamFlashFile(server.uri(), tubeicon, sizeof(tubeicon));
  else if (server.uri() == F("/nr.webp")) handleStreamFlashFile(server.uri(), nricon, sizeof(nricon));
  else if (server.uri() == F("/favicon.png")) handleStreamFlashFile(server.uri(), faviconpng, sizeof(faviconpng));
  else if (server.uri() == F("/rss.json")) handleStreamFlashFile(server.uri(), rssjson, sizeof(rssjson));
/**
 * @brief Send response
 * @param 404
 * @param Found")
 * @return Return value
 */
  else sendResponse(404,F("Not Found"));
}

/**
 * @brief Translates an internal result code into a human readable text string
 * @param resultCode The internal status code (e.g. UPD_SUCCESS)
 * @return String Human readable error or success message
 */
String getResultCodeText(int resultCode) {
  switch (resultCode) {
    case UPD_SUCCESS:
/**
 * @brief f
 * @param "SUCCESS"
 * @return Return value
 */
      return F("SUCCESS");
      break;
    case UPD_NO_CHANGE:
/**
 * @brief f
 * @param CHANGES)"
 * @return Return value
 */
      return F("SUCCESS (NO CHANGES)");
      break;
    case UPD_DATA_ERROR:
/**
 * @brief f
 * @param ERROR"
 * @return Return value
 */
      return F("DATA ERROR");
      break;
    case UPD_UNAUTHORISED:
/**
 * @brief f
 * @param "UNAUTHORISED"
 * @return Return value
 */
      return F("UNAUTHORISED");
      break;
    case UPD_HTTP_ERROR:
/**
 * @brief f
 * @param ERROR"
 * @return Return value
 */
      return F("HTTP ERROR");
      break;
    case UPD_INCOMPLETE:
/**
 * @brief f
 * @param RECEIVED"
 * @return Return value
 */
      return F("INCOMPLETE DATA RECEIVED");
      break;
    case UPD_NO_RESPONSE:
/**
 * @brief f
 * @param SERVER"
 * @return Return value
 */
      return F("NO RESPONSE FROM SERVER");
      break;
    case UPD_TIMEOUT:
/**
 * @brief f
 * @param SERVER"
 * @return Return value
 */
      return F("TIMEOUT WAITING FOR SERVER");
      break;
    default:
/**
 * @brief f
 * @param ERROR"
 * @return Return value
 */
      return F("OTHER ERROR");
      break;
  }
}

/**
 * @brief Endpoint to send useful system & station information JSON to the browser
 */
void handleInfo() {
  unsigned long uptime = millis();
  char sysUptime[30];
  int days = uptime / msDay ;
  int hours = (uptime % msDay) / msHour;
  int minutes = ((uptime % msDay) % msHour) / msMin;

  // Let WebServer handle chunked transfer encoding automatically
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/plain", ""); // Sending empty string with content-length unknown starts chunked mode

  sprintf(sysUptime,"%d days, %d hrs, %d min", days,hours,minutes);

  server.sendContent("Hostname: " + String(hostname) + F("\nFirmware version: v") + String(VERSION_MAJOR) + "." + String(VERSION_MINOR) + " " + getBuildTime());
  server.sendContent(F("\nSystem uptime: "));
  server.sendContent(String(sysUptime));
  server.sendContent(F("\nFree Heap: "));
  server.sendContent(String(ESP.getFreeHeap()));
  server.sendContent(F("\nMax Free Block: "));
  server.sendContent(String(ESP.getMaxAllocHeap()));
  server.sendContent(F("\nFree LittleFS space: "));
  server.sendContent(String(LittleFS.totalBytes() - LittleFS.usedBytes()));
  server.sendContent("\nCore Plaform: " + String(ESP.getCoreVersion()) + F("\nCPU speed: ") + String(ESP.getCpuFreqMHz()) + F("MHz"));
  server.sendContent(F("\nCPU Temperature: "));
  server.sendContent(String(temperatureRead()));
  server.sendContent(F("\nWiFi network: "));
  server.sendContent(String(WiFi.SSID()));
  server.sendContent(F("\nWiFi signal strength: "));
  server.sendContent(String(WiFi.RSSI()));
  server.sendContent(F("dB"));
  
  getLocalTime(&timeinfo);
  sprintf(sysUptime,"%02d:%02d:%02d %02d/%02d/%04d",timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec,timeinfo.tm_mday,timeinfo.tm_mon+1,timeinfo.tm_year+1900);
  
  server.sendContent(F("\nSystem clock: "));
  server.sendContent(String(sysUptime));
  server.sendContent(F("\nCRS station code: "));
  server.sendContent(String(crsCode));
  server.sendContent(F("\nNaptan station code: "));
  server.sendContent(String(tubeId));
  server.sendContent(F("\nSuccessful: "));
  server.sendContent(String(dataLoadSuccess));
  server.sendContent(F("\nFailures: "));
  server.sendContent(String(dataLoadFailure));
  server.sendContent(F("\nTime since last data load: "));
  server.sendContent(String((int)((millis()-lastDataLoadTime)/1000)));
  server.sendContent(F(" seconds"));
  
  if (dataLoadFailure) {
    server.sendContent(F("\nTime since last failure: "));
    server.sendContent(String((int)((millis()-lastLoadFailure)/1000)));
    server.sendContent(F(" seconds"));
  }
  
  server.sendContent(F("\nLast Result: "));
  switch (boardMode) {
    case MODE_RAIL:
      server.sendContent(raildata->getLastError());
      break;

    case MODE_TUBE:
      server.sendContent(tfldata->lastErrorMsg);
      break;

    case MODE_BUS:
      server.sendContent(busdata->lastErrorMsg);
      break;
  }
  
  server.sendContent(F("\nUpdate result code: "));
  server.sendContent(getResultCodeText(lastUpdateResult));
  server.sendContent("\nServices: " + String(station.numServices) + F("\nMessages: "));
  
  int nMsgs = messages.numMessages;
  if (rssEnabled && rssAddedtoMsgs) nMsgs--;
  if (boardMode == MODE_TUBE) nMsgs--;
  
  server.sendContent(String(nMsgs) + F("\n\n"));

  if (rssEnabled) {
    server.sendContent(F("Last RSS result: "));
    server.sendContent(rss.getLastError());
    server.sendContent(F("\nResult code: "));
    server.sendContent(getResultCodeText(lastRssUpdateResult));
    server.sendContent(F("\nNext RSS update: "));
    server.sendContent(String((unsigned long)(nextRssUpdate-millis())));
    server.sendContent(F("ms\n\n"));
  }

  if (weatherEnabled) {
    server.sendContent(F("Last weather result: "));
    server.sendContent(currentWeather.lastErrorMsg);
    server.sendContent(F("\nNext weather update: "));
    server.sendContent(String((unsigned long)(nextWeatherUpdate-millis())));
    server.sendContent(F("ms\n"));
  }

  server.sendContent(""); // Empty content indicates end of chunked transmission
}

/**
 * @brief Endpoint to stream the index.htm page or setup guide
 */
void handleRoot() {
  if (!apiKeys) {
    if (LittleFS.exists(F("/keys.htm"))) handleStreamFile(F("/keys.htm")); else handleStreamFlashFile(F("/keys.htm"),keyshtm,sizeof(keyshtm));
  } else {
    if (LittleFS.exists(F("/index_d.htm"))) handleStreamFile(F("/index_d.htm")); else handleStreamFlashFile(F("/index.htm"),indexhtm,sizeof(indexhtm));
  }
}

/**
 * @brief Endpoint to send the firmware version to the client
 */
void handleFirmwareInfo() {
  String response = "{\"firmware\":\"B" + String(VERSION_MAJOR) + "." + String(VERSION_MINOR) + "-W" + String(WEBAPPVER_MAJOR) + "." + String(WEBAPPVER_MINOR) + F(" Build:") + getBuildTime() + F("\"}");
  server.send(200,contentTypeJson,response);
}

/**
 * @brief Endpoint to force a reboot of the ESP32
 */
void handleReboot() {
  sendResponse(200,F("The Departures Board is restarting..."));
  delay(1000);
  ESP.restart();
}

/**
 * @brief Endpoint to erase the stored WiFiManager credentials and reboot
 */
void handleEraseWiFi() {
  sendResponse(200,F("Erasing stored WiFi. You will need to connect to the \"Departures Board\" network and use WiFi Manager to reconfigure."));
  delay(1000);
  WiFiManager wm;
  wm.resetSettings();
  delay(500);
  ESP.restart();
}

/**
 * @brief Endpoint to factory reset the app (delete WiFi, format FS and reboot)
 */
void handleFactoryReset() {
  LOG_WARN("Factory reset initiated. Wiping WiFi credentials and formatting LittleFS.");
  sendResponse(200,F("Factory reseting the Departures Board..."));
  delay(1000);
  WiFiManager wm;
  wm.resetSettings();
  delay(500);
  LittleFS.format();
  delay(500);
  ESP.restart();
}

/**
 * @brief Endpoint to interactively change the brightness of the OLED panel
 */
void handleBrightness() {
  if (server.hasArg(F("b"))) {
    int level = server.arg(F("b")).toInt();
    if (level>0 && level<256) {
      u8g2.setContrast(level);
      brightness = level;
      sendResponse(200,F("OK"));
      return;
    }
  }
  sendResponse(200,F("invalid request"));
}

/**
 * @brief Endpoint where the Web GUI has requested an OTA GitHub update to be installed
 */
void handleOtaUpdate() {
  sendResponse(200,F("Update initiated - check Departure Board display for progress"));
  delay(500);
  u8g2.clearBuffer();
  u8g2.setFont(NatRailTall12);
  centreText(F("Getting latest firmware details from GitHub..."),26);
  u8g2.sendBuffer();

  if (ghUpdate.getLatestRelease()) {
    checkForFirmwareUpdate();
  } else {
    for (int i=15;i>=0;i--) {
      showUpdateCompleteScreen("Firmware Update Check Failed","Unable to retrieve latest release information.",ghUpdate.getLastError(),"",i,false);
      delay(1000);
    }
    log_e("FW Update failed: %s\n",ghUpdate.getLastError());
  }
  // Always restart
  ESP.restart();
}

/**
 * @brief Endpoint for controlling and querying sleep mode via JSON
 */
void handleControl() {
  String resp = "{\"sleeping\":";
  if (server.hasArg(F("sleep"))) {
    if (server.arg(F("sleep")) == "1") forcedSleep=true; else forcedSleep=false;
  }
  if (server.hasArg(F("clock"))) {
    if (server.arg(F("clock")) == "1") sleepClock=true; else sleepClock=false;
  }
  resp += (isSleeping || forcedSleep) ? "true":"false";
  resp += F(",\"display\":");
  resp += (sleepClock || (!isSleeping && !forcedSleep)) ? "true":"false";
  resp += "}";
  server.send(200, contentTypeJson, resp);
}

/*
 * External data functions - weather, stationpicker, firmware updates
 */

/**
 * @brief Endpoint to call the National Rail Station Picker API via proxy
 */
void handleStationPicker() {
  if (!server.hasArg(F("q"))) {
    sendResponse(400, F("Missing Query"));
    return;
  }

  String query = server.arg(F("q"));
  if (query.length() <= 2) {
    sendResponse(400, F("Query Too Short"));
    return;
  }

  const char* host = "stationpicker.nationalrail.co.uk";
  WiFiClientSecure httpsClient;
  httpsClient.setInsecure();
  httpsClient.setTimeout(10000);

  int retryCounter = 0;
  while (!httpsClient.connect(host, 443) && retryCounter++ < 20) {
    delay(50);
  }

  if (retryCounter >= 20) {
    sendResponse(408, F("NR Timeout"));
    return;
  }

  httpsClient.print(String("GET /stationPicker/") + query + F(" HTTP/1.0\r\n") +
                    F("Host: stationpicker.nationalrail.co.uk\r\n") +
                    F("Referer: https://www.nationalrail.co.uk\r\n") +
                    F("Origin: https://www.nationalrail.co.uk\r\n") +
                    F("Connection: close\r\n\r\n"));

  // Wait for response header
  retryCounter = 0;
  while (!httpsClient.available() && retryCounter++ < 15) {
    delay(100);
  }

  if (!httpsClient.available()) {
    httpsClient.stop();
    sendResponse(408, F("NRQ Timeout"));
    return;
  }

  // Parse status code
  String statusLine = httpsClient.readStringUntil('\n');
  if (!statusLine.startsWith(F("HTTP/")) || statusLine.indexOf(F("200 OK")) == -1) {
    httpsClient.stop();

    if (statusLine.indexOf(F("401")) > 0) {
      sendResponse(401, F("Not Authorized"));
    } else if (statusLine.indexOf(F("500")) > 0) {
      sendResponse(500, F("Server Error"));
    } else {
      sendResponse(503, statusLine.c_str());
    }
    return;
  }

  // Skip the remaining headers
  while (httpsClient.connected() || httpsClient.available()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") break;
  }

  // Start sending response
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, contentTypeJson, "");

  uint8_t buffer[512];
  size_t bufLen = 0;
  unsigned long timeout = millis() + 5000UL;
  WiFiClient client = server.client();

  while ((httpsClient.connected() || httpsClient.available()) && millis() < timeout) {
    while (httpsClient.available()) {
      char c = httpsClient.read();
      if (c <= 128) { // Only printable/standard ASCII
        buffer[bufLen++] = c;
      }
      
      if (bufLen >= sizeof(buffer)) {
        client.write(buffer, bufLen);
        bufLen = 0;
        yield();
      }
    }
  }

  // Flush remaining buffer
  if (bufLen > 0) {
    client.write(buffer, bufLen);
  }

  httpsClient.stop();
  server.sendContent(""); // empty chunk to terminate
  client.stop();
}

/**
 * @brief Update the current weather message if weather updates are enabled
 * @param latitude Station/Bus stop latitude
 * @param longitude Station/Bus stop longitude
 */
void updateCurrentWeather(float latitude, float longitude) {
  nextWeatherUpdate = millis() + 1200000; // update every 20 mins
  if (!latitude || !longitude) return; // No location co-ordinates
  strcpy(weatherMsg,"");
  bool currentWeatherValid = currentWeather.updateWeather(openWeatherMapApiKey, String(latitude), String(longitude));
  if (currentWeatherValid) {
    currentWeather.currentWeather.toCharArray(weatherMsg,sizeof(weatherMsg));
    weatherMsg[0] = toUpperCase(weatherMsg[0]);
    weatherMsg[sizeof(weatherMsg)-1] = '\0';
  } else {
    nextWeatherUpdate = millis() + 30000; // Try again in 30s
  }
}

/*
 * Setup / Loop functions
*/

//
// The main processing cycle for the National Rail Departures Board
//
void departureBoardLoop() {

  if (altStationEnabled && !isSleeping && altStationActive != isAltActive()) softResetBoard(); // Switch between station views

  if ((millis() > nextDataUpdate) && (!isScrollingStops) && (!isScrollingService) && (lastUpdateResult != UPD_UNAUTHORISED) && (!isSleeping) && (wifiConnected)) {
    timer = millis() + 2000;
    if (getStationBoard()) {
      if ((lastUpdateResult == UPD_SUCCESS) || (lastUpdateResult == UPD_NO_CHANGE && firstLoad)) {
        drawStationBoard(); // Something changed so redraw the board.
        dumpBoardToSerial(); 
      }
    } else if (lastUpdateResult == UPD_UNAUTHORISED) showTokenErrorScreen();
	  else if (lastUpdateResult == UPD_DATA_ERROR) {
	    if (noDataLoaded) showNoDataScreen();
/**
 * @brief Draw station board
 * @return Return value
 */
	    else drawStationBoard();
	  } else if (noDataLoaded) showNoDataScreen();
  } else if (weatherEnabled && (millis()>nextWeatherUpdate) && (!noDataLoaded) && (!isScrollingStops) && (!isScrollingService) && (!isSleeping) && (wifiConnected)) {
    updateCurrentWeather(stationLat,stationLon);
  } else if (rssEnabled && (millis()>nextRssUpdate) && (!noDataLoaded) && (!isScrollingStops) && (!isScrollingService) && (!isSleeping) && (wifiConnected)) {
    updateRssFeed();
  }

  if (millis()>timer && numMessages && !isScrollingStops && !isSleeping && lastUpdateResult!=UPD_UNAUTHORISED && lastUpdateResult!=UPD_DATA_ERROR && !noScrolling) {
    // Need to start a new scrolling line 2
    prevMessage = currentMessage;
    prevScrollStopsLength = scrollStopsLength;
    currentMessage++;
    if (currentMessage>=numMessages) currentMessage=0;
    scrollStopsXpos=0;
    scrollStopsYpos=10;
    scrollStopsLength = getStringWidth(line2[currentMessage]);
    isScrollingStops=true;
  }

  // Check if there's a via destination
  if (millis()>viaTimer) {
    if (station.numServices && station.service[0].via[0] && !isSleeping && lastUpdateResult!=UPD_UNAUTHORISED && lastUpdateResult!=UPD_DATA_ERROR) {
      isShowingVia = !isShowingVia;
      drawPrimaryService(isShowingVia);
      u8g2.updateDisplayArea(0,1,32,3);
      if (isShowingVia) viaTimer = millis()+3000; else viaTimer = millis()+4000;
    }
  }

  if (millis()>serviceTimer && !isScrollingService && !isSleeping && lastUpdateResult!=UPD_UNAUTHORISED && lastUpdateResult!=UPD_DATA_ERROR) {
    // Need to change to the next service if there is one
    if (station.numServices <= 1 && !weatherMsg[0]) {
      // There's no other services and no weather so just so static attribution.
      drawServiceLine(1,LINE3); //TODO?
      serviceTimer = millis() + 30000;
      isScrollingService = false;
    } else {
      prevService = line3Service;
      line3Service++;
      if (station.numServices) {
        if ((line3Service>station.numServices && !weatherMsg[0]) || (line3Service>station.numServices+1 && weatherMsg[0])) line3Service=(noScrolling && station.numServices>1) ? 2:1;  // First 'other' service
      } else {
        if (weatherMsg[0] && line3Service>1) line3Service=0;
      }
      scrollServiceYpos=10;
      isScrollingService = true;
    }
  }

  if (isScrollingStops && millis()>timer && !isSleeping && !noScrolling) {
    blankArea(0,LINE2,256,9);
    if (scrollStopsYpos) {
      // we're scrolling up the message initially
      u8g2.setClipWindow(0,LINE2,256,LINE2+9);
      // if the previous message didn't scroll then we need to scroll it up off the screen
      if (prevScrollStopsLength && prevScrollStopsLength<256 && strncmp("Calling",line2[prevMessage],7)) centreText(line2[prevMessage],scrollStopsYpos+LINE2-12);
      if (scrollStopsLength<256 && strncmp("Calling",line2[currentMessage],7)) centreText(line2[currentMessage],scrollStopsYpos+LINE2-2); // Centre text if it fits
      else u8g2.drawStr(0,scrollStopsYpos+LINE2-2,line2[currentMessage]);
      u8g2.setMaxClipWindow();
      scrollStopsYpos--;
      if (scrollStopsYpos==0) timer=millis()+1500;
    } else {
      // we're scrolling left
      if (scrollStopsLength<256 && strncmp("Calling",line2[currentMessage],7)) centreText(line2[currentMessage],LINE2-1); // Centre text if it fits
      else u8g2.drawStr(scrollStopsXpos,LINE2-1,line2[currentMessage]);
      if (scrollStopsLength < 256) {
        // we don't need to scroll this message, it fits so just set a longer timer
        timer=millis()+6000;
        isScrollingStops=false;
      } else {
        scrollStopsXpos--;
        if (scrollStopsXpos < -scrollStopsLength) {
          isScrollingStops=false;
          timer=millis()+500;  // pause before next message
        }
      }
    }
  }

  if (isScrollingService && millis()>serviceTimer && !isSleeping) {
    blankArea(0,LINE3,256,9);
    if (scrollServiceYpos) {
      // we're scrolling the service into view
      u8g2.setClipWindow(0,LINE3,256,LINE3+9);
      // if the prev service is showing, we need to scroll it up off
      if (prevService>0) drawServiceLine(prevService,scrollServiceYpos+LINE3-12);
      drawServiceLine(line3Service,scrollServiceYpos+LINE3-1);
      u8g2.setMaxClipWindow();
      scrollServiceYpos--;
      if (scrollServiceYpos==0) {
        serviceTimer=millis()+5000;
        isScrollingService=false;
      }
    }
  }

  if (!isSleeping) {
    // Check if the clock should be updated
    doClockCheck();

    // To ensure a consistent refresh rate (for smooth text scrolling), we update the screen every 25ms (around 40fps)
    // so we need to wait any additional ms not used by processing so far before sending the frame to the display controller
    long delayMs = fpsDelay - (millis()-refreshTimer);
    if (delayMs>0) delay(delayMs);
    u8g2.updateDisplayArea(0,3,32,4);
    refreshTimer=millis();
  }
}

//
// Processing loop for London Underground Arrivals board
//
void undergroundArrivalsLoop() {
  char serviceData[8+MAXLINESIZE+MAXLOCATIONSIZE];
  bool fullRefresh = false;

  if (millis()>nextDataUpdate && !isScrollingService && !isScrollingPrimary && !isSleeping && wifiConnected) {
    if (getUndergroundBoard()) {
      if ((lastUpdateResult == UPD_SUCCESS) || (lastUpdateResult == UPD_NO_CHANGE && firstLoad)) {
         drawUndergroundBoard();
         dumpBoardToSerial();
      }
    } else if (lastUpdateResult == UPD_UNAUTHORISED) showTokenErrorScreen();
	  else if (lastUpdateResult == UPD_DATA_ERROR) {
	    if (noDataLoaded) showNoDataScreen();
/**
 * @brief Draw underground board
 * @return Return value
 */
	    else drawUndergroundBoard();
	  } else if (noDataLoaded) showNoDataScreen();
  }

    // Scrolling the additional services
  if (millis()>serviceTimer && !isScrollingService && !isSleeping && !noDataLoaded && lastUpdateResult!=UPD_UNAUTHORISED && lastUpdateResult!=UPD_DATA_ERROR) {
    if (station.numServices<=2 && messages.numMessages==1 && attributionScrolled) {
      // There are no additional services to scroll in so static attribution.
      serviceTimer = millis() + 30000;
    } else {
      // Need to change to the next service or message if there is one
      attributionScrolled = true;
      prevService = line3Service;
      line3Service++;
      scrollServiceYpos=11;
      scrollStopsXpos=0;
      isScrollingService = true;
      if (line3Service>=station.numServices) {
        // Showing the messages
        prevMessage = currentMessage;
        prevScrollStopsLength = scrollStopsLength;  // Save the length of the previous message
        currentMessage++;
        if (currentMessage>=messages.numMessages) {
          if (station.numServices>2) {
            line3Service=2;
            currentMessage=-1; // Rollover back to services
          } else {
            line3Service = station.numServices;
            currentMessage=0;
          }
        }
        scrollStopsLength = getStringWidth(line2[currentMessage]);
      } else {
        scrollStopsLength=SCREEN_WIDTH;
      }
    }
  }

  if (isScrollingService && millis()>serviceTimer && !isSleeping) {
    blankArea(0,ULINE3,256,10);
    if (scrollServiceYpos) {
      // we're scrolling up the message initially
      u8g2.setClipWindow(0,ULINE3,256,ULINE3+10);
      // Was the previous display a service?
      if (prevService<station.numServices) {
        drawUndergroundService(prevService,scrollServiceYpos+ULINE3-13);
      } else {
        // if the previous message didn't scroll then we need to scroll it up off the screen
        if (prevScrollStopsLength && prevScrollStopsLength<256) centreText(line2[prevMessage],scrollServiceYpos+ULINE3-13);
      }
      // Is this entry a service?
      if (line3Service<station.numServices) {
        drawUndergroundService(line3Service,scrollServiceYpos+ULINE3-1);
      } else {
        if (scrollStopsLength<256) centreText(line2[currentMessage],scrollServiceYpos+ULINE3-2); // Centre text if it fits
        else u8g2.drawStr(0,scrollServiceYpos+ULINE3-2,line2[currentMessage]);
      }
      u8g2.setMaxClipWindow();
      scrollServiceYpos--;
      if (scrollServiceYpos==0) {
        if (line3Service<station.numServices) {
          serviceTimer=millis()+3500;
          isScrollingService=false;
        } else {
          serviceTimer=millis()+500;
        }
      }
    } else {
      // we're scrolling left
      if (scrollStopsLength<256) centreText(line2[currentMessage],ULINE3-1); // Centre text if it fits
      else u8g2.drawStr(scrollStopsXpos,ULINE3-1,line2[currentMessage]);
      if (scrollStopsLength < 256) {
        // we don't need to scroll this message, it fits so just set a longer timer
        serviceTimer=millis()+3000;
        isScrollingService=false;
      } else {
        scrollStopsXpos--;
        if (scrollStopsXpos < -scrollStopsLength) {
          isScrollingService=false;
          serviceTimer=millis()+500;  // pause before next message
        }
      }
    }
  }

  if (isScrollingPrimary && !isSleeping) {
    blankArea(0,ULINE1,256,ULINE3-ULINE1);
    fullRefresh = true;
    // we're scrolling the primary service(s) into view
    u8g2.setClipWindow(0,ULINE1,256,ULINE1+10);
    if (station.numServices) drawUndergroundService(0,scrollPrimaryYpos+ULINE1-1);
    else centreText(F("There are no scheduled arrivals at this station."),scrollPrimaryYpos+ULINE1-1);
    if (station.numServices>1) {
      u8g2.setClipWindow(0,ULINE2,256,ULINE2+10);
      drawUndergroundService(1,scrollPrimaryYpos+ULINE2-1);
    }
    u8g2.setMaxClipWindow();
    scrollPrimaryYpos--;
    if (scrollPrimaryYpos==0) {
      isScrollingPrimary=false;
    }
  }

  if (!isSleeping) {
    // Check if the clock should be updated
    if (millis()>nextClockUpdate) {
      nextClockUpdate = millis()+250;
      drawCurrentTimeUG(true);
    }

    long delayMs = 18 - (millis()-refreshTimer);
    if (delayMs>0) delay(delayMs);
    if (fullRefresh) u8g2.updateDisplayArea(0,1,32,6); else u8g2.updateDisplayArea(0,5,32,2);
    refreshTimer=millis();
  }
}

//
// Processing loop for Bus Departures board
//
void busDeparturesLoop() {
  char serviceData[8+MAXLINESIZE+MAXLOCATIONSIZE];
  bool fullRefresh = false;

  if (millis()>nextDataUpdate && !isScrollingService && !isScrollingPrimary && !isSleeping && wifiConnected) {
    if (getBusDeparturesBoard()) {
      if ((lastUpdateResult == UPD_SUCCESS) || (lastUpdateResult == UPD_NO_CHANGE && firstLoad)) {
          drawBusDeparturesBoard(); // Something changed so redraw the board.
          dumpBoardToSerial();
      }
    } else if (lastUpdateResult == UPD_UNAUTHORISED) showTokenErrorScreen();
	  else if (lastUpdateResult == UPD_DATA_ERROR) {
	    if (noDataLoaded) showNoDataScreen();
/**
 * @brief Draw bus departures board
 * @return Return value
 */
	    else drawBusDeparturesBoard();
	  } else if (noDataLoaded) showNoDataScreen();
  } else if (weatherEnabled && millis()>nextWeatherUpdate && !noDataLoaded && !isScrollingService && !isScrollingPrimary && !isSleeping && wifiConnected) {
    updateCurrentWeather(busLat,busLon);
    // Update the weather text immediately
    if (weatherMsg[0]) {
      strcpy(line2[1],btAttribution);
      strcpy(line2[0],weatherMsg);
      messages.numMessages=2;
    }
  }

  // Scrolling the additional services
  if (millis()>serviceTimer && !isScrollingPrimary && !isScrollingService && !isSleeping && !noDataLoaded && lastUpdateResult!=UPD_UNAUTHORISED && lastUpdateResult!=UPD_DATA_ERROR) {
    // Need to change to the next service if there is one
    if (station.numServices<=2 && messages.numMessages==1) {
      // There are no additional services or weather to scroll in so static attribution.
      serviceTimer = millis() + 10000;
      line3Service=station.numServices;
    } else {
      // Need to change to the next service or message
      prevService = line3Service;
      line3Service++;
      scrollServiceYpos=11;
      isScrollingService = true;
      if (line3Service>=station.numServices) {
        // Showing the messages
        prevMessage = currentMessage;
        currentMessage++;
        if (currentMessage>=messages.numMessages) {
          if (station.numServices>2) {
            line3Service = 2;
            currentMessage=-1; // Rollover back to services
          } else {
            line3Service = station.numServices;
            currentMessage=0;
          }
        }
      }
    }
  }

  if (isScrollingService && millis()>serviceTimer && !isSleeping) {
    if (scrollServiceYpos) {
      blankArea(0,ULINE3,256,10);
      // we're scrolling up the message
      u8g2.setClipWindow(0,ULINE3,256,ULINE3+10);
      // Was the previous display a service?
      if (prevService<station.numServices) {
        drawBusService(prevService,scrollServiceYpos+ULINE3-13,busDestX);
      } else {
        // Scrolling up the previous message
        centreText(line2[prevMessage],scrollServiceYpos+ULINE3-13);
      }
      // Is this entry a service?
      if (line3Service<station.numServices) {
        drawBusService(line3Service,scrollServiceYpos+ULINE3-1,busDestX);
      } else {
        centreText(line2[currentMessage],scrollServiceYpos+ULINE3-2);
      }
      u8g2.setMaxClipWindow();
      scrollServiceYpos--;
      if (scrollServiceYpos==0) {
        serviceTimer = millis()+2800;
        if (station.numServices<=2) serviceTimer+=3000;
      }
    } else isScrollingService=false;
  }

  if (isScrollingPrimary && !isSleeping) {
    blankArea(0,ULINE1,256,ULINE3-ULINE1+10);
    fullRefresh = true;
    // we're scrolling the primary service(s) into view
    u8g2.setClipWindow(0,ULINE1,256,ULINE1+10);
    if (station.numServices) drawBusService(0,scrollPrimaryYpos+ULINE1-1,busDestX);
    else centreText(F("There are no scheduled services at this stop."),scrollPrimaryYpos+ULINE1-1);
    if (station.numServices>1) {
      u8g2.setClipWindow(0,ULINE2,256,ULINE2+10);
      drawBusService(1,scrollPrimaryYpos+ULINE2-1,busDestX);
    }
    if (station.numServices>2) {
      u8g2.setClipWindow(0,ULINE3,256,ULINE3+10);
      drawBusService(2,scrollPrimaryYpos+ULINE3-1,busDestX);
    } else if (station.numServices<3 && messages.numMessages==1) {
      // scroll up the attribution once...
      u8g2.setClipWindow(0,ULINE3,256,ULINE3+10);
      centreText(btAttribution,scrollPrimaryYpos+ULINE3-1);
    }
    u8g2.setMaxClipWindow();
    scrollPrimaryYpos--;
    if (scrollPrimaryYpos==0) {
      isScrollingPrimary=false;
      serviceTimer = millis()+2800;
    }
  }

  if (!isSleeping) {
    // Check if the clock should be updated
    if (millis()>nextClockUpdate) {
      nextClockUpdate = millis()+250;
      drawCurrentTimeUG(true);    // just use the Tube clock for bus mode
      u8g2.setFont(NatRailSmall9);
    }

    long delayMs = 40 - (millis()-refreshTimer);
    if (delayMs>0) delay(delayMs);
    if (fullRefresh) u8g2.updateDisplayArea(0,1,32,6); else u8g2.updateDisplayArea(0,5,32,2);
    refreshTimer=millis();
  }
}

//
// Setup code

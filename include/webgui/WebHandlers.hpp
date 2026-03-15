#pragma once

extern class appContext appContext;
extern WebServer server;

static const char* contentTypeText = "text/plain";
static const char* contentTypeHtml = "text/html";
static const char* contentTypeJson = "application/json";

static const unsigned long msMin = 60000;
static const unsigned long msHour = 3600000;
static const unsigned long msDay = 86400000;
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

#include <boards/interfaces/iDataSource.hpp>
#include <boards/nationalRailBoard/nationalRailBoard.hpp>
#include <boards/tflBoard/tflBoard.hpp>
#include <boards/busBoard/busBoard.hpp>
#include <boards/systemBoard/sleepingBoard.hpp>
#include <boards/systemBoard/firmwareUpdateBoard.hpp>
#include <boards/systemBoard/messageBoard.hpp>
#include <configManager.hpp>



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
    return F("text/html");
  } else if (filename.endsWith(F(".html"))) {
    return F("text/html");
  } else if (filename.endsWith(F(".css"))) {
    return F("text/css");
  } else if (filename.endsWith(F(".js"))) {
    return F("application/javascript");
  } else if (filename.endsWith(F(".png"))) {
    return F("image/png");
  } else if (filename.endsWith(F(".gif"))) {
    return F("image/gif");
  } else if (filename.endsWith(F(".jpg"))) {
    return F("image/jpeg");
  } else if (filename.endsWith(F(".ico"))) {
    return F("image/x-icon");
  } else if (filename.endsWith(F(".xml"))) {
    return F("text/xml");
  } else if (filename.endsWith(F(".pdf"))) {
    return F("application/x-pdf");
  } else if (filename.endsWith(F(".zip"))) {
    return F("application/x-zip");
  } else if (filename.endsWith(F(".json"))) {
    return F("application/json");
  } else if (filename.endsWith(F(".gz"))) {
    return F("application/x-gzip");
  } else if (filename.endsWith(F(".svg"))) {
    return F("image/svg+xml");
  } else if (filename.endsWith(F(".webp"))) {
    return F("image/webp");
  }
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
  LOG_INFO("WEB", "API configuration request received (handleSaveKeys).");
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
          // Check if this is a valid token...
          WeatherStatus tempStatus;
          tempStatus.lat = 51.52;
          tempStatus.lon = -0.13;
          // Temporarily swap key for validation
          String oldKey = appContext.getWeather().getOpenWeatherMapApiKey();
          appContext.getWeather().setOpenWeatherMapApiKey(owmToken.c_str());
          if (!appContext.getWeather().updateWeather(tempStatus)) {
            msg = F("The OpenWeather Map API key is not valid. Please check you have copied your key correctly. It may take up to 30 minutes for a newly created key to become active.\n\nNo changes have been saved.");
            result = false;
          }
          appContext.getWeather().setOpenWeatherMapApiKey(oldKey.c_str());
        }
        if (result) {
          if (!appContext.getConfigManager().saveFile(F("/apikeys.json"),newJSON)) {
            msg = F("Failed to save the API keys to the file system (file system corrupt or full?)");
            result = false;
          } else {
            nrToken = settings[F("nrToken")].as<String>();
            if (!nrToken.length()) msg+=F("\n\nNote: Only Tube and Bus Departures will be available without a National Rail token.");
            LOG_INFO("WEB", "API keys successfully validated and saved.");
          }
        }
      } else {
      msg = F("Invalid JSON format. No changes have been saved.");
      LOG_ERROR("WEB", "API keys update failed: Invalid JSON format received.");
      result = false;
    }
    if (result) {
      // Load/Update the API Keys in memory
      appContext.getConfigManager().loadApiKeys();
      const Config& config = appContext.getConfigManager().getConfig();
      // If all location codes are blank we're in the setup process. If not, the keys have been changed so just reboot.
      if (!config.crsCode[0] && !config.tubeId[0] && !config.busId[0]) {
        sendResponse(200,msg);
        appContext.getConfigManager().writeDefaultConfig();
        appContext.getDisplayManager().showBoard(appContext.getDisplayManager().getSystemBoard(SystemBoardId::SYS_HELP_CRS));
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
 * @brief Endpoint to serve the current configuration settings as JSON to the browser.
 *        Constructs a flat JSON object compatible with the current legacy Web GUI.
 */
void handleGetConfigSettings() {
  LOG_INFO("WEB", "Configuration settings requested (handleGetConfigSettings).");
  const Config& config = appContext.getConfigManager().getConfig();
  JsonDocument doc;

  // System Settings
  doc[F("hostname")] = config.hostname;
  doc[F("TZ")] = config.timezone;
  doc[F("appKey")] = "";
  doc[F("brightness")] = config.brightness;
  doc[F("mode")] = config.defaultBoardIndex;
  doc[F("sleep")] = config.sleepEnabled;
  doc[F("sleepStarts")] = config.sleepStarts;
  doc[F("sleepEnds")] = config.sleepEnds;
  doc[F("showDate")] = config.dateEnabled;
  doc[F("clock")] = config.showClockInSleep;
  doc[F("update")] = config.firmwareUpdatesEnabled;
  doc[F("updateDaily")] = config.dailyUpdateCheckEnabled;
  doc[F("noScroll")] = config.noScrolling;
  doc[F("flip")] = config.flipScreen;
  doc[F("fastRefresh")] = (config.apiRefreshRate == FASTDATAUPDATEINTERVAL);
  doc[F("rssUrl")] = config.rssUrl;
  doc[F("rssName")] = config.rssName;

  // Map from internal board array to legacy flat fields for GUI compatibility
  // Rail (Slot 0)
  for (int i = 0; i < config.boardCount; i++) {
    const BoardConfig& bc = config.boards[i];
    if (bc.type == MODE_RAIL && i == 0) {
      doc[F("crs")] = bc.id;
      doc[F("station")] = bc.name;
      // Protection: Only write coordinates for the main Rail board if it's the active slot
      // or if no other board has claimed the 'lat'/'lon' keys yet.
      if (!doc[F("lat")] || (config.defaultBoardIndex == i && bc.lat != 0)) {
          doc[F("lat")] = bc.lat;
          doc[F("lon")] = bc.lon;
      }
      doc[F("callingCrs")] = bc.secondaryId;
      doc[F("callingStation")] = bc.secondaryName;
      doc[F("platformFilter")] = bc.filter;
      doc[F("nrTimeOffset")] = bc.timeOffset;
    } else if (bc.type == MODE_TUBE) {
      doc[F("tubeId")] = bc.id;
      doc[F("tubeName")] = bc.name;
      // Protection: Only write coordinates for Tube if it's the active slot
      // or if no other board (like Rail) has provided them yet.
      if (!doc[F("lat")] || (config.defaultBoardIndex == i && bc.lat != 0)) {
          doc[F("lat")] = bc.lat;
          doc[F("lon")] = bc.lon;
      }
    } else if (bc.type == MODE_BUS) {
      doc[F("busId")] = bc.id;
      doc[F("busName")] = bc.name;
      doc[F("busLat")] = bc.lat;
      doc[F("busLon")] = bc.lon;
      doc[F("busFilter")] = bc.filter;
    } else if (bc.type == MODE_RAIL && i > 0) {
      // Alt Rail logic
      doc[F("altCrs")] = bc.id;
      doc[F("altLat")] = bc.lat;
      doc[F("altLon")] = bc.lon;
      doc[F("altCallingCrs")] = bc.secondaryId;
      doc[F("altCallingStation")] = bc.secondaryName;
      doc[F("altPlatformFilter")] = bc.filter;
    }
  }

  // Weather is tied to Global context currently
  doc[F("weather")] = appContext.getWeather().getWeatherEnabled();
  doc[F("showBus")] = config.altStationEnabled; // Legacy flag for 'Include bus replacement' in NRE

  String output;
  serializeJson(doc, output);
  server.send(200, contentTypeJson, output);
}

/**
 * @brief Endpoint to save configuration settings POSTed from index.htm.
 *        Updates the in-memory Config struct and then calls save() to persist.
 */
void handleSaveSettings() {
  LOG_INFO("WEB", "Configuration settings update requested (handleSaveSettings).");
  
  if ((server.method() == HTTP_POST) && (server.hasArg("plain"))) {
    String newJSON = server.arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, newJSON);
    
    if (error) {
      LOG_ERROR("WEB", String("Failed to parse settings JSON: ") + error.c_str());
      sendResponse(400, F("Invalid settings format."));
      return;
    }

    Config& config = appContext.getConfigManager().getConfig();
    JsonObject settings = doc.as<JsonObject>();

    // Update System Settings
    if (settings[F("hostname")].is<const char*>()) strlcpy(config.hostname, settings[F("hostname")], sizeof(config.hostname));
    if (settings[F("TZ")].is<const char*>()) strlcpy(config.timezone, settings[F("TZ")], sizeof(config.timezone));
    if (settings[F("brightness")].is<int>()) config.brightness = settings[F("brightness")];
    if (settings[F("mode")].is<int>()) config.defaultBoardIndex = settings[F("mode")];
    if (settings[F("sleep")].is<bool>()) config.sleepEnabled = settings[F("sleep")];
    if (settings[F("sleepStarts")].is<int>()) config.sleepStarts = settings[F("sleepStarts")];
    if (settings[F("sleepEnds")].is<int>()) config.sleepEnds = settings[F("sleepEnds")];
    if (settings[F("showDate")].is<bool>()) config.dateEnabled = settings[F("showDate")];
    if (settings[F("clock")].is<bool>()) config.showClockInSleep = settings[F("clock")];
    if (settings[F("update")].is<bool>()) config.firmwareUpdatesEnabled = settings[F("update")];
    if (settings[F("updateDaily")].is<bool>()) config.dailyUpdateCheckEnabled = settings[F("updateDaily")];
    if (settings[F("noScroll")].is<bool>()) config.noScrolling = settings[F("noScroll")];
    if (settings[F("flip")].is<bool>()) config.flipScreen = settings[F("flip")];
    if (settings[F("fastRefresh")].is<bool>()) config.apiRefreshRate = settings[F("fastRefresh")] ? FASTDATAUPDATEINTERVAL : DATAUPDATEINTERVAL;
    if (settings[F("rssUrl")].is<const char*>()) strlcpy(config.rssUrl, settings[F("rssUrl")], sizeof(config.rssUrl));
    if (settings[F("rssName")].is<const char*>()) strlcpy(config.rssName, settings[F("rssName")], sizeof(config.rssName));

    // Map legacy flat fields back to the boards array
    // This maintains compatibility while the Web GUI is still flat.
    // In the future, the GUI should send a "boards" array directly.
    
    // Board 0: Main Rail (by convention)
    BoardConfig& br = config.boards[0];
    br.type = MODE_RAIL;
    if (settings[F("crs")].is<const char*>()) strlcpy(br.id, settings[F("crs")], sizeof(br.id));
    if (settings[F("station")].is<const char*>()) strlcpy(br.name, settings[F("station")], sizeof(br.name));
    // ONLY update Rail coordinates if Rail mode is currently active
    if (settings[F("type")].as<int>() == MODE_RAIL) {
        if (settings[F("lat")].is<float>() && settings[F("lat")].as<float>() != 0) br.lat = settings[F("lat")].as<float>();
        if (settings[F("lon")].is<float>() && settings[F("lon")].as<float>() != 0) br.lon = settings[F("lon")].as<float>();
    }
    if (settings[F("callingCrs")].is<const char*>()) strlcpy(br.secondaryId, settings[F("callingCrs")], sizeof(br.secondaryId));
    if (settings[F("callingStation")].is<const char*>()) strlcpy(br.secondaryName, settings[F("callingStation")], sizeof(br.secondaryName));
    if (settings[F("platformFilter")].is<const char*>()) strlcpy(br.filter, settings[F("platformFilter")], sizeof(br.filter));
    if (settings[F("nrTimeOffset")].is<int>()) br.timeOffset = settings[F("nrTimeOffset")];
    br.complete = (strlen(br.id) > 0);

    // Board 1: Tube
    BoardConfig& bt = config.boards[1];
    bt.type = MODE_TUBE;
    if (settings[F("tubeId")].is<const char*>()) strlcpy(bt.id, settings[F("tubeId")], sizeof(bt.id));
    if (settings[F("tubeName")].is<const char*>()) strlcpy(bt.name, settings[F("tubeName")], sizeof(bt.name));
    // ONLY update Tube coordinates if Tube mode is currently active
    if (settings[F("type")].as<int>() == MODE_TUBE) {
        if (settings[F("lat")].is<float>() && settings[F("lat")].as<float>() != 0) bt.lat = settings[F("lat")].as<float>();
        if (settings[F("lon")].is<float>() && settings[F("lon")].as<float>() != 0) bt.lon = settings[F("lon")].as<float>();
    }
    bt.complete = (strlen(bt.id) > 0);

    // Board 2: Bus
    BoardConfig& bb = config.boards[2];
    bb.type = MODE_BUS;
    if (settings[F("busId")].is<const char*>()) strlcpy(bb.id, settings[F("busId")], sizeof(bb.id));
    if (settings[F("busName")].is<const char*>()) strlcpy(bb.name, settings[F("busName")], sizeof(bb.name));
    if (settings[F("busLat")].is<float>()) bb.lat = settings[F("busLat")].as<float>();
    if (settings[F("busLon")].is<float>()) bb.lon = settings[F("busLon")].as<float>();
    if (settings[F("busFilter")].is<const char*>()) strlcpy(bb.filter, settings[F("busFilter")], sizeof(bb.filter));
    bb.complete = (strlen(bb.id) > 0);

    // Board 3: Alt Rail
    if (settings[F("altCrs")].is<const char*>() && strlen(settings[F("altCrs")]) > 0) {
        BoardConfig& ba = config.boards[3];
        ba.type = MODE_RAIL;
        strlcpy(ba.id, settings[F("altCrs")], sizeof(ba.id));
        if (settings[F("altLat")].is<float>()) ba.lat = settings[F("altLat")].as<float>();
        if (settings[F("altLon")].is<float>()) ba.lon = settings[F("altLon")].as<float>();
        if (settings[F("altCallingCrs")].is<const char*>()) strlcpy(ba.secondaryId, settings[F("altCallingCrs")], sizeof(ba.secondaryId));
        if (settings[F("altCallingStation")].is<const char*>()) strlcpy(ba.secondaryName, settings[F("altCallingStation")], sizeof(ba.secondaryName));
        if (settings[F("altPlatformFilter")].is<const char*>()) strlcpy(ba.filter, settings[F("altPlatformFilter")], sizeof(ba.filter));
        ba.complete = true;
        config.boardCount = 4;
    } else {
        config.boardCount = 3;
    }

    // Persist to flash
    appContext.getConfigManager().save();
    
    LOG_INFO("WEB", "Settings updated in memory and saved to disk.");
    
    if (server.hasArg("reboot")) {
      sendResponse(200, F("Configuration saved. The system will now restart."));
      delay(1000);
      ESP.restart();
    } else {
      sendResponse(200, F("Configuration updated. The system will update shortly."));
      appContext.getsystemManager().softResetBoard();
    }
  } else {
    sendResponse(400, F("Invalid request. No settings provided."));
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

void handleNotFound() {
  LOG_INFO("WEB", "Request: " + server.uri() + " from " + server.client().remoteIP().toString());
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
      return F("SUCCESS");
      break;
    case UPD_NO_CHANGE:
      return F("SUCCESS (NO CHANGES)");
      break;
    case UPD_DATA_ERROR:
      return F("DATA ERROR");
      break;
    case UPD_UNAUTHORISED:
      return F("UNAUTHORISED");
      break;
    case UPD_HTTP_ERROR:
      return F("HTTP ERROR");
      break;
    case UPD_INCOMPLETE:
      return F("INCOMPLETE DATA RECEIVED");
      break;
    case UPD_NO_RESPONSE:
      return F("NO RESPONSE FROM SERVER");
      break;
    case UPD_TIMEOUT:
      return F("TIMEOUT WAITING FOR SERVER");
      break;
    default:
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

  const Config& config = appContext.getConfigManager().getConfig();
  server.sendContent("Hostname: " + String(config.hostname) + F("\nFirmware version: v") + String(VERSION_MAJOR) + "." + String(VERSION_MINOR) + " " + appContext.getsystemManager().getBuildTime());
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
  
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  sprintf(sysUptime,"%02d:%02d:%02d %02d/%02d/%04d",timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec,timeinfo.tm_mday,timeinfo.tm_mon+1,timeinfo.tm_year+1900);
  
  server.sendContent(F("\nSystem clock: "));
  server.sendContent(String(sysUptime));
  server.sendContent(F("\nCRS station code: "));
  server.sendContent(String(config.crsCode));
  server.sendContent(F("\nNaptan station code: "));
  server.sendContent(String(config.tubeId));
  server.sendContent(F("\nSuccessful: "));
  server.sendContent(String(appContext.getsystemManager().getDataLoadSuccess()));
  server.sendContent(F("\nFailures: "));
  server.sendContent(String(appContext.getsystemManager().getDataLoadFailure()));
  server.sendContent(F("\nTime since last data load: "));
  server.sendContent(String((int)((millis()-appContext.getsystemManager().getLastDataLoadTime())/1000)));
  server.sendContent(F(" seconds"));
  
  if (appContext.getsystemManager().getDataLoadFailure()) {
    server.sendContent(F("\nTime since last failure: "));
    server.sendContent(String((int)((millis()-appContext.getsystemManager().getLastLoadFailure())/1000)));
    server.sendContent(F(" seconds"));
  }
  
  server.sendContent(F("\nLast Result: "));
  if (appContext.getsystemManager().getLastUpdateResult()==UPD_SUCCESS || appContext.getsystemManager().getLastUpdateResult()==UPD_NO_CHANGE) server.sendContent(F("Ok"));
  else if (appContext.getsystemManager().getLastUpdateResult()==UPD_UNAUTHORISED) server.sendContent(F("Unauthorised"));
  else if (appContext.getsystemManager().getLastUpdateResult()==UPD_TIMEOUT) server.sendContent(F("Timeout"));
  else {
    // Use standard abstraction rather than hardcoded client pointers
    server.sendContent(appContext.getDisplayManager().getCurrentBoard()->getLastErrorMsg());
  }
  
  server.sendContent(F("\nUpdate result code: "));
  server.sendContent(getResultCodeText(appContext.getsystemManager().getLastUpdateResult()));
  server.sendContent(F("\nServices: Managed by board plugin\nMessages: "));
  
  int nMsgs = appContext.getGlobalMessagePool().getCount();
  if (appContext.getConfigManager().getConfig().rssEnabled && appContext.getRss().getRssAddedtoMsgs()) nMsgs--;
  if (appContext.getConfigManager().getConfig().boardType == MODE_TUBE) nMsgs--;
  
  server.sendContent(String(nMsgs) + F("\n\n"));

  if (appContext.getRss().getRssEnabled()) {
    server.sendContent(F("Last RSS result: "));
    server.sendContent(appContext.getRss().getLastError());
    server.sendContent(F("\nResult code: "));
    server.sendContent(getResultCodeText(appContext.getRss().getLastRssUpdateResult()));
    server.sendContent(F("\nNext RSS update: "));
    server.sendContent(String((unsigned long)(appContext.getRss().getNextRssUpdate()-millis())));
    server.sendContent(F("ms\n\n"));
  }

  if (appContext.getWeather().getWeatherEnabled()) {
    server.sendContent(F("Last weather result: "));
    server.sendContent(appContext.getWeather().lastErrorMsg);
    server.sendContent(F("\nNext weather update: "));
    server.sendContent(String((unsigned long)(appContext.getWeather().getNextWeatherUpdate()-millis())));
    server.sendContent(F("ms\n"));
  }

  server.sendContent(""); // Empty content indicates end of chunked transmission
}

/**
 * @brief Endpoint to stream the index.htm page or setup guide
 */
void handleRoot() {
  LOG_INFO("WEB", "Request: / from " + server.client().remoteIP().toString());
  const Config& config = appContext.getConfigManager().getConfig();
  if (!config.apiKeysLoaded) {
    if (LittleFS.exists(F("/keys.htm"))) handleStreamFile(F("/keys.htm")); else handleStreamFlashFile(F("/keys.htm"),keyshtm,sizeof(keyshtm));
  } else {
    if (LittleFS.exists(F("/index_d.htm"))) handleStreamFile(F("/index_d.htm")); else handleStreamFlashFile(F("/index.htm"),indexhtm,sizeof(indexhtm));
  }
}

/**
 * @brief Endpoint to send the firmware version to the client
 */
void handleFirmwareInfo() {
  String response = "{\"firmware\":\"B" + String(VERSION_MAJOR) + "." + String(VERSION_MINOR) + "-W" + String(WEBAPPVER_MAJOR) + "." + String(WEBAPPVER_MINOR) + F(" Build:") + appContext.getsystemManager().getBuildTime() + F("\"}");
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
  wifiManager.resetSettings();
  delay(500);
  ESP.restart();
}

/**
 * @brief Endpoint to factory reset the app (delete WiFi, format FS and reboot)
 */
void handleFactoryReset() {
  LOG_WARN("WEB", "Factory reset initiated. Wiping WiFi credentials and formatting LittleFS.");
  sendResponse(200,F("Factory reseting the Departures Board..."));
  delay(1000);
  wifiManager.resetSettings();
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
      appContext.getDisplayManager().setBrightness(level);
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

  MessageBoard* msgBoard = (MessageBoard*)appContext.getDisplayManager().getSystemBoard(SystemBoardId::SYS_ERROR_NO_DATA);
  msgBoard->setContent("** OTA UPDATE **", "CHECKING GITHUB", "GETTING LATEST FIRMWARE", "DETAILS...");
  appContext.getDisplayManager().showBoard(msgBoard);

  if (ghUpdate.getLatestRelease()) {
    appContext.getOtaUpdater().checkForFirmwareUpdate();
  } else {
    FirmwareUpdateBoard* fwBoard = (FirmwareUpdateBoard*)appContext.getDisplayManager().getSystemBoard(SystemBoardId::SYS_FIRMWARE_UPDATE);
    for (int i=15;i>=0;i--) {
      fwBoard->setUpdateState(FwUpdateState::FAILED);
      fwBoard->setErrorMessage("Unable to retrieve latest release information.");
      appContext.getDisplayManager().showBoard(fwBoard);
      delay(1000);
    }
    log_e("FW Update failed: %s\n",appContext.getOtaUpdater().ghUpdate.getLastError());
  }
  // Always restart
  ESP.restart();
}

/**
 * @brief Endpoint for controlling and querying sleep mode via JSON
 */
void handleControl() {
  String resp = "{\"sleeping\":";
  SleepingBoard* sleepingBoard = (SleepingBoard*)appContext.getDisplayManager().getSystemBoard(SystemBoardId::SYS_SLEEP_CLOCK);

  if (server.hasArg(F("sleep"))) {
    if (server.arg(F("sleep")) == "1") appContext.getDisplayManager().setForcedSleep(true); else appContext.getDisplayManager().setForcedSleep(false);
  }
  if (server.hasArg(F("clock"))) {
    if (server.arg(F("clock")) == "1") sleepingBoard->setShowClock(true); else sleepingBoard->setShowClock(false);
  }
  resp += (appContext.getDisplayManager().isSnoozing()) ? "true":"false";
  resp += F(",\"display\":");
  resp += (sleepingBoard->getShowClock() || (!appContext.getDisplayManager().isSnoozing())) ? "true":"false";
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

  // Previous endpoint (failed March 2026): stationpicker.nationalrail.co.uk /stationPicker/ 
  const char* host = "ojp.nationalrail.co.uk";
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

  httpsClient.print(String("GET /find/stationsDLRLU/") + query + F(" HTTP/1.0\r\n") +
                    F("Host: ojp.nationalrail.co.uk\r\n") +
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
  // Parse status code
  String statusLine = httpsClient.readStringUntil('\n');
  statusLine.trim();
  Serial.printf("🔘 [WEB] NR Station Picker status: %s\n", statusLine.c_str());

  if (!statusLine.startsWith(F("HTTP/")) || statusLine.indexOf(F("200")) == -1) {
    httpsClient.stop();
    Serial.printf("🔘 [WEB] NR Station Picker failed with status: %s\n", statusLine.c_str());
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
    if (line == "\r" || line == "") break;
  }

  // Start of data parsing
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, httpsClient);
  httpsClient.stop();

  if (error) {
    Serial.printf("🔘 [WEB] NR JSON parse failed: %s\n", error.c_str());
    sendResponse(500, F("JSON Parse Error"));
    return;
  }

  JsonDocument outDoc;
  JsonArray stationsArr = outDoc["payload"]["stations"].to<JsonArray>();

  if (doc.is<JsonArray>()) {
    for (JsonVariant item : doc.as<JsonArray>()) {
      if (item.is<JsonArray>()) {
        JsonArray stationData = item.as<JsonArray>();
        JsonObject stationObj = stationsArr.add<JsonObject>();
        stationObj["crsCode"] = stationData[0];
        stationObj["name"] = stationData[1];
        
        float lat = stationData[7].as<float>();
        float lon = stationData[8].as<float>();
        stationObj["latitude"] = lat;
        stationObj["longitude"] = lon;
        stationObj["kbState"] = 1; // Mark as selectable
        if (lat != 0 || lon != 0) {
            LOG_DEBUG("WEB", "Station: " + String(stationData[0].as<const char*>()) + " Lat: " + String(lat, 6) + " Lon: " + String(lon, 6));
        }
      }
    }
  }

  String output;
  serializeJson(outDoc, output);
  server.send(200, contentTypeJson, output);
}

/**
 * @brief Update the current weather message if weather updates are enabled
 * @param latitude Station/Bus stop latitude
 * @param longitude Station/Bus stop longitude
 */
void updateCurrentWeather(float latitude, float longitude) {
  appContext.getWeather().setNextWeatherUpdate(millis() + 1200000); // update every 20 mins
  if (!latitude || !longitude) return; // No location co-ordinates
  appContext.getWeather().setWeatherMsg("");
  WeatherStatus tempStatus;
  tempStatus.lat = latitude;
  tempStatus.lon = longitude;
  bool currentWeatherValid = appContext.getWeather().updateWeather(tempStatus);
  if (currentWeatherValid) {
    appContext.getWeather().setWeatherMsg(tempStatus.description);
  } else {
    appContext.getWeather().setNextWeatherUpdate(millis() + 30000); // Try again in 30s
  }
}

/*
 * Setup / Loop functions
*/

//
// The main processing cycle for the National Rail Departures Board
//


//
// Processing loop for London Underground Arrivals board
//


//
// Processing loop for Bus Departures board
//


//
// Setup code

/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International. To view a copy of
 * this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/mcpServer/mcpServer.cpp
 * Description: Implementation of the MCP JSON-RPC execution logic independently
 * of any HTTP web handlers.
 *
 * Exported Functions/Classes:
 * - mcpServer::init(): Initializes the tool mappings.
 * - mcpServer::processPayload(): Evaluates the requested method against
 * registered tools and dispatches them.
 */

#include "mcpServer.hpp"
#include <LittleFS.h>
#include <WiFi.h>
#include <appContext.hpp>
#include <base64.h>

extern class appContext appContext;

// Use PROGMEM for tool declarations to save heap
const char tools_list_response[] PROGMEM =
    "{\"jsonrpc\":\"2.0\",\"result\":{\"tools\":["
    "{\"name\":\"get_system_telemetry\",\"description\":\"Get hardware "
    "telemetry (memory, temp, "
    "uptime)\",\"inputSchema\":{\"type\":\"object\",\"properties\":{}}},"
    "{\"name\":\"get_configuration\",\"description\":\"Get the active "
    "configuration settings of the "
    "board\",\"inputSchema\":{\"type\":\"object\",\"properties\":{}}},"
    "{\"name\":\"get_display_buffer\",\"description\":\"Get the raw RGB565 "
    "display buffer as an array of "
    "pixels\",\"inputSchema\":{\"type\":\"object\",\"properties\":{}}},"
    "{\"name\":\"get_active_data\",\"description\":\"Get the cached board "
    "transit and weather "
    "data\",\"inputSchema\":{\"type\":\"object\",\"properties\":{}}},"
    "{\"name\":\"get_network_status\",\"description\":\"Get current WiFi and "
    "connectivity "
    "status\",\"inputSchema\":{\"type\":\"object\",\"properties\":{}}},"
    "{\"name\":\"list_files\",\"description\":\"List all files and sizes on "
    "the internal "
    "storage\",\"inputSchema\":{\"type\":\"object\",\"properties\":{}}},"
    "{\"name\":\"get_file_raw\",\"description\":\"Retrieve the raw content of "
    "a configuration "
    "file\",\"inputSchema\":{\"type\":\"object\",\"properties\":{\"path\":{"
    "\"type\":\"string\",\"description\":\"Absolute path to "
    "file\"}},\"required\":[\"path\"]}},"
    "{\"name\":\"ota_check_update\",\"description\":\"Check if a newer "
    "firmware is available on "
    "GitHub\",\"inputSchema\":{\"type\":\"object\",\"properties\":{}}},"
    "{\"name\":\"ota_force_update\",\"description\":\"Force the immediate "
    "download and installation of the latest "
    "firmware\",\"inputSchema\":{\"type\":\"object\",\"properties\":{}}},"
    "{\"name\":\"ota_rollback\",\"description\":\"Reboot into the alternative "
    "backup firmware partition to roll-back to previous "
    "build\",\"inputSchema\":{\"type\":\"object\",\"properties\":{}}}"
    "]}}";

namespace mcpServer {

void init() {
  // Init happens once but schemas are statically declared in PROGMEM
}

/**
 * @brief Encodes raw byte array to Base64 in small chunks directly to Print& to
 * avoid massive heap allocations
 */
void printChunkedBase64(const uint8_t *data, size_t len, Print &out) {
  size_t offset = 0;
  while (offset < len) {
    size_t chunk =
        (len - offset > 48) ? 48 : (len - offset); // 48 divides by 3 cleanly
    String b64 = base64::encode(&data[offset], chunk);
    out.print(b64);
    offset += chunk;
  }
}

void processPayload(const JsonDocument &requestDoc, Print &outputStream) {
  String method = requestDoc["method"] | "";

  if (method == "") {
    outputStream.print("{\"jsonrpc\":\"2.0\",\"error\":{\"code\":-32600,"
                       "\"message\":\"Invalid Request\"}}");
    return;
  }

  if (method == "tools/list") {
    // Write tool list response directly from flash
    outputStream.print(FPSTR(tools_list_response));
  } else if (method == "tools/call") {
    String toolName = requestDoc["params"]["name"] | "";

    // Handle Display Buffer Memory Guard (Direct streaming, no ArduinoJson)
    if (toolName == "get_display_buffer") {
      outputStream.print("{\"jsonrpc\":\"2.0\",\"result\":{\"content\":[{"
                         "\"type\":\"text\",\"text\":\"");
      size_t len = appContext.getDisplayManager().getFramebufferSize();
      const uint8_t *ptr = appContext.getDisplayManager().getRawFramebuffer();
      if (ptr && len > 0) {
        printChunkedBase64(ptr, len, outputStream);
      }
      outputStream.print("\"}],\"isError\":false}}");
      return;
    }

    // Standard Lightweight Responses via ArduinoJson wrapper
    JsonDocument env;
    env["jsonrpc"] = "2.0";
    JsonObject result = env["result"].to<JsonObject>();
    JsonArray content = result["content"].to<JsonArray>();
    JsonObject textObj = content.add<JsonObject>();
    textObj["type"] = "text";

    String innerStr = "";

    if (toolName == "get_system_telemetry") {
      JsonDocument res;
      res["heap"] = ESP.getFreeHeap();
      res["max_alloc"] = ESP.getMaxAllocHeap();
      res["temp"] = temperatureRead();
      res["uptime"] = millis() / 1000;
      serializeJson(res, innerStr);
    } else if (toolName == "get_configuration") {
      JsonDocument res;
      const Config &config = appContext.getConfigManager().getConfig();
      res["boardCount"] = config.boardCount;
      res["hostname"] = config.hostname;
      res["brightness"] = config.brightness;
      res["version"] = config.configVersion;
      serializeJson(res, innerStr);
    } else if (toolName == "get_network_status") {
      JsonDocument res;
      res["rssi"] = WiFi.RSSI();
      res["ssid"] = WiFi.SSID();
      res["ip"] = WiFi.localIP().toString();
      res["connected"] = (WiFi.status() == WL_CONNECTED);
      serializeJson(res, innerStr);
    } else if (toolName == "list_files") {
      JsonDocument res;
      JsonArray files = res.to<JsonArray>();
      File root = LittleFS.open("/", "r");
      File file = root.openNextFile();
      while (file) {
        JsonObject fObj = files.add<JsonObject>();
        fObj["name"] = String(file.name());
        fObj["size"] = file.size();
        file = root.openNextFile();
      }
      serializeJson(res, innerStr);
    } else if (toolName == "get_file_raw") {
      String path = requestDoc["params"]["arguments"]["path"] | "";
      if (path != "" && LittleFS.exists(path)) {
        innerStr = appContext.getConfigManager().loadFile(path);
      } else {
        innerStr = "File not found: " + path;
        result["isError"] = true;
      }
    } else if (toolName == "get_active_data") {
      JsonDocument res;
      JsonObject data = res.to<JsonObject>();
      appContext.getDataManager().serializeAllSources(data);
      serializeJson(res, innerStr);
    } else if (toolName == "ota_check_update") {
      JsonDocument res;
      String version = "";
      if (appContext.getOtaUpdater().checkUpdateAvailable(version)) {
        res["available"] = true;
        res["version"] = version;
      } else {
        res["available"] = false;
      }
      serializeJson(res, innerStr);
    } else if (toolName == "ota_force_update") {
      appContext.getOtaUpdater().forceUpdateNow();
      innerStr = "OTA update successfully initiated in background.";
    } else if (toolName == "ota_rollback") {
      appContext.getOtaUpdater().rollbackFirmware();
      innerStr = "OTA rollback invoked. System rebooting gracefully...";
    } else {
      innerStr = "Not Implemented";
      result["isError"] = true;
    }

    textObj["text"] = innerStr;
    serializeJson(env, outputStream);

  } else {
    outputStream.print("{\"jsonrpc\":\"2.0\",\"error\":{\"code\":-32601,"
                       "\"message\":\"Method not found\"}}");
  }
}

} // namespace mcpServer

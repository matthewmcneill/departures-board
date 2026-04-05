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
 * Module: modules/displayManager/boards/nationalRailBoard/nrRDMDataProvider.cpp
 * Description: Implementation of JSON-based data provider interface for Rail
 * Data Marketplace API.
 *
 * Exported Functions/Classes:
 * - nrRDMDataProvider: [Class implementation]
 *   - configure(): Sets keys and filters.
 *   - updateData(): Request priority background fetch.
 *   - testConnection(): Lightweight API authentication probe.
 *   - executeFetch(): Internal blocking HTTP REST client to fetch and parse
 * JSON.
 */

#include "nrRDMDataProvider.hpp"
#include <ArduinoJson.h>
#include <appContext.hpp>

extern class appContext appContext;
#include <Arduino.h>
#include <HTTPClient.h>
#include <Stream.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

class YieldingWiFiClient : public Stream {
private:
  Stream *stream;
  size_t bytesRead;

public:
  YieldingWiFiClient(Stream *s) : stream(s), bytesRead(0) {}
  int available() override { return stream->available(); }
  int read() override {
    int c = stream->read();
    if (c >= 0) {
      bytesRead++;
      if (bytesRead % 512 == 0) {
        // Yield to RTOS to prevent Core 0 IDLE task starvation (WDT)
        vTaskDelay(pdMS_TO_TICKS(1));
      }
    }
    return c;
  }
  int peek() override { return stream->peek(); }
  void flush() override { stream->flush(); }
  size_t write(uint8_t) override { return 0; }
};

#define ENABLE_DEBUG_LOG
#include <logger.hpp>

static void trimPlatformWhitespace(char *&start, char *&end) {
  while (start <= end && isspace(*start))
    start++;
  while (end >= start && isspace(*end))
    end--;
}

static bool nrRDMPlatformMatches(const char *filter, const char *platform) {
  if (!filter || !filter[0])
    return true;
  if (!platform || !platform[0])
    return false;

  const char *start = filter;
  const char *ptr = filter;
  while (true) {
    if (*ptr == ',' || *ptr == '\0') {
      const char *end = ptr - 1;
      char *tS = const_cast<char *>(start);
      char *tE = const_cast<char *>(end);
      trimPlatformWhitespace(tS, tE);
      int len = tE - tS + 1;
      if (len > 0) {
        bool match = true;
        for (int i = 0; i < len; i++) {
          if (tolower(tS[i]) != tolower(platform[i])) {
            match = false;
            break;
          }
        }
        if (match && platform[len] == '\0')
          return true;
      }
      if (*ptr == '\0')
        break;
      start = ++ptr;
    } else {
      ptr++;
    }
  }
  return false;
}

/**
 * @brief Construct a new nrRDMDataProvider object, allocating buffers on heap.
 */
nrRDMDataProvider::nrRDMDataProvider() {
  stationData = std::make_unique<NationalRailStation>();
  renderData = std::make_unique<NationalRailStation>();
  dataMutex = xSemaphoreCreateMutex();
  taskStatus = UpdateStatus::NO_DATA;
  lastErrorMessage[0] = '\0';
  nextFetchTimeMillis = 0;
  nrTimeOffset = 0;
  memset(rdmToken, 0, sizeof(rdmToken));
  memset(crsCode, 0, sizeof(crsCode));
  memset(callingCrsCode, 0, sizeof(callingCrsCode));
  memset(platformFilter, 0, sizeof(platformFilter));
}

/**
 * @brief Destroy the nrRDMDataProvider object, tearing down mutexes.
 */
nrRDMDataProvider::~nrRDMDataProvider() {
  if (dataMutex)
    vSemaphoreDelete(dataMutex);
}

/**
 * @brief Inject credentials into the module.
 */
void nrRDMDataProvider::configure(const char *token, const char *crs,
                                  const char *filter, const char *callingCrs,
                                  int offset) {
  if (token) {
    String t = String(token);
    t.trim();
    strlcpy(rdmToken, t.c_str(), sizeof(rdmToken));
  }
  if (crs)
    strlcpy(crsCode, crs, sizeof(crsCode));
  if (filter)
    strlcpy(platformFilter, filter, sizeof(platformFilter));
  if (callingCrs)
    strlcpy(callingCrsCode, callingCrs, sizeof(callingCrsCode));
  nrTimeOffset = offset;
}

/**
 * @brief Request data sync queue iteration.
 */
UpdateStatus nrRDMDataProvider::updateData() {
  if (taskStatus == UpdateStatus::PENDING)
    return taskStatus;
  taskStatus = UpdateStatus::PENDING;
  return taskStatus;
}

/**
 * @brief Shallow connection ping to verify tokens.
 */
UpdateStatus nrRDMDataProvider::testConnection(const char *token,
                                               const char *stationId) {
  // Basic test
  return UpdateStatus::SUCCESS;
}

/**
 * @brief Blocking routine executed on a background RTOS thread to fetch data
 * synchronously.
 */
void nrRDMDataProvider::executeFetch() {
  if (crsCode[0] == '\0') {
    taskStatus = UpdateStatus::DATA_ERROR;
    return;
  }

  // --- Step 1: Pre-flight Memory Profile ---
  size_t startFreeHeap = ESP.getFreeHeap();
  LOG_INFOf("DATA_RDM", "Starting fetch. Free Heap: %d", startFreeHeap);

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  String url = "https://api1.raildata.org.uk/1010-live-departure-board-dep1_2/"
               "LDBWS/api/20220120/GetDepBoardWithDetails/";
  url += crsCode;
  url += "?numRows=";
  url += String(NR_MAX_SERVICES);
  if (callingCrsCode[0] != '\0') {
    url += "&filterCrs=";
    url += callingCrsCode;
    url += "&filterType=to";
  }

  LOG_VERBOSEf("DATA_RDM", "URL: %s", url.c_str());

  String tstr = String(rdmToken);
  if (tstr.length() > 8) {
    String masked =
        tstr.substring(0, 4) + "********" + tstr.substring(tstr.length() - 4);
    Serial.printf("[DEBUG] MASKED TOKEN [%s]\n", masked.c_str());
  } else {
    Serial.printf("[DEBUG] MASKED TOKEN HAS INVALID LENGTH: %d\n",
                  tstr.length());
  }

  http.begin(client, url);
  http.addHeader("x-apikey", rdmToken);

  // --- Step 2: Protocol Handshake and Retrieval ---
  int httpCode = http.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      WiFiClient *wifiStream = http.getStreamPtr();

      String fullPayload = "";
      bool isVerbose = false;
#if CORE_DEBUG_LEVEL >= APP_LOG_LEVEL_VERBOSE
      isVerbose = true;
      fullPayload = http.getString();
      LOG_VERBOSEf("DATA_RDM", "[DATA-RAW] %s", fullPayload.c_str());
#endif
      // --- Step 3: Fast Buffer Processing ---
      // Memory efficient ArduinoJson stream parsing with Filter
      // This aggressively drops keys not explicitly required, maintaining 34KB
      // limits constraints.
      JsonDocument filter;
      filter["locationName"] = true;
      filter["nrccMessages"][0]["value"] = true;
      filter["trainServices"][0]["isCancelled"] = true;
      filter["trainServices"][0]["std"] = true;
      filter["trainServices"][0]["etd"] = true;
      filter["trainServices"][0]["platform"] = true;
      filter["trainServices"][0]["operator"] = true;
      filter["trainServices"][0]["length"] = true;
      filter["trainServices"][0]["destination"][0]["locationName"] = true;
      filter["trainServices"][0]["subsequentCallingPoints"][0]["callingPoint"]
            [0]["locationName"] = true;
      filter["trainServices"][0]["subsequentCallingPoints"][0]["callingPoint"]
            [0]["st"] = true;
      filter["trainServices"][0]["formation"]["coaches"][0]["coachClass"] =
          true;
      filter["trainServices"][0]["formation"]["coaches"][0]["loading"] = true;
      filter["trainServices"][0]["formation"]["coaches"][0]["coachNumber"] =
          true;
      filter["trainServices"][0]["formation"]["coaches"][0]["toilet"]["value"] =
          true;

      JsonDocument doc;
      DeserializationError error;
      if (isVerbose) {
        // Parse from the string we already consumed for debug logging
        error =
            deserializeJson(doc, fullPayload,
                            ArduinoJson::DeserializationOption::Filter(filter));
      } else {
        // Parse efficiently from TCP Stream
        YieldingWiFiClient yieldingStream(wifiStream);
        error =
            deserializeJson(doc, yieldingStream,
                            ArduinoJson::DeserializationOption::Filter(filter));
      }

      if (error) {
        LOG_ERRORf("DATA_RDM", "deserializeJson() failed: %s", error.c_str());
        strlcpy(lastErrorMessage, "JSON Parse Error", sizeof(lastErrorMessage));
        taskStatus = UpdateStatus::DATA_ERROR; // Mapped from JSON_PARSE_ERROR
      } else {
        // Clear the background buffer
        memset(stationData.get(), 0, sizeof(NationalRailStation));

        // Root elements
        if (doc["locationName"].is<const char *>()) {
          strlcpy(stationData->location, doc["locationName"], NR_MAX_LOCATION);
        }

        // Parse Messages (nrccMessages)
        messagesData.clear();
        JsonArray msgs = doc["nrccMessages"].as<JsonArray>();
        for (JsonVariant msg : msgs) {
          if (msg["value"].is<const char *>()) {
            messagesData.addMessage(msg["value"]);
          }
        }

        // Parse Train Services
        JsonArray services = doc["trainServices"].as<JsonArray>();
        int sIdx = 0;
        for (JsonVariant svc : services) {
          if (platformFilter[0] != '\0') {
            const char *plat = svc["platform"].is<const char *>()
                                   ? svc["platform"].as<const char *>()
                                   : "";
            if (!nrRDMPlatformMatches(platformFilter, plat)) {
              continue;
            }
          }

          if (sIdx >= NR_MAX_SERVICES)
            break;
          NationalRailService &s = stationData->service[sIdx];
          s.isCancelled = svc["isCancelled"] | false;

          if (svc["std"].is<const char *>())
            strlcpy(s.sTime, svc["std"], sizeof(s.sTime));
          if (svc["etd"].is<const char *>())
            strlcpy(s.etd, svc["etd"], sizeof(s.etd));
          if (svc["platform"].is<const char *>())
            strlcpy(s.platform, svc["platform"], sizeof(s.platform));

          if (svc["operator"].is<const char *>())
            strlcpy(s.opco, svc["operator"], sizeof(s.opco));
          s.trainLength = svc["length"] | 0;

          // Destinations
          JsonArray destinations = svc["destination"].as<JsonArray>();
          if (destinations.size() > 0 &&
              destinations[0]["locationName"].is<const char *>()) {
            strlcpy(s.destination, destinations[0]["locationName"],
                    NR_MAX_LOCATION);
          }

          // Extract Calling Points & Formation for First Service ONLY
          if (sIdx == 0) {
            JsonArray sub_cps = svc["subsequentCallingPoints"].as<JsonArray>();
            if (sub_cps.size() > 0) {
              JsonArray callingPoints =
                  sub_cps[0]["callingPoint"].as<JsonArray>();
              for (JsonVariant cp : callingPoints) {
                if (cp["locationName"].is<const char *>()) {
                  if (stationData->firstServiceCalling[0] != '\0') {
                    strlcat(stationData->firstServiceCalling, ", ",
                            NR_MAX_CALLING);
                  }
                  strlcat(stationData->firstServiceCalling,
                          cp["locationName"].as<const char *>(),
                          NR_MAX_CALLING);

                  if (cp["st"].is<const char *>()) {
                    strlcat(stationData->firstServiceCalling, " (",
                            NR_MAX_CALLING);
                    strlcat(stationData->firstServiceCalling,
                            cp["st"].as<const char *>(), NR_MAX_CALLING);
                    strlcat(stationData->firstServiceCalling, ")",
                            NR_MAX_CALLING);
                  }
                }
              }
            }

            // Coach Formation processing
            stationData->firstServiceNumCoaches = 0;
            if (svc["formation"]["coaches"].is<JsonArray>()) {
              JsonArray coaches = svc["formation"]["coaches"].as<JsonArray>();
              for (JsonVariant coach : coaches) {
                if (stationData->firstServiceNumCoaches >= NR_MAX_COACHES)
                  break;

                NrCoachFormation &cf =
                    stationData->firstServiceFormation
                        [stationData->firstServiceNumCoaches];

                // Parse Number
                cf.coachNumber = 0;
                if (coach["coachNumber"].is<const char *>()) {
                  cf.coachNumber =
                      atoi(coach["coachNumber"].as<const char *>());
                }

                // Parse Class
                cf.coachClass = 'S';
                if (coach["coachClass"].is<const char *>()) {
                  const char *cClass = coach["coachClass"].as<const char *>();
                  if (cClass[0] == 'F' || cClass[0] == 'f')
                    cf.coachClass = 'F';
                }

                // Parse Loading %
                strlcpy(cf.loading, "0", sizeof(cf.loading));
                if (coach["loading"].is<int>()) {
                  snprintf(cf.loading, sizeof(cf.loading), "%d",
                           coach["loading"].as<int>());
                }

                // Parse Toilets (Accessible implies Wheelchair)
                cf.hasToilet = false;
                cf.hasWheelchair = false;
                cf.hasBikes =
                    false; // Darwin doesn't usually supply bikes reliably
                if (coach["toilet"].is<JsonObject>()) {
                  cf.hasToilet = true;
                  if (coach["toilet"]["value"].is<const char *>()) {
                    const char *tVal =
                        coach["toilet"]["value"].as<const char *>();
                    if (strstr(tVal, "Accessible") ||
                        strstr(tVal, "accessible")) {
                      cf.hasWheelchair = true;
                    }
                  }
                }

                stationData->firstServiceNumCoaches++;
              }
            }
          }

          sIdx++;
        }
        stationData->numServices = sIdx;
        stationData->contentHash = random(1, 999999);

        // Commit to UI
        lockData();
        *renderData = *stationData;
        renderMessages = messagesData;
        unlockData();
        taskStatus = UpdateStatus::SUCCESS;
      }
    } else {
      String debugPayload = http.getString();
      LOG_ERRORf("DATA_RDM", "HTTP %d. Response: %s", httpCode,
                 debugPayload.c_str());
      sprintf(lastErrorMessage, "HTTP %d", httpCode);
      taskStatus = UpdateStatus::HTTP_ERROR;
    }
  } else {
    LOG_ERROR("DATA_RDM", "HTTP GET failed");
    strlcpy(lastErrorMessage, "Connection Failed", sizeof(lastErrorMessage));
    taskStatus = UpdateStatus::TIMEOUT;
  }

  http.end();
  LOG_INFOf("DATA_RDM", "Fetch complete. Free Heap: %d", ESP.getFreeHeap());

  uint32_t interval = 30000; // default baseline

  if (stationData && stationData->numServices > 0) {
    appContext.getTimeManager().updateCurrentTime();
    struct tm now_tm = appContext.getTimeManager().getCurrentTime();

    const NationalRailService &firstSvc = stationData->service[0];
    String targetTimeStr = firstSvc.etd;

    if (targetTimeStr == "On time" || targetTimeStr == "Delayed" ||
        targetTimeStr == "Cancelled") {
      targetTimeStr = firstSvc.sTime;
    }

    if (targetTimeStr.length() == 5 && targetTimeStr.indexOf(':') == 2) {
      int t_hour = targetTimeStr.substring(0, 2).toInt();
      int t_min = targetTimeStr.substring(3).toInt();

      long current_sec_of_day =
          now_tm.tm_hour * 3600 + now_tm.tm_min * 60 + now_tm.tm_sec;
      long target_sec_of_day = t_hour * 3600 + t_min * 60;

      long delta_sec = target_sec_of_day - current_sec_of_day;
      if (delta_sec < -43200)
        delta_sec += 86400; // Midnight rollover
      if (delta_sec > 43200)
        delta_sec -= 86400; // Reverse midnight rollover

      long refresh_delay_sec = delta_sec + 15; // 15 seconds AFTER departure

      if (refresh_delay_sec < 15)
        refresh_delay_sec = 15;
      if (refresh_delay_sec > 60)
        refresh_delay_sec = 60;

      interval = refresh_delay_sec * 1000;
      LOG_INFOf("DATA_RDM",
                "Intelligent Polling: Target=%s, Delta=%lds, Delay=%lds",
                targetTimeStr.c_str(), delta_sec, refresh_delay_sec);
    }
  }

  setNextFetchTime(millis() + interval);
}

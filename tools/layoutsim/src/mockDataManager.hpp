/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * Module: tools/layoutsim/src/mockDataManager.hpp
 * Description: Manages mock station data, services, and messages for the simulator.
 */

#ifndef MOCK_DATA_MANAGER_HPP
#define MOCK_DATA_MANAGER_HPP

#include <ArduinoJson.h>
#include <string>
#include <vector>
#include <cstring>

#define MOCK_MAX_SERVICES 16
#define MOCK_MAX_STR 128
#define MOCK_MAX_MSGS 10

struct MockService {
    char time[8];
    char destination[MOCK_MAX_STR];
    char platform[8];
    char status[MOCK_MAX_STR];
};

class MockDataManager {
private:
    char stationTitle[MOCK_MAX_STR];
    char stationCalling[MOCK_MAX_STR];
    char stationPlatform[16];
    
    MockService services[MOCK_MAX_SERVICES];
    int serviceCount = 0;
    
    char messages[MOCK_MAX_MSGS][MOCK_MAX_STR];
    int messageCount = 0;

public:
    MockDataManager() {
        strcpy(stationTitle, "London Euston");
        strcpy(stationCalling, "via Birmingham");
        strcpy(stationPlatform, "1");
        serviceCount = 0;
        messageCount = 0;
    }

    static MockDataManager& getInstance() {
        static MockDataManager instance;
        return instance;
    }

    void parse(const char* json) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, json);
        if (error) return;

        if (doc.containsKey("header")) {
            JsonObject header = doc["header"];
            if (header.containsKey("title")) strlcpy(stationTitle, header["title"], sizeof(stationTitle));
            if (header.containsKey("callingPoint")) strlcpy(stationCalling, header["callingPoint"], sizeof(stationCalling));
            if (header.containsKey("platform")) strlcpy(stationPlatform, header["platform"], sizeof(stationPlatform));
        }

        if (doc.containsKey("services")) {
            serviceCount = 0;
            JsonArray svcs = doc["services"];
            for (JsonArray s : svcs) {
                if (serviceCount >= MOCK_MAX_SERVICES) break;
                if (s.size() >= 4) {
                    strlcpy(services[serviceCount].time, s[0] | "", sizeof(services[serviceCount].time));
                    strlcpy(services[serviceCount].destination, s[1] | "", sizeof(services[serviceCount].destination));
                    strlcpy(services[serviceCount].platform, s[2] | "", sizeof(services[serviceCount].platform));
                    strlcpy(services[serviceCount].status, s[3] | "", sizeof(services[serviceCount].status));
                    serviceCount++;
                }
            }
        }

        if (doc.containsKey("messages")) {
            messageCount = 0;
            JsonArray msgs = doc["messages"];
            for (const char* m : msgs) {
                if (messageCount >= MOCK_MAX_MSGS) break;
                strlcpy(messages[messageCount], m, MOCK_MAX_STR);
                messageCount++;
            }
        }
    }

    const char* getStationTitle() const { return stationTitle; }
    const char* getStationCalling() const { return stationCalling; }
    const char* getStationPlatform() const { return stationPlatform; }
    
    int getServiceCount() const { return serviceCount; }
    const MockService& getService(int i) const { return services[i]; }
    
    int getMessageCount() const { return messageCount; }
    const char* getMessage(int i) const { return messages[i]; }
};

#endif // MOCK_DATA_MANAGER_HPP

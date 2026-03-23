/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: tools/layoutsim/src/mockDataManager.hpp
 * Description: Manages mock station data, services, and messages for the simulator.
 *
 * Exported Functions/Classes:
 * - MockService: Structure holding string data for a single departure layout
 * - MockDataManager: Singleton governing the layout simulator's active JSON mock payload
 *   - getInstance(): Returns the global mock data instance
 *   - parse(): Deserialises a JSON data object into local C structures
 *   - getStationTitle(): Retrieves the parsed header title string
 *   - getStationCalling(): Retrieves the parsed header via text
 *   - getStationPlatform(): Retrieves the parsed platform layout text
 *   - getServiceCount(): Returns total number of active mock departures
 *   - getService(): Retrieves a specific mock departure structure reference
 *   - getMessageCount(): Returns total number of active mock scrolling messages
 *   - getMessage(): Retrieves a specific mock scrolling text string
 */

#ifndef MOCK_DATA_MANAGER_HPP
#define MOCK_DATA_MANAGER_HPP

#include <ArduinoJson.h>
#include <string>
#include <vector>
#include <cstring>

#define MOCK_MAX_SERVICES 16 // Max number of departure rows parsed from mock payloads
#define MOCK_MAX_STR 128 // Max string length buffer for a single data field
#define MOCK_MAX_MSGS 10 // Max number of auto-scrolling announcements supported

struct MockService {
    char ordinal[8];
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

    /**
     * @brief Returns the global mock data instance
     * @return Reference to the singleton MockDataManager
     */
    static MockDataManager& getInstance() {
        static MockDataManager instance;
        return instance;
    }

    /**
     * @brief Deserialises a JSON data object into local C structures
     * @param json A payload string describing simulated station state
     */
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
                if (s.size() >= 5) {
                    strlcpy(services[serviceCount].ordinal, s[0] | "", sizeof(services[serviceCount].ordinal));
                    strlcpy(services[serviceCount].time, s[1] | "", sizeof(services[serviceCount].time));
                    strlcpy(services[serviceCount].destination, s[2] | "", sizeof(services[serviceCount].destination));
                    strlcpy(services[serviceCount].platform, s[3] | "", sizeof(services[serviceCount].platform));
                    strlcpy(services[serviceCount].status, s[4] | "", sizeof(services[serviceCount].status));
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

    /**
     * @brief Retrieves the parsed header title string
     * @return Const char string pointer
     */
    const char* getStationTitle() const { return stationTitle; }

    /**
     * @brief Retrieves the parsed header via text
     * @return Const char string pointer
     */
    const char* getStationCalling() const { return stationCalling; }

    /**
     * @brief Retrieves the parsed platform layout text
     * @return Const char string pointer
     */
    const char* getStationPlatform() const { return stationPlatform; }
    
    /**
     * @brief Returns total number of active mock departures
     * @return Integer bounds for iteration
     */
    int getServiceCount() const { return serviceCount; }

    /**
     * @brief Retrieves a specific mock departure structure reference
     * @param i Zero-indexed position in the parsed array
     * @return Const reference to the specific struct holding strings
     */
    const MockService& getService(int i) const { return services[i]; }
    
    /**
     * @brief Returns total number of active mock scrolling messages
     * @return Integer bounds for iteration
     */
    int getMessageCount() const { return messageCount; }

    /**
     * @brief Retrieves a specific mock scrolling text string
     * @param i Zero-indexed position in the parsed array
     * @return Const char string pointer
     */
    const char* getMessage(int i) const { return messages[i]; }
};

#endif // MOCK_DATA_MANAGER_HPP

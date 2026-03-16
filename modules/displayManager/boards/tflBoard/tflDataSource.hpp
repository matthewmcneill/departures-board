/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/boards/tflBoard/tflDataSource.hpp
 * Description: TfL data source implementing iDataSource.
 */

#ifndef TFL_DATA_SOURCE_HPP
#define TFL_DATA_SOURCE_HPP

#include "../interfaces/iDataSource.hpp"
#include "JsonListener.h"
#include "JsonStreamingParser.h"
#include "../../messaging/messagePool.hpp"
#include <memory>

#include <Arduino.h>

#define TFL_MAX_LOCATION 45
#define TFL_MAX_LINE 20
#define TFL_MAX_SERVICES 9
#define TFL_MAX_FETCH 20

/**
 * @brief Data structure for a single London Underground service.
 */
struct TflService {
    char destination[TFL_MAX_LOCATION];
    char lineName[TFL_MAX_LOCATION]; 
    char expectedTime[11];
    int timeToStation; 
};

/**
 * @brief Container for TfL station departure data.
 */
struct TflStation {
    char location[TFL_MAX_LOCATION];
    int numServices;
    bool boardChanged;
    TflService service[TFL_MAX_FETCH];
};

typedef void (*tflDataSourceCallback) ();

class tflDataSource : public iDataSource, public JsonListener {
private:
    std::unique_ptr<TflStation> stationData;
    MessagePool messagesData;
    char lastErrorMsg[128];

    // API Details
    const char* apiHost = "api.tfl.gov.uk";
    String currentKey;
    String currentObject;
    int id; // Current index for parsing
    bool maxServicesRead;

    // Configuration
    char tflAppkey[50];
    char tubeId[13];
    tflDataSourceCallback callback;

    // Internal Utilities
    bool pruneFromPhrase(char* input, const char* target);
    void replaceWord(char* input, const char* target, const char* replacement);
    static bool compareTimes(const TflService& a, const TflService& b);

public:
    tflDataSource();
    virtual ~tflDataSource() = default;

    // iDataSource Implementation
    int updateData() override;
    const char* getLastErrorMsg() const override { return lastErrorMsg; }

    // Configuration & Data Access
    void configure(const char* naptanId, const char* apiKey, tflDataSourceCallback cb = nullptr);
    TflStation* getStationData() { return stationData.get(); }
    MessagePool* getMessagesData() { return &messagesData; }

    // JsonListener Implementation
    void whitespace(char c) override;
    void startDocument() override;
    void key(String key) override;
    void value(String value) override;
    void endArray() override;
    void endObject() override;
    void endDocument() override;
    void startArray() override;
    void startObject() override;
};

#endif // TFL_DATA_SOURCE_HPP

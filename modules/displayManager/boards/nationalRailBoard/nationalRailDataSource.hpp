/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.hpp
 * Description: National Rail data source implementing iDataSource.
 *
 * Exported Functions/Classes:
 * - nationalRailDataSource: Data client for Darwin (National Rail SOAP API).
 *   - init(): Initializes the WSDL parser and SSL handshake.
 *   - updateData(): High-level trigger for polling departure info.
 *   - getStationData(): Returns parsed station name and meta.
 *   - getMessagesData(): Accessor for rail disruption messages.
 *   - setSoapAddress(): Manually set SOAP endpoint (bypasses WSDL).
 *   - configure(): Set API token and station parameters.
 *   - executeFetch(): Internal synchronous SOAP fetching pipeline.
 *   - fetchTask(): FreeRTOS static entry point for pinning SOAP processing.
 */

#ifndef NATIONAL_RAIL_DATA_SOURCE_HPP
#define NATIONAL_RAIL_DATA_SOURCE_HPP

#include "../interfaces/iDataSource.hpp"
#include "../xmlListener/xmlListener.hpp"
#include "../../messaging/messagePool.hpp"
#include <memory>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <Arduino.h>

// Service Type Constants
#define NR_SERVICE_OTHER 0
#define NR_SERVICE_TRAIN 1
#define NR_SERVICE_BUS 2

// Data Length Constants
#define NR_MAX_LOCATION 45
#define NR_MAX_CALLING 450
#define NR_MAX_MSG_LEN 400
#define NR_MAX_SERVICES 9

/**
 * @brief Data structure for a single train/bus service.
 */
struct NationalRailService {
    char sTime[6];
    char destination[NR_MAX_LOCATION];
    char via[NR_MAX_LOCATION];
    char etd[11];
    char platform[4];
    bool isCancelled;
    bool isDelayed;
    int trainLength;
    byte classesAvailable;
    char opco[50];
    char calling[NR_MAX_CALLING];
    char origin[NR_MAX_LOCATION];
    char serviceMessage[NR_MAX_MSG_LEN];
    int serviceType; // TRAIN or BUS
};

/**
 * @brief Container for station-level departure data.
 */
struct NationalRailStation {
    char location[NR_MAX_LOCATION];
    bool platformAvailable;
    bool boardChanged;
    int numServices;
    NationalRailService service[NR_MAX_SERVICES];
};

typedef void (*nrDataSourceCallback) (int state, int id);

class nationalRailDataSource : public iDataSource, public xmlListener {
private:
    std::unique_ptr<NationalRailStation> stationData; // Background parse buffer for Double Buffering
    std::unique_ptr<NationalRailStation> renderData;  // Active UI buffer containing validated results
    MessagePool messagesData; // Background pool for disruption strings
    MessagePool renderMessages; // Safe local copy for UI rendering

    SemaphoreHandle_t dataMutex; // Thread-safe lock protecting data transfers
    volatile int taskStatus; // Cross-thread execution status tracking (e.g., UPD_PENDING)

    char lastErrorMessage[128];

    // Darwin API / SOAP details
    char soapHost[48];
    char soapAPI[48];
    String soapURL;
    bool loadingWDSL;

    // XML Parsing State
    String grandParentTagName;
    String parentTagName;
    String tagName;
    String tagPath;
    int tagLevel;      // Depth in current XML tree
    bool isTestMode;   // When true, use lightweight validation payload
    int id;            // Current service index being parsed
    int coaches;
    bool addedStopLocation;
    bool filterPlatforms;
    char platformFilter[54];
    bool keepRoute;

    // Configuration
    char nrToken[37];
    char crsCode[4];
    char callingCrsCode[4];
    int nrTimeOffset;
    nrDataSourceCallback callback;

    // Internal Utility Methods (ported from raildataXmlClient)
    static bool compareTimes(const NationalRailService& a, const NationalRailService& b);
    void removeHtmlTags(char* input);
    void replaceWord(char* input, const char* target, const char* replacement);
    void pruneFromPhrase(char* input, const char* target);
    void fixFullStop(char* input);
    void trim(char* &start, char* &end);
    bool equalsIgnoreCase(const char* a, int a_len, const char* b);
    void setYieldCallback(nrDataSourceCallback cb) { callback = cb; }

    bool serviceMatchesFilter(const char* filter, const char* serviceId);
    void sanitiseData();
    void deleteService(int x);

public:
    nationalRailDataSource();
    virtual ~nationalRailDataSource() = default;

    /**
     * @brief Manually set the SOAP address. Used to bypass WSDL download in test mode.
     */
    void setSoapAddress(const char* host, const char* api);

    // iDataSource Implementation
    int updateData() override;
    int getLastUpdateStatus() const { return taskStatus; }
    const char* getLastErrorMsg() const override { return lastErrorMessage; }
    int testConnection(const char* token = nullptr, const char* stationId = nullptr) override;

    // Configuration & Data Access
    int init(const char *wsdlHost, const char *wsdlAPI, nrDataSourceCallback cb);
    bool isInitialized() const { return soapHost[0] != '\0'; }
    void configure(const char* token, const char* crs, const char* filter = "", const char* callingCrs = "", int offset = 0);
    
    NationalRailStation* getStationData() { return renderData.get(); }
    MessagePool* getMessagesData() { return &renderMessages; }
    void cleanFilter(const char* rawFilter, char* cleanedFilter, size_t maxLen);

    // xmlListener Implementation
    void startTag(const char *tagName) override;
    void endTag(const char *tagName) override;
    void parameter(const char *param) override;
    void value(const char *value) override;
    void attribute(const char *attribute) override;

public:
    void executeFetch() override;
    
    /**
     * @brief Performs a full WSDL discovery to find the latest SOAP endpoints.
     *        Updates the global config and persists to disk on success.
     * @return int Update status code.
     */
    int refreshWsdl();
};

#endif // NATIONAL_RAIL_DATA_SOURCE_HPP

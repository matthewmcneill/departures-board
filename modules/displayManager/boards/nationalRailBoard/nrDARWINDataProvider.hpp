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
 * - nrDARWINDataProvider: Concrete legacy DARWIN backend provider implementing iNationalRailDataProvider.
 *   - init(): Attempts discovery initialization for SOAP endpoints via cached parameters.
 *   - refreshWsdl(): SOAP discovery and WSDL parsing over HTTPS.
 *   - updateData(): Intercept and request a priority update from the data manager.
 *   - getStationData(): Returns pointer to parsed station metadata payload.
 *   - getMessagesData(): Accessor for rail disruption string pool.
 *   - configure(): Bootstrapping routine to sink configuration parameters into instance.
 *   - executeFetch(): Internal blocking HTTP REST/SOAP engine for retrieving and parsing XML payload.
 *   - lockData() / unlockData(): Thread synchronization protecting the double buffer.
 *   - testConnection(): Validates station codes and tokens using lightweight query.
 *   - getNextFetchTime() / setNextFetchTime(): Polling interval tracking mapping.
 */

#ifndef NR_DARWIN_DATA_PROVIDER_HPP
#define NR_DARWIN_DATA_PROVIDER_HPP

#include "iNationalRailDataProvider.hpp"
#include "../xmlListener/xmlListener.hpp"
#include <memory>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <Arduino.h>

/**
 * @brief Concrete XML implementation of iNationalRailDataProvider for the legacy DARWIN LDBWS API.
 */
class nrDARWINDataProvider : public iNationalRailDataProvider, public xmlListener {
private:
    std::unique_ptr<NationalRailStation> stationData; // Internal background writing buffer
    std::unique_ptr<NationalRailStation> renderData;  // Thread-safe protected readable payload
    MessagePool messagesData; // Mutable string pool for alerts
    MessagePool renderMessages; // Immutable presented pool

    SemaphoreHandle_t dataMutex; // Thread-safe lock protecting data transfers
    volatile UpdateStatus taskStatus; // Cross-thread execution status tracking (e.g., UpdateStatus::PENDING)

    char lastErrorMessage[128]; // Detailed string descriptor of errors

    // Darwin API / SOAP endpoints tracking
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

    // Configuration Settings
    char nrToken[37];
    char crsCode[4];
    char callingCrsCode[4];
    int nrTimeOffset;

    // Internal Utility Methods (ported from legacy raildataXmlClient)
    static bool compareTimes(const NationalRailService& a, const NationalRailService& b);
    void removeHtmlTags(char* input);
    void replaceWord(char* input, const char* target, const char* replacement);
    void pruneFromPhrase(char* input, const char* target);
    void fixFullStop(char* input);
    void trim(char* &start, char* &end);
    bool equalsIgnoreCase(const char* a, int a_len, const char* b);

    bool serviceMatchesFilter(const char* filter, const char* serviceId);
    void sanitiseData();
    void deleteService(int x);

public:
    nrDARWINDataProvider();
    virtual ~nrDARWINDataProvider() = default;

    // DataManager Scheduling Implementations
    uint32_t getNextFetchTime() override { return nextFetchTimeMillis; }
    PriorityTier getPriorityTier() override;
    void setNextFetchTime(uint32_t forceTimeMillis) override { nextFetchTimeMillis = forceTimeMillis; }

private:
    uint32_t nextFetchTimeMillis; // Real-world millis representation to delay API polling
    static const uint32_t BASELINE_MIN_INTERVAL = 30000;

    /**
     * @brief Manually set the SOAP address. Used to bypass WSDL download in test mode.
     */
    void setSoapAddress(const char* host, const char* api);

public:
    // --- iDataSource overrides ---
    
    UpdateStatus updateData() override;
    UpdateStatus getLastUpdateStatus() const override { return taskStatus; }
    const char* getLastErrorMsg() const override { return lastErrorMessage; }
    UpdateStatus testConnection(const char* token = nullptr, const char* stationId = nullptr) override;

    /**
     * @brief Returns the strict legal attribution string for the Darwin API.
     * @return Literal static string for the OLED message widget.
     */
    const char* getAttributionString() const override { return "Powered by National Rail Enquiries"; }


    // Configuration & Data Access
    
    /**
     * @brief Initialize the data source discovering SOAP endpoints.
     * @param wsdlHost Host location resolving WSDL mappings.
     * @param wsdlAPI Relative address fragment mapping configuration.
     * @return UpdateStatus evaluation depending on network routing capabilities.
     */
    UpdateStatus init(const char *wsdlHost, const char *wsdlAPI);
    bool isInitialized() const { return soapHost[0] != '\0'; }
    void configure(const char* token, const char* crs, const char* filter = "", const char* callingCrs = "", int offset = 0) override;
    
    NationalRailStation* getStationData() override { return renderData.get(); }
    MessagePool* getMessagesData() override { return &renderMessages; }
    void cleanFilter(const char* rawFilter, char* cleanedFilter, size_t maxLen);

    // iDataSource Mutex controls
    void lockData() override { if(dataMutex) xSemaphoreTake(dataMutex, portMAX_DELAY); }
    void unlockData() override { if(dataMutex) xSemaphoreGive(dataMutex); }

    // --- xmlListener overrides ---
    void startTag(const char *tagName) override;
    void endTag(const char *tagName) override;
    void parameter(const char *param) override;
    void value(const char *value) override;
    void attribute(const char *attribute) override;

public:
    void executeFetch() override;
    void serializeData(JsonObject& doc) override;
    
    /**
     * @brief Performs a full WSDL discovery to find the latest SOAP endpoints.
     *        Updates the global config and persists to disk on success.
     * @return UpdateStatus code depending on HTTP success status boundary.
     */
    UpdateStatus refreshWsdl();
};

#endif // NR_DARWIN_DATA_PROVIDER_HPP

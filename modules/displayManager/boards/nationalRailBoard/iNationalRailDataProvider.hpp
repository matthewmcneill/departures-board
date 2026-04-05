/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/boards/nationalRailBoard/iNationalRailDataProvider.hpp
 * Description: Abstract interface for National Rail Data structures and Data Providers.
 *
 * Exported Functions/Classes:
 * - NrServiceType: Enum defining the type of transit service.
 * - NrCoachFormation: Struct defining the physical train formation data.
 * - NationalRailService: Struct tracking an individual train or bus departure.
 * - NationalRailStation: Struct aggregating the total data payload for a station.
 * - iNationalRailDataProvider: Abstract base class for National Rail sources.
 *   - configure(): Inject API credentials and filtering rules.
 *   - getLastUpdateStatus(): Retrieve the health status of the previous poll.
 *   - getStationData(): Retrieve the primary processed display payload pointer.
 *   - getMessagesData(): Retrieve the global disruptions message pool.
 */

#pragma once

#include <Arduino.h>
#include "../../../dataManager/iDataSource.hpp"
#include "../../messaging/messagePool.hpp"

/**
 * @brief Service Type Constants
 */
enum class NrServiceType : uint8_t {
    OTHER = 0,
    TRAIN = 1,
    BUS = 2
};

// Data Length Constants
constexpr int NR_MAX_LOCATION = 45; // Max characters for location names
constexpr int NR_MAX_CALLING = 450; // Max characters for calling point aggregation
constexpr int NR_MAX_MSG_LEN = 400; // Max characters for disruption messages
constexpr int NR_MAX_SERVICES = 9;  // Maximum services processed per board
constexpr int NR_MAX_COACHES = 12;  // Maximum coach data structures per service

/**
 * @brief Data structure defining individual train coach formations
 */
struct NrCoachFormation {
    char coachClass;
    char loading[16];
    uint8_t coachNumber;
};

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
    uint8_t classesAvailable;
    char opco[50];
    NrServiceType serviceType; // TRAIN or BUS
    
    // Note: To save RAM, formation data is only retained for the first due service and 
    // is stored at the NationalRailStation level instead of within individual services.
};

/**
 * @brief Container for station-level departure data.
 */
struct NationalRailStation {
    char location[NR_MAX_LOCATION];
    char stationName[NR_MAX_LOCATION];
    char filterVia[NR_MAX_LOCATION];
    char filterPlatform[16];
    bool platformAvailable;
    bool boardChanged;
    
    // Details strictly for the first train in the sequence
    char firstServiceCalling[NR_MAX_CALLING]; // Calling points for the first due service
    char firstServiceOrigin[NR_MAX_LOCATION]; // Origin station name for the first due service
    char firstServiceLastSeen[NR_MAX_LOCATION]; // Real-time actual location snippet
    char firstServiceMessage[NR_MAX_MSG_LEN]; // Specific operator disruption messages for the service
    
    // Features for the first train in the sequence
    NrCoachFormation firstServiceFormation[NR_MAX_COACHES];
    uint8_t firstServiceNumCoaches;

    int numServices;
    NationalRailService service[NR_MAX_SERVICES];
    uint32_t contentHash;
};

/**
 * @brief Interface providing polymorphism for National Rail backend APIs (e.g., DARWIN, RDM).
 */
class iNationalRailDataProvider : public iDataSource {
public:
    virtual ~iNationalRailDataProvider() = default;

    /**
     * @brief Configure the data provider with required credentials and logic filters.
     * @param token Provider-specific API authentication token.
     * @param crs The principal CRS station code for departures.
     * @param filter Optional comma-separated list of platform numbers to whitelist.
     * @param callingCrs Optional destination CRS code to filter services journeying towards.
     * @param offset Optional temporal offset in minutes to look ahead.
     */
    virtual void configure(const char* token, const char* crs, const char* filter = "", const char* callingCrs = "", int offset = 0) = 0;

    /**
     * @brief Get the status code of the last background update execution.
     * @return UpdateStatus enumeration evaluating network and API parse success.
     */
    virtual UpdateStatus getLastUpdateStatus() const = 0;

    /**
     * @brief Get pointer to the parsed domain data for rendering.
     * @return Raw pointer to the NationalRailStation struct aggregate.
     */
    virtual NationalRailStation* getStationData() = 0;

    /**
     * @brief Get pointer to the parsed disruption message pool.
     * @return Raw pointer to the internal MessagePool storing operator alerts.
     */
    virtual MessagePool* getMessagesData() = 0;
};

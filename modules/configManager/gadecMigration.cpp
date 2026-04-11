/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/configManager/gadecMigration.cpp
 * Description: Implementation of the epoch detection matrix and translation 
 * logic for legacy Gadec-uk configurations.
 */

#include "gadecMigration.hpp"
#include <Arduino.h>

namespace GadecMigration {

/**
 * @brief Analyzes a JSON object to determine its configuration epoch.
 */
UpstreamEpoch detectConfigEpoch(JsonObject root) {
    // If it has a version field, we can use it to pinpoint the epoch
    if (root.containsKey("version")) {
        float version = root["version"] | 1.0f;
        if (version >= 2.6f) return EPOCH_LATEST_NATIVE;
        if (version >= 2.0f) return EPOCH_GADEC_V2;
        return EPOCH_GADEC_V1;
    }

    // No version field: check for characteristic legacy keys
    // Gadec v1.x often had flat crs/tubeId/busId keys without a version
    if (root.containsKey("crs") || root.containsKey("tubeId") || root.containsKey("busId")) {
        return EPOCH_GADEC_V1;
    }

    // If it has a boards array but no version >= 2.6, it's Gadec v2.x
    if (root.containsKey("boards")) {
        return EPOCH_GADEC_V2;
    }

    return EPOCH_UNKNOWN;
}

/**
 * @brief Translates a legacy configuration document into the modern v2.6 nested schema.
 */
bool translateToModern(JsonDocument& doc, UpstreamEpoch epoch) {
    if (epoch == EPOCH_LATEST_NATIVE) return true;

    JsonObject root = doc.as<JsonObject>();
    float loadedVersion = root["version"] | 1.0f;

    // --- Step 1: Structural Elevation (Flat to Array) ---
    // Handle very old flat schemas (v1.x / < 2.2f)
    if (loadedVersion < 2.2f && !root.containsKey("boards")) {
        JsonArray boards = root["boards"].to<JsonArray>();

        // Migrate NR Rail Station
        if (root.containsKey("crs")) {
            JsonObject b = boards.add<JsonObject>();
            b["type"] = 0; // MODE_RAIL
            b["id"] = root["crs"] | "";
            b["lat"] = root["lat"] | root["stationLat"] | 0.0f;
            b["lon"] = root["lon"] | root["stationLon"] | 0.0f;
            b["secId"] = root["callingCrs"] | "";
            b["secName"] = root["callingStation"] | "";
            b["filter"] = root["platformFilter"] | "";
            b["offset"] = root["nrTimeOffset"] | 0;
        }

        // Migrate Tube Station
        if (root.containsKey("tubeId")) {
            JsonObject b = boards.add<JsonObject>();
            b["type"] = 1; // MODE_TUBE
            b["id"] = root["tubeId"] | "";
            b["name"] = root["tubeName"] | "";
        }

        // Migrate Bus Stop
        if (root.containsKey("busId")) {
            JsonObject b = boards.add<JsonObject>();
            b["type"] = 2; // MODE_BUS
            b["id"] = root["busId"] | "";
            b["name"] = root["busName"] | "";
            b["lat"] = root["busLat"] | 0.0f;
            b["lon"] = root["busLon"] | 0.0f;
            b["filter"] = root["busFilter"] | "";
        }
    }

    // --- Step 2: Elevate Secondary Rail Boards (v2.2 logic) ---
    if (loadedVersion < 2.3f && root.containsKey("altCrs")) {
        JsonArray boards = root["boards"].as<JsonArray>();
        const char* altCrs = root["altCrs"];
        
        bool alreadyIn = false;
        for (JsonObject b : boards) {
            if ((b["type"] | 0) == 0 && b["id"] == altCrs) {
                alreadyIn = true;
                break;
            }
        }

        if (!alreadyIn) {
            JsonObject b = boards.add<JsonObject>();
            b["type"] = 0; // MODE_RAIL
            b["id"] = altCrs;
            b["lat"] = root["altLat"] | 0.0f;
            b["lon"] = root["altLon"] | 0.0f;
            b["secId"] = root["altCallingCrs"] | "";
            b["secName"] = root["altCallingStation"] | "";
            b["filter"] = root["altPlatformFilter"] | "";
        }
    }

    // --- Step 3: Sleep & Clock Migration (v2.5 logic) ---
    if (loadedVersion < 2.5f) {
        bool legacySleep = root["sleep"] | false;
        JsonArray boards = root["boards"].as<JsonArray>();

        if (legacySleep) {
            int startH = root["sleepStarts"] | 23;
            int endH = root["sleepEnds"] | 8;

            // Ensure a clock board exists if it was meant to be shown
            int clockIdx = -1;
            int idx = 0;
            for (JsonObject b : boards) {
                if ((b["type"] | 0) == 3) { // MODE_CLOCK
                    clockIdx = idx;
                    break;
                }
                idx++;
            }

            if (clockIdx == -1) {
                clockIdx = boards.size();
                JsonObject b = boards.add<JsonObject>();
                b["type"] = 3; // MODE_CLOCK
                b["name"] = "Snooze Clock";
                b["oledOff"] = !(root["clock"] | true);
            }

            // Provision Scheduler Rule
            if (!root.containsKey("schedules")) {
                root["schedules"].to<JsonArray>();
            }
            JsonArray schedules = root["schedules"].as<JsonArray>();
            JsonObject r = schedules.add<JsonObject>();
            r["startH"] = startH;
            r["startM"] = 0;
            r["endH"] = endH;
            r["endM"] = 0;
            r["board"] = clockIdx;
        }

        // Handle reordering if legacy "mode" (active board index) was not 0
        int legacyActiveIndex = root["mode"] | 0;
        if (legacyActiveIndex > 0 && legacyActiveIndex < (int)boards.size()) {
            // Reordering logic is complex in ArduinoJson without copying. 
            // For now, we just ensure the data is structurally correct.
        }
    }

    // --- Step 4: Feeds Root Unification (v2.6 logic) ---
    if (loadedVersion < 2.6f) {
        if (!root.containsKey("feeds")) {
            JsonObject feeds = root["feeds"].to<JsonObject>();
            
            // Move RSS fields
            if (root.containsKey("rssUrl")) feeds["rss"] = root["rssUrl"];
            if (root.containsKey("rssName")) feeds["rssName"] = root["rssName"];
            
            // Move Weather Key
            if (root.containsKey("weatherKeyId")) feeds["weatherKeyId"] = root["weatherKeyId"];
            
            // Delete old root keys (optional, but cleaner)
            root.remove("rssUrl");
            root.remove("rssName");
            root.remove("weatherKeyId");
        }
    }

    // --- Finalize: Set Target Version ---
    root["version"] = 2.6f;

    return true;
}

} // namespace GadecMigration

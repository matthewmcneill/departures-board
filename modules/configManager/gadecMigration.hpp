/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/configManager/gadecMigration.hpp
 * Description: Standalone translation layer to convert legacy Gadec-uk schemas 
 * into modern v3.0 nested structures (v2.6) in memory.
 *
 * Exported Functions/Classes:
 * - GadecMigration: Namespace containing migration utilities
 *   - detectConfigEpoch(): Sniffs a JsonObject to determine its schema version
 *   - translateToModern(): In-place translation of legacy JSON into v2.6 schema
 */

#pragma once
#include <ArduinoJson.h>

namespace GadecMigration {

    /**
     * @brief Enum representing the detected version/epoch of a configuration file.
     */
    enum UpstreamEpoch {
        EPOCH_LATEST_NATIVE = 0, // Already compliant with v2.6+
        EPOCH_GADEC_V1 = 1,      // Flat schema with older tube variables
        EPOCH_GADEC_V2 = 2,      // Standard flat Gadec-uk schema
        EPOCH_UNKNOWN = 99       // Unrecognized schema
    };

    /**
     * @brief Analyzes a JSON object to determine its configuration epoch.
     * @param root The JsonObject to analyze.
     * @return The detected UpstreamEpoch.
     */
    UpstreamEpoch detectConfigEpoch(JsonObject root);

    /**
     * @brief Translates a legacy configuration document into the modern v2.6 nested schema.
     * @param doc The JsonDocument to modify in-place.
     * @param epoch The detected epoch to translate from.
     * @return True if translation succeeded or was unnecessary, false on failure.
     */
    bool translateToModern(JsonDocument& doc, UpstreamEpoch epoch);

} // namespace GadecMigration

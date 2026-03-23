/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: tools/layoutsim/src/LittleFS.h
 * Description: Emscripten/WASM mock stub for the Arduino framework.
 */

#ifndef MOCK_LITTLEFS_H
#define MOCK_LITTLEFS_H

#include "FS.h"

class LittleFSFS : public fs::FS {
public:
    bool begin(bool formatOnFail = false, const char *basePath = "/littlefs", uint8_t maxOpenFiles = 10, const char *partitionLabel = "spiffs") {
        return true;
    }
};

extern LittleFSFS LittleFS;

#endif

/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * MD5 Utilities Library - calculate the MD5 hash of a file on the LittleFS file system. Convert a base64 MD5 hash to Hex.
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/md5Utils/md5Utils.h
 * Description: Utilities for MD5 calculation and base64-to-hex conversion.
 *
 * Exported Functions/Classes:
 * - class md5Utils: Wrapper class for MD5 generation operations.
 */

#pragma once
#include <Arduino.h>

class md5Utils {
    private:

    public:
        md5Utils();
/**
 * @brief Calculates the MD5 hash of a specified file on the LittleFS system.
 * @param filePath The absolute path to the file on LittleFS (e.g. "/webgui.html").
 * @return A 32-character hexadecimal MD5 string, or empty if the file cannot be read.
 */
        String calculateFileMD5(const char* filePath);
/**
 * @brief Converts a base64 encoded MD5 hash (such as those from GitHub headers) into a hex string.
 * @param base64Hash The base64 encoded string.
 * @return A 32-character hexadecimal MD5 string.
 */
        String base64ToHex(String base64Hash);
};

/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: Portal Assets
 * Description: Automated Portal Assets - embedded gzipped binary data.
 * Exported cleanly via .h/.cpp to remain in ESP32 Flash (.rodata) and conserve RAM.
 */
#pragma once

#include <Arduino.h>

// Original: index.html (181484 bytes), Minified: 125973 bytes, Gzipped: (29251 bytes)
extern const uint8_t index_html_gz[] __attribute__((section(".rodata")));
extern const uint32_t index_html_gz_len __attribute__((section(".rodata")));

// Original: rss.json (831 bytes), Minified: 791 bytes, Gzipped: (301 bytes)
extern const uint8_t rss_json_gz[] __attribute__((section(".rodata")));
extern const uint32_t rss_json_gz_len __attribute__((section(".rodata")));

// Original: screenshot.html (6681 bytes), Minified: 4667 bytes, Gzipped: (2047 bytes)
extern const uint8_t screenshot_html_gz[] __attribute__((section(".rodata")));
extern const uint32_t screenshot_html_gz_len __attribute__((section(".rodata")));


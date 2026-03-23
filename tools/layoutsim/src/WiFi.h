/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: tools/layoutsim/src/WiFi.h
 * Description: Emscripten/WASM mock stub for the Arduino framework.
 */

#ifndef MOCK_WIFI_H
#define MOCK_WIFI_H

#include <stdint.h>

enum wl_status_t {
    WL_IDLE_STATUS,
    WL_NO_SSID_AVAIL,
    WL_SCAN_COMPLETED,
    WL_CONNECTED,
    WL_CONNECT_FAILED,
    WL_CONNECTION_LOST,
    WL_DISCONNECTED
};

class MockWiFi {
public:
    wl_status_t status() { return WL_CONNECTED; }
    const char* SSID() { return "Simulator-WiFi"; }
    int8_t RSSI() { return -50; }
};

extern MockWiFi WiFi;

#endif

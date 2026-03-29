/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 */

#ifndef MOCK_WIFI_H
#define MOCK_WIFI_H

#include <stdint.h>
#include "Arduino.h"
#include "SystemState.hpp"

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
    wl_status_t status() { 
        return SystemState::getInstance().isWifiConnected() ? WL_CONNECTED : WL_DISCONNECTED; 
    }
    const char* SSID() { return "Simulator-WiFi"; }
    int8_t RSSI() { 
        // Logic Injection: If connected, return -50 (good signal). 
        // Future: could add setRssi to SystemState.
        return -50; 
    }
};

extern MockWiFi WiFi;

#endif

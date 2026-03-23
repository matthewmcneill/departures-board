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

#ifndef WIFI_MOCK_H
#define WIFI_MOCK_H

#include "Arduino.h"
#include "IPAddress.h"
#include "Network.h"

/**
 * @brief Mock implementation of WiFi for native host testing. 
 */
class WiFiClass {
public:
    WiFiClass() {}
    void begin(const char* ssid, const char* pass) {}
    void disconnect() {}
    wl_status_t status() { return WL_CONNECTED; }
    IPAddress localIP() { return IPAddress(192, 168, 1, 100); }
    String macAddress() { return "00:00:00:00:00:00"; }
};

extern WiFiClass WiFi;

#endif

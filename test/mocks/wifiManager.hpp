#ifndef WIFI_MANAGER_MOCK_HPP
#define WIFI_MANAGER_MOCK_HPP

#include <Arduino.h>

class wifiManager {
public:
    wifiManager() : wifiConfigured(true), wifiConnected(true) {}
    void init() {}
    void tick() {}
    bool isWifiConfigured() const { return wifiConfigured; }
    bool isWifiConnected() const { return wifiConnected; }
    void setWifiConnected(bool c) { wifiConnected = c; }
    
    bool wifiConfigured;
    bool wifiConnected;
};

#endif

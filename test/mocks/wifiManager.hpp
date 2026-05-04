#ifndef WIFI_MANAGER_MOCK_HPP
#define WIFI_MANAGER_MOCK_HPP

#include <Arduino.h>

#include "iConfigurable.hpp"
#include "deviceCrypto.hpp"

class WifiManager : public iConfigurable {
public:
    WifiManager() : wifiConfigured(true), wifiConnected(true) {}
    void init() {}
    void tick() {}
    bool isWifiConfigured() const { return wifiConfigured; }
    bool isWifiConnected() const { return wifiConnected; }
    void setWifiConnected(bool c) { wifiConnected = c; }
    
    void bindCrypto(DeviceCrypto* dc) {}
    void begin(const String& hostname) {}
    bool isReady() const { return true; }
    void reapplyConfig(const Config& config) override {}
    bool getAPMode() const { return false; }
    const char* getSSID() const { return ""; }
    bool isWifiPersistentError() const { return false; }
    
    bool wifiConfigured;
    bool wifiConnected;
};

#endif

#ifndef WIFI_CLIENT_SECURE_MOCK_H
#define WIFI_CLIENT_SECURE_MOCK_H

#include "Arduino.h"
#include "WiFiClient.h"

/**
 * @brief Mock implementation of WiFiClientSecure for native host testing. 
 *        Inherits from WiFiClient to provide the full Stream interface.
 */
class WiFiClientSecure : public WiFiClient {
public:
    WiFiClientSecure() {}
    void setInsecure() {}
    void setCACert(const char* cert) {}
    void setTimeout(uint32_t seconds) {}
    void setConnectionTimeout(uint32_t seconds) {}
};

#endif

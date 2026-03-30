#ifndef WIFI_CLIENT_MOCK_H
#define WIFI_CLIENT_MOCK_H

#include "Arduino.h"

/**
 * @brief Mock implementation of WiFiClient for native host testing. 
 */
class WiFiClient : public Stream {
public:
    WiFiClient() {}
    int connect(const char* host, uint16_t port) { return 1; }
    void stop() {}
    uint8_t connected() { return 1; }
    
    // Abstract Stream methods
    size_t write(uint8_t b) override { return 1; }
    int available() override { return 0; }
    int read() override { return -1; }
    int peek() override { return -1; }
    void flush() override {}
};

#endif

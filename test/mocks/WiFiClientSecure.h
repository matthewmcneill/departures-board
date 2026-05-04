#ifndef WIFI_CLIENT_SECURE_MOCK_H
#define WIFI_CLIENT_SECURE_MOCK_H

#include <WString.h>
#include "WiFiClient.h"

class WiFiClientSecure : public WiFiClient {
public:
    void setTimeout(int) {}
    void setConnectionTimeout(int) {}
    bool connect(const char*, int) { return false; }
    bool connect(const String&, int) { return false; }
    int available() { return 0; }
    int read() { return -1; }
    int read(uint8_t *buf, size_t size) { return size; }
    String readStringUntil(char terminator) { return ""; }
    void stop() {}
    bool connected() { return false; }
    void setInsecure() {}
    void setHandshakeTimeout(int) {}
};

#endif

#ifndef HTTP_CLIENT_MOCK_H
#define HTTP_CLIENT_MOCK_H

#include "Arduino.h"
#include "FS.h"
#include "WiFiClient.h"

/**
 * @brief Mock implementation of HTTPClient for native host testing and simulator data sources. 
 */
class HTTPClient {
public:
    HTTPClient() {}
    ~HTTPClient() {}

    bool begin(WiFiClient& client, String url) { return true; }
    bool begin(String url) { return true; }
    void end() {}

    int GET() { return 200; }
    int POST(String payload) { return 200; }
    int POST(uint8_t* payload, size_t size) { return 200; }

    String getString() { return "{}"; }
    Stream& getStream() { static String s="{}"; return (Stream&)s; }
    int getSize() { return 0; }
};

#endif

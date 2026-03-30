#ifndef WEB_SERVER_MOCK_H
#define WEB_SERVER_MOCK_H

#include "Arduino.h"

class WebServer {
public:
    WebServer(int port = 80) {}
    void begin() {}
    void handleClient() {}
    void on(const char* path, void (*handler)()) {}
    void send(int code, const char* contentType, const char* content) {}
    String arg(const char* name) { return ""; }
};

#endif

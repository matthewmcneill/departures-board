#pragma once
#include <string>

class AsyncWebServerRequest {};
class AsyncWebServer {
public:
    AsyncWebServer(int port) {}
    void begin() {}
    void end() {}
};

extern AsyncWebServer server;

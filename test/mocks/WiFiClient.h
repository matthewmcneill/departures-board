#ifndef MOCK_WIFICLIENT_H
#define MOCK_WIFICLIENT_H

#include <WString.h>
#include <Stream.h>

class WiFiClient : public Stream {
public:
    WiFiClient() {}
    virtual ~WiFiClient() {}
    virtual bool connect(const char* host, uint16_t port) { return true; }
    virtual void stop() {}
    virtual size_t write(uint8_t) { return 0; }
    virtual size_t write(const uint8_t *buf, size_t size) { return size; }
    virtual size_t print(const String &s) { return s.length(); }
    virtual int available() { return 0; }
    virtual int read() { return -1; }
    virtual int read(uint8_t *buf, size_t size) { return size; }
    virtual String readStringUntil(char terminator) { return ""; }
    virtual bool connected() { return false; }
    virtual int peek() { return -1; }
    virtual void flush() {}
};

#endif // MOCK_WIFICLIENT_H

#ifndef ARDUINO_COMPAT_H
#define ARDUINO_COMPAT_H

#include <stdint.h>
#include <string.h>
#include <time.h> 
#include <string>
#include <iostream>

// Forward declarations or early definitions
class String {
private:
    std::string data;
public:
    size_t write(uint8_t c) {
        data.push_back((char)c);
        return 1;
    }
    size_t write(const uint8_t *buffer, size_t size) {
        data.append((const char*)buffer, size);
        return size;
    }
    
    const char* c_str() const { return data.c_str(); }
    operator const char*() const { return data.c_str(); }

    String() : data("") {}
    String(const char* s) : data(s ? s : "") {}
    String(const std::string& s) : data(s) {}
    String(int num) : data(std::to_string(num)) {}
    String(float num) : data(std::to_string(num)) {}
    String(double num) : data(std::to_string(num)) {}
    String(unsigned int num) : data(std::to_string(num)) {}
    String(long num) : data(std::to_string(num)) {}
    String(unsigned long num) : data(std::to_string(num)) {}
    
    int indexOf(String str) const { 
        size_t pos = data.find(str.c_str());
        return (pos == std::string::npos) ? -1 : (int)pos;
    }
    String substring(int start, int end = -1) const {
        if (end == -1) return data.substr(start).c_str();
        return data.substr(start, end - start).c_str();
    }
    bool startsWith(const char* s) const { return data.find(s) == 0; }
    void toLowerCase() { for(auto &c : data) c = std::tolower(c); }
    size_t length() const { return data.length(); }
    void replace(String from, String to) {
        size_t pos = 0;
        while ((pos = data.find(from.c_str(), pos)) != std::string::npos) {
            data.replace(pos, from.length(), to.data);
            pos += to.length();
        }
    }
    bool concat(const String& s) { data += s.data; return true; }
    bool concat(const char* s) { if (s) data += s; return true; }
    bool concat(int num) { data += std::to_string(num); return true; }
    bool concat(unsigned int num) { data += std::to_string(num); return true; }
    bool concat(long num) { data += std::to_string(num); return true; }
    bool concat(unsigned long num) { data += std::to_string(num); return true; }
    
    bool operator==(const String& other) const { return data == other.data; }
    bool operator!=(const String& other) const { return data != other.data; }
    bool operator==(const char* other) const { return other ? data == other : false; }
    bool operator!=(const char* other) const { return other ? data != other : true; }
    String& operator+=(const String& other) { data += other.data; return *this; }
    String& operator+=(const char* other) { if (other) data += other; return *this; }
};

inline String operator+(const String& lhs, const String& rhs) { String res = lhs; res += rhs; return res; }
inline String operator+(const String& lhs, const char* rhs) { String res = lhs; res += rhs; return res; }
inline String operator+(const char* lhs, const String& rhs) { String res(lhs); res += rhs; return res; }

#include "Print.h"
#include "Stream.h"

// Define byte as uint8_t globally.
// This is exactly how it's done in the Arduino framework.
typedef uint8_t byte;
typedef bool boolean;

class Printable {
public:
    virtual size_t printTo(Print& p) const = 0;
};

// Mock PROGMEM and Flash strings
#define PROGMEM
#define PSTR(s) (s)
#define F(s) (s)

#define LOW 0
#define HIGH 1
#define INPUT 0
#define OUTPUT 1

// Mock Arduino functions
#define yield() (void)0
#define millis() ((uint32_t)time(NULL) * 1000)
#define pinMode(pin, mode) (void)0
#define digitalWrite(pin, val) (void)0
#define digitalRead(pin) 0
#define delay(ms) (void)0
#define delayMicroseconds(us) (void)0
#define configTime(off, dst, s1) (void)0
inline bool getLocalTime(struct tm* t) { return false; }

// SPI Mocks
#define SPI_MODE0 0
#define SPI_MODE1 1
#define SPI_MODE2 2
#define SPI_MODE3 3
#define MSBFIRST 0
#define SPI_CLOCK_DIV2 2
#define SPI_CLOCK_DIV4 4
#define SPI_CLOCK_DIV8 8

class MockSPI {
public:
    void begin() {}
    void end() {}
    void beginTransaction(void*) {}
    void endTransaction() {}
    uint8_t transfer(uint8_t b) { return 0; }
    void transfer(void* buf, size_t count) {}
    void setBitOrder(uint8_t) {}
    void setDataMode(uint8_t) {}
    void setClockDivider(uint8_t) {}
    void setClock(uint32_t) {}
};
extern MockSPI SPI;

class MockWire {
public:
    void begin() {}
    void beginTransmission(uint8_t) {}
    uint8_t endTransmission() { return 0; }
    size_t write(uint8_t) { return 1; }
    size_t write(const uint8_t*, size_t) { return 0; }
};
extern MockWire Wire;

struct MockSerialRef : public Print {
    void begin(unsigned long baud) {}
    virtual size_t write(uint8_t c) override { putchar(c); return 1; }
    virtual size_t write(const uint8_t *buffer, size_t size) override { 
        return fwrite(buffer, 1, size, stdout); 
    }
};
extern MockSerialRef& Serial;

class __FlashStringHelper;

#define strcpy_P(dest, src) strcpy(dest, (const char*)(src))
#define pgm_read_byte(addr) (*(const uint8_t*)(addr))

#endif // ARDUINO_COMPAT_H

#ifndef ARDUINO_COMPAT_H
#define ARDUINO_COMPAT_H

#include <stdint.h>
#include <string.h>
#include <time.h> // Required for time(NULL)
#include <string>
#include "Print.h"

typedef std::string String;

class Printable {
public:
    virtual size_t printTo(Print& p) const = 0;
};

// Mock PROGMEM and Flash strings
#define PROGMEM
#define PSTR(s) (s)
#define F(s) (s)

// Mock Arduino functions
#define yield() (void)0
#define millis() ((uint32_t)time(NULL) * 1000)
#define pinMode(pin, mode) (void)0
#define digitalWrite(pin, val) (void)0
#define digitalRead(pin) 0
#define delay(ms) (void)0
#define delayMicroseconds(us) (void)0

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

class __FlashStringHelper;

#define strcpy_P(dest, src) strcpy(dest, (const char*)(src))
#define pgm_read_byte(addr) (*(const uint8_t*)(addr))

#endif // ARDUINO_COMPAT_H

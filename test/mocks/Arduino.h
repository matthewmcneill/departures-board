/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: tools/layoutsim/src/Arduino.h
 * Description: Emscripten/WASM mock stub for the Arduino framework.
 */

#ifndef ARDUINO_COMPAT_H
#define ARDUINO_COMPAT_H

#include <stdint.h>
#include <string.h>
#include <time.h> // Required for time(NULL)
#include <string>
#include "Print.h"

class String : public std::string {
public:
    String() : std::string() {}
    String(const char* s) : std::string(s ? s : "") {}
    String(const std::string& s) : std::string(s) {}
    String(int num) : std::string(std::to_string(num)) {}
    String(float num) : std::string(std::to_string(num)) {}
    String(double num) : std::string(std::to_string(num)) {}
    String(unsigned int num) : std::string(std::to_string(num)) {}
    String(long num) : std::string(std::to_string(num)) {}
    String(unsigned long num) : std::string(std::to_string(num)) {}
};

class IPAddress {
public:
    IPAddress() { memset(ip, 0, 4); }
    IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) { ip[0]=a; ip[1]=b; ip[2]=c; ip[3]=d; }
    uint8_t ip[4];
    std::string toString() const { return std::to_string(ip[0])+"."+std::to_string(ip[1])+"."+std::to_string(ip[2])+"."+std::to_string(ip[3]); }
};

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

#include "Arduino.h"
#include "WiFi.h"
#include "LittleFS.h"
#include "SPI.h"
#include "Wire.h"
#include <cstdio>
#include <map>
#include <string>

std::map<std::string, std::string> mockFSMap;

// Hardware Mock Storage
MockSPI SPI;
MockWire Wire;
WiFiClass WiFi;
LittleFS_Mock LittleFS;

// --- Serial Mock ---
MockSerialRef SerialMock;
MockSerialRef& Serial = SerialMock;

// --- Serial Mock ---
// --- TimeManager Stub ---
#include "timeManager.hpp"
const char TimeManager::ukTimezone[] = "GMT0BST,M3.5.0/1,M10.5.0";

// --- Logger Stub ---
#include "logger.hpp"
void Logger::registerSecret(const String& secret) {}

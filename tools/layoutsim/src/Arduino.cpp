#include "Arduino.h"
MockSPI SPI;
MockWire Wire;

#include "WiFi.h"
MockWiFi WiFi;

#include "LittleFS.h"
LittleFSFS LittleFS;

#include "appContext.hpp"
class appContext appContext;

#include "displayManager.hpp"
DisplayManager displayManager;

const char* weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Supported Hardware:
 * - ESP32 "Mini" (env: esp32dev)
 * - Waveshare ESP32-S3-Nano (env: esp32s3nano)
 * Both use a 3.12" 256x64 OLED Display Panel with SSD1322 controller on-board.
 *
 * PIN CONNECTIONS (Configurable via platformio.ini):
 * OLED PANEL     ESP32 MINI     ESP32-S3 NANO
 * 1  VSS         GND            GND
 * 2  VCC_IN      3.3V           3.3V
 * 4  D0/CLK      IO18           D13 (SCK)
 * 5  D1/DIN      IO23           D11 (COPI/MOSI)
 * 14 D/C#        DISPLAY_DC_PIN DISPLAY_DC_PIN  (e.g., IO5, D9)
 * 16 CS#         DISPLAY_CS_PIN DISPLAY_CS_PIN  (e.g., IO26, D10)
 *
 * NOTE ON HARDWARE SPI:
 * Because we are using the U8G2_..._HW_SPI (Hardware SPI) version of the display 
 * library rather than a Software SPI setup, the U8g2 library intentionally doesn't ask 
 * for the Clock and Data pin numbers simply because those pins are physically hardwired
 * inside the ESP32 chip to dedicated hardware micro-controllers for maximum performance.
 *
 * When we tell PlatformIO to build for board = arduino_nano_esp32 or board = esp32dev, 
 * it pulls in the specific Arduino Core variants for those boards under the hood. 
 * The core automatically maps the default Hardware SPI pathways to the correct pins 
 * for that respective board without us having to lift a finger in our codebase!
 *
 * Module: src/departuresBoard.cpp
 * Description: Main application entry point for the Departures Board.
 *              Manages hardware initialization and the core executive loop.
 *
 * Exported Functions/Classes:
 * - setup: Hardware and software initialization.
 * - loop: Main application polling and animation loop.
 */

// -----------------------------------------------------------------------------
// Libraries and Includes
// -----------------------------------------------------------------------------

// Core Library Includes
#include <Arduino.h>
#include <LittleFS.h>

// Networking & Web Libraries
#include <WiFi.h>
#include <ESPmDNS.h>

// Internal Modules & Managers
#include <Logger.hpp>
#include <WiFiConfig.hpp>
#include <configManager.hpp>
#include <timeManager.hpp>
#include <boards/systemBoard/loadingBoard.hpp>
#include <boards/systemBoard/splashBoard.hpp>
#include <displayManager.hpp>
#include <otaUpdater.hpp>
#include <webServer.hpp>

// API Service Clients
#include <weatherClient.h>
#include <githubClient.h>
#include <rssClient.h>


// -----------------------------------------------------------------------------
// Definitions & Macros
// -----------------------------------------------------------------------------

#include <buildOptions.h>

// -----------------------------------------------------------------------------
// Global Object Instantiations
// -----------------------------------------------------------------------------

#include <appContext.hpp>

appContext appContext;

// -----------------------------------------------------------------------------
// Environment Variables & State Management 
// -----------------------------------------------------------------------------

// Versioning
int VERSION_MAJOR = 2;
int VERSION_MINOR = 2;

// API Credits and Attribution Text
extern const char nrAttributionn[] = "Powered by National Rail Enquiries";
extern const char tflAttribution[] = "Powered by TfL Open Data";
extern const char btAttribution[] = "Powered by bustimes.org";



// -----------------------------------------------------------------------------
// Boot Setup
// -----------------------------------------------------------------------------

/**
 * @brief Initialize hardware and software components.
 */
void setup(void) {
  Logger::begin();
  LOG_SPLASH("DEPARTURES BOARD BOOTING...");

  LOG_INFO("SYSTEM", "Starting boot sequence...");  
  appContext.begin();
  LOG_INFO("SYSTEM", "appContext.begin() finished.");

  LOG_SPLASH("BOOT SEQUENCE COMPLETE");

}

// -----------------------------------------------------------------------------
// Core Execution Loop
// -----------------------------------------------------------------------------

/**
 * @brief Executive rendering and state management loop.
 */
void loop(void) {
  static unsigned long lastSerialHeartbeat = 0;
  if (millis() - lastSerialHeartbeat > 10000) {
    LOG_INFO("HEARTBEAT", "System is alive and ticking.");
    lastSerialHeartbeat = millis();
  }
  appContext.tick();
}

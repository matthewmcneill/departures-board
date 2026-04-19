/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International. To view a copy of
 * this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
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
 * Because we are using the U8G2_..._HW_SPI (Hardware SPI) version of the
 * display library rather than a Software SPI setup, the U8g2 library
 * intentionally doesn't ask for the Clock and Data pin numbers simply because
 * those pins are physically hardwired inside the ESP32 chip to dedicated
 * hardware micro-controllers for maximum performance.
 *
 * When we tell PlatformIO to build for board = arduino_nano_esp32 or board =
 * esp32dev, it pulls in the specific Arduino Core variants for those boards
 * under the hood. The core automatically maps the default Hardware SPI pathways
 * to the correct pins for that respective board without us having to lift a
 * finger in our codebase!
 *
 * V3.0 Architecture removes direct driver dependencies via interface
 * abstraction.
 */

#include "departuresBoard.hpp"
#include "../lib/boardLED/boardLED.hpp"

// PlatformIO LDF workaround for Core 3.x
#if defined(ESP_IDF_VERSION_MAJOR) && ESP_IDF_VERSION_MAJOR >= 5
#include <Network.h>
#endif

/*
 * Module: src/departuresBoard.cpp
 * Description: Main application entry point for the Departures Board firmware.
 *              Manages hardware initialization and the core executive loop.
 *
 * Exported Functions/Classes:
 * - setup: Hardware and software initialization bootstrap.
 * - loop: Primary executive rendering and maintenance loop.
 */

// -----------------------------------------------------------------------------
// Libraries and Includes
// -----------------------------------------------------------------------------

// Core Library Includes
#include <Arduino.h>

// Internal Modules & Managers
#include "buildTime.hpp"
#include <appContext.hpp>
#include <logger.hpp>
#include <otaUpdater.hpp>

// -----------------------------------------------------------------------------
// Definitions & Macros
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Global Object Instantiations
// -----------------------------------------------------------------------------

appContext
    appContext; // Global context managing application state and shared services

// -----------------------------------------------------------------------------
// Environment Variables & State Management
// -----------------------------------------------------------------------------

// (Attribution constants moved to specific board implementations)

// -----------------------------------------------------------------------------
// Boot Setup
// -----------------------------------------------------------------------------

/**
 * @brief Initialize all system hardware and software components.
 * Configures serial logging, starts the appContext orchestrator, and enters the
 * boot splash sequence.
 */
void setup(void) {
  LOG_BEGIN(115200);
  WAIT_FOR_SERIAL(
      5000); // Give the Native USB CDC stack time to reconnect for logs

#ifdef BUILD_TIME
  LOG_INFOf("SYSTEM", "Departures Board Version %d.%d (Build: %s)",
            VERSION_MAJOR, VERSION_MINOR, BUILD_TIME);
#else
  LOG_INFOf("SYSTEM", "Departures Board Version %d.%d", VERSION_MAJOR,
            VERSION_MINOR);
#endif

  LOG_INFO("SYSTEM", "Starting boot sequence...");
  appContext.begin();
  LOG_INFO("SYSTEM", "appContext.begin() finished.");

  // --- Step 3: Validate Firmware ---
  // Notify the bootloader partition manager that this boot was successful
  // preventing the hardware from automatically rolling back to the previous
  // firmware.
  ota.markAppValid();
}

// -----------------------------------------------------------------------------
// Core Execution Loop
// -----------------------------------------------------------------------------

/**
 * @brief Executive rendering and state management loop.
 */
void loop(void) {
  static unsigned long lastSerialHeartbeat = 0;
  static unsigned long heartbeatLedPulseTimer = 0;
  static bool ledPulsing = false;

  unsigned long currentMillis = millis();

  // 10-second heartbeat cycle triggers the 50ms LED pulse (independent of debug
  // level)
  if (currentMillis - lastSerialHeartbeat > 10000) {
    lastSerialHeartbeat = currentMillis;

    // Trigger LED pulse
    BoardLED::on();
    ledPulsing = true;
    heartbeatLedPulseTimer = currentMillis;

#if CORE_DEBUG_LEVEL >= APP_LOG_LEVEL_INFO
    char diagMsg[128];
    snprintf(diagMsg, sizeof(diagMsg),
             "Alive | Heap: %lu (Max Block: %lu) | Temp: %.1fC",
             (unsigned long)ESP.getFreeHeap(),
             (unsigned long)ESP.getMaxAllocHeap(), temperatureRead());
    LOG_INFO("HEARTBEAT", diagMsg);
#endif
  }

  // Turn off the heartbeat LED exactly 50ms after the pulse started
  if (ledPulsing && (currentMillis - heartbeatLedPulseTimer >= 50)) {
    BoardLED::off();
    ledPulsing = false;
  }

  appContext.tick();
  delay(1); // Yield to FreeRTOS to prevent IDLE1 starvation on Core 3.x
}

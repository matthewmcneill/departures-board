#ifndef __EMSCRIPTEN__
#include <ArduinoFake.h>
#undef F
#define F(s) (s)
using namespace fakeit;
#endif

const uint8_t u8g2_font_5x7_tr[] = { 0 };

#include "appContext.hpp"
#include <logger.hpp>
#include <timeManager.hpp>
#include <deviceCrypto.hpp>
#include <displayManager.hpp>
#include <dataManager.hpp>
#include "MockLittleFS.hpp"

// Forward Mocks that are not natively compiled
#include "WebServer.h"

#ifndef __EMSCRIPTEN__
void setupArduinoFake() {
    ArduinoFakeReset();
    When(OverloadedMethod(ArduinoFake(Serial), print, size_t(const char*))).AlwaysReturn(1);
    When(OverloadedMethod(ArduinoFake(Serial), print, size_t(const String&))).AlwaysReturn(1);
    When(OverloadedMethod(ArduinoFake(Serial), println, size_t(void))).AlwaysReturn(1);
    When(OverloadedMethod(ArduinoFake(Serial), println, size_t(const char*))).AlwaysReturn(1);
    When(OverloadedMethod(ArduinoFake(Serial), println, size_t(const String&))).AlwaysReturn(1);
}
#endif

// --- Logger Stub ---
#ifdef __EMSCRIPTEN__
void Logger::registerSecret(const String& secret) {}
#endif

// --- Global ESP32 time mock ---
bool getLocalTime(struct tm * info, uint32_t ms=5000) { return true; }
void configTime(long gmtOffset_sec, int daylightOffset_sec, const char* server1, const char* server2 = nullptr, const char* server3 = nullptr) {}

// --- DeviceCrypto Stub ---
#include "deviceCrypto.hpp"
void DeviceCrypto::begin() {}
void DeviceCrypto::applyPKCS7Padding(uint8_t* buffer, size_t originalLen, size_t paddedLen) {}
size_t DeviceCrypto::removePKCS7Padding(const uint8_t* buffer, size_t paddedLen) { return paddedLen; }
std::unique_ptr<char[]> DeviceCrypto::encrypt(const char* plaintext, size_t inputLen, size_t* outLen) {
    if (outLen) *outLen = inputLen;
    std::unique_ptr<char[]> buf(new char[inputLen + 1]);
    if (inputLen > 0) memcpy(buf.get(), plaintext, inputLen);
    buf.get()[inputLen] = '\0';
    return buf;
}
std::unique_ptr<char[]> DeviceCrypto::decrypt(const char* ciphertext, size_t inputLen, size_t* outLen) {
    if (outLen) *outLen = inputLen;
    std::unique_ptr<char[]> buf(new char[inputLen + 1]);
    if (inputLen > 0) memcpy(buf.get(), ciphertext, inputLen);
    buf.get()[inputLen] = '\0';
    return buf;
}



// --- WifiManager Stub ---

#include <webServer.hpp>
#include "WiFi.h"

MockWiFi WiFi;

class AsyncWebServer {};
class WebHandlerManager {};
WebServerManager::WebServerManager() {}
WebServerManager::~WebServerManager() {}
void WebServerManager::init() {}
void WebServerManager::updateCurrentWeather(float lat, float lon) {}

#include <otaUpdateManager.hpp>
otaUpdateManager::otaUpdateManager() {}
void otaUpdateManager::tick() {}
bool otaUpdateManager::checkForFirmwareUpdate() { return false; }
bool otaUpdateManager::checkUpdateAvailable(String& outVersion) { return false; }
bool otaUpdateManager::forceUpdateNow() { return false; }
void otaUpdateManager::rollbackFirmware() {}
void otaUpdateManager::markAppValid() {}
void otaUpdateManager::checkPostWebUpgrade() {}
void otaUpdateManager::reapplyConfig(const Config& config) {}

#include <buttonHandler.hpp>
buttonHandler::buttonHandler(unsigned char) {}
void buttonHandler::update() {}
bool buttonHandler::wasShortTapped() { return false; }
bool buttonHandler::wasLongTapped() { return false; }

#include <LittleFS.h>
LittleFSFS LittleFS;

extern "C" {
    void* xQueueCreate(uint32_t length, uint32_t itemSize) { return (void*)1; }
    long xQueueSendToFront(void* queue, const void* item, TickType_t ticksToWait) { return pdPASS; }
    long xQueueSend(void* queue, const void* item, TickType_t ticksToWait) { return pdPASS; }
    long xQueueReceive(void* queue, void* buffer, TickType_t ticksToWait) { return pdFAIL; }
    void* xSemaphoreCreateBinary() { return (void*)1; }
    void* xSemaphoreCreateMutex() { return (void*)1; }
    long xSemaphoreTake(void* semaphore, TickType_t ticksToWait) { return pdPASS; }
    long xSemaphoreGive(void* semaphore) { return pdPASS; }
    
    long xTaskCreatePinnedToCore(TaskFunction_t pvTaskCode, const char* const pcName, const uint32_t usStackDepth, void* const pvParameters, UBaseType_t uxPriority, TaskHandle_t* const pvCreatedTask, const BaseType_t xCoreID) {
        if (pvCreatedTask) *pvCreatedTask = (void*)1;
        return pdPASS;
    }
    void vTaskDelay(TickType_t xTicksToDelay) { }
}

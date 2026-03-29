#include <Arduino.h>
#include <logger.hpp>
#include <timeManager.hpp>
#include <wifiManager.hpp>
#include <appContext.hpp>

// --- Logger Stub ---
void Logger::log(const char* level, const char* module, const char* msg) {
    printf("[%s][%s] %s\n", level, module, msg);
}
void Logger::registerSecret(const char* secret) {}

// --- TimeManager Stub ---
const char* TimeManager::ukTimezone = "GMT0BST,M3.5.0/1,M10.5.0";

// --- WiFiManager Stub ---
WiFiManager::WiFiManager() {}
void WiFiManager::begin(appContext* context) {}
void WiFiManager::tick(unsigned long currentMillis) {}
bool WiFiManager::isConnected() { return true; }
String WiFiManager::getIpAddress() { return "127.0.0.1"; }

// --- AppContext Stub ---
appContext::appContext() : configManager(*this) {}
void appContext::begin() {}
void appContext::tick() {}
AppState appContext::getAppState() const { return AppState::RUNNING; }
void appContext::setAppState(AppState state) {}
ConfigManager& appContext::getConfigManager() { return configManager; }

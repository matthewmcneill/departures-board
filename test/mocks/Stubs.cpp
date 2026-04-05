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
appContext::appContext() : schedule(this) {}
void appContext::begin() {}
void appContext::tick() {}
AppState appContext::getAppState() const { return AppState::RUNNING; }
ConfigManager& appContext::getConfigManager() { static ConfigManager cm; return cm; }
TimeManager& appContext::getTimeManager() { static TimeManager tm; return tm; }
schedulerManager& appContext::getSchedulerManager() { return schedule; }
dataManager& appContext::getDataManager() { static dataManager dm; return dm; }
displayManager& appContext::getDisplayManager() { static displayManager dsm(*this); return dsm; }
wifiManager& appContext::getWifiManager() { static wifiManager wm; return wm; }
weatherClient& appContext::getWeather() { static weatherClient wc; return wc; }
rssClient& appContext::getRss() { static rssClient rc; return rc; }
MessagePool& appContext::getGlobalMessagePool() { static MessagePool mp(1); return mp; }
Portal& appContext::getPortal() { static Portal p; return p; }

// --- LittleFS Mock ---
#include <LittleFS.h>
#include <map>

class MockFile : public File {
    std::string _name;
    std::string _data;
    size_t _pos = 0;
public:
    MockFile(std::string name, std::string data) : _name(name), _data(data) {}
    size_t write(const uint8_t *buf, size_t size) override { return size; }
    int read() override { return (_pos < _data.length()) ? _data[_pos++] : -1; }
    void close() override {}
    size_t size() const override { return _data.length(); }
    operator bool() const override { return true; }
};

class MockFS : public fs::FS {
    std::map<std::string, std::string> _files;
public:
    bool begin(bool formatOnFail = false, const char *basePath = "/littlefs", uint8_t maxOpenFiles = 10, const char *partitionLabel = "spiffs") { return true; }
    void _setFile(std::string path, std::string data) { _files[path] = data; }
    File open(const char* path, const char* mode = "r") override {
        if (_files.count(path)) return MockFile(path, _files[path]);
        return File(); 
    }
    bool exists(const char* path) override { return _files.count(path) > 0; }
};

MockFS LittleFS;

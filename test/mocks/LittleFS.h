#pragma once
#include "FS.h"

class LittleFS_Mock : public FS {
public:
    bool begin(bool formatOnFail = false, const char* basePath = "/littlefs", uint8_t maxOpenFiles = 10, const char* partitionLabel = "spiffs") { return true; }
    size_t totalBytes() { return 1024 * 1024; }
    size_t usedBytes() { return 0; }
    void _setFile(const char* path, const char* content) { 
        mockFSMap[path] = content; 
    }
};

extern LittleFS_Mock LittleFS;

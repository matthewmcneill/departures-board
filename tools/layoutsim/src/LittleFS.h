#ifndef MOCK_LITTLEFS_H
#define MOCK_LITTLEFS_H

#include "FS.h"

class LittleFSFS : public fs::FS {
public:
    bool begin(bool formatOnFail = false, const char *basePath = "/littlefs", uint8_t maxOpenFiles = 10, const char *partitionLabel = "spiffs") {
        return true;
    }
};

extern LittleFSFS LittleFS;

#endif

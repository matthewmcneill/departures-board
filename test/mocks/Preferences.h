#pragma once
#include <Arduino.h>
#include <cstddef>
#include <cstring>

class Preferences {
public:
    Preferences() {}
    ~Preferences() {}

    bool begin(const char * name, bool readOnly=false, const char* partition_label=NULL) {
        (void)name; (void)readOnly; (void)partition_label;
        return true;
    }
    void end() {}

    size_t getBytesLength(const char* key) {
        (void)key;
        return 0; // Return 0 to emulate empty NVS, forcing key generation
    }

    size_t getBytes(const char* key, void * buf, size_t maxLen) {
        (void)key;
        if (buf && maxLen > 0) {
            memset(buf, 0, maxLen);
            return maxLen;
        }
        return 0;
    }

    size_t putBytes(const char* key, const void * value, size_t len) {
        (void)key; (void)value;
        return len;
    }

    bool putString(const char* key, const char* value) {
        (void)key; (void)value;
        return true;
    }

    const char* getString(const char* key, const char* defaultValue = "") {
        (void)key;
        return defaultValue;
    }
};

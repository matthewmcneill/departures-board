#ifndef FS_MOCK_H
#define FS_MOCK_H

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <map>

extern std::map<std::string, std::string> mockFSMap;

class File {
    std::string path;
public:
    File() {}
    File(std::string p) : path(p) {}
    operator bool() const { return true; }
    size_t write(const uint8_t* buf, size_t size) { return size; }
    size_t write(const char* s) { mockFSMap[path] += s; return 0; }
    void print(const char* s) { mockFSMap[path] += s; }
    void print(std::string s) { mockFSMap[path] += s; }
    std::string readString() { return mockFSMap[path]; }
    size_t read(uint8_t* buf, size_t size) { return 0; }
    void close() {}
};

class FS {
public:
    bool begin() { return true; }
    File open(const char* path, const char* mode = "r") { 
        if (mode[0] == 'w') mockFSMap[path] = ""; // Clear on write 
        return File(path); 
    }
    bool exists(const char* path) { return mockFSMap.find(path) != mockFSMap.end(); }
    bool remove(const char* path) { mockFSMap.erase(path); return true; }
    bool mkdir(const char* path) { return true; }
    bool rmdir(const char* path) { return true; }
};

#endif

/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 */

#ifndef MOCK_FS_H
#define MOCK_FS_H

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <map>
#include <vector>
#include "Arduino.h"

namespace fs {

enum SeekMode {
    SeekSet = 0,
    SeekCur = 1,
    SeekEnd = 2
};

class File : public Print {
public:
    File() : _content(nullptr), _pos(0), _valid(false) {}
    File(std::string* content) : _content(content), _pos(0), _valid(true) {}

    size_t write(uint8_t b) override {
        if (!_valid) return 0;
        if (_pos >= _content->size()) _content->push_back(b);
        else (*_content)[_pos] = b;
        _pos++;
        return 1;
    }

    size_t write(const uint8_t *buf, size_t size) override {
        if (!_valid) return 0;
        _content->append((const char*)buf, size);
        _pos += size;
        return size;
    }

    int read() {
        if (!_valid || _pos >= _content->size()) return -1;
        return (uint8_t)(*_content)[_pos++];
    }

    size_t read(uint8_t* buf, size_t size) {
        if (!_valid) return 0;
        size_t available = _content->size() - _pos;
        size_t toRead = (size < available) ? size : available;
        memcpy(buf, _content->data() + _pos, toRead);
        _pos += toRead;
        return toRead;
    }

    bool seek(uint32_t pos, SeekMode mode = SeekSet) {
        if (!_valid) return false;
        if (mode == SeekSet) _pos = pos;
        else if (mode == SeekCur) _pos += pos;
        else if (mode == SeekEnd) _pos = _content->size() - pos;
        return true;
    }

    size_t position() const { return _pos; }
    size_t size() const { return _valid ? _content->size() : 0; }
    void close() { _valid = false; }
    operator bool() const { return _valid; }

private:
    std::string* _content;
    size_t _pos;
    bool _valid;
};

class FS {
public:
    bool exists(const char* path) {
        return _files.find(path) != _files.end();
    }

    bool exists(const String& path) {
        return exists(path.c_str());
    }

    File open(const char* path, const char* mode = "r") {
        std::string sPath = path;
        if (mode[0] == 'w') {
            _files[sPath] = "";
            return File(&_files[sPath]);
        }
        if (exists(path)) {
            return File(&_files[sPath]);
        }
        return File();
    }

    File open(const String& path, const char* mode = "r") {
        return open(path.c_str(), mode);
    }

    bool remove(const char* path) {
        _files.erase(path);
        return true;
    }

    /** @brief Test-only method to inject a file. */
    void _setFile(const char* path, const char* content) {
        _files[path] = content;
    }

    /** @brief Test-only method to clear all files. */
    void _clear() {
        _files.clear();
    }

private:
    std::map<std::string, std::string> _files;
};

}

#endif

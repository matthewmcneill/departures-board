#include "MockLittleFS.hpp"
#include <FS.h>
#include <LittleFS.h>
#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>

const std::string BASE_NATIVE_PATH = ".agents/tmp/native_fs";

// Ensure the directory exists
void initMockLittleFS() {
    mkdir(".agents", 0777);
    mkdir(".agents/tmp", 0777);
    mkdir(BASE_NATIVE_PATH.c_str(), 0777);
}

std::string getHostPath(const char* localPath) {
    std::string path = localPath;
    if (!path.empty() && path[0] == '/') {
        path = path.substr(1);
    }
    return BASE_NATIVE_PATH + "/" + path;
}

class PosixFile : public fs::FileImpl {
private:
    FILE* _file = nullptr;
    DIR* _dir = nullptr;
    std::string _path;
    std::string _name;
    bool _isDir;

public:
    PosixFile(const char* path, const char* mode) {
        _path = getHostPath(path);
        _name = path;
        
        struct stat st;
        if (stat(_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            _isDir = true;
            _dir = opendir(_path.c_str());
        } else {
            _isDir = false;
            _file = fopen(_path.c_str(), mode);
        }
    }

    ~PosixFile() override {
        close();
    }

    size_t write(const uint8_t *buf, size_t size) override {
        if (!_file) return 0;
        return fwrite(buf, 1, size, _file);
    }

    size_t read(uint8_t* buf, size_t size) override {
        if (!_file) return 0;
        return fread(buf, 1, size, _file);
    }

    void flush() override {
        if (_file) fflush(_file);
    }

    bool seek(uint32_t pos, SeekMode mode) override {
        if (!_file) return false;
        int m = SEEK_SET;
        if (mode == SeekCur) m = SEEK_CUR;
        if (mode == SeekEnd) m = SEEK_END;
        return fseek(_file, pos, m) == 0;
    }

    size_t position() const override {
        if (!_file) return 0;
        return ftell(_file);
    }

    size_t size() const override {
        if (!_file) return 0;
        struct stat st;
        if (stat(_path.c_str(), &st) == 0) return st.st_size;
        return 0;
    }

    void close() override {
        if (_file) {
            fclose(_file);
            _file = nullptr;
        }
        if (_dir) {
            closedir(_dir);
            _dir = nullptr;
        }
    }

    operator bool() {
        return _file != nullptr || _dir != nullptr;
    }

    const char* name() const override {
        return _name.c_str();
    }

    bool isDirectory() const override {
        return _isDir;
    }

    std::shared_ptr<fs::FileImpl> openNextFile(const char* mode) override {
        if (!_dir) return nullptr;
        struct dirent* ent = readdir(_dir);
        while (ent != nullptr) {
            if (std::string(ent->d_name) != "." && std::string(ent->d_name) != "..") {
                std::string childPath = _name + (back()._name == '/' ? "" : "/") + ent->d_name;
                return std::make_shared<PosixFile>(childPath.c_str(), mode);
            }
            ent = readdir(_dir);
        }
        return nullptr;
    }

    void rewindDirectory() override {
        if (_dir) rewinddir(_dir);
    }

    bool isDirectory() override {
        return _isDir;
    }

    time_t getLastWrite() override {
        struct stat st;
        if (stat(_path.c_str(), &st) == 0) return st.st_mtime;
        return 0;
    }
};

class PosixFS : public fs::FSImpl {
public:
    std::shared_ptr<fs::FileImpl> open(const char* path, const char* mode, const bool create) override {
        initMockLittleFS();
        return std::make_shared<PosixFile>(path, mode);
    }

    bool exists(const char* path) override {
        struct stat st;
        return stat(getHostPath(path).c_str(), &st) == 0;
    }

    bool remove(const char* path) override {
        return ::remove(getHostPath(path).c_str()) == 0;
    }

    bool rename(const char* pathFrom, const char* pathTo) override {
        return ::rename(getHostPath(pathFrom).c_str(), getHostPath(pathTo).c_str()) == 0;
    }

    bool mkdir(const char *path) override {
        return ::mkdir(getHostPath(path).c_str(), 0777) == 0;
    }

    bool rmdir(const char *path) override {
        return ::rmdir(getHostPath(path).c_str()) == 0;
    }
};

// Global instance overriding the standard LittleFS
fs::FS LittleFS(std::make_shared<PosixFS>());


#ifndef MOCK_FS_H
#define MOCK_FS_H

#include <stddef.h>
#include <stdint.h>

namespace fs {
class FS {
public:
    bool exists(const char* path) { return false; }
};
}

#endif

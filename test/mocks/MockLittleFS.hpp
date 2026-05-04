#ifndef MOCK_LITTLE_FS_HPP
#define MOCK_LITTLE_FS_HPP

#include <FS.h>

// Initialize the POSIX filesystem directory structures natively.
// Call this at the start of any test suite that requires disk I/O.
void initMockLittleFS();

#endif // MOCK_LITTLE_FS_HPP

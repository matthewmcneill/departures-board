#ifndef PRINT_H
#define PRINT_H

#include <stddef.h>
#include <stdint.h>

class Print {
public:
    virtual size_t write(uint8_t) = 0;
    size_t write(const char *str) {
        if (str == NULL) return 0;
        return write((const uint8_t *)str, strlen(str));
    }
    virtual size_t write(const uint8_t *buffer, size_t size) {
        size_t n = 0;
        while (size--) {
            if (write(*buffer++)) n++;
            else break;
        }
        return n;
    }
    size_t print(const char *s) { return write(s); }
    size_t println(const char *s) { return print(s) + print("\n"); }
    size_t print(int n) { return 0; } // Dummy
    size_t println() { return print("\n"); }
};

#endif // PRINT_H

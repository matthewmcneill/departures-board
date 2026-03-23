/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: tools/layoutsim/src/Print.h
 * Description: Emscripten/WASM mock stub for the Arduino framework.
 */

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

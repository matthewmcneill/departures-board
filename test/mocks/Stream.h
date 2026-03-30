#ifndef STREAM_H
#define STREAM_H

#include "Print.h"
#include <stdint.h>

/**
 * @brief Mock implementation of the Arduino Stream class for native host testing. 
 */
class Stream : public Print {
public:
    virtual int available() = 0;
    virtual int read() = 0;
    virtual int peek() = 0;
    virtual void flush() = 0;

    size_t readBytes(char *buffer, size_t length) {
        size_t count = 0;
        while (count < length) {
            int c = read();
            if (c < 0) break;
            *buffer++ = (char)c;
            count++;
        }
        return count;
    }
    
    String readStringUntil(char terminator) {
        String ret = "";
        int c = read();
        while (c >= 0 && (char)c != terminator) {
            ret += (char)c;
            c = read();
        }
        return ret;
    }
};

#endif // STREAM_H

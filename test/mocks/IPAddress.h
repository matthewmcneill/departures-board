#ifndef IPADDRESS_MOCK_H
#define IPADDRESS_MOCK_H

#include "Arduino.h"

/**
 * @brief Mock implementation of IPAddress for native host testing. 
 */
class IPAddress {
public:
    IPAddress() { memset(ip, 0, 4); }
    IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) { ip[0]=a; ip[1]=b; ip[2]=c; ip[3]=d; }
    uint8_t ip[4];
    uint8_t operator[](int i) const { return ip[i]; }
    uint8_t& operator[](int i) { return ip[i]; }
    String toString() const { return String((unsigned int)ip[0])+"."+String((unsigned int)ip[1])+"."+String((unsigned int)ip[2])+"."+String((unsigned int)ip[3]); }
};

#endif // IPADDRESS_MOCK_H

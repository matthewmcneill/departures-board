#ifndef NETWORK_MOCK_H
#define NETWORK_MOCK_H

#include "Arduino.h"

/**
 * @brief Mock implementation of ESP32 Network status for native host testing. 
 */
enum wl_status_t {
    WL_IDLE_STATUS = 0,
    WL_NO_SSID_AVAIL,
    WL_SCAN_COMPLETED,
    WL_CONNECTED,
    WL_CONNECT_FAILED,
    WL_CONNECTION_LOST,
    WL_DISCONNECTED
};

#endif

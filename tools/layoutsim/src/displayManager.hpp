#ifndef DISPLAY_MANAGER_MOCK_HPP
#define DISPLAY_MANAGER_MOCK_HPP

#include <U8g2lib.h>

class DisplayManager {
public:
    void yieldAnimationUpdate() {}
};

extern DisplayManager displayManager;

// Mock constants usually found in display headers
#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 64

#endif

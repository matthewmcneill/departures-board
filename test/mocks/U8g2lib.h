#ifndef MOCK_U8G2_H
#define MOCK_U8G2_H

#include <stdint.h>
#include <vector>
#include <string>

#define U8G2_R0 0
#define U8X8_PIN_NONE 255

#ifndef U8G2_FONT_SECTION
#define U8G2_FONT_SECTION(name) 
#endif

// --- Mock U8G2 Types ---
typedef uint16_t u8g2_uint_t;
typedef uint8_t u8g2_cb_t;

typedef struct u8g2_struct {
    const uint8_t* font;
    uint8_t draw_color;
    u8g2_uint_t clip_x0, clip_y0, clip_x1, clip_y1;
} u8g2_t;

/**
 * @brief Mock U8G2 display class for pixel-level unit testing and simulator UI.
 */
class U8G2 {
public:
    struct DrawAction {
        int x, y, w, h;
        std::string text;
    };
    std::vector<DrawAction> drawHistory;

    U8G2() {
        u8g2.draw_color = 1;
        u8g2.clip_x0 = 0; u8g2.clip_y0 = 0;
        u8g2.clip_x1 = 255; u8g2.clip_y1 = 63;
    }
    ~U8G2() {}

    void begin() {}
    void setFont(const uint8_t *font) { u8g2.font = font; }
    void setFontMode(uint8_t mode) {}
    void setFontRefHeightAll() {}
    void setFontPosTop() {}
    void clearDisplay() {}
    void clearBuffer() {}
    void setPowerSave(uint8_t is_enable) {}
    void setContrast(uint8_t value) {}
    void setFlipMode(uint8_t mode) {}
    void clearHistory() { drawHistory.clear(); }

    // Mock Methods
    void setDrawColor(uint8_t c) { u8g2.draw_color = c; }
    void setClipWindow(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t x1, u8g2_uint_t y1) {
        u8g2.clip_x0 = x0; u8g2.clip_y0 = y0; u8g2.clip_x1 = x1; u8g2.clip_y1 = y1;
    }

    u8g2_uint_t getDisplayWidth() { return 256; }
    u8g2_uint_t getDisplayHeight() { return 64; }
    
    int drawStr(int x, int y, const char* s) { 
        drawHistory.push_back({x, y, 0, 0, s}); 
        return strlen(s) * 5; // Simplified width
    }
    void drawBox(int x, int y, int w, int h) { drawHistory.push_back({x, y, w, h, "BOX"}); }
    void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2) { drawHistory.push_back({x0, y0, 0, 0, "TRIANGLE"}); }

    int getStrWidth(const char* s) { return strlen(s) * 5; }
    int getUTF8Width(const char* s) { return getStrWidth(s); }
    void drawUTF8(int x, int y, const char* s) { drawHistory.push_back({x, y, 0, 0, s}); }
    void drawLine(int x0, int y0, int x1, int y1) {}
    int getMaxCharHeight() { return 10; }
    
    void drawXBM(int x, int y, int w, int h, const uint8_t* bitmap) {}
    int getAscent() { return 10; }
    int getDescent() { return -2; }
    void drawHVLine(int x, int y, int l, int dir) {}
    void drawRFrame(int x, int y, int w, int h, int r) {}
    void drawFrame(int x, int y, int w, int h) {}

    void sendBuffer() {}
    
    u8g2_t* getU8g2() { return &_u8g2; }
    
    const uint8_t* getBufferPtr() { return nullptr; }
    size_t getBufferTileHeight() { return 8; }
    size_t getBufferTileWidth() { return 32; }

    void updateDisplayArea(int x, int y, int w, int h) {}
private:
    u8g2_t _u8g2;
    u8g2_t u8g2;
};

// Driver classes
class U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI : public U8G2 {
public:
    U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI(const u8g2_cb_t *rotation, uint8_t dc, uint8_t reset, uint8_t cs) : U8G2() {}
};

class U8G2_SH1122_256X64_F_4W_SW_SPI : public U8G2 {
public:
    U8G2_SH1122_256X64_F_4W_SW_SPI(const u8g2_cb_t *rotation, uint8_t clock, uint8_t data, uint8_t cs, uint8_t dc, uint8_t reset = 255) : U8G2() {}
};

extern const uint8_t u8g2_font_5x7_tr[];

#endif // MOCK_U8G2_H

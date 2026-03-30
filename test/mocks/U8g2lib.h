#ifndef U8G2LIB_MOCK_H
#define U8G2LIB_MOCK_H

#include <stdint.h>
#include <stddef.h>
#include <string>

#ifndef U8G2_FONT_SECTION
#define U8G2_FONT_SECTION(name)
#endif

#define U8G2_R0 0

typedef uint8_t u8g2_uint_t;

struct u8g2_t {
    u8g2_uint_t clip_x0, clip_y0, clip_x1, clip_y1;
    const uint8_t* font;
    uint8_t draw_color;
}; // Dummy struct for mock

class U8G2 {
public:
    void begin() {}
    void clearBuffer() {}
    void sendBuffer() {}
    void setFont(const uint8_t* font) {}
    void setFontPosTop() {}
    void drawStr(int x, int y, const char* s) {}
    void setDrawColor(int color) {}
    void setContrast(int contrast) {}
    void setPowerSave(int save) {}
    void setDisplayRotation(int rot) {}
    int getStrWidth(const char* s) { return 0; }
    void drawXBM(int x, int y, int w, int h, const uint8_t* bitmap) {}
    void drawFrame(int x, int y, int w, int h) {}
    void drawBox(int x, int y, int w, int h) {}
    void drawLine(int x1, int y1, int x2, int y2) {}
    void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2) {}
    void setClipWindow(int x0, int y0, int x1, int y1) {}
    void drawPixel(int x, int y) {}
    void updateDisplayArea(int x_tile, int y_tile, int w_tile, int h_tile) {}
    int getMaxCharHeight() { return 10; }
    int getAscent() { return 8; }
    int getDescent() { return -2; }
    uint8_t* getBufferPtr() { static uint8_t buf[8192]; return buf; }
    u8g2_t* getU8g2() { static u8g2_t dummy; return &dummy; }
};

class U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI : public U8G2 {
public:
    U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI(int rot, int cs, int dc, int reset = -1) {}
};

class U8G2_SH1122_256X64_F_4W_SW_SPI : public U8G2 {
public:
    U8G2_SH1122_256X64_F_4W_SW_SPI(int rot, int clock, int data, int cs, int dc, int reset = -1) {}
};

#endif

#include <unity.h>
#include <drawingPrimitives.hpp>

void test_drawText_fast_path() {
    U8G2 display;
    display.clearHistory();
    
    // Fast-path: align=LEFT, truncate=false, w=-1, h=-1
    drawText(display, "Hello", 10, 20, -1, -1, TextAlign::LEFT, false, nullptr);
    
    TEST_ASSERT_EQUAL(1, (int)display.drawHistory.size());
    TEST_ASSERT_EQUAL(10, display.drawHistory[0].x);
    TEST_ASSERT_EQUAL(20, display.drawHistory[0].y);
    TEST_ASSERT_EQUAL_STRING("Hello", display.drawHistory[0].text.c_str());
}

void test_drawText_centered() {
    U8G2 display;
    display.clearHistory();
    
    // Width 100, "Hello" (5 chars * 5 px = 25px)
    // Centered: 10 + (100 - 25)/2 = 10 + 37 = 47
    drawText(display, "Hello", 10, 20, 100, 10, TextAlign::CENTER, false, nullptr);
    
    TEST_ASSERT_EQUAL(1, (int)display.drawHistory.size());
    TEST_ASSERT_EQUAL(47, display.drawHistory[0].x);
}

void test_drawText_truncation() {
    U8G2 display;
    display.clearHistory();
    
    // Max width 30. "HelloWorldLongText" (18 chars * 5 px = 90px)
    // "..." (3 chars * 5 px = 15px)
    // Space for text: 30 - 15 = 15px (which is 3 characters)
    // Expected result: "Hel..."
    drawText(display, "HelloWorldLongText", 0, 0, 30, 10, TextAlign::LEFT, true, nullptr);
    
    TEST_ASSERT_EQUAL(1, (int)display.drawHistory.size());
    // In our mock, char width is 5.
    // Length 3*5 = 15. Ellipsis width 3*5 = 15. Total 30.
    TEST_ASSERT_EQUAL_STRING("Hel...", display.drawHistory[0].text.c_str());
}

void runDrawingPrimitivesTests() {
    RUN_TEST(test_drawText_fast_path);
    RUN_TEST(test_drawText_centered);
    RUN_TEST(test_drawText_truncation);
}

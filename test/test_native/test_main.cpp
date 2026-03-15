#include <unity.h>

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_simple_assertion(void) {
    TEST_ASSERT_EQUAL(32, 32);
}

void test_another_assertion(void) {
    TEST_ASSERT_TRUE(true);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_simple_assertion);
    RUN_TEST(test_another_assertion);
    return UNITY_END();
}

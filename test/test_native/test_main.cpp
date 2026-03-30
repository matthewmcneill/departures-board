#include <unity.h>
#include <Arduino.h>

// Forward declarations of test runners
void runConfigManagerTests();
void runDataManagerTests();

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    runConfigManagerTests();
    runDataManagerTests();
    
    return UNITY_END();
}

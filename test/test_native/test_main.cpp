#include <unity.h>
#include <Arduino.h>

// --- Mocks & Stubs ---
#include "../mocks/Stubs.cpp"

// --- Module Implementations ---
#include "../../modules/configManager/configManager.cpp"
#include "../../modules/dataManager/dataManager.cpp"
#include "../../modules/schedulerManager/schedulerManager.cpp"
#include "../../modules/displayManager/widgets/drawingPrimitives.cpp"

// Forward declarations of test runners
void runConfigManagerTests();
void runDataManagerTests();
void runSchedulerManagerTests();
void runDrawingPrimitivesTests();

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    runConfigManagerTests();
    runDataManagerTests();
    runSchedulerManagerTests();
    runDrawingPrimitivesTests();
    
    return UNITY_END();
}

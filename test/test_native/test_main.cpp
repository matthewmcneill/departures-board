#include <unity.h>
#include <Arduino.h>
#include <ArduinoFake.h>

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

// --- Mocks & Stubs ---
#include "../mocks/Stubs.cpp"

// --- Module Implementations ---
#include "../../modules/configManager/gadecMigration.cpp"
#include "../../modules/configManager/configManager.cpp"
#include "../../modules/dataManager/dataManager.cpp"
#include "../../modules/schedulerManager/schedulerManager.cpp"
#include "../../modules/timeManager/timeManager.cpp"
#include "../../modules/displayManager/displayManager.cpp"
#include "../../modules/displayManager/boardFactory.cpp"
#include "../../modules/appContext/appContext.cpp"


// Widgets, Fonts & Formatting
#include "../../modules/displayManager/fonts/fonts.cpp"
#include "../../modules/displayManager/widgets/drawingPrimitives.cpp"
#include "../../modules/displayManager/widgets/wifiStatusWidget.cpp"

// System Boards
#include "../../modules/displayManager/boards/systemBoard/splashBoard.cpp"
#include "../../modules/displayManager/boards/systemBoard/wizardBoard.cpp"
#include "../../modules/displayManager/boards/systemBoard/loadingBoard.cpp"
#include "../../modules/displayManager/boards/systemBoard/sleepingBoard.cpp"
#include "../../modules/displayManager/boards/systemBoard/diagnosticBoard.cpp"
#include "../../modules/displayManager/boards/systemBoard/messageBoard.cpp"
#include "../../modules/displayManager/boards/systemBoard/firmwareUpdateBoard.cpp"
#include "../../modules/displayManager/boards/systemBoard/helpBoard.cpp"

// Source Boards
#include "../../modules/displayManager/boards/nationalRailBoard/nationalRailBoard.cpp"
#include "../../modules/displayManager/boards/tflBoard/tflBoard.cpp"
#include "../../modules/displayManager/boards/tflBoard/tflDataSource.cpp"
#include "../../modules/displayManager/boards/busBoard/busBoard.cpp"
#include "../../modules/displayManager/boards/busBoard/busDataSource.cpp"

// Common libs
#include "../../modules/displayManager/messaging/messagePool.cpp"
#include "../../.pio/libdeps/unit_testing_host/JsonStreamingParser/JsonStreamingParser.cpp"
#include "../../lib/xmlStreamingParser/xmlStreamingParser.cpp"

// Widgets
#include "../../modules/displayManager/widgets/imageWidget.cpp"
#include "../../modules/displayManager/widgets/labelWidget.cpp"
#include "../../modules/displayManager/widgets/progressBarWidget.cpp"
#include "../../modules/displayManager/widgets/serviceListWidget.cpp"
#include "../../modules/displayManager/widgets/scrollingTextWidget.cpp"
#include "../../modules/displayManager/widgets/trainFormationWidget.cpp"
#include "../../modules/displayManager/widgets/locationAndFiltersWidget.cpp"
#include "../../modules/displayManager/widgets/scrollingMessagePoolWidget.cpp"
#include "../../modules/displayManager/widgets/clockWidget.cpp"
#include "../../modules/displayManager/widgets/weatherWidget.cpp"

// Layouts & Providers
#include "../../modules/displayManager/boards/nationalRailBoard/iNationalRailLayout.cpp"
#include "../../modules/displayManager/boards/busBoard/iBusLayout.cpp"
#include "../../modules/displayManager/boards/tflBoard/iTflLayout.cpp"
#include "../../modules/displayManager/boards/nationalRailBoard/layouts/layoutSWR.cpp"
#include "../../modules/displayManager/boards/nationalRailBoard/layouts/layoutGadec.cpp"
#include "../../modules/displayManager/boards/nationalRailBoard/layouts/layoutDefault.cpp"
#include "../../modules/displayManager/boards/nationalRailBoard/layouts/layoutReplica.cpp"
#include "../../modules/displayManager/boards/busBoard/layouts/layoutDefault.cpp"
#include "../../modules/displayManager/boards/tflBoard/layouts/layoutDefault.cpp"
#include "../../modules/displayManager/boards/systemBoard/layouts/layoutDiagnostic.cpp"
#include "../../modules/displayManager/boards/nationalRailBoard/nrRDMDataProvider.cpp"
#include "../../modules/displayManager/boards/nationalRailBoard/nrDARWINDataProvider.cpp"

// Forward declarations of test runners
void runConfigManagerTests();
void runDataManagerTests();
void runSchedulerManagerTests();
void runDrawingPrimitivesTests();

// Satisfy global references from unmodified providers
class appContext appContext{};
class appContext g_appContext{};

void setUp(void) {
    // set stuff up here
#ifndef __EMSCRIPTEN__
    setupArduinoFake();
#endif
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

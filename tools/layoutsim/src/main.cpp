/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: tools/layoutsim/src/main.cpp
 * Description: Main WebAssembly entry point for the layout engine. Exposes initialization, layout parsing, and frame rendering APIs to JavaScript.
 *
 * Exported Functions/Classes:
 * - initEngine(): Initializes the simulator dependencies
 * - syncData(): Synchronizes generic mock data to all active widgets
 * - applyLayout(): Compiles a JSON layout description within the IDE
 * - applyMockData(): Updates the global mock data singleton from JSON payload
 * - setDebugMode(): Toggles debug diagnostic outputs
 * - getLayoutMetadata(): Returns JSON diagnostics for IDE bounding boxes
 * - renderFrame(): Paints a single IDE frame to an RGBA buffer
 */

#include <emscripten.h>
#include <emscripten/bind.h>
#include <iostream>
#include <vector>
#include "U8g2lib.h"
#include "headerWidget.hpp"
#include "serviceListWidget.hpp"
#include "scrollingMessagePoolWidget.hpp"
#include "labelWidget.hpp"
#include "mockDataManager.hpp"
#include "timeManager.hpp"
#include "layoutParser.hpp"
#include "designerRegistry.hpp"
#include "generated_registry.hpp"

// OLED dimensions
const int OLED_WIDTH = 256; // Standard physical width of the display array
const int OLED_HEIGHT = 64; // Standard physical height of the display array

// Simulation state
U8G2_SSD1322_NHD_256X64_F_4W_SW_SPI *g_u8g2; // Global mock graphics context pointer
TimeManager *g_timeMgr; // Global mock time manager instance
LayoutParser *g_layoutParser; // Global IDE JSON layout parser engine
MessagePool *g_msgPool; // Global scrolling message buffer

// Memory buffer for JavaScript to read
uint8_t rgba_buffer[OLED_WIDTH * OLED_HEIGHT * 4]; // Persistent RGBA pixel buffer exposed to Html5 canvas

extern "C" {

/**
 * @brief Initializes the simulator dependencies
 */
EMSCRIPTEN_KEEPALIVE
void initEngine() {
    // Setup a dummy U8g2 instance
    g_u8g2 = new U8G2_SSD1322_NHD_256X64_F_4W_SW_SPI(U8G2_R0, /* clock=*/ 0, /* data=*/ 0, /* cs=*/ 0, /* dc=*/ 0, /* reset=*/ 0);
    g_u8g2->begin();
    
    // Initialize dependencies
    g_timeMgr = new TimeManager();
    g_layoutParser = new LayoutParser();
    g_msgPool = new MessagePool(10);
    
    // We no longer instantiate widgets here. The layout parser will invoke
    // loadLayoutProfile() when it parses the JSON `"layout"` field.
}

/**
 * @brief Synchronizes generic mock data to all active widgets
 */
void syncData() {
    auto& mdm = MockDataManager::getInstance();
    
    // --- Step 1: Sync Header ---
    iGfxWidget* header = DesignerRegistry::getInstance().getWidget("headWidget");
    if (header) {
        ((headerWidget*)header)->setTitle(mdm.getStationTitle());
        ((headerWidget*)header)->setCallingPoint(mdm.getStationCalling());
        ((headerWidget*)header)->setPlatform(mdm.getStationPlatform());
    }
    
    // --- Step 2: Sync Services ---
    iGfxWidget* services = DesignerRegistry::getInstance().getWidget("servicesWidget");
    if (services) {
        ((serviceListWidget*)services)->clearRows();
        for (int i = 0; i < mdm.getServiceCount(); i++) {
            const auto& s = mdm.getService(i);
            const char* row[5] = { s.ordinal, s.time, s.destination, s.platform, s.status };
            ((serviceListWidget*)services)->addRow(row);
        }
    }
    
    iGfxWidget* row0 = DesignerRegistry::getInstance().getWidget("row0Widget");
    if (row0) {
        ((serviceListWidget*)row0)->clearRows();
        if (mdm.getServiceCount() > 0) {
            const auto& s = mdm.getService(0);
            const char* row[5] = { s.ordinal, s.time, s.destination, s.platform, s.status };
            ((serviceListWidget*)row0)->addRow(row);
        }
    }
    
    // --- Step 3: Sync Specialized Rows ---
    iGfxWidget* row0Time = DesignerRegistry::getInstance().getWidget("row0Time");
    iGfxWidget* row0Dest = DesignerRegistry::getInstance().getWidget("row0Dest");
    if (row0Time && row0Dest && mdm.getServiceCount() > 0) {
        const auto& s = mdm.getService(0);
        ((scrollingTextWidget*)row0Time)->setText(s.time);
        ((scrollingTextWidget*)row0Dest)->setText(s.destination);
    }
    
    // --- Step 4: Sync Messages ---
    // MessagePool is shared globally across the simulation via generated_registry
    g_msgPool->clear();
    for (int i = 0; i < mdm.getMessageCount(); i++) {
        g_msgPool->addMessage(mdm.getMessage(i));
    }
}

/**
 * @brief Compiles a JSON layout description within the IDE
 * @param json A layout definition string passed from the JS engine
 * @return Const char string denoting parser state (e.g. error message) or nullptr on success
 */
EMSCRIPTEN_KEEPALIVE
const char* applyLayout(const char* json) {
    if (g_layoutParser) {
        const char* result = g_layoutParser->parse(json, g_timeMgr, g_msgPool);
        
        // When the layout is applied, the active profile INSTANTIATES new 
        // widget pointers. We must re-hydrate them with the latest known mock data.
        syncData();
        
        return result;
    }
    return "Parser error";
}

/**
 * @brief Updates the global mock data singleton from JSON payload
 * @param json A payload string describing simulated station state
 */
EMSCRIPTEN_KEEPALIVE
void applyMockData(const char* json) {
    MockDataManager::getInstance().parse(json);
    syncData();
}

static bool debugMode = false; // Flag governing browser console printing
static char metadataBuffer[2048]; // Serialisation cache for IDE layout responses

/**
 * @brief Toggles debug diagnostic outputs
 * @param enabled True to print additional debug to the browser console
 */
EMSCRIPTEN_KEEPALIVE
void setDebugMode(bool enabled) {
    debugMode = enabled;
}

/**
 * @brief Returns JSON diagnostics for IDE bounding boxes
 * @return JSON string of component statuses
 */
EMSCRIPTEN_KEEPALIVE
const char* getLayoutMetadata() {
    JsonDocument doc;
    JsonArray widgetsArr = doc["widgets"].to<JsonArray>();
    
    if (g_layoutParser) {
        auto const& validation = g_layoutParser->getValidationStatus();
        for (auto const& [name, status] : validation) {
            JsonObject w = widgetsArr.add<JsonObject>();
            w["id"] = name.c_str();
            w["validation_status"] = status.c_str();
            
            if (status != "unmapped") {
                iGfxWidget* widget = DesignerRegistry::getInstance().getWidget(name);
                if (widget) {
                    JsonObject geom = w.createNestedObject("geometry");
                    geom["x"] = widget->getX();
                    geom["y"] = widget->getY();
                    geom["w"] = widget->getW();
                    geom["h"] = widget->getH();
                }
            }
        }
    }
    
    serializeJson(doc, metadataBuffer);
    return metadataBuffer;
}

/**
 * @brief Paints a single IDE frame to an RGBA buffer
 * @param currentMillis Current mock execution time elapsed
 * @return Pointer to the raw RGBA pixel memory segment
 */
EMSCRIPTEN_KEEPALIVE
uint8_t* renderFrame(uint32_t currentMillis) {
    if (!g_u8g2) return nullptr;

    // --- Step 1: Initialize Buffer ---
    g_u8g2->clearBuffer();
    
    // --- Step 2: Render Layout Primitives ---
    // Draw static background details from the JSON definition
    if (g_layoutParser) {
        g_layoutParser->render(*g_u8g2);
    }

    // --- Step 3: Update Logic & Render Widgets ---
    tickActiveProfile(currentMillis);
    renderActiveProfile(*g_u8g2);
    
    // --- Step 4: Convert U8G2 1-bit Buffer to RGBA Canvas Format ---
    uint8_t *internal_buf = g_u8g2->getBufferPtr();
    
    for (int y = 0; y < OLED_HEIGHT; y++) {
        for (int x = 0; x < OLED_WIDTH; x++) {
            // Standard U8g2 tile mapping (vertical tiles)
            // Each byte contains 8 vertical pixels.
            int tile_y = y / 8;
            int bit_y = y % 8;
            int index = tile_y * OLED_WIDTH + x;
            bool pixel = (internal_buf[index] >> bit_y) & 0x01;
            
            int base = (y * OLED_WIDTH + x) * 4;
            uint8_t val = pixel ? 255 : 0;
            rgba_buffer[base + 0] = val; // R
            rgba_buffer[base + 1] = val; // G
            rgba_buffer[base + 2] = val; // B
            rgba_buffer[base + 3] = 255; // A
        }
    }
    
    return rgba_buffer;
}

}

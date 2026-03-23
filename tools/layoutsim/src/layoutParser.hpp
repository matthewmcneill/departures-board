/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: tools/layoutsim/src/layoutParser.hpp
 * Description: Parses JSON layout definitions and manages drawing primitives for the simulator IDE.
 *
 * Exported Functions/Classes:
 * - PrimitiveType: Enumeration of supported primitive draw shapes
 * - DesignerPrimitive: Structure defining a raw graphic operation
 * - LayoutParser: Engine for evaluating custom JSON descriptions and driving the UI component registry
 *   - getValidationStatus(): Returns diagnostic information regarding layout properties
 *   - parse(): Ingests JSON documents and generates component map updates
 *   - render(): Dispatches primitive rendering operations to the active display manager
 */

#ifndef LAYOUT_PARSER_HPP
#define LAYOUT_PARSER_HPP

#include <ArduinoJson.h>
#include <vector>
#include <string>
#include <map>
#include "designerRegistry.hpp"
#include "generated_registry.hpp"
#include "drawingPrimitives.hpp"
#include "serviceListWidget.hpp"
#include "clockWidget.hpp"
#include "labelWidget.hpp"
#include "fonts.hpp"

enum class PrimitiveType {
    LINE,
    BOX,
    FRAME,
    TEXT
};

struct DesignerPrimitive {
    PrimitiveType type;
    int x1, y1, x2, y2; // Also used for x, y, w, h
    std::string text;
    const uint8_t* font;
};

class LayoutParser {
private:
    std::vector<DesignerPrimitive> primitives;
    std::map<std::string, std::string> validationStatus;

    /**
     * @brief Maps a string name to its respective font byte array
     * @param name The font name literal referenced in JSON
     * @return Pointer to the U8g2 compatible byte array, or nullptr
     */
    const uint8_t* getFontByName(const std::string& name) {
        if (name == "NatRailSmall9") return NatRailSmall9;
        if (name == "NatRailTall12") return NatRailTall12;
        if (name == "NatRailClockSmall7") return NatRailClockSmall7;
        if (name == "NatRailClockLarge9") return NatRailClockLarge9;
        if (name == "Underground10") return Underground10;
        if (name == "UndergroundClock8") return UndergroundClock8;
        return nullptr;
    }

public:
    /**
     * @brief Returns diagnostic information regarding layout properties
     * @return Map of string IDs to their current semantic state
     */
    const std::map<std::string, std::string>& getValidationStatus() const {
        return validationStatus;
    }

    /**
     * @brief Parse a JSON layout string and update all registered widgets.
     * @param json The JSON string to parse.
     * @param tm TimeManager pointer for clock widgets 
     * @param mp MessagePool pointer for scrolling widgets
     * @return const char* Returns nullptr on success, or an error message on failure.
     */
    const char* parse(const char* json, TimeManager* tm, MessagePool* mp) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, json);
        if (error) return error.c_str();

        validationStatus.clear();

        // --- Step 1: Dynamic Profile Loading ---
        // Retrieve the top-level layout class and instantiate its respective mock profile
        const char* layoutName = doc["layout"];
        if (layoutName) {
            loadLayoutProfile(layoutName, tm, mp);
        }

        // --- Step 2: Invalidate Existent Components ---
        // Mark all registered widgets as missing to track unused definitions
        auto const& widgetMap = DesignerRegistry::getInstance().getAllWidgets();
        for (auto const& [name, widget] : widgetMap) {
            validationStatus[name] = "missing_style";
        }

        // --- Step 3: Parse Widgets & Validate ---
        // Traverse the JSON widget array and apply state/metrics to matched registry objects
        JsonArray widgets = doc["widgets"];
        for (JsonObject w : widgets) {
            const char* id = w["id"];
            if (!id) continue;
            
            iGfxWidget* widget = DesignerRegistry::getInstance().getWidget(id);
            if (widget) {
                validationStatus[id] = "valid";
                JsonObject geom = w["geometry"];
                if (!geom.isNull()) {
                    int x = geom["x"] | -1;
                    int y = geom["y"] | -1;
                    int width = geom["w"] | -1;
                    int height = geom["h"] | -1;
                    widget->setCoords(x, y, width, height);
                }
                if (w["visible"].is<bool>()) {
                    widget->setVisible(w["visible"].as<bool>());
                }

                if (w["font"].is<const char*>()) {
                    const uint8_t* f = getFontByName(w["font"].as<const char*>());
                    if (f) {
                        std::string idStr = id ? id : "";
                        if (idStr == "label" || idStr.compare(0, 4, "row0") == 0) {
                            ((labelWidget*)widget)->setFont(f);
                        } else if (idStr == "clock" || idStr == "sysClock") {
                            ((clockWidget*)widget)->setFont(f);
                        }
                    }
                }

                if (w["text"].is<const char*>()) {
                    std::string idStr = id ? id : "";
                    if (idStr == "label" || idStr.compare(0, 4, "row0") == 0 || idStr == "noDataLabel") {
                        ((labelWidget*)widget)->setText(w["text"].as<const char*>());
                    }
                }

                if (w["blink"].is<bool>()) {
                    std::string idStr = id ? id : "";
                    if (idStr == "clock" || idStr == "sysClock") {
                        ((clockWidget*)widget)->setBlink(w["blink"].as<bool>());
                    }
                }

                // Special case for serviceListWidget configuration
                if (w["type"].is<const char*>() && strcmp(w["type"].as<const char*>(), "serviceListWidget") == 0) {
                    serviceListWidget* sw = (serviceListWidget*)widget;
                    
                    if (w["skipRows"].is<int>() || w["maxRows"].is<int>()) {
                        int skip = w["skipRows"] | 0;
                        int max = w["maxRows"] | -1;
                        sw->setDataLimits(skip, max);
                    }

                    if (w["scrollDurationMs"].is<int>()) {
                        sw->setScrollDuration(w["scrollDurationMs"].as<int>());
                    }
                    if (w["scrollDwellMs"].is<int>()) {
                        sw->setScrollDwell(w["scrollDwellMs"].as<int>());
                    }

                    if (w["columns"].is<JsonArray>()) {
                        JsonArray cols = w["columns"];
                        ColumnDef defs[MAX_SERVICE_COLUMNS];
                        int i = 0;
                        for (JsonObject c : cols) {
                            if (i >= MAX_SERVICE_COLUMNS) break;
                            defs[i].width = c["width"] | 20;
                            defs[i].align = c["align"] | 0;
                            i++;
                        }
                        sw->setColumns(i, defs);
                    }
                }
            } else {
                validationStatus[id] = "unmapped";
            }
        }

        // --- Step 4: Parse Background Primitives ---
        // Clear old primitives and ingest raw draw instructions like lines and boxes
        primitives.clear();
        JsonArray prims = doc["primitives"];
        for (JsonObject p : prims) {
            const char* typeStr = p["type"];
            if (!typeStr) continue;

            DesignerPrimitive prim;
            JsonObject geom = p["geometry"];
            if (strcmp(typeStr, "line") == 0) {
                prim.type = PrimitiveType::LINE;
                prim.x1 = geom["x1"] | 0;
                prim.y1 = geom["y1"] | 0;
                prim.x2 = geom["x2"] | 0;
                prim.y2 = geom["y2"] | 0;
            } else if (strcmp(typeStr, "box") == 0) {
                prim.type = PrimitiveType::BOX;
                prim.x1 = geom["x"] | 0;
                prim.y1 = geom["y"] | 0;
                prim.x2 = geom["w"] | 0;
                prim.y2 = geom["h"] | 0;
            } else if (strcmp(typeStr, "text") == 0) {
                prim.type = PrimitiveType::TEXT;
                prim.x1 = geom["x"] | 0;
                prim.y1 = geom["y"] | 0;
                prim.text = p["text"] | "";
                prim.font = getFontByName(p["font"] | "");
            }
            primitives.push_back(prim);
        }
        return nullptr;
    }

    /**
     * @brief Dispatches primitive rendering operations to the active display manager
     * @param display The underlying graphics context to write buffer pixels to
     */
    void render(U8G2& display) {
        for (const auto& prim : primitives) {
            switch (prim.type) {
                case PrimitiveType::LINE:
                    display.drawLine(prim.x1, prim.y1, prim.x2, prim.y2);
                    break;
                case PrimitiveType::BOX:
                    display.drawBox(prim.x1, prim.y1, prim.x2, prim.y2);
                    break;
                case PrimitiveType::TEXT:
                    if (prim.font) display.setFont(prim.font);
                    display.drawStr(prim.x1, prim.y1, prim.text.c_str());
                    break;
                default: break;
            }
        }
    }
};

#endif // LAYOUT_PARSER_HPP

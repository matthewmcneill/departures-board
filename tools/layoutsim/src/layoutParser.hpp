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
    const std::map<std::string, std::string>& getValidationStatus() const {
        return validationStatus;
    }

    const char* parse(const char* json, TimeManager* tm, MessagePool* mp) {
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, json);
        if (error) return error.c_str();

        validationStatus.clear();

        const char* layoutName = doc["layout"];
        if (layoutName) {
            loadLayoutProfile(layoutName, tm, mp);
        }

        auto const& widgetMap = DesignerRegistry::getInstance().getAllWidgets();
        for (auto const& [name, widget] : widgetMap) {
            validationStatus[name] = "missing_style";
        }

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
                        const char* typeStrPtr = w["type"];
                        std::string typeStr = typeStrPtr ? typeStrPtr : "";
                        if (typeStr == "labelWidget") {
                            ((labelWidget*)widget)->setFont(f);
                        } else if (typeStr == "scrollingTextWidget") {
                            ((scrollingTextWidget*)widget)->setFont(f);
                        } else if (typeStr == "clockWidget") {
                            ((clockWidget*)widget)->setFont(f);
                        } else if (typeStr == "serviceListWidget") {
                            ((serviceListWidget*)widget)->setFont(f);
                        } else if (typeStr == "scrollingMessagePoolWidget") {
                            ((scrollingMessagePoolWidget*)widget)->setFont(f);
                        }
                    }
                }

                if (w["text"].is<const char*>()) {
                    const char* typeStrPtr = w["type"];
                    std::string typeStr = typeStrPtr ? typeStrPtr : "";
                    if (typeStr == "labelWidget") {
                        ((labelWidget*)widget)->setText(w["text"].as<const char*>());
                    } else if (typeStr == "scrollingTextWidget") {
                        ((scrollingTextWidget*)widget)->setText(w["text"].as<const char*>());
                    }
                }

                if (w["blink"].is<bool>()) {
                    const char* typeStrPtr = w["type"];
                    std::string typeStr = typeStrPtr ? typeStrPtr : "";
                    if (typeStr == "clockWidget") {
                        ((clockWidget*)widget)->setBlink(w["blink"].as<bool>());
                    }
                }
                
                const char* tsPtr = w["type"];
                if (tsPtr && std::string(tsPtr) == "serviceListWidget") {
                    serviceListWidget* sw = (serviceListWidget*)widget;
                    
                    if (w["skipRows"].is<int>()) {
                        int skip = w["skipRows"].as<int>();
                        int max = w["maxRows"].is<int>() ? w["maxRows"].as<int>() : -1;
                        sw->setDataLimits(skip, max);
                    }
                    
                    if (w["scrollDurationMs"].is<int>()) {
                        sw->setScrollDuration(w["scrollDurationMs"].as<int>());
                    }
                    
                    if (w["scrollDwellMs"].is<int>()) {
                        sw->setScrollDwell(w["scrollDwellMs"].as<int>());
                    }
                    
                    JsonArray cols = w["columns"];
                    if (!cols.isNull()) {
                        ColumnDef defs[MAX_SERVICE_COLUMNS];
                        int count = 0;
                        for (JsonObject c : cols) {
                            if (count >= MAX_SERVICE_COLUMNS) break;
                            defs[count].width = c["width"] | 0;
                            defs[count].align = static_cast<TextAlign>(c["align"] | 0);
                            count++;
                        }
                        sw->setColumns(count, defs);
                    }
                }
                
                // ... (simplified for revert)
            }
        }

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

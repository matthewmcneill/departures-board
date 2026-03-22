/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/systemMessageWidget.hpp
 * Description: Full-screen modal widget for system alerts, diagnostic messages, 
 *              and setup configuration screens. Provides consistent centered 
 *              typography for multi-line notifications.
 *
 * Exported Functions/Classes:
 * - systemMessageWidget: Graphics widget for system-level notifications.
 *   - setMessage(): Batch update title and up to 3 diagnostic lines.
 *   - tick(): No-op lifecycle hook.
 *   - render(): Primary drawing method.
 */

#ifndef SYSTEM_MESSAGE_WIDGET_HPP
#define SYSTEM_MESSAGE_WIDGET_HPP

#include "iGfxWidget.hpp"

class systemMessageWidget : public iGfxWidget {
private:
    char title[64];
    char lines[3][128];
    int numLines;

public:
    /**
     * @brief Construct a new system message widget.
     * @param _x X coordinate.
     * @param _y Y coordinate.
     * @param _w Width in pixels.
     * @param _h Height in pixels.
     */
    systemMessageWidget(int _x = 0, int _y = 0, int _w = 256, int _h = 64);

    /**
     * @brief Update the content of the widget.
     * @param newTitle Primary heading.
     * @param l1 Optional line 1.
     * @param l2 Optional line 2.
     * @param l3 Optional line 3.
     */
    void setMessage(const char* newTitle, const char* l1 = nullptr, const char* l2 = nullptr, const char* l3 = nullptr);
    
    /**
     * @brief No-op. System messages are usually static.
     * @param currentMillis Milliseconds since boot.
     */
    void tick(uint32_t currentMillis) override { /* No animation usually */ }

    /**
     * @brief Draw the multi-line message centered in the bounding box.
     * @param display U8g2 reference.
     */
    void render(U8G2& display) override;
};

#endif // SYSTEM_MESSAGE_WIDGET_HPP

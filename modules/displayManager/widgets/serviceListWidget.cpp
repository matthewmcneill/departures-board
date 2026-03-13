/*
 * Departures Board (c) 2025-2026 Gadec Software
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/gfxUtilities/src/serviceListWidget.cpp
 * Description: Implementation of multi-column service row rendering.
 */

#include "serviceListWidget.hpp"
#include "drawingPrimitives.hpp"
#include <displayManager.hpp>

serviceListWidget::serviceListWidget(int _x, int _y, int _w, int _h, const uint8_t* _font)
    : iGfxWidget(_x, _y, _w, _h), numColumns(0), font(_font), totalRows(0), currentPage(0), 
      lastPageChange(0), pageTimeoutMs(8000) {
    if (font == nullptr) font = Underground10;
    clearRows();
}

void serviceListWidget::setColumns(int _numCols, const ColumnDef* _cols) {
    numColumns = (_numCols > MAX_SERVICE_COLUMNS) ? MAX_SERVICE_COLUMNS : _numCols;
    for (int i = 0; i < numColumns; i++) {
        columns[i] = _cols[i];
    }
}

void serviceListWidget::setPageTimeout(int ms) {
    pageTimeoutMs = ms;
}

void serviceListWidget::clearRows() {
    totalRows = 0;
    currentPage = 0;
    lastPageChange = 0; // Force immediate render logic if asked
}

void serviceListWidget::addRow(const char** data) {
    if (totalRows >= 16) return; // Hard array limit
    
    for (int i = 0; i < numColumns; i++) {
        rowData[totalRows][i] = data[i];
    }
    totalRows++;
}

/**
 * @brief Draws a single row of service data onto the display.
 * @param display The U8G2 display object to draw on.
 * @param rowY The Y-coordinate for the baseline of the row.
 * @param data An array of C-style strings representing the data for the row.
 */
void serviceListWidget::drawRow(U8G2& display, int rowY, const char** data) {
    if (!isVisible) return;

    display.setFont(font);
    int currentX = x;

    for (int i = 0; i < numColumns; i++) {
        int colW = columns[i].width;
        const char* text = data[i];
        if (!text) {
            currentX += colW;
            continue;
        }

        int textW = getStringWidth(display, text);
        int drawX = currentX;

        // Apply alignment within the column
        if (columns[i].align == 1) { // Center
            drawX += (colW - textW) / 2;
        } else if (columns[i].align == 2) { // Right
            drawX += (colW - textW);
        }

        // Clip to column width if necessary
        bool clipping = (textW > colW);
        if (clipping) {
            display.setClipWindow(currentX, rowY - 12, currentX + colW, rowY + 2);
        }

        display.drawStr(drawX, rowY, text);

        if (clipping) {
            display.setMaxClipWindow();
        }

        currentX += colW;
    }
}

/**
 * @brief Updates the widget's internal state, handling page changes.
 * @param currentMillis The current system time in milliseconds.
 */
void serviceListWidget::tick(uint32_t currentMillis) {
    if (totalRows == 0 || height <= 0) return;
    
    int rowsPerPage = height / 13; // Assume 13px pitch for now.
    int totalPages = (totalRows + rowsPerPage - 1) / rowsPerPage;
    
    if (totalPages > 1) {
        if (lastPageChange == 0) lastPageChange = currentMillis;
        
        if (currentMillis - lastPageChange >= pageTimeoutMs) {
            currentPage++;
            if (currentPage >= totalPages) currentPage = 0;
            lastPageChange = currentMillis;
        }
    } else {
        currentPage = 0;
    }
}

/**
 * @brief Handles animation updates, specifically for page transitions.
 * @param display The U8G2 display object.
 * @param currentMillis The current system time in milliseconds.
 */
void serviceListWidget::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    if (!isVisible || totalRows == 0 || height <= 0) return;
    
    int oldPage = currentPage;
    tick(currentMillis);
    
    if (currentPage != oldPage) {
        int renderW = (width > 0) ? width : SCREEN_WIDTH;
        int renderH = height;
        
        display.setClipWindow(x, y, x + renderW, y + renderH);
        blankArea(display, x, y, renderW, renderH);
        render(display);
        display.setMaxClipWindow();
        
        display.updateDisplayArea(x / 8, y / 8, (renderW + 7) / 8, (renderH + 7) / 8);
    }
}

void serviceListWidget::render(U8G2& display) {
    if (!isVisible || totalRows == 0) return;
    
    int rowsPerPage = (height > 0) ? (height / 13) : 3; // Fallback to 3 if unbound
    
    int startIndex = currentPage * rowsPerPage;
    int endIndex = startIndex + rowsPerPage;
    if (endIndex > totalRows) endIndex = totalRows;
    
    int renderY = y + 12; // First row baseline
    
    for (int i = startIndex; i < endIndex; i++) {
        drawRow(display, renderY, rowData[i]);
        renderY += 13; // 13px pitch
    }
}

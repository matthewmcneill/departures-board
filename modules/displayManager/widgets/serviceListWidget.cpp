/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/serviceListWidget.cpp
 * Description: Implementation of tabular data rendering with pagination.
 */

#include "serviceListWidget.hpp"
#include "drawingPrimitives.hpp"
#include <displayManager.hpp>

/**
 * @brief Initialize the service list with default fonts and pagination state.
 */
serviceListWidget::serviceListWidget(int _x, int _y, int _w, int _h, const uint8_t* _font)
    : iGfxWidget(_x, _y, _w, _h), numColumns(0), font(_font), totalRows(0), currentPage(0), 
      lastPageChange(0), pageTimeoutMs(8000), skipRows(0), maxRows(-1), totalRowsAdded(0) {
    if (font == nullptr) font = Underground10;
    clearRows();
}

/**
 * @brief Define the column schema for the table.
 * @param _numCols Number of columns.
 * @param _cols Array of column definitions.
 */
void serviceListWidget::setColumns(int _numCols, const ColumnDef* _cols) {
    numColumns = (_numCols > MAX_SERVICE_COLUMNS) ? MAX_SERVICE_COLUMNS : _numCols;
    for (int i = 0; i < numColumns; i++) {
        columns[i] = _cols[i];
    }
}

/**
 * @brief Configure the automated pagination speed.
 * @param ms Duration in milliseconds.
 */
void serviceListWidget::setPageTimeout(int ms) {
    pageTimeoutMs = ms;
}

/**
 * @brief Configure the slice boundaries for data ingestion.
 * @param skip The number of initial rows to ignore.
 * @param max The maximum number of rows to ingest limit (-1 for unbounded, capped at 16 internally based on array limits).
 */
void serviceListWidget::setDataLimits(int skip, int max) {
    skipRows = skip;
    maxRows = max;
}

/**
 * @brief Clear all data rows and reset pagination to page zero.
 */
void serviceListWidget::clearRows() {
    totalRows = 0;
    totalRowsAdded = 0;
    currentPage = 0;
    lastPageChange = 0; // Force immediate render logic if asked
}

/**
 * @brief Add a new row of data strings, respecting slice limits. Only pointers are stored.
 * @param data Array of string pointers matching the column count.
 */
void serviceListWidget::addRow(const char** data) {
    if (totalRowsAdded < skipRows) {
        totalRowsAdded++;
        return;
    }
    
    if (maxRows >= 0 && totalRows >= maxRows) {
        totalRowsAdded++;
        return;
    }

    if (totalRows >= 16) return; // Hard array limit
    
    for (int i = 0; i < numColumns; i++) {
        rowData[totalRows][i] = data[i];
    }
    totalRows++;
    totalRowsAdded++;
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

    // --- Step 1: Iterate Columns ---
    for (int i = 0; i < numColumns; i++) {
        int colW = columns[i].width;
        const char* text = data[i];
        if (!text) {
            currentX += colW;
            continue;
        }

        int textW = getStringWidth(display, text);
        int drawX = currentX;

        // --- Step 2: Alignment Logic ---
        if (columns[i].align == 1) { // Center
            drawX += (colW - textW) / 2;
        } else if (columns[i].align == 2) { // Right
            drawX += (colW - textW);
        }

        // --- Step 3: Clipping and Rendering ---
        // If the text is wider than the column, restrict the drawing window.
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
    if (!isVisible || totalRows == 0 || height <= 0) return;
    
    // --- Step 1: Determine Capacity ---
    // Calculate how many rows fit based on a standard 13px baseline pitch.
    int rowsPerPage = height / 13;
    int totalPages = (totalRows + rowsPerPage - 1) / rowsPerPage;
    
    // --- Step 2: Page Management ---
    if (totalPages > 1) {
        if (lastPageChange == 0) lastPageChange = currentMillis;
        
        // Swap page after timeout
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

/**
 * @brief Render the current page of service data.
 * @param display U8g2 reference.
 */
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

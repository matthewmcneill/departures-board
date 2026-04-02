#include <fonts/fonts.hpp>
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
 *
 * Exported Functions/Classes:
 * - serviceListWidget: [Class implementation]
 *   - setColumns(): Defines the table layout (widths, alignments).
 *   - setDataLimits(): Configures row slicing (skipping and capping).
 *   - clearRows()/addRow(): Data ingestion lifecycle hooks.
 *   - tick(): Automated pagination logic with page-reset animation.
 *   - render(): Paints the visible row window with clipping support.
 */

#include "serviceListWidget.hpp"
#include "drawingPrimitives.hpp"
#include <displayManager.hpp>

/**
 * @brief Initialize the service list with default fonts and pagination state.
 */
/**
 * @brief Initialize the service list widget.
 * @param _x X coordinate.
 * @param _y Y coordinate.
 * @param _w Width.
 * @param _h Height (determines rows per page).
 * @param _font Optional font override.
 */
serviceListWidget::serviceListWidget(int _x, int _y, int _w, int _h, const uint8_t* _font)
    : iGfxWidget(_x, _y, _w, _h), numColumns(0), font(_font), totalRows(0), topRowIndex(0), 
      isAnimating(false), isPageResetAnimating(false), animationStartMs(0), lastDwellStart(0), 
      scrollDurationMs(1000), scrollDwellMs(5000), currentYOffset(0), skipRows(0), maxRows(-1), totalRowsAdded(0) {
    if (font == nullptr) font = Underground10;
    clearRows();
}

/**
 * @brief Define the column schema for the table.
 * @param _numCols Number of columns.
 * @param _cols Array of column definitions.
 */
/**
 * @brief Configure the table columns.
 * @param _numCols Number of active columns.
 * @param _cols Array of definitions.
 */
void serviceListWidget::setColumns(int _numCols, const ColumnDef* _cols) {
    numColumns = (_numCols > MAX_SERVICE_COLUMNS) ? MAX_SERVICE_COLUMNS : _numCols;
    for (int i = 0; i < numColumns; i++) {
        columns[i] = _cols[i];
    }
}

/**
 * @brief Set the font to use for rendering row data.
 * @param _font Pointer to the U8g2 font array.
 */
void serviceListWidget::setFont(const uint8_t* _font) {
    if (_font) font = _font;
}

/**
 * @brief Configure the scroll animation duration.
 * @param ms Duration in milliseconds.
 */
void serviceListWidget::setScrollDuration(int ms) {
    if (ms >= 0) scrollDurationMs = ms;
}

/**
 * @brief Configure the dwell time between scrolls.
 * @param ms Dwell time in milliseconds.
 */
void serviceListWidget::setScrollDwell(int ms) {
    if (ms >= 0) scrollDwellMs = ms;
}

/**
 * @brief Configure the slice boundaries for data ingestion.
 * @param skip The number of initial rows to ignore.
 * @param max The maximum number of rows to ingest limit (-1 for unbounded, capped at 16 internally based on array limits).
 */
/**
 * @brief Configure pagination skip/limit.
 * @param skip Rows to ignore at start.
 * @param max Maximum rows to display.
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
    topRowIndex = 0;
    isAnimating = false;
    isPageResetAnimating = false;
    currentYOffset = 0;
    lastDwellStart = 0; // Force immediate render logic if asked
}

/**
 * @brief Add a new row of data strings, respecting slice limits. Only pointers are stored.
 * @param data Array of string pointers matching the column count.
 */
/**
 * @brief Append a row of data string pointers.
 * Respects skip/max limits and internal array capacity (16).
 * @param data Array of string pointers.
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
 * @brief Render a single record row.
 * @param display U8g2 reference.
 * @param rowY Top Y coordinate.
 * @param data Array of column string pointers.
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

        // --- Step 2 & 3: Alignment, Clipping, and Native Rendering via drawText ---
        drawText(display, text, currentX, rowY, colW, height > 0 ? height : 64, static_cast<TextAlign>(columns[i].align), true);

        currentX += colW;
    }
}

/**
 * @brief Periodic logic for pagination timing and animation lerping.
 * @param currentMillis Milliseconds since boot.
 */
void serviceListWidget::tick(uint32_t currentMillis) {
    if (!isVisible || totalRows == 0 || height <= 0) return;
    
    int rowsPerPage = height / 13;
    
    // Do not scroll if all rows fit
    if (totalRows <= rowsPerPage) {
        topRowIndex = 0;
        isAnimating = false;
        isPageResetAnimating = false;
        currentYOffset = 0;
        return;
    }
    
    if (!isAnimating && !isPageResetAnimating) {
        if (lastDwellStart == 0) lastDwellStart = currentMillis;
        
        if (currentMillis - lastDwellStart >= scrollDwellMs) {
            if (topRowIndex + rowsPerPage >= totalRows) {
                // List reached the bottom. Trigger Page Reset Sweep.
                isPageResetAnimating = true;
                animationStartMs = currentMillis;
                topRowIndex = 0;
            } else {
                isAnimating = true;
                animationStartMs = currentMillis;
            }
        }
    } 
    
    if (isAnimating) {
        uint32_t elapsed = currentMillis - animationStartMs;
        if (elapsed >= scrollDurationMs) {
            isAnimating = false;
            topRowIndex++;
            lastDwellStart = currentMillis;
            currentYOffset = 0;
        } else {
            float p = (float)elapsed / scrollDurationMs;
            currentYOffset = (int)(p * 13.0f); // 13px line height
        }
    } else if (isPageResetAnimating) {
        uint32_t elapsed = currentMillis - animationStartMs;
        int pagePixelHeight = rowsPerPage * 13;
        
        if (elapsed >= scrollDurationMs) {
            isPageResetAnimating = false;
            lastDwellStart = currentMillis;
            currentYOffset = 0;
        } else {
            float p = (float)elapsed / scrollDurationMs;
            currentYOffset = -pagePixelHeight + (int)(p * pagePixelHeight);
        }
    }
}

/**
 * @brief Targeted high-framerate redraw for smooth scrolling.
 * @param display U8G2 reference.
 * @param currentMillis Milliseconds since boot.
 */
void serviceListWidget::renderAnimationUpdate(U8G2& display, uint32_t currentMillis) {
    if (!isVisible || totalRows == 0 || height <= 0) return;
    
    bool wasAnimating = isAnimating || isPageResetAnimating;
    tick(currentMillis);
    
    // Refresh during animation active frame, or the first frame it finishes.
    if (isAnimating || isPageResetAnimating || (wasAnimating && !isAnimating && !isPageResetAnimating)) {
        int renderW = (width > 0) ? width : 256;
        int renderH = height;
        
        U8g2StateSaver stateSaver(display);
        
        // --- Arcane Logic ---
        // Prevent 8-bit boundary wraps (256 -> 0) by using inclusive end coordinates.
        display.setClipWindow(x, y, x + renderW - 1, y + renderH - 1);
        blankArea(display, x, y, renderW, renderH);
        render(display);
        
        display.updateDisplayArea(x / 8, y / 8, (renderW + 7) / 8, (renderH + 7) / 8);
    }
}

/**
 * @brief Full frame render for the visible data window.
 * @param display U8g2 reference.
 */
void serviceListWidget::render(U8G2& display) {
    if (!isVisible || totalRows == 0) return;
    
    int rowsPerPage = (height > 0) ? (height / 13) : 3;
    
    int rowsToDraw = rowsPerPage;
    // If we're scrolling line-by-line, we need to draw an extra row sliding in from the bottom
    if (isAnimating) {
        rowsToDraw = rowsPerPage + 1;
    }
    
    int renderY = y - currentYOffset; // Align with setFontPosTop
    
    U8g2StateSaver stateSaver(display);

    int renderW = (width > 0) ? width : 256;
    int renderH = height > 0 ? height : 64;
    
    // --- Arcane Logic ---
    // Prevent 8-bit boundary wraps (256 -> 0) by using inclusive end coordinates.
    display.setClipWindow(x, y, x + renderW - 1, y + renderH - 1);

    for (int i = 0; i < rowsToDraw; i++) {
        int idx = topRowIndex + i; // Strict bounds, no infinitely looping array
        if (idx < totalRows) {
            drawRow(display, renderY, rowData[idx]);
        }
        renderY += 13; // 13px pitch
    }
}

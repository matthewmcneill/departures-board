/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: lib/gfxUtilities/serviceListWidget.hpp
 * Description: Multi-column departure list widget with configurable alignments.
 */

#ifndef SERVICE_LIST_WIDGET_HPP
#define SERVICE_LIST_WIDGET_HPP

#include "iGfxWidget.hpp"

#define MAX_SERVICE_COLUMNS 4

struct ColumnDef {
    int width;
    uint8_t align; // 0 = Left, 1 = Center, 2 = Right
};

class serviceListWidget : public iGfxWidget {
private:
    ColumnDef columns[MAX_SERVICE_COLUMNS];
    int numColumns;
    const uint8_t* font;
    
    // Data storage (pointers only)
    const char* rowData[16][MAX_SERVICE_COLUMNS];
    int totalRows;
    
    // Pagination state
    int currentPage;
    uint32_t lastPageChange;
    int pageTimeoutMs;

public:
    serviceListWidget(int _x, int _y, int _w = -1, int _h = -1, const uint8_t* _font = nullptr);

    /**
     * @brief Configure the column layout for this list.
     * @param _numCols Number of columns (max 4).
     * @param _cols Array of ColumnDef structures.
     */
    void setColumns(int _numCols, const ColumnDef* _cols);

    /**
     * @brief Set how long each page should display before swapping.
     * @param ms Timeout in milliseconds. Default is 8000 (8 seconds).
     */
    void setPageTimeout(int ms);

    /**
     * @brief Clears all stored rows. Call this before adding new data.
     */
    void clearRows();

    /**
     * @brief Appends a row of data to the widget.
     * @param rowData Array of column string pointers.
     */
    void addRow(const char** data);

    /**
     * @brief Render a single row of data (called internally, but exposed for flexibility).
     * @param display Reference to U8g2.
     * @param rowY The Y coordinate for the baseline of this row.
     * @param rowData Array of strings (one per column).
     */
    void drawRow(U8G2& display, int rowY, const char** rowData);

    void tick(uint32_t currentMillis) override;
    void render(U8G2& display) override;
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;
};

#endif // SERVICE_LIST_WIDGET_HPP

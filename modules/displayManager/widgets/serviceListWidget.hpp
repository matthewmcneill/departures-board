/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/widgets/serviceListWidget.hpp
 * Description: High-level table widget for rendering multi-column transport data. 
 *              Supports configurable alignments, clipping, and automatic pagination 
 *              for large data sets.
 *
 * Exported Functions/Classes:
 * - ColumnDef: Configuration structure for column width and alignment.
 * - serviceListWidget: Graphics widget for tabular service information.
 *   - setColumns(): Define the table schema.
 *   - setPageTimeout(): Configure rotation speed for paginated data.
 *   - clearRows() / addRow(): Data ingestion methods.
 *   - drawRow(): Internal utility for rendering a single record.
 *   - tick(): Logic update for pagination timing.
 *   - render(): Primary drawing method.
 *   - renderAnimationUpdate(): Targeted page transition animation.
 */

#ifndef SERVICE_LIST_WIDGET_HPP
#define SERVICE_LIST_WIDGET_HPP

#include "iGfxWidget.hpp"
#include "drawingPrimitives.hpp"

#define MAX_SERVICE_COLUMNS 6

struct ColumnDef {
    int width;
    TextAlign align; // LEFT, CENTER, RIGHT
};

class serviceListWidget : public iGfxWidget {
private:
    ColumnDef columns[MAX_SERVICE_COLUMNS];
    int numColumns;
    const uint8_t* font;
    
    // Data storage (pointers only)
    const char* rowData[16][MAX_SERVICE_COLUMNS];
    int totalRows;
    
    // Scrolling state
    int topRowIndex;
    bool isAnimating;
    bool isPageResetAnimating;
    uint32_t animationStartMs;
    uint32_t lastDwellStart;
    int scrollDurationMs;
    int scrollDwellMs;
    int currentYOffset;

    // Data Slicing Configuration
    int skipRows;
    int maxRows;
    int totalRowsAdded;

public:
    /**
     * @brief Construct a new service list widget.
     * @param _x X coordinate.
     * @param _y Y coordinate.
     * @param _w Optional width.
     * @param _h Optional height (controls rows per page).
     * @param _font Optional font override.
     */
    serviceListWidget(int _x, int _y, int _w = -1, int _h = -1, const uint8_t* _font = nullptr);

    /**
     * @brief Configure the column layout for this list.
     * @param _numCols Number of columns (max 4).
     * @param _cols Array of ColumnDef structures.
     */
    void setColumns(int _numCols, const ColumnDef* _cols);
    
    /**
     * @brief Set the font to use for rendering row data.
     * @param _font Pointer to the U8g2 font byte array.
     */
    void setFont(const uint8_t* _font);

    /**
     * @brief Set how long the scroll animation should take.
     * @param ms Duration in milliseconds.
     * @designer_prop int scrollDurationMs = 1000 - Scroll duration in ms.
     */
    void setScrollDuration(int ms);

    /**
     * @brief Set how long to dwell on each line.
     * @param ms Dwell time in milliseconds.
     * @designer_prop int scrollDwellMs = 5000 - Dwell time in ms.
     */
    void setScrollDwell(int ms);

    /**
     * @brief Define the data slicing boundaries for this list.
     * @param skip Number of rows to discard from the beginning of the data set.
     * @param max Maximum number of rows to ingest. Set to -1 for no limit.
     */
    void setDataLimits(int skip, int max);

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

    /**
     * @brief Periodic logic for pagination timing.
     * @param currentMillis Milliseconds since boot.
     */
    void tick(uint32_t currentMillis) override;

    /**
     * @brief Full frame render for the visible page of data.
     * @param display U8g2 reference.
     */
    void render(U8G2& display) override;

    /**
     * @brief Targeted redraw for smooth page transitions.
     * @param display U8g2 reference.
     * @param currentMillis Milliseconds since boot.
     */
    void renderAnimationUpdate(U8G2& display, uint32_t currentMillis) override;
};

#endif // SERVICE_LIST_WIDGET_HPP

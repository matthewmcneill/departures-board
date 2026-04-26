/*
 * Departures Board (c) 2025-2026 Gadec Software
 * Refactored for v3.0 by Matt McNeill 2026 CB Labs
 *
 * https://github.com/gadec-uk/departures-board
 *
 * This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
 * To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
 *
 * Module: modules/displayManager/displayManager.hpp
 * Description: Central orchestrator for the hardware display subsystem. 
 *              Manages a statically allocated memory pool of Board implementations 
 *              using std::variant to prevent heap fragmentation. Handles display 
 *              lifecycle, carousel rotation, power management (sleep/dimming), 
 *              and global UI overlays.
 *
 * Exported Functions/Classes:
 * - BoardType: Enum identifying the specific board implementation class.
 * - SystemBoardId: Identifiers for functional layout templates in the system registry.
 * - BoardVariant: Type-safe union for statically allocated board memory.
 * - DisplayManager: Main singleton orchestrator for display logic and rendering.
 *   - begin(): Initialize hardware and apply initial configurations.
 *   - tick(): Main execution logic loop (sleep/rotate/update).
 *   - render(): Forced synchronized display refresh.
 *   - showBoard(): Explicitly transition to a specific board.
 *   - setPowerSave(): Control hardware OLED power state (on/off).
 *   - applyConfig(): Bulk provision system state from a configuration object.
 *   - cycleNext(): Advance the board carousel.
 *   - yieldAnimationUpdate(): Logic for smooth sub-frame UI animations.
 *   - setBrightness() / getBrightness(): Display intensity control.
 *   - setFlipScreen() / getFlipScreen(): Orientation control.
 *   - setSleepEnabled() / getSleepEnabled(): Schedule toggle.
 *   - setForcedSleep() / getForcedSleep(): Manual sleep override.
 *   - getIsSleeping(): Status check.
 *   - addBoard(): Dynamic carousel registration.
 *   - getDisplayBoard(): Board accessor.
 * - displayManager: Global singleton instance.
 */

#pragma once

class appContext;

#include <U8g2lib.h>
#include <variant>
#include "iConfigurable.hpp"
#include "boards/interfaces/iDisplayBoard.hpp"
#include <widgets/wifiStatusWidget.hpp>
#include <messaging/messagePool.hpp>

#include "departuresBoard.hpp"



/**
 * @brief Identifiers for specific board implementation classes.
 */
enum class BoardType {
    NR_BOARD,  // National Rail Departures
    TFL_BOARD, // London Underground / TfL Departures
    BUS_BOARD, // Local Bus Times
    CLOCK_BOARD, // Screensaver / Clock
    DIAGNOSTIC_BOARD // Hardware Diagnostics
};

/**
 * @brief Identifiers for functional layout templates within the System Registry.
 */
enum class SystemBoardId {
    SYS_BOOT_SPLASH,    // Gadec logo screen
    SYS_BOOT_LOADING,   // Progress indicator during init
    SYS_WIFI_WIZARD,    // QR and connection guide
    SYS_SETUP_HELP,     // Station setup instructions with IP
    SYS_ERROR_NO_DATA,  // Failure to pull JSON
    SYS_ERROR_WSDL,     // NRE WSDL server failure
    SYS_ERROR_TOKEN,    // API key rejection
    SYS_ERROR_CRS,      // Invalid station code
    SYS_FIRMWARE_UPDATE, // OTA progress screen
    SYS_SLEEP_CLOCK,     // Snooze/Screensaver clock
    SYS_ERROR_WIFI,       // Persistent WiFi disconnection
    SYS_DIAGNOSTIC       // Hardware calibration grid
};

class DisplayManager : public iConfigurable {
private:
    U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2; ///< Concrete hardware display driver instance
    iDisplayBoard* slots[MAX_BOARDS]; // Memory pool for heap allocated boards
    int activeSlotIndex;           // Currently selected slot in the pool (0 to MAX_BOARDS-1)

    // System Boards Memory
    iDisplayBoard* splashBoard;
    iDisplayBoard* loadingBoard;
    iDisplayBoard* wizardBoard;
    iDisplayBoard* helpBoard;
    iDisplayBoard* messageBoard;
    iDisplayBoard* firmwareUpdateBoard;
    iDisplayBoard* sleepingBoard;
    iDisplayBoard* diagnosticBoard;
    
    wifiStatusWidget wifiWarning;
    appContext* context; ///< Reference to parent context for DI

    // Display configuration set by ConfigManager
    int brightness;                ///< Display brightness level (0-255)
    bool flipScreen;               ///< True if display is inverted 180 degrees
    bool forcedSleep;              ///< Manual sleep override
    
    // Unified rendering target (pool or system)
    iDisplayBoard* currentBoard; // Pointer to a board from either the pool or registry
    unsigned long lastActivity;  // Timestamp of last user/data interaction for sleep logic
    bool diagModeActive = false; // Run-time flag for hardware calibration grid
    
    // Upstream B2.4: Pacing and Power extensions
    bool waitForScrollComplete = false;
    bool prioritiseRss = false;
    bool oledPowerSaveActive = false; ///< Tracks if the hardware is currently in power-save mode
    unsigned long lastScheduleCheck = 0; ///< Throttling for scheduler evaluation in tick()

    /**
     * @brief Access the encapsulated hardware display instance.
     * @return Reference to the hardware display object.
     */
    U8G2& getDisplay() { return u8g2; }

public:
    /**
     * @brief Set the hardware power save state (OLED on/off) and track it internally.
     * @param off True to power down the OLED, false to wake it up.
     */
    void setPowerSave(bool off);

    /**
     * @brief Set the global display brightness.
     * @param level Hardware level (0-255).
     */
    void setBrightness(int level);

    /**
     * @brief Get the current brightness level.
     * @return Hardware level (0-255).
     */
    int getBrightness() const { return brightness; }

    /**
     * @brief Obtain raw pointer access to the U8g2 memory driver byte array buffer.
     *        Warning: Thread safety required out of bounds of DisplayManager.
     */
    const uint8_t* getRawFramebuffer() { return u8g2.getBufferPtr(); }

    /**
     * @brief Calculate exact RAM footprint of the initialized U8g2 buffer array.
     */
    size_t getFramebufferSize() { return (size_t)8 * u8g2.getBufferTileHeight() * u8g2.getBufferTileWidth(); }

    /**
     * @brief Switch to a new board and optionally block for an animation duration.
     * @param board Pointer to the board implementation to render.
     * @param reason The semantic trigger or event causing this board to be explicitly shown.
     * @param durationMs Optional duration in ms to block and animate.
     */
    void showBoard(iDisplayBoard* board, const char* reason = "Unknown Lifecycle Event", uint32_t durationMs = 0);

    /**
     * @brief Get the currently active rendering target.
     * @return Pointer to active iDisplayBoard.
     */
    iDisplayBoard* getCurrentBoard();

    /**
     * @brief Retrieve a functional system board singleton from the registry.
     * @param id The identifier for the required screen.
     * @return iDisplayBoard* Pointer to the configured singleton.
     */
    iDisplayBoard* getSystemBoard(SystemBoardId id);

    /**
     * @brief Maps an interface error status (UPD_*) to a functional SystemBoardId.
     * @param status The error code from a data source.
     * @return The corresponding system board ID for delegation.
     */
    SystemBoardId mapErrorToId(UpdateStatus status);

    /**
     * @brief Trigger a synchronized render of the current board.
     * @note Manages the clearBuffer()/sendBuffer() transaction.
     */
    void render();


    /**
     * @brief Get the current screen flip state.
     * @return True if display is inverted 180 degrees.
     */
    bool getFlipScreen() const;
    /**
     * @brief Control the screen orientation.
     * @param newFlipScreen True to invert the display 180 degrees.
     */
    void setFlipScreen(bool newFlipScreen);

    /**
     * @brief Control the hardware diagnostic grid overlay.
     * @param active True to force the diagnostic screen.
     */
    void setDiagMode(bool active);

    /**
     * @return True if diagnostic mode is currently forced.
     */
    bool getDiagMode() const { return diagModeActive; }

    /**
     * @brief Default constructor.
     */
    DisplayManager();

    /**
     * @brief Access the system-wide global message pool.
     * @return MessagePool* Pointer to the global pool.
     */
    MessagePool* getGlobalMessagePool();

    /**
     * @brief Initialize the hardware display and apply global settings.
     * @param contextPtr Pointer to the parent application context.
     */
    void begin(appContext* contextPtr);



    // Main execution loop for rendering the active board or the screensaver
    void tick(unsigned long currentMillis);

    /**
     * @brief Unconditionally resumes the primary display carousel context.
     *        This effectively wakes up the display from sleep mode and transitions
     *        away from system screens (Splash, Loading, Warning).
     */
    void resumeDisplays();

    /**
     * @brief Get current hardware execution mode wrapped inside the active board.
     * @return BoardType enum indicating active board API type.
     */
    BoardType getActiveBoardType();

    /**
     * @brief Allocates a specific Board type into a designated Carousel slot.
     */
    void setBoardType(int slotIndex, BoardType type);

    /**
     * @brief Clears a particular slot back to uninitialized memory.
     */
    void clearSlot(int slotIndex);

    /**
     * @brief Obtain the specific Board object stored in a slot.
     */
    /**
     * @brief Retrieve the board pointer for a specific slot.
     * @param slotIndex Index (0 to MAX_BOARDS-1).
     * @return iDisplayBoard** Pointer to the struct storage.
     */
    iDisplayBoard** getSlot(int slotIndex);

    /**
     * @brief Retrieve the board implementation from a specific slot.
     * @param slotIndex Index (0 to MAX_BOARDS-1).
     * @return iDisplayBoard* Pointer to the board, or nullptr if empty.
     */
    iDisplayBoard* getDisplayBoard(int slotIndex);



    /**
     * @brief Clear all carousel slots.
     */
    void clearSlots();

    /**
     * @brief Add a board of the specified type to the first available slot.
     * @return iDisplayBoard* Pointer to the new board, or nullptr if full.
     */
    iDisplayBoard* addBoard(BoardType type);

    /**
     * @brief Get the active array index within the Carousel pool.
     */
    int getActiveSlotIndex() const { return activeSlotIndex; }

    /**
     * @brief Rotates the carousel index to the next initialized board.
     * @return The new active slot index.
     */
    int cycleNext();
    

    /**
     * @brief Apply a central configuration object to the display settings and all active boards.
     * @param config Reference to the loaded configuration.
     */
    void applyConfig(const struct Config& config);

    /**
     * @brief Invokes renderAnimationUpdate on the active board while respecting a 60fps cap (16ms).
     */
    void yieldAnimationUpdate();

    /**
     * @brief Implements the iConfigurable interface.
     */
    void reapplyConfig(const Config& config) override { applyConfig(config); }
};

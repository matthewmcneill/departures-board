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
#include <boards/nationalRailBoard/nationalRailBoard.hpp>
#include <boards/tflBoard/tflBoard.hpp>
#include <boards/busBoard/busBoard.hpp>
#include <boards/systemBoard/diagnosticBoard.hpp>
#include <boards/systemBoard/splashBoard.hpp>
#include <boards/systemBoard/loadingBoard.hpp>
#include <boards/systemBoard/wizardBoard.hpp>
#include <boards/systemBoard/helpBoard.hpp>
#include <boards/systemBoard/messageBoard.hpp>
#include <boards/systemBoard/firmwareUpdateBoard.hpp>
#include <boards/systemBoard/sleepingBoard.hpp>
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

/**
 * @brief Type-safe union overlay spanning the maximum memory footprint 
 *        of any hardware Board class implementation.
 */
using BoardVariant = std::variant<std::monostate, NationalRailBoard, TfLBoard, BusBoard, DiagnosticBoard>;

class DisplayManager : public iConfigurable {
private:
    U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2; ///< Concrete hardware display driver instance
    BoardVariant slots[MAX_BOARDS]; // Memory pool for statically allocated boards
    int activeSlotIndex;           // Currently selected slot in the pool (0 to MAX_BOARDS-1)

    // System Boards Memory
    SplashBoard splashBoard;
    LoadingBoard loadingBoard;
    WizardBoard wizardBoard;
    HelpBoard helpBoard;
    MessageBoard messageBoard;
    FirmwareUpdateBoard firmwareUpdateBoard;
    SleepingBoard sleepingBoard;
    DiagnosticBoard diagnosticBoard;
    
    // Global Feature Overlays
    wifiStatusWidget wifiWarning;
    bool otaUpdateAvailable;

    appContext* context; ///< Reference to parent context for DI

    // Display configuration set by ConfigManager
    int brightness;                ///< Display brightness level (0-255)
    bool flipScreen;               ///< True if display is inverted 180 degrees
    bool sleepEnabled;             ///< True if scheduled sleep is active
    byte sleepStarts;              ///< Hour (0-23) to start sleep mode
    byte sleepEnds;                ///< Hour (0-23) to end sleep mode
    bool forcedSleep;              ///< Manual sleep override
    
    // Unified rendering target (pool or system)
    iDisplayBoard* currentBoard; // Pointer to a board from either the pool or registry
    unsigned long lastActivity;  // Timestamp of last user/data interaction for sleep logic
    bool diagModeActive = false; // Run-time flag for hardware calibration grid

    /**
     * @brief Access the encapsulated hardware display instance.
     * @return Reference to the hardware display object.
     */
    U8G2& getDisplay() { return u8g2; }

public:
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
    SystemBoardId mapErrorToId(int status);

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
     * @brief Check if scheduled sleep is enabled.
     * @return True if the sleep timer is active.
     */
    /**
     * @brief Check if scheduled sleep is currently enabled.
     * @return True if the sleep timer is active.
     */
    bool getSleepEnabled() const;
    /**
     * @brief Enable or disable scheduled sleep.
     * @param newSleepEnabled True to activate the timer.
     */
    void setSleepEnabled(bool newSleepEnabled);

    /**
     * @brief Get the hour the sleep schedule begins.
     * @return Hour (0-23).
     */
    byte getSleepStarts() const;

    /**
     * @brief Configure the sleep schedule start time.
     * @param newSleepStarts Hour (0-23).
     */
    void setSleepStarts(byte newSleepStarts);

    /**
     * @brief Get the hour the sleep schedule ends.
     * @return Hour (0-23).
     */
    byte getSleepEnds() const;

    /**
     * @brief Configure the sleep schedule end time.
     * @param newSleepEnds Hour (0-23).
     */
    void setSleepEnds(byte newSleepEnds);

    /**
     * @brief Get the manual forced sleep state.
     * @return True if sleep is manually overridden to active.
     */
    bool getForcedSleep() const;

    /**
     * @brief Manually force the screen into sleep mode.
     * @param active True to snooze immediately.
     */
    void setForcedSleep(bool active);

    /**
     * @brief Instruct the DisplayManager to render the OTA update icon.
     */
    void setOtaUpdateAvailable(bool available) { otaUpdateAvailable = available; }

    /**
     * @brief Check if an OTA update is pending.
     */
    bool isOtaUpdateAvailable() const { return otaUpdateAvailable; }

    /**
     * @brief Check if the display is currently in a sleep/clock state.
     * @return True if sleeping.
     */
    bool getIsSleeping() const;

    /**
     * @brief Internal logic to determine if the system SHOULD be snoozing.
     * @return True if schedule or manual override dictates sleep.
     */
    bool isSnoozing();

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
     * @brief Retrieve the raw BoardVariant pointer for a specific slot.
     * @param slotIndex Index (0 to MAX_BOARDS-1).
     * @return BoardVariant* Pointer to the variant storage.
     */
    BoardVariant* getSlot(int slotIndex);

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

// Global instance exposed
extern DisplayManager displayManager;

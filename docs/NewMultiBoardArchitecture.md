# Board Data Structures Evaluation

## Current Implementation Overview

You've correctly identified the current design pattern: `rdStation` and `rdService` (defined in `lib/stationData/stationData.h`) act as monolithic "mega structures". 

Instead of having different types of station or service objects for National Rail, TfL, and buses, the codebase uses a single, unified `rdStation` struct containing an array of `rdService` structs. 

Here is what the `rdService` struct looks like:
```cpp
struct rdService {
    char sTime[6];
    char destination[MAXLOCATIONSIZE];
    char via[MAXLOCATIONSIZE];  // also used for line name for TfL
    char etd[11];
    char platform[4];
    bool isCancelled;
    bool isDelayed;
    int trainLength;            // National Rail only
    byte classesAvailable;      // National Rail only
    char opco[50];              // National Rail only

    int serviceType;
    int timeToStation;          // TfL only
};
```

### How different modes use this structure:
1. **National Rail (`MODE_RAIL`)**: Populates almost all fields (ETD, platform, train length, operator, classes). The `via` field stores the routing information.
2. **Underground (`MODE_TUBE`)**: Leaves rail-specific fields like `trainLength` empty. It repurposes the `via` field to store the tube line name (e.g., "Jubilee") and uses the specific `timeToStation` integer field for countdowns.
3. **Bus (`MODE_BUS`)**: Uses `destination`, leaves `platform` blank or uses it for stop letters, and populates `etd`.

---

## Evaluation of the Abstraction

### The Good: Memory Predictability
Based on the `docs/memory_management.md` document, this design choice was very intentional. By ensuring `rdStation` and `rdService` are fixed-size structs with statically sized `char` arrays, the application guarantees **zero dynamic heap allocation** during API parsing. 

When you instantiate a `rdStation` statically (as done in `Departures Board.cpp` line 302: `rdStation station;`), you allocate exactly the maximum memory it could ever need once at boot. This completely eliminates heap fragmentation, which is critical for long-running stability on the ESP32.

### The Bad: Extensibility & Semantic Clarity
However, from an Object-Oriented Design (OOD) perspective, the abstraction is quite poor:
1. **Semantic Overloading**: Reusing the `via` field to mean "Tube Line Name" is confusing for future maintainers.
2. **Wasted Memory**: A bus data fetch still allocates bytes in memory for `trainLength` and `classesAvailable`, even though buses don't have first-class carriages or 12-car lengths.
3. **Rigid Extensibility**: If a new API needs complex nested data (e.g., tram carriage capacity, or ferry weather warnings), you must add those fields to the global `rdService` struct, bloating the memory size for all other unrelated modes.
4. **Coupled Display Logic**: Currently, the `src/Departures Board.cpp` main loop manually branches based on `boardMode`. The display module has to "know" which fields of the struct to render to the OLED based on a global state.

---

## Architectural Deep Dive: Multi-Board Switching

To build a extensible, multi-board system without introducing heap fragmentation, we need a **Carousel Architecture** using polymorphism backed by static allocation.

### Sizing the Memory Impact (`MAX_BOARDS`)
The current `rdStation` struct is ~2.5 KB. Holding an array of these natively in memory is perfectly safe on an ESP32 (300 KB+ RAM).
To allow for different capacities depending on hardware, we define `MAX_BOARDS` via the environment:
  ```cpp
  #ifndef MAX_BOARDS
  #define MAX_BOARDS 3 // Default for constrained chips
  #endif
  StationVariant boards[MAX_BOARDS];
  ```
  *(e.g., inside `platformio.ini`, `esp32s3nano` might define `-D MAX_BOARDS=5`)*

### The Ideal Architecture (OOD + Static Memory Pool)

#### 1. Abstract Base Classes (The Interfaces)
We start by defining pure virtual classes. 
Crucially, **the display logic should sit here.** The main `.cpp` file shouldn't "pull" data and guess how to format it; instead, it should "ask" the active board to draw itself onto the canvas.

```cpp
class IService {
public:
    virtual const char* getScheduledTime() const = 0;
    virtual const char* getDestination() const = 0;
    virtual const char* getExpectedTime() const = 0;
};

class IStation {
public:
    virtual const char* getLocationName() const = 0;
    virtual int getNumServices() const = 0;
    virtual const IService* getService(int index) const = 0;
    
    // Abstracted Core Loop Responsibilities
    virtual void updateData() = 0; // Trigger API fetch
    
    // The Station knows how to draw its own unique layout!
    virtual void drawHeader(U8G2& display) const = 0;
    virtual void drawService(U8G2& display, int serviceIndex, int yPos) const = 0;
    virtual void animateTick() = 0; // Handle any mode-specific scrolling
};
```

**How Display Logic is Managed:** 
If `TrainStation` implements `drawService()`, it knows exactly where the Platform column goes.
If `BusStation` implements `drawService()`, it doesn't draw a platform column at all, yielding more screen space.
The main loop in `Departures Board.cpp` just becomes:
```cpp
getActiveBoard().drawHeader(u8g2);
for (int i=0; i < 3; i++) {
   getActiveBoard().drawService(u8g2, i, calculateYPos(i));
}
```

#### 2. The Carousel Pool (`std::variant` or `union`)
To prevent fragmentation, we statically allocate a pool of generic memory blocks that can safely hold *any* subclass object.

```cpp
#include <variant>

// A "Slot" takes up the memory of the largest possible station subclass, plus a type index.
using StationVariant = std::variant<TrainStation, TubeStation, BusStation>;

class BoardCarousel {
private:
    StationVariant boards[MAX_BOARDS]; // Statically allocated pool based on env variable!
    int currentDisplayIdx = 0;

public:
    void cycleNext() {
        currentDisplayIdx = (currentDisplayIdx + 1) % numActiveBoards;
        // Trigger immediate fetch when selected!
        getActiveBoard().updateData(); 
    }
};
```

---

## Configuration Storage & Web UX Impact

If the device supports 5 boards, the Web GUI must allow users to configure all 5, which means significant changes to how settings are saved and presented.

### Storage Changes (`config.json`)
Currently, `loadConfig()` (in `include/gfx/DisplayEngine.hpp`) parses a flat JSON structure because the board only has one global mode:
```json
{
  "mode": 0,
  "crs": "PAD",
  "altCrs": "",
  "tubeId": "",
  "busId": ""
}
```
**The New Schema**: The `config.json` must pivot to an array-based schema for the board configurations, while keeping system-wide settings (brightness, sleep times) at the root level:
```json
{
  "brightness": 20,
  "sleepStarts": 23,
  "boards": [
    { "mode": 0, "crs": "PAD", "altCrs": "" },
    { "mode": 2, "busId": "490000077E" },
    { "mode": 1, "tubeId": "940GZZLUKSX" }
  ]
}
```
### Schema Migration (Backward Compatibility)
When a user updates their firmware via OTA, they will have the old flat `config.json` on their device. If we try to parse `settings["boards"].as<JsonArray>()`, it will fail or return null, crashing the board on reboot.

To solve this, `loadConfig()` must gracefully migrate the old schema to the new array schema:
```cpp
if (settings["boards"].isNull()) {
    // Legacy schema detected! Perform migration.
    LOG_INFO("Legacy config.json detected. Migrating to Multi-Board schema...");
    
    // 1. Manually extract the active legacy board based on settings["mode"]
    int legacyMode = settings["mode"].as<int>();
    
    // 2. Initialize the first slot of our std::variant array with the legacy parameters
    if (legacyMode == MODE_RAIL) {
        boards[0] = TrainStation(settings["crs"].as<const char*>(), settings["altCrs"].as<const char*>(), ...);
    } else if (legacyMode == MODE_TUBE) {
        boards[0] = TubeStation(settings["tubeId"].as<const char*>());
    } // ...
    
    // 3. Optional: Automatically re-serialize and overwrite config.json 
    // immediately to format it to the new schema for future reboots.
    saveMigrationConfig(); 
} else {
    // Parse normally from the Array
    JsonArray boardConfigArray = settings["boards"].as<JsonArray>();
    // ...
}
```
This ensures zero disruption for existing users. They will upgrade the firmware, the board will boot, detect the flat schema, instantiate their single active board into slot `0` of the Carousel, and act exactly as it did before until they visit the Web GUI to add a second board.

### Web GUI UX Changes (`index.htm`)
The current web interface (`web/index.htm`) acts like a traditional HTML `<form>`, with hardcoded `<input>` fields for the National Rail CRS, the Tube Naptan ID, etc., mapped to the singular file state.

**The New UX**:
1. **Dynamic Form Generation**: The JavaScript within the GUI must loop over the `boards` array from `config.json` and dynamically render a UI "Card" for each configured board.
2. **Add/Remove Functionality**: There must be an "Add Board" button that appends a new board card to the UI (up to `MAX_BOARDS`). There should also be a "Delete" button on each board's card.
3. **Mode Selection Dropdown**: Inside each Board Card, the first option must be a "Transport Mode" dropdown (National Rail/Tube/Bus). When changed, the JavaScript should hide/show the relevant input fields *for that specific card* (e.g., hiding the Tube Naptan field when National Rail is selected).
4. **JSON Serialization**: When the user clicks "Save Settings", the web client must serialize all the dynamically generated board cards back into the `boards: []` JSON array and `POST` it to `/savesettings`.

By shifting the Web GUI to a dynamic JavaScript UI, the user can visually build their custom "Carousel" of boards, and the `config.json` accurately reflects the memory slots needed at boot time.

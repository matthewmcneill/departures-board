# Board Data Structures Evaluation

## 1. Current Implementation Overview

The current design pattern: `rdStation` and `rdService` (defined in `lib/stationData/stationData.h`) act as monolithic "mega structures". 

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

### 1.1 How different modes use this structure:
1. **National Rail (`MODE_RAIL`)**: Populates almost all fields (ETD, platform, train length, operator, classes). The `via` field stores the routing information.
2. **Underground (`MODE_TUBE`)**: Leaves rail-specific fields like `trainLength` empty. It repurposes the `via` field to store the tube line name (e.g., "Jubilee") and uses the specific `timeToStation` integer field for countdowns.
3. **Bus (`MODE_BUS`)**: Uses `destination`, leaves `platform` blank or uses it for stop letters, and populates `etd`.

---

## 2. Evaluation of the Historical Abstraction

### 2.1 The Good: Memory Predictability
Based on the `docs/MemoryManagement.md` document, this design choice was very intentional. By ensuring `rdStation` and `rdService` are fixed-size structs with statically sized `char` arrays, the application guarantees **zero dynamic heap allocation** during API parsing. 

When you instantiate a `rdStation` statically (as done in `Departures Board.cpp` line 302: `rdStation station;`), you allocate exactly the maximum memory it could ever need once at boot. This completely eliminates heap fragmentation, which is critical for long-running stability on the ESP32.

### 2.2 The Bad: Extensibility & Semantic Clarity
However, from an Object-Oriented Design (OOD) perspective, the abstraction is quite poor:
1. **Semantic Overloading**: Reusing the `via` field to mean "Tube Line Name" is confusing for future maintainers.
2. **Wasted Memory**: A bus data fetch still allocates bytes in memory for `trainLength` and `classesAvailable`, even though buses don't have first-class carriages or 12-car lengths.
3. **Rigid Extensibility**: If a new API needs complex nested data (e.g., tram carriage capacity, or ferry weather warnings), you must add those fields to the global `rdService` struct, bloating the memory size for all other unrelated modes.
4. **Coupled Display Logic**: Currently, the `src/Departures Board.cpp` main loop manually branches based on `boardMode`. The display module has to "know" which fields of the struct to render to the OLED based on a global state.

---

## 3. The Object-Oriented Refactoring Plan

### 3.1 Stage 1 & 2: Multi-Board Switching & Memory Pooling
To build a extensible, multi-board system without introducing heap fragmentation, we need a **Carousel Architecture** using polymorphism backed by static allocation.

#### Sizing the Memory Impact (`MAX_BOARDS`)
The current `rdStation` struct is ~2.5 KB. Holding an array of these natively in memory is perfectly safe on an ESP32 (300 KB+ RAM).
To allow for different capacities depending on hardware, we define `MAX_BOARDS` via the environment:
  ```cpp
  #ifndef MAX_BOARDS
  #define MAX_BOARDS 3 // Default for constrained chips
  #endif
  StationVariant boards[MAX_BOARDS];
  ```
  *(e.g., inside `platformio.ini`, `esp32s3nano` might define `-D MAX_BOARDS=5`)*

#### Abstract Base Classes (The Interfaces)
We start by defining pure virtual classes. 
Crucially, **the display logic should sit here.** The main `.cpp` file shouldn't "pull" data and guess how to format it; instead, it should "ask" the active board to draw itself onto the canvas.

```cpp
class iDisplayBoard {
public:
    virtual void tick(uint32_t currentMillis) = 0;
    virtual void render(U8G2& display) = 0;
    virtual int updateData() = 0;
};
```

#### The Carousel Pool (`std::variant`)
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

// Global instance replaces `extern rdStation station;`
extern BoardCarousel carousel;
```

---

### 3.2 Stage 3: Drawing Framework Abstraction (The "True" Plugin Architecture)

While Stages 1 and 2 successfully decoupled the Memory and API Data Fetching into object-oriented plugins, the system remains in a "halfway" decoupled state regarding its display routines.

Currently, the `Departures Board.cpp` main loop retains a `switch(boardMode)` statement that manually delegates to centralized animation loops (`departureBoardLoop()`, `undergroundArrivalsLoop()`, `busDeparturesLoop()`) located within the monolithic `DisplayEngine.hpp`. 

In these centralized loops, `DisplayEngine` actively inspects dataset properties (like `getStationData()->service[0].opco`) to construct strings and manage local static state variables like `yScrollPos` for scrolling animations. This breaks encapsulation: if an integrater wants to add an entirely new screen type (e.g., an Airport Departures board), they still must modify the core Framework files rather than just dropping a new class into `lib/boards`.

#### The Rationale for Stage 3
To achieve a **True Plugin Architecture**, the core framework should be completely agnostic to *what* data is being fetched, *what* is being drawn, and *how* it animates. 

We must dissolve the legacy global `rdStation` mega-struct entirely. Each board plugin must define its own bespoke data structure tailored strictly to its needs (e.g., `NationalRailStation` with a `platform` field vs. an ultra-lean `BusStop` struct).

Furthermore, the rendering layout logic (e.g., drawing `timeToStation` vs `isDelayed`) must move out of the central `Departures Board.cpp` main loop. The framework simply commands the active board to calculate its own next frame state (`tick()`) and output its own pixels based on its own data (`render()`).

#### Critical Assessment

**The Pros:**
1. **Authenticity & Flexibility:** Currently, all boards are forced into a rigid 3-row layout inherited from National Rail. By moving `render()` into the plugins, TfL could implement a denser 4-row dot matrix, or the Bus board could render large custom graphics, completely independent of the other modes.
2. **Absolute Decoupling:** Integrating a new board type requires zero changes to `DisplayEngine.hpp` or `Departures Board.cpp`. The new class simply implements `tick()` and `render()` and is added to the Carousel wrapper.
3. **Maintainability:** Debugging an animation glitch on the TfL board no longer requires parsing through a monolithic 1,500-line `DisplayEngine.hpp` file; the logic is isolated within `tflBoard.cpp`.

**The Cons:**
1. **Refactoring Overhead:** Gutting complex, coupled animation states out of global scope and cleanly wrapping them into the respective board classes is a highly invasive refactoring effort.
2. **Loss of Shared Math:** All three existing boards share math for the bottom-row scrolling text animation. Moving layout logic out of the `DisplayEngine` into the plugins means duplicating `yScrollPos` scrolling algorithms into three distinct `render()` implementations.

#### The Architectural Impact
By executing Stage 3:
* **`iDisplayBoard` Interface** discards legacy monolithic structures. It simplifies from generic column math to pure lifecycle hooks: `virtual void tick(uint32_t currentMillis) = 0;` and `virtual void render(U8G2& display) = 0;`.
* **Memory footprint** becomes 100% efficient. A Bus board no longer provisions empty arrays for Train carriages.
* **`loop()` simplifies to:**
  ```cpp
  carousel.getActiveBoard()->tick(millis());
  u8g2.clearBuffer();
  carousel.getActiveBoard()->render(u8g2);
  u8g2.sendBuffer();
  ```
This represents the logical conclusion of the Object-Oriented refactor, transforming the project from a hardcoded 3-mode train departures board into a generic, endlessly extensible "Smart Carousel Display".

---

## 4. Configuration Storage & Web UX Impact (Stage 4 & 5)

If the device supports 5 boards, the Web GUI must allow users to configure all 5, which means significant changes to how settings are saved and presented.

### 4.1 Storage Changes (`config.json`)
Currently, `loadConfig()` (in `modules/displayManager/displayManager.hpp`) parses a flat JSON structure because the board only has one global mode:
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
    { "type": 0, "id": "PAD", "lat": 51.517, "lon": -0.176, "weather": true },
    { "type": 2, "id": "490000077E", "lat": 51.503, "lon": -0.005, "weather": true },
    { "type": 1, "id": "940GZZLUKSX", "lat": 0, "lon": 0, "weather": false }
  ]
}
```
> [!NOTE]
> The `weather` toggle and `lat`/`lon` coordinates are now stored per-board. This allows different transport modes (which may be miles apart) to fetch accurate local weather data.

### 4.2 Schema Migration (Backward Compatibility)
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

### 4.3 Web GUI UX Changes (`index.htm`)
The current web interface (`web/index.htm`) acts like a traditional HTML `<form>`, with hardcoded `<input>` fields for the National Rail CRS, the Tube Naptan ID, etc., mapped to the singular file state.

**The New UX**:
1. **Dynamic Form Generation**: The JavaScript within the GUI must loop over the `boards` array from `config.json` and dynamically render a UI "Card" for each configured board.
2. **Add/Remove Functionality**: There must be an "Add Board" button that appends a new board card to the UI (up to `MAX_BOARDS`). There should also be a "Delete" button on each board's card.
3. **Mode Selection Dropdown**: Inside each Board Card, the first option must be a "Transport Mode" dropdown (National Rail/Tube/Bus). When changed, the JavaScript should hide/show the relevant input fields *for that specific card* (e.g., hiding the Tube Naptan field when National Rail is selected).
4. **JSON Serialization**: When the user clicks "Save Settings", the web client must serialize all the dynamically generated board cards back into the `boards: []` JSON array and `POST` it to `/savesettings`.

By shifting the Web GUI to a dynamic JavaScript UI, the user can visually build their custom "Carousel" of boards, and the `config.json` accurately reflects the memory slots needed at boot time.

---

## 5. Architectural Benefits Deep Dive

### 5.1 Future API Extensibility (e.g. National Rail REST)

A major advantage of this architecture is how easily it adapts to changing external APIs. Soon, National Rail will require migrating from the legacy XML SOAP (`raildataXmlClient`) endpoint to a modern REST endpoint.

In the old "Mega Struct" model, changing the engine meant risking breakage across the entire global `rdStation` state and the `Departures Board.cpp` rendering loop because they were tightly coupled.

With the **Carousel Architecture** using pure interfaces (`iDisplayBoard`), adding the new API requires zero modifications to the display logic:

1. **Create a New Implementation**: You build a new `TrainStationRest` class that inherits from `iDisplayBoard`. It implements `updateData()` using an HTTP JSON parser instead of XML.
2. **Inherit Rendering logic**: It either implements its own `render()` method independently, or they share common components.
3. **Update the Factory**: In `loadConfig()`, when a user selects "National Rail (REST)" in their web configuration, the board instantiates `TrainStationRest(crs)` into the Carousel slot instead of the SOAP version.

Because the main `loop()` exclusively calls `getActiveBoard().updateData()` and `getActiveBoard().render()`, it never knows (or cares) if the underlying board fetched data via XML SOAP, JSON REST, or even from a local Bluetooth source. The contract is purely visual, making API transitions completely risk-free.

### 5.2 UI Extensibility: Mode-Specific Rendering

A critical benefit of moving the UI drawing routines directly into the `iDisplayBoard` subclasses (e.g., `render()`) is the elimination of massive `switch(boardMode)` blocks from the main rendering loop.

**1. Reclaiming Pixel Space:**
The 256x64 OLED screen is severely constrained. In the legacy "mega struct" design, drawing logic was heavily reused. This meant that `MODE_BUS` had to navigate a physical screen layout designed for trains. 

If `BusStation` implements its own isolated `render()` routine, it can omit the Platform column entirely. It reclaims those 30 pixels to allow incredibly long bus destination names without truncation, tailoring the layout specifically to the transport mode without breaking the Train layout.

**2. Streamlining the Core Loop:**
The sprawling `Departures Board.cpp` display formatting logic shrinks down to a clean, generic contract. This pushes all complex, mode-specific pixel math down into the concrete classes where it belongs, fully adhering to the **Single Responsibility Principle**.

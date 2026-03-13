# Architecture Evaluation: `iDisplayBoard`, `iGfxWidget`, & `iDataSource`

Based on our discussion, here is the refined architectural direction for the display boards. We are moving towards a **Composition over Inheritance** model, leveraging reusable UI Widgets.

## 1. The Core Abstraction: `iDisplayBoard`
Your idea to introduce a single interface for all screens remains solid. It unifies the state machine in `main.cpp`/`displayManager`.

**Critical View on the 'I' Prefix for Interfaces**: Prefixing interfaces with an 'I' (like `IStation`, `IDisplayBoard`) is a deeply ingrained and helpful standard in C++ and Object-Oriented Programming (like COM or C#). It clearly communicates to developers that a class is a pure abstract contract with no implementation. To merge this standard with the project's strict `camelCase` requirement, I recommend using a lowercase `i` (e.g., `iDisplayBoard`, `iGfxWidget`, `iDataSource`). This preserves the distinct interface naming convention while honouring the camelCase rule!

```cpp
class iDisplayBoard {
public:
    virtual ~iDisplayBoard() = default;
    
    // Lifecycle hooks
    virtual void onActivate() = 0;   
    virtual void onDeactivate() = 0;

    // Main loop methods
    virtual void tick(uint32_t ms) = 0; 
    virtual void render(U8G2& display) = 0; 

    // Used by DisplayManager during blocking operations to keep UI animated
    virtual void animationTick(U8G2& display, uint32_t currentMillis) = 0;
};
```
Every specific screen (e.g., `busBoard`, `tflBoard`, `wifiSetupScreen`, `systemMessageScreen`) will inherit directly from `iDisplayBoard`. This flattens the class hierarchy and prevents unrelated boards from being overly entangled.

## 2. The Solution: Composition via UI Widgets
Instead of pushing common rendering variables (like `scrollStopsXpos`, `isScrollingStops`) up into a shared `transportBoard` base class—which risks future rigidity if the boards diverge—we will abstract these rendering components into standalone **Widget Classes**.

A Widget is responsible for maintaining its own state, variables, and rendering logic. A Board simply instantiates the required Widgets, positions them, and calls their `render()` and `tick()` methods.

### The `iGfxWidget` Interface
To standardise how widgets are drawn and updated, they should all inherit from a common interface. I completely agree with your feedback regarding the constructor! By requiring coordinates during widget instantiation, we ensure that a widget is never rendered in an invalid or unknown state. A `setGeometry` method could optionally be kept if you plan on creating dynamic layouts that move at runtime, but the constructor is definitively the right place for initialization.

```cpp
class iGfxWidget {
protected:
    int x, y;
    int width, height;
    bool isVisible = true; // Visibility toggle

public:
    // Specify the layout box. Width/Height are optional (-1 signifies "full screen" or "content-driven")
    iGfxWidget(int _x, int _y, int _w = -1, int _h = -1) : x(_x), y(_y), width(_w), height(_h) {}
    virtual ~iGfxWidget() = default;

    // Visibility control
    void setVisible(bool visible) { isVisible = visible; }
    bool getVisible() const { return isVisible; }

    // Standard loop hooks
    virtual void tick(uint32_t ms) = 0; 
    virtual void render(U8G2& display) = 0; 

    // Partial/Hardware accelerated update for smooth animations during blocking ops
    virtual void animationTick(U8G2& display, uint32_t currentMillis) {}
};
```

### Proposed Widget Classes (`lib/gfxUtilities`):
*   **`headerWidget`**: Renders the top bar, handles scrolling text if the location name is too long, and draws the destination text block. Embeds a `clockWidget` instance for time rendering.
*   **`clockWidget`**: Handles fetching the time and rendering the blinking colon and time string at the top right. Implements `animationTick` to safely execute partial hardware-accelerated screen updates independent of the main `render()` loop.
*   **`serviceListWidget`**: Renders a list of departures. It will be designed to support a configurable number of columns and alignments (e.g., passing in an array of column widths/alignments, so it can handle a 2-column layout for bus routes and expected times, or a 3-column layout for train platform, destination, and due time). 
*   **`scrollingMessageWidget`**: Handles horizontal marquee scrolling logic for service messages or calling points. Implements `animationTick` to maintain scroll fluidity during background network loads.
*   **`systemMessageWidget`**: A widget to standardize the rendering of full-screen alerts (e.g., Wi-Fi setup instructions, token errors, CRS warnings), ensuring visual consistency. It will support a title and multiple lines of formatted text, replacing the redundant layout logic currently scattered throughout `systemBoard.cpp`.

## 3. Example Structure of a Specific Board

With this approach, a `busBoard` becomes very lean. It acts merely as a controller that feeds data into its Widgets.

```cpp
class busBoard : public iDisplayBoard {
private:
    BusStop stationData;
    
    // UI Composition! The board owns these widgets.
    headerWidget header;
    clockWidget clock;
    serviceListWidget serviceList;
    scrollingMessageWidget statusTicker;

public:
    // Initialize the widgets with their screen bounding boxes
    busBoard() 
        : header(0, 0, 256, 16),
          clock(200, 0, 56, 16),
          serviceList(0, 16, 256, 32),
          statusTicker(0, 48, 256, 16) {}

    void onActivate() override {
        // Reset scrolling states when the screen appears
        statusTicker.resetScroll();
        serviceList.resetPosition();
    }

    void tick(uint32_t currentMillis) override {
        // Delegate timing to the widgets
        clock.tick(currentMillis);
        statusTicker.tick(currentMillis);
        serviceList.tick(currentMillis);
    }

    void render(U8G2& display) override {
        // Assemble the screen
        header.render(display, stationData.location);
        clock.render(display);
        
        // Pass the subset of data to the relevant widgets
        serviceList.render(display, stationData.service, stationData.numServices);
        statusTicker.render(display, getLastErrorMsg());
    }
};
```

## 4. The Separation of Data: `iDataSource`
Apologies for missing this out! You are absolutely right; returning to the idea of a separate data source interface aligns perfectly with the **Single Responsibility Principle**. 

The UI (`iDisplayBoard`) shouldn't care *how* data is fetched, it only consumes the structural struct. Creating an `iDataSource` cleanly abstracts the backend logic.

```cpp
class iDataSource {
public:
    virtual ~iDataSource() = default;

    // Fetch new updates from the remote network
    virtual int updateData() = 0;

    // Retrieve global error states
    virtual const char* getLastErrorMsg() const = 0;
};
```

A board (like `busBoard`) will be constructed by passing it a reference to its specific `iDataSource` (e.g., `busDataClient`). The board handles the UI lifecycle, calling `updateData()` when necessary, and distributing the resulting parsed struct/vars locally to the `iGfxWidget`s for rendering. 

### Data Structures & Messages
We will **not** enforce generic shared types (like `stnMessages` or generic `serviceData` structs) across all boards. Instead, each individual board will define and manage the data structures it needs for its specific data sources. 

The `iDataSource` interface only guarantees that data can be updated. The `iDisplayBoard` holds a pointer to its specific client, asks it for its parsed custom struct, and feeds standard types (e.g., `const char*`) into agnostic `iGfxWidget` classes (like `scrollingMessageWidget`).

## 5. Non-Blocking Display Updates (Yield Callbacks)
A significant challenge when running on a single core (e.g., using `xmlListener`) is that large payload parsing blocks the main `loop()` for several seconds. To keep the UI responsive (e.g., the clock blinking, text scrolling), we implement a pipeline based on **Yield Callbacks**:

1. **Parser Yield**: The data source parsers are configured to fire a callback periodically during execution.
2. **Display Manager `yieldAnimationUpdate()`**: This method intercepts the callback. It maintains an internal frame-rate limiter (capping execution to ~60fps / 16ms intervals). If the threshold is met, it calls `renderAnimationUpdate()` on the active `iDisplayBoard`.
3. **Widget Delegation**: The board propagates `renderAnimationUpdate()` to any configured widgets. Valid widgets (like `clockWidget` or `scrollingMessageWidget`) calculate their *internal* deltas. If a visual change is needed, they execute a highly localized, hardware-accelerated **partial update** directly to the SPI bus (`display.updateDisplayArea()`), completely independent of the bulky screen-wide `render()` method.

### Guidelines: `tick()` vs `renderAnimationUpdate()`
When building widgets, it is crucial to understand the separation of these two methods to prevent screen tearing and SPI contention:

#### The "99% Rule": Use `tick()`
If your widget is part of the normal screen layout (the system is fully loaded and `displayManager.tick()` is running freely in the main loop):
*   **Always put your state-change logic in `tick()`.**
*   Example: Is it time to flash an icon? Did 5 seconds pass to rotate a page? Need to increment `scrollX++`? Let the math run here.
*   **Why?** Because the main loop will safely follow your `tick()` with a unified, synchronized, full-screen `render()` call.

#### The "Emergency Hatch": Use `renderAnimationUpdate()`
You **only** use `renderAnimationUpdate()` if the widget contains an element that *must remain physically animated* while the main ESP32 processor loop is completely blocked (e.g. downloading 100KB of XML). 
*   **Primary Use Cases**: The system clock, scrolling marquee text, or loading spinners.
*   **When does it execute?** Only during network yields.
*   **How does it work?** 
    1. It should immediately call `tick(currentMillis)` to deduplicate all math and state updates.
    2. It compares the new state to the old state (e.g. `scrollX != oldScrollX`).
    3. If the state changed, it locally renders *itself* to the RAM buffer, and uses `display.updateDisplayArea(x, y, w, h)` to push *only that tiny modified square of pixels* to the OLED.
*   **WARNING**: You must be extremely precise with your bounding boxes to avoid corrupting the frozen screen around the widget.

## Summary of Benefits
1.  **Future-Proof Flexibility:** If `tflBoard` decides to radically change its layout in the future, it just swaps out or stops using certain widgets. It avoids the fragile base class problem.
2.  **Highly Testable & Reusable:** A `headerWidget` can be tested in isolation. It can be easily reused if you ever decide to add a new display type (like a Tram board).
3.  **Declutters `systemBoard`:** We can replace the dozen standalone functions (`showWifiSetup()`, `showTokenError()`) with simple classes that instantiate a `systemMessageWidget` and pass it the relevant string constants.
4.  **Responsive UI:** The `renderAnimationUpdate` pipeline guarantees that critical animations survive long-running blocking operations without the immense complexity of managing thread synchronization across multiple FreeRTOS cores.

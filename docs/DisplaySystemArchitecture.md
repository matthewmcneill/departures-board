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

public:
    // Specify the layout box when creating the widget
    iGfxWidget(int _x, int _y, int _w, int _h) : x(_x), y(_y), width(_w), height(_h) {}
    virtual ~iGfxWidget() = default;

    // Standard loop hooks
    virtual void tick(uint32_t ms) = 0; 
    virtual void render(U8G2& display) = 0; 
};
```

### Proposed Widget Classes (`lib/gfxUtilities`):
*   **`headerWidget`**: Renders the top bar, handles scrolling text if the location name is too long, and draws the destination text block.
*   **`clockWidget`**: Handles fetching the time and rendering the blinking colon and time string at the top right.
*   **`serviceListWidget`**: Renders a list of departures. It will be designed to support a configurable number of columns and alignments (e.g., passing in an array of column widths/alignments, so it can handle a 2-column layout for bus routes and expected times, or a 3-column layout for train platform, destination, and due time). 
*   **`scrollingMessageWidget`**: Handles horizontal marquee scrolling logic for service messages or calling points.
*   **`systemMessageWidget`**: A widget to standardize the rendering of full-screen alerts (e.g., Wi-Fi setup instructions, token errors, CRS warnings), ensuring visual consistency across all `systemBoard` states.

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

## Summary of Benefits
1.  **Future-Proof Flexibility:** If `tflBoard` decides to radically change its layout in the future, it just swaps out or stops using certain widgets. It avoids the fragile base class problem.
2.  **Highly Testable & Reusable:** A `headerWidget` can be tested in isolation. It can be easily reused if you ever decide to add a new display type (like a Tram board).
3.  **Declutters `systemBoard`:** We can replace the dozen standalone functions (`showWifiSetup()`, `showTokenError()`) with simple classes that instantiate a `systemMessageWidget` and pass it the relevant string constants.

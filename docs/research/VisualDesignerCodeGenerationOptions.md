# Visual Designer Code Generation Architectures

**Date:** March 2026  
**Context:** Research and evaluation into the architecture required to support a "Visual Designer" tool for the Departures Board platform. The goal is to allow users to build display layouts visually, exporting C++ files that the ESP32 firmware consumes seamlessly, without overwriting custom developer logic.

---

## The Core Challenge
When a visual designer generates C++ code, it poses a structural problem: **Where does the handwritten business logic live?** 

If a Python script exports the entire `tflBoard.cpp` file—including the fetching of TfL API data and standard routines—any custom logic written by a developer to debug or improve that board will be permanently overwritten the next time the layout is re-exported.

To solve this, the execution of the UI layout (the visual boxes) must be structurally segregated from the Controller (the network logic). Three core patterns for achieving this on embedded systems were evaluated.

---

## Option 1: The Pure `.inc` (C Includes) Pattern

The `.inc` pattern is the simplest, lowest-friction method for injecting visual designer layouts into handwritten C++ code. It avoids complex Object-Oriented paradigms entirely.

### Approach
The Python generation script does not generate full C++ classes. Instead, it generates tiny `.inc` files containing literal text snippets of C++ code (e.g., `decl.inc`, `initList.inc`, `tick.inc`, `render.inc`). The developer manually injects these snippets into their unified handwritten class using the `#include` preprocessor.

The `TflBoard.cpp` naturally acts as a "God Class", directly executing `#include "tick.inc"` within its own `tick()` loop.

### Arguments For:
* **Absolute Simplicity:** No extra classes, no MVC layers, no virtual function overhead. 
* **Direct Widget Access:** Since the generated widgets are literally injected into the `TflBoard` class scope, the developer can manipulate them directly (`this->servicesWidget.addRow(...)`) without building complex routing logic.

### Arguments Against:
* **Destroyed Encapsulation:** The Board handles API networking, error state management, and holds bounds for every visual element on the screen. Single Responsibility is broken.
* **Multi-Layout Inflexibility:** You cannot swap layouts dynamically at runtime. Because the UI fragments are aggressively injected directly into the `.hpp` string by the compiler, supporting "Layout A" and "Layout B" simultaneously requires duplicating the entire API-fetching Controller class.
* **IDE & Debugging Friction:** Modern IDEs struggle to provide error-tracing for `.inc` snippets dropped maliciously into the middle of constructors.

---

## Option 2: The Model-View-Controller (Composition) Pattern

To fix the structural mess of `.inc`, the visual designer generates a standalone View class (`GeneratedTflView`). The developer handwrites the Controller (`TflBoard`).

### Approach
The designer strictly generates the View (`.hpp`/`.cpp`), which contains all widgets, X/Y plotting, and basic formatting. The Controller is a handwritten C++ class that *owns* the view: `GeneratedTflView view;`. The Controller fetches API data and decides *if* and *when* the view is allowed to draw itself.

### Arguments For:
* **Bulletproof Architecture:** The visual designer knows nothing about errors or network drops. If a WiFi connection drops, the Controller safely intercepts the process and draws an Error Board instead of the normal View.
* **Zero Overwrite Risk:** The designer purely regenerates the `View`, leaving developer logic totally untouched.

### Arguments Against:
* **Data-Binding Boilerplate:** If the designer places a new text widget, the Controller must manually write the route to it. This can be mitigated by injecting a global `DataDictionary` that the View polls automatically, but string-hashing inside a 30fps rendering loop introduces dangerous CPU jitter constraints.
* **Memory Headroom Risk:** Allocating massive hierarchical view objects and Controllers creates twin-peaks on the ESP32’s limited heap/stack.

---

## Option 3: The Lightweight Generated View (LGV) Pattern

This is the ultimate architectural compromise and the industry standard for embedded UI frameworks like ST's TouchGFX. 

### Approach
The script generates a single standalone class (`GeneratedNrView.hpp`). However, it treats the generated class as a "dumb box" containing **public** widgets. It does not attempt to architect data-binding dictionaries.

```cpp
// GENERATED
class GeneratedNrView : public iBoardView {
public:
    headerWidget headWidget; // Explicitly PUBLIC
    serviceListWidget servicesWidget;
};

// HANDWRITTEN CONTROLLER
void NationalRailBoard::tick(uint32_t ms) {
    updateData(); 
    // Developer pushes data directly via IntelliSense autocomplete
    view->servicesWidget.addRow(data->destination);
    view->tick(ms); 
}
```

### Arguments For:
* **Solves the `.inc` Flaws:** It restores Encapsulation (Controller vs View) and prevents IDE tracing issues. Most importantly, it completely solves the *Multi-Layout Nightmare*. Because standard C++ classes are generated, the layout Controller can easily instantiate `new GeneratedNrViewA()` or `new GeneratedNrViewB()` natively at runtime without duplicating network request logic.
* **Solves the MVC Flaws:** By utilizing public widgets, the developer types exactly one line of explicit, type-safe C++ getter/setter logic instead of constructing heavy Hash-Map bindings. 

---

## Final Decision & Implementation Nuances

**Conclusion: We are proceeding with the Lightweight Generated View (LGV) pattern.** It provides the optimal balance of C++ runtime flexibility, ESP32 memory safety, and visual designer ease-of-use.

### Handling "The Optional Widget Problem"

When using the LGV pattern, if a Controller (e.g. `TflBoard`) expects to call `view->clockWidget.setTime()`, it will immediately crash the C++ compiler if the designer failed to place a Clock onto the layout canvas for that specific export.

We evaluated two mitigations:
1. **The MVP Interface pattern:** We entirely restrict the Controller to an interface (`iNrView->updateTime(t)`), meaning unused widgets simply drop the payload on the floor.
2. **The Dummy Widget approach:** We generate *all* known widgets inside the generated class, but set `isVisible = false` for the ones the designer did not actively place.

**The Verdict on Optional Widgets:** 
We have elected to proceed with the **`isVisible = false` approach**. 
While it technically provisions unused bytes in RAM for hidden dummy widgets, it is exponentially faster to implement and avoids overly restrictive Interface definitions. The `clock.tick()` natively checks `isVisible` before execution, costing near-zero CPU cycles. We accept the small footprint penalty in exchange for immense simplicity of codebase setup. If RAM constraints on the ESP32-S3 later emerge as an issue, the architecture can trivially be upgraded to a Null Pointer or MVP Interface strategy without halting development momentum.

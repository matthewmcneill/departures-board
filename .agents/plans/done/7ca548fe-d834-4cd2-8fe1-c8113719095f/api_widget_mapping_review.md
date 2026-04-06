# API to Widget Mapping Strategy Review (Granular High-Fidelity Toolkit)

## 1. Deep Field Review & Mapping Strategy

Based on our review of the online API schemas (TfL, National Rail, Bus) and the requirement for **maximum display layout variance**, we must move forward with a **"Widget Super-Set"** (or Granular Toolkit) approach. 

To mimic real-world boarding screens accurately, the JSON layout engine must be able to address and physically place any individual piece of API data on the screen grid independently.

Here is the comprehensive map of API endpoints to discrete, addressable UI elements for the layout designer:

### A. Identifier & Target Widgets
*   **`LineWidget` / `OperatorWidget`**: Binds to TfL `lineName`, Bus `RouteNumber`, or NR `operator`.
*   **`DestinationWidget`**: Binds to the primary terminating location.
*   **`ViaWidget`**: Binds to secondary routing nodes (NR `via`, or TfL `towards`).
*   **`VehicleIdWidget`**: Binds to explicit Fleet Numbers or License Plates (Bus `VehicleRef`, TfL `vehicleId`) when space allows.

### B. Spatio-Temporal Widgets
*   **`ScheduledTimeWidget`**: Displays strict timetable strings (`sTime`).
*   **`ExpectedTimeWidget`**: Displays live countdowns (`timeToStation`, `expectedTime`, `etd`).
*   **`UtilityNumberWidget`**: For displaying sequential ordering (`orderNum`) or physical locations (`platform`).

### C. Contextual & Disruption Widgets
*   **`CurrentLocationWidget`**: A floating text widget that binds to highly specific tracking strings (e.g., TfL's *"Between Bank and St. Paul's"*).
*   **`DisruptionReasonWidget`**: A long-form scrolling block binding to NR `delayReason` or Bus `InCongestion` statuses.

### D. Bespoke Graphical Exceptions
*   **`TrainFormationWidget`**: A dedicated visual drawing widget explicitly handling the complex NR `formation` schema to graphically render carriage lengths, loading bars, and onboard toilet/disabled access overlays.

## 2. Evaluation of the "Versatile Super-Set" Approach

### Core Advantage: Unrestricted Realism
The primary strength of the Widget Super-Set is that the display designer is not locked into a "sterile" tabular format. If a specific National Rail LED board places the "via" text *below* the destination rather than next to it, or places the Platform number in a disconnected box on the far right, the JSON layout engine can replicate this exactly. 

### Resolving the Memory & Alignment Constraints
Previously, I expressed concern that instantiating 30 unique C++ classes (like a `ViaLabelClass`, a `DestinationLabelClass`) would bloat the ESP32 RAM over a strictly tabular `serviceListWidget`. 

To achieve the layout versatility you want *without* the memory overhead, the "Widget Super-set" should be architected using a **Data Binding** model rather than deep class inheritance.

**Proposed Implementation Strategy for Versatility:**
Instead of independent C++ classes, the JSON parser uses a universal `MappedLabelWidget`. The JSON defines the X, Y, W, H, and Font, and then provides a `data_binding_key` (e.g., `"data": "NR_via_line_1"`). 
The `MappedLabelWidget` holds nothing in RAM but a lightweight pointer directly to the `NR_via` buffer in the `NationalRailStation` struct. 

This gives the JSON layout *infinite* granular flexibility to place API strings anywhere on the X/Y coordinate plane, while actually being infinitely more memory efficient than maintaining standalone UI objects.

---

### Conclusion
By adopting a granular toolkit driven by the versatile JSON engine, we prioritize high-fidelity, board-specific layouts. We can map almost the entire breadth of the rich online APIs (from delay reasons to specific vehicle identifiers and train formations) into independent, addressable widgets for maximum design freedom.

# TfL & Bus Board Memory Fix + 4-Column Layout

## 📍 Current State & Focus
The project is currently in the **Planning/Approval** phase. We have identified a critical memory corruption bug in the `TfLBoard` (stack-allocated string pointers) and are transitioning `TfLBoard` and `BusBoard` to a high-fidelity **4-column layout** (Order, Line/Route, Destination, Time). The design is architecturally optimized to use zero extra RAM by moving numbering logic into the `DataSource` classes with static string pointers.

## 🎯 Next Immediate Actions
1. **Approval**: Wait for user approval on the [implementation_plan.md](file:///Users/mcneillm/.gemini/antigravity/brain/2e1382f8-65d9-410b-bfaa-53dbcf9bc7bf/implementation_plan.md).
2. **Execute**:
    - Build numbering logic into `tflDataSource` and `busDataSource`.
    - Update `layoutDefault.json` for both boards.
    - Update `TfLBoard` and `BusBoard` controllers to use the new 4-column data flow.
    - Verify on hardware.

## 🧠 Decisions & Designs
- **Architectural Shift**: Business logic for service numbering is moved from the UI `Board` classes to the `DataSource` classes.
- **Memory Optimization**: Use `static const char* serviceNumbers[]` to provide stable pointers for position numbers (1-9), avoiding `snprintf` and heap/stack allocations.
- **Layout Standard**: Standardizing on 4 columns for Tube and Bus to match real-world London Underground/Bus displays.

## 🐛 Active Quirks, Bugs & Discoveries
- **Bug**: `TfLBoard` currently uses a local `char ordinalDest[64]` on the stack, which corrupts the `serviceListWidget` after the function returns.
- **Quirk**: `BusBoard` currently doesn't show service numbers at all; it's being upgraded for consistency.

## 💻 Commands Reference
- `pio run` (Build firmware)
- `pio device monitor` (Verify serial logs for heap usage)

## 🌿 Execution Environment
- **Branch**: main (presumably)
- **Hardware**: ESP32 with OLED (Hardware Lock: NONE)

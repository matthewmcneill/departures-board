[x] Reviewed by `house-style-documentation` - passed
[x] Reviewed by `embedded-systems` - memory/power/security impact assessed
[x] Reviewed by `embedded-web-designer` - UI mockups included

# Upstream Features Implementation (B2.4-W3.1)

> [!WARNING]
> **WORK IN PROGRESS**: Not ready for implementation yet. Queued for later execution.

This plan details the integration of the upstream B2.4-W3.1 features into the specialized v3.0 object-oriented codebase.

## Goal

Integrate the latest validated upstream behaviors:
1. Custom Vanilla JS modal overlays for the web interface (to replace standard browser dialogs, without relying on Bootstrap 5).
2. Tube Mode: Integrate Line and Direction filtering seamlessly into our existing "Configure Board" modal rather than polluting the root settings page (as upstream does).
3. Rail Mode: Service ordinals (e.g. 2nd, 3rd) and "Last Seen" location rendering.
4. Display/Power: Complete OLED power-off during sleep mode via `displayManager::sleep()`, and scrolling pacing controls.
5. Misc: Tube weather integration and RSS prioritization.

## Review Required

- **Web GUI Flow**: Verify if the proposed layout for the vanilla modal overlays fits the existing Simple.css / Pico CSS lightweight aesthetic.

## Resource Impact Assessment

- **Flash/RAM**: The custom Vanilla overlay JS/CSS will add approximately ~500-800 bytes of gzipped payload to the `portalAssets.h` footprint. The new C++ configurations and parsing logic will consume an estimated ~1.5KB Flash and ~200 bytes heap RAM.
- **Power**: The implementation of "True OLED off" via `drawingPrimitives` during the sleep cycle will drastically improve hardware lifespan and power utilization compared to the screensaver.
- **Security**: Web inputs for new settings (Tube filters, booleans) must be strictly validated during deserialization. Care must be taken not to dump sensitive `apikeys.json` to the Serial monitor (as noted in recent configManager edits).

## Proposed Changes

---

### Embedded Web UI Design & Payload

Instead of importing a heavy Bootstrap 5 library, we will implement a custom, ultra-lightweight Vanilla JS overlay to be injected dynamically into the DOM.

#### [MODIFY] web/keys.htm / web/index.htm / portal generation

- Inject lightweight Vanilla JS into the base HTML or add a small utility script.
- Replace all `alert()` and `confirm()` calls with a generic `showModal(title, body, [buttons])` Promise-based utility.

**ASCII UI Mockup**
```text
+-------------------------------------------------+
|                                                 |
|  +-------------------------------------------+  |
|  | Departures Board                     [x]  |  |
|  |-------------------------------------------|  |
|  |                                           |  |
|  |  Are you sure you want to delete this     |  |
|  |  RSS Feed?                                |  |
|  |                                           |  |
|  |                 [ Cancel ]  [ Confirm ]   |  |
|  +-------------------------------------------+  |
|                                                 |
+-------------------------------------------------+
(Overlay dims the background with rgba(0,0,0,0.5))
```

---

### Core Hardware Configuration Model

#### [MODIFY] modules/configManager/configManager.hpp & .cpp
- Extend setting structures to include: `tfLLineFilter`, `tflDirectionFilter`, `showServiceOrdinals`, `showLastSeenLocation`, `waitForScrollComplete`, `turnOffOledInSleep`, `prioritiseRss`.

---

### TfL Data Mapping

#### [MODIFY] modules/displayManager/boards/londonUndergroundBoard/tflDataSource.cpp & .hpp
- Support extracting Line ID and Direction bounds.
- Inject weather data fetching logic dynamically when in Tube mode.

---

### National Rail Mapping

#### [MODIFY] modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.cpp & .hpp
- Extract "Last Seen Location" properties from the Darwin SOAP XML payload.

---

### Display Primitives & States

#### [MODIFY] modules/displayManager/displayManager.cpp & .hpp
- Add high-level `sleep()` and `wake()` methods to control overall OLED power state across all widgets and timelines securely.

#### [MODIFY] modules/displayManager/widgets/drawingPrimitives.cpp & .hpp
- Add explicit low-level hardware support for `screenOff()` and `screenOn()` bridging to U8g2 sleep commands, invoked exclusively by the `displayManager`.

## Verification Plan

### Automated Tests
- Run `npm test` inside `test/web` to validate the native vanilla modal implementation logic.
- Ensure `pio run` builds cleanly to verify asset minification and stringification scripts successfully bundle the newly added web scripts without pushing flash limits globally.

### Manual Verification
- Engage Touch sensor or sleep timeline to assert the OLED completely turns off.
- Verify through Web Interface the functionality of the new filtering rules.

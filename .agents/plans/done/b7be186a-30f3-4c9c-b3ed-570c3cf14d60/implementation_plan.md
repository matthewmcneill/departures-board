# Implementation Plan: System Tab Enhancements

This plan outlines the consolidation of global settings, a dynamic diagnostics dashboard, and optimized polling logic.

## 1. Specification & Design Goals

### Global Settings (Priority)
Consolidate remaining settings from legacy screens at the top of the tab:
- **Hostname & Timezone**: Core networking and locality.
- **Date Display**: Boolean toggle for on-screen date.
- **Scroll Suppression**: Disable animations and news feeds globally.
- **Refresh Rate**: Toggle for high-frequency API mode.
- **Display**: Brightness slider and Flip Screen toggle.
- **Note**: Sleep/Power schedules are being moved to a dedicated "Schedule" tab in a future migration.

### Real-Time ESP32 Diagnostics (Dynamic)
A premium diagnostics experience that adapts to available hardware:
- **Dynamic Content**: The `/api/status` endpoint will only return keys for metrics the current board supports. The UI will render bars based on the presence of these keys.
- **CPU Load**: Per-core usage estimation (CPU0/CPU1).
- **Micro-Climate**: Internal chip temperature (°C).
- **Memory (Heap)**: Total vs. Free heap visualization.
- **Storage (LittleFS)**: Total vs. Used bytes.
- **Uptime**: Human-readable duration.
- *Removed*: WiFi Signal Strength (already in WiFi tab).

### Firmware Management
A dedicated section for version tracking and updates:
- **Status Display**: Current version and build date.
- **Update Logic**: "Update Available" notification area with an `[ Update Now ]` button.
- **Background Checks**: `Automatically install updates` checkbox.

### System Safety
- **Factory Reset**: Must implement a "double-check" confirmation flow (e.g., "Are you sure?" -> "Are you *really* sure?").

### Polling Optimization
- **Active Tab**: Poll `/api/status` every 5 seconds.
- **Inactive Tab**: Stop background polling entirely.
- **Implementation**: Use the `Visibility API` and tab listeners.

---

## 2. ASCII Wireframe: System Tab

```text
+---------------------------------------+
|        SYSTEM & DIAGNOSTICS           |
+---------------------------------------+
| GLOBAL SETTINGS                       |
|                                       |
|  Hostname: [ DeparturesBoard        ] |
|  Timezone: [ Europe/London          ] |
|                                       |
|  [x] Show Date on Screen              |
|  [ ] Suppress Scrolling & Info        |
|  [ ] Increase API Refresh Rate        |
|                                       |
|  Flip Screen: [x]                     |
|  Brightness:  [----O----]             |
+---------------------------------------+
| HARDWARE STATUS (Dynamic)             |
|                                       |
|  CPU Core 0  [|||.......] 30%         |
|  CPU Core 1  [|.........] 10%         |
|  Memory      [|||||.....] 52% (124kB) |
|  Storage     [||||||||..] 81% (3.2MB) |
|  Chip Temp:  42.5°C                   |
|  Uptime:     12d 04h 22m              |
+---------------------------------------+
| FIRMWARE                              |
|                                       |
|  Current: v3.0.2 (Build 240317)       |
|  Status:  Update Available (v3.1.0)   |
|                                       |
|  [ Update Now ]                       |
|  [x] Automatically install updates    |
+---------------------------------------+
| [ REBOOT DEVICE ] [ FACTORY RESET ]   |
+---------------------------------------+
```

---

## 3. Proposed Changes

### Backend: System & Status API
#### [MODIFY] [webHandlerManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/webServer/webHandlerManager.cpp)
- Enhance `/api/status` to return dynamic keys (omit if unsupported):
  - `total_heap`, `cpu_0`, `cpu_1`, `temp`, `storage_total`, `storage_used`.
- Update config save/load for new global settings.

### Frontend: Web Portal UI
#### [MODIFY] [index.html](file:///Users/mcneillm/Documents/Projects/departures-board/portal/index.html)
- **Structure**: Move Global Settings to the top.
- **Logic**: 
  - Render hardware bars dynamically based on API response.
  - Implement 2-step confirmation for "Factory Reset".
  - Optimize polling frequency.

---

## 4. Verification Plan

### Automated Tests
- `tests/system_tab.spec.ts`: Verify dynamic rendering, reordered sections, and 2-step reset.

### Manual Verification
- Confirm reset safety logic works on-device.
- Verify status bars disappear if the backend omits the keys.

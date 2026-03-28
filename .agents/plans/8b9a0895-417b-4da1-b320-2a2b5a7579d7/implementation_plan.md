# Implementation Plan - Generalized Message Prioritization

This plan generalizes the prioritization logic within the `scrollingMessagePoolWidget` to support an "Interleaved Priority Message". This allows any board type (National Rail, Bus, TfL) to designate a specific string to appear between every other message in the background rotation.

## User Review Required

> [!IMPORTANT]
> This change modifies the core `scrollingMessagePoolWidget` used by all board types. While it defaults to existing behavior (no interleaving), it introduces a new "Priority Slot" that takes up 50% of the scroll time when populated.
> 
> [!NOTE]
> The "Calling Points" on National Rail will be the first feature to utilize this generalized slot.

## Proposed Changes

### [Component] Display Manager Widgets

Enhance the base `scrollingMessagePoolWidget` to handle interleaved priority logic internally.

#### [MODIFY] [scrollingMessagePoolWidget.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/widgets/scrollingMessagePoolWidget.hpp)
- Add `void setPriorityMessage(const char* msg)`: Sets the interleaved message.
- Add `char priorityMessage[512]` and `bool showPriorityNext` to private state.

#### [MODIFY] [scrollingMessagePoolWidget.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/widgets/scrollingMessagePoolWidget.cpp)
- Update `loadNextMessage()`:
    - If `priorityMessage` is set AND `showPriorityNext` is true:
        - Load `priorityMessage`, set `showPriorityNext = false`.
    - Else:
        - Load from pools, set `showPriorityNext = true` (if priority message exists).

---

### [Component] National Rail Board

Update the NR controller to use the new generalized priority slot.

#### [MODIFY] [nationalRailBoard.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/nationalRailBoard/nationalRailBoard.cpp)
- **`updateData()`**: Replace `msgWidget.setText()` with `msgWidget.setPriorityMessage()` for calling points.
- **`onActivate()`**: Ensure Station Messages pool is added before the Global pool.

---

### [Component] Design Mockup

```text
+-------------------------------------------------------------+
| 1st | 12:45 | London Waterloo    | Plat 1 | On Time        |
|-------------------------------------------------------------|
| 2nd | 12:55 | Woking             | Plat 2 | On Time        |
| 3rd | 13:05 | Basingstoke        | Plat 1 | On Time        |
|-------------------------------------------------------------|
| [ SCROLLING TICKER AREA ]                                   |
| Cycle 1: "Calling at: Surbiton, Weybridge, Woking..."       |
| Cycle 2: "Station Alert: Lift out of service at Plat 1"      |
| Cycle 3: "Calling at: Surbiton, Weybridge, Woking..."       |
| Cycle 4: "Weather: 12C Sunny"                               |
+-------------------------------------------------------------+
```

## Verification Plan

### Automated Tests
- No hardware tests available. Logic will be verified via visual inspection.

### Manual Verification
1. **National Rail**: Verify Calling Points appear between every RSS/Station message.
2. **Generic Boards**: Verify Bus/TfL boards still function normally without a priority message set (no gaps in rotation).
3. **Empty Priority**: Clearing the priority message should resume normal sequential pool rotation.

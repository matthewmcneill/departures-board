# Display Slot List Evolution

The goal is to modernize the "Active Displays" list by adopting an ordered "slot-based" list metaphor. This involves visually upgrading each item into a clickable card (slot), adding transport-specific logos, and removing redundant action buttons while maintaining a single-column ordered list.

## Proposed Changes

### [Web Portal] [index.html](file:///Users/mcneillm/Documents/Projects/departures-board/portal/index.html)

#### [MODIFY] UI Layout & Styles
- **Ordered List Layout**: Implement a single-column vertical list of `.board-slot` cards. This ensures the execution order (0-5) is visually clear.
- **Slot Design**: Create `.board-slot` cards (padding: 1.25rem, border-radius: 12px) that:
    - Act as a single button to trigger `app.editBoard(i)`.
    - Contain a transport logo on the left.
    - Display board name, mode, and ID in the center.
    - Feature a status dot and small reorder buttons (`↑` / `↓`) on the right.
- **Logos**: Implement `TRANSPORT_LOGOS` constant using the extracted SVGs:
    - **Rail**: National Rail double arrow (Blue).
    - **Tube**: TfL Underground roundel (Red ring / Blue bar, no text).
    - **Bus**: London Buses roundel (Solid Red ring / bar, no text).
    - **Clock**: Minimal clock icon.

#### [MODIFY] Logic & Rendering
- **`renderBoards`**:
    - Update to iterate through exactly `MAX_BOARDS` (6) slots.
    - Filled slots: Clickable card showing board details and transport logo.
    - Empty slots: Clickable "Add" card with a "+" icon.
    - Reorder logic: Maintain buttons but ensure they are positioned so they don't block the main click area of the slot.

## Verification Plan

### Automated Tests
- `npm test -- tests/boards_v2.spec.ts` to ensure core board operations (editing, saving, reordering) still work with the new UI structure.

### Manual Verification
- Verify the single-column ordered layout and consistent slot sizing.
- Confirm logos match provided designs exactly (Tube roundel, Bus roundel without text).
- Ensure clicking any part of a filled slot (except the `↑`/`↓` buttons) opens the editor.
- Verify "Add" logic via empty slots.

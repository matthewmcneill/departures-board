# Implementation Plan - Refine API Key Modal UI

Address visual and behavioral issues reported in the API key edit dialog:
1. Delete button layout and spacing.
2. Test button shape-shifting during loading.

## Proposed Changes

### Web Portal UI

#### [MODIFY] [index.html](file:///Users/mcneillm/Documents/Projects/departures-board/portal/index.html)

- **Delete Button Refinement**:
    - Re-add `.btn-delete` CSS to the `<style>` section.
    - Set `width: 100%`, `margin-top: 2rem`, and `padding: 0.75rem`.
    - Set `border: 1px solid var(--error)`, `color: var(--error)`, `background: transparent`.
    - Ensure it is clearly separated from the primary action buttons.
- **Test Button Stabilization**:
    - Add CSS for `#key-test-btn` to ensure consistent height and layout.
    - Use `display: inline-flex`, `align-items: center`, `justify-content: center` for centering the spinner and text.
    - Set `min-height` or a fixed `height` to prevent the button from growing/shrinking when text changes to "Testing...".
    - Use `gap: 0.5rem` for the spinner.

## Verification Plan

### Automated Tests
- `npx playwright test tests/portal.spec.ts`

### Manual Verification
1. **Modal Layout**:
    - Verify delete button spans the bottom and has 2rem margin above it.
    - Confirm test button does not jump or change height when clicked.
2. **Delete Flow**:
    - Confirm the browser `confirm()` dialog still appears.

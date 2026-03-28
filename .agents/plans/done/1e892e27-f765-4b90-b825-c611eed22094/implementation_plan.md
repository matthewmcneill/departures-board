# Reorder Portal Bottom Buttons and Add Schedule Tab

Reorder the bottom navigation buttons in the web portal to follow the user flow and add a new "Schedule" tab placeholder.

## Proposed Changes

### Web Portal

#### [MODIFY] [index.html](file:///Users/mcneillm/Documents/Projects/departures-board/portal/index.html)

- Reorder `<a>` tags in `<nav class="bottom-nav">`:
  1. WIFI (`tab-wifi`)
  2. KEYS (`tab-apikeys`)
  3. FEEDS (`tab-feeds`)
  4. DISPLAYS (`tab-displays`)
  5. SCHEDULE (`tab-schedule`) [NEW]
  6. SYSTEM (`tab-system`)
- Add a new `<section id="tab-schedule" class="tab-content">` before `tab-system`.
- (Optional) Reorder `<section>` blocks to match the navigation order.

## Verification Plan

### Automated Tests (Playwright)
- I will use the browser subagent to:
  - Load the portal.
  - Assert the order of links in `.bottom-nav`.
  - Click each link and verify the corresponding section becomes `.active`.
  - Verify the "Schedule" tab content is visible when selected.

### Manual Verification
- None required if automated tests pass.

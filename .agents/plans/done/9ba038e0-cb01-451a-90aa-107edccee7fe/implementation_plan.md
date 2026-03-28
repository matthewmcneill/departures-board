# Implementation Plan -# Stabilizing Test Button Size

Preventing layout shifts when "Test Feed" buttons enter the loading state.

## Proposed Changes

### Portal UI
#### [MODIFY] [index.html](file:///Users/mcneillm/Documents/Projects/departures-board/portal/index.html)

- **CSS**:
    - Update `.btn-test-action` to use `inline-flex`, `align-items: center`, `justify-content: center`, `gap: 0.5rem`, and `min-height: 44px`. This matches the approach used for the key dialog buttons.
- **JavaScript**:
    - Update `testWeather()` to set the button to a loading state (`<div class="loading-spinner"></div> Testing...`) and disable it during the fetch, just like `testFeed()`.
    - Ensure both `testWeather()` and `testFeed()` restore the button text and state in a `finally` block.

## Verification Plan

### Automated Tests
- None (On-device UI verification required)

### Manual Verification
- Rebuild portal assets via `portalBuilder.py`.
- Flash firmware.
- Open portal and click FEEDS tab.
- Click "Test Feed" for both News and Weather.
- Observe:
    - The buttons do not change size (width or height) when transitioning to the "Testing..." state.
    - Both buttons show the loading spinner.
- **Weather Logic**:
    - Update `testWeather()` to send the currently selected key's token if available (to support testing unsaved keys).

#### [NEW] [rss.json](file:///Users/mcneillm/Documents/Projects/departures-board/portal/rss.json)
## Verification Plan

### Manual Verification
- Rebuild and flash.
- Check FEEDS tab -> Weather Service:
    - Verify help text is in the right place.
    - Click "Test Feed" and verify success for the hard-coded location.
- Check FEEDS tab -> News Feed Service:
    - Verify dropdown is populated with items from `rss.json`.

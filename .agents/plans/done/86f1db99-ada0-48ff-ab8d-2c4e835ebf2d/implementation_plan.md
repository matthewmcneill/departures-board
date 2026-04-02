# Implementation Plan: Fix Feed Test Failures

Some feed tests (TfL, Weather, RSS) are failing or reporting incorrect status in the web portal. This plan addresses the logic errors in board testing and investigates the feed failures.

## Proposed Changes

### [Component] Display Manager (TfL)

#### [MODIFY] [tflDataSource.cpp](modules/displayManager/boards/tflBoard/tflDataSource.cpp)
- Update `testConnection` to normalize `UPD_NO_CHANGE` to `UPD_SUCCESS`. This ensures that if a board is tested and the data is identical to the previous fetch (e.g., 0 services found both times), it is reported as a success rather than a failure.

### [Component] Web Server

#### [MODIFY] [webHandlerManager.cpp](modules/webServer/webHandlerManager.cpp)
- Update `handleTestBoard` to treat both `UPD_SUCCESS` and `UPD_NO_CHANGE` as a successful test result. This provides a second layer of defense for all data sources.

### [Component] Weather Feed Testing

#### [MODIFY] [webHandlerManager.cpp](modules/webServer/webHandlerManager.cpp)
- In `handleTestWeather`, explicitly hardcode the coordinates for testing to **51.7487° N, 3.3816° W** (Pen-y-darren Ironworks).
- Update the diagnostic logging to reflect these coordinates are being used for the test.

#### [MODIFY] [index.html](portal/index.html)
- Update the UI text in the Feeds tab to say "Test Coordinates" instead of "Current Coordinates".
- Include the explicit lat/lon (51.7487° N, 3.3816° W) in the description text.

### [Component] News Feed (RSS) Testing

#### [MODIFY] [index.html](portal/index.html)
- Update `testFeed` and `renderFeeds` to handle the case where no feed is selected.
- If no feed is selected (`url` is empty), ensure the status dot remains **grey** and the status text says **NOT CONFIGURED**.
- Investigate why the portal reports an error while the serial port shows data returning (possibly a parsing or timeout issue in the web handler).

### [Component] Configuration & Feeds

#### [INVESTIGATE] Weather and RSS Failures
- The Weather feed is currently skipping because coordinates are `0,0`. I will check the configuration via the API to see if this is a user configuration issue or a default value that should be handled better.
- The RSS feed is returning `HTTP ERROR`. I will verify the URL and test with a known good feed (e.g., `http://feeds.bbci.co.uk/news/rss.xml`).

## Verification Plan

### Manual Verification
1. **Serial Console Monitoring:**
   - Run `pio device monitor --port /dev/cu.usbserial-5AA70033491 --baud 115200`.
   - Observe the `[WEB_API] Board test result` logs.
2. **Web Portal Testing:**
   - Open `http://192.168.86.152/portal`.
   - Navigate to the **DISPLAYS** tab and click 'Test' on the TfL board.
   - Verify it now shows a green 'Success' status even if no departures are found.
   - Navigate to the **FEEDS** tab and trigger a test for Weather and RSS.
   - Verify that updating coordinates (if they were 0,0) fixes the Weather test.
   - Verify that the RSS test passes with a valid URL.

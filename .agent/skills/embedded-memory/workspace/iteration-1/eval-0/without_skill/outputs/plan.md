# Implementation Plan: Refactor BusDataSource to ArduinoJson (Baseline)

## Goal
Replace the manual HTML scraping with `ArduinoJson` for more robust and maintainable data retrieval.

## Proposed Changes

### BusDataSource Refactor
- Include `ArduinoJson.h` in the source file.
- Update `updateData()` to fetch the payload from the API endpoint.
- Use `JsonDocument doc;` to parse the response.
- Access services via `doc["services"]` and map them to the existing `BusStop` structure.

## Verification
- Run the board and monitor the serial output for successful updates.
- Verify that arrivals matches expected values.

## Performance
- The refactor will simplify the code and make it easier to add new features or handle changes in the API response format.

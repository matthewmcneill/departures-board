# ADR: v3.1 Library Modernization

## Context
The project was exhibiting several compiler warnings regarding deprecated methods in `ArduinoJson` (v7.2.0) and `ESPAsyncWebServer`. Specifically:
- `JsonObject::containsKey()` is deprecated in favor of `.is<T>()` or `.isNull()`.
- `AsyncWebServerRequest::beginResponse_P()` is deprecated in favor of the unified `beginResponse()`.

## Decision
We decided to:
1.  **Refactor Code**: Update all occurrences of the deprecated methods to the modern equivalents recommended by the library maintainers.
2.  **Modernize Dependencies**: Update the `platformio.ini` to use the latest stable releases of core libraries (`ArduinoJson`, `U8g2`) and switch to the officially maintained `ESP32Async` organization fork of the web server.

## Consequences
- **Build Quality**: Eliminates 14+ compiler warnings, leading to a cleaner build log.
- **Maintainability**: Aligns the codebase with the modern ArduinoJson 7 API, reducing technical debt for future version upgrades.
- **Stability**: Leverages upstream bug fixes in the updated AsyncTCP/WebServer stacks, particularly beneficial for the ESP32-S3 target.

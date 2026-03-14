# Weather System Design

This document captures the architectural decisions and design evolution of the Departures Board weather system.

## 1. Architectural Evolution

### 1.1 From Global to Injectable
Historically, weather was a global service that updated a single shared string. This was insufficient for the new **Multi-Board Architecture** where different boards represent different geographical locations.

**Decision**: We moved to an **Injectable Weather Object (`WeatherStatus`)**. Each board instance in the carousel owns its own `WeatherStatus` object. This ensures:
- **Per-Board State**: Each board maintains its own weather data, coordinates, and refresh cycle.
- **Encapsulation**: The board manages its own display logic while the `WeatherClient` acts as a stateless service that updates these objects.

### 1.2 Configuration Standardization
To reduce semantic complexity and redundancy in the configuration schema:
- **`mode` -> `type`**: The term `mode` was renamed to `type` to better reflect the category of transport (Rail, Bus, Tube).
- **Unified Coordinates**: Various location-specific fields (e.g., `stationLat`, `busLat`) were standardized into simple `lat` and `lon` fields within the `BoardConfig`.

## 2. Data Representation

### 2.1 Condition IDs vs. Icon Codes
OpenWeatherMap (OWM) provides two ways to identify weather:
1. **Condition IDs (integers)**: Semantically rich numerical codes (e.g., 800 for Clear).
2. **Icon Codes (strings)**: 3-character codes (e.g., "01d") used for fetching web images.

**Decision**: We store the **Condition ID (`int`)** and a **Boolean `isNight` flag**. 
- The `int` is highly efficient for comparisons and lookup tables.
- The `isNight` flag is extracted from the OWM icon suffix ('d' or 'n').

### 2.2 Status Tracking
The `WeatherStatus` object includes a data status flag:
- `WS_NOT_CONFIGURED`: Default state.
- `WS_FETCH_ERROR`: Network or API failure.
- `WS_OK`: Data is current and valid.

## 3. Future Icon Rendering Assessment

For the hardware OLED (U8g2), we evaluated two strategies for rendering weather icons:

| Feature | Unicode/Fonts | XBM Bitmaps |
| :--- | :--- | :--- |
| **Storage Efficiency** | **Medium**. Requires embedding a font. Even a cut-down font includes overhead for character mapping. | **High**. Stores only the raw bits for the required icons. |
| **Rendering Performance** | **High**. Uses standard `drawGlyph` routines. | **High**. Uses standard `drawXBM` routines. |
| **Flexibility** | **Low**. Limited to the glyphs defined in the font. | **High**. Any custom graphic can be converted to XBM. |
| **Complexity** | **Low**. Simple text API. | **Medium**. Requires managing a library of arrays. |

### Recommendation: XBM Bitmaps
For a constrained environment like the ESP32 where Flash space is at a premium:
- **XBM is the winner**. We only need ~10 icons (Clear, Few Clouds, Rain, Thunder, etc.). 
- 10 icons at 16x16 pixels consume only **320 bytes** of Flash total. 
- Even a tiny custom font would likely consume 2-5 KB of Flash.

By storing the `conditionId`, the system is ready to map OWM categories directly to optimized XBM bitmaps in the future.

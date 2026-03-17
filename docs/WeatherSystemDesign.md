# Weather System Design

This document captures the architectural decisions and design evolution of the Departures Board weather system.

## 1. Architectural Evolution

### 1.1 From Global to Injectable
Historically, weather was a global service that updated a single shared string. This was insufficient for the new **Multi-Board Architecture** where different boards represent different geographical locations.

**Decision**: We moved to an **Injectable Weather Object (`WeatherStatus`)**. Each board instance in the carousel owns its own `WeatherStatus` object. This ensures:
- **Per-Board State**: Each board maintains its own weather data, coordinates, and refresh cycle.
- **Encapsulation**: The board manages its own display logic while the `WeatherClient` acts as a stateless service that updates these objects.
- **Resource Optimization**: Weather fetching is only performed for the **active board**. When the carousel switches, an immediate fetch is triggered for the new location if data is stale.
- **Feature Toggles**: A `showWeather` flag was added to `BoardConfig`. This allows disabling weather fetching (and saving display real estate) on a per-board basis. Tube boards default to `OFF` while Rail and Bus default to `ON`.

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
- `WS_NOT_CONFIGURED`: Default state or coordinates are explicit `0,0`.
- `WS_FETCH_ERROR`: Network or API failure.
- `WS_OK`: Data is current and valid.

### 2.3 Coordinate Validation
To prevent unnecessary network traffic and API-key consumption:
- The system checks if coordinates are `0,0` before attempting a fetch.
- If invalid, a `LOG_WARN` is issued, and the status is set to `WS_NOT_CONFIGURED`.
- This is particularly important for boards that haven't been configured via the Web UI yet.

### Implementation: Custom Icon Font (`WeatherIcons16`)

While XBM was initially considered, we implemented a **Human-Readable Font Source** pipeline:
- **Source**: `modules/displayManager/fonts/source/WeatherIcons16.txt` contains visual ASCII-art of the icons.
- **Pipeline**: A Python build script converts these to BDF and then to compressed U8G2 font arrays in `fonts.cpp`.
- **Mapping**: `WeatherStatus::getIconChar()` translates OWM condition IDs into font glyphs.

This approach balances the storage efficiency of XBM with the ease of use of a standard text API.

## 4. Non-Blocking I/O (Yield Mechanism)

The `weatherClient` participates in the system-wide **Yield Mechanism**:
- Long-running HTTP GET requests and JSON parsing loops periodically invoke a `yieldCallback`.
## 5. API Key Management & Testing

### 5.1 Multi-Key Selection Strategy
To allow for flexible API key management:
- **Registry Integration**: The system can now store multiple API keys in the Global Key Registry.
- **Specific Assignment**: A `weatherKeyId` is stored in the system configuration to map the weather service to a specific key from the registry.
- **Fallback Logic**: If no specific `weatherKeyId` is assigned, the client performs a scan of the registry for any key of type `owm`.

### 5.2 Test Overrides (Real-time Validation)
For the Web Portal "Test Feed" functionality, the `weatherClient` supports real-time overrides:
- **In-Memory Testing**: Users can test a new key *before* saving it to the registry by passing an `overrideToken` to the `updateWeather()` method.
- **Stateless Validation**: This allows the Web Handler to validate connectivity without mutating the flashed device configuration.

### 5.3 Diagnostic Status (Unconfigured)
A new state, `WS_UNCONFIGURED` (represented as a **GREY** dot in the UI), was added:
- **Immediate Feedback**: Transitioning to this state happens immediately in the UI when "No key selected" is chosen.
- **Semantic Clarity**: Distinguishes between a "failed" connection and a service that simply hasn't been set up yet.

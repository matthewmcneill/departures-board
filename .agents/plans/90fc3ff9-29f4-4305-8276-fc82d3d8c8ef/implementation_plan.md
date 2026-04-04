# Dynamic Attribution Architecture Refactor

This plan moves hard-coded attribution strings from the presentation layer (display boards) down to the data providers (`iDataSource`). This creates a polymorphic resolution suitable for dynamic provider switching (like transitioning between classic Darwin XML and RDM JSON without hardcoding UI logic).

## Proposed Changes

### iDataSource Interface

#### [MODIFY] iDataSource.hpp
Add a virtual method with a safe-default implementation to avoid breaking compilation of test mocks or un-migrated providers:
```cpp
    /**
     * @brief Gets the provider-specific semantic attribution string.
     * @return Attribution string, or nullptr if none.
     */
    virtual const char* getAttributionString() const { return nullptr; }
```

---

### Data Providers

#### [MODIFY] nrDARWINDataProvider.hpp & .cpp
- Override `getAttributionString()` to return `"Powered by National Rail Enquiries"`.

#### [MODIFY] nrRDMDataProvider.hpp & .cpp
- Override `getAttributionString()` to return `"Powered by Rail Delivery Group"`.

#### [MODIFY] busDataSource.hpp & .cpp
- Override `getAttributionString()` to return `"Powered by bustimes.org"`.

#### [MODIFY] tflDataSource.hpp & .cpp
- Override `getAttributionString()` to return `"Powered by TfL Open Data"`.

#### [MODIFY] weatherClient.hpp & .cpp
- Override `getAttributionString()` to return `"Weather by OpenWeatherMap"`.

#### [MODIFY] lib/rssClient/rssClient.hpp & .cpp
- Override `getAttributionString()` to return `"Custom RSS Feed"`. *(Note: RSS feed headlines are injected differently into global pools, so we may use this specifically for 'system booting' texts instead).*

---

### Display Boards

#### [MODIFY] nationalRailBoard.cpp
- Remove `const char nrAttribution[]`.
- Update `onActivate()` and `updateData()` missing-data states to fetch the text dynamically via `activeDataSource->getAttributionString()`.

#### [MODIFY] busBoard.cpp
- Remove `const char btAttribution[]`.
- Update fallback/empty message calls to use `dataSource.getAttributionString()`.

#### [MODIFY] tflBoard.cpp
- Remove `const char tflAttribution[]`.
- Update fallback/empty message calls to use `dataSource.getAttributionString()`.

## Open Questions

> [!IMPORTANT]  
> How do you want the Weather and RSS strings displayed?
> Currently, OpenWeather and RSS do not control the main scrolling display - they run in the background. Should we push their new attribution strings into the `globalMessagePool` once on boot, or append them during specific UI events?

## Verification Plan

### Automated Tests
- Build native using `pio run -e native` to ensure interface changes don't break mock overrides.
- Flash to hardware to evaluate the message injection logic.

### Manual Verification
- Swap API keys on the web portal between RDM and Darwin and verify the National Rail Board displays the correct new attribution strings immediately after reboot.

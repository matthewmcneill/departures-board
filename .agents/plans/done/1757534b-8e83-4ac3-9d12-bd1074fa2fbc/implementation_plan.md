# Unified iDataSource Testing Interface

The goal is to encapsulate connection and authentication testing logic within data source classes, replacing hardcoded logic in the web handlers and providing a reusable interface for diagnostics.

## Proposed Changes

### Display Manager Interfaces

#### [MODIFY] [iDataSource.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/interfaces/iDataSource.hpp)

Define the `iDataSourceTest` interface to standardize connection/auth verification.

```cpp
/**
 * @brief Interface for testing data source connectivity and authentication.
 */
class iDataSourceTest {
public:
    virtual ~iDataSourceTest() = default;
    
    /**
     * @brief Performs a lightweight connection and authentication test.
     * @param token Optional token to test (overrides stored configuration).
     * @return 0 for success (UPD_SUCCESS), non-zero for error (UPD_*).
     */
    virtual int testConnection(const char* token = nullptr) = 0;
};
```

### Data Source Implementations

#### [MODIFY] [nationalRailDataSource.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.hpp)
#### [MODIFY] [nationalRailDataSource.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.cpp)

- Inherit from `iDataSourceTest`.
- Implement `testConnection()` by performing an optimized Darwin lookup for a default station (e.g., "HWD").
- Encapsulate the "HWD" and SOAP address defaults within this method.

#### [MODIFY] [tflDataSource.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/tflBoard/tflDataSource.hpp)
#### [MODIFY] [tflDataSource.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/tflBoard/tflDataSource.cpp)

- Inherit from `iDataSourceTest`.
- Implement `testConnection()` by performing a single-result lookup for a default station (e.g., Bond Street).

### Web Server

#### [MODIFY] [webHandlerManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/webServer/webHandlerManager.cpp)

- Refactor `handleTestKey()` to use the new `iDataSourceTest` interface.
- Remove hardcoded station IDs and optimized configuration logic, delegating to the data source.

## Verification Plan

### Automated Tests
1. Verify that `pio test -e native` still compiles and runs.
2. Manually trigger key tests from the Web UI to confirm they still work (and fail correctly with invalid keys).

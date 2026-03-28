# National Rail API Key Validation Optimization

Implement a high-speed, minimal Darwin SOAP call to validate API keys without resource-heavy WSDL downloads or details parsing.

## Proposed Changes

### [displayManager]

#### [MODIFY] [nationalRailDataSource.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.cpp)
- **Token Corruption Fix**: Switch to `strlcpy` for token handling.
- **v12 Payload**: Update `data` and `soapAction` to use the 2021 namespace and `GetDepartureBoard` with `numRows=1`.

#### [MODIFY] [tflDataSource.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/boards/tflBoard/tflDataSource.cpp)
- **Lightweight Auth**: Implement `setTestMode` to perform a minimal Victoria Line status check instead of a full station pull.

### [webServer]

#### [MODIFY] [webHandlerManager.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/webServer/webHandlerManager.cpp)
- **National Rail Test**: 
    - Use `/OpenLDBWS/ldb12.asmx` as the endpoint.
    - Use `PAD` (Paddington) for the validation query.

## Verification Plan

### Automated Tests
- Build and flash (Verified compilation).
- Manual `curl` verification (Success with 200 OK and data).

### Manual Verification
- Verify the "Key validated successfully!" message appears in the portal.
- Confirm Serial logs show the 200 OK response from `ldb12.asmx`.

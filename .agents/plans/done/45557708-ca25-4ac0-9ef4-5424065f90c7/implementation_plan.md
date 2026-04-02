# National Rail API Key Validation Optimization

Implement a high-speed, minimal Darwin SOAP call to validate API keys without resource-heavy WSDL downloads or details parsing.

## Proposed Changes

### [displayManager]

#### [MODIFY] [nationalRailDataSource.cpp](modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.cpp)
- **Minified Payload**: When explicitly configured with a SOAP address (test mode), `updateData` will:
    - Target `ldb11.asmx`.
    - Use `GetDepartureBoardRequest` (Standard, not "WithDetails").
    - Omit XML prefixes (`soap-env:`, `ns1:`) to save bandwidth.
    - Set the Token namespace to `http://thalesgroup.com/RTTI/2013-11-28/Token/types`.
    - Set the Body namespace to `http://thalesgroup.com/RTTI/2017-10-01/ldb/`.
    - Use legacy `SOAPAction`: `http://thalesgroup.com/RTTI/2012-01-13/ldb/GetDepartureBoard`.

### [webServer]

#### [MODIFY] [webHandlerManager.cpp](modules/webServer/webHandlerManager.cpp)
- **National Rail Test**: 
    - Use `/OpenLDBWS/ldb11.asmx` as the endpoint.
    - Use `PAD` (Paddington) for the validation query.

## Verification Plan

### Automated Tests
- Build and flash. 
- Trigger "Test" in the portal.

### Manual Verification
- Verify the "Key validated successfully!" message appears in the portal.
- Confirm Serial logs show the 200 OK response from `ldb11.asmx`.

---
name: "Unified iDataSourceTest Interface & API Key UI Refactoring"
description: "Standardized the way external data sources verify their connectivity and authentication credentials by introducing a `testConnection` method to the `iDataSource` interface. Refactored `WebHandlerManag..."
created: "2026-03-17"
status: "DONE"
commits: ['7573fe2']
---

# Summary
Standardized the way external data sources verify their connectivity and authentication credentials by introducing a `testConnection` method to the `iDataSource` interface. Refactored `WebHandlerManager` to encapsulate and delegate testing logic directly to the individual data source implementations.

## Key Decisions
- **Interface Segregation**: Added `virtual int testConnection(const char* token = nullptr)` to `iDataSource`, making data sources responsible for their own internal test capabilities rather than hardcoding tests in the web handler.
- **National Rail Optimization**: Used the lightweight V12 payload and the default "PAD" CRS code to quickly validate the National Rail token without pulling the 500KB WSDL.
- **TfL Validation**: Implemented token validation for TfL using the `/Line/victoria/Status` endpoint as a lightweight test vector.
- **Bus Non-Auth Path**: Allowed `busDataSource` to bypass authentication checks by immediately returning success, as bustimes.org does not currently require tokens.
- **House Style Assurance**: Guaranteed documentation standards by appending mandatory Doxygen headers for all newly created methods.

## Technical Context

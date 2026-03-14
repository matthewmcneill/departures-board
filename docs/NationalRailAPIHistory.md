# National Rail API History

This document tracks the different API endpoints used for National Rail services (autocomplete and live data) to ensure we can revert or retry if needed.

## Station Picker (Autocomplete)

### Active (Working - March 2026)
**Endpoint:** `https://ojp.nationalrail.co.uk/find/stationsDLRLU/{query}`
- **Usage:** National Rail Online Journey Planner (OJP).
- **Format:** Array of arrays `[ ["CRS", "Station Name", ...], ... ]`.
- **Notes:** Efficient and reliable for station identification. **CRITICAL CHANGE (March 2026)**: This endpoint stopped providing latitude/longitude coordinates (indices 7 and 8 now return `0.0`). 

### Coordinate Recovery (TfL Fallback)
**Endpoint:** `https://api.tfl.gov.uk/StopPoint/Search/{name}?modes=national-rail`
- **Usage:** Secondary lookup in the Web UI to recover coordinates for National Rail boards.
- **Notes:** The TfL API provides comprehensive coverage of all National Rail stations in Great Britain via the national NaPTAN database. 
- **Rationale**: Replaces the lost spatial data from the official NR picker, ensuring the weather system can function without requiring manual coordinate entry.

### Legacy (Recent Failure - March 2026)
**Endpoint:** `https://stationpicker.nationalrail.co.uk/stationPicker/{query}`
- **Status:** Failing with 400/500 errors.
- **Notes:** Previously used for direct search proxying.

### Bulk Alternative
**Endpoint:** `https://jpservices.nationalrail.co.uk/stations`
- **Notes:** Full station list (~300KB+). Too large for ESP32 memory but useful for reference.

## Darwin SOAP Service (Live Data)

### Active (Working - March 2026)
**WSDL Host:** `lite.realtime.nationalrail.co.uk`
**WSDL Path:** `/OpenLDBWS/wsdl.aspx?ver=2021-11-01`
- **Usage:** Official Public Darwin OpenLDBWS API.
- **Notes:** Replaced a broken private/deprecated host.

### Legacy (Recent Failure - March 2026)
**WSDL Host:** `online.gadec.uk`
**WSDL Path:** `/darwin/wsdl.xml`
- **Status:** Failing with connection timeout / Refused.
- **Notes:** Likely a deprecated or private configuration host.

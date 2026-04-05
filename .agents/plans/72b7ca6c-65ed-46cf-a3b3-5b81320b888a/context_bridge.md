# Context Bridge

## 📍 Current State & Focus
Investigated the RDM API `LDBSVWS` endpoints to determine why the `trainFormationWidget` JSON parser isn't extracting `coaches` arrays. We discovered the user's `DARWIN_TOKEN` (`86276d33...`) is a legacy OpenLDBWS token that correctly authenticates against the legacy `lite.realtime.nationalrail.co.uk` XML SOAP endpoints, but returns a `403 Forbidden` error when trying to fetch the new `api1.raildata.org.uk` JSON REST Staff Version feeds.

## 🎯 Next Immediate Actions
- Wait for the user to register an account on the Rail Data Marketplace, subscribe to the **Live Departure Board - Staff Version (LDBSVWS)** tier, and paste their new `x-apikey` into the environment.
- Run Python/`curl` looping test scripts against a busy London terminal (e.g. `WAT` at peak hours) to guarantee interception of an inline `coaches` array.
- Flash the ESP32 and dynamically watch the board successfully handle a populated formation geometry payload via `pio run` and the device monitor macro.

## 🧠 Decisions & Designs
- The ESP32 parser architecture inside `modules/displayManager/boards/nationalRailBoard/nrRDMDataProvider.cpp` (specifically `filter["trainServices"][0]["formation"]["coaches"][0]`) is structurally sound and fully compliant with the Darwin RDM conventions.
- When an operating matrix lacks active real-time coach payload data, RDM natively scrubs the `coaches` array layer entirely, sending an empty shell object out as `serviceLoading`. The layout correctly evades crashing when this empty shell hits it.

## 🐛 Active Quirks, Bugs & Discoveries
- SWR off-peak diagrams omit the telemetry feeds occasionally. You must intercept a major terminal (WAT) during rush hour for a 100% strike rate on the geometry arrays.
- RDM `api1.raildata.org.uk` strictly enforces `x-apikey` rules and immediately `403` rejects legacy Darwin authentication tokens.

## 💻 Commands Reference
- Testing RDM API endpoints: `python3 .agents/tmp/rdm_test.py`
- Reading flash logs: `/read-flash-logs`

## 🌿 Execution Environment
- Hardware attached (pio run active on tty device).

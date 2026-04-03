# Context Bridge: Train Formations Widget

## 📍 Current State & Focus
We are establishing a new, separate architectural plan to focus exclusively on creating the **Train Formations Widget**. After evaluating the memory impact of upgrading the legacy XML SOAP parser to pull formation data, we confirmed that utilizing a JSON-based Rail Data Marketplace (RDM) API is the safest route for the ESP32 (tracked under plan `ded0e136-d474-473e-a80b-fd3352eaac2d`). 

To prevent UI development from blocking on backend API migration, this new plan isolates the widget creation. We will mock the required formation data (`length`, `coachClass`, `toilet`, `loading`) internally in `NationalRailStation` to fully build out the front-end C++ graphic layout and layout simulator.

## 🎯 Next Immediate Actions
1. **Mock Data Definitions**: Expand the `NationalRailService` struct in `modules/displayManager/boards/nationalRailBoard/nationalRailDataSource.hpp` to safely hold mock coach array data without overflowing memory bounds.
2. **Layout Simulator Mock**: Update `tools/layoutsim/mock_data/nationalRailBoard.json` to inject mock formation objects.
3. **Widget Construction**: Design and write a `trainFormationWidget` class (or update existing ones) in `modules/displayManager/widgets/` to handle graphically drawing coaches.

## 🧠 Decisions & Designs
- **Separation of Concerns**: We have formally split the RDM API ingestion (Backend) from the Widget rendering (Frontend).
- **Embedded Restraints**: Confirmed that extending the rigid `xmlStreamingParser` for deeply nested SOAP coach lists is extremely dangerous due to string allocations leading to heap fragmentation. The UI will proceed relying on simple arrays that the eventual RDM JSON pipeline will trivially populate.

## 🐛 Active Quirks, Bugs & Discoveries
- OpenLDBWS *does* contain `<length>` and `<formation>` data via XML SOAP, but dynamically extracting it with our current flat-state string concatenation logic would fracture the device runtime. It remains deliberately ignored by the `xmlStreamingParser`.

## 💻 Commands Reference
- Run layout simulation: `python3 tools/layoutsim/scripts/dev_server.py`
- Generate layouts: `python3 tools/layoutsim/scripts/gen_layout_cpp.py`

## 🌿 Execution Environment
- Target: Local ESP32 Layout Simulator / Web UI
- Portability Rule: All file references must remain strictly repository-relative.

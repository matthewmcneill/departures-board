# Transitioning to Rail Data Marketplace (RDM) API

- [ ] Plan implementation architecture
  - [ ] Review `iDataSource` interface and how boards instantiate data sources
  - [ ] Review configuration manager for how 'RailData' key is passed
  - [ ] Write implementation plan with new `rdmDataSource` class
- [ ] Implement `rdmDataSource` class
  - [ ] Create header `rdmDataSource.hpp` implementing `iDataSource`
  - [ ] Create implementation `rdmDataSource.cpp` using HTTP GET, `x-apikey`, and JSON parsing
- [ ] Update display boards to support RDM
  - [ ] Modify `nationalRailBoard` to dynamically instantiate `rdmDataSource` vs `nationalRailDataSource` on the heap inside `configure()` based on the selected API key's configured type.
- [ ] Update Configuration or UI if necessary to select the RDM provider
- [ ] Verification
  - [ ] Ensure it compiles
  - [ ] Validate implementation matches the V1.1 JSON format expectations

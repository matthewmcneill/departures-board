# Context Bridge
    
## 📍 Current State & Focus
The current focus is on fixing the `scrollingMessagePoolWidget` in `nationalRailBoard.cpp` so that it alternates displaying calling points and secondary informational messages (station messages, global RSS messages). The C++ Native firmare logic was audited, confirming that calling points display exactly once whenever LDBWS updates the data hash, and then the widget continuously cycles the informational message pool indefinitely, resulting in a discrepancy between the simulated UI and actual physical hardware behaviors compared to real-world UK Rail boards. An implementation plan to fix the pooling has been prepared and queued.

## 🎯 Next Immediate Actions
Execute the implementation plan by modifying `scrollingMessagePoolWidget` (`.cpp` and `.hpp`) to add an explicitly configurable interleaved message state, and then update `nationalRailBoard.cpp` to map the calling points properly.

## 🧠 Decisions & Designs
We decided to configure `scrollingMessagePoolWidget` gracefully by adding an optional `setInterleavedMessage` state buffer. This allows National Rail to interleave the calling points between dynamic message pool entries, while preserving compatibility for Tube/Bus layouts that do not utilize interleaving.

## 🐛 Active Quirks, Bugs & Discoveries
- Found that `data->firstServiceCalling` is explicitly passed via `.setText()` directly after an LDBWS/RDM data content hash change block. This natively wipes the marquee cache, and effectively makes calling points render just once per request interval.

## 💻 Commands Reference
- Automated web tests: `npm test` inside `test/web`
- DisplaySim Server: `python3 tools/layoutsim/scripts/dev_server.py`
- Compile hardware check: `make` (for native compilation)

## 🌿 Execution Environment
- WASM Simulator is running and actively tested. Note: `tools/layoutsim/scripts/dev_server.py` is actively bound and serving the workspace.
- The `implementation_plan.md` has been successfully drafted and saved awaiting structural execution natively.

# Context Bridge

**📍 Current State & Focus**
We have successfully analyzed the `DeviceSideMCPServerSpecification.md` and synthesized an implementation plan. The goal is to build an MCP server via HTTP for the ESP32 that exposes local telemetry, configuration mutants, and display buffers to external AI agents. We finalized the `implementation_plan.md` using strict OOP and memory constraints, mapping out an MVP rollout broken into 3 phases. We are pausing operations prior to beginning the build phase.

**🎯 Next Immediate Actions**
- Verify the active branch.
- Begin execution of Phase 1 of `implementation_plan.md` by constructing the `modules/mcpServer/mcpServer.hpp` header files mapping the abstracted execution engine.

**🧠 Decisions & Designs**
- **Dependency Inversion**: `mcpServer` will NOT inherently import `ESPAsyncWebServer.h`. Following OO rules, `webHandlerManager` maintains HTTP routing logic entirely on its own, parsing incoming JSON-RPC calls and passing the lightweight input alongside an abstract `Print&` object down to `mcpServer::processPayload()`. This isolates the network layer while giving `mcpServer` the required stream writer to avoid RAM exhaustion.
- **Phased Approach**: Phase 1 is strictly Telemetry and Observability ("getters"). Phase 2 involves safe configuration mutation. Phase 3 focuses on edge-case emulation logic (like mock data injection).
- **Reduced Scope**: We explicitly chose not to implement `get_recent_logs` to save RAM overhead, delegating that to a local PCP server. Additionally, `set_calibration_display` will hook into pre-existing functionality instead of reinventing it.

**🐛 Active Quirks, Bugs & Discoveries**
- Due to tight Flash limits (~250KB remaining application budget), all MCP schemas MUST be read entirely from PROGMEM directly to the network streams. 
- Spooling arrays in RAM (like using a single large `DynamicJsonDocument` response dump) guarantees a crash within the `async_tcp` stack. Do not bypass the `Print&` stream execution.

**💻 Commands Reference**
- Use Python's `requests` library or `curl` to `POST /mcp` when testing. Since JSON payloads will be required, use `curl -X POST -H "Content-Type: application/json" -d '{"jsonrpc": "2.0", "method": "tools/list", "id": 1}' http://<ESP_IP>/mcp`

**🌿 Execution Environment**
- PlatformIO ESP32 environment. `main` branch. Local repository configuration applied.

**⚠️ PORTABILITY RULE**
- Ensure all plan updates and code modifications strictly use repository-relative paths (e.g., `modules/mcpServer/...`). No absolute paths are permitted.

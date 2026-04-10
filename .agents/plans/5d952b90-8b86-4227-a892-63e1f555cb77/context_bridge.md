---
title: ESP32 Device-Side MCP Server
distilled_at: 2026-04-10T09:25:00Z
original_plan_id: 5d952b90-8b86-4227-a892-63e1f555cb77
artifacts:
  - context_artifacts/adr_mcp_architecture.md
  - context_artifacts/graveyard_mcp_memory.md
---
# Context Bridge

## Executive Summary
We have finalized the `implementation_plan.md` for constructing an ESP32 onboard MCP server over HTTP. The scope was adjusted to build the Agent Client Skill (`device-mcp-client`) immediately in Phase 2 to unlock interactive log discovery and screen buffer PNG rendering.

## Next Steps
- Execute `/plan-start` to invoke hardware locks and start work.
- Construct `modules/mcpServer/mcpServer.hpp` to define the abstract `Print&` executor logic.
- Construct `modules/mcpServer/mcpServer.cpp` and inject basic `tools/list` payloads from PROGMEM.

> [!WARNING]
> Do NOT read the detailed artifacts below unless your current task explicitly requires the deep context. If needed, use your read tool on `context_artifacts/[filename]`.

- `context_artifacts/adr_mcp_architecture.md` - Technical reasoning behind Dependency Inversion and OOP paradigms for `mcpServer`.
- `context_artifacts/graveyard_mcp_memory.md` - Documentation of WDT timeouts caused by monolithic `DynamicJsonDocument` allocation over `AsyncResponseStream`.

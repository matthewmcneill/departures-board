---
name: "Layout Simulator Consolidation"
description: "Consolidated the WebAssembly-based Layout Simulator by relocating the `dataHarvester.py` from the root `scripts/` directory to the tool's internal source tree. Implemented a new POSIX-compliant [layou..."
created: "2026-04-05"
status: "DONE"
commits: ['9c7f575']
---

# Summary
Consolidated the WebAssembly-based Layout Simulator by relocating the `dataHarvester.py` from the root `scripts/` directory to the tool's internal source tree. Implemented a new POSIX-compliant [layoutsim.sh](tools/layoutsim/layoutsim.sh) entry point that handles absolute-path safety and includes an interactive check for mock data. Fixed several `NoneType` iteration bugs in the harvester script to improve data ingestion reliability from the Huxley-2 API proxy.

## Key Decisions
- **Tool Encapsulation**: Relocated `dataHarvester.py` to `tools/layoutsim/scripts/` to create a self-sufficient tool bundle, reducing repository root clutter.
- **Interactive Harvesting**: Added a bash-driven detection loop to `layoutsim.sh` that prompts users to fetch real-time data if `mock_data/` is empty, lowering the barrier to entry for new developers.
- **House Style Compliance**: Enforced standard project license headers and `camelCase` consistency across the simulator's shell and python utilities.

## Technical Context
- [sessions.md](sessions.md)
- [task.md](task.md)

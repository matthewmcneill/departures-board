# Memory Impact Assessment Rule

Every `implementation_plan.md` created for this project MUST include a "Memory Impact Assessment" section.

## Context
This is an embedded software project targeting the ESP32 platform. Memory efficiency, heap stability, and footprint minimization are critical for long-term reliability.

## Requirements
Whenever you are drafting an implementation plan for C++ source or header files:
1.  **Use the Skill**: You MUST consult and follow the `embedded-memory-assessment` skill located in `.agent/skills/embedded-memory-assessment/SKILL.md`.
2.  **Section Format**: Include a top-level `## Memory Impact Assessment` section in your plan.
3.  **Mandatory Checks**:
    - **Flash/ROM**: Estimated increase in binary size.
    - **Static RAM**: Changes in global/static variables.
    - **Stack**: Risk of stack overflow from large local buffers or recursion.
    - **Heap**: Dynamic allocation patterns and fragmentation risk.
    - **PSRAM**: Potential for offloading to external RAM.

## Enforcement
Do not proceed to the implementation stage until the memory impact has been explicitly evaluated and documented in the plan.

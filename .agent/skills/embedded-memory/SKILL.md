---
name: embedded-memory-assessment
description: Perform a rigorous memory impact assessment for embedded C++ projects (specifically ESP32). Use this skill whenever an implementation plan is being drafted to ensure memory efficiency, heap stability, and footprint minimization are addressed.
---

# Embedded Memory Impact Assessment

This skill provides a structured methodology for assessing the memory impact of code changes in embedded environments, with a focus on the ESP32 platform used in this project.

## Assessment Criteria

Whenever you are tasked with producing an implementation plan, you MUST include a "Memory Impact Assessment" section covering the following areas:

### 1. Flash / Program Memory (ROM)
- **Estimate Increase**: How many bytes/KB will the binary size increase?
- **Optimization**: Are large data structures (lookup tables, fonts, bitmaps) marked as `const` to stay in Flash?
- **Impact**: Will this push the binary closer to partition limits?

### 2. Static RAM (DRAM / IRAM)
- **Global/Static Variables**: Have any new global or static variables been introduced?
- **BSS/Data Sections**: What is the impact on the constant memory footprint?

### 3. Stack Usage
- **Local Buffers**: Are there any large local arrays (e.g., `char buf[1024]`) that could risk a stack overflow?
- **Recursion**: Does the change introduce deep or unbounded recursion?
- **Task Stacks**: If creating a new FreeRTOS task, estimate the required stack size based on local variable usage.

### 4. Heap Usage & Fragmentation
- **Allocation Pattern**: Does the change use `new`, `malloc`, or dynamic containers (`std::vector`, `std::string`)?
- **Persistence**: Are allocations short-lived (fragmentation risk) or long-lived (static-like)?
- **Fragmentation Risk**: Frequent small allocations/deallocations of varying sizes lead to fragmentation. Can these be consolidated into a single larger allocation or made static?
- **PSRAM**: If the change involves large buffers (e.g., framebuffers, network buffers), can they be explicitly allocated in PSRAM using `MALLOC_CAP_SPIRAM`?

### 5. Long-term Stability
- **Leaks**: Is every allocation matched with a corresponding deletion in all possible execution paths (including exceptions/error returns)?
- **Low Watermark**: How will this change affect the minimum free heap observed during stress tests?

## Recommended Methodology

1. **Analyze Code Path**: Map out the allocation and deallocation lifecycle of the new feature.
2. **Quantify**: Provide numeric estimates where possible (e.g., "Expected heap usage: ~2KB peak").
3. **Identify Risks**: Explicitly call out "High fragmentation risk" if using dynamic strings in a loop.
4. **Propose Mitigations**: Suggest using `reserve()` for vectors or switching to fixed-size buffers if appropriate.

## Example Assessment Section

```markdown
## Memory Impact Assessment

### Flash/ROM
- Estimated increase: ~450 bytes (new logic + constants).
- Action: Bitmaps moved to `const` to ensure they reside in Flash.

### Static RAM
- No significant change; no new global variables.

### Stack
- Added one local buffer of 256 bytes for JSON parsing; well within the 4KB task stack limit.

### Heap & Fragmentation
- **Allocation**: One dynamic allocation of 1KB for the network response buffer.
- **Stability**: Buffer is allocated once at start and reused, minimizing fragmentation.
- **PSRAM**: Not used as the buffer is small enough for internal RAM.
```

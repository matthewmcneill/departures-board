# ESP32 Memory Architecture: Structural Review

This document provides a structural breakdown of how memory is managed in the Departures Board project and the trade-offs involved in different allocation strategies.

## 1. The Three Memory Regions

On the ESP32, we balance three distinct pools of RAM:

| Region | Capacity | Use Case | Lifecycle | Risk |
| :--- | :--- | :--- | :--- | :--- |
| **Task Stack** | **~8 KB** (Default) | Local variables, function calls, strings. | Automatic (ends with function). | **CRASH**: Overflows corrupt the OS kernel. |
| **Static DRAM** | **~320 KB** (Shared) | Globals, Static variables, Class members. | Permanent (starts at boot). | **DRAM FULL**: Binary won't link/start. |
| **System Heap** | **~300 KB** (Shared) | `new`, `malloc`, Arduino `String` buffers. | Manual (`delete`) or until string grows. | **FRAGMENTATION**: Memory becomes a "swiss cheese" of small gaps. |

---

## 2. Structural Audit: `appContext`

### Current State: Static Allocation
Currently, `appContext` is declared as a **global variable**. This means it and all its children live in **Static DRAM**.

*   **Hierarchy**: `Global context` -> `DisplayManager` -> `slots[3]` -> `NationalRailBoard` -> `NationalRailDataSource` -> `NationalRailStation`.
*   **Cost**: The `NationalRailStation` struct takes ~10.5KB. Because it's a member variable, it consumes that 10.5KB of DRAM from the moment the device boots, even if you are only looking at the Tube board.
*   **Pros**: Zero heap fragmentation. We know exactly how much RAM we have left at compile-time.
*   **Cons**: Rigid. We can't "unload" a board to save RAM for something else.

### Why the Stack Crashed anyway?
Even though the data structures are in DRAM, the **execution code** happens on the stack.
1.  `appContext::begin()` is called. (Stack depth 1)
2.  `displayManager::applyConfig()` is called. (Stack depth 2)
3.  `NationalRailBoard::onActivate()` is called. (Stack depth 3)
4.  `NationalRailDataSource::init()` is called. (Stack depth 4)
5.  `WiFiClientSecure` creates internal SSL buffers (on the stack). (Stack depth 5+)
6.  **BOOM**: The pointer hits the 8KB ceiling.

---

## 3. Offloading Strategy: What belongs where?

### Keep in Static DRAM (Current)
*   **Core Managers**: DisplayManager, ConfigManager. They are small and singular.
*   **Board Slots**: The `slots[]` array. Static allocation here prevents the "OOM (Out Of Memory) at 2 AM" problem caused by heap fragmentation.

### Move to Heap (Recommended Candidates)
*   **Network Clients**: (Done for NR). Objects like `WiFiClientSecure` or `HTTPClient` should always be heap-allocated inside the function that needs them and deleted immediately after.
*   **JSON Documents**: Use `JsonDocument` as a local heap-based object rather than a member variable.
*   **Transient Buffers**: Any `char buffer[1024]` used for temporary text manipulation should be offloaded to the heap if the call stack is deep.

## 4. The Trade-off Summary

*   **Stack vs Heap**: Use Stack for anything under ~200 bytes. Use Heap for anything over ~512 bytes if you are more than a few levels deep in function calls.
*   **Static vs Heap**: Use Static for objects that *must* exist forever. Use Heap for objects that only exist during a specific action (like "Update Data").

**Conclusion**: We should not move the boards themselves to the heap yet, as that would introduce instability over weeks of runtime (fragmenting the RAM). Instead, we should continue to "lighten the stack" by ensuring transient network and parsing objects are heap-allocated.

## 5. Why wasn't this done from the start?

The project likely leveraged the **Stack-First** approach because it is the "safe default" in C++:
*   **Automatic Cleanup**: You can't forget to delete a stack variable.
*   **Determinism**: Allocation is instantaneous and won't fail (until the stack is full).
*   **Simplicity**: Code is easier to read without pointers and `new/delete`.

The need for Heap allocation only emerged as the system became more "Modern":
1.  **SSL/TLS**: Secure connections (HTTPS) require much larger internal buffers than standard HTTP.
2.  **Increased Abstraction**: Moving from flat C functions to deep Object-Oriented hierarchies (like `appContext`) naturally deepens the call stack.

## 6. Downsides and Risks of the Heap Approach

While necessary to solve our crash, the Heap approach has its own engineering challenges:

### 1. Risk of Memory Leaks
Unlike the stack, if we call `new` and forget to call `delete` (perhaps due to an early `return` in an error case), that RAM is lost until the device restarts. Over days or weeks, the device would eventually "run out of air" and crash.
*   **Mitigation**: Always wrap heap objects in "Smart Pointers" (like `std::unique_ptr`) or be extremely disciplined with `delete`.

### 2. Heap Fragmentation
If we frequently allocate and deallocate objects of varying sizes, the heap can become "fragmented" like a hard drive. We might have 100KB free in total, but if it's in 1KB chunks, a single request for 2KB will fail.
*   **Mitigation**: Keep large, permanent objects (like Boards) in **Static DRAM** and only use the heap for short-lived, transient tools (like Clients).

### 3. Allocation Failure
`new` can return `NULL` if the heap is full. The stack doesn't have a "return value" check—it just crashes.
*   **Mitigation**: Always check `if (!ptr)` after a heap allocation.

## 7. Implementation & Verification Results (March 2026)

Following the critical boot loop incidents, a project-wide memory refactor was implemented to offload transient "peaked" objects to the heap.

### Verification Success
- **Boot Loop Resolved**: The system successfully passes the `WiFiClientSecure` handshake phase, which was previously the primary crash point.
- **Improved Stack Headroom**: By offloading clients and parsers, we have freed approximately **10-15 KB of transient stack space** across the main execution loop.
- **Stable Static Footprint**: Static DRAM remains stable at **~30% utilization**, ensuring core managers have permanent, non-fragmented space.

### Modules Optimized
The following modules were converted to use `std::unique_ptr` for their heavy dependencies:
1.  **TfL**: Heap-allocated client, parser, and station buffers.
2.  **Bus**: Heap-allocated scraping client and comparison structs.
3.  **National Rail**: Heap-allocated secure SOAP client.
4.  **Weather**: Heap-allocated JSON parser and client.
5.  **RSS/OTA**: Heap-allocated download and update clients.

**Final Verdict**: The system is significantly more robust against "Deep Call" crashes. The use of Smart Pointers provides the flexibility of the heap with the deterministic cleanup of the stack.

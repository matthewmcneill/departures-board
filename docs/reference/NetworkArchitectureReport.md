# Network Data Architecture Redesign Report (Updated)

## 1. Evaluation of the Current Approach
The existing `dataManager` implementation uses a FreeRTOS `QueueHandle_t` pinned to Core 0. Clients push an `iDataSource*` to the back of the queue (FIFO), and a single worker task blocks on `xQueueReceive`, executing requests sequentially. 

**Strengths:**
- **Heap Protection**: Strictly serializes TLS allocations, preventing Out-Of-Memory (OOM) errors and Watchdog Timer (WDT) panics.
- **Core Separation**: Keeps heavy networking on Core 0, leaving Core 1 free for UI and display updates.

**Weaknesses & Gaps:**
- **No Periodic Polling**: It is fully reactive. Data sources must manually enqueue themselves, violating Principle 2 (continuous freshness).
- **No Priority Control**: FIFO queueing means a newly activated display (needing immediate data) must wait for all pending background updates to finish (violates Principle 3).
- **No Temporal Scheduling**: Impossible to delay a fetch or request one at a custom future time (violates Principle 4).
- **Incomplete Coverage**: Historically, not all fetchers (e.g. some web portal API key tests, Weather, RSS) may have been uniformly routed through this queue, exposing the system to potential concurrent TLS panics.

---

## 2. Proposed Architecture: The Schedule-Driven `dataManager`

To satisfy all principles and embedded-systems constraints, the simple queue should be replaced by a **Priority-Aware Scheduling Loop**.

### Core Concepts:
1. **Universal Registry (Weather, RSS, Rail)**: All network operations (via `iDataSource` or a generic `iFetchable` interface) MUST register with the `dataManager` on boot. Weather clients and RSS feed scrapers are treated exactly like transit boards, ensuring 100% of TLS allocations are serialized globally on Core 0.
2. **Next Fetch Timestamps & Baseline Minimums**: Each source maintains a `uint32_t nextFetchTime` (in `millis()`). A built-in configurable baseline minimum interval (e.g., 30 seconds) is fundamentally enforced to prevent runaway hammering of external APIs.
3. **Web Portal API Tests**: Ad-hoc tests triggered by the web portal form a special case. They skip normal registry scheduling but submit a one-off high-priority event to the Core 0 worker to execute immediately and return a result sync/async pattern back to the Core 1 web server securely.
4. **Smart Blocking**: The Core 0 worker task computes the time until the soonest `nextFetchTime` across all sources. It then blocks on a FreeRTOS queue with a timeout equal to that duration.

### Addressing the Principles:
**0) Asynchronous Core 0 Scheduling:** Maintained. The worker task remains pinned to Core 0.
**1) Sequential Fetching:** Maintained globally. The worker processes one source at a time.
**2) Regular Background Updates:** The worker automatically iterates through the registry. If `millis() >= source->nextFetchTime`, it executes the fetch and calculates the subsequent `nextFetchTime` based on a default periodic interval.

**3) Dynamic Priority Tiering (The Empty/Active Rule):** 
Instead of a simple FIFO, the scheduler evaluates priority dynamically. When waking up, it sorts/selects the next source based on these Tiers:
   - **Tier 0 (Critical)**: Web Portal Ad-hoc API Test Request. Needs immediate execution to prevent browser timeouts.
   - **Tier 1 (High)**: Data Frame is EMPTY (never loaded) **AND** Display is ACTIVE. This guarantees the screen populates instantly on boot or switch.
   - **Tier 2 (Medium)**: Normal scheduled update for an ACTIVE Display.
   - **Tier 3 (Low)**: Normal scheduled update for a BACKGROUND Display (can be throttled extensively, e.g. every 5 mins).

**4) Dynamic Schedule Parsing (Custom Refresh):** Data sources possess the intelligence to parse their own schedule data during a fetch. By reading the actual arrival/departure times, the source determines when the data will realistically change next. For example, the National Rail source spots that the soonest train departs in exactly 3 minutes. It sets its own next refresh to "3 mins + 10 seconds." This intelligent internal parsing guarantees we fetch precisely when the information changes, replacing blind periodic polling (subject always to the baseline minimum interval).

---

## 3. Embedded-Systems Skill Efficiency Evaluation

*As evaluated against `.agents/skills/embedded-systems/SKILL.md` constraints:*

### RAM & Flash Footprint
- **RAM**: Highly optimized. Uses a single array/vector of pointers (`std::vector<iDataSource*>`) and a single FreeRTOS event queue for priority interrupts.
- **Stack**: Maintained at exactly one 8192-byte stack for the single worker task.
- **Heap**: Peak heap usage is bounded by the size of the single largest TLS JSON parsed at any one time, perfectly managing fragmentation.

### Power & CPU Optimizations
- **Sleep Modes**: By utilizing `xQueueReceive(eventQueue, &event, ticksToWait)`, Core 0 can smoothly enter low-power sleep (IDLE task) while waiting for the next scheduled fetch. This avoids busy-waiting loops.
- **Network Duty Cycling**: API calls are expensive in terms of battery and RF transmission. The "Custom Scheduled Refresh" combined with Tier 3 throttling acts as intelligent duty cycling—only turning on the WiFi radio when the data is known to have changed or when the user is actively watching.

### Recommendations & Refinements to Principles
- **Head-of-Line Blocking**: Even with priority queues, an active long-running fetch will block a priority request. *Recommendation:* Enforce strict hardware TCP timeouts (e.g., 5000ms) on all `WiFiClientSecure` connections to ensure the worker never deadlocks.
- **Pre-Emptive Aborts**: If a Web Portal Test (Tier 0) arrives while a Background Display (Tier 3) fetch is slowly executing, we cannot strictly interrupt it mid-stream without memory leaks. The system must wait for the current fetch to finish, emphasizing the need for short TCP timeouts.

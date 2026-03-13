# Implementation Plan: GraphWidget for Arrival History

## Goal
Add a `GraphWidget` to visualize recent arrival performance using a sparkline.

## Proposed Changes
- Create `GraphWidget` class.
- Store historical data points (e.g., last 60 minutes).
- Implement `render(U8G2& display, int x, int y, int w, int h)`.

## Memory Impact Assessment

### Flash / Program Memory (ROM)
- **Estimate Increase**: ~1-2 KB for the drawing logic (Bresenham's line algorithm or similar).
- **Optimization**: No large assets added.

### Static RAM
- No significant change.

### Stack Usage
- **Local Variables**: Minimal; coordinate mapping logic uses standard integer arithmetic.
- **Recursion**: None.

### Heap Usage & Fragmentation
- **Historical Data**: Storing 60 data points (e.g., `uint8_t` for delay in minutes).
- **Allocation**: ~60 bytes if using a fixed-size array.
- **Fragmentation Risk**: Low, provided we use a fixed-size buffer (circular buffer) instead of a dynamically resizing `std::vector`.
- **Mitigation**: Define the buffer size at compile-time or allocate once during widget initialization.

### Long-term Stability
- **Leaks**: Fixed-size buffer managed by the widget's lifecycle.
- **Heap Health**: No repeated allocations/deallocations during the render loop.
- **PSRAM**: Not necessary as the data structure is very small (~60-120 bytes).

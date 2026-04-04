# Replicating National Rail Message Alternation

The current code behavior actually differs from what you would expect from real National Rail displays. 

## Code Audit Findings
I've audited the `nationalRailBoard.cpp` and its `scrollingMessagePoolWidget` component. Currently, the implementation does **not** alternate the calling points with the informational messages:
1. When data arrives, `nationalRailBoard.cpp` forces the calling points to display ONCE using a hard override (`setText()`) on the marquee widget.
2. Once the calling points finish scrolling, the `scrollingMessagePoolWidget` automatically takes over and iterates through the `MessagePool`s (station messages, global RSS messages) continuously.
3. The calling points are currently **never** displayed again until the LDBWS/RDM data update indicates a changed content hash roughly 45 seconds later, which resets the whole cycle.

## User Review Required

Because both the physical ESP32 firmware and the WebAssembly DisplaySim share the exact same C++ UI classes (`scrollingMessagePoolWidget` is cross-compiled for both), we must update the core C++ widgets. **Any changes we apply to the main C++ codebase will automatically fix the behavior in your simulator.**

## Proposed Changes

### displayManager

To implement the "calling points -> message -> calling points -> message" cycle, I propose enhancing the `scrollingMessagePoolWidget` to formally support an "Interleaved Message":

#### [MODIFY] modules/displayManager/widgets/scrollingMessagePoolWidget.hpp
- Add a new private buffer `char interleavedMessage[512]`.
- Add a boolean flag `bool showingInterleavedNext`.
- Add a public method `void setInterleavedMessage(const char* msg)`.

#### [MODIFY] modules/displayManager/widgets/scrollingMessagePoolWidget.cpp
- Update `loadNextMessage()`: 
  - If `showingInterleavedNext` is true and `interleavedMessage` is not empty, `setText()` to the `interleavedMessage` and toggle the flag to `false`.
  - Otherwise, pull the next message from the registered pools (station or global).
  - Crucially, if all pools are completely empty, gracefully fall back to continuously scrolling the `interleavedMessage`.

#### [MODIFY] modules/displayManager/boards/nationalRailBoard/nationalRailBoard.cpp
- Eliminate the initial manual `.setText()` call inside `updateData()`.
- Instead, invoke `activeLayout->msgWidget.setInterleavedMessage(msg.c_str())`.
- For the attribution string when the board initially activates, we can also inject this as the starting state of the scrolling widget.

## Open Questions

1. Is the proposed strategy of extending the base widget to support an alternating interleaved message acceptable? This safely retains compatibility with TfL/Bus boards since they will just leave the interleaved message blank.
2. When the attribution displays ONCE on boot, do you want it to behave like a standard pool message that gets cycled out and never comes back, or should we just let the interleaved model handle it?

## Verification Plan

### Automated Tests
- Run `make` for ESP32 unit tests to ensure `scrollingMessagePoolWidget` compiles cleanly.
- Build the `layoutsim` WASM client.

### Manual Verification
- Open the simulator frontend with your National Rail layout.
- We will visually verify that the calling points scroll once, followed by a station message (e.g. "Trains to London are delayed..."), followed immediately by the calling points again.

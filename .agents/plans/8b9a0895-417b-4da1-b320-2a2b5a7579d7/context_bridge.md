# 📍 Current State & Focus
We are enhancing the `scrollingMessagePoolWidget` to support interleaved priority messages. This is primarily to satisfy the requirement that "Calling Points" on the National Rail board should appear between every other message in the rotation (Station Alerts, RSS, Weather).

# 🎯 Next Immediate Actions
1.  **Modify `scrollingMessagePoolWidget.hpp`**: Add `priorityMessage` buffer and `setPriorityMessage()` method.
2.  **Modify `scrollingMessagePoolWidget.cpp`**: Implement the interleaving logic in `loadNextMessage()`.
3.  **Update `nationalRailBoard.cpp`**: Link the calling points data to the new priority slot instead of `setText()`.
4.  **Verification**: Flashing to hardware and observing the 1:1 rotation ratio.

# 🧠 Decisions & Designs
- **Interleaved Slot**: Instead of creating a high-priority "pool", we implemented a single "Priority Slot" in the widget. This ensures we can easily hit the 50% screen-time requirement without complex scheduling logic.
- **Generic Widget API**: The solution is built into the base `scrollingMessagePoolWidget`, making it available for Bus and TfL boards to use for their own critical alerts in the future.

# 🐛 Active Quirks, Bugs & Discoveries
- **The "Overwrite" Bug**: Discovered that the current implementation of Calling Points uses `setText()`, which is a "one-shot" load. Once the message finishes scrolling, the widget's internal pool rotation takes over and permanently clears the calling points until the next API refresh.

# 💻 Commands Reference
- `pio run -e esp32dev` (Build)
- `pio device upload` (Flash)
- `pio device monitor` (Log Verification)

# 🌿 Execution Environment
- **Hardware**: ESP32 Dev Module attached to `/dev/cu.usbserial-*`.
- **Git**: Branch `main`.

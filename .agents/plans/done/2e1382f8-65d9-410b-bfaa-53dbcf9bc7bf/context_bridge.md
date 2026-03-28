# Context Bridge: TfL & Bus Board Memory Safety & Layout Upgrade

## 📍 Current State & Focus
The memory corruption issue in the `TfLBoard` and `BusBoard` has been permanently resolved. The system has transitioned from a dangerous stack-allocated string pointer pattern to a stable, zero-copy architecture using static position number arrays in the `DataSource` classes. Simultaneously, both boards have been upgraded to a high-fidelity 4-column layout (Order, Line/Route, Destination, Time), and a critical build-blocking syntax error in `schedulerManager.cpp` has been fixed.

## 🎯 Next Immediate Actions
- **None**: This plan is 100% complete and verified.
- **Future Enhancement**: Consider applying this zero-copy numbering pattern to future transport modules (e.g. RSS or Weather) if they ever require indexed numbering.

## 🧠 Decisions & Designs
- **Zero-Copy Numbering**: Static `const char* serviceNumbers[]` arrays in `tflDataSource.cpp` and `busDataSource.cpp` provide persistent pointers ("1" through "20"). This is faster, memory-safe, and zero-overhead compared to a buffer pool.
- **4-Column Layout**: The `serviceListWidget` now renders 4 columns. Widths were adjusted (20, 20, 160, 50) to accommodate the new "Order" column while maintaining destination readability.
- **Conditional Ordinals**: The "Order" column only shows numbers if `config.showServiceOrdinals` is enabled, otherwise it passes an empty string `""` to the widget.

## 🐛 Active Quirks, Bugs & Discoveries
- **Layout Generation**: confirmed that `scripts/generate_layouts.py` correctly handles the new 4-column JSON schemas and updates the C++ implementation files automatically.

## 💻 Commands Reference
- **Build**: `pio run`
- **Test**: `pio test -e unit_testing_host`
- **Flash**: `pio run -t upload`

## 🌿 Execution Environment
- **Branch**: `feature/multi-board-architecture` (Revision `10e5e55`)
- **Hardware**: Physical `esp32dev` verified via serial monitor.

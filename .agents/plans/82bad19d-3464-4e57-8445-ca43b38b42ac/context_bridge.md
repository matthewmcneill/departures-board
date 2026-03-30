# Context Bridge: API Mapping & Train Formation Widget

**📍 Current State & Focus**
We have just concluded a deep architectural research phase assessing the limitations and capabilities of the underlying APIs (TfL, Bus SIRI, National Rail Darwin). The focus culminated in a formal `implementation_plan.md` to build an animated, 3-state `TrainFormationWidget` that parses Darwin coach formations (loading %, accessibility, 1st class) without crashing the low-resolution visual footprint or the ESP32 RAM.

**🎯 Next Immediate Actions**
1. Initiate the execution of the `implementation_plan.md` found in this folder.
2. Extend `NationalRailService` struct in `nationalRailDataSource.hpp`.
3. Build the `TrainFormationWidget` matching the responsive sizing and marquee scrolling constraints defined in the plan.
4. Update the JSON layout parser map so designers can bind to it.

**🧠 Decisions & Designs**
- **The Granular UI Toolkit Engine**: We abandoned a fixed "Semantic Table" approach to give designers maximum layout variance through JSON. Text strings (like `Destination`, `Via`, `Platform`) will be dynamically bindable floating elements.
- **Message Pool Dumping**: Extra rich context like `delayReason` or `currentLocation` goes into the rolling `MessagePool` to save UI space.
- **3-State Carriage Animation**: `TrainFormationWidget` rotates through `MODE_NUMBER` -> `MODE_FACILITIES` -> `MODE_LOADING` pausing 3 seconds each. If a train exceeds `MIN_COACH_WIDTH`, it initiates a Marquee horizontal pan. The animation state ONLY rotates after a complete pan pass across the matrix.

**🐛 Active Quirks, Bugs & Discoveries**
- **Platform Zones Abandoned**: The public NR Darwin feed does *not* transmit physical specific platform zones (e.g. 1-10 string mappings for coaches). Discarded this feature to prevent misalignment.
- **Data Scarcity**: Many Darwin public feeds strip `<facilities>` and `<loading>` tags entirely. The widget must gracefully skip empty states (`tick()` skips directly to `MODE_NUMBER` if no loading data is parsed).

**💻 Commands Reference**
- Unit compile check: `pio test -e unit_testing_host`

**🌿 Execution Environment**
- Host evaluation mode. Code edits are focused on modules and unit testing before dropping onto ESP32 hardware.

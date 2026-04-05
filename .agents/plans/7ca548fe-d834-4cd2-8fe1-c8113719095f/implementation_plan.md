# Goal
Design and implement a dynamic `TrainFormationWidget` that visually represents the carriage structure of a National Rail service. To maximize readability on low-resolution displays, the widget will rotate through a 3-state animation loop: showing Coach Numbers, showing Facilities, and showing Loading bars.

## 1. Architectural Alignment
[x] Evaluated against House Style: Standard `camelCase` and interface boundaries apply. All standard sections are present.
[x] Evaluated against Embedded Patterns: Instead of overlaying all three data points in a tiny carriage box (which creates illegible UI), the state-machine animation pattern ensures crisp, readable graphics without allocating 3 separate widgets. Data structures decouple UI from API.
[x] Evaluated against Architectural Standards: Ensures SRP by decoupling RDM API ingestion from widget rendering. DI is used by injecting `NationalRailService` pointers into `trainFormationWidget`.
[x] Evaluated against Embedded Resource Impact: Added full Memory and Power resource impact assessment below.

## User Review Required
> [!IMPORTANT]
> **API Availability Check**
> While we will write the parsing logic for `loading` and `facilities`, please note that OpenLDBWS Public feeds historically strip some of this data unless you are using the Staff Token or the Push Port. If the data is empty, the widget will handle this gracefully (empty boxes), but be aware of the upstream limits.

## Proposed Changes

---

### Data Manager Layer
Data structures for `NrCoachFormation` already exist in `iNationalRailDataProvider.hpp`. We will use these structures and apply mock data in the Layout Simulator directly to bypass backend dependencies.

#### [MODIFY] [tools/layoutsim/mock_data/nationalRailBoard.json](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/mock_data/nationalRailBoard.json)
- Inject mock `NrCoachFormation` arrays (coach class, toilet/wheelchair, loading, numbers) to simulate a 12-coach formation.

---

### Widget Layer
Create the new graphical UI component for the display designer with dynamic sizing. We will incorporate the glyphs from `FormationIcons7.txt`.

**Widget ASCII Art Representation:**
```text
[MODE_NUMBER - Static Mode]
   ___     ___     ___     ___ 
 /  1  | |  2  | |  3  | |  4  |
/______| |_____| |_____| |_____|

[MODE_FACILITIES - Static Mode]
   ___     ___     ___     ___ 
 / [W] | | [1] | | [B] | |     |
/______| |_____| |_____| |_____|
```


#### [NEW] [trainFormationWidget.hpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/widgets/trainFormationWidget.hpp)
Define the `trainFormationWidget` class extending `iGfxWidget`.
- **Inputs**: Pointer to the active `NationalRailService`.
- **State Machine**: An `enum` tracking the animation phase (`MODE_NUMBER`, `MODE_FACILITIES`, `MODE_LOADING`).
- **Timing & Scrolling**: Variables to track `lastAnimationMs`, `intervalMs` (default 3000ms), and a `scrollOffset` for when the train exceeds the widget bounds.

#### [NEW] [trainFormationWidget.cpp](file:///Users/mcneillm/Documents/Projects/departures-board/modules/displayManager/widgets/trainFormationWidget.cpp)
Implement the drawing logic using `U8g2` methods with responsive constraints.

**Responsive Sizing Logic**:
1. Calculate `availableWidth = widgetWidth - noseConeWidth`.
2. Calculate `carriageWidth = availableWidth / numCoaches`.
3. If `carriageWidth > MAX_COACH_WIDTH` (e.g., 20px), clamp it to 20px and optionally center the train in the widget bounds so a 2-coach train doesn't look bizarrely stretched.
4. If `carriageWidth < MIN_COACH_WIDTH` (which is 15px to accommodate the 12px bicycle glyph, 1px padding on each side, and overlapping frames):
    - Clamp `carriageWidth = MIN_COACH_WIDTH`.
    - Enable **Marquee Mode**. The total drawn width now strictly exceeds `widgetWidth`.

**`tick()` and Rendering Logic**:
- **Static Mode (Train Fits)**: Advances the `MODE` every 3 seconds. Skips states automatically if the required data (like `loading`) is entirely missing.
- **Marquee Mode (Train is too long)**: The train slowly pans horizontally across the clipping boundary. The sequence is:
    1. Pan the entire length of the train displaying `MODE_NUMBER`.
    2. Once the final carriage exits the left boundary (or reaches the end), reset the `scrollOffset`.
    3. Increment to `MODE_FACILITIES` and begin panning the entire train length again.
    4. Repeat for `MODE_LOADING`.
    This ensures the graphics do not flash chaotically while the train is in motion, keeping the interface clean and legible.

**`render()`**:
- Set a clipping window `u8g2.setClipWindow()` to ensure the moving train doesn't draw outside its assigned widget bounds.
- Draw the nose cone offset by `scrollOffset`.
- Loop through `numCoaches`, drawing the bounding rects.
- **If MODE_NUMBER**: Print `coach.number` centered in the clipped box at `(x_pos + scrollOffset)`.
- **If MODE_FACILITIES**: Draw the wheelchair/1st class bitmap centered.
- **If MODE_LOADING**: `drawBox` from the bottom up based on `(boxHeight * coach.loadingPercent) / 100`.

## Open Questions
None. Standardizing the Marquee mode to only rotate on full-pass completion resolves the visual sync issues.

## Resource Impact Assessment

> [!WARNING]
> **Memory (RAM/Heap Constraint)**
> To save memory, instead of attaching formation data to every `NationalRailService` (which would consume ~5.6KB), we will bind `firstServiceFormation[NR_MAX_COACHES]` strictly to the `NationalRailStation` level, mirroring how `firstServiceCalling` points are handled. This drops the heap overhead of formations down to a nominal ~200 bytes.

> [!TIP]
> **Power and CPU Cycles**
> The Marquee animation requires recalculating bounding boxes during the `MODE` scroll phase. By using a 3000ms static mode interval and limiting the Marquee scroll repaint to low delta values, the CPU overhead remains minimal.

## Verification Plan
### Automated Tests
- Run `pio test -e unit_testing_host` to test compiling.

### Manual Verification
- Deploy to hardware or browser visualizer.
- Trigger a mock XML payload with 12 coaches to force **Marquee Mode**, verify clamping, scrolling, and that the `MODE` only changes after the train has fully panned across the screen.
- Trigger a mock XML payload with 2 coaches to force `MAX_COACH_WIDTH` clamping, verify centering and stationary 3-second mode rotation.

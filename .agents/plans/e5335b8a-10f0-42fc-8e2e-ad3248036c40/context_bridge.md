# Context Bridge: Weather Icon Rendering Debugging

## 📍 Current State & Focus
We are investigating a persistent UI bug where the weather icon is not rendering on the departures board. The firmware is successfully fetching and parsing weather data from OpenWeatherMap (e.g., retrieving condition ID 804 correctly and resolving it to the `'E'` glyph for overcast clouds), and assigning a state of `READY` to the `WeatherStatus` object. 

During our last step, we entirely bypassed the layout engine's color management and the standard internal drawing pipeline. We explicitly filled an area black, explicitly set `display.setDrawColor(1)`, explicitly selected `WeatherIcons11`, and raw-drew the string directly from coordinates using deterministic U8G2 API calls inside `weatherWidget::render()`. The device flashed successfully, but the user reported that the target icon is **still** invisible on the physical display. 

We are temporarily halting hardware-first debugging to build/integrate the `layoutsim` (WASM display simulator) to properly visualize and trace the raw layout bounds and states without repeatedly compiling to physical hardware.

## 🎯 Next Immediate Actions
1. **Bring Up the WASM Simulator**: The web simulator must be running. Check the `tools/layoutsim` directory and spin up the frontend. 
2. **Execute and Verify in WASM**: Run the simulation specifically configured for the National Rail layout to emulate the exact U8G2 state pipeline.
3. **Trace the Glyph Flow**: Add print logging straight into `modules/displayManager/widgets/weatherWidget.cpp` or the simulator harness to explicitly verify the rendering boundaries in a controlled environment. 
4. **Resurrect the Custom Override**: Determine why explicitly drawing with raw generic primitives (which successfully drew `wifiStatusWidget`) failed for `weatherWidget`. The code for `weatherWidget::render` is currently hacked to bypass `drawText`.

## 🧠 Decisions & Designs
- We established that the widget does not overlap with adjacent widgets like `wifiWarning` (they are 12 pixels apart horizontally).
- We confirmed the layout dynamically pushes the widget coordinates into place perfectly (`192, 0`).
- We proved `weatherWidget::renderAnimationUpdate` is being correctly iterated and tracks the delta change correctly when the `WeatherStatus` updates from `NO_DATA` to `READY`.
- The `bdf` to `txt` configuration for `WeatherIcons11` theoretically fits standard U8g2 `setFontPosTop` geometry.

## 🐛 Active Quirks, Bugs & Discoveries
- **Drawing Primitives Breakdown**: We suspected the `drawText` helper method (and specifically the `U8g2StateSaver` combined with `blankArea`) might be permanently caching a black draw color (`0`) locally, resulting in invisible text. We completely ripped this logic out of `weatherWidget::render` to test native `display.drawStr()` and forced `display.setDrawColor(1)`. This proved the bug was *not* just a simple draw color state leak.
- **`appContext` Synchronization**: We validated that the `nationalRailBoard` uses the identical memory-reference to `weatherStatus` manipulated by the background `weatherClient`. Data state is provably `READY`.

## 💻 Commands Reference
- Run flashed ESP32 tests: `./.agents/skills/hardware-testing/scripts/safe-flash.sh`
- Read active hardware serial output (if plugged in): `cat logs/latest-monitor.log`

## 🌿 Execution Environment
- Branch: Actively testing on ESP32 developer module. (Awaiting transition to WASM/simulated build environment).

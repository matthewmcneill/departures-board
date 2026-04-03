# Widget Architecture Alignment for Bus and Tube Boards

**Audit Checklist Status:**
- [x] House Style (CamelCase naming, standard headers)
- [x] Architectural Standards (SRP/OCP, Injection patterns, UI geometry logic separation)
- [x] Resource Impact (Heap allocations, widget tree memory)
- [ ] UI Design Mockup (N/A – Backend UI alignment only)

This plan outlines the steps to align the Bus and TfL (Tube) board widget architectures with the National Rail board pattern, specifically splitting the `serviceListWidget` into a `row0Widget` and a trailing `servicesWidget`, and ensuring all boards have `weatherWidget` configured.

## Architecture & Resource Impact
> [!TIP]
> - **Heap Usage:** The `serviceListWidget` uses vector allocation per column inside the layout logic. By adding `row0Widget` alongside `servicesWidget`, we mildly increase peak heap usage during the layout tick, but `U8g2` display RAM remains unaffected since this is a UI logical separation. Wait, we should reduce the max capacity of the `servicesWidget` since it will render one fewer row, meaning the memory impact is almost perfectly neutral.
> - **Naming:** All updated C++ members conform to project `camelCase` standards (`row0Widget`, `servicesWidget`, `weather`).

## Background Context
- The National Rail board successfully achieved smooth scrolling by splitting its list widget: a static `row0Widget` (for the upcoming service which can toggle "Via" text) and a trailing `servicesWidget` containing subsequent services.
- The user wishes to align Bus and TfL Tube boards to this same pattern.
- Furthermore, we aim to ensure all boards have `wifiStatusWidget`, `weatherWidget`, and `locationAndFiltersWidget`. Both Bus and Tube currently possess `locationAndFiltersWidget` and `wifiStatusWidget`, but lack the `weatherWidget`, and they use a single combined `servicesWidget`.

## User Review Required

> [!IMPORTANT]
> - National Rail layout dimensions (y: 13 for row0, y: 26 for services) leave 1px gaps or rely on specific column heights. For Bus and Tube, the current single list is `h: 39` at `y: 12`. Sticking to exactly 13px per row, `row0Widget` will be `y: 12`, `h: 13`, and `servicesWidget` will be `y: 25`, `h: 26`. Let me know if you would prefer different y-coordinates.
> - **Clock Added:** As requested, I will add the `sysClock` to both Tube and Bus. This means the top header row for ALL boards will become exactly identical: `location` gets width 192, `weather` at 192 (w: 12), `wifi` at 204 (w: 12), and `sysClock` at 216 (w: 40). 

## Proposed Changes

### TfL Tube Board

#### [MODIFY] iTflLayout.hpp
- **Injection:** Add `#include <widgets/weatherWidget.hpp>` and `#include <widgets/clockWidget.hpp>`.
- **Properties:** Add `weatherWidget weather;`, `clockWidget sysClock;`, and `serviceListWidget row0Widget;`.
- **Methods:** Update `tick()`, `render()`, and `renderAnimationUpdate()` to delegate calls to `weather`, `sysClock`, and `row0Widget`.

#### [MODIFY] tflBoard.cpp
- **Data Binding (updateData):** Inside the `for` loop where TfL services are added, duplicate the `addRow` call to concurrently add the row to both `activeLayout->row0Widget` and `activeLayout->servicesWidget`.
- **Render State:** In `render()`, bind the `setVisible` state of `row0Widget` symmetrically with `servicesWidget` depending on `numServices > 0`.

#### [MODIFY] tflBoard/layouts/layoutDefault.json
- **Top Bar Alignment:** Match National Rail exactly: Shrink `locationAndFilters` width to `192`. Inject `weatherWidget` at `x: 192, w: 12`. Adjust `wifiWarning` to `x: 204, w: 12`. Inject `clockWidget` at `x: 216, w: 40`.
- **List Split:** Rename the current `servicesWidget` to `row0Widget`, change its height to `h: 13`, set `"skipRows": 0`, and `"maxRows": 1`. Then copy the entire widget definition, name it `servicesWidget`, shift it to `y: 25` with `h: 26`, and add `"skipRows": 1`.

---

### Bus Board

#### [MODIFY] iBusLayout.hpp
- **Injection:** Add `#include <widgets/weatherWidget.hpp>` and `#include <widgets/clockWidget.hpp>`.
- **Properties:** Add `weatherWidget weather;`, `clockWidget sysClock;`, and `serviceListWidget row0Widget;`.
- **Methods:** Update `tick()`, `render()`, and `renderAnimationUpdate()` to delegate calls to `weather`, `sysClock`, and `row0Widget`.

#### [MODIFY] busBoard.cpp
- **Data Binding (updateData):** Inside the `numServices` loop, call `addRow` for both `row0Widget` and `servicesWidget`.
- **Render State:** Symmetrically bind `.setVisible()` for both `row0Widget` and `servicesWidget`.

#### [MODIFY] busBoard/layouts/layoutDefault.json
- **Top Bar Alignment:** Match National Rail exactly: Shrink `locationAndFilters` width to `192`. Inject `weatherWidget` at `x: 192, w: 12`. Adjust `wifiWarning` to `x: 204, w: 12`. Inject `clockWidget` at `x: 216, w: 40`.
- **List Split:** Similar to TfL, fork the combined `servicesWidget` definition into a 1-row `row0Widget` (`h: 13, maxRows: 1`) and a trailing `servicesWidget` (`y: 25, h: 26, skipRows: 1`). Both copy the existing column alignment config.

## Open Questions

None - User confirmed keeping simple row population logic directly inside `updateData()`.

## Verification Plan

### Automated Tests
1. Perform compilation of the test environment (`platformio run -e test`) to verify header injections and API signatures.
2. Ensure UI limits compile successfully with the altered `iTflLayout` and `iBusLayout` instantiations.
3. Run the WASM display simulator (`tools/layoutsim/scripts/dev_server.py`) with mock data payloads, verifying that the icon renders in the top right corner without overlapping the location text.

### Manual Verification
1. Flash changes via `/flash-test`.
2. Cycle the device board display mode to Tube (TfL) and Bus.
3. Observe if the top destination row remains static while the trailing rows (2 and 3) scroll gracefully. Observe the presence of the weather icon.

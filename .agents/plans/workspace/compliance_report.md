# Codebase Compliance Review

We ran an automated compliance scan on all `.cpp` and `.hpp` files within `src/` and `modules/` to verify adherence to the project's **house-style-documentation** standards, specifically focusing on the presence of standard header blocks.

## Summary
- **Total files scanned**: ~100
- **Files with formatting issues**: 38

## Layout Files (Missing Full Header)
The following layout files are completely missing the license, module name, and description headers:
- `modules/displayManager/boards/systemBoard/layouts/layoutDiagnostic.hpp`
- `modules/displayManager/boards/systemBoard/layouts/layoutDiagnostic.cpp`
- `modules/displayManager/boards/tflBoard/layouts/layoutDefault.hpp`
- `modules/displayManager/boards/tflBoard/layouts/layoutDefault.cpp`
- `modules/displayManager/boards/busBoard/layouts/layoutDefault.hpp`
- `modules/displayManager/boards/busBoard/layouts/layoutDefault.cpp`
- `modules/displayManager/boards/nationalRailBoard/layouts/layoutDefault.hpp`
- `modules/displayManager/boards/nationalRailBoard/layouts/layoutReplica.hpp`
- `modules/displayManager/boards/nationalRailBoard/layouts/layoutReplica.cpp`
- `modules/displayManager/boards/nationalRailBoard/layouts/layoutDefault.cpp`
- *(Note: Layouts are heavily generated/templated. We should consider if full headers should simply be added to the layout generator scripts instead of the generated files.)*

## Third-Party Scripts / Fonts
- `modules/displayManager/fonts/fonts.cpp` (Missing license and full header block. Generated code from txt_to_bdf).

## Standard Modules (Missing 'Exported Functions/Classes:' in Header)
The following files have the license and description but lack the explicit `Exported Functions/Classes:` index required by the house style:

### Boards
- `firmwareUpdateBoard.cpp`
- `loadingBoard.cpp`
- `diagnosticBoard.cpp / .hpp`
- `helpBoard.cpp`
- `sleepingBoard.cpp`
- `wizardBoard.cpp`
- `splashBoard.cpp`
- `messageBoard.cpp`
- `iNationalRailLayout.cpp`

### Widgets
- `weatherWidget.cpp / .hpp`
- `locationAndFiltersWidget.cpp`
- `scrollingTextWidget.cpp`
- `progressBarWidget.cpp`
- `scrollingMessagePoolWidget.cpp`
- `serviceListWidget.cpp`
- `drawingPrimitives.cpp`
- `systemMessageWidget.cpp`
- `imageWidget.cpp`
- `labelWidget.cpp`

### Core Services
- `fonts.hpp`
- `messagePool.cpp`
- `dataManager.cpp`
- `webServer.cpp / .hpp`
- `portalAssets.cpp`

## Recommendation
Since there are 38 files lacking strictly required formatting (predominantly missing `Exported Functions/Classes:`), I recommend adding a phase to our current implementation plan. We can write an automated patching script to insert the missing `Exported Functions/Classes:` block into `.cpp` files with `None` as value, and manually update the essential `.hpp` files. For the layout files, we can update the `generate_layouts.py` script.

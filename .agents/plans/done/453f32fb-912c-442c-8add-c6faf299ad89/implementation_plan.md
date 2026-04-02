[x] Reviewed by house-style-documentation - passed
[x] Reviewed by architectural-refactoring - passed (Script fix only, no architecture changes)
[x] Reviewed by embedded-systems - passed (WASM simulator fix only, no flash/RAM impact)

# Fix Layout Simulator Data Binding and Widget Mappings

The recent refactoring of the display simulator decoupled layout configuration into JSON files but introduced a mapping discrepancy. The `gen_sim_registry.py` script automatically scans the firmware headers to build a mock simulator registry, but it currently renames layout class names (e.g., stripping `iNationalRailLayout` into `nationalRailBoard`). However, the `layoutDefault.json` definitions continue to identify themselves using the original C++ interface names (`"layout": "iNationalRailLayout"`). This mismatch prevents the `loadLayoutProfile()` function from instantiating any mock widgets, resulting in empty display canvases, incorrect validation metadata, and broken IDE data binding.

## User Review Required
No major system changes requiring specific user intervention. The fix correctly re-aligns the simulator registry generator with the JSON schema.

## Proposed Changes

### Simulator Pipeline (tools/layoutsim/scripts)
#### [MODIFY] [gen_sim_registry.py](tools/layoutsim/scripts/gen_sim_registry.py)
- Refactor the string manipulation block within `main()` to preserve the raw layout class name (e.g., `iNationalRailLayout`) instead of converting it to a formatted board name (e.g., `nationalRailBoard`).
- This will ensure the generated conditional statements in `generated_registry.hpp` perfectly match the `"layout"` value loaded from JSON at runtime.

## Verification Plan
### Automated Tests
- Run the python unit script: `python3 tools/layoutsim/scripts/gen_sim_registry.py` 
- Validate the output in `generated_registry.hpp` matches `if (layoutName == "iNationalRailLayout")`.

### Manual Verification
- Recompile the simulator via `npm run build` or the WASM builder script.
- Navigate to the local web simulator and verify the displays populate with expected widget outlines and valid mock data.

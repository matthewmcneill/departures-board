# Consolidate Layout Simulator Tool

> [!NOTE]
> **Implementation Plan Review Status**
> - **House Style**: 🟢 PASSED (Applied `camelCase` and license headers)
> - **Architectural Standards**: 🟢 PASSED (Encapsulation of simulator tool, SRP)
> - **UI Design**: 🟢 PASSED (ASCII mockup of interactive prompt)
> - **Resource Impact**: 🟢 PASSED (Zero hardware footprint)

This plan consolidates the Layout Simulator tool by moving the `dataHarvester.py` script from the repository root into the tool's source tree (`tools/layoutsim/scripts/`). It also enhances the `layoutsim.sh` entry point with an interactive check for mock data, offering to run the harvester if no data is found.

## User Review Required

> [!IMPORTANT]
> - Moving `scripts/dataHarvester.py` may break any external scripts or manual workflows that rely on the root path. However, our analysis suggests no other internal tools reference this path.
> - The interactive check in `layoutsim.sh` will interrupt the startup flow if mock data is missing, but only once (or until data is provided).

## Proposed Changes

### tools/layoutsim [Component]

#### [DELETE] [dataHarvester.py](file:///Users/mcneillm/Documents/Projects/departures-board/scripts/dataHarvester.py)
- Move to new location.

#### [NEW] [dataHarvester.py](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/scripts/dataHarvester.py)
- Relocated from root.
- Update `DEFAULT_OUT_DIR` to use a path relative to the new script location:
  ```python
  DEFAULT_OUT_DIR = os.path.join(os.path.dirname(__file__), "..", "mock_data")
  ```

#### [MODIFY] [layoutsim.sh](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/layoutsim.sh)
- Add logic to check for `.json` files in `mock_data/`.
- If missing, prompt the user for a CRS code and run the harvester before starting the server.

```bash
# --- Mock Data Check ---
MOCK_DATA_DIR="$SCRIPT_DIR/mock_data"
# Count JSON files (excluding layout.schema.json)
JSON_COUNT=$(find "$MOCK_DATA_DIR" -name "*.json" ! -name "layout.schema.json" | wc -l)

if [ "$JSON_COUNT" -eq "0" ]; then
    echo "⚠️  No mock data found in $MOCK_DATA_DIR."
    echo "The simulator works best with real-world snapshots."
    read -p "❓ Would you like to harvest some real data now? (y/N) " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        read -p "➡️  Enter 3-digit CRS code (e.g., EUS for Euston): " crs
        # Run harvester and wait for completion
        python3 "$SCRIPT_DIR/scripts/dataHarvester.py" --station "$crs"
        echo "✅ Data harvested. Starting simulator..."
    else
        echo "⚠️  Continuing with empty simulator. You can run dataHarvester manually later."
    fi
fi
```

### Documentation [Component]

#### [MODIFY] [DeveloperGuide.md](file:///Users/mcneillm/Documents/Projects/departures-board/docs/DeveloperGuide.md)
- Update the "Running the Simulator" section to mention `layoutsim.sh` as the primary entry point and briefly explain the automatic data harvesting check.

## Verification Plan

### Automated Tests
- Run `./tools/layoutsim/layoutsim.sh` with an empty `mock_data/` directory.
- Verify prompt logic.
- Run harvester via prompt and verify files are created in `tools/layoutsim/mock_data/`.
- Run again and verify prompt is skipped once data is present.

### Manual Verification
- Verify the simulator server starts correctly and serves the newly harvested data.

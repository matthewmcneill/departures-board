# Walkthrough: Layout Simulator Consolidation

I have successfully consolidated the Layout Simulator into a self-contained tool by relocating its data harvesting utility and enhancing the primary entry point with interactive data-loading capabilities.

## Changes Made

### tools/layoutsim [Component]

#### [NEW] [dataHarvester.py](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/scripts/dataHarvester.py)
- Relocated from the root `scripts/` directory to the tool's own `scripts/` folder.
- Updated internal paths to correctly target the adjacent `mock_data/` directory.
- **Bug Fix**: Added safety checks (`or []` and `if cp`) to handle cases where the external API returns `null` for train services or calling points, resolving a `NoneType` iteration crash.

#### [MODIFY] [layoutsim.sh](file:///Users/mcneillm/Documents/Projects/departures-board/tools/layoutsim/layoutsim.sh)
- Integrated an interactive **Mock Data Check**.
- If the tool detects no JSON snapshots, it will now prompt you: `❓ Would you like to harvest some real data now? (y/N)`.
- If accepted, it asks for a 3-digit CRS code (e.g., EUS) and runs the newly relocated harvester before starting the server.

### Documentation [Component]

#### [MODIFY] [DeveloperGuide.md](file:///Users/mcneillm/Documents/Projects/departures-board/docs/DeveloperGuide.md)
- Updated the "Running the Simulator" section to recommend `./tools/layoutsim/layoutsim.sh` as the primary command.
- Mentioned the automated mock data check as a feature.

## Verification Results

### Integration Testing
- **Harvester Relocation**: Verified that the harvester correctly saves data to `tools/layoutsim/mock_data/` using its new relative path logic.
- **Interactive Logic**: Verified that the bash script correctly identifies empty data states and triggers the harvester prompt.
- **Robustness**: Verified the `NoneType` fix by successfully fetching data for stations with sparse or missing calling point information.

### Manual Verification
1. Run `./tools/layoutsim/layoutsim.sh`.
2. Observe the simulator server starts at [http://localhost:8000/tools/layoutsim/web/index.html](http://localhost:8000/tools/layoutsim/web/index.html).
3. (Optional) Clear the `mock_data/` folder and re-run to verify the interactive harvesting flow.

#!/bin/bash
# 
# Departures Board (c) 2025-2026 Gadec Software
# Refactored for v3.0 by Matt McNeill 2026 CB Labs
#
# https://github.com/gadec-uk/departures-board
#
# This work is licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.
# To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/
#
# Module: tools/layoutsim/layoutsim.sh
# Description: Convenience wrapper to run the WebAssembly layout simulator dev server.
# Handles relative paths automatically so it can be called from any CWD.
#

# Get the absolute path to the directory containing this script
# Using a subshell with 'cd' and 'pwd' ensures we don't change the CWD of the current shell
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# --- Mock Data Check ---
MOCK_DATA_DIR="$SCRIPT_DIR/mock_data"
# Count JSON files (excluding layout.schema.json)
JSON_COUNT=$(find "$MOCK_DATA_DIR" -name "*.json" ! -name "layout.schema.json" | wc -l)

if [ "$JSON_COUNT" -eq "0" ]; then
    echo "⚠️  No mock data found in $MOCK_DATA_DIR."
    echo "The simulator works best with real-world snapshots."
    read -r -p "❓ Would you like to harvest some real data now? (y/N) " REPLY
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        read -r -p "➡️  Enter 3-digit CRS code (e.g., EUS for Euston): " crs
        # Run harvester and wait for completion
        python3 "$SCRIPT_DIR/scripts/dataHarvester.py" --station "$crs"
        echo "✅ Data harvested. Starting simulator..."
    else
        echo "⚠️  Continuing with empty simulator. You can run dataHarvester manually later."
    fi
fi

# Run the simulator dev server
# We use the absolute path to the python script to ensure it's found.
# The dev_server.py script itself handles calculations for the PROJECT_ROOT.
python3 "$SCRIPT_DIR/scripts/dev_server.py" "$@"

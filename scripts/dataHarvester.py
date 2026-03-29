#!/usr/bin/env python3
"""
Departures Board (c) 2025-2026 Gadec Software
Refactored for v3.0 by Matt McNeill 2026 CB Labs

Module: scripts/dataHarvester.py
Description: Fetches real-time departure data from public APIs and converts it 
             into the JSON format required by the Layout Simulator.
"""

import os
import sys
import json
import argparse
import requests

# Default Output Directory
DEFAULT_OUT_DIR = os.path.join(os.path.dirname(__file__), "..", "tools", "layoutsim", "mock_data")

def harvest_national_rail(crs, api_url, out_path):
    """
    Fetches data from a Huxley-compatible National Rail proxy (Open API).
    """
    print(f"Harvesting National Rail data for {crs} via {api_url}...")
    try:
        response = requests.get(f"{api_url}/all/{crs}/10")
        response.raise_for_status()
        data = response.json()

        mock_data = {
            "header": {
                "title": data.get("locationName", "Unknown Station"),
                "callingPoint": "via " + (data.get("filterLocationName", "") if data.get("filterLocationName") else "Main Line"),
                "platform": ""
            },
            "services": [],
            "messages": [m.get("value", "") for m in data.get("nrccMessages", []) if m.get("value")],
            "weather": { "id": 800, "isNight": False }
        }

        for i, s in enumerate(data.get("trainServices", [])):
            ordinal = f"{i+1}{'st' if i==0 else 'nd' if i==1 else 'rd' if i==2 else 'th'}"
            time = s.get("std", "")
            dest = s.get("destination", [{}])[0].get("locationName", "Unknown")
            plat = s.get("platform", "-")
            status = s.get("etd", "On Time")
            
            mock_data["services"].append([ordinal, time, dest, plat, status])

        with open(out_path, 'w') as f:
            json.dump(mock_data, f, indent=2)
        print(f"Successfully saved to {out_path}")

    except Exception as e:
        print(f"Error harvesting National Rail: {e}")

def main():
    parser = argparse.ArgumentParser(description="Harvest real departure data for the Layout Simulator.")
    parser.add_argument("--source", choices=["nr", "tfl"], default="nr", help="Data source (National Rail or TfL)")
    parser.add_argument("--station", required=True, help="Station identifier (e.g., EUS for Euston)")
    parser.add_argument("--api", default="https://huxley2.azurewebsites.net", help="API Base URL (for NR, uses Huxley proxy)")
    parser.add_argument("--out", help="Output filename (relative to mock_data/)")

    args = parser.parse_args()

    out_filename = args.out if args.out else f"{args.station.lower()}.json"
    out_path = os.path.join(DEFAULT_OUT_DIR, out_filename)

    if args.source == "nr":
        harvest_national_rail(args.station, args.api, out_path)
    else:
        print("TfL harvester not yet implemented. PRs welcome!")

if __name__ == "__main__":
    main()

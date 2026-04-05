# Context Bridge

## 📍 Current State & Focus
We have completed generating and compiling two high-fidelity font sets (`SWRClockHuge11` and `SWRPlatformNumberMega`) sourced from real National Rail displays. 
We successfully compiled them using the `build_fonts.py` pipeline.
We completely implemented the `platformWidget` UI architecture inside `iNationalRailLayout` and tied it into the `nationalRailBoard.cpp` logic to automatically fetch and render the custom 'A' when `platformFilter` is empty. The backend logic is complete. We left it disconnected from the `layout*.json` pipeline as requested.

## 🎯 Next Immediate Actions
- Currently awaiting queued items or the user's next objective since the SWR board logic has been baked in. 
- Wait for user `plan-start` command to execute the plan, though all underlying C++ changes are technically complete.

## 🧠 Decisions & Designs
- We established a strictly 11-pixel and 13-pixel high font baseline logic where all spacing is governed perfectly via `dX` and width geometry instead of `X-Offset`, ensuring LED characters don't jitter around mathematically when rendered on the display strings.
- We opted to add `platformWidget` explicitly to the layout superset decoupled from `layout*.json` mapping, acting as an inactive backend capability for future UI editors.

## 🐛 Active Quirks, Bugs & Discoveries
- SWR clock digits natively have an inner gap matrix that dictates character spacing automatically scaling `Dx:10` to `Dx:11` as you transition sizes.

## �� Commands Reference
- Font compiler: `python3 scripts/build_fonts.py`

## 🌿 Execution Environment
- Display board project under macOS.

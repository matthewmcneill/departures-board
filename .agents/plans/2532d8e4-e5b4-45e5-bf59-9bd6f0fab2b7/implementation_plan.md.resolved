# Platform Number Mega-Widget Integration

We need to add a dedicated label widget to the architectural superset specifically for rendering the new `SWRPlatformNumberMega` font, and wire it up to automatically pull the active platform filter state.

## Proposed Changes

### `iNationalRailLayout.hpp`
- Add `labelWidget platformWidget;` to the public superset variables.
- Append `platformWidget` down the layout pipeline inside `tick()`, `render()`, and `renderAnimationUpdate()`.

### `nationalRailBoard.cpp`
We will hook into the `updateData()` cycle (around line 245, where `locationAndFilters` gets its text).
We will add the logic to explicitly populate this new label:
```cpp
if (platformFilter[0] != '\0') {
    activeLayout->platformWidget.setText(platformFilter);
} else {
    // No specific filter applied = show "All" platforms, utilizing the new custom graphic
    activeLayout->platformWidget.setText("A");
}
```

### `layoutGadec.json`
We will inject the new widget definition so the layout system instantiates it:
```json
{
  "id": "platformWidget",
  "type": "labelWidget",
  "geometry": {
    "x": X,
    "y": Y,
    "w": W,
    "h": 13
  },
  "visible": true,
  "font": "SWRPlatformNumberMega"
}
```

## Open Questions
> [!IMPORTANT]
> **Widget Geometry Coordinates**
> Where exactly in the `layoutGadec.json` UI should this giant platform indicator be placed? E.g., next to the system clock, or up at the top right of the screen? I need the exact `x`, `y`, `w`, and `h` bounding box values you want for `platformWidget` in the JSON config.

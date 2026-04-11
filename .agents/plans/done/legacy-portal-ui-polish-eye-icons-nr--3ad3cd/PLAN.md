---
name: "Portal UI Polish: Eye Icons, NR & TfL Logos"
description: "Corrected the inverted logic for the password visibility toggle and upgraded the National Rail and TfL provider logos to their official SVG versions. Verified the fixes on live hardware across multipl..."
created: "2026-03-16"
status: "DONE"
commits: []
---

# Summary
Corrected the inverted logic for the password visibility toggle and upgraded the National Rail and TfL provider logos to their official SVG versions. Verified the fixes on live hardware across multiple deployment iterations. Addressed initial stale-code issues through clean builds and forced browser cache bypass.

## Key Decisions
- **CSS-Driven Toggle**: Transitioned the password visibility logic to rely on the CSS `active` class. This resolved conflicts with `!important` rules in the dark-mode CSS and ensured hardware-consistent performance.
- **Icon Logic Harmonization**: Standardized the visibility icon states to use the "Slashed Eye" for the visible password state and the "Open Eye" for the masked state, matching modern digital accessibility patterns.
- **Provider Logo Refresh**: Replaced outdated logos in the `PROVIDER_LOGOS` registry and selection modals with high-resolution, branded SVGs (National Rail double-arrow and TfL Roundel with `#0019A8` fill).
- **Automated Validation Integration**: Verified all UI changes using both local simulation and live hardware subagent testing, ensuring that user-provided formatting changes did not introduce regressions.

## Technical Context

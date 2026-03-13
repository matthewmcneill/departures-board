# Implementation Plan: GraphWidget (Baseline)

## Goal
Add a `GraphWidget` to show a sparkline of recent bus arrivals.

## Proposed Changes
- Create `GraphWidget.hpp` and `GraphWidget.cpp`.
- Add a data structure to store the recent arrivals (e.g., `std::vector<int> history`).
- Implement the drawing logic using `u8g2.drawLine()`.
- Add the widget to the relevant boards.

## Verification
- Run the board and check the display to see if the graph appears.
- Verify that the graph updates as new data arrives.

## Performance
- The widget will provide an easy way to see arrival history at a glance.

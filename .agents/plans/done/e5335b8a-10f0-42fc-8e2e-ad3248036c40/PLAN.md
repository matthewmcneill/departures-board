---
name: "Migration to HTTPS (`NetworkClientSecure`) for Weather Fetch"
description: "Diagnosed and resolved a weather condition loading failure caused by OpenWeatherMap deprecating unencrypted HTTP support on the free tier. Upgraded `weatherClient.cpp` to use `NetworkClientSecure` wit..."
created: "2026-04-03"
status: "DONE"
commits: ['6a8a534']
---

# Summary
Diagnosed and resolved a weather condition loading failure caused by OpenWeatherMap deprecating unencrypted HTTP support on the free tier. Upgraded `weatherClient.cpp` to use `NetworkClientSecure` with port `443` and `setInsecure()` rather than the legacy `WiFiClient`. This ensures the streaming JSON parser actually receives the active weather stream and that `WeatherStatus` objects hydrate successfully globally, restoring display icons across all boards.

## Technical Context
- [sessions.md](sessions.md)

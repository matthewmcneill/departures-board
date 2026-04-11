---
name: "Sequential API Key Validation & Memory Optimization"
description: "Implemented sequential API key validation in the web portal to prevent ESP32 network module saturation and handled stack overflow issues by offloading SSL clients to the heap. Improved visual feedback..."
created: "2026-03-16"
status: "DONE"
commits: []
---

# Summary
Implemented sequential API key validation in the web portal to prevent ESP32 network module saturation and handled stack overflow issues by offloading SSL clients to the heap. Improved visual feedback for API tests and network disconnection states.

## Key Decisions
- **Sequential Validation Logic**: Orchestrated API key tests to execute one-by-one with a 1-second delay. This ensures hardware stability during the intensive SSL handshake process and provides a predictable user experience with sequential status dot updates.
- **Heap-Allocated SSL Clients**: Refactored `NationalRailDataSource` and `TfLDataSource` to use heap-allocated `WiFiClientSecure` objects. This prevents stack overflow crashes during complex TLS handshakes without introducing long-term fragmentation (short-lived allocations).
- **Network Error Observability**: Integrated a `isWifiPersistentError` method and updated `wifiStatusWidget` with a blinking logic (after 30s) to clearly signal prolonged connectivity loss on the hardware display.
- **Safe Password/Key Masking**: Implemented a "No Change" detection pattern for password and API key fields. The system now ignores placeholders (`••••••••`) during saves, preventing accidental erasure of existing secrets.

## Technical Context

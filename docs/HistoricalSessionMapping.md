# Historical Session Mapping

This document maps historical development sessions to the components and sections of the [System Specification Document](file:///Users/mcneillm/Documents/Projects/departures-board/docs/SystemSpecificationDocument.md).

## Mapping Summary

| Session ID / Hash | Date | Goal / Key Feature | Primary SSD Section(s) |
| :--- | :--- | :--- | :--- |
| 685a3ce5-c3cd-41b8-a78a-3eea17b2e153 | 2026-03-18 | Screensaver Clock Enhancements | 5. Detailed Component (ClockWidget), 6. UI Design |
| 16b784aa-e89e-432b-a85a-ff992f95c8dc | 2026-03-18 | Dynamic Default Tab Selection | 6. UI Design (Navigation) |
| 3ddab13a-27ec-4349-8349-73ad04f384b6 | 2026-03-18 | Displays Tab Overhaul | 6. UI Design |
| b5ab1a93-935d-456a-b852-6ad8393fa56b | 2026-03-18 | Bus Data Fetching & Network Robustness | 5. Detailed Component (BusSource), 9. Non-Functional |
| b7be186a-30f3-4c9c-b3ed-570c3cf14d60 | 2026-03-17 | Web Portal HTML Documentation | 6. UI Design |
| 8abec268-ceff-449c-acb1-e360604f3100 | 2026-03-17 | Time Manager DI Refactor | 3. Architecture, 7. System Services (Time) |
| 22312c00-ae60-410e-bcc6-09a7b478fc93 | 2026-03-17 | Session Wrap-Up & House Style Audit | 10. Appendices |
| 6e8a57d1-dcf6-4726-920f-6f9858ac482c | 2026-03-17 | Archiving Legacy Web Infrastructure | 8. Infrastructure |
| 86f1db99-ada0-48ff-ab8d-2c4e835ebf2d | 2026-03-17 | Debugging Feed Failures | 5. Detailed Component (Feeds) |
| 855c3f99-f0c2-45bd-a3ec-6965fc983851 | 2026-03-17 | Unified Test Queue Implementation | 6. UI Design, 5. Component (WebHandlers) |
| 1bb86576-d754-4e3b-8f87-cb92edbc818f | 2026-03-17 | Clock Display Rendering Issue | 5. Component (ClockWidget) |
| 3a01e364-7839-42ce-bf75-7dc1a7930ed9 | 2026-03-17 | UI Button Style Proposal | 6. UI Design |
| fe2cc001-b748-46c2-beb5-4b2e002d6023 | 2026-03-17 | Diagnosing ESP32 Board Startup | 9. Non-Functional |
| 2be270b5-2932-4a27-a371-d6bf2d93ebcd | 2026-03-17 | TimeManager strict DI refactoring | 3. Architecture, 7. System Services |
| 2ec805fd-2f7b-43bd-a86a-6d17347e6fe4 | 2026-03-17 | Archiving Legacy Web & Renaming Portal | 8. Infrastructure |
| cf3f6db7-7ac3-4267-8341-fccc4937cf71 | 2026-03-17 | Board Editor Refinements (Tube, Bus Filter) | 5. Detailed Component (Boards) |
| 1e892e27-f765-4b90-b825-c611eed22094 | 2026-03-17 | Portal Navigation Reorder & Schedule Tab | 6. UI Design |
| 02d69dd3-1b75-448c-930d-3e14292b29fa | 2026-03-17 | Remove bustimes.org from API Key selection | 5. Component (WebHandlers), 7. Security |
| 1757534b-8e83-4ac3-9d12-bd1074fa2fbc | 2026-03-17 | Unified iDataSourceTest interface | 3. Architecture, 5. Component (DataSources) |
| 33fec7a6-4bb3-4e24-9cf7-8e28a07679c5 | 2026-03-17 | Optimization of TfL/NR API validation | 5. Component (DataSources), 9. Non-Functional |
| 45557708-ca25-4ac0-9ef4-5424065f90c7 | 2026-03-17 | National Rail API Test Failure Fix | 5. Component (NationalRail) |
| 9ba038e0-cb01-451a-90aa-107edccee7fe | 2026-03-17 | Feeds Tab UI & Async Diagnostics | 6. UI Design, 5. Component (Feeds) |
| ad513d90-fb1c-4c56-9f90-8fe6049e0594 | 2026-03-16 | Refining API Key UI and Build System | 7. Security, 8. Infrastructure |
| a57da37d-5394-4798-84f2-791ef8118b40 | 2026-03-16 | Repository-wide House Style Update | 10. Appendices |
| 43a2533e-f07d-418c-b934-1167fef97ee7 | 2026-03-16 | Sequential Validation and Memory Optimization | 9. Non-Functional, 3. Architecture |
| 7253f91e-a7b9-4b21-8a21-357988484c5b | 2026-03-16 | API Key validation UI & backend logic | 7. Security, 5. Component (WebHandlers) |
| 4f0d242c-6851-43f0-b4a8-9065a534315e | 2026-03-16 | Branded API Key Icons in Web Portal | 6. UI Design |
| f59e3969-179b-425f-b4c1-977c3ffab9f6 | 2026-03-16 | Portal UI Polish: Eye Icons, Logos | 6. UI Design |
| 36b83bfe-d960-4008-95c5-1d5d2bc7b326 | 2026-03-15 | Bootloop fix and UI for Keys | 9. Non-Functional |
| 2c182619-b8f2-4fb3-94c5-26c1a77fd51a | 2026-03-15 | Portal UI Debugging: WiFi & Connectivity | 6. UI Design, 7. System Services (WiFi) |
| d09180d0-b976-4ddf-858f-a977de894d05 | 2026-03-15 | Web Portal Test Suite & API Key Verification | 9. Non-Functional (QA), 7. Security |
| 61cae59c-a0c3-4f76-8082-1590261c7c79 | 2026-03-15 | Repository-wide House Style & Header Audit | 10. Appendices |
| 8783aa65-8a5f-4263-913a-68ac04023415 | 2026-03-15 | Web Portal Refinement: WiFi Reset | 7. System Services (WiFi), 6. UI Design |
| ad861467-eb82-4db6-ac73-5951a249d77d | 2026-03-14 | Additional drawing primitives | 5. Component (DisplayManager) |
| a748448f-24f2-4b58-8f72-c0d5ce22163f | 2026-03-14 | Web UI Performance & Data Reliability | 9. Non-Functional, 6. UI Design |
| cd253c52-5bcc-44c5-bf78-64593f457fd7 | 2026-03-14 | Weather Refactor & Config Standardization | 5. Component (Weather), 8. Config |
| bbd5c6df-f4f5-4ac2-9f80-8d782824009c | 2026-03-13 | Logger Encapsulation & Efficiency | 3. Architecture, 9. Non-Functional |
| d8f1c7fb-efc1-40d3-96e0-d3ca37de6f22 | 2026-03-13 | Debug logging to data retrievals | 9. Non-Functional |
| 26032d26-7d5d-41e6-826c-0319d6893787 | 2026-03-13 | Delete redundant IStation interface | 3. Architecture |
| 429fcc34-e7d5-42c0-b087-1e7659f16d9c | 2026-03-13 | Audit and Rationalize Build Flags | 8. Infrastructure |
| 2a05aad1-7642-43f7-9f4e-dd9a5431428b | 2026-03-13 | RSS Feed Debugging and Logging | 5. Component (RSS) |

*(Note: Additional legacy sessions from the brain directory (approx. 50-60) are categorized as "Legacy Evolution" and documented via code synthesis in the implementation phase.)*

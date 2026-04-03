# 📍 Current State & Focus
Completed a comprehensive review and critical evaluation of `String` usage in the firmware. Established a pragmatic strategy for refactoring: focusing on high-frequency/high-impact areas (Logging and API Builders) while keeping `String` for resilient, unpredictable network payloads.

# 🎯 Next Immediate Actions
1. **Draft Implementation Plan**: Create a plan for Phase 1: Logging Optimization. This involves refactoring the `Logger` to support `printf`-style formatting or `std::string_view` to eliminate 90% of heap-churning string concatenations.
2. **Audit Network Clients**: Identify the most critical URL builders in `weatherClient.cpp`, `busDataSource.cpp`, and `rssClient.cpp` to replace with `snprintf` vs. fixed buffers.

# 🧠 Decisions & Designs
- **Pragmatic Refactoring**: Decided against a 100% "String purge" to maintain compatibility with third-party libraries and preserve resilience for unknown-length payloads.
- **Priority Tiering**: Ranked `Logger` and API Builders as "High Priority" for stable firmware operation.
- **Buffer Safety**: Acknowledged the 8KB stack limitation on the `Data_Manager` task and the importance of avoiding large, wasteful stack-allocated buffers.

# 🐛 Active Quirks, Bugs & Discoveries
- **Stack Constraint**: The `Data_Manager` task operates on an 8KB stack, meaning large fixed buffers (e.g., >1KB) significantly increase the risk of stack overflow.
- **Library Reliance**: Many core ESP32/Arduino components (WiFi, HTTPClient) are strictly `String`-based, creating "friction" when trying to go pure C-string.

# 💻 Commands Reference
- `pio run` (Build verification)
- `git grep "String"` (Audit usage)

# 🌿 Execution Environment
- **Branch**: `main`
- **Testing**: Local research and codebase analysis on Mac. Hardware is referenced but not currently attached for flashing.

---
**⚠️ PORTABILITY RULE**: All in-repo paths are relative to the repository root.

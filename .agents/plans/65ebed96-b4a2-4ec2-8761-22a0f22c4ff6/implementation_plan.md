# Implementation Plan: System Specification Document Drafting

This plan outlines the iterative process for populating the `docs/SystemSpecificationDocument.md` by synthesizing historical session data, existing documentation, and current codebase state.

## Proposed Strategy

The drafting will be divided into four thematic phases to ensure logical flow and technical accuracy. Each phase will involve reading specified source files, analyzing recent code changes from the project log, and drafting the corresponding sections in the SSD.

### Phase 1: High-Level Foundation (Sections 1-3)
*   **Goal**: Define the purpose, context, and overarching architecture.
*   **Sources**:
    *   `docs/ArchitectureAndEncapsulation.md`
    *   `docs/AppContextStateMachine.md`
    *   `docs/NewMultiBoardArchitecture.md`
*   **Tasks**:
    *   Draft Introduction, Glossary, and Scope.
    *   Detail the Tech Stack (ESP32, PlatformIO, u8g2, Vanilla JS).
    *   Explain the dependency injection and state machine patterns.

### Phase 2: Data & Core Logic (Sections 4-5)
*   **Goal**: Document data flows and specific module implementations.
*   **Sources**:
    *   `docs/DisplaySystemArchitecture.md`
    *   `docs/NationalRailAPIHistory.md`
    *   `docs/WeatherSystemDesign.md`
    *   `.agents/project_log.md` (Recent refactors)
*   **Tasks**:
    *   Detail the Data Models (Board configurations, API tokens).
    *   Document major modules: `NationalRail`, `TfL`, `BusDataSource`, `WeatherClient`, `RSSReader`.
    *   Include error handling strategies (e.g., sequential test queue).

### Phase 3: Interface, Security & Services (Sections 6-7)
*   **Goal**: Detail the web portal, security models, and background services.
*   **Sources**:
    *   `docs/WebInterfaceDesign.md`
    *   `docs/PasswordFieldImplementationGuide.md`
    *   Recent sessions on API Key Registry and WiFi Reset.
*   **Tasks**:
    *   Describe the SPA architecture and asset pipeline (`portalBuilder.py`).
    *   Document Authentication (API Key Redaction) and Authorization logic.
    *   Explain Time Management (NTP) and WiFi connectivity states.

### Phase 4: Operations & Quality (Sections 8-10)
*   **Goal**: Document deployment, performance targets, and testing.
*   **Sources**:
    *   `docs/TestingApproach.md`
    *   `docs/MemoryArchitecture.md`
    *   `platformio.ini` (Build environments)
*   **Tasks**:
    *   Detail the CI/CD pipeline and PlatformIO build environments.
    *   Document memory management strategies (Heap vs Stack for SSL).
    *   Finalize appendices and cross-reference with [Historical Session Mapping](docs/HistoricalSessionMapping.md).
    
### Phase 5: Font Architecture & Build Pipeline
*   **Goal**: Document the custom font system and the U8G2 compression format.
*   **Sources**:
    *   `docs/fonts/ReverseEngineering.md`
    *   `docs/fonts/fonts_recovered.h`
*   **Tasks**:
    *   Explain the "Round Trip" font pipeline (BDF to C++).
    *   Detail the bit-level RLE compression used by U8G2.
    *   Document the font editing workflow and recommended tools.

### Phase 6: DisplayManager Expansion
*   **Goal**: Significantly expand Section 5.3 with depth on display models, animation, and widgets.
*   **Tasks**:
    -   Detail the `std::variant` memory buffering strategy for board persistent storage.
    -   Document the "Logic vs. Yield" animation dual-path approach.
    -   Describe the `iGfxWidget` composition model and its "Deduplication Pattern."
    -   Cross-reference the U8G2 font work from Section 11 into the widget rendering logic.

## Verification Plan

### Automated Verification
*   **Link Integrity**: Verify all internal file links and cross-references within the SSD work correctly.
    *   `find docs/SystemSpecificationDocument.md -name "*.md"` (check existence).
*   **Markdown Linting**: Ensure the document adheres to clean markdown formatting.

### Manual Verification
*   **User Review**: The user will review the drafted sections at each phase completion to ensure alignment with their mental model of the system.
*   **Content Accuracy**: Cross-check specific technical details (e.g., API endpoints, class names) against the actual source code.

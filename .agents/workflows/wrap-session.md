---
description: Session wrap-up: House style check, doc updates, validation, git commit, and project log update.
---

1.  **House Style Audit**: Run the `house-style-documentation` skill on all files modified during this session to ensure compliance with naming conventions (camelCase) and header documentation standards.
2.  **Architecture Documentation**: Identify any changes to system design or interfaces. Run the `architectural-refactoring` skill to update relevant documents in `/docs` (e.g., `NewMultiBoardArchitecture.md`, `WeatherSystemDesign.md`).
3.  **Validation**: Verify that the code compiles successfully (`pio run`) and that any relevant tests have passed. Record any notable build statistics (Flash/RAM usage).
4.  **Project TODO**: Review `TODO.md`. Mark completed items with `[x]` and add any new technical debt or follow-on tasks identified during the session.
5.  **Git Commit**: Prepare and execute a git commit with a "rich" description. The message should include:
    - A high-level summary of the changes.
    - Key architectural decisions made.
    - Any breaking changes or important reminders for future sessions.
6.  **Project Log**: Update `.agents/project_log.md` with:
    - Date and session summary.
    - Key decisions and their rationales.
    - The Git commit ID generated in the previous step.
7.  **Session Walkthrough**: Finalize the `walkthrough.md` for this session, ensuring it captures the final state and verification results.

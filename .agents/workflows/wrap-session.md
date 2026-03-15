---
description: Session wrap-up: House style check, doc updates, validation, git commit, and project log update.
---

1.  **House Style Audit**: Run the `house-style-documentation` skill on all files modified during this session to ensure compliance with naming conventions (camelCase) and header documentation standards.
2.  **Architecture Documentation**: Identify any changes to system design or interfaces. Run the `architectural-refactoring` skill to update relevant documents in `/docs` (e.g., `NewMultiBoardArchitecture.md`, `WeatherSystemDesign.md`).
3.  **Validation**: Verify that the code compiles successfully (`pio run`) and that any relevant tests have passed. **IMPORTANT**: If any code modifications were made during the House Style Audit or Architecture Documentation steps, you MUST rerun this validation to ensure only tested code is committed. Record any notable build statistics (Flash/RAM usage).
4.  **Test Suite Assessment**: Based on the lessons learned from this session (e.g., bugs found, edge cases missed, new functionality added), assess whether new or additional tests should be added to the test suite (local web portal, live hardware, or C++ unit tests). Document these required tests either by implementing them now or capturing them in the project backlog/TODO.
5.  **Project TODO**: Review `TODO.md`. Mark completed items with `[x]` and add any new technical debt or follow-on tasks identified during the session.
6.  **Git Commit**: Prepare and execute a git commit with a "rich" description. The message should include:
    - A high-level summary of the changes.
    - Key architectural decisions made.
    - Any breaking changes or important reminders for future sessions.
7.  **Project Log**: Update `.agents/project_log.md` with:
    - Date and session summary.
    - Key decisions and their rationales.
    - The Git commit ID generated in the previous step.
8.  **Session Walkthrough**: Finalize the `walkthrough.md` for this session, ensuring it captures the final state and verification results.
9.  **Queue Release**: Check if the current session ID is holding the lock in `.agents/queue.md`. If it is, forcefully release the lock by setting `Locked By` to `NONE` and clearing `Reason` and `Since`, following the `queue-release` workflow.

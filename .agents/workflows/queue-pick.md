---
description: Pick a specific implementation plan from the queue to work on.
---

1.  Read `.agents/queue.md`.
2.  Identify the target session ID or task name based on user input.
3.  If a match is found in the `## Pending Queue`:
    - Move that session to the `## Lock Status` section (set `Locked By`, `Reason`, `Since`).
    - Remove it from the `## Pending Queue`.
    - Notify the user that the task has been claimed and they should switch to that session's window.
4.  If not found:
    - List the available tasks in the queue and ask for clarification.

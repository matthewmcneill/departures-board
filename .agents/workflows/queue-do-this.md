---
description: Select the current session and kick off the implementation plan.
---

1.  Identify the current session ID and task summary.
2.  Read `.agents/queue.md`.
3.  Check if the lock is held by another session.
4.  If anyone else holds the lock:
    - Notify the user: "Hardware is still locked by [Session ID]. Please go to that window and `/queue-do-next` or use `/queue-release` to force it."
    - Stop here.
5.  If the lock is `NONE` or held by the current session:
    - Set the `Locked By` to the current session ID.
    - Update `Reason` to common task name or "Executing Implementation Plan".
    - Remove the current session from the `## Pending Queue` table if it was there.
    - Notify the user: "Lock claimed. Starting work now."
    - Proceed to execute the current implementation plan.
6.  **IMPORTANT**: Upon completion of implementation, DO NOT release the lock. You MUST retain the lock for testing and hardware verification. The lock should only be released via `/queue-do-next`, `/queue-release`, or as the final step of the `/wrap-session` workflow.

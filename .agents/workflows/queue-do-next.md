---
description: Release the current lock and hand over to the next session in the queue.
---

1.  Read `.agents/queue.md`.
2.  Check if the current session holds the lock.
3.  If yes:
    - Clear the `Lock Status` fields (set to `NONE`).
    - Move your session entry from `Pending Queue` to `Execution History`.
    - Look at the top of the `Pending Queue`.
    - If there is a next session, notify the user: "Next in line is [Session ID]. Please switch windows to continue."
4.  If no:
    - Inform the user that this session doesn't hold the lock.

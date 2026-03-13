# Queue Manager Skill

---
name: queue-manager
description: Manage the agent execution queue and hardware locks to prevent conflicts across multiple sessions.
---

## Overview
This skill provides the logic for interacting with `.agents/queue.md`. It is used to request locks, release them, and manage the waiting list.

## Core Instructions

### request_lock
- Read `.agents/queue.md`.
- Check `Locked By` field.
- If `NONE` or `[MY_SESSION_ID]`:
    - Update `Locked By` to `[MY_SESSION_ID]`.
    - Update `Reason` and `Since`.
    - Return SUCCESS.
- Else:
    - Add session to `Pending Queue` if missing.
    - Return LOCKED with the current holder's ID.

### release_lock
- Read `.agents/queue.md`.
- If `Locked By` == `[MY_SESSION_ID]`:
    - Set `Locked By` to `NONE`.
    - Clear `Reason` and `Since`.
    - Move own session to `Execution History`.
    - Notify user: "Lock released. Use `/queue-do-next` to trigger the next task."
    - Return SUCCESS.
- Else:
    - Return ERROR ("Not the lock holder").

### list_queue
- Read `.agents/queue.md`.
- Display the "Lock Status" and "Pending Queue" in a clean markdown table.

### do_next
- Read `.agents/queue.md`.
- If `Locked By` is NOT `NONE`, return ERROR.
- Take the FIRST item from the "Pending Queue".
- Update `Locked By` to that session ID.
- Notify the user: "Session [ID] has been granted the lock. Please switch to that window."

### pick_task
- Read `.agents/queue.md`.
- If `Locked By` is NOT `NONE`, return ERROR (must release first).
- Find the session in the "Pending Queue" by ID or Task summary.
- Update `Locked By` to that session ID.
- Remove from "Pending Queue".
- Notify the user: "Session [ID] has been granted the lock. Please switch to that window."

### claim_current_session
- Read `.agents/queue.md`.
- If `Locked By` is NOT `NONE` and NOT `[MY_SESSION_ID]`, return ERROR.
- Set `Locked By` to `[MY_SESSION_ID]`.
- Remove from "Pending Queue".
- Return SUCCESS and proceed to start the implementation plan.

# Queue Enforcement Rule

This rule ensures that NO agent session performs a build or flash operation if the system is locked by another session.

## Context
When working in parallel, multiple agents might try to access the ESP32 hardware or the `build/` directory simultaneously. This leads to failures and corrupted builds.

## Requirements
Before running ANY of the following commands:
- `pio run`
- `pio device upload`
- `pio device monitor`
- `npm run build`
- `npx next build`

You MUST:
1.  Read `.agents/queue.md`.
2.  If **Locked By** is NOT `NONE` and is NOT your current session ID:
    - ABORT the command.
    - Notify the user: "Hardware is currently locked by Session [ID]. I've added myself to the queue."
    - Add your current session to the "Pending Queue" if not already present.
3.  If **Locked By** is `NONE`:
    - Set **Locked By** to your session ID.
    - Set the **Reason** (e.g., "Building firmware").
    - Proceed with the command.
4.  After the command finishes (Success or Failure):
    - If you were the one holding the lock, you MAY keep it if you have more commands to run, OR release it if you are done.

## Handling Conflicts
If you encounter a lock, do NOT attempt to force it unless the user explicitly tells you to or uses `/queue-release`.

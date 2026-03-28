# Queue Enforcement Rule

This rule ensures that NO agent session performs a build or flash operation if the system is locked by another session.

## Context
When working in parallel, multiple agents might try to access the ESP32 hardware or the `build/` directory simultaneously. This leads to failures and corrupted builds.

## Requirements
Before running ANY of the following compilation or hardware commands:
- `pio run`
- `pio device upload`
- `pio device monitor`
- `npm run build`
- `npx next build`

You MUST ensure you hold the Hardware Lock.

1.  **Check the Lock**: Use `/plan-list` (or view `.agents/plans/lock.md` strictly as read-only) to check the `## Lock Status`.
2.  **If Locked by Another Session**: 
    - ABORT the compilation command immediately.
    - Notify the user: "Hardware is currently locked by Session [ID]. The task has safely been placed in the queue."
    - Run the `/plan-queue` workflow to systematically place your `implementation_plan.md` into the pending queue.
3.  **If the Lock is Free**: 
    - You MUST run the `/plan-start` workflow natively to formally and securely claim the lock.
    - DO NOT manually edit the `lock.md` file yourself.
    - Once `/plan-start` completes successfully, you may proceed with the compilation commands.
4.  **Releasing the Lock**: 
    - The lock must remain held until all development and verification is seamlessly completed.
    - Release the lock ONLY by running the terminal `/plan-wrap` workflow at the very end of your active task.

## Handling Conflicts
If you encounter a lock, do NOT attempt to force it unless the user explicitly tells you to or uses `/queue-release`.

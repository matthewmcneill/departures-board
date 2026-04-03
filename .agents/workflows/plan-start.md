---
description: Claim the hardware lock and begin the execution of the active plan.
---

1. Determine the active Plan ID based on your session lineage.
2. Read `.agents/plans/lock.md` and check the `## Lock Status` section.
3. If the lock is actively held by another session that is still working, halt and warn the user.
4. If the lock is `NONE` or you can safely take it (e.g. previous session is cleanly finished):
   - Set `Locked By` to your current session ID.
   - Update `Reason` to the name of the Plan.
5. Notify the user: "Hardware lock securely claimed. Starting implementation execution."
6. Begin executing the steps in `implementation_plan.md` and check off items in `task.md`.
7. **Important:** The hardware lock must remain held during your execution until you or the user runs `/plan-wrap` or `/queue-release`.

---
description: Claim the hardware lock and begin the execution of the active plan.
---

1. Determine the active Plan ID based on your session lineage.
2. Consult the hardware acquisition paths and verification endpoints natively instructed within the `@[.agents/skills/pio-manager]` skill to safely lock the workspace.
3. Notify the user: "Hardware lock safely assessed. Starting implementation execution."
4. Begin executing the steps in `implementation_plan.md` and check off items in `task.md`.
7. **Important:** The hardware lock must remain held during your execution until you or the user runs `/plan-wrap` or `/queue-release`.

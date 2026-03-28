---
description: Queue a saved plan by formally marking it as READY for execution.
---

1. Attempt to resolve the current active Plan ID via `.agents/plans/*/sessions.md` lineage or via user argument `/plan-queue [ID]`.
2. If the plan has NOT been saved to disk (i.e. no `PLAN.md` exists for it), halt and explicitly warn the user: *"This plan has not been saved. You must use `/plan-save` to persist the context before you can queue it for execution."*
3. If the plan IS saved:
   - Update the `.agents/plans/[ID]/PLAN.md` YAML frontmatter to explicitly set `status: READY`.
4. Respond to the user confirming the queue placement.

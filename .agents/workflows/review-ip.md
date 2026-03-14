---
description: Forces the application or reapplication of the implementation-plan-review.md rule to the latest implementation plan.
---

1. Locate the latest `implementation_plan.md` in the current session's artifact directory (e.g., `<appDataDir>/brain/<conversation-id>/implementation_plan.md`).
2. Read and strictly apply the requirements and checklists defined in the project's `implementation-plan-review.md` rule.
3. Specifically, ensure that:
   - The `house-style-documentation` skill has audited the plan for naming conventions and section headers.
   - The `architectural-refactoring` skill has audited the plan for SRP, OCP, DIP, and injection patterns.
   - The `embedded-memory-assessment` skill has audited the plan for Flash, RAM, Stack, and Heap impacts.
4. Update the `implementation_plan.md` file to:
   - Reflect any findings or recommendations from these reviews using GitHub alerts (IMPORTANT/WARNING).
   - Update the status of the audit checklist at the top of the file for each skill.
5. Notify the user once the review and updates are complete.

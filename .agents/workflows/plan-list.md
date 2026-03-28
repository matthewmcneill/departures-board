---
description: Survey all persisted plans on disk by parsing PLAN.md YAML frontmatter natively.
---

1. Execute `python3 .agents/skills/plan-manager/scripts/plan_list.py` silently in the background (append `--all` if requested).
2. The script will generate the file `/Users/mcneillm/Documents/Projects/departures-board/.agents/plans/plan_list_view.md`.
3. You MUST present this file to the user by calling the `notify_user` tool natively with `PathsToReview` exactly targeting `["/Users/mcneillm/Documents/Projects/departures-board/.agents/plans/plan_list_view.md"]`. Do NOT copy and paste the markdown into your chat response.

---
description: Survey all persisted plans on disk by parsing PLAN.md YAML frontmatter natively.
---

1. Execute `python3 .agents/skills/plan-manager/scripts/plan_list.py --stdout` (append `--all` if requested).
2. Capture the markdown output from the terminal.
3. Use the `write_to_file` tool to create a new artifact named `plan_list_view.md`.
   - Set `IsArtifact` to `true`.
   - Provide an appropriate `ArtifactMetadata`.
   - Ensure the `TargetFile` is an absolute path to the `artifacts/` directory for the active session.
4. This will automatically open the rendered plan list in the user's view pane.

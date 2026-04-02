---
description: Generate standard export prompts to spin up new architectural implementation sessions.
---

When the user asks you to execute the `/plan-spawn` workflow (or requests an export prompt/handoff prompt to start a new session), construct a detailed Markdown prompt for the user to copy/paste. Adhere to the following structure and rules:

1. **Structured Formatting**: Use clear top-level headings: `# Objective`, `# Context`, and `# Execution Instructions`.
2. **Context Continuity**: Do not assume the new session will know anything. Provide a deep technical summary of the goal.
   - If the task relies heavily on analysis, logs, or reports generated in your *current* session, explicitly instruct the new agent to read those specific artifacts. Provide the exact absolute paths to your current session's `.gemini/antigravity/brain/...` directory so they can bridge the context effortlessly.
   - Alternatively, mention the use of the `/plan-load` tool if appropriate.
3. **Immediate Actionable Steps**: Tell the new agent exactly what to do first. (e.g., "Perform a `grep_search` across `modules/` for `<X>`").
4. **Mandatory Planning Step**: You **MUST** include an explicit instruction requiring an implementation plan. Use wording similar to:
   > "Start in `PLANNING` mode using the `task_boundary` tool. Gather your context and then draft an `implementation_plan.md` addressing the objective. Present this plan back to me for review using the `notify_user` tool before executing any code modifications. Do this even if the task is small."

**Output Format**: Output the finalized prompt to the user within a single Markdown code block so they can easily copy it to their clipboard.

---
description: Persist the current session's artifacts, generating context_bridge.md, sessions.md, and the standardized PLAN.md metadata file.
---

1. If the user provided an `[ID]` as an argument, set `TargetPlanId` to `[ID]`.
2. If `[ID]` is NOT provided: 
   - Check if this session's ID exists inside any `.agents/plans/*/sessions.md` file to determine implicit lineage.
   - If a match is found, use that existing folder's ID as `TargetPlanId`.
   - If no match is found, set `TargetPlanId` to the current session ID, and create the `.agents/plans/[TargetPlanId]/` directory structure.
3. **Context Bridge Generation**: Meticulously distill the current working memory into `context_bridge.md`. This file MUST strictly follow this template:
    - **📍 Current State & Focus**: A summary of where the project sits overall, and a laser-focused description of exactly what was happening right before pencils down.
    - **🎯 Next Immediate Actions**: The exact, literal next steps the arriving agent needs to do to resume momentum immediately.
    - **🧠 Decisions & Designs**: Record any major architectural decisions taken or design patterns agreed upon during this session.
    - **🐛 Active Quirks, Bugs & Discoveries**: Any hidden context discovered (e.g. quirks of an API) or unresolved bugs being tracked.
    - **💻 Commands Reference**: The literal terminal commands currently relied on for building or testing so the next agent doesn't guess flags.
    - **🌿 Execution Environment**: The active Git branch state and the physical hardware testing state (e.g. is hardware attached, or are we using WASM?).
    - **⚠️ PORTABILITY RULE**: You MUST use repository-relative paths (e.g., `modules/foo/bar.cpp`) for all in-repo file references. You are **STRICTLY PROHIBITED** from using absolute paths starting with `/Users/` (or equivalents) for project files. The only exceptions are absolute paths to the session's `/brain` directory.
4. **Distillation**: Rewrite the `implementation_plan.md`, `task.md`, generated `context_bridge.md`, and `sessions.md` into the `.agents/plans/[TargetPlanId]/` directory. (Be sure to copy all current brain artifacts and embedded image resources).
5. Extract the Plan's succinct title and description.
6. Create or update the `.agents/plans/[TargetPlanId]/PLAN.md` file to structurally match the YAML frontmatter found in `.agents/skills`:
   ```markdown
   ---
   name: [Plan Title]
   description: [Short Summary]
   created: [Timestamp]
   status: SAVED
   ---
   ```
7. Report back to the user that the context has been securely persisted locally to disk, and instruct them to use `/plan-queue` when they are ready to schedule it for work.

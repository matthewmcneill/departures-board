---
description: Persist the current session's artifacts, generating context_bridge.md, sessions.md, and the standardized PLAN.md metadata file.
---

1. If the user provided an `[ID]` as an argument, set `TargetPlanId` to `[ID]`.
2. If `[ID]` is NOT provided: 
   - Check if this session's ID exists inside any `.agents/plans/*/sessions.md` file to determine implicit lineage.
   - If a match is found, use that existing folder's ID as `TargetPlanId`.
   - If no match is found, set `TargetPlanId` to the current session ID, and create the `.agents/plans/[TargetPlanId]/` directory structure.
3. **Context Distillation**: Invoke the `distillery` skill, passing it the target directory `.agents/plans/[TargetPlanId]/`. The distillery skill will extract deep granular context into `context_artifacts/` payloads and generate the `context_bridge.md` index file.
4. **Persistence**: Rewrite the `implementation_plan.md`, `task.md`, the generated `context_bridge.md`, `sessions.md`, and the entire `context_artifacts/` directory into the `.agents/plans/[TargetPlanId]/` directory. (Be sure to copy all current brain artifacts and embedded image resources).
5. Extract the Plan's succinct title and description.
6. Create or update the `.agents/plans/[TargetPlanId]/PLAN.md` file to structurally match the local metadata standards:
   ```markdown
   ---
   name: [Plan Title]
   description: [Short Summary]
   created: [Timestamp]
   status: SAVED [or WIP/READY]
   commits: []
   ---
   
   # Summary
   [Succinct high-level summary of the plan's goal]
   
   ## Technical Context
   - [Context Bridge](context_bridge.md): Distilled technical findings.
   - [Sessions](sessions.md): Complete session history.
   - [Task List](task.md): Progress checklist.
   ```
7. Report back to the user that the context has been securely persisted locally to disk, and instruct them to use `/plan-queue` when they are ready to schedule it for work.

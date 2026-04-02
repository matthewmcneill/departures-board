---
description: Shows the complete user guide and flowchart for the plan lifecycle.
---

[This workflow prints the user manual for the plan persistence lifecycle]

# Plan & Queue Lifecycle Guide

The system manages your work through an independent **Plan Lifecycle** that spans **Disk Persistence** (the rich context and artifacts) and **Queue State** (scheduling and hardware locks).

Every complex request should follow these 6 strict steps:

### 1. `/plan-save [Optional ID]`
Persists your temporary session data securely to `.agents/plans/[ID]/`. It writes the `task.md`, the `context_bridge.md`, and creates a `PLAN.md` file natively embedded with your project metadata.
- *Tip: If you run this without an ID on a new task, it auto-generates the folder structure for you. If you provide an ID, it securely merges!*

### 2. `/plan-list`
Scans the `.agents/plans/` directory for YAML frontmatter in all `PLAN.md` files. This gives you a fast, native readout of all available Plan IDs, titles, and their current tracked states without executing them.

### 3. `/plan-queue`
When your plan is thoroughly drafted and saved, use this to mark the ticket as **READY**. It updates the `.agents/queue.md` ledger and places the work into the `## Pending Queue`.

### 4. `/plan-load [Plan ID]`
When you are ready to begin work in a fresh session, this command pulls the rich context from `.agents/plans/[ID]/` into your active session's brain. It safely moves the ticket in the queue ledger down to **Work In Progress (WIP)**, but *pauses* before compiling or locking hardware.

### 5. `/plan-start`
Validates that the hardware is free, securely claims the hardware lock in `.agents/queue.md`, and formally begins the execution of your `implementation_plan.md` tasks!

### 6. `/plan-wrap`
Concludes the lifecycle. This handles the final save of the `100% DONE` state back to disk, releases your hardware lock, and permanently logs the task as completed in the `## Execution History` table.

---

> [!TIP]
> Use **`/queue-list`** to inspect exactly who currently holds the hardware lock and see what is universally pending or WIP. Use **`/plan-list`** to inspect the structural context available to load locally!

# Plan Lifecycle Orchestrator

---
name: plan-manager
description: Manage the queue, persist execution state, and safely claim hardware locks by orchestrating the native /plan-* workflows.
---

## Overview

This skill teaches you how to manage the lifecycle of a complex ticket, from surveying the queue to pulling context, taking hardware locks, and persisting artifacts. 

**CRITICAL RULE**: The system uses a strict set of encapsulated `/plan-*` markdown workflows and `.agents/plans/lock.md` to guarantee ACID-compliant state transitions. Do not invent processes; rely purely on the scripts inside `scripts/` or the workflows.

If the user asks you to interact with the queue, switch context, or finalize a session, you MUST execute the appropriate workflow by strictly following its markdown file instructions.

## The 6-Step Workflow API

When queried to perform any queue management task, map the intent to one of the following commands and immediately execute precisely what the corresponding `.md` file instructs:

### 1. `/plan-list` -> [view `.agents/workflows/plan-list.md`]
- **Use when**: The user wants to see the active lock statuses, pending tasks, or what plans are securely saved on disk.

### 2. `/plan-queue` -> [view `.agents/workflows/plan-queue.md`]
- **Use when**: You have just drafted a robust `implementation_plan.md` for a brand new task, safely persisted it via `/plan-save`, and now need to flag it as `READY` for scheduling.
- **CRITICAL**: This is a queuing action ONLY. You are explicitly forbidden from executing or starting the plan after queuing it. You must await further instructions.

### 3. `/plan-load [ID]` -> [view `.agents/workflows/plan-load.md`]
- **Use when**: The user asks you to "pick up", "load", or "do" a specific session ID from the pending queue. This safely pulls the rich context into your brain natively.

### 4. `/plan-start` -> [view `.agents/workflows/plan-start.md`]
- **Use when**: You have loaded a plan and are ready to claim the hardware lock and begin coding. (You cannot compile or push code without claiming this lock natively).

### 5. `/plan-wrap` -> [view `.agents/workflows/plan-wrap.md`]
- **Use when**: At the end of a session, when all tasks in `implementation_plan.md` are 100% finished. This triggers the massive validation sweep, Git commit generation, and final disk persistence before formally releasing the hardware lock.

## Mid-Flight Context Saving (Pencils Down)

If work is abruptly stopped or the user requests to "pause" or "switch contexts" before task completion:
- **Do not** run `/plan-wrap`. 
- **DO run `/plan-save`** -> [view `.agents/workflows/plan-save.md`]
- This preserves your exact point by running the `/plan-save` workflow and implicitly invoking the `distillery` skill. This generates a `context_bridge.md` and a token-efficient paging structure so the next agent can seamlessly resume your WIP code branch.

> [!IMPORTANT]
> If a user prompts you with a slash command (e.g. `/plan-start`), do not invent the logic! Use the `view_file` tool to explicitly read the corresponding `.agents/workflows/plan-<name>.md` file and execute its exact literal steps.

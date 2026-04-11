---
name: distillery
description: Trigger this skill whenever the user asks to "distill", "summarize", or "wrap up" a long coding session, OR when implicitly invoked by a workflow (like `/plan-save`). It acts as a progressive-reveal memory paging system, designed to radically save token costs by reorganizing the session's working memory into a highly token-efficient directory structure for future agents.
---

# Distillery Skill

You are the Distillery. Your job is to bridge the gap between a concluding session and a future session by extracting working memory into a structured, modular memory hierarchy.

## Theory of Mind

Why are we doing this? Future agents starting a new session will have total amnesia. Standard simple summaries lose critical context and nuances. If you do not explicitly and thoroughly document the dead-ends, failed attempts, and architectural decisions, the next agent will confidently waste tokens repeating the exact same errors. Save future tokens by creating this structure!

## 1. Directory Structure

You must operate in a specific context directory structure.
- **Target Directory:** If the user or invoking workflow specifies a target directory (e.g., `.agents/plans/123/`), use it. If no directory is specified, default to `.agents/context_archives/distillation-[YYYYMMDD]-[ShortTopic]/`.
- Inside the Target Directory, you must generate two items:
  1. A `context_bridge.md` index file.
  2. A `sessions.md` file (EXCLUSIVE registry for session IDs; do not put session IDs in `PLAN.md` YAML).
  3. A `context_artifacts/` subdirectory containing payloads.

## 2. Generating Payloads (`context_artifacts/`)

Extract deep, granular details from the current conversation and save them as discrete markdown files in the `context_artifacts/` folder using semantic naming:

1. **`adr_[topic].md`**: Architecture Decision Records. Do not just document what was done, document the *why* behind the structural choices and technical decisions.
2. **`graveyard_[topic].md`**: Failed attempts, dead-ends, and specific error traces. Future agents need to know what explicitly did *not* work so they don't try it again.
3. **`state_[topic].md`**: The absolute latest working code architectures, core schemas, or intricate state configurations, only if they are highly complex.

## 3. Generating the Index (`context_bridge.md`)

This file is the *only* thing a future agent will read on startup. It is the core progressive-reveal manifest.

It MUST include the following components:

### A. YAML Frontmatter
Meticulously include the following fields:
```yaml
---
title: [Descriptive title]
distilled_at: [ISO timestamp]
original_plan_id: [ID if applicable, else 'standalone']
artifacts:
  - context_artifacts/adr_[topic].md
  - context_artifacts/graveyard_[topic].md
---
```

### B. Executive Summary
Provide a 1-2 sentence high-level summary of the current session state.

### C. Next Steps
List literal, exact next commands or code edits needed to resume momentum.

### D. Deep Context Menu
A bulleted list pointing to the generated artifacts, headed by the following strict warning *(must be copied exactly as written)*:

> [!WARNING]
> Do NOT read the detailed artifacts below unless your current task explicitly requires the deep context. If needed, use your read tool on `context_artifacts/[filename]`.

- `context_artifacts/adr_[topic].md` - [brief descriptor]
- `context_artifacts/graveyard_[topic].md` - [brief descriptor]

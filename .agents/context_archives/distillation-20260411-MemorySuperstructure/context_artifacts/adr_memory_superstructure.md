# Architecture Decision Record: Modular Memory Superstructure

## Context
The project's original `project_log.md` was a monolithic file that grew too large to be token-efficient for agents. It contained technical metadata (commits, sessions) mixed with prose, making it difficult to parse and use as "Local Truth".

## Decision
Migrate the historical log into a **Modular Memory Superstructure** consisting of individual `PLAN.md` files for every logical work unit.

## Consequences
- **Token Efficiency**: Agents now only need to read the specific `PLAN.md` for a target feature rather than the entire history.
- **Improved Metadata**: Technical metadata (commits, session IDs) is now stored as structured YAML frontmatter.
- **Centralized Indexing**: `project_index.md` provides a high-level map of the project by system module (`displayManager`, `configManager`, etc.).
- **Chronological Ledgers**: [ACTIVE_PLANS.md](file://.agents/plans/ACTIVE_PLANS.md) and [ARCHIVED_PLANS.md](file://.agents/plans/ARCHIVED_PLANS.md) provide views of the project workflow over time.

## Indexing Strategy
The [migrate_logs.py](file://.agents/tmp/migrate_logs.py) script is the "Single Source of Truth" for maintaining the superstructure. It uses a hybrid approach:
1. **Log Parsing**: Extracts history from the legacy `project_log.md`.
2. **Disk Scanning**: Scans `.agents/plans/` for `PLAN.md` files that are not in the log.
3. **Heuristic Recovery**: Regenerates titles and dates for stub or "Untitled" plans based on content context.

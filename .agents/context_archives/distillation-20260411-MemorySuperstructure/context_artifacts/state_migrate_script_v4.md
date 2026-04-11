# State: Memory Migration Script (v4)

The primary maintenance tool for the memory system is `.agents/tmp/migrate_logs.py`.

## Capabilities
- **FileSystem Scan**: Recursively finds all `PLAN.md` files in `.agents/plans/`.
- **Metadata Reconciliation**: Merges log-based history with disk-based `PLAN.md` metadata.
- **Index Generation**: Rebuilds the three core index files:
    - [project_index.md](file://.agents/project_index.md) (Modular map)
    - [ACTIVE_PLANS.md](file://.agents/plans/ACTIVE_PLANS.md) (Current task list)
    - [ARCHIVED_PLANS.md](file://.agents/plans/ARCHIVED_PLANS.md) (Chronological ledger grouped by `YYYY-MMM`)
- **Orphan Auditing**: Compares the final index against the disk and reports any `PLAN.md` files that have no historical record in the log.

## Example Heuristic Recovery
If a plan has `name: "Untitled"`, the script now performs:
```python
clean_summary = re.sub(r'#.*?\n', '', summary).strip()
title = " ".join(clean_summary.split()[:10]) + "..."
```
This ensures the indices are always semantically useful.

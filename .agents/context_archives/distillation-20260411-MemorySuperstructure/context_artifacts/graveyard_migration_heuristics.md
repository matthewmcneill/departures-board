# Graveyard: Legacy Log Migration Challenges

## regex-based extraction
Initially, we attempted to use simple regex to extract metadata from the legacy log. This failed because:
- **Inconsistent Headers**: Some plans used `[hex]`, others `## [hex]`, and others purely descriptive titles.
- **Nested Lists**: Commit IDs were sometimes in inline text, sometimes in bullet points, and sometimes only in the header of the next entry.
- **Broken Sections**: A simple `split('##')` produced empty stubs when the log had trailing headers or notes.

## Index highlights limit
We initially implemented a "Highlights" view (top 5 plans per module) in `project_index.md`. This was rejected because it made the "Done" folder feel disconnected from the index. We migrated to a full-listing approach under each module.

## UUID-only links
Link text using raw UUIDs was deemed "broken" by the user because it lacked semantic value. We shifted to heuristic title recovery (taking the first sentence of the summary) for all plans that lacked a formal title.

---
id: 7205c1a3-9c68-4c0b-8ef1-d5bd6e55239f
title: "Refactor: Architectural Migration & Include Scrub (Phase 1 & 2)"
status: DONE
created_at: 2026-04-26T20:55:59Z
completed_at: 2026-04-26T21:33:00Z
commit_id: ca80e620a7508cc1a5ebeefe84755d31ced7c49c
---

# Plan Summary
Executed an intense architectural code audit and resolved broken/dirty dependencies throughout the codebase. The `otaUpdateManager`, `rssClient`, and `timeManager` components were formally relocated from `lib/` (standalone libraries) to `modules/` (context-dependent app modules). As a second phase, built an automated script (`scrub_includes.py`) that strictly followed an audit matrix to purge 42 redundant/dangling `#include` directives to eliminate graph bloat, which was subsequently validated via a full `chain+` ESP-IDF CMake LDF compilation.

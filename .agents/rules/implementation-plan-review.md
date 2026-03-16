# Implementation Plan Review Rule

Every `implementation_plan.md` created for this project MUST be reviewed and updated to adhere to the project's house style, architectural standards, and resource impact assessment.

## Context
Standardizing the quality of implementation plans ensures that all proposed changes are consistent with the project's technical goals, naming conventions, and resource constraints (especially important for ESP32).

## Requirements
Whenever you are drafting or finalizing an implementation plan:
1.  **House Style**: You MUST consult the `house-style-documentation` skill (`.agents/skills/house-style-docs/SKILL.md`) to ensure:
    - `camelCase` naming for files and libraries.
    - Standard section headers (Goal, Review Required, Proposed Changes, Verification).
2.  **Architectural Standards**: You MUST consult the `architectural-refactoring` skill (`.agents/skills/oo-expert/SKILL.md`) to ensure:
    - Adherence to SRP, OCP, and DIP.
    - Minimal global state and use of Dependency Injection.
    - Proper use of interfaces (i-prefixed).
3.  **Resource Impact**: You MUST consult the `embedded-systems` skill (`.agents/skills/embedded-systems/SKILL.md`) to ensure:
    - Flash/ROM, RAM, Stack, and Heap memory impacts are evaluated and documented.
    - ESP32-specific optimizations (const in Flash, PSRAM usage) are considered.
    - Power consumption and Duty Cycling profiles are evaluated.
    - Security posture, attack surfaces, and cryptography are evaluated.

## Enforcement
The implementation plan MUST reflect that these reviews have taken place before it is presented to the user for final approval. Use GitHub alerts (IMPORTANT/WARNING) to highlight any compromises or critical decisions found during these reviews.

Whenever an implementation plan is finalized (e.g. before `notify_user`), you MUST execute the `/review-ip` workflow to perform this audit.

## Audit
A checklist should be placed at the top of every implementation plan by each skill to say that the files have been audited by each of them.

e.g.
[x] Reviewed by xxxx - passed
[x] Reviewed by yyyy - changes recommended to ensure documentation is at standard
[x] Reviewed by zzzz - user review required to select option for resource optimisation

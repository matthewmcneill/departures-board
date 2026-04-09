---
trigger: always_on
---

# 02 - Situational Pointers

These are situational workflows. Do not load their full contents unless your current task requires them. 
If your task triggers any of the following conditions, you MUST execute `view_file` on the corresponding workflow BEFORE proceeding:

- **Hardware Interaction**: If you need to compile, flash, check logs, or handle queues for PlatformIO, you MUST load `.agents/skills/pio-manager/SKILL.md` exclusively.
- **Web Portal Development**: If you are modifying files in `test/web` or working on the UI portal, you MUST view `.agents/workflows/web-testing-strategy.md` for the strict 3-phase testing pipeline.
- **Finalizing Plans**: Before finishing an `implementation_plan.md`, you MUST execute `.agents/workflows/review-ip.md` to ensure code quality style checks pass.  If you have loaded an implementation plan without a review block near the top saying it has been reviewed you MUST execute `.agents/workflows/review-ip.md` to ensure code quality style checks pass.

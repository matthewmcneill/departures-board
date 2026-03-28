# Task: Unified Test Queue & Diagnostic UI

- [ ] Planning
    - [x] Analyze backend test endpoints
    - [x] Draft `implementation_plan.md`
    - [x] Refine plan based on user feedback
- [x] Implementation
    - [x] Create `app.enqueueTest` and `app.clearTestQueue` with 500ms delay
    - [x] Hook `app.clearTestQueue` into `app.switchTab`
    - [x] Refactor `testBoardAsync`, `testWeather`, `testFeed`, `testKeyAsync` to use queue
    - [x] Unify Display slot UI with API Keys UI (add text labels next to dots)
- [x] Verification
    - [x] Test queue behavior on tab switch
    - [x] Test sequential execution in Network tab
    - [x] Verify UI visual alignment

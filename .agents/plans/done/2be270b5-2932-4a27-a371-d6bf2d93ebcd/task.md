# Task: Audit Time Manager and NTP Service

- [ ] Find where `TimeManager::initialize()` is called [/]
- [ ] Verify how `TimeManager` is passed to widgets [/]
    - [ ] Add `TimeManager` accessor to `appContext` [ ]
    - [ ] Refactor `clockWidget` to use injected `TimeManager` [ ]
- [ ] Audit `TimeManager` and `NTPService` (if it exists) [ ]
- [ ] Confirm `appContext` integration [x]
- [ ] Report findings to user [ ]

# Task: Resolve National Rail API Test Failure

- [x] Initial Research & Optimization
    - [x] Implement `setSoapAddress` to bypass WSDL download
    - [x] Add `SOAPAction` header support
- [x] Token Bug Fix
    - [x] Identify token corruption (garbage chars `!HWD`)
    - [x] Replace `strncpy` with `strlcpy` in all data sources (NR, TfL, Bus)
- [/] SOAP Protocol Debugging
    - [/] Align namespaces and SOAPAction (Trying hybrid 2012/2021)
    - [/] Verify results via serial logs
- [ ] Final Verification
    - [ ] Confirm National Rail test passes in portal
    - [ ] Update walkthrough.md
    - [ ] Release hardware lock

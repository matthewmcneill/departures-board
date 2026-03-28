# Task: Resolve National Rail API Test Failure

- [x] Initial Research & Optimization
    - [x] Implement `setSoapAddress` to bypass WSDL download
    - [x] Add `SOAPAction` header support
- [x] Token Bug Fix
    - [x] Identify token corruption (garbage chars `!HWD`)
    - [x] Replace `strncpy` with `strlcpy` in all data sources (NR, TfL, Bus)
- [x] SOAP Protocol Debugging
    - [x] Align namespaces and SOAPAction (Fixed: 2021 namespace + ldb12.asmx)
    - [x] Verify results via serial logs
- [x] TfL Auth Optimization
    - [x] Implement lightweight auth check (Victoria Line Status)
    - [x] Update tflDataSource to support `isTestMode`
    - [x] Update WebHandlerManager to trigger optimized check
- [x] Final Verification & Wrap-up
    - [x] Confirm all tests pass in portal (NR, TfL, OWM)
    - [x] Audit code for house-style and OOP principles
    - [x] Update walkthrough.md and consolidate artifacts
    - [x] Release hardware lock

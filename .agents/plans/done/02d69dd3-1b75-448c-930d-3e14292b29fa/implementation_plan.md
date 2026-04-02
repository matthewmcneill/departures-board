# Implementation Plan - Remove bustimes.org Button

The "bustimes.org" button in the API Key selector is being removed because it does not require an API key in the current implementation (bus boards use a scraper that doesn't need a token). This will prevent user confusion.

## Proposed Changes

### Web Portal [portal](portal)

#### [MODIFY] [index.html](portal/index.html)
- Remove the `type-card` element for "bustimes.org" (approx line 1105-1115).
- Remove the `bus` entry from the `PROVIDER_LOGOS` constant (approx line 1196).

### Web Tests [test/web/tests](test/web/tests)

#### [MODIFY] [portal.spec.ts](test/web/tests/portal.spec.ts)
- Update the expected logo count in the key type picker from 4 to 3 (line 205).

### Build Scripts [scripts](scripts)

#### [MODIFY] [portalAssets.h](include/webServer/portalAssets.h)
- This file will be automatically updated by running the `portalBuilder.py` script.

---

## Verification Plan

### Automated Tests
- Run the web portal tests to ensure the UI still functions correctly and reflects the change.
```bash
cd test/web
npx playwright test
```

### Manual Verification
- Visual inspection of the "Select Key Type" dialog (once deployed or running locally) to confirm the "bustimes.org" card is gone.
- Confirm that "Rail", "TfL", and "Weather" buttons still work correctly.

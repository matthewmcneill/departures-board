import { test, expect } from '@playwright/test';

test.describe('Web Portal - Local Mocked Tests', () => {
  test.beforeEach(async ({ page }) => {
    // Log console messages for debugging
    page.on('console', msg => console.log(`BROWSER LOG: ${msg.text()}`));

    // Mock the initial config API call
    await page.route('**/api/config', async route => {
      console.log('Mocking /api/config');
      const json = {
        system: {
          hostname: "departures-test",
          timezone: "GMT0BST,M3.5.0/2,M10.5.0/3",
          brightness: 128,
          flip: false
        },
        wifi: {
          ssid: "TestWiFi",
          passExists: true
        },
        keys: [
          { id: "key_1", type: "rail", label: "My Rail Key", token: "", tokenExists: true }
        ],
        boards: [
          { name: "Waterloo", type: 0, id: "WAT", weather: true, apiKeyId: "key_1" }
        ]
      };
      await route.fulfill({ json });
    });

    // Mock the status API call
    await page.route('**/api/status', async route => {
      console.log('Mocking /api/status');
      const json = {
        ap_mode: false,
        ssid: "TestWiFi",
        ip: "192.168.1.100",
        rssi: -50,
        heap: 102400,
        uptime: 3600
      };
      await route.fulfill({ json });
    });

    await page.goto('/');
    // Give it a moment to initialize
    await page.waitForLoadState('networkidle');
  });
  test('should load the portal with mock data', async ({ page }) => {
    await expect(page.locator('h2')).toContainText('DEPARTURES');
    await expect(page.locator('input[name="hostname"]')).toHaveValue('departures-test');
  });

  test('should display active boards and trigger health checks on tab switch', async ({ page }) => {
    let testCalls = 0;
    // Mock board testing API
    await page.route('**/api/boards/test', async route => {
      testCalls++;
      await route.fulfill({ json: { status: 'ok' } });
    });

    // Switch to Displays tab
    await page.click('a[data-target="tab-displays"]');
    const items = page.locator('#boards-list .board-slot:not(.empty)');
    await expect(items).toContainText('Waterloo');
    
    // Wait for the health check to start (transition from grey)
    const dot = page.locator('#board-dot-0');
    await expect(dot).toBeVisible();
    await expect(dot).not.toHaveClass(/grey/, { timeout: 10000 });
    
    // Verify health check was triggered
    expect(testCalls).toBeGreaterThanOrEqual(1);
    await expect(dot).toHaveClass(/green|dot green/);
  });

  test('should display API keys with correct masking', async ({ page }) => {
    // Switch to Keys tab
    await page.click('a[data-target="tab-apikeys"]');
    
    // Verify slot 1 is filled and masked
    const slot = page.locator('.key-slot.filled').first();
    await expect(slot).toContainText('My Rail Key');
    await expect(slot).toContainText('rail');
    
    // Open edit modal by clicking the card
    await slot.click();
    await expect(page.locator('#modal-apikey')).toBeVisible();
    await expect(page.locator('input[name="token"]')).toHaveValue('');
    await expect(page.locator('input[name="token"]')).toHaveAttribute('placeholder', '••••••••');
  });

  test('should handle adding a new API key', async ({ page }) => {
    await page.click('a[data-target="tab-apikeys"]');
    
    // Mock the save API call
    await page.route('**/api/keys', async route => {
      expect(route.request().method()).toBe('POST');
      await route.fulfill({ status: 200 });
    });

    // Click an empty slot
    await page.locator('.key-slot').filter({ hasText: 'Empty Slot' }).first().click();
    await expect(page.locator('#modal-key-selection')).toBeVisible();
    
    // Select a type (National Rail / TfL / Weather)
    await page.click('.type-card h5:has-text("TfL")');
    await expect(page.locator('#modal-apikey')).toBeVisible();
    
    // Fill the form
    await page.fill('input[name="label"]', 'New Test Key');
    await page.fill('input[name="token"]', '0123456789abcdef0123456789abcdef');
    
    // Save
    await page.click('#key-form button[type="submit"]');
    await expect(page.locator('#modal-apikey')).not.toBeVisible();
  });

  test('should validate an API key and show feedback', async ({ page }) => {
    await page.click('a[data-target="tab-apikeys"]');
    // Wait for the key slot to render and click it
    const slot = page.locator('.key-slot.filled').first();
    await slot.click();
    
    // Mock test success
    await page.route('**/api/keys/test', async route => {
      await route.fulfill({ json: { status: 'ok' } });
    });

    await page.click('#key-test-btn');
    await expect(page.locator('#key-test-feedback')).toContainText('validated successfully');
    await expect(page.locator('#key-test-feedback')).toHaveClass(/success/);

    // Mock test failure
    await page.route('**/api/keys/test', async route => {
      await route.fulfill({ json: { status: 'error', msg: 'Invalid token' } });
    });

    await page.click('#key-test-btn');
    await expect(page.locator('#key-test-feedback')).toContainText('Validation failed: Invalid token');
    await expect(page.locator('#key-test-feedback')).toHaveClass(/error/);
  });

  test('should handle WiFi scanning and connection test', async ({ page }) => {
    // Mock WiFi scan
    await page.route('**/api/wifi/scan', async route => {
      await route.fulfill({ json: [
        { ssid: 'MockNet1', rssi: -60, secure: true },
        { ssid: 'MockNet2', rssi: -70, secure: false }
      ]});
    });

    // Click Scan
    await page.click('#scan-btn');
    const select = page.locator('#wifi-ssid-select');
    
    // Wait for the scan to populate the dropdown
    await expect(select.locator('option:has-text("MockNet1")')).toBeAttached();
    await expect(select.locator('option')).toHaveCount(5); // Default + Configured + 2 Mock + Manual

    // Mock WiFi test success
    await page.route('**/api/wifi/test', async route => {
      await route.fulfill({ json: { status: 'ok', ip: '192.168.1.50' } });
    });

    await select.selectOption('MockNet1');
    await page.click('#test-btn');
    await expect(page.locator('#wifi-test-log')).toContainText('SUCCESS! IP assigned: 192.168.1.50');
  });

  test('should display system diagnostics correctly', async ({ page }) => {
    // Expand diagnostics drawer
    await page.click('.drawer-trigger');
    await expect(page.locator('#diag-drawer')).toHaveClass(/active/);
    
    // Verify mock status data
    const table = page.locator('#wifi-diag-table');
    await expect(table).toContainText('192.168.1.100');
    await expect(table).toContainText('-50 dBm');
    await expect(table).toContainText('100 KB');
  });

  test('should show configured WiFi even if scan results are empty', async ({ page }) => {
    // Note: The beforeEach already mocks /api/config with { wifi: { ssid: "TestWiFi" } }
    // No need to re-mock unless we want a different SSID
    
    // Mock WiFi scan to be empty
    await page.route('**/api/wifi/scan', async route => {
      await route.fulfill({ json: [] });
    });

    // Refresh config to trigger render (or just wait for initial render)
    // Actually, at this point, the page has already loaded with "TestWiFi"
    const select = page.locator('#wifi-ssid-select');
    await expect(select).toHaveValue('TestWiFi');
    await expect(select.locator('option:has-text("TestWiFi")')).toBeAttached();

    // Click Scan and verify it still shows TestWiFi
    await page.click('#scan-btn');
    // Wait for the scan spinner to disappear (implicit in next check)
    await expect(select.locator('option:has-text("TestWiFi")')).toBeAttached();
    await expect(select).toHaveValue('TestWiFi');
    
    // Verify it's labeled with lightning bolt or "Configured"
    const text = await select.locator('option:checked').textContent();
    expect(text).toContain('TestWiFi');
    expect(text).toContain('Configured');
  });

  test('should display provider logos in key slots and type picker', async ({ page }) => {
    // Check key slot logo
    await page.click('a[data-target="tab-apikeys"]');
    const slotLogo = page.locator('.key-slot.filled .provider-logo svg');
    await expect(slotLogo).toBeVisible();

    // Check type picker logos
    await page.locator('.key-slot').filter({ hasText: 'Empty Slot' }).first().click();
    await expect(page.locator('#modal-key-selection')).toBeVisible();
    const pickerLogos = page.locator('#modal-key-selection .icon svg');
    await expect(pickerLogos).toHaveCount(3); // Rail, TfL, OWM
  });

  test('should implement secure placeholder pattern for WiFi password', async ({ page }) => {
    const passInput = page.locator('#wifi-pass');
    await expect(passInput).toHaveValue('');
    await expect(passInput).toHaveAttribute('placeholder', '••••••••');
    await expect(passInput).toHaveClass(/existing-password-mask/);
    
    // Toggle button should be disabled when empty
    const eyeBtn = page.locator('label:has-text("WiFi Password") .eye-btn');
    await expect(eyeBtn).toBeDisabled();
    
    // Typing should enable eye button
    await passInput.fill('newpassword');
    await expect(eyeBtn).toBeEnabled();
  });

  test('should execute API key tests sequentially on tab switch', async ({ page }) => {
    // Mock the status to have multiple keys
    await page.route('**/api/config', async route => {
      const json = {
        keys: [
          { id: "key_1", type: "rail", label: "Rail Key", tokenExists: true },
          { id: "key_2", type: "tfl", label: "TfL Key", tokenExists: true }
        ]
      };
      await route.fulfill({ json });
    });

    // Reload to pick up the mocked keys
    await page.goto('http://localhost:3000/');

    let testCalls = 0;
    const callTimes: number[] = [];

    // Mock the test API with a slight delay
    await page.route('**/api/keys/test', async route => {
      testCalls++;
      callTimes.push(Date.now());
      await new Promise(resolve => setTimeout(resolve, 100));
      await route.fulfill({ json: { status: 'ok' } });
    });

    // Prepare to wait for console message via Promise
    const finishedSignal = new Promise(resolve => {
      page.on('console', (msg: any) => {
        if (msg.text() === 'Sequential tests finished.') resolve(true);
      });
    });

    // Switch to Keys tab
    await page.click('a[data-target="tab-apikeys"]');
    
    // Wait for tests to signal completion
    await finishedSignal;

    // Wait for BOTH to be green
    await page.waitForFunction(() => document.querySelectorAll('.key-status-dot.green').length === 2, { timeout: 5000 });

    expect(testCalls).toBe(2);
    // Verify at least 1s gap between calls (sequential logic uses 1000ms delay)
    if (callTimes.length >= 2) {
      const diff = callTimes[1] - callTimes[0];
      expect(diff).toBeGreaterThanOrEqual(1000);
    }
  });
});

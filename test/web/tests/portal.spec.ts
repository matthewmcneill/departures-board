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
          ssid: "TestWiFi"
        },
        keys: [
          { id: "key_1", type: "rail", label: "My Rail Key", token: "●●●●●●●●" }
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

  test('should display active boards', async ({ page }) => {
    // Switch to Boards tab
    await page.click('a[data-target="tab-boards"]');
    await expect(page.locator('#boards-list')).toContainText('Waterloo');
  });

  test('should display API keys with correct masking', async ({ page }) => {
    // Switch to Keys tab
    await page.click('a[data-target="tab-apikeys"]');
    
    // Verify slot 1 is filled and masked
    const slot = page.locator('.key-slot.filled').first();
    await expect(slot).toContainText('My Rail Key');
    await expect(slot).toContainText('rail');
    
    // Open edit modal
    await slot.getByRole('button', { name: 'Edit' }).click();
    await expect(page.locator('#modal-apikey')).toBeVisible();
    await expect(page.locator('input[name="token"]')).toHaveValue('●●●●●●●●');
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
    await page.fill('input[name="token"]', 'secret_token_123');
    
    // Save
    await page.click('#key-form button[type="submit"]');
    await expect(page.locator('#modal-apikey')).not.toBeVisible();
  });

  test('should validate an API key and show feedback', async ({ page }) => {
    await page.click('a[data-target="tab-apikeys"]');
    // Wait for the key slot to render and find the Edit button within it
    const slot = page.locator('.key-slot.filled').first();
    await slot.getByRole('button', { name: 'Edit' }).click();
    
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
    await expect(select.locator('option')).toHaveCount(4); // Default + 2 Mock + Manual

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
});

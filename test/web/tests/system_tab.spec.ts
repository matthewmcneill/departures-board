import { test, expect } from '@playwright/test';

test.describe('System Tab Enhancements', () => {
  const mockConfig = {
    system: {
      hostname: "TestBoard",
      timezone: "Europe/London",
      brightness: 150,
      flip: true,
      dateEnabled: true,
      noScrolling: true,
      fastRefresh: true,
      firmwareUpdatesEnabled: true,
      configVersion: "3.1"
    },
    feeds: { rss: "", weatherKeyId: "" },
    keys: [],
    boards: []
  };

  const mockStatus = {
    heap: 150000,
    total_heap: 250000,
    temp: 45.5,
    storage_total: 4000000,
    storage_used: 1000000,
    uptime: 12345,
    connected: true,
    ssid: "TestWiFi",
    ip: "192.168.1.10"
  };

  test.beforeEach(async ({ page }) => {
    await page.route('**/api/config', async route => {
      await route.fulfill({ json: mockConfig });
    });
    await page.route('**/api/status', async route => {
      await route.fulfill({ json: mockStatus });
    });
    await page.goto('/');
    // Switch to system tab
    await page.click('[data-target="tab-system"]');
  });

  test('should render consolidated global settings with correct values', async ({ page }) => {
    await expect(page.locator('input[name="timezone"]')).toHaveValue('Europe/London');
    await expect(page.locator('input[name="dateEnabled"]')).toBeChecked();
    await expect(page.locator('input[name="noScrolling"]')).toBeChecked();
    // Hostname moved to wifi tab, checking it there
    await page.click('[data-target="tab-wifi"]');
    await expect(page.locator('input[name="hostname"]')).toHaveValue('TestBoard');
    await page.click('[data-target="tab-system"]');
    await expect(page.locator('input[name="fastRefresh"]')).toBeChecked();
    await expect(page.locator('input[name="flip"]')).toBeChecked();
    await expect(page.locator('#system-form input[name="brightness"]')).toHaveValue('150');
  });

  test('should render dynamic hardware diagnostic bars', async ({ page }) => {
    const diagContainer = page.locator('#hardware-diagnostics');
    await expect(diagContainer).toContainText('Memory (Heap)');
    await expect(diagContainer).toContainText('Storage (LittleFS)');
    await expect(diagContainer).toContainText('Chip Temperature');
    await expect(diagContainer).toContainText('System Uptime');

    // Verify values
    await expect(diagContainer).toContainText('45.5°C');
    // 250000 - 150000 = 100000 bytes used. 100000 / 1024 = 97.6 KB
    await expect(diagContainer).toContainText('98KB / 244KB'); 
    await expect(diagContainer).toContainText('3h 25m'); // 12345s
  });

  test('should show firmware version and auto-update status', async ({ page }) => {
    await expect(page.locator('#fw-version-text')).toContainText('Version: B3.1');
    await expect(page.locator('#auto-update')).toBeChecked();
  });

  test('should implement 2-step factory reset confirmation', async ({ page }) => {
    const resetBtn = page.locator('#btn-factory-reset');
    await expect(resetBtn).toHaveText('Factory Reset');
    
    await resetBtn.click();
    await expect(resetBtn).toHaveText('Are you sure? Tap again.');
    
    // It should reset after 3 seconds, but we test the immediate second click
    await page.route('**/api/wifi/reset', async route => {
      await route.fulfill({ json: { status: "ok" } });
    });
    
    await resetBtn.click();
    const modalBody = page.locator('#modal-generic-body');
    await expect(modalBody).toContainText('Factory reset initiated');
    await page.locator('#modal-generic-buttons button').click();
  });
});

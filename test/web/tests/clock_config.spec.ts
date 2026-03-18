import { test, expect } from '@playwright/test';

test.describe('Screensaver Clock Configuration', () => {
  test.beforeEach(async ({ page }) => {
    // Mock the initial config API call
    await page.route('**/api/config', async route => {
      const json = {
        system: {
          hostname: "departures-test",
          timezone: "GMT0BST,M3.5.0/2,M10.5.0/3",
          brightness: 128,
          flip: false
        },
        wifi: { ssid: "TestWiFi", passExists: true },
        keys: [],
        boards: [
          { name: "My Clock", type: 3, id: "CLOCK", brightness: 50 }
        ]
      };
      await route.fulfill({ json });
    });

    // Mock the status API call
    await page.route('**/api/status', async route => {
      const json = {
        ap_mode: false,
        ssid: "TestWiFi",
        ip: "192.168.1.100",
        rssi: -50,
        heap: 102400,
        uptime: 3600,
        local_time: "12:34:56",
        timezone: "GMT0BST,M3.5.0/2,M10.5.0/3"
      };
      await route.fulfill({ json });
    });

    await page.goto('/');
    await page.waitForLoadState('networkidle');
  });

  test('should display clock-specific fields and hide redundant ones', async ({ page }) => {
    // Switch to Displays tab
    await page.click('a[data-target="tab-displays"]');
    
    // Click the Clock board to edit
    await page.locator('.board-slot:has-text("My Clock")').click();
    await expect(page.locator('#modal-board')).toBeVisible();

    // Verify "Filter" section is hidden
    await expect(page.locator('#board-filters-section')).not.toBeVisible();

    // Verify "Brightness Override" slider is visible and has correct value
    await expect(page.locator('#label-board-brightness')).toBeVisible();
    await expect(page.locator('#modal-board input[name="brightness"]')).toHaveValue('50');

    // Open Diagnostics
    await page.click('#board-diag-drawer .drawer-trigger');
    await expect(page.locator('#board-diag-drawer')).toHaveClass(/active/);

    // Verify Time and Timezone are shown
    await expect(page.locator('#diag-row-time')).toBeVisible();
    await expect(page.locator('#diag-row-tz')).toBeVisible();
    await expect(page.locator('#diag-time')).toContainText('12:34:56');

    // Verify Latitude and Longitude are hidden
    await expect(page.locator('#diag-row-lat')).not.toBeVisible();
    await expect(page.locator('#diag-row-lon')).not.toBeVisible();
  });

  test('should styled range inputs with orange accent color', async ({ page }) => {
    const accentColor = await page.evaluate(() => {
      const input = document.querySelector('input[type="range"]');
      return input ? window.getComputedStyle(input).accentColor : 'auto';
    });
    // This depends on the CSS variable --primary being applied
    // Since we used accent-color: var(--primary), we just check it's set
    expect(accentColor).not.toBe('auto');
  });

  test('should send brightness in save payload', async ({ page }) => {
    await page.click('a[data-target="tab-displays"]');
    await page.locator('.board-slot:has-text("My Clock")').click();

    // Intercept save call
    let savePayload: any = null;
    await page.route('**/api/saveall', async route => {
      savePayload = route.request().postDataJSON();
      await route.fulfill({ json: { status: 'ok' } });
    });

    // Change brightness
    await page.fill('#modal-board input[name="brightness"]', '100');
    
    // Save
    await page.click('#modal-board button[type="submit"]');
    
    // Global Save
    await page.click('#save-btn');

    expect(savePayload).not.toBeNull();
    const clockBoard = savePayload.boards.find((b: any) => b.type === 3);
    expect(clockBoard.brightness).toBe(100);
  });
});

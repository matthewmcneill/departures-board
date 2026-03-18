import { test, expect } from '@playwright/test';

test.describe('Web Portal - Help Text Checks', () => {
  test.beforeEach(async ({ page }) => {
    // Mock the initial config API call so the page boots
    await page.route('**/api/config', async route => {
      const json = {
        system: { hostname: "departures", timezone: "GMT0BST", brightness: 128, flip: false },
        wifi: { ssid: "TestWiFi", passExists: true },
        keys: [],
        boards: []
      };
      await route.fulfill({ json });
    });

    await page.route('**/api/status', async route => {
      const json = { ap_mode: false, ssid: "TestWiFi", ip: "192.168.1.100", rssi: -50, heap: 102400, uptime: 3600 };
      await route.fulfill({ json });
    });

    await page.goto('/');
    await page.waitForLoadState('networkidle');
  });

  test('should display instruction boxes across tabs', async ({ page }) => {
    // WiFi tab is active by default
    await expect(page.locator('#tab-wifi .instruction-box').first()).toContainText('Hostname');

    // Keys tab
    await page.click('a[data-target="tab-apikeys"]');
    await expect(page.locator('#tab-apikeys .instruction-box').first()).toContainText('register for free API keys');

    // Feeds tab
    await page.click('a[data-target="tab-feeds"]');
    await expect(page.locator('#tab-feeds .instruction-box').first()).toContainText('RSS Feed URL');
    await expect(page.locator('#tab-feeds .instruction-box').nth(1)).toContainText('Configure the weather information');
    
    // System tab
    await page.click('a[data-target="tab-system"]');
    await expect(page.locator('#tab-system .instruction-box', { hasText: 'POSIX TZ string' })).toBeVisible();
    await expect(page.locator('#tab-system .instruction-box', { hasText: 'burn-in' })).toBeVisible();
    await expect(page.locator('#tab-system .instruction-box', { hasText: 'Fast Refresh' })).toBeVisible();
    await expect(page.locator('#tab-system .instruction-box', { hasText: 'GitHub Repository' })).toBeVisible();
  });

  test('should display grey footer on every tab', async ({ page }) => {
    const tabs = ['tab-wifi', 'tab-apikeys', 'tab-feeds', 'tab-displays', 'tab-schedule', 'tab-system'];
    for (const tab of tabs) {
      await page.click(`a[data-target="${tab}"]`);
      const footer = page.locator(`#${tab} > div`).filter({ hasText: 'Gadec Software' }).last();
      await expect(footer).toBeVisible();
      await expect(footer).toHaveCSS('opacity', '0.7');
    }
  });

  test('modals should use instruction-box style for help texts', async ({ page }) => {
    // Check Rail key help
    const railHelp = page.locator('#key-help-rail');
    await expect(railHelp).toHaveClass(/instruction-box/);
    await expect(railHelp).toHaveClass(/key-help-section/);

    // Check TfL key help
    const tflHelp = page.locator('#key-help-tfl');
    await expect(tflHelp).toHaveClass(/instruction-box/);
    await expect(tflHelp).toHaveClass(/key-help-section/);

    // Check OWM key help
    const owmHelp = page.locator('#key-help-owm');
    await expect(owmHelp).toHaveClass(/instruction-box/);
    await expect(owmHelp).toHaveClass(/key-help-section/);
  });
});

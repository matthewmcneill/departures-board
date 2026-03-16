import { test, expect } from '@playwright/test';

test.describe('WiFi Status Dot Thresholds', () => {
  const mockStatus = (ap_mode, connected, rssi) => ({
    ap_mode,
    connected,
    rssi,
    ssid: "TestWiFi",
    ip: "192.168.1.100",
    heap: 102400,
    uptime: 3600,
    gateway: "192.168.1.1",
    subnet: "255.255.255.0",
    dns1: "8.8.8.8",
    dns2: "8.8.4.4",
    mac: "AA:BB:CC:DD:EE:FF",
    bssid: "00:11:22:33:44:55",
    channel: 6,
    storage_total: 1024 * 1024,
    storage_used: 512 * 1024
  });

  test.beforeEach(async ({ page }) => {
    // Mock config
    await page.route('**/api/config', async route => {
      await route.fulfill({ json: {
        system: { hostname: "departures", timezone: "GMT", brightness: 128, flip: false },
        wifi: { ssid: "TestWiFi", passExists: true },
        keys: [],
        boards: []
      }});
    });
  });

  test('should show GREEN when connected and RSSI >= -80', async ({ page }) => {
    await page.route('**/api/status', async route => {
      await route.fulfill({ json: mockStatus(false, true, -70) });
    });
    await page.goto('/');
    const dot = page.locator('#wifi-dot');
    await expect(dot).toHaveClass(/green/);
    await expect(page.locator('#wifi-status-text')).toContainText('Connected: TestWiFi');
  });

  test('should show YELLOW when connected and RSSI < -80', async ({ page }) => {
    await page.route('**/api/status', async route => {
      await route.fulfill({ json: mockStatus(false, true, -85) });
    });
    await page.goto('/');
    const dot = page.locator('#wifi-dot');
    await expect(dot).toHaveClass(/yellow/);
    await expect(page.locator('#wifi-status-text')).toContainText('Connected: TestWiFi');
  });

  test('should show YELLOW when in Access Point mode', async ({ page }) => {
    await page.route('**/api/status', async route => {
      await route.fulfill({ json: mockStatus(true, false, -50) });
    });
    await page.goto('/');
    const dot = page.locator('#wifi-dot');
    await expect(dot).toHaveClass(/yellow/);
    await expect(page.locator('#wifi-status-text')).toContainText('Access Point Mode');
  });

  test('should show RED when disconnected', async ({ page }) => {
    await page.route('**/api/status', async route => {
      await route.fulfill({ json: mockStatus(false, false, 0) });
    });
    await page.goto('/');
    const dot = page.locator('#wifi-dot');
    await expect(dot).toHaveClass(/red/);
    await expect(page.locator('#wifi-status-text')).toContainText('Disconnected');
  });
});

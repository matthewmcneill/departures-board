import { test, expect } from '@playwright/test';

test.describe('Web Portal - Feeds Tab Tests', () => {
  test.beforeEach(async ({ page }) => {
    // Log console messages for debugging
    page.on('console', msg => console.log(`BROWSER LOG: ${msg.text()}`));

    // Mock /rss.json
    await page.route('**/rss.json', async route => {
      const json = [
        { "name": "BBC News", "url": "http://feeds.bbci.co.uk/news/rss.xml" },
        { "name": "Sky News", "url": "http://feeds.skynews.com/feeds/rss/home.xml" }
      ];
      await route.fulfill({ json });
    });

    // Mock the initial config API call
    await page.route('**/api/config', async route => {
      const json = {
        system: { hostname: "departures-test", timezone: "Europe/London", brightness: 128, flip: false },
        keys: [
          { id: "key_owm", type: "owm", label: "Weather Key", tokenExists: true }
        ],
        feeds: {
          rss: "http://feeds.bbci.co.uk/news/rss.xml",
          weatherKeyId: "key_owm"
        },
        boards: [
          { name: "Waterloo", type: 0, id: "WAT", lat: 51.50, lon: -0.11, weather: true, apiKeyId: "key_owm" }
        ]
      };
      await route.fulfill({ json });
    });

    // Mock status
    await page.route('**/api/status', async route => {
      const json = { ap_mode: false, ssid: "TestWiFi", ip: "192.168.1.100", rssi: -50, heap: 100000, uptime: 3600 };
      await route.fulfill({ json });
    });

    await page.goto('/');
    await page.waitForLoadState('networkidle');
  });

  test('should navigate to FEEDS tab and show configured data', async ({ page }) => {
    // Click FEEDS tab
    await page.click('a[data-target="tab-feeds"]');
    
    // Verify tab is active
    await expect(page.locator('#tab-feeds')).toBeVisible();
    
    // Verify RSS dropdown has selection
    const rssSelect = page.locator('#feed-select');
    await expect(rssSelect).toHaveValue('http://feeds.bbci.co.uk/news/rss.xml');
    
    // Verify Weather key is selected
    const weatherSelect = page.locator('#weather-key-select');
    await expect(weatherSelect).toHaveValue('key_owm');
  });

  test('should handle custom RSS URL input', async ({ page }) => {
    await page.click('a[data-target="tab-feeds"]');
    
    const rssSelect = page.locator('#feed-select');
    const customGroup = page.locator('#custom-feed-group');
    const customInput = page.locator('#custom-feed-url');
    
    // Select Custom
    await rssSelect.selectOption('__custom__');
    await expect(customGroup).toBeVisible();
    
    // Fill custom URL
    await customInput.fill('https://example.com/rss');
    
    // Verify test button is enabled
    const testBtn = page.locator('#feed-test-btn');
    await expect(testBtn).toBeEnabled();
  });

  test('should display RSS diagnostic results', async ({ page }) => {
    await page.click('a[data-target="tab-feeds"]');
    
    // Mock RSS test API
    await page.route('**/api/feeds/test?url=*', async route => {
      const json = { status: 'ok', count: 10, title: 'BBC News - Home' };
      await route.fulfill({ json });
    });

    // Click Test
    await page.click('#feed-test-btn');
    
    // Verify diagnostic drawer opens or shows data
    const drawer = page.locator('#feed-diag-drawer');
    await expect(drawer).toHaveClass(/active/);
    
    const diagTable = page.locator('#feed-diag-table');
    await expect(diagTable).toContainText('10');
    await expect(diagTable).toContainText('BBC News - Home');
    
    // Check status dot
    const dot = page.locator('#feed-dot');
    await expect(dot).toHaveClass(/green/);
  });

  test('should display Weather diagnostic results', async ({ page }) => {
    await page.click('a[data-target="tab-feeds"]');
    
    // Mock Weather test API
    await page.route('**/api/weather/test?keyId=*', async route => {
      const json = { status: 'ok', temp: 15, condition: 'Partly Cloudy', name: 'Test (London)' };
      await route.fulfill({ json });
    });

    // Changing key triggers test
    const weatherSelect = page.locator('#weather-key-select');
    await weatherSelect.selectOption('key_owm');
    
    // Verify diagnostic drawer opens/shows data
    const drawer = page.locator('#weather-diag-drawer');
    await expect(drawer).toHaveClass(/active/);
    
    const diagTable = page.locator('#weather-diag-table');
    await expect(diagTable).toContainText('15°C');
    await expect(diagTable).toContainText('Partly Cloudy');
    
    // Check status dot
    const dot = page.locator('#weather-dot');
    await expect(dot).toHaveClass(/green/);
  });

  test('should show error state for failed RSS test', async ({ page }) => {
    await page.click('a[data-target="tab-feeds"]');
    
    // Mock fail
    await page.route('**/api/feeds/test?url=*', async route => {
      const json = { status: 'fail', msg: 'DNS Resolution Error' };
      await route.fulfill({ json });
    });

    await page.click('#feed-test-btn');
    
    const dot = page.locator('#feed-dot');
    await expect(dot).toHaveClass(/red/);
    
    const diagTable = page.locator('#feed-diag-table');
    await expect(diagTable).toContainText('DNS Resolution Error');
  });

  test('should show grey UNCONFIGURED state when no weather key is selected', async ({ page }) => {
    await page.click('a[data-target="tab-feeds"]');
    
    // Select No key
    const weatherSelect = page.locator('#weather-key-select');
    await weatherSelect.selectOption(''); // Value for "No key selected" is empty string usually
    
    // Check status dot
    const dot = page.locator('#weather-dot');
    await expect(dot).toHaveClass(/grey/);
    await expect(page.locator('#weather-status-text')).toContainText('Unconfigured');
  });
});

import { test, expect } from '@playwright/test';

test.describe('Web Portal - Enhanced Boards Management', () => {
  test.beforeEach(async ({ page }) => {
    // Mock the initial config API call
    await page.route('**/api/config', async route => {
      const json = {
        system: { hostname: "departures-test", defaultBoardIndex: 0 },
        wifi: { ssid: "TestWiFi", passExists: true },
        keys: [
          { id: "key_rail", type: "rail", label: "Rail Key", tokenExists: true }
        ],
        boards: [
          { name: "Waterloo", type: 0, id: "WAT", weather: true, apiKeyId: "key_rail" },
          { name: "Victoria", type: 0, id: "VIC", weather: true, apiKeyId: "key_rail" }
        ]
      };
      await route.fulfill({ json });
    });

    // Mock status
    await page.route('**/api/status', async route => {
      await route.fulfill({ json: { connected: true, ssid: "TestWiFi" } });
    });

    // Mock station picker
    await page.route('**/stationpicker?q=Lon', async route => {
      await route.fulfill({ json: {
        payload: {
          stations: [
            { name: "London Waterloo", crsCode: "WAT", latitude: 51.503, longitude: -0.113 },
            { name: "London Victoria", crsCode: "VIC", latitude: 51.493, longitude: -0.144 }
          ]
        }
      }});
    });

    // Mock board testing
    await page.route('**/api/boards/test', async route => {
      await route.fulfill({ json: { status: 'ok' } });
    });

    await page.goto('/');
    await page.waitForLoadState('networkidle');
    await page.click('a[data-target="tab-displays"]');
  });

  test('should display boards with reorder buttons and primary slot', async ({ page }) => {
    const items = page.locator('.board-slot:not(.empty)');
    await expect(items).toHaveCount(2);

    // First board should be the startup board
    await expect(items.first()).toHaveClass(/primary-slot/);
    await expect(items.first().locator('.board-slot-title')).toContainText('Waterloo');

    // Second board should not be primary-slot
    await expect(items.nth(1)).not.toHaveClass(/primary-slot/);

    // Reorder buttons
    await expect(items.first().locator('.board-reorder-btn[title="Move Up"]')).toBeDisabled();
    await expect(items.first().locator('.board-reorder-btn[title="Move Down"]')).toBeEnabled();
    await expect(items.nth(1).locator('.board-reorder-btn[title="Move Up"]')).toBeEnabled();
    await expect(items.nth(1).locator('.board-reorder-btn[title="Move Down"]')).toBeDisabled();
  });



  test('should allow reordering boards', async ({ page }) => {
    const items = page.locator('.board-slot:not(.empty)');
    await expect(items.first()).toContainText('Waterloo');
    
    // Move second board UP
    await items.nth(1).locator('.board-reorder-btn[title="Move Up"]').click();
    
    await expect(items.first()).toContainText('Victoria');
    await expect(items.nth(1)).toContainText('Waterloo');
  });

  test('should handle station search and auto-population', async ({ page }) => {
    await page.click('button:has-text("+ Add")');
    await expect(page.locator('#modal-board-selection')).toBeVisible();
    
    // Select Rail
    await page.click('.type-card:has-text("Rail")');
    await expect(page.locator('#modal-board')).toBeVisible();
    
    // Search for "Lon"
    await page.fill('#station-search-input', 'Lon');
    const suggestion = page.locator('.suggestion-item:has-text("London Waterloo")');
    await expect(suggestion).toBeVisible();
    
    // Select suggestion
    await suggestion.click();
    
    // Check auto-populated fields
    const form = page.locator('#board-form');
    await expect(form.locator('input[name="name"]')).toHaveValue('London Waterloo');
    
    // Technical IDs and coordinates are in the diagnostic drawer
    const diagDrawer = page.locator('#board-diag-drawer');
    await expect(diagDrawer).not.toHaveClass(/active/);
    
    await diagDrawer.locator('.drawer-trigger').click();
    await expect(diagDrawer).toHaveClass(/active/);

    await expect(page.locator('#diag-id')).toHaveText('WAT');
    await expect(page.locator('#diag-lat')).toHaveText('51.503');
    await expect(page.locator('#diag-lon')).toHaveText('-0.113');
  });

  test('should toggle fields based on board type', async ({ page }) => {
    await page.click('button:has-text("+ Add")');
    await page.click('.type-card:has-text("Clock")');
    
    // Clock should hide most sections
    await expect(page.locator('#board-key-section')).not.toBeVisible();
    await expect(page.locator('#station-search-container')).not.toBeVisible();
    
    // Switch to Rail (via Cancel and re-adding)
    await page.click('#modal-board button:has-text("Cancel")');
    await page.click('button:has-text("+ Add")');
    await page.click('.type-card:has-text("Rail")');
    
    await expect(page.locator('#board-key-section')).toBeVisible();
    await expect(page.locator('#station-search-container')).toBeVisible();
  });

  test('should allow testing a board from the editor', async ({ page }) => {
    await page.click('button:has-text("+ Add")');
    await page.click('.type-card:has-text("Rail")');
    await page.fill('#station-search-input', 'Lon');
    await page.click('.suggestion-item:has-text("London Waterloo")');
    
    const testBtn = page.locator('#btn-test-board');
    await testBtn.click();
    
    await expect(testBtn).toContainText('Test');
    await expect(page.locator('#board-diag-dot')).toHaveClass(/green/);
    await expect(page.locator('#board-diag-result')).toContainText('LIVE DATA PULL');
  });
});

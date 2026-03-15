import { test, expect } from '@playwright/test';

// Use BASE_URL from environment or default to local index.html if not provided
// Note: BASE_URL should include the full path to the portal on the ESP32, e.g., http://192.168.1.100/portal
const BASE_URL = process.env.BASE_URL;

test.describe('Web Portal - Live Hardware Test', () => {
  test.skip(!BASE_URL, 'Skipping live tests because BASE_URL is not set');

  test('should load the portal from the ESP32', async ({ page }) => {
    await page.goto(BASE_URL!);
    await expect(page.locator('h2')).toContainText('DEPARTURES');
    
    // Check if we got a real status response
    const statusText = await page.locator('#wifi-status-text').innerText();
    expect(statusText).not.toBe('Detecting...');
  });

  test('should display API key registry with masked tokens', async ({ page }) => {
    await page.goto(BASE_URL!);
    
    // Switch to Keys tab
    await page.click('.tab-link[data-target="tab-apikeys"]');
    
    // Verify that the registry header is present
    await expect(page.locator('#tab-apikeys header')).toContainText('API Key Registry');
    
    // Verify that at least one slot is visible and correctly formatted
    const slot = page.locator('.key-slot').first();
    await expect(slot).toBeVisible();
    
    // Check for masking in existing keys (if any)
    const filledSlot = page.locator('.key-slot.filled').first();
    const count = await filledSlot.count();
    if (count > 0) {
      await filledSlot.getByRole('button', { name: 'Edit' }).click();
      await expect(page.locator('#modal-apikey input[name="token"]')).toHaveValue('●●●●●●●●');
      await page.click('#modal-apikey button:has-text("Cancel")');
    }
  });

  test('should handle network diagnostics drawer on live device', async ({ page }) => {
    await page.goto(BASE_URL!);
    
    // The diagnostics drawer might be inside the WiFi tab
    await page.click('.tab-link[data-target="tab-wifi"]');
    
    // Toggle diagnostics drawer
    await page.click('.drawer-trigger');
    await expect(page.locator('#diag-drawer')).toHaveClass(/active/);
    
    // Verify that we are getting actual data (not just the mock placeholders)
    const table = page.locator('#wifi-diag-table');
    await expect(table).toContainText('192.168.86.152'); // The known live IP
    await expect(table).toContainText('MAC Address');
  });
});

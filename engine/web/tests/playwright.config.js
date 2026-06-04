// @ts-check
import { defineConfig, devices } from '@playwright/test';

/**
 * Playwright config for the FTD web dashboard smoke suite.
 *
 * The tests boot a real Chromium against index.html served over HTTP by
 * `python -m http.server 8081 -d ..`. The webServer block below starts
 * the server before tests and tears it down after.
 *
 * Why port 8081: port 8080 is commonly in use by a manually-started dev
 * server; 8081 is free for the test harness.
 */
export default defineConfig({
  testDir: '.',
  testMatch: /.*\.spec\.js/,
  fullyParallel: false,  // the engine is stateful per page; serial is simpler
  forbidOnly: !!process.env.CI,
  retries: process.env.CI ? 1 : 0,
  workers: 1,
  reporter: [['list']],

  use: {
    baseURL: 'http://localhost:8081',
    trace: 'retain-on-failure',
    screenshot: 'only-on-failure',
    // Give the WASM + Three.js stack time to initialize on slower machines
    actionTimeout: 15_000,
    navigationTimeout: 30_000,
  },

  projects: [
    {
      name: 'chromium',
      use: { ...devices['Desktop Chrome'] },
    },
  ],

  webServer: {
    // Parent dir of this tests/ folder is engine/web, which is the docroot.
    // Cached http.server keeps per-test page loads fast (the wasm64 binary is
    // large; re-fetching it per test pushes WASM init past the readiness
    // timeout). NOTE: Phase-2 worker tests need cross-origin isolation
    // (SharedArrayBuffer) — they must run against a COOP/COEP server that ALSO
    // allows caching (serve.py's no-cache headers cause WASM-init timeouts here).
    // The SAB test in scale0-sparse-tick.spec.js skips when not crossOriginIsolated.
    command: 'python -m http.server 8081',
    cwd: '..',
    port: 8081,
    reuseExistingServer: !process.env.CI,
    timeout: 30_000,
  },
});

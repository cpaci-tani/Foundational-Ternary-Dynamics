// @ts-check
import { defineConfig, devices } from '@playwright/test';
import { reuseExistingServer, testPort } from './_test-server.js';

const hardwareWebgl = process.env.FTD_HARDWARE_WEBGL === '1';
const port = testPort(8081);

/**
 * Playwright config for the FTD web dashboard smoke suite.
 *
 * The tests boot a real Chromium against index.html served over HTTP by
 * `python serve.py <port> --cache --quiet`. The webServer block below starts
 * the server before tests and tears it down after.
 *
 * Why port 8081: port 8080 is commonly in use by a manually-started dev
 * server; 8081 is free for the test harness.
 *
 * Concurrent sessions. Outside CI this config reuses whatever already listens
 * on its port, so two checkouts running the suite at once can end up testing
 * each other's files. Two environment variables (read in _test-server.js)
 * control that:
 *
 *   FTD_TEST_PORT=<1024..65535>   Port for baseURL and the webServer, here and
 *                                 in every sibling playwright.*.config.js
 *                                 (each keeps its own default when unset).
 *                                 Setting it also turns server reuse off: the
 *                                 run starts its own serve.py and aborts with
 *                                 "http://localhost:<port> is already used" if
 *                                 something else holds the port. Pick another
 *                                 port; do not stop the other server.
 *   FTD_TEST_REUSE_SERVER=0|1     Overrides the reuse default of this config
 *                                 and playwright.manual.config.js (1 outside
 *                                 CI when FTD_TEST_PORT is unset, else 0; the
 *                                 other siblings never reuse). 0 with no
 *                                 FTD_TEST_PORT pins a run to its own server
 *                                 on 8081; 1 with FTD_TEST_PORT reuses a
 *                                 long-lived server on a private port.
 *
 * Before any spec, the globalSetup asks the server for its docroot (GET
 * /api/server-info) and fails the run unless it is this checkout's engine/web.
 * A server that cannot answer, such as a serve.py older than the endpoint or a
 * plain `python -m http.server`, fails the same way. Every config that serves
 * engine/web through serve.py carries this check.
 *
 *   bash:        FTD_TEST_PORT=8091 npx playwright test simulation-menu.spec.js
 *   PowerShell:  $env:FTD_TEST_PORT = '8091'; npx playwright test simulation-menu.spec.js
 */
export default defineConfig({
  testDir: '.',
  testMatch: /.*\.spec\.js/,
  // This writes publication artifacts; it is not a visual regression oracle.
  testIgnore: [
    '**/take_gallery_screenshots.spec.js',
    '**/scale0-comprehensive-performance.spec.js',
  ],
  fullyParallel: false,  // the engine is stateful per page; serial is simpler
  forbidOnly: !!process.env.CI,
  retries: process.env.CI ? 1 : 0,
  workers: 1,
  reporter: [['list']],
  outputDir: '../test-results',  // keep artifacts under engine/web, not the repo root
  timeout: 60_000,      // 60s per test instead of default 30s
  globalSetup: './_test-server.js',

  use: {
    baseURL: `http://localhost:${port}`,
    trace: 'retain-on-failure',
    screenshot: 'only-on-failure',
    // Give the WASM + Three.js stack time to initialize on slower machines
    actionTimeout: 30_000,
    navigationTimeout: 45_000,
    // Headless Chromium otherwise selects SwiftShader on Windows even when a
    // discrete GPU is available. Release performance runs opt into the normal
    // ANGLE backend and independently assert that the resulting renderer is
    // hardware-backed before making a 60 FPS claim.
    launchOptions: hardwareWebgl ? { args: ['--use-angle=default'] } : undefined,
  },

  projects: [
    {
      name: 'chromium',
      use: { ...devices['Desktop Chrome'] },
    },
  ],

  webServer: {
    // Parent dir of this tests/ folder is engine/web, which is the docroot.
    // serve.py --cache keeps per-test page loads fast while sending COOP/COEP
    // headers, so worker/SAB coverage runs without bypassing the cached WASM.
    command: `python serve.py ${port} --cache --quiet`,
    cwd: '..',
    port,
    reuseExistingServer: reuseExistingServer(),
    timeout: 30_000,
  },
});

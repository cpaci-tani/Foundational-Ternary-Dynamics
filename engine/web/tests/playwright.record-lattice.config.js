import { defineConfig, devices } from '@playwright/test';
import { testPort } from './_test-server.js';
const port = testPort(8094);
export default defineConfig({
    testDir: '.', testMatch: 'record-lattice.spec.js', workers: 1, fullyParallel: false,
    retries: 0, timeout: 120000, reporter: [['list']], outputDir: '../test-results/record-lattice',
    globalSetup: './_test-server.js',
    use: {baseURL: `http://localhost:${port}`, trace: 'retain-on-failure', screenshot: 'only-on-failure', actionTimeout: 20000},
    projects: [{name: 'web-lattice-chromium', use: {...devices['Desktop Chrome']}}],
    webServer: {command: `python serve.py ${port} --quiet`, cwd: '..', port, reuseExistingServer: false, timeout: 30000},
});

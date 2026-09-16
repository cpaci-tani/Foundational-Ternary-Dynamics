import { defineConfig, devices } from '@playwright/test';
export default defineConfig({
    testDir: '.', testMatch: 'record-lattice.spec.js', workers: 1, fullyParallel: false,
    retries: 0, timeout: 120000, reporter: [['list']], outputDir: '../test-results/record-lattice',
    use: {baseURL: 'http://localhost:8094', trace: 'retain-on-failure', screenshot: 'only-on-failure', actionTimeout: 20000},
    projects: [{name: 'web-lattice-chromium', use: {...devices['Desktop Chrome']}}],
    webServer: {command: 'python serve.py 8094 --quiet', cwd: '..', port: 8094, reuseExistingServer: false, timeout: 30000},
});

import { defineConfig, devices } from '@playwright/test';

export default defineConfig({
    testDir: '.', testMatch: ['scale0-fluid-integration.spec.js', 's0-overlay-accordion.spec.js'],
    workers: 1, fullyParallel: false, retries: 0, reporter: 'list',
    outputDir: '../test-results/scale0-fluid', timeout: 90000,
    use: {
        baseURL: 'http://127.0.0.1:8095',
        ...devices['Desktop Chrome'],
        launchOptions: process.env.FTD_HARDWARE_WEBGL === '1' ? { args: ['--use-angle=default'] } : {},
        screenshot: 'only-on-failure', trace: 'retain-on-failure',
    },
    webServer: { command: 'python serve.py 8095 --quiet', cwd: '..',
        url: 'http://127.0.0.1:8095', reuseExistingServer: false },
});

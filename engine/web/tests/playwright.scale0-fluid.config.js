import { defineConfig, devices } from '@playwright/test';
import { testPort } from './_test-server.js';

const port = testPort(8095);

export default defineConfig({
    testDir: '.', testMatch: ['scale0-fluid-integration.spec.js', 's0-overlay-accordion.spec.js'],
    workers: 1, fullyParallel: false, retries: 0, reporter: 'list',
    outputDir: '../test-results/scale0-fluid', timeout: 90000,
    globalSetup: './_test-server.js',
    use: {
        baseURL: `http://127.0.0.1:${port}`,
        ...devices['Desktop Chrome'],
        launchOptions: process.env.FTD_HARDWARE_WEBGL === '1' ? { args: ['--use-angle=default'] } : {},
        screenshot: 'only-on-failure', trace: 'retain-on-failure',
    },
    webServer: { command: `python serve.py ${port} --quiet`, cwd: '..',
        url: `http://127.0.0.1:${port}`, reuseExistingServer: false },
});

import { defineConfig, devices } from '@playwright/test';

// Isolated local candidate only; never changes the main dashboard test server.
process.env.FTD_STRICT_LAB = '1';
const hardware = process.env.FTD_HARDWARE_WEBGL === '1';

export default defineConfig({
    testDir: '.', testMatch: 'strict-candidate-lab.spec.js',
    fullyParallel: false, workers: 1, retries: 0, timeout: 90_000,
    reporter: [['list']], outputDir: '../test-results/strict-lab',
    use: {
        baseURL: 'http://localhost:8092', trace: 'retain-on-failure',
        screenshot: 'only-on-failure', actionTimeout: 20_000,
        navigationTimeout: 30_000,
        launchOptions: hardware ? { args: ['--use-angle=default'] } : undefined,
    },
    projects: [{ name: 'strict-chromium', use: { ...devices['Desktop Chrome'] } }],
    webServer: {
        command: 'python ../../strict/serve_lab.py --port 8092',
        port: 8092, reuseExistingServer: false, timeout: 30_000,
    },
});

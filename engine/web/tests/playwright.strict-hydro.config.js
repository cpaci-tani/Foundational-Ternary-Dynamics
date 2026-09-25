import { defineConfig, devices } from '@playwright/test';

// Separate hydro law only; the main web-lattice tests use port 8094.
process.env.FTD_STRICT_HYDRO_LAB = '1';
const hardware = process.env.FTD_HARDWARE_WEBGL === '1';

export default defineConfig({
    testDir: '.', testMatch: 'strict-hydro-lab.spec.js',
    fullyParallel: false, workers: 1, retries: 0, timeout: 90_000,
    reporter: [['list']], outputDir: '../test-results/strict-hydro-lab',
    use: {
        baseURL: 'http://localhost:8097', trace: 'retain-on-failure',
        screenshot: 'only-on-failure', actionTimeout: 20_000,
        navigationTimeout: 30_000,
        launchOptions: hardware ? { args: ['--use-angle=default'] } : undefined,
    },
    projects: [{ name: 'strict-hydro-chromium', use: { ...devices['Desktop Chrome'] } }],
    webServer: {
        command: 'python ../../strict/serve_lab.py --port 8097',
        port: 8097, reuseExistingServer: false, timeout: 30_000,
    },
});

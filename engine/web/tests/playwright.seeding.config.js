import {defineConfig, devices} from '@playwright/test';
export default defineConfig({
    testDir: '.', testMatch: 'seeding-panel.spec.js', workers: 1, fullyParallel: false,
    retries: 0, timeout: 120000, reporter: [['list']], outputDir: '../test-results/seeding',
    use: {baseURL: 'http://localhost:8096', trace: 'retain-on-failure', screenshot: 'only-on-failure', actionTimeout: 20000},
    projects: [{name: 'seeding-chromium', use: {...devices['Desktop Chrome']}}],
    webServer: {command: 'python serve.py 8096 --quiet', cwd: '..', port: 8096, reuseExistingServer: false, timeout: 30000},
});

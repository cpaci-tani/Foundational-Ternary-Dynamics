import base from './playwright.config.js';

export default {
    ...base,
    testMatch: 'scale0-comprehensive-performance.spec.js',
    testIgnore: [],
    outputDir: '../test-results/scale0-hardware',
    use: { ...base.use, baseURL: 'http://localhost:8099' },
    webServer: {
        ...base.webServer,
        command: 'python serve.py 8099 --cache --quiet',
        port: 8099,
        reuseExistingServer: false,
    },
};

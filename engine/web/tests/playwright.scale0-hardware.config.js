import base from './playwright.config.js';
import { testPort } from './_test-server.js';

const port = testPort(8099);

export default {
    ...base,
    testMatch: 'scale0-comprehensive-performance.spec.js',
    testIgnore: [],
    outputDir: '../test-results/scale0-hardware',
    use: { ...base.use, baseURL: `http://localhost:${port}` },
    webServer: {
        ...base.webServer,
        command: `python serve.py ${port} --cache --quiet`,
        port,
        reuseExistingServer: false,
    },
};

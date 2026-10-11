import base from './playwright.config.js';
import { testPort } from './_test-server.js';
const port=testPort(8096);
export default {
    ...base,
    testMatch:/assistant-mcp\.spec\.js/,
    outputDir:'../test-results-mcp',
    use:{...base.use,baseURL:`http://localhost:${port}`},
    webServer:{...base.webServer,command:`python serve.py ${port} --cache --quiet`,port,reuseExistingServer:false},
};

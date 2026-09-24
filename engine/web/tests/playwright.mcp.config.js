import base from './playwright.config.js';
export default {
    ...base,
    testMatch:/assistant-mcp\.spec\.js/,
    outputDir:'../test-results-mcp',
    use:{...base.use,baseURL:'http://localhost:8096'},
    webServer:{...base.webServer,command:'python serve.py 8096 --cache --quiet',port:8096,reuseExistingServer:false},
};

import base from './playwright.config.js';
export default {
    ...base,
    testMatch:/assistant-.*\.spec\.js/,
    outputDir:'../test-results-assistant',
    use:{...base.use,baseURL:'http://localhost:8093',...(process.env.FTD_HARDWARE_WEBGL==='1'?{channel:'chrome'}:{})},
    webServer:{...base.webServer,command:'python serve.py 8093 --cache --quiet',port:8093,reuseExistingServer:false},
};

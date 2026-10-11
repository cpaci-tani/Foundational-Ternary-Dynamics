import base from './playwright.config.js';
import { testPort } from './_test-server.js';
const port=testPort(8093);
export default {
    ...base,
    testMatch:/assistant-.*\.spec\.js/,
    outputDir:'../test-results-assistant',
    use:{...base.use,baseURL:`http://localhost:${port}`,...(process.env.FTD_HARDWARE_WEBGL==='1'?{channel:'chrome'}:{})},
    webServer:{...base.webServer,command:`python serve.py ${port} --cache --quiet`,port,reuseExistingServer:false},
};

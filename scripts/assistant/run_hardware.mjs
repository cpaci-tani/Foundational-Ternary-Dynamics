import {spawnSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
const cwd=fileURLToPath(new URL('../../engine/web/tests/',import.meta.url));
const cli=fileURLToPath(new URL('../../engine/web/tests/node_modules/@playwright/test/cli.js',import.meta.url));
const tests=process.argv.slice(2);
const result=spawnSync(process.execPath,[cli,'test','--config','playwright.assistant.config.js',
    ...(tests.length?tests:['assistant-corpus.spec.js','assistant-model.spec.js','assistant-experiment-model.spec.js','assistant-performance.spec.js','assistant-live-jev.spec.js']),'--workers=1'],
{cwd,env:{...process.env,FTD_HARDWARE_WEBGL:'1'},stdio:'inherit'});
if(result.error)console.error(result.error.message);
process.exitCode=result.status??1;

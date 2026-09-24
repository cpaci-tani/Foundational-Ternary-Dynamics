import {test,expect} from '@playwright/test';
import {gotoAndReady} from './_helpers.js';
import {writeFile} from 'node:fs/promises';

test('real local model conducts an exact-step experiment through the real lattice worker',async({page},testInfo)=>{
    test.skip(process.env.FTD_HARDWARE_WEBGL!=='1','Requires actual local WebGPU inference.');
    test.setTimeout(180000);
    await gotoAndReady(page,{path:'/?engine=wasm&lattice=9'});
    await page.waitForFunction(()=>window.__FTD_DEV__?.registry.get('assistant')?.control.observe()?.capabilities.includes('lattice.step'));
    const evidence=await page.evaluate(async()=>{
        const ai=window.__FTD_DEV__.registry.get('assistant');
        const {getActiveScale0Bridge}=await import('/js/scales/scale0/state/store.js');
        const owner=getActiveScale0Bridge(window.__ftdCtx);
        if(!owner?.isWorker||!owner?.isWasm)throw new Error('Requires real WASM worker');
        await ai.control.execute({type:'lattice.pause',args:{}},{expected:ai.control.observe()});
        await ai.model.load();
        ai.knowledge.search=async()=>[];
        // Only the external decision service is a test fixture; no credentials
        // or provider calls. Model planning and every engine write are real.
        ai.jev.setKey('test-fixture-not-sent');
        const decisions=[];
        const generations=[];const generate=ai.model.generate.bind(ai.model);
        ai.model.generate=async(...args)=>{const raw=await generate(...args);generations.push(raw);return raw;};
        ai.jev.evaluate=async request=>{decisions.push(request);return{decision:'execute',confidence:1,model:'offline-decision-fixture'};};
        const before=ai.control.observe();
        const timer=setTimeout(()=>ai.service.stop('Test experiment deadline'),90000);
        await ai.service.submit('Run an experiment to advance exactly 10 ticks in total from the baseline, then compare the before and after measurements. Keep playback paused.');
        clearTimeout(timer);
        const after=ai.control.observe();
        return{model:ai.model.modelId,remoteJev:'not-tested',before,after,sameOwner:getActiveScale0Bridge(window.__ftdCtx)===owner,
            decisions:decisions.map(row=>({plan:row.plan,tick:row.observation.tick,sampleTick:row.observation.facts.sampleTick})),
            events:ai.service.transcript,generations};
    });
    const path=testInfo.outputPath('live-experiment-model.json');await writeFile(path,JSON.stringify(evidence,null,2));
    await testInfo.attach('live-experiment-model.json',{path,contentType:'application/json'});
    expect(evidence.sameOwner).toBe(true);
    expect(evidence.events.filter(event=>event.type==='error'),JSON.stringify(evidence.events.filter(event=>event.type==='error'))).toEqual([]);
    expect(BigInt(evidence.after.tick)-BigInt(evidence.before.tick)).toBe(10n);
    expect(evidence.after.facts.running).toBe(false);
    expect(evidence.decisions.some(row=>row.plan.experiment.phase==='act')).toBe(true);
    expect(evidence.decisions.at(-1).plan.experiment.phase).toBe('complete');
    expect(evidence.events.filter(event=>event.type==='experiment').at(-1).status).toBe('complete');
    expect(evidence.events.filter(event=>event.type==='answer').at(-1).text).toContain(`Completed engine tick: ${evidence.before.tick} → ${evidence.after.tick}`);
});

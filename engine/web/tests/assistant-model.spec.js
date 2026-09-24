import {test,expect} from '@playwright/test';
import {gotoAndReady,openObserverWorkspace} from './_helpers.js';

test('actual browser model produces supported command plans on the recorded GPU',async({page},testInfo)=>{
    test.skip(process.env.FTD_HARDWARE_WEBGL!=='1','Explicit hardware inference run');
    test.setTimeout(240000);
    page.on('console',message=>{if(message.text().startsWith('[AI model]'))console.log(message.text());});
    await gotoAndReady(page,{path:'/?engine=wasm&lattice=9'});
    await page.waitForFunction(()=>window.__FTD_DEV__.registry.get('assistant'));
    const evidence=await page.evaluate(async()=>{
        const ai=window.__FTD_DEV__.registry.get('assistant');const start=performance.now();
        const original=ai.model.options.onStatus;ai.model.options.onStatus=status=>{console.log('[AI model]',status.text);original?.(status);};
        await ai.model.load();
        const loadMs=performance.now()-start;
        const observation=ai.control.observe();const controller=new AbortController();
        const cases=[['Pause the simulation.','lattice.pause'],['Resume the simulation.','lattice.resume'],['Advance exactly 100 ticks.','lattice.step']];
        const results=[];
        for(const [text,expected] of cases){const t=performance.now();try{const plan=await ai.model.plan(text,observation,[],controller.signal);results.push({text,expected,plan,ms:performance.now()-t});}catch(error){results.push({text,expected,error:error.message});}}
        const gpu=await navigator.gpu.requestAdapter();
        return{loadMs,results,gpu:gpu.info,model:ai.model.modelId};
    });
    await testInfo.attach('actual-model-evidence',{body:JSON.stringify(evidence,null,2),contentType:'application/json'});
    for(const row of evidence.results){expect(row.error,JSON.stringify(row)).toBeUndefined();expect(row.plan.kind).toBe('actions');expect(row.plan.actions.some(a=>a.type===row.expected),JSON.stringify(row)).toBe(true);}
    await openObserverWorkspace(page);
    const observer=await page.evaluate(async()=>{
        const reg=window.__FTD_DEV__.registry,w=reg.get('observerWorkspace'),ai=reg.get('assistant');
        await w.command({type:'pause'});w.selectedId=w.snapshot.entities[0].id;
        const controller=new AbortController();const observation=ai.control.observe();
        const start=performance.now();
        let raw='';const generate=ai.model.generate.bind(ai.model);ai.model.generate=async(...args)=>{raw=await generate(...args);return raw;};
        try{const plan=await ai.model.plan('Make the selected object twice as heavy.',observation,[],controller.signal);return{plan,ms:performance.now()-start};}
        catch(error){return{error:error.message,raw,ms:performance.now()-start};}
    });
    await testInfo.attach('observer-model-evidence',{body:JSON.stringify(observer,null,2),contentType:'application/json'});
    expect(observer.error,JSON.stringify(observer)).toBeUndefined();
    expect(observer.plan.actions.some(a=>a.type==='observer.update'&&a.args.massFactor===2),JSON.stringify(observer)).toBe(true);
    const recovery=await page.evaluate(async()=>{
        const ai=window.__FTD_DEV__.registry.get('assistant'),controller=new AbortController();
        const pending=ai.model.generate([{role:'user',content:'Write a long explanation of coordinate geometry and optical clocks.'}],controller.signal);
        const timer=setTimeout(()=>controller.abort(new Error('User stopped generation')),200);
        let interrupted=false;
        try{await pending;}catch(error){interrupted=error.message==='User stopped generation';}finally{clearTimeout(timer);}
        const plan=await ai.model.plan('Pause the Observer.',ai.control.observe(),[],new AbortController().signal);
        let streamed='';const answer=await ai.model.explain('What is this reference world?',ai.control.observe(),[],new AbortController().signal,text=>{streamed+=text;});
        await ai.model.unload();
        return{interrupted,plan,answerLength:answer.length,streamMatches:answer===streamed,ready:ai.model.ready,workerReleased:ai.model.worker===null};
    });
    expect(recovery.interrupted).toBe(true);expect(recovery.plan.actions[0].type).toBe('observer.pause');
    expect(recovery.answerLength).toBeGreaterThan(0);expect(recovery.streamMatches).toBe(true);
    expect(recovery.ready).toBe(false);expect(recovery.workerReleased).toBe(true);
});

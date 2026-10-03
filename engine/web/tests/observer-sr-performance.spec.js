import {test, expect} from '@playwright/test';
import {writeFile} from 'node:fs/promises';
import {execFileSync} from 'node:child_process';

test('deterministic SR frame and segment cost evidence', async ({page}, info) => {
    if(process.env.FTD_SR_BASELINE==='1')await page.route('**/js/observer/*.js',async route=>{
        const path=new URL(route.request().url()).pathname.slice(1);
        const source=execFileSync('git',['show',`4e3853a06a458e0d5eaac2fc3d7c4bdfffe4596f:engine/web/${path}`],{encoding:'utf8'});
        await route.fulfill({body:source,contentType:'text/javascript'});
    });
    await page.goto('/tests/observer-sr-harness.html');
    await page.waitForFunction(() => window.sr?.renderer);
    const evidence = await page.evaluate(async () => {
        const {session, renderer} = window.sr;
        session.state.entities = []; session.state.segments = [];
        for(let i=0;i<12;i++) session.addEntity({shape:'beacon',position:[(i%4-1.5)*2, 2+Math.floor(i/4), -3],properAcceleration:[.01,0,0]},true);
        const settings = {layers:{grid:false}, feedbackEnabled:false, artisticShading:false, autoQuality:false, renderScale:1};
        for(let i=0;i<120;i++)session.step(1/120,{});
        let stepTime=0;
        for(let i=0;i<120;i++){const start=performance.now();session.step(1/120,{});stepTime+=performance.now()-start;}
        const snapshot = session.snapshot();
        for(let i=0;i<20;i++) renderer.render(snapshot,settings);
        const costs=[],frames=[]; let previous=performance.now();
        for(let i=0;i<120;i++) {
            await new Promise(resolve=>requestAnimationFrame(resolve));
            const start=performance.now();frames.push(start-previous);previous=start;
            renderer.render(snapshot,settings); renderer.renderer.getContext().finish(); costs.push(performance.now()-start);
        }
        const summarize = values => {const sorted=[...values].sort((a,b)=>a-b);return {mean:values.reduce((a,b)=>a+b,0)/values.length,p95:sorted[Math.floor(sorted.length*.95)],max:sorted.at(-1)};};
        const {ObserverSession}=await import('/js/observer/session.js'),motion=new ObserverSession({playing:false});
        const marker=motion.addEntity({shape:'beacon',position:[0,0,0],properAcceleration:[.5,0,0]});motion.state.observer.position=[0,0,0];
        for(let i=0;i<120;i++)motion.step(1/120,{move:[1,0,0],speed:.99,acceleration:.5});
        const actual={observerX:motion.state.observer.position[0],observerClock:motion.state.observer.properTime,markerX:marker.position[0],markerClock:marker.clockOffset};motion.dispose();
        return {gpu:renderer.diagnostics.gpu,segments:snapshot.segments.length,resolution:renderer.diagnostics.internalResolution,frames:120,stepMeanMs:stepTime/120,render:summarize(costs),frame:summarize(frames),motion:actual};
    });
    expect(evidence.segments).toBeLessThan(65536);
    await writeFile(info.outputPath('sr-performance.json'), JSON.stringify(evidence,null,2));
    await info.attach('sr-performance',{body:JSON.stringify(evidence,null,2),contentType:'application/json'});
});

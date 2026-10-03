import {test,expect} from '@playwright/test';
import {writeFile} from 'node:fs/promises';

test('compiled GPU event, clock and Doppler diagnostics agree with Float64 geometry',async({page,browser},info)=>{
    await page.goto('/tests/observer-sr-harness.html');await page.waitForFunction(()=>window.sr?.renderer);
    const evidence=await page.evaluate(async()=>{
        const {session,renderer,optics}=window.sr;
        const {SHAPE_NAMES}=await import('/js/observer/geometry.js');
        renderer.resize(200,200);
        const snapshot=session.snapshot(), template=snapshot.entities[0];
        const settings={layers:{grid:false},feedbackEnabled:false,artisticShading:false,autoQuality:false,selectedId:''};
        const maxima={distance:0,emissionTime:0,properTime:0,doppler:0,position:0,relativeDistance:0,relativeDoppler:0};
        const failures=[],boundaries=[],misses=[];let hits=0;
        // Fixed budgets: interior event/clock/position 0.003 coordinate units;
        // D 0.0002 relative. Silhouettes are tested separately, never certified
        // at the same conditioning as stable interior intersections.
        for(const translation of [0,1e7])for(const beta of [0,.6,.99])for(const mirrored of [false,true])for(const shape of SHAPE_NAMES){
            snapshot.time=10+translation;snapshot.historyStart=snapshot.time-60;
            snapshot.observer={...snapshot.observer,position:[translation,3,6],velocity:[beta,0,0],yaw:.15,pitch:mirrored?-.35:.12,roll:.2};
            const ray=optics.observerRay(snapshot,settings,0,0),v=[.2,.1,-.12];
            const imageCenter=ray.origin.map((x,j)=>x+ray.direction[j]*8);
            const physicalCenter=imageCenter.map((x,j)=>mirrored&&j===1?-x:x);
            const e={...template,id:'parity',shape,size:[2.5,2.5,2.5],rotation:[.2,.3,-.1],position:physicalCenter.map((x,j)=>x+v[j]*8),velocity:v,originTime:snapshot.time,clockOffset:translation+1,revision:7};
            snapshot.entities=[{...e,alive:false,revision:8}];snapshot.segments=[{...e,entityId:e.id,start:snapshot.time-60,end:snapshot.time-1}];
            const options={...settings,mirrorWorld:mirrored};
            renderer.prepare(snapshot,options);
            for(const x of [-.05,0,.05]) {
                const cpu=renderer.pick(snapshot,options,x,0),gpu=renderer.readPixelHit(snapshot,options,x,0);
                if(!cpu){if(gpu)failures.push({shape,beta,translation,mirrored,x,cpu,gpu});else misses.push({shape,beta,mirrored});continue;}
                const nearby=[-1,1].map(sign=>renderer.pick(snapshot,options,x+sign*1e-4,0));
                const stable=nearby.every(hit=>hit?.revision===cpu.revision&&hit?.mirrored===cpu.mirrored&&Math.abs(hit.distance-cpu.distance)<.02);
                if(!stable){boundaries.push({shape,beta,translation,mirrored,x,cpu,gpu});continue;}
                hits++;
                if(!gpu||gpu.entityId!==cpu.entityId||gpu.revision!==cpu.revision||gpu.mirrored!==cpu.mirrored){failures.push({shape,beta,translation,mirrored,x,cpu,gpu});continue;}
                const error={};
                for(const key of ['distance','emissionTime','properTime','doppler']){error[key]=Math.abs(cpu[key]-gpu[key]);maxima[key]=Math.max(maxima[key],error[key]);}
                error.position=Math.max(...cpu.sourcePosition.map((value,j)=>Math.abs(value-gpu.sourcePosition[j])));maxima.position=Math.max(maxima.position,error.position);
                const relativeD=error.doppler/Math.max(1,cpu.doppler);maxima.relativeDoppler=Math.max(maxima.relativeDoppler,relativeD);
                maxima.relativeDistance=Math.max(maxima.relativeDistance,error.distance/Math.max(1,cpu.distance));
                if(error.distance>.003||error.emissionTime>.003||error.properTime>.003||error.position>.003||relativeD>.0002)failures.push({shape,beta,translation,mirrored,x,error,cpu,gpu});
            }
        }
        const gl=renderer.renderer.getContext(),precision=gl.getShaderPrecisionFormat(gl.FRAGMENT_SHADER,gl.HIGH_FLOAT);
        return {hits,misses:misses.length,boundaries:boundaries.length,boundaryCases:boundaries,failures:failures.slice(0,12),maxima,gpu:renderer.diagnostics.gpu,precision:{bits:precision.precision,rangeMin:precision.rangeMin,rangeMax:precision.rangeMax}};
    });
    const path=info.outputPath('sr-gpu-evidence.json');await writeFile(path,JSON.stringify({...evidence,browser:browser.version()},null,2));
    await info.attach('GPU-SR-evidence',{path,contentType:'application/json'});
    expect(evidence.hits).toBeGreaterThan(120);expect(evidence.failures).toEqual([]);
    for(const boundary of evidence.boundaryCases)if(boundary.gpu){
        expect(boundary.gpu.entityId).toBe(boundary.cpu.entityId);
        expect(Math.abs(boundary.cpu.distance-boundary.gpu.distance)).toBeLessThan(.05);
    }
});

test('actual accelerated session histories remain optical CPU/GPU clock evidence',async({page},info)=>{
    await page.goto('/tests/observer-sr-harness.html');await page.waitForFunction(()=>window.sr?.renderer);
    const evidence=await page.evaluate(()=>{
        const {session,renderer}=window.sr;session.state.entities=[];session.state.segments=[];
        const e=session.addEntity({shape:'beacon',position:[0,3,0],size:[.0004,.0004,.0004],properAcceleration:[.5,0,0]});
        for(let i=0;i<120;i++)session.step(1/120,{});
        const snapshot=session.snapshot(),emissionTime=.9041666666666667;
        const s=snapshot.segments.find(s=>s.entityId===e.id&&s.start<=emissionTime&&s.end>emissionTime);
        const source=s.position.map((x,j)=>x+s.velocity[j]*(emissionTime-s.originTime));
        snapshot.observer={...snapshot.observer,position:source.map((x,j)=>x+(j===2?snapshot.time-emissionTime:0)),velocity:[0,0,0]};
        renderer.resize(200,200);
        const options={layers:{grid:false},mirrorWorld:false,feedbackEnabled:false};
        return {cpu:renderer.pick(snapshot,options),gpu:renderer.readPixelHit(snapshot,options),segment:s};
    });
    const path=info.outputPath('sr-accelerated-event.json');await writeFile(path,JSON.stringify(evidence,null,2));
    await info.attach('accelerated-optical-event',{path,contentType:'application/json'});
    expect(evidence.cpu?.revision).toBe(evidence.segment.revision);expect(evidence.gpu?.revision).toBe(evidence.cpu.revision);
    expect(Math.abs(evidence.cpu.properTime-evidence.gpu.properTime)).toBeLessThan(3e-5);
    expect(Math.abs(evidence.cpu.doppler-evidence.gpu.doppler)).toBeLessThan(3e-5);
});

test('sphere silhouettes and near/far surfaces have an explicit geometric precision budget',async({page},info)=>{
    await page.goto('/tests/observer-sr-harness.html');await page.waitForFunction(()=>window.sr?.renderer);
    const evidence=await page.evaluate(()=>{
        const {session,renderer}=window.sr,snapshot=session.snapshot();renderer.resize(200,200);
        snapshot.observer={...snapshot.observer,position:[0,3,6],velocity:[0,0,0]};
        const entity={...snapshot.entities[0],id:'sphere',shape:'sphere',position:[0,3,0],size:[2,2,2],rotation:[0,0,0],velocity:[0,0,0],originTime:0};
        snapshot.entities=[entity];snapshot.segments=[{...entity,entityId:entity.id,start:-60,end:null}];
        const settings={layers:{grid:false},mirrorWorld:false,feedbackEnabled:false};
        const tangent=1/Math.sqrt(35)/Math.tan(Math.PI/6),rows=[];
        for(const delta of [-1e-4,0,1e-4]){
            const cpu=renderer.pick(snapshot,settings,tangent+delta,0),gpu=renderer.readPixelHit(snapshot,settings,tangent+delta,0);
            rows.push({delta,cpu,gpu});
        }
        const distances=[];
        for(const centerDistance of [.01,50]){
            const radius=centerDistance===.01?.001:1;
            snapshot.segments[0].position=[0,3,6-centerDistance];snapshot.segments[0].size=[2*radius,2*radius,2*radius];
            renderer.prepare(snapshot,settings);
            distances.push({expected:centerDistance-radius,cpu:renderer.pick(snapshot,settings),gpu:renderer.readPixelHit(snapshot,settings)});
        }
        return {rows,distances};
    });
    expect(evidence.rows[0].gpu).toBeTruthy();expect(evidence.rows[2].gpu).toBeNull();
    expect(Math.abs(evidence.rows[0].cpu.distance-evidence.rows[0].gpu.distance)).toBeLessThan(.003);
    if(evidence.rows[1].gpu)expect(Math.abs(evidence.rows[1].gpu.distance-Math.sqrt(35))).toBeLessThan(.004);
    for(const d of evidence.distances){expect(Math.abs(d.gpu.distance-d.expected)).toBeLessThan(.003);expect(d.gpu.entityId).toBe('sphere');}
    const path=info.outputPath('sr-conditioning-evidence.json');await writeFile(path,JSON.stringify(evidence,null,2));
    await info.attach('silhouette-conditioning',{path,contentType:'application/json'});
});

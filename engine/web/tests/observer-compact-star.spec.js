/* global window, process, structuredClone */
import {test,expect} from '@playwright/test';
import {writeFile} from 'node:fs/promises';
import {gotoAndReady,openObserverWorkspace} from './_helpers.js';

test('Schwarzschild production GPU rays, clocks and local Doppler match Float64 geodesics',async({page,browser},info)=>{
    const errors=[];page.on('pageerror',e=>errors.push(e.message));page.on('console',m=>{if(m.type()==='error')errors.push(m.text());});
    await page.goto('/tests/observer-sr-harness.html');await page.waitForFunction(()=>window.sr?.renderer);
    const evidence=await page.evaluate(async()=>{
        const {session,renderer}=window.sr;await session.command({type:'preset',preset:'compact-star'});renderer.resize(240,240);
        const settings={autoQuality:false,mirrorWorld:false,feedbackEnabled:false,artisticShading:false,layers:{},fov:60};
        const maxima={distance:0,emissionTime:0,properTime:0,doppler:0,position:0};let hits=0,misses=0;const failures=[];
        for(const position of [[0,1.6,3],[0,1.6,1],[.4,1.6,1]])for(const velocity of [[0,0,0],[0,0,-.6],[.2,0,-.3],[0,0,-.9],[0,0,.6]]){
            const snapshot=session.snapshot();snapshot.time=8;snapshot.historyStart=-52;snapshot.observer.position=position;snapshot.observer.velocity=velocity;
            snapshot.observer.yaw=Math.atan2(position[0],position[2]);
            for(const [x,y] of [[0,0],[-.035,.02],[.035,.02],[0,-.15],[.8,.8]]){
                const cpu=renderer.pick(snapshot,settings,x,y),gpu=renderer.readPixelHit(snapshot,settings,x,y);
                if(!cpu){misses++;if(gpu)failures.push({position,velocity,x,y,cpu,gpu});continue;}
                hits++;if(!gpu||gpu.entityId!==cpu.entityId||gpu.revision!==cpu.revision){failures.push({position,velocity,x,y,cpu,gpu});continue;}
                const error={};for(const key of ['distance','emissionTime','properTime','doppler']){error[key]=Math.abs(cpu[key]-gpu[key]);maxima[key]=Math.max(maxima[key],error[key]);}
                error.position=Math.max(...cpu.position.map((v,j)=>Math.abs(v-gpu.position[j])));maxima.position=Math.max(maxima.position,error.position);
                if(Object.values(error).some(v=>v>.0005))failures.push({position,velocity,x,y,error});
            }
        }
        const staticSnapshot=session.snapshot();staticSnapshot.time=8;const normal=renderer.readPixelHit(staticSnapshot,settings),linear=renderer.readPixelLinearColor(staticSnapshot,settings),raw=[...renderer.readDiagnosticPixel(staticSnapshot,settings,0,0,2)];
        const belowSphere=[-.22,-.4,-.7].map(y=>({y,cpu:renderer.pick(staticSnapshot,settings,0,y),gpu:renderer.readPixelHit(staticSnapshot,settings,0,y)}));
        const cutoff={...staticSnapshot,historyStart:7};const unavailable=renderer.readPixelHit(cutoff,settings);
        const star=staticSnapshot.spacetime,q=star.rs/12,Aobs=(1-q)/(1+q),Bobs=(1+q)**2,As=Math.sqrt(1-star.rs/.25);
        const angle=Math.asin((.25/As)/(3*Bobs/Aobs)),silhouette=[];
        for(const fraction of [.999,1.001]){const x=Math.tan(angle*fraction)/Math.tan(Math.PI/6);silhouette.push({fraction,cpu:renderer.pick(staticSnapshot,settings,x,0),gpu:renderer.readPixelHit(staticSnapshot,settings,x,0)});}
        const {compactStar,starApparatus}=await import('/js/observer/compact-star.js'),zero=structuredClone(staticSnapshot);zero.spacetime=compactStar(0);zero.spacetime.sourceId=zero.entities[0].id;
        const apparatus=starApparatus(zero.spacetime);zero.segments=zero.segments.map((s,i)=>({...s,...apparatus[i]}));zero.entities=zero.entities.map((e,i)=>({...e,...apparatus[i]}));
        const flatFrequency=renderer.readPixelHit(zero,settings).doppler,baselineOn=renderer.readPixelLinearColor(zero,{...settings,doppler:true}),baselineOff=renderer.readPixelLinearColor(zero,{...settings,doppler:false});
        const unboosted=renderer.readPixelLinearColor(staticSnapshot,{...settings,doppler:false,beaming:false}),boosted=renderer.readPixelLinearColor(staticSnapshot,{...settings,doppler:false,beaming:true});
        const intensityError=Math.max(...boosted.map((v,i)=>Math.abs(v-unboosted[i]*normal.doppler**4)));
        const late={...staticSnapshot,time:1e7,historyStart:1e7-60};const lateClock={cpu:renderer.pick(late,settings),gpu:renderer.readPixelHit(late,settings)};
        const gl=renderer.renderer.getContext(),precision=gl.getShaderPrecisionFormat(gl.FRAGMENT_SHADER,gl.HIGH_FLOAT);
        const program=gl.getParameter(gl.CURRENT_PROGRAM),uniforms={};for(const name of ['uStarRs','uStarRadius','uStarEscape','uDebugMode','uStarCenter','uForward']){const location=gl.getUniformLocation(program,name);uniforms[name]=location?gl.getUniform(program,location):null;}
        return {hits,misses,maxima,failures:failures.slice(0,6),normal,linear,unavailable,raw,belowSphere,silhouette,flatFrequency,baselineOn,baselineOff,intensityError,lateClock,uniforms,entityCount:staticSnapshot.entities.length,segmentCount:staticSnapshot.segments.length,programLinked:gl.getProgramParameter(program,gl.LINK_STATUS),programLog:gl.getProgramInfoLog(program),glError:gl.getError(),gpu:renderer.diagnostics.gpu,precision:{bits:precision.precision},provider:staticSnapshot.spacetime,shader:renderer.mesh.material.fragmentShader.includes('renderCompactStar')};
    });
    const path=info.outputPath('compact-star-gpu-evidence.json');await writeFile(path,JSON.stringify({...evidence,browser:browser.version()},null,2));await info.attach('actual-GPU-geodesic-evidence',{path,contentType:'application/json'});
    expect(errors).toEqual([]);expect(evidence.hits).toBeGreaterThan(12);expect(evidence.misses).toBeGreaterThan(0);expect(evidence.failures).toEqual([]);
    expect(evidence.normal.doppler).toBeLessThan(1);expect(evidence.linear.every(Number.isFinite)).toBe(true);expect(evidence.unavailable).toBeNull();
    expect(evidence.entityCount).toBe(1);expect(evidence.segmentCount).toBe(1);expect(evidence.belowSphere.every(ray=>ray.cpu===null&&ray.gpu===null)).toBe(true);
    expect(evidence.silhouette[0].gpu?.entityId).toBe(evidence.provider.sourceId);expect(evidence.silhouette[1].gpu).toBeNull();
    expect(Math.abs(evidence.silhouette[0].gpu.emissionTime-evidence.silhouette[0].cpu.emissionTime)).toBeLessThan(.005);
    expect(Math.abs(evidence.flatFrequency-1)).toBeLessThan(1e-6);expect(evidence.baselineOn).toEqual(evidence.baselineOff);expect(evidence.intensityError).toBeLessThan(1e-6);
    expect(evidence.lateClock.gpu.entityId).toBe(evidence.lateClock.cpu.entityId);expect(Math.abs(evidence.lateClock.gpu.properTime-evidence.lateClock.cpu.properTime)).toBeLessThan(.0001);
    if(process.env.FTD_HARDWARE_WEBGL==='1')expect(evidence.gpu.renderer).not.toMatch(/swiftshader|llvmpipe|software/i);
});

test('compact-star experiment is reachable through Mind’s Eye with freefall and support controls',async({page},info)=>{
    await gotoAndReady(page);await openObserverWorkspace(page);
    await page.locator('[data-observer-panel-tab="experiments"]').click();
    await page.locator('[data-observer-action="experiment"][data-value="compact-star"]').click();
    await expect(page.locator('[data-observer-compact-star]')).toContainText('surface redshift z=0.235');
    await expect(page.getByText('SCHWARZSCHILD GR + LOCAL SR',{exact:true})).toBeVisible();
    expect(await page.evaluate(()=>window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.entities.map(e=>e.shape))).toEqual(['sphere']);
    await page.screenshot({path:info.outputPath('compact-star-controls.png')});
    await page.evaluate(()=>{window.__starStart=window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.observer.position[2];});
    await page.getByRole('button',{name:'Release into free fall'}).click();
    await expect(page.locator('[data-observer-compact-star]')).toContainText('FREE FALL');
    await page.waitForFunction(()=>window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.observer.position[2]<window.__starStart-1e-5);
    await page.getByRole('button',{name:'Hold at rest'}).click();
    await expect(page.locator('[data-observer-compact-star]')).toContainText('GUIDED / SUPPORTED');
    await page.evaluate(async()=>{const w=window.__FTD_DEV__.registry.get('observerWorkspace');await w.command({type:'pause'});w.ui.closePanel();});
    await page.screenshot({path:info.outputPath('compact-star-scene.png')});
    const evidence=await page.evaluate(()=>{const w=window.__FTD_DEV__.registry.get('observerWorkspace');return {provider:w.snapshot.spacetime,observer:w.snapshot.observer,rendering:w.renderer.diagnostics,observation:w.exportObservationDiagnostics()};});
    await writeFile(info.outputPath('compact-star-ui-evidence.json'),JSON.stringify(evidence,null,2));
});

test('GR GPU material survives metric switching and a real WebGL context loss',async({page},info)=>{
    await page.goto('/tests/observer-sr-harness.html');await page.waitForFunction(()=>window.sr?.renderer);
    const before=await page.evaluate(async()=>{
        const {session,renderer}=window.sr;await session.command({type:'preset',preset:'compact-star'});
        window.__starSnapshot=session.snapshot();window.__starSettings={autoQuality:false,feedbackEnabled:false};
        const star=renderer.readPixelHit(window.__starSnapshot,window.__starSettings);
        await session.command({type:'preset',preset:'baseline'});const flat=session.snapshot();flat.observer.position[1]=1;
        const minkowski=renderer.readPixelHit(flat,window.__starSettings);
        window.__starLose=renderer.renderer.getContext().getExtension('WEBGL_lose_context');window.__starLose.loseContext();
        return {star,minkowski};
    });
    await page.waitForFunction(()=>window.sr.renderer.contextLost);await page.evaluate(()=>window.__starLose.restoreContext());
    await page.waitForFunction(()=>!window.sr.renderer.contextLost);
    const after=await page.evaluate(()=>{const {renderer}=window.sr;const a=JSON.stringify(window.__starSnapshot);const star=renderer.readPixelHit(window.__starSnapshot,window.__starSettings);return {star,unchanged:a===JSON.stringify(window.__starSnapshot),gpu:renderer.diagnostics.gpu};});
    await writeFile(info.outputPath('compact-star-context-evidence.json'),JSON.stringify({before,after},null,2));
    expect(before.minkowski).not.toBeNull();expect(after.star.entityId).toBe(before.star.entityId);expect(after.unchanged).toBe(true);
    expect(Math.abs(after.star.doppler-before.star.doppler)).toBeLessThan(1e-5);
});

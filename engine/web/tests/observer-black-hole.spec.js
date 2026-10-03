/* global window, structuredClone */
import {test,expect} from '@playwright/test';
import {writeFile} from 'node:fs/promises';
import {gotoAndReady,openObserverWorkspace} from './_helpers.js';

// These tests instrument the production WebGL material. Analytic controls use
// areal Schwarzschild coordinates, separately from the Float64 Fermat tracer.
async function harness(page){
    await page.goto('/tests/observer-sr-harness.html');
    await page.waitForFunction(()=>window.sr?.renderer);
}
async function evidenceFile(info,name,evidence){
    const path=info.outputPath(name);
    await writeFile(path,JSON.stringify(evidence,null,2));
    await info.attach(name,{path,contentType:'application/json'});
}
function hardware(gpu){
    expect(gpu.floatReadback,'Actual float-target readback is required').toBe(true);
    expect(gpu.renderer,'The hardware renderer must be identified').toBeTruthy();
    expect(gpu.renderer,'Software rendering cannot certify these GPU regressions').not.toMatch(/swiftshader|llvmpipe|software|softpipe/i);
}

test('actual GPU black-hole shadow separates capture and escape at the analytic critical impact parameter',async({page,browser},info)=>{
    test.setTimeout(120000);await harness(page);
    const evidence=await page.evaluate(async()=>{
        const {session,renderer}=window.sr;await session.command({type:'preset',preset:'black-hole'});renderer.resize(240,240);
        const settings={autoQuality:false,mirrorWorld:false,feedbackEnabled:false,artisticShading:false,layers:{},fov:150};
        const original=session.snapshot(),hole=original.spacetime,critical=1.5*Math.sqrt(3)*hole.rs,rays=[];
        for(const rho of [3,1,.4,.15,.11]){
            const q=hole.rs/(4*rho),A=(1-q)/(1+q),B=(1+q)**2,r=rho*B;
            const outsidePhotonSphere=r>1.5*hole.rs,angle=Math.asin(critical*A/r);
            // Inside the photon sphere, test the complementary outward escape
            // cone. Its exterior is the > hemisphere black-hole shadow.
            for(const fraction of [.99,1.01])for(const sign of [-1,1]){
                const snapshot=structuredClone(original);snapshot.observer.position=[0,1.6,rho];snapshot.observer.velocity=[0,0,0];
                snapshot.observer.yaw=outsidePhotonSphere?0:Math.PI;snapshot.observer.pitch=0;
                const x=sign*Math.tan(angle*fraction)/Math.tan(settings.fov*Math.PI/360);
                const cpu=renderer.inspectRay(snapshot,settings,x,0),path=[...renderer.readDiagnosticPixel(snapshot,settings,x,0,5)];
                const point=[...renderer.readDiagnosticPixel(snapshot,settings,x,0,4)],direction=[...renderer.readDiagnosticPixel(snapshot,settings,x,0,6)];
                const frequency=[...renderer.readDiagnosticPixel(snapshot,settings,x,0,2)],color=renderer.readPixelLinearColor(snapshot,settings,x,0);
                const endpoint=point.slice(0,3).map((v,i)=>v+snapshot.observer.position[i]-hole.center[i]),endRho=Math.hypot(...endpoint);
                const qe=hole.rs/(4*endRho),ne=(1+qe)**3/(1-qe),cross=[endpoint[1]*direction[2]-endpoint[2]*direction[1],endpoint[2]*direction[0]-endpoint[0]*direction[2],endpoint[0]*direction[1]-endpoint[1]*direction[0]];
                const finalImpact=ne*Math.hypot(...cross),initialImpact=r/A*Math.sin(angle*fraction);
                const expected=outsidePhotonSphere?(fraction<1?1:2):(fraction<1?2:1);
                rays.push({rho,fraction,sign,expected,cpu,path,point,direction,frequency,color,endRho,initialImpact,finalImpact,hit:renderer.readPixelHit(snapshot,settings,x,0)});
            }
        }
        const gl=renderer.renderer.getContext(),program=gl.getParameter(gl.CURRENT_PROGRAM);
        return {hole,critical,rays,gpu:renderer.diagnostics.gpu,linked:gl.getProgramParameter(program,gl.LINK_STATUS),programLog:gl.getProgramInfoLog(program),glError:gl.getError(),entityCount:original.entities.length,segmentCount:original.segments.length};
    });
    await evidenceFile(info,'black-hole-shadow-gpu.json',{...evidence,browser:browser.version()});hardware(evidence.gpu);
    expect(evidence.linked).toBe(true);expect(evidence.glError).toBe(0);expect(evidence.entityCount).toBe(0);expect(evidence.segmentCount).toBe(0);
    for(const ray of evidence.rays){
        expect(ray.path[0],JSON.stringify({rho:ray.rho,fraction:ray.fraction,sign:ray.sign})).toBe(ray.expected);
        expect(ray.cpu.status).toBe(ray.expected===1?'captured':'escaped');expect(ray.point[3]).toBe(ray.expected);expect(ray.frequency[3]).toBe(ray.expected);
        expect(ray.hit,'A horizon is never an emitting entity').toBeNull();expect([...ray.path,...ray.point,...ray.direction,...ray.frequency].every(Number.isFinite)).toBe(true);
        expect(Math.abs(ray.path[3]-ray.initialImpact)).toBeLessThan(1e-5);
        expect(Math.abs(Math.hypot(...ray.direction.slice(0,3))-1)).toBeLessThan(2e-5);
        if(ray.expected===2){
            expect(Math.abs(ray.endRho-evidence.hole.escapeRadius)).toBeLessThan(.0002);
            expect(Math.abs(ray.finalImpact-ray.initialImpact)).toBeLessThan(.0002);
            expect(Math.abs(ray.direction[3]-ray.finalImpact)).toBeLessThan(.00002);
            expect(Math.abs(ray.path[2]-ray.cpu.delay)).toBeLessThan(.005);
        }else{
            expect(ray.color.every(v=>Math.abs(v)<1e-8),'Captured light is dark').toBe(true);
            expect(ray.frequency[0],'A captured ray has no received boundary frequency').toBe(0);
        }
    }
});

test('actual GPU finite-shell travel time, local Doppler and null invariant agree with independent Schwarzschild controls',async({page,browser},info)=>{
    test.setTimeout(120000);await harness(page);
    const evidence=await page.evaluate(async()=>{
        const {session,renderer}=window.sr;await session.command({type:'preset',preset:'black-hole'});renderer.resize(240,240);
        const settings={autoQuality:false,feedbackEnabled:false,artisticShading:false,layers:{},fov:60};
        const original=session.snapshot(),hole=original.spacetime,rays=[];
        const metric=rho=>{const q=hole.rs/(4*rho);return {A:(1-q)/(1+q),r:rho*(1+q)**2};};
        const observerMetric=metric(3),shell=metric(hole.escapeRadius);
        const integrate=fn=>{const a=observerMetric.r,b=shell.r,n=8000,h=(b-a)/n;let sum=fn(a)+fn(b);for(let j=1;j<n;j++)sum+=(j%2?4:2)*fn(a+j*h);return sum*h/3;};
        for(const x of [0,.12,.3,.6]){
            const snapshot=structuredClone(original);snapshot.observer.position=[0,1.6,3];snapshot.observer.velocity=[0,0,0];snapshot.observer.yaw=Math.PI;
            const angle=Math.atan(x*Math.tan(Math.PI/6)),impact=observerMetric.r/observerMetric.A*Math.sin(angle);
            const root=r=>Math.sqrt(1-impact**2*(1-hole.rs/r)/r**2);
            const exactDelay=integrate(r=>1/((1-hole.rs/r)*root(r)));
            const exactDistance=integrate(r=>{const rho=(r-hole.rs/2+Math.sqrt(r*(r-hole.rs)))/2,q=hole.rs/(4*rho);return 1/((1-q*q)*root(r));});
            const cpu=renderer.inspectRay(snapshot,settings,x,0),path=[...renderer.readDiagnosticPixel(snapshot,settings,x,0,5)],frequency=[...renderer.readDiagnosticPixel(snapshot,settings,x,0,2)];
            rays.push({x,impact,exactDelay,exactDistance,cpu,path,frequency,expectedFrequency:shell.A/observerMetric.A});
        }
        const motion=[];
        for(const velocity of [[0,0,.6],[0,0,-.6],[.3,0,0]]){
            const snapshot=structuredClone(original);snapshot.observer.position=[0,1.6,3];snapshot.observer.velocity=velocity;snapshot.observer.yaw=Math.PI;
            const g=1/Math.sqrt(1-velocity.reduce((sum,v)=>sum+v*v,0)),expected=shell.A/observerMetric.A/(g*(1-velocity[2]));
            motion.push({velocity,expected,cpu:renderer.inspectRay(snapshot,settings,0,0),path:[...renderer.readDiagnosticPixel(snapshot,settings,0,0,5)],frequency:[...renderer.readDiagnosticPixel(snapshot,settings,0,0,2)]});
        }
        // An outward collinear observer speed can exactly cancel the shell's
        // gravitational blueshift. Both color-toggle branches then have D=1.
        const baseline=structuredClone(original),K=shell.A/observerMetric.A;
        baseline.observer.position=[0,1.6,3];baseline.observer.yaw=Math.PI;baseline.observer.velocity=[0,0,(1-K*K)/(1+K*K)];
        const baselineFrequency=[...renderer.readDiagnosticPixel(baseline,settings,0,0,2)];
        const baselineOn=renderer.readPixelLinearColor(baseline,{...settings,doppler:true,beaming:false}),baselineOff=renderer.readPixelLinearColor(baseline,{...settings,doppler:false,beaming:false});
        return {rays,motion,baselineFrequency,baselineOn,baselineOff,gpu:renderer.diagnostics.gpu,hole,radialExact:shell.r-observerMetric.r+hole.rs*Math.log((shell.r-hole.rs)/(observerMetric.r-hole.rs))};
    });
    await evidenceFile(info,'black-hole-finite-shell-gpu.json',{...evidence,browser:browser.version()});hardware(evidence.gpu);
    expect(Math.abs(evidence.rays[0].exactDelay-evidence.radialExact)).toBeLessThan(1e-9);
    for(const ray of evidence.rays){
        expect(ray.path[0]).toBe(2);expect(ray.cpu.status).toBe('escaped');expect(ray.frequency[3]).toBe(2);
        expect(Math.abs(ray.path[1]-ray.exactDistance)).toBeLessThan(.0002);
        expect(Math.abs(ray.path[2]-ray.exactDelay)).toBeLessThan(.0002);
        expect(Math.abs(ray.cpu.delay-ray.exactDelay)).toBeLessThan(2e-6);
        expect(Math.abs(ray.path[3]-ray.impact)).toBeLessThan(2e-6);
        expect(Math.abs(ray.frequency[0]-ray.expectedFrequency)).toBeLessThan(2e-6);
        expect(ray.path[2]).toBeGreaterThan(ray.path[1]);
    }
    for(const ray of evidence.motion){expect(ray.path[0]).toBe(2);expect(Math.abs(ray.frequency[0]-ray.expected)).toBeLessThan(2e-6);}
    expect(Math.abs(evidence.baselineFrequency[0]-1)).toBeLessThan(2e-6);
    expect(Math.max(...evidence.baselineOn.map((v,i)=>Math.abs(v-evidence.baselineOff[i])))).toBeLessThan(1e-6);
});

test('actual GPU near-critical uncertainty stays explicit without a horizon clock or shadow color',async({page},info)=>{
    await harness(page);
    const evidence=await page.evaluate(async()=>{
        const {session,renderer}=window.sr;await session.command({type:'preset',preset:'black-hole'});renderer.resize(240,240);
        const snapshot=session.snapshot(),hole=snapshot.spacetime,rho=1,q=hole.rs/(4*rho),A=(1-q)/(1+q),r=rho*(1+q)**2,critical=1.5*Math.sqrt(3)*hole.rs;
        snapshot.observer.position=[0,1.6,rho];snapshot.observer.velocity=[0,0,0];snapshot.observer.yaw=0;
        // This lies inside the Float32 critical-impact uncertainty band. Early
        // classification avoids presenting an unstable numerical winding path
        // as resolved, while the Float64 radial potential predicts capture.
        const impact=critical*(1-2e-6),angle=Math.asin(impact*A/r),settings={autoQuality:false,feedbackEnabled:false,fov:150};
        const x=Math.tan(angle)/Math.tan(150*Math.PI/360),cpu=renderer.inspectRay(snapshot,settings,x,0);
        return {impact,critical,cpu,path:[...renderer.readDiagnosticPixel(snapshot,settings,x,0,5)],frequency:[...renderer.readDiagnosticPixel(snapshot,settings,x,0,2)],color:renderer.readPixelLinearColor(snapshot,settings,x,0),hit:renderer.readPixelHit(snapshot,settings,x,0),gpu:renderer.diagnostics.gpu};
    });
    await evidenceFile(info,'black-hole-unresolved-gpu.json',evidence);hardware(evidence.gpu);
    expect(evidence.impact).toBeLessThan(evidence.critical);expect(evidence.cpu.status).toBe('captured');
    expect(evidence.path[0]).toBe(3);expect(evidence.path[1]).toBeGreaterThanOrEqual(0);expect(evidence.path[2]).toBeGreaterThanOrEqual(0);
    expect(evidence.path.every(Number.isFinite)).toBe(true);
    expect(evidence.hit).toBeNull();expect(evidence.frequency[0]).toBe(0);expect(evidence.frequency[1]).toBe(0);expect(evidence.frequency[3]).toBe(3);
    expect(evidence.color.every(v=>Number.isFinite(v)&&v>0)).toBe(true);
    for(const [i,v] of [.13,.045,.008].entries())expect(Math.abs(evidence.color[i]-v)).toBeLessThan(2e-8);
});

test('black-hole material keeps SR and stellar hits correct through a real graphics-context recovery',async({page},info)=>{
    await harness(page);
    const before=await page.evaluate(async()=>{
        const {session,renderer}=window.sr,settings={autoQuality:false,feedbackEnabled:false};
        await session.command({type:'preset',preset:'black-hole'});window.__blackHoleSnapshot=session.snapshot();window.__blackHoleSettings=settings;
        const capture=[...renderer.readDiagnosticPixel(window.__blackHoleSnapshot,settings,0,0,5)];
        await session.command({type:'preset',preset:'compact-star'});const star=renderer.readPixelHit(session.snapshot(),settings);
        await session.command({type:'preset',preset:'baseline'});const flat=session.snapshot();flat.observer.position[1]=1;const sr=renderer.readPixelHit(flat,settings);
        renderer.readDiagnosticPixel(window.__blackHoleSnapshot,settings,0,0,5);
        window.__blackHoleLose=renderer.renderer.getContext().getExtension('WEBGL_lose_context');window.__blackHoleLose.loseContext();
        return {capture,star,sr,gpu:renderer.diagnostics.gpu};
    });
    hardware(before.gpu);await page.waitForFunction(()=>window.sr.renderer.contextLost);await page.evaluate(()=>window.__blackHoleLose.restoreContext());
    await page.waitForFunction(()=>!window.sr.renderer.contextLost);
    const after=await page.evaluate(()=>{const {renderer}=window.sr,a=JSON.stringify(window.__blackHoleSnapshot);return {capture:[...renderer.readDiagnosticPixel(window.__blackHoleSnapshot,window.__blackHoleSettings,0,0,5)],hit:renderer.readPixelHit(window.__blackHoleSnapshot,window.__blackHoleSettings),unchanged:a===JSON.stringify(window.__blackHoleSnapshot),gpu:renderer.diagnostics.gpu};});
    await evidenceFile(info,'black-hole-context-gpu.json',{before,after});hardware(after.gpu);
    expect(before.capture[0]).toBe(1);expect(before.star).not.toBeNull();expect(before.sr).not.toBeNull();expect(after.capture[0]).toBe(1);
    expect(after.unchanged).toBe(true);expect(after.hit).toBeNull();expect(Math.abs(before.capture[2]-after.capture[2])).toBeLessThan(.0001);
});

test('public black-hole experiment retains motion clocks and portable model boundaries',async({page},info)=>{
    await gotoAndReady(page);await openObserverWorkspace(page);await page.locator('[data-observer-panel-tab="experiments"]').click();
    await page.locator('[data-observer-action="experiment"][data-value="black-hole"]').click();
    await page.waitForFunction(()=>window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.spacetime?.kind==='schwarzschild-black-hole');
    const initial=await page.evaluate(()=>{const w=window.__FTD_DEV__.registry.get('observerWorkspace');return {position:w.snapshot.observer.position,entities:w.snapshot.entities.length,segments:w.snapshot.segments.length};});
    expect(initial.entities).toBe(0);expect(initial.segments).toBe(0);
    await page.getByRole('button',{name:'Release into free fall'}).click();
    await page.waitForFunction(start=>{const s=window.__FTD_DEV__.registry.get('observerWorkspace').snapshot;return s.spacetime.observerMode==='freefall'&&s.observer.position[2]<start-1e-5;},initial.position[2]);
    await page.getByRole('button',{name:'Hold at rest'}).click();
    const evidence=await page.evaluate(async()=>{
        const w=window.__FTD_DEV__.registry.get('observerWorkspace');await w.command({type:'pause'});const snapshot=w.snapshot;
        const text=w.storage.exportWorld(snapshot,w.settings),document=w.storage.importWorld(text);
        const rejected=await w.command({type:'observer',patch:{position:[0,1.6,0]}}),after=w.snapshot;
        const loaded=await w.command({type:'load',snapshot:document.snapshot});
        const corrupt=structuredClone(document.snapshot);corrupt.spacetime.rs*=2;const corruptLoad=await w.command({type:'load',snapshot:corrupt});
        const horizon=structuredClone(document.snapshot);horizon.observer.position=[0,1.6,horizon.spacetime.radius*.5];let importError='';
        try{const bad=JSON.parse(text);bad.snapshot=horizon;w.storage.importWorld(JSON.stringify(bad));}catch(e){importError=e.message;}
        return {snapshot,restored:document.snapshot,rejected,afterPosition:after.observer.position,loaded,corruptLoad,importError,gpu:w.renderer.diagnostics.gpu};
    });
    await evidenceFile(info,'black-hole-session-browser.json',evidence);hardware(evidence.gpu);
    expect(evidence.snapshot.spacetime.observerMode).toBe('guided');expect(evidence.snapshot.observer.velocity).toEqual([0,0,0]);expect(evidence.snapshot.observer.properTime).toBeGreaterThan(0);
    expect(evidence.restored.spacetime).toEqual(evidence.snapshot.spacetime);expect(evidence.loaded.ok).toBe(true);
    expect(evidence.rejected.ok).toBe(false);expect(evidence.afterPosition).toEqual(evidence.snapshot.observer.position);expect(evidence.corruptLoad.ok).toBe(false);expect(evidence.importError).toMatch(/exterior|horizon|boundary/i);
});

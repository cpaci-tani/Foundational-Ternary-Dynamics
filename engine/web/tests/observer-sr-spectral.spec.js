/* global window, process */
import {test, expect} from '@playwright/test';
import {writeFile} from 'node:fs/promises';

test('compiled shader uses one D=1 spectral baseline on every display path', async ({page}, info) => {
    await page.goto('/tests/observer-sr-harness.html');
    await page.waitForFunction(()=>window.sr?.renderer);
    const evidence = await page.evaluate(async () => {
        const {session, renderer} = window.sr;
        const {displayLineSpectrum, referenceLineWeights} = await import('/js/observer/spectrum.js');
        const snapshot = session.snapshot();
        snapshot.observer = {...snapshot.observer, position:[0,3,6],velocity:[0,0,0]};
        const settings={optical:true,feedbackEnabled:false,mirrorWorld:false,artisticShading:false,layers:Object.fromEntries(Array.from({length:0})),selectedId:'',autoQuality:false};
        renderer.resize(200,200);
        const failures=[], samples=[];
        for(const color of [[1,1,1],[1,0,0],[0,1,0],[0,0,1],[.2,.5,.8],[0,0,0]]) {
            const entity={...snapshot.entities[0],id:'line-test',shape:'sphere',size:[3,3,3],position:[0,3,0],rotation:[0,0,0],velocity:[0,0,0],color,originTime:0,spectral:'white',emission:.7,clockOffset:0};
            snapshot.entities=[entity];snapshot.segments=[{...entity,entityId:entity.id,start:-60,end:null}];
            for(const velocity of [[0,0,0],[0,0,.6],[0,0,-.6],[.4,.2,.1]])for(const observerV of [[0,0,0],[0,0,.3]]) {
                snapshot.observer.velocity=observerV;snapshot.segments[0].velocity=velocity;
                // Keep the present center close for receding and oblique cases.
                snapshot.time=0;
                const ray=window.sr.optics.observerRay(snapshot,settings);
                snapshot.segments[0].position=ray.origin.map((x,j)=>x+6*ray.direction[j]+6*velocity[j]);
                renderer.prepare(snapshot,settings);
                const cpu=renderer.pick(snapshot,settings,0,0);
                if(!cpu)throw new Error('Spectral fixture requires an interior surface hit.');
                for(const doppler of [false,true])for(const beaming of [false,true]) {
                    const options={...settings,doppler,beaming};
                    const gpu=renderer.readPixelLinearColor(snapshot,options,0,0);
                    const expected=displayLineSpectrum(referenceLineWeights(color),cpu.doppler,{doppler,beaming,emission:entity.emission});
                    const error=Math.max(...expected.map((x,i)=>Math.abs(x-gpu[i])/Math.max(1,Math.abs(x))));
                    samples.push({color,velocity,observerV,doppler,beaming,D:cpu.doppler,error});
                    if(!gpu.every(Number.isFinite)||!Number.isFinite(error)||error>3e-5)failures.push(samples.at(-1));
                }
            }
        }
        // Presentation sky and ground also share the same no-shift conversion.
        snapshot.entities=[];snapshot.segments=[];snapshot.observer.velocity=[0,0,0];
        const paths=[];
        for(const pitch of [.3,-.6]){
            snapshot.observer.pitch=pitch;
            const off=renderer.readPixelLinearColor(snapshot,{...settings,doppler:false});
            const on=renderer.readPixelLinearColor(snapshot,{...settings,doppler:true});
            paths.push({pitch,off,on});
            if(![...off,...on].every(Number.isFinite)||off.some((x,i)=>x!==on[i]))failures.push({path:pitch>0?'environment':'ground',off,on});
        }
        return {failures,samples: samples.length,maxRelativeError:Math.max(...samples.map(x=>x.error)),paths,gpu:renderer.diagnostics.gpu};
    });
    const path=info.outputPath('sr-spectral-evidence.json');await writeFile(path,JSON.stringify(evidence,null,2));
    await info.attach('spectral-evidence',{path,contentType:'application/json'});
    expect(evidence.failures).toEqual([]);
    expect(Number.isFinite(evidence.maxRelativeError)).toBe(true);
});

test('actual GPU Gaussian responses handle signed offsets in every production material',async({page,browser},info)=>{
    await page.goto('/tests/observer-sr-harness.html');
    await page.waitForFunction(()=>window.sr?.renderer);
    const evidence=await page.evaluate(()=>{
        const {renderer}=window.sr,gl=renderer.renderer.getContext();
        const centers=[610,545,455],widths=[35,30,25];
        const wavelengths=[...new Set([200,300,800,1220,...centers.flatMap((center,i)=>[-2,-1,0,1,2].map(offset=>center+offset*widths[i]))])].sort((a,b)=>a-b);
        const savedMaterial=renderer.mesh.material,savedTarget=renderer.renderer.getRenderTarget(),materials=[];
        try{
            for(const [name,production] of [['Minkowski',renderer.material],['compact-star',renderer.compactMaterial],['black-hole',renderer.blackHoleMaterial]]){
                // Compile the exact production spectrum function and surrounding
                // program. Only main is instrumented to expose its float output.
                const probe=production.clone();
                const instrumented=production.fragmentShader.replace(/void\s+main\s*\(\s*\)\s*\{/,'void productionMain(){');
                if(instrumented===production.fragmentShader)throw new Error('Production main was not instrumented');
                probe.fragmentShader=instrumented+'\nuniform float uSpectrumWavelength;\nvoid main(){fragColor=vec4(spectrum(uSpectrumWavelength),1.);}\n';
                probe.uniforms.uSpectrumWavelength={value:0};
                const rows=[];
                try{
                    renderer.mesh.material=probe;renderer.renderer.setRenderTarget(renderer.pickTarget);
                    for(const wavelength of wavelengths){
                        probe.uniforms.uSpectrumWavelength.value=wavelength;
                        renderer.renderer.render(renderer.scene,renderer.camera);
                        const pixel=new Float32Array(4);
                        renderer.renderer.readRenderTargetPixels(renderer.pickTarget,0,0,1,1,pixel);
                        const expected=centers.map((center,i)=>{const z=(wavelength-center)/widths[i];return Math.exp(-.5*z*z);});
                        rows.push({wavelength,offsets:centers.map((center,i)=>(wavelength-center)/widths[i]),gpu:[...pixel],expected,error:Math.max(...expected.map((value,i)=>Math.abs(value-pixel[i])))});
                    }
                    const program=gl.getParameter(gl.CURRENT_PROGRAM);
                    materials.push({name,rows,linked:gl.getProgramParameter(program,gl.LINK_STATUS),programLog:gl.getProgramInfoLog(program),glError:gl.getError()});
                }finally{renderer.mesh.material=savedMaterial;probe.dispose();}
            }
        }finally{renderer.mesh.material=savedMaterial;renderer.renderer.setRenderTarget(savedTarget);}
        return {materials,wavelengths,gpu:renderer.diagnostics.gpu};
    });
    const path=info.outputPath('observer-gaussian-gpu-evidence.json');
    await writeFile(path,JSON.stringify({...evidence,browser:browser.version()},null,2));
    await info.attach('production-Gaussian-GPU-evidence',{path,contentType:'application/json'});
    expect(evidence.gpu.floatReadback).toBe(true);
    expect(evidence.gpu.renderer).toBeTruthy();
    if(process.env.FTD_HARDWARE_WEBGL==='1')expect(evidence.gpu.renderer).not.toMatch(/swiftshader|llvmpipe|software|softpipe/i);
    for(const material of evidence.materials){
        expect(material.linked,material.name).toBe(true);expect(material.glError,material.name).toBe(0);
        for(const row of material.rows){
            expect(row.gpu.every(Number.isFinite),`${material.name} at ${row.wavelength} nm`).toBe(true);
            expect(row.gpu[3]).toBe(1);expect(Number.isFinite(row.error)).toBe(true);expect(row.error).toBeLessThan(3e-6);
        }
        for(let i=0;i<3;i++){
            expect(material.rows.some(row=>row.offsets[i]<0)).toBe(true);
            expect(material.rows.some(row=>row.offsets[i]>0)).toBe(true);
        }
    }
});

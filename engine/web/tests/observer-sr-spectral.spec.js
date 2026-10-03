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
                    if(error>3e-5)failures.push(samples.at(-1));
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
            if(off.some((x,i)=>x!==on[i]))failures.push({path:pitch>0?'environment':'ground',off,on});
        }
        return {failures,samples: samples.length,maxRelativeError:Math.max(...samples.map(x=>x.error)),paths,gpu:renderer.diagnostics.gpu};
    });
    const path=info.outputPath('sr-spectral-evidence.json');await writeFile(path,JSON.stringify(evidence,null,2));
    await info.attach('spectral-evidence',{path,contentType:'application/json'});
    expect(evidence.failures).toEqual([]);
});

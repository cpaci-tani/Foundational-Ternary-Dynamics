import {test,expect} from '@playwright/test';
test('D=1 color toggle preserves actual compiled display output', async ({page},info)=>{
    await page.goto('/tests/observer-sr-harness.html');await page.waitForFunction(()=>window.sr?.renderer);
    const evidence=await page.evaluate(()=>{
        const {session,renderer}=window.sr,snapshot=session.snapshot();
        snapshot.observer={...snapshot.observer,position:[0,3,6],velocity:[0,0,0]};
        const e={...snapshot.entities[0],position:[0,3,0],shape:'sphere',rotation:[0,0,0],size:[3,3,3],color:[1,1,1],velocity:[0,0,0],originTime:0};
        snapshot.entities=[e];snapshot.segments=[{...e,entityId:e.id,start:-60,end:null}];
        const colors=[];
        for(const doppler of [false,true]){
            renderer.prepare(snapshot,{doppler,beaming:false,feedbackEnabled:false,artisticShading:false,layers:{grid:false},selectedId:''});
            renderer.uniforms.uDebugMode.value=0;
            const pixels=new Float32Array(4);renderer.renderer.setRenderTarget(renderer.pickTarget);
            renderer.renderer.render(renderer.scene,renderer.camera);renderer.renderer.readRenderTargetPixels(renderer.pickTarget,0,0,1,1,pixels);
            renderer.renderer.setRenderTarget(null);colors.push([...pixels]);
        }
        return {colors,gpu:renderer.diagnostics.gpu};
    });
    await info.attach('D1-toggle-evidence',{body:JSON.stringify(evidence,null,2),contentType:'application/json'});
    expect(evidence.colors[0]).toEqual(evidence.colors[1]);
});

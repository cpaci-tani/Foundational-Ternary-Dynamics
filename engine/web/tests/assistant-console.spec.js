import {test,expect} from '@playwright/test';
import {gotoAndReady,openObserverWorkspace} from './_helpers.js';

async function ready(page){
    await gotoAndReady(page,{path:'/?engine=wasm&lattice=9'});
    await page.waitForFunction(()=>window.__FTD_DEV__.registry.get('assistant'));
}

test('JEV instructions use native browser fetch and handle approval or authentication failure',async({page})=>{
    const requests=[];let status=200;
    // Stub only the HTTP response. Keep the production JevClient and Window.fetch
    // intact so browser receiver checks execute on the normal command path.
    await page.route('**/api/ai/jev',async route=>{
        const request=route.request();
        requests.push({method:request.method(),authorization:request.headers().authorization,body:request.postDataJSON()});
        await route.fulfill({status,contentType:'application/json',body:JSON.stringify(status===200?{decision:'execute',confidence:1,model:'http-fixture'}:{error:'Unauthorized'})});
    });
    await ready(page);
    await page.waitForFunction(()=>window.__FTD_DEV__.registry.get('assistant').control.observe()?.capabilities.includes('lattice.step'));
    await page.evaluate(()=>{
        const ai=window.__FTD_DEV__.registry.get('assistant');
        ai.knowledge.search=async()=>[];
        ai.model.plan=async()=>({kind:'actions',message:'Pause and advance two ticks',actions:[{type:'lattice.pause',args:{}},{type:'lattice.step',args:{count:2}}]});
    });
    await page.getByRole('button',{name:'Open the JEV console',exact:true}).click();
    await page.getByText('Connection & local model',{exact:true}).click();
    await page.getByLabel('JEV API key').fill('http-fixture-key');
    await page.getByRole('button',{name:'Connect key',exact:true}).click();
    await page.locator('#jev-input').fill('Pause and advance exactly two ticks.');
    await page.getByRole('button',{name:'Send',exact:true}).click();
    await expect.poll(()=>page.evaluate(()=>{
        const ai=window.__FTD_DEV__.registry.get('assistant');
        return ai.service.transcript.some(row=>row.type==='receipt'||row.type==='error');
    })).toBe(true);
    const approved=await page.evaluate(()=>window.__FTD_DEV__.registry.get('assistant').service.transcript);
    expect(approved.filter(row=>row.type==='error')).toEqual([]);
    await expect(page.locator('.jev-messages')).toContainText('applied: lattice.step');
    expect(requests).toHaveLength(1);
    expect(requests[0].method).toBe('POST');expect(requests[0].authorization).toBe('Bearer http-fixture-key');
    expect(requests[0].body.observation.workspace).toBe('lattice');
    expect(requests[0].body.plan.actions.at(-1)).toEqual({type:'lattice.step',args:{count:2}});

    status=401;
    const rejected=await page.evaluate(async()=>{
        const ai=window.__FTD_DEV__.registry.get('assistant');
        ai.service.clear();const before=ai.control.observe();
        await ai.service.submit('Pause and advance exactly two ticks.');
        return{before,after:ai.control.observe(),events:ai.service.transcript};
    });
    expect(requests).toHaveLength(2);
    expect(rejected.events.filter(row=>row.type==='error').map(row=>row.text)).toEqual(['JEV key was rejected.']);
    expect(rejected.events.filter(row=>row.type==='receipt')).toEqual([]);
    expect(rejected.after.tick).toBe(rejected.before.tick);expect(rejected.after.ownerId).toBe(rejected.before.ownerId);
});
test('shared console moves with workspace, releases input, restores focus and never starts a second owner',async({page},testInfo)=>{
    await ready(page);
    const owner=await page.evaluate(()=>{window.__assistantOwner=window.__FTD_DEV__.registry.get('assistant').control.observe().ownerId;return window.__assistantOwner;});
    await page.getByRole('button',{name:'Open the JEV console',exact:true}).click();
    await expect(page.getByRole('dialog',{name:'JEV console'})).toBeVisible();
    await expect(page.locator('#jev-input')).toBeFocused();
    await expect(page.getByLabel('Live experiment (5 min / 50 actions)')).not.toBeChecked();
    await page.getByRole('button',{name:'Compare live observations',exact:true}).click();
    await expect(page.getByLabel('Live experiment (5 min / 50 actions)')).toBeChecked();
    expect(await page.evaluate(()=>window.__FTD_DEV__.registry.get('assistant').service.run)).toBeNull();
    await page.getByRole('button',{name:'Step 100 ticks',exact:true}).click();
    await expect(page.getByLabel('Live experiment (5 min / 50 actions)')).not.toBeChecked();
    await page.screenshot({path:testInfo.outputPath('jev-console.png')});
    await page.getByRole('button',{name:'Close JEV console'}).click();
    await openObserverWorkspace(page);
    await page.locator('#observer-workspace').getByRole('button',{name:/JEV/,exact:false}).click();
    await expect(page.locator('#observer-workspace .jev-console')).toBeVisible();
    expect(await page.evaluate(()=>document.pointerLockElement)).toBeNull();
    await page.getByRole('button',{name:'Close JEV console'}).click();
    await page.getByRole('button',{name:/Lattice Sim/}).click();
    await expect.poll(()=>page.evaluate(()=>window.__FTD_DEV__.registry.get('assistant').control.observe().ownerId)).toBe(owner);
    expect(await page.locator('.jev-console').count()).toBe(1);
});
test('service executes crosshair object changes through real worker receipts and rejects stale proposals',async({page})=>{
    await ready(page);await openObserverWorkspace(page);
    const result=await page.evaluate(async()=>{
        const reg=window.__FTD_DEV__.registry,w=reg.get('observerWorkspace'),ai=reg.get('assistant');
        await w.command({type:'pause'});w.selectedId=w.snapshot.entities[0].id;
        const before=w.snapshot.entities[0].mass;
        ai.knowledge.search=async()=>[];
        ai.jev.evaluate=async()=>({decision:'execute',confidence:1,model:'test-fixture'});
        ai.model.plan=async()=>({kind:'actions',message:'Double selected mass',actions:[{type:'observer.update',args:{massFactor:2}}]});
        await ai.service.submit('Make the selected object twice as heavy');
        const after=w.snapshot.entities[0].mass;
        ai.model.plan=async()=>{await w.authorCommand({type:'update',id:w.selectedId,patch:{name:'Manual edit'}});return{kind:'actions',message:'',actions:[{type:'observer.delete',args:{}}]};};
        await ai.service.submit('Delete the selected object');
        return{before,after,alive:w.snapshot.entities[0].alive,transcript:ai.service.transcript};
    });
    expect(result.after).toBe(result.before*2);expect(result.alive).toBe(true);
    expect(result.transcript.some(t=>t.type==='receipt'&&t.receipt.status==='applied')).toBe(true);
    expect(result.transcript.some(t=>t.type==='error'&&/changed/.test(t.text))).toBe(true);
});
test('public key is not written to browser storage or exported conversation',async({page})=>{
    await ready(page);await page.getByRole('button',{name:'Open the JEV console',exact:true}).click();
    await page.getByText('Connection & local model',{exact:true}).click();
    await page.getByLabel('JEV API key').fill('test-private-key');await page.getByRole('button',{name:'Connect key',exact:true}).click();
    await expect(page.getByLabel('JEV API key')).toHaveValue('');
    const data=await page.evaluate(()=>({local:JSON.stringify(localStorage),session:JSON.stringify(sessionStorage),transcript:JSON.stringify(window.__FTD_DEV__.registry.get('assistant').service.transcript)}));
    expect(JSON.stringify(data)).not.toContain('test-private-key');
});

test('conversational requests bypass the command filter and questions cannot pause playback',async({page})=>{
    await ready(page);await page.getByRole('button',{name:'Open the JEV console',exact:true}).click();
    await page.locator('#jev-input').fill('Hello');await page.getByRole('button',{name:'Send',exact:true}).click();
    const bounds=await page.locator('.jev-console').boundingBox();
    expect(bounds.y+bounds.height).toBeLessThanOrEqual(page.viewportSize().height);
    await expect(page.locator('.jev-messages')).toContainText('You can ask a question, give a command, or request a live experiment.');
    await expect(page.locator('.jev-messages')).toContainText('Download / load model');
    const result=await page.evaluate(async()=>{
        const ai=window.__FTD_DEV__.registry.get('assistant'),before=ai.control.observe();
        ai.knowledge.search=async()=>[];ai.model.explain=async()=> 'Explanation only; no command executed.';
        ai.model.generate=async()=>{throw new Error('Question attempted action generation');};
        ai.jev.evaluate=async()=>{throw new Error('Question attempted JEV mutation approval');};
        await ai.service.submit('Please explain how to pause and resume.');
        await ai.service.submit('Is the simulation paused?',{autonomous:true});
        return{before,after:ai.control.observe(),events:ai.service.transcript};
    });
    expect(result.events.filter(e=>e.type==='error')).toEqual([]);
    expect(result.events.filter(e=>e.type==='receipt')).toEqual([]);
    expect(result.after.ownerId).toBe(result.before.ownerId);
    expect(result.after.preparationVersion).toBe(result.before.preparationVersion);
    expect(result.after.facts.running).toBe(result.before.facts.running);
});

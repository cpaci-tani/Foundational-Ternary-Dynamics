/* global window, document, Event */
import {test,expect} from '@playwright/test';
import {gotoAndReady,openObserverWorkspace} from './_helpers.js';

async function ready(page){
    await gotoAndReady(page,{path:'/?engine=wasm&lattice=9'});
    await openObserverWorkspace(page);
    await page.waitForFunction(()=>!!window.__FTD_DEV__.registry.get('observerWorkspace')?.snapshot);
    await page.evaluate(async()=>{
        const w=window.__FTD_DEV__.registry.get('observerWorkspace');
        w.setSetting('renderScale',.25);w.setSetting('autoQuality',false);
        await w.command({type:'pause'});
    });
}

async function playback(page,playing){
    await expect.poll(()=>page.evaluate(()=>window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.playing)).toBe(playing);
}

test('JEV starts hidden and its controls preference survives a portable-world round trip',async({page})=>{
    await ready(page);
    await expect(page.locator('[data-observer-assistant]')).toBeHidden();
    await page.locator('[data-observer-panel-tab="help"]').click();
    await expect(page.getByLabel('Show JEV button',{exact:true})).not.toBeChecked();
    await page.getByLabel('Show JEV button',{exact:true}).check();
    await expect(page.locator('[data-observer-assistant]')).toBeVisible();
    const setting=await page.evaluate(()=>{
        const w=window.__FTD_DEV__.registry.get('observerWorkspace');
        const restored=w.storage.importWorld(w.storage.exportWorld(w.snapshot,w.settings));
        return {current:w.settings.showAssistant,restored:restored.settings.showAssistant};
    });
    expect(setting).toEqual({current:true,restored:true});
    await page.getByLabel('Show JEV button',{exact:true}).uncheck();
    await expect(page.locator('[data-observer-assistant]')).toBeHidden();
});

test('live telemetry owns no playback pause and drawer switching releases only inspection pauses',async({page})=>{
    await ready(page);
    expect(await page.evaluate(()=>window.__FTD_DEV__.registry.get('observerWorkspace').settings.pauseOnInspect)).toBe(true);
    await page.getByRole('button',{name:'Play',exact:true}).click();await playback(page,true);
    await page.locator('[data-observer-panel-tab="telemetry"]').click();
    await expect(page.locator('[data-observer-panel="telemetry"]')).toBeVisible();
    await playback(page,true);
    expect(await page.evaluate(()=>document.pointerLockElement)).toBeNull();
    const clock=page.locator('[data-observer-telemetry="coordinate-time"]');
    const clockBefore=await clock.textContent();
    const before=await page.evaluate(()=>window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.time);
    await page.waitForFunction(t=>window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.time>t+.08,before);
    await expect(clock).not.toHaveText(clockBefore);
    await page.locator('[data-observer-panel-tab="camera"]').click();await playback(page,false);
    await page.locator('[data-observer-panel-tab="telemetry"]').click();await playback(page,true);
    await page.evaluate(async()=>{await window.__FTD_DEV__.registry.get('observerWorkspace').command({type:'pause'});});
    await playback(page,false);
    const paused=await page.evaluate(()=>window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.time);
    await page.getByRole('button',{name:'Close controls',exact:true}).click();
    await page.waitForTimeout(300);
    expect(await page.evaluate(()=>{const s=window.__FTD_DEV__.registry.get('observerWorkspace').snapshot;return {playing:s.playing,time:s.time};})).toEqual({playing:false,time:paused});
});

test('live telemetry permits captured navigation while category keyboard controls suppress movement',async({page})=>{
    await ready(page);
    await page.getByRole('button',{name:'Play',exact:true}).click();await playback(page,true);
    await page.locator('[data-observer-panel-tab="telemetry"]').click();
    const panel=page.locator('[data-observer-panel="telemetry"]');
    const speed=page.locator('[data-observer-telemetry="speed"]');
    const before=await page.evaluate(()=>{
        const w=window.__FTD_DEV__.registry.get('observerWorkspace');
        return {position:w.snapshot.observer.position,forward:w.settings.bindings.forward};
    });
    const speedBefore=await speed.textContent();
    await page.locator('.observer-canvas').click({position:{x:280,y:360}});
    await expect.poll(()=>page.evaluate(()=>document.pointerLockElement===window.__FTD_DEV__.registry.get('observerWorkspace').renderer.canvas)).toBe(true);
    try{
        await page.keyboard.down(before.forward);
        await page.waitForFunction(z=>window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.observer.position[2]<z-1e-5,before.position[2]);
        await expect(speed).not.toHaveText(speedBefore);
        await expect(panel).toBeVisible();await playback(page,true);
    }finally{
        await page.keyboard.up(before.forward);
        await page.evaluate(()=>{if(document.pointerLockElement)document.exitPointerLock();});
    }
    await expect.poll(()=>page.evaluate(()=>document.pointerLockElement)).toBeNull();
    await expect(panel).toBeVisible();
    const group=panel.locator('[data-observer-telemetry-section="model"]');
    const wasOpen=await group.getAttribute('open')!==null;
    await group.locator('summary').focus();
    await page.keyboard.press('Space');
    await expect.poll(async()=>await group.getAttribute('open')!==null).toBe(!wasOpen);
    expect(await page.evaluate(()=>{
        const input=window.__FTD_DEV__.registry.get('observerWorkspace').input;
        return {keys:input.keys.size,move:input.sample().move};
    })).toEqual({keys:0,move:[0,0,0]});
    await expect(panel).toBeVisible();await playback(page,true);
});

test('delivered document visibility changes pause and resume a playing world with live telemetry open',async({page},info)=>{
    await ready(page);
    await page.getByRole('button',{name:'Play',exact:true}).click();await playback(page,true);
    await page.locator('[data-observer-panel-tab="telemetry"]').click();
    await playback(page,true);
    info.annotations.push({type:'visibility-path',description:'Controlled document.hidden and delivered visibilitychange exercise the production listeners with the live telemetry drawer open.'});
    await page.evaluate(()=>{
        window.__observerTelemetryHiddenDescriptor=Object.getOwnPropertyDescriptor(document,'hidden');
        window.__observerTelemetryHidden=true;
        Object.defineProperty(document,'hidden',{configurable:true,get:()=>window.__observerTelemetryHidden});
        document.dispatchEvent(new Event('visibilitychange'));
    });
    try{
        await playback(page,false);
        const paused=await page.evaluate(()=>{
            const w=window.__FTD_DEV__.registry.get('observerWorkspace');
            return {time:w.snapshot.time,tick:w.snapshot.tick,keys:w.input.keys.size};
        });
        expect(paused.keys).toBe(0);
        await page.waitForTimeout(300);
        expect(await page.evaluate(()=>{
            const w=window.__FTD_DEV__.registry.get('observerWorkspace');
            return {time:w.snapshot.time,tick:w.snapshot.tick,keys:w.input.keys.size};
        })).toEqual(paused);
        await page.evaluate(()=>{
            window.__observerTelemetryHidden=false;
            document.dispatchEvent(new Event('visibilitychange'));
        });
        await playback(page,true);
        await expect(page.locator('[data-observer-panel="telemetry"]')).toBeVisible();
        await page.waitForFunction(t=>window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.time>t+.08,paused.time);
    }finally{
        await page.evaluate(()=>{
            const descriptor=window.__observerTelemetryHiddenDescriptor;
            if(descriptor)Object.defineProperty(document,'hidden',descriptor);else delete document.hidden;
            delete window.__observerTelemetryHidden;delete window.__observerTelemetryHiddenDescriptor;
            document.dispatchEvent(new Event('visibilitychange'));
        });
    }
});

test('hiding the interface closes JEV and releases its input ownership without pausing',async({page})=>{
    await ready(page);
    await page.getByRole('button',{name:'Play',exact:true}).click();await playback(page,true);
    await page.locator('[data-observer-panel-tab="help"]').click();
    await page.getByLabel('Show JEV button',{exact:true}).check();
    await page.getByRole('button',{name:'Close controls',exact:true}).click();await playback(page,true);
    await page.locator('[data-observer-assistant]').click();
    await expect(page.locator('#observer-workspace .jev-console')).toBeVisible();
    expect(await page.evaluate(()=>{
        const w=window.__FTD_DEV__.registry.get('observerWorkspace');
        return {assistant:w.assistantOpen,blocked:w.input.panelOpen,playing:w.snapshot.playing};
    })).toEqual({assistant:true,blocked:true,playing:true});
    await page.getByRole('button',{name:'Hide interface',exact:true}).click();
    await expect(page.locator('#observer-workspace .jev-console')).toBeHidden();
    await expect.poll(()=>page.evaluate(()=>{
        const w=window.__FTD_DEV__.registry.get('observerWorkspace');
        return {assistant:w.assistantOpen,blocked:w.input.panelOpen,playing:w.snapshot.playing};
    })).toEqual({assistant:false,blocked:false,playing:true});
    await page.getByRole('button',{name:'Show interface',exact:true}).click();
    await expect(page.locator('[data-observer-assistant]')).toBeVisible();
    await expect(page.locator('#observer-workspace .jev-console')).toBeHidden();
    await playback(page,true);
});

test('minimal interface toggle hides scene UI without pausing and hidden shortcuts restore controls',async({page})=>{
    await ready(page);
    await page.getByRole('button',{name:'Play',exact:true}).click();await playback(page,true);
    await page.locator('[data-observer-panel-tab="telemetry"]').click();
    await page.getByRole('button',{name:'Hide interface',exact:true}).click();
    const workspace=page.locator('#observer-workspace');
    await expect(workspace).toHaveClass(/observer-interface-hidden/);
    await expect(page.locator('[data-observer-ui-toggle]')).toHaveAttribute('aria-pressed','true');
    await expect(page.getByRole('button',{name:'Show interface',exact:true})).toBeVisible();
    await expect(page.locator('.observer-head')).toBeHidden();
    await expect(page.locator('.observer-hud')).toBeHidden();
    await expect(page.locator('.observer-toolbar')).toBeHidden();
    await expect(page.locator('[data-observer-panel]')).toBeHidden();
    await expect(page.locator('.observer-reticle')).toBeHidden();
    await expect(page.locator('.observer-spatial-labels')).toBeHidden();
    await playback(page,true);
    const before=await page.evaluate(()=>window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.time);
    await page.waitForFunction(t=>window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.time>t+.08,before);
    await page.getByRole('button',{name:'Show interface',exact:true}).click();
    await expect(workspace).not.toHaveClass(/observer-interface-hidden/);
    await expect(page.locator('.observer-toolbar')).toBeVisible();
    await playback(page,true);
    await page.getByRole('button',{name:'Hide interface',exact:true}).click();
    await page.locator('.observer-canvas').focus();
    const key=await page.evaluate(()=>window.__FTD_DEV__.registry.get('observerWorkspace').settings.bindings.world);
    await page.keyboard.press(key);
    await expect(workspace).not.toHaveClass(/observer-interface-hidden/);
    await expect(page.locator('[data-observer-panel="world"]')).toBeVisible();
    await playback(page,false);
    await page.getByRole('button',{name:'Close controls',exact:true}).click();await playback(page,true);
});

for(const viewport of [{width:1440,height:900},{width:390,height:844}]){
    test(`telemetry categories remain reachable at ${viewport.width}×${viewport.height}`,async({page})=>{
        await page.setViewportSize(viewport);await ready(page);
        await page.locator('[data-observer-panel-tab="telemetry"]').click();
        const panel=page.locator('[data-observer-panel="telemetry"]');
        await expect(panel).toBeVisible();
        expect(await panel.locator('[data-observer-telemetry-section]').count()).toBeGreaterThanOrEqual(6);
        expect(await panel.locator('[data-observer-telemetry]').count()).toBeGreaterThanOrEqual(40);
        const last=panel.locator('[data-observer-telemetry-section]').last();
        const summary=last.locator('summary');await summary.scrollIntoViewIfNeeded();
        if(await last.getAttribute('open')===null)await summary.click();
        await expect(last.locator('[data-observer-telemetry]').first()).toBeVisible();
        const bounds=await panel.boundingBox();expect(bounds).not.toBeNull();
        expect(bounds.x).toBeGreaterThanOrEqual(0);expect(bounds.y).toBeGreaterThanOrEqual(0);
        expect(bounds.x+bounds.width).toBeLessThanOrEqual(viewport.width+1);
        expect(bounds.y+bounds.height).toBeLessThanOrEqual(viewport.height+1);
        const toggle=await page.locator('[data-observer-ui-toggle]').boundingBox();expect(toggle).not.toBeNull();
        expect(toggle.x).toBeGreaterThanOrEqual(0);expect(toggle.x+toggle.width).toBeLessThanOrEqual(viewport.width+1);
    });
}

test('floating compact star exposes physical GR quantities and clears received-light readings on a miss',async({page})=>{
    await ready(page);
    await page.locator('[data-observer-panel-tab="experiments"]').click();
    await page.locator('[data-observer-action="experiment"][data-value="compact-star"]').click();
    await page.evaluate(async()=>{
        const w=window.__FTD_DEV__.registry.get('observerWorkspace');
        await w.command({type:'pause'});
    });
    await page.locator('[data-observer-panel-tab="telemetry"]').click();
    const field=id=>page.locator(`[data-observer-telemetry="${id}"]`);
    expect(await page.evaluate(()=>window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.entities.map(e=>e.shape))).toEqual(['sphere']);
    await expect(field('star-mass-solar')).toContainText('1.4');
    await expect(field('stellar-radius')).toContainText('12,000');
    await expect(field('stellar-radius')).toContainText('m');
    await expect(field('schwarzschild-radius')).toContainText('4,134.55');
    await expect(field('surface-redshift')).toContainText('0.235');
    await expect(field('clock-rate')).toContainText('0.985');
    await expect(field('length-scale')).toContainText('48,000');
    await expect(field('frequency-ratio')).not.toHaveText('—');
    const prior=await field('frequency-ratio').textContent();
    expect(prior).toMatch(/0\.82/);
    await page.evaluate(()=>{
        const w=window.__FTD_DEV__.registry.get('observerWorkspace');
        w.input.setPose({yaw:0,pitch:1.4,roll:0});
    });
    await expect(field('frequency-ratio')).toHaveText('—');
    await expect(field('source-clock')).toHaveText('—');
    await expect(field('optical-delay')).toHaveText('—');
    await expect(field('optical-status')).toHaveText('No current source hit');
    await expect(field('receiver-lapse')).not.toHaveText('—');
    expect(await page.evaluate(()=>window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.playing)).toBe(false);
});

/* global window, document, Event */
import {test,expect} from '@playwright/test';
import {writeFile} from 'node:fs/promises';
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
    const panelBounds=await panel.boundingBox();
    const canvasBounds=await page.locator('.observer-canvas').boundingBox();
    const toolbarBounds=await page.locator('.observer-toolbar').boundingBox();
    expect(panelBounds).not.toBeNull();expect(canvasBounds).not.toBeNull();expect(toolbarBounds).not.toBeNull();
    const sceneTop=panelBounds.y+panelBounds.height;
    expect(toolbarBounds.y-sceneTop).toBeGreaterThan(12);
    const capturePoint=await page.locator('.observer-canvas').evaluate((canvas,{top,bottom})=>{
        const bounds=canvas.getBoundingClientRect();
        for(const fraction of [.75,.25,.5])for(const y of [(top+bottom)/2,top+6,bottom-6]){
            const x=bounds.left+bounds.width*fraction;
            if(document.elementFromPoint(x,y)===canvas)return {x:x-bounds.left,y:y-bounds.top};
        }
        return null;
    },{top:sceneTop,bottom:toolbarBounds.y});
    expect(capturePoint,'The dropdown leaves a directly clickable scene area').not.toBeNull();
    await page.locator('.observer-canvas').click({position:capturePoint});
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

test('telemetry drops down into a wide category grid with readable values and restores narrow controls',async({page},info)=>{
    await page.setViewportSize({width:1440,height:900});await ready(page);
    for(const viewport of [{width:1440,height:900},{width:1920,height:1080}]){
        await page.setViewportSize(viewport);
        await page.locator('[data-observer-panel-tab="telemetry"]').click();
        const panel=page.locator('[data-observer-panel="telemetry"]');
        await expect(panel).toBeVisible();
        expect(await panel.locator('[data-observer-telemetry-section]').count()).toBe(9);
        expect(await panel.locator('[data-observer-telemetry]').count()).toBe(123);
        const bounds=await panel.boundingBox();expect(bounds).not.toBeNull();
        expect(bounds.width).toBeGreaterThanOrEqual(1000);
        expect(bounds.width).toBeGreaterThan(bounds.height*1.4);
        expect(Math.abs(bounds.x+bounds.width/2-viewport.width/2)).toBeLessThanOrEqual(2);
        expect(bounds.y).toBeLessThan(viewport.height*.2);
        const columns=await panel.locator('[data-observer-telemetry-section]').evaluateAll(groups=>
            new Set(groups.map(group=>Math.round(group.getBoundingClientRect().left))).size);
        expect(columns).toBeGreaterThanOrEqual(3);
        const contrast=await panel.locator('[data-observer-telemetry="coordinate-time"]').evaluate(output=>{
            const card=output.closest('.observer-telemetry-reading');
            const label=card.querySelector('dt');
            const value=output.firstElementChild??output;
            const style=element=>window.getComputedStyle(element);
            const rgb=color=>color.match(/[\d.]+/g).map(Number);
            const luminance=color=>rgb(color).slice(0,3).map(channel=>{
                const normalized=channel/255;
                return normalized<=.04045?normalized/12.92:((normalized+.055)/1.055)**2.4;
            }).reduce((sum,channel,index)=>sum+channel*[.2126,.7152,.0722][index],0);
            const background=style(card).backgroundColor;
            const bg=luminance(background),v=luminance(style(value).color),l=luminance(style(label).color);
            const ratio=foreground=>(Math.max(foreground,bg)+.05)/(Math.min(foreground,bg)+.05);
            return {alpha:rgb(background)[3]??1,value:ratio(v),label:ratio(l),valueLuminance:v,labelLuminance:l,
                valueSize:parseFloat(style(value).fontSize),labelSize:parseFloat(style(label).fontSize)};
        });
        expect(contrast.alpha).toBe(1);
        expect(contrast.value).toBeGreaterThanOrEqual(4.5);
        expect(contrast.label).toBeGreaterThanOrEqual(4.5);
        expect(contrast.valueLuminance).toBeGreaterThan(contrast.labelLuminance);
        expect(contrast.valueSize).toBeGreaterThan(contrast.labelSize);
        await page.screenshot({path:info.outputPath(`telemetry-landscape-${viewport.width}.png`)});
        await page.locator('[data-observer-panel-tab="camera"]').click();
        const controls=page.locator('[data-observer-panel="camera"]');
        await expect(controls).toBeVisible();
        const narrow=await controls.boundingBox();expect(narrow).not.toBeNull();
        expect(narrow.width).toBeLessThanOrEqual(460);
        expect(viewport.width-narrow.x-narrow.width).toBeLessThanOrEqual(32);
        await page.getByRole('button',{name:'Close controls',exact:true}).click();
    }
    await page.locator('[data-observer-panel-tab="telemetry"]').click();
    await page.getByRole('button',{name:'Close controls',exact:true}).click();
    await expect(page.locator('[data-observer-panel="telemetry"]')).toBeHidden();
    await expect(page.locator('[data-observer-panel-tab="telemetry"]')).toHaveAttribute('aria-expanded','false');
    await page.locator('[data-observer-panel-tab="world"]').click();
    expect((await page.locator('[data-observer-panel="world"]').boundingBox()).width).toBeLessThanOrEqual(460);
});

test('telemetry categories and controls remain reachable on mobile and a short desktop without horizontal overflow',async({page},info)=>{
    await page.setViewportSize({width:390,height:844});await ready(page);
    for(const viewport of [{width:390,height:844},{width:1280,height:600}]){
        await page.setViewportSize(viewport);
        await page.locator('[data-observer-panel-tab="telemetry"]').click();
        const panel=page.locator('[data-observer-panel="telemetry"]');
        await expect(panel).toBeVisible();
        expect(await panel.locator('[data-observer-telemetry-section]').count()).toBe(9);
        expect(await panel.locator('[data-observer-telemetry]').count()).toBe(123);
        const bounds=await panel.boundingBox();expect(bounds).not.toBeNull();
        expect(bounds.x).toBeGreaterThanOrEqual(0);expect(bounds.y).toBeGreaterThanOrEqual(0);
        expect(bounds.x+bounds.width).toBeLessThanOrEqual(viewport.width+1);
        expect(bounds.y+bounds.height).toBeLessThanOrEqual(viewport.height+1);
        expect(await panel.evaluate(element=>{
            const body=element.querySelector('.observer-panel-body');
            return {panel:element.scrollWidth-element.clientWidth,body:body.scrollWidth-body.clientWidth,
                document:document.documentElement.scrollWidth-window.innerWidth};
        })).toEqual({panel:0,body:0,document:0});
        const groups=panel.locator('[data-observer-telemetry-section]');
        for(let i=0;i<await groups.count();i++){
            const group=groups.nth(i),summary=group.locator('summary');
            await summary.scrollIntoViewIfNeeded();await expect(summary).toBeInViewport();
            if(await group.getAttribute('open')===null)await summary.click();
            const last=group.locator('[data-observer-telemetry]').last();
            await last.scrollIntoViewIfNeeded();await expect(last).toBeInViewport();
        }
        await expect(page.getByRole('button',{name:'Close controls',exact:true})).toBeInViewport();
        await expect(page.getByRole('button',{name:'Hide interface',exact:true})).toBeInViewport();
        await panel.locator('[data-observer-telemetry-section]').first().locator('summary').scrollIntoViewIfNeeded();
        await page.screenshot({path:info.outputPath(`telemetry-responsive-${viewport.width}x${viewport.height}.png`)});
        await page.getByRole('button',{name:'Close controls',exact:true}).click();
        await expect(panel).toBeHidden();
        await page.getByRole('button',{name:'Hide interface',exact:true}).click();
        await expect(page.getByRole('button',{name:'Show interface',exact:true})).toBeInViewport();
        await page.getByRole('button',{name:'Show interface',exact:true}).click();
    }
});

test('floating compact star exposes physical GR quantities and clears received-light readings on a miss',async({page},info)=>{
    await page.setViewportSize({width:1440,height:900});await ready(page);
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
    await page.screenshot({path:info.outputPath('telemetry-compact-star-landscape.png')});
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

async function denseTelemetryLayout(page){
    return page.locator('[data-observer-panel="telemetry"]').evaluate(panel=>{
        const body=panel.querySelector('.observer-panel-body'),bodyBounds=body.getBoundingClientRect();
        const rect=node=>{const r=node.getBoundingClientRect();return {x:r.x,y:r.y,width:r.width,height:r.height};};
        const rgb=color=>color.match(/[\d.]+/g).map(Number);
        const luminance=color=>rgb(color).slice(0,3).map(channel=>{
            const normalized=channel/255;
            return normalized<=.04045?normalized/12.92:((normalized+.055)/1.055)**2.4;
        }).reduce((sum,channel,index)=>sum+channel*[.2126,.7152,.0722][index],0);
        const rows=[...panel.querySelectorAll('[data-observer-telemetry]')].map(output=>{
            const card=output.closest('.observer-telemetry-reading'),bounds=card.getBoundingClientRect();
            const label=card.querySelector('dt'),value=output.firstElementChild??output;
            const unit=output.children.length>1?output.lastElementChild:null;
            const background=window.getComputedStyle(card).backgroundColor,bg=luminance(background);
            const text=node=>{
                const range=document.createRange();range.selectNodeContents(node);
                return [...range.getClientRects()].filter(r=>r.width>0&&r.height>0).map(r=>({left:r.left,right:r.right,top:r.top,bottom:r.bottom}));
            };
            const measurement=node=>{
                if(!node||!node.textContent.trim())return null;
                const style=window.getComputedStyle(node),lum=luminance(style.color),boxes=text(node);
                return {fontSize:parseFloat(style.fontSize),contrast:(Math.max(lum,bg)+.05)/(Math.min(lum,bg)+.05),
                    clipped:boxes.some(r=>r.left<bounds.left-1||r.right>bounds.right+1||r.top<bounds.top-1||r.bottom>bounds.bottom+1)};
            };
            const top=bounds.top-bodyBounds.top+body.scrollTop,bottom=bounds.bottom-bodyBounds.top+body.scrollTop;
            return {id:output.dataset.observerTelemetry,text:output.textContent,numeric:Number.isFinite(Number(value.textContent.replaceAll(',','').trim())),
                label:measurement(label),value:measurement(value),unit:measurement(unit),backgroundAlpha:rgb(background)[3]??1,
                fullyVisible:bounds.top>=bodyBounds.top&&bounds.bottom<=bodyBounds.bottom&&bounds.left>=bodyBounds.left&&bounds.right<=bodyBounds.right,
                reachable:bounds.width>0&&bounds.height>0&&bounds.left>=bodyBounds.left-1&&bounds.right<=bodyBounds.right+1&&top>=-1&&bottom<=body.scrollHeight+1};
        });
        return {viewport:{width:window.innerWidth,height:window.innerHeight},deviceScaleFactor:window.devicePixelRatio,
            panel:rect(panel),body:rect(body),rowCount:rows.length,fullyVisibleRows:rows.filter(row=>row.fullyVisible).length,
            overflow:{panel:panel.scrollWidth-panel.clientWidth,body:body.scrollWidth-body.clientWidth,document:document.documentElement.scrollWidth-window.innerWidth},
            rows,gpu:window.__FTD_DEV__.registry.get('observerWorkspace').renderer.diagnostics.gpu};
    });
}

function expectDenseTelemetry(layout,viewport,rowCount){
    expect(layout.viewport).toEqual(viewport);
    expect(layout.panel.x).toBeGreaterThanOrEqual(0);expect(layout.panel.y).toBeGreaterThanOrEqual(0);
    expect(layout.panel.x+layout.panel.width).toBeLessThanOrEqual(viewport.width+1);
    expect(layout.panel.y+layout.panel.height).toBeLessThanOrEqual(viewport.height+1);
    expect(layout.overflow).toEqual({panel:0,body:0,document:0});
    expect(layout.rowCount).toBe(rowCount);
    expect(new Set(layout.rows.map(row=>row.id)).size).toBe(rowCount);
    expect(layout.rows.filter(row=>!row.reachable),`Unreachable readings at ${viewport.width}×${viewport.height}`).toEqual([]);
    const clipped=layout.rows.filter(row=>[row.value,row.label,row.unit].some(text=>text?.clipped));
    expect(clipped,`Clipped text at ${viewport.width}×${viewport.height}`).toEqual([]);
    for(const row of layout.rows){
        expect(row.backgroundAlpha,`${row.id} background`).toBe(1);
        expect(row.label.fontSize,`${row.id} label`).toBeGreaterThanOrEqual(11);
        expect(row.label.contrast,`${row.id} label contrast`).toBeGreaterThanOrEqual(4.5);
        expect(row.value.contrast,`${row.id} value contrast`).toBeGreaterThanOrEqual(4.5);
        if(row.numeric)expect(row.value.fontSize,`${row.id} numeric value`).toBeGreaterThanOrEqual(15);
        if(row.unit){
            expect(row.unit.fontSize,`${row.id} unit`).toBeGreaterThanOrEqual(11);
            expect(row.unit.contrast,`${row.id} unit contrast`).toBeGreaterThanOrEqual(4.5);
        }
    }
}

async function expectExposedTelemetryControls(page){
    const transport=await page.locator('.observer-transport button').evaluateAll(buttons=>buttons.map(button=>{
        const bounds=button.getBoundingClientRect(),hit=document.elementFromPoint(bounds.left+bounds.width/2,bounds.top+bounds.height/2);
        return {label:button.textContent,visible:bounds.width>0&&bounds.height>0,
            exposed:hit===button||button.contains(hit)};
    }));
    expect(transport.length).toBeGreaterThanOrEqual(3);
    expect(transport.filter(button=>!button.visible||!button.exposed),'Transport buttons remain directly clickable below the dropdown').toEqual([]);
    for(const name of ['Close controls','Hide interface']){
        const button=page.getByRole('button',{name,exact:true});
        await expect(button).toBeInViewport();
        expect(await button.evaluate(element=>{
            const bounds=element.getBoundingClientRect(),hit=document.elementFromPoint(bounds.left+bounds.width/2,bounds.top+bounds.height/2);
            return hit===element||element.contains(hit);
        }),`${name} remains directly clickable`).toBe(true);
    }
}

for(const deviceScaleFactor of [1,2,3]){
    test.describe(`dense telemetry at device scale ${deviceScaleFactor}`,()=>{
        test.use({deviceScaleFactor});
        test('numbers, units and labels stay unclipped and all readings remain usable across viewport sizes',async({page},info)=>{
            test.setTimeout(120_000);
            await page.setViewportSize({width:1920,height:1080});await ready(page);
            const evidence=[];
            for(const viewport of [{width:1920,height:1080},{width:1440,height:900},{width:390,height:844},{width:1280,height:600},{width:800,height:480}]){
                await page.setViewportSize(viewport);
                await page.locator('[data-observer-panel-tab="telemetry"]').click();
                const panel=page.locator('[data-observer-panel="telemetry"]');await expect(panel).toBeVisible();
                const layout=await denseTelemetryLayout(page);evidence.push(layout);
                await writeFile(info.outputPath(`dense-telemetry-dpr${deviceScaleFactor}.json`),JSON.stringify(evidence,null,2));
                expect(layout.deviceScaleFactor).toBe(deviceScaleFactor);
                expectDenseTelemetry(layout,viewport,123);
                await expectExposedTelemetryControls(page);
                const groups=panel.locator('[data-observer-telemetry-section]');
                for(let i=0;i<await groups.count();i++){
                    const output=groups.nth(i).locator('[data-observer-telemetry]').last();
                    await output.scrollIntoViewIfNeeded();await expect(output).toBeInViewport();
                }
                const summary=panel.locator('[data-observer-telemetry-section="model"] summary');
                await summary.scrollIntoViewIfNeeded();await summary.focus();
                await expect(page.getByRole('tooltip')).toBeVisible();await page.keyboard.press('Escape');
                await expect(page.getByRole('tooltip')).toBeHidden();await expect(panel).toBeVisible();
                await page.keyboard.press('Space');
                await expect(panel.locator('[data-observer-telemetry-section="model"]')).not.toHaveAttribute('open');
                await page.keyboard.press('Space');
                await expect(panel.locator('[data-observer-telemetry-section="model"]')).toHaveAttribute('open','');
                const gamma=panel.locator('[data-observer-telemetry="gamma"]');
                await gamma.scrollIntoViewIfNeeded();await gamma.focus();
                const tooltip=page.getByRole('tooltip');await expect(tooltip).toBeVisible();await expect(tooltip).toContainText('Lorentz factor');
                const explanation=await tooltip.boundingBox();expect(explanation).not.toBeNull();
                expect(explanation.x).toBeGreaterThanOrEqual(0);expect(explanation.y).toBeGreaterThanOrEqual(0);
                expect(explanation.x+explanation.width).toBeLessThanOrEqual(viewport.width+1);
                expect(explanation.y+explanation.height).toBeLessThanOrEqual(viewport.height+1);
                await page.keyboard.press('Escape');await expect(tooltip).toBeHidden();await expect(panel).toBeVisible();
                await summary.scrollIntoViewIfNeeded();
                await page.screenshot({path:info.outputPath(`dense-telemetry-${viewport.width}x${viewport.height}-dpr${deviceScaleFactor}.png`)});
                await expect(page.getByRole('button',{name:'Close controls',exact:true})).toBeInViewport();
                await page.getByRole('button',{name:'Close controls',exact:true}).click();
            }
            await writeFile(info.outputPath(`dense-telemetry-dpr${deviceScaleFactor}.json`),JSON.stringify(evidence,null,2));
        });
    });
}

test.describe('populated GR telemetry at device scale 2',()=>{
    test.use({deviceScaleFactor:2});
    for(const preset of ['compact-star','black-hole']){
        test(`${preset} keeps physical readings, units and explanation mathematics readable`,async({page},info)=>{
            test.setTimeout(120_000);
            await page.setViewportSize({width:1440,height:900});await ready(page);
            await page.locator('[data-observer-panel-tab="experiments"]').click();
            await page.locator(`[data-observer-action="experiment"][data-value="${preset}"]`).click();
            await page.evaluate(async()=>{await window.__FTD_DEV__.registry.get('observerWorkspace').command({type:'pause'});});
            const evidence=[];
            for(const viewport of [{width:1440,height:900},{width:390,height:844}]){
                await page.setViewportSize(viewport);
                await page.locator('[data-observer-panel-tab="telemetry"]').click();
                const panel=page.locator('[data-observer-panel="telemetry"]');await expect(panel).toBeVisible();
                const field=id=>panel.locator(`[data-observer-telemetry="${id}"]`);
                await expect(field('star-mass-solar')).toContainText(preset==='compact-star'?'1.4':'10');
                await expect(field('star-mass-solar')).toContainText('GM☉ᴺ');
                await expect(field('star-gm')).toContainText('m³/s²');
                await expect(field('star-mass-kg')).toContainText('kg');
                await expect(field('hover-acceleration')).toContainText('m/s²');
                await expect(field('kretschmann')).toContainText('m⁻⁴');
                await expect(field('speed-si')).toContainText('m/s');
                const layout=await denseTelemetryLayout(page);evidence.push(layout);
                await writeFile(info.outputPath(`dense-${preset}-dpr2.json`),JSON.stringify(evidence,null,2));
                expect(layout.deviceScaleFactor).toBe(2);
                expect(layout.rowCount).toBe(preset==='compact-star'?123:135);
                expectDenseTelemetry(layout,viewport,layout.rowCount);
                await expectExposedTelemetryControls(page);
                if(preset==='compact-star'){
                    await expect(field('stellar-radius')).toHaveText('12,000 m');
                    await expect(field('schwarzschild-radius')).toHaveText('4,134.55 m');
                    await expect(field('surface-redshift')).toContainText('0.235');
                    await expect(field('frequency-ratio')).not.toHaveText('—');
                }else{
                    await expect(field('horizon-radius')).toHaveText('29,532.5 m');
                    // bc=(3√3/2)rs=76,727.687689 m for the adopted ten-solar-GM preparation.
                    await expect(field('critical-impact')).toHaveText('76,727.7 m');
                    await expect(field('horizon-area')).toContainText('m²');
                    await expect(field('static-shadow-angle')).toContainText('°');
                    await expect(field('stellar-radius')).toHaveText('—');
                    await expect(field('source-clock')).toHaveText('—');
                }
                const explain=preset==='compact-star'?field('clock-rate'):field('critical-impact');
                await explain.scrollIntoViewIfNeeded();await explain.focus();
                const tooltip=page.getByRole('tooltip');await expect(tooltip).toBeVisible();
                await expect(tooltip.locator('math')).toHaveCount(1);
                await expect(tooltip).toContainText(preset==='compact-star'?'coordinate':'unstable photon sphere');
                const bounds=await tooltip.boundingBox();expect(bounds).not.toBeNull();
                expect(bounds.x).toBeGreaterThanOrEqual(0);expect(bounds.y).toBeGreaterThanOrEqual(0);
                expect(bounds.x+bounds.width).toBeLessThanOrEqual(viewport.width+1);expect(bounds.y+bounds.height).toBeLessThanOrEqual(viewport.height+1);
                await page.screenshot({path:info.outputPath(`dense-${preset}-${viewport.width}-dpr2-tooltip.png`)});
                await page.keyboard.press('Escape');await expect(tooltip).toBeHidden();await expect(panel).toBeVisible();
                const groups=panel.locator('[data-observer-telemetry-section]');
                for(let i=0;i<await groups.count();i++){
                    const output=groups.nth(i).locator('[data-observer-telemetry]').last();
                    await output.scrollIntoViewIfNeeded();await expect(output).toBeInViewport();
                }
                await groups.first().locator('summary').scrollIntoViewIfNeeded();
                await page.screenshot({path:info.outputPath(`dense-${preset}-${viewport.width}-dpr2.png`)});
                await page.getByRole('button',{name:'Close controls',exact:true}).click();
            }
        });
    }
});

test.describe('mobile telemetry control clearance at device scale 2',()=>{
    test.use({deviceScaleFactor:2});
    test('all toolbar and transport controls remain exposed on narrow and short mobile screens',async({page},info)=>{
        await page.setViewportSize({width:390,height:844});await ready(page);
        const evidence=[];
        for(const viewport of [{width:390,height:844},{width:390,height:600},{width:320,height:600}]){
            await page.setViewportSize(viewport);
            await page.locator('[data-observer-panel-tab="telemetry"]').click();
            const panel=page.locator('[data-observer-panel="telemetry"]');await expect(panel).toBeVisible();
            const layout=await denseTelemetryLayout(page);
            expect(layout.deviceScaleFactor).toBe(2);
            expect(layout.panel.x).toBeGreaterThanOrEqual(0);expect(layout.panel.y).toBeGreaterThanOrEqual(0);
            expect(layout.panel.x+layout.panel.width).toBeLessThanOrEqual(viewport.width+1);
            expect(layout.panel.y+layout.panel.height).toBeLessThanOrEqual(viewport.height+1);
            expect(layout.overflow).toEqual({panel:0,body:0,document:0});
            await expectExposedTelemetryControls(page);
            const tabs=await page.locator('[data-observer-panel-tab]').evaluateAll(buttons=>buttons.map(button=>{
                const bounds=button.getBoundingClientRect(),hit=document.elementFromPoint(bounds.left+bounds.width/2,bounds.top+bounds.height/2);
                return {tab:button.dataset.observerPanelTab,
                    inViewport:bounds.left>=0&&bounds.top>=0&&bounds.right<=window.innerWidth+1&&bounds.bottom<=window.innerHeight+1,
                    exposed:hit===button||button.contains(hit)};
            }));
            expect(tabs).toHaveLength(11);
            expect(tabs.filter(tab=>!tab.inViewport||!tab.exposed),'Every toolbar tab stays directly clickable').toEqual([]);
            evidence.push({viewport,layout,tabs});
            await page.screenshot({path:info.outputPath(`mobile-control-clearance-${viewport.width}x${viewport.height}-dpr2.png`)});
            await page.getByRole('button',{name:'Close controls',exact:true}).click();
        }
        await writeFile(info.outputPath('mobile-control-clearance-dpr2.json'),JSON.stringify(evidence,null,2));
    });
});

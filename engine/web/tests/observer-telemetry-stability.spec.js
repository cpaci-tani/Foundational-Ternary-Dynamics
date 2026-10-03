/* global window, document */
import {test,expect} from '@playwright/test';
import {writeFile} from 'node:fs/promises';
import process from 'node:process';
import {gotoAndReady,openObserverWorkspace} from './_helpers.js';

const panel=page=>page.locator('[data-observer-panel="telemetry"]');

async function ready(page){
    await gotoAndReady(page,{path:'/?engine=wasm&lattice=9'});await openObserverWorkspace(page);
    await page.waitForFunction(()=>!!window.__FTD_DEV__.registry.get('observerWorkspace')?.snapshot);
    await page.evaluate(async()=>{
        const w=window.__FTD_DEV__.registry.get('observerWorkspace');
        w.setSetting('renderScale',.25);w.setSetting('autoQuality',false);await w.command({type:'pause'});
    });
    await page.locator('[data-observer-panel-tab="telemetry"]').click();await expect(panel(page)).toBeVisible();
}

async function resizeWidth(page,width){
    const bounds=await panel(page).boundingBox();
    const handle=await page.getByRole('button',{name:'Resize telemetry panel',exact:true}).boundingBox();
    const x=handle.x+handle.width/2,y=handle.y+handle.height/2;
    await page.mouse.move(x,y);await page.mouse.down();
    try{await page.mouse.move(x+(width-bounds.width)/2,y,{steps:8});}finally{await page.mouse.up();}
    await page.mouse.move(5,5);
}

async function transitions(page){
    return page.evaluate(async()=>{
        const w=window.__FTD_DEV__.registry.get('observerWorkspace'),base=w.snapshot,lattice=w.lattice.snapshot();
        const panel=document.querySelector('[data-observer-panel="telemetry"]');
        const rect=node=>{const r=node.getBoundingClientRect();return {x:r.x,y:r.y,width:r.width,height:r.height};};
        const update=w.ui.update;
        // Hold only renderer-to-UI publication while the deterministic fixture is
        // displayed. Simulation state, rendering and all production resize paths
        // remain intact; the original UI owner is restored in finally.
        w.ui.update=()=>{};
        const hit={entityId:'telemetry-layout-fixture',revision:1,emissionTime:.5,properTime:.1,doppler:.82,distance:.5};
        const states=[
            {name:'received-short',time:1,tick:'999999999999999999',hit},
            {name:'missing',time:1,tick:'999999999999999999',hit:null},
            {name:'received-large',time:999999,tick:'1000000000000000000',hit:{...hit,properTime:999999999999,emissionTime:.5,distance:999999}},
            {name:'received-small',time:.0000001,tick:'9',hit:{...hit,properTime:.0000001,emissionTime:0,distance:.0000001}},
            {name:'live',time:9.999,tick:'100',hit,playing:true},
            {name:'paused',time:10,tick:'101',hit,playing:false},
        ];
        const evidence=[];
        try{for(const state of states){
            const snapshot={...base,time:state.time,tick:state.tick,playing:state.playing??false,historyStart:0,
                observer:{...base.observer,properTime:state.time/2,worldlineStart:0}};
            update(snapshot,{settings:w.settings,hit:state.hit,lattice,rendering:w.renderer.diagnostics});
            await new Promise(resolve=>window.requestAnimationFrame(()=>window.requestAnimationFrame(resolve)));
            const body=panel.querySelector('.observer-panel-body');body.scrollTop=0;
            const rows=[...panel.querySelectorAll('[data-observer-telemetry]')].map(output=>{
                const card=output.closest('.observer-telemetry-reading'),label=card.querySelector('dt');
                const value=output.querySelector('.observer-telemetry-value'),unit=output.querySelector('.observer-telemetry-unit');
                const bodyBounds=body.getBoundingClientRect(),r=card.getBoundingClientRect();
                const clipped=[value,unit,label].some(node=>{
                    if(!node.textContent||window.getComputedStyle(node).visibility==='hidden')return false;
                    const range=document.createRange();range.selectNodeContents(node);
                    return [...range.getClientRects()].some(text=>text.width>0&&(text.left<r.left-1||text.right>r.right+1||text.top<r.top-1||text.bottom>r.bottom+1));
                });
                return {id:output.dataset.observerTelemetry,card:{x:r.x-bodyBounds.x,y:r.y-bodyBounds.y,width:r.width,height:r.height},clipped,
                    value:{text:value.textContent,font:parseFloat(window.getComputedStyle(value).fontSize),bounds:rect(value)},
                    unit:{text:unit.textContent,font:parseFloat(window.getComputedStyle(unit).fontSize),bounds:rect(unit)},
                    label:{text:label.textContent,font:parseFloat(window.getComputedStyle(label).fontSize),bounds:rect(label)}};
            });
            const head=panel.querySelector('.observer-panel-head'),headBounds=head.getBoundingClientRect();
            const headerClipped=[...head.querySelectorAll('h2,[data-observer-telemetry-summary]')].filter(node=>{
                const range=document.createRange();range.selectNodeContents(node);
                return [...range.getClientRects()].some(r=>r.width>0&&(r.left<headBounds.left-1||r.right>headBounds.right+1||r.top<headBounds.top-1||r.bottom>headBounds.bottom+1));
            }).map(node=>node.textContent);
            evidence.push({name:state.name,panel:rect(panel),body:rect(body),rows,headerClipped,summary:rect(panel.querySelector('[data-observer-telemetry-summary]')),
                controls:[...document.querySelectorAll('.observer-transport button,[data-observer-ui-toggle],[data-observer-action="close-panel"]')].map(button=>({name:button.dataset.observerAction,bounds:rect(button)}))});
        }}finally{
            w.ui.update=update;w.updateUI();
        }
        return {evidence,gpu:w.renderer.diagnostics.gpu,dpr:window.devicePixelRatio};
    });
}

function unchanged(actual,expected,label){
    for(const key of ['x','y','width','height'])expect(Math.abs(actual[key]-expected[key]),`${label} ${key}`).toBeLessThanOrEqual(.5);
}

async function currentReadings(page){
    return panel(page).evaluate(panel=>{
        const body=panel.querySelector('.observer-panel-body');body.scrollTop=0;
        const bodyBounds=body.getBoundingClientRect();
        return [...panel.querySelectorAll('[data-observer-telemetry]')].map(output=>{
            const card=output.closest('.observer-telemetry-reading'),bounds=card.getBoundingClientRect();
            const text=[card.querySelector('dt'),...output.children].map(node=>{
                const style=window.getComputedStyle(node),range=document.createRange();range.selectNodeContents(node);
                return {text:node.textContent,font:parseFloat(style.fontSize),clipped:style.visibility!=='hidden'&&style.display!=='none'&&[...range.getClientRects()].some(r=>r.width>0&&(r.left<bounds.left-1||r.right>bounds.right+1||r.top<bounds.top-1||r.bottom>bounds.bottom+1))};
            });
            return {id:output.dataset.observerTelemetry,card:{x:bounds.x-bodyBounds.x,y:bounds.y-bodyBounds.y,width:bounds.width,height:bounds.height},text};
        });
    });
}

async function headerClipping(page){
    return panel(page).evaluate(panel=>{
        const head=panel.querySelector('.observer-panel-head'),bounds=head.getBoundingClientRect();
        return [...head.querySelectorAll('h2,[data-observer-telemetry-summary]')].filter(node=>{
            const range=document.createRange();range.selectNodeContents(node);
            return [...range.getClientRects()].some(r=>r.width>0&&(r.left<bounds.left-1||r.right>bounds.right+1||r.top<bounds.top-1||r.bottom>bounds.bottom+1));
        }).map(node=>node.textContent);
    });
}

test('live number, missing-light and exact tick transitions keep telemetry rows and controls stable',async({page},info)=>{
    await page.setViewportSize({width:1440,height:900});await ready(page);
    const measurements=[];
    for(const width of [1400,500,320]){
        await resizeWidth(page,width);measurements.push({width,...await transitions(page)});
    }
    await writeFile(info.outputPath('telemetry-transition-geometry.json'),JSON.stringify(measurements,null,2));
    for(const measurement of measurements){
        const initial=measurement.evidence[0];
        if(process.env.FTD_HARDWARE_WEBGL==='1')expect(measurement.gpu.renderer).not.toMatch(/SwiftShader|llvmpipe|software/i);
        for(const state of measurement.evidence){
            const context=`at ${measurement.width}px during ${state.name}`;
            expect(state.rows.filter(row=>row.clipped).map(row=>row.id),`Unclipped readings ${context}`).toEqual([]);
            expect(state.headerClipped,`Header text stays within the header ${context}`).toEqual([]);
            unchanged(state.panel,initial.panel,`Panel ${context}`);unchanged(state.body,initial.body,`Body ${context}`);
            unchanged(state.summary,initial.summary,`Summary ${context}`);
            for(const row of state.rows)unchanged(row.card,initial.rows.find(item=>item.id===row.id).card,`${row.id} ${context}`);
            for(const control of state.controls)unchanged(control.bounds,initial.controls.find(item=>item.name===control.name).bounds,`${control.name} ${context}`);
        }
        expect(measurement.evidence[2].rows.find(row=>row.id==='tick').value.text).toBe('1000000000000000000');
    }
});

test('changing a measurement note preserves the focused reading, scroll position and collapsed categories',async({page},info)=>{
    await page.setViewportSize({width:1440,height:900});await ready(page);await resizeWidth(page,500);
    const model=panel(page).locator('[data-observer-telemetry-section="model"]');
    await model.locator('summary').click();await expect(model).not.toHaveAttribute('open');
    const clock=panel(page).locator('[data-observer-telemetry="optical-delay"]');
    await clock.scrollIntoViewIfNeeded();await clock.focus();
    const evidence=await page.evaluate(async()=>{
        const w=window.__FTD_DEV__.registry.get('observerWorkspace'),update=w.ui.update,base=w.snapshot,lattice=w.lattice.snapshot();
        const panel=document.querySelector('[data-observer-panel="telemetry"]'),body=panel.querySelector('.observer-panel-body');
        const focused=document.activeElement,scroll=body.scrollTop,note=panel.querySelector('[data-observer-telemetry="clock-slip"]').closest('.observer-telemetry-reading').querySelector('.observer-telemetry-note');
        const before=note.textContent;
        w.ui.update=()=>{};
        try{
            update({...base,observer:{...base.observer,worldlineStart:base.time+1}},{settings:w.settings,lattice,rendering:w.renderer.diagnostics});
            await new Promise(resolve=>window.requestAnimationFrame(()=>window.requestAnimationFrame(resolve)));
            return {before,after:panel.querySelector('[data-observer-telemetry="clock-slip"]').closest('.observer-telemetry-reading').querySelector('.observer-telemetry-note').textContent,
                sameFocusedNode:document.activeElement===focused,stillConnected:focused.isConnected,scrollBefore:scroll,scrollAfter:body.scrollTop,
                modelOpen:panel.querySelector('[data-observer-telemetry-section="model"]').open};
        }finally{w.ui.update=update;w.updateUI();}
    });
    await writeFile(info.outputPath('telemetry-metadata-focus.json'),JSON.stringify(evidence,null,2));
    expect(evidence.after).not.toBe(evidence.before);expect(evidence.sameFocusedNode).toBe(true);expect(evidence.stillConnected).toBe(true);
    expect(evidence.scrollBefore).toBeGreaterThan(100);
    expect(Math.abs(evidence.scrollAfter-evidence.scrollBefore)).toBeLessThanOrEqual(.5);expect(evidence.modelOpen).toBe(false);
});

test('compact-star and black-hole readings remain stable and unclipped in a 320-pixel panel as the sight ray changes',async({page},info)=>{
    await page.setViewportSize({width:1440,height:900});await ready(page);
    const evidence=[];
    for(const preset of ['compact-star','black-hole']){
        await page.locator('[data-observer-panel-tab="experiments"]').click();
        await page.locator(`[data-observer-action="experiment"][data-value="${preset}"]`).click();
        await page.evaluate(async()=>{await window.__FTD_DEV__.registry.get('observerWorkspace').command({type:'pause'});});
        await page.locator('[data-observer-panel-tab="telemetry"]').click();await resizeWidth(page,320);
        const status=panel(page).locator('[data-observer-telemetry="optical-status"]');
        await expect(status).toHaveText(preset==='compact-star'?'Source hit':'captured');
        await page.evaluate(()=>new Promise(resolve=>window.requestAnimationFrame(()=>window.requestAnimationFrame(resolve))));
        const initial=await currentReadings(page);
        expect(await headerClipping(page),`${preset} initial header stays within the header`).toEqual([]);
        await page.evaluate(()=>window.__FTD_DEV__.registry.get('observerWorkspace').input.setPose({yaw:0,pitch:1.4,roll:0}));
        await expect(status).toHaveText(preset==='compact-star'?'No current source hit':'escaped');
        await page.evaluate(()=>new Promise(resolve=>window.requestAnimationFrame(()=>window.requestAnimationFrame(resolve))));
        const changed=await currentReadings(page);
        expect(await headerClipping(page),`${preset} changed header stays within the header`).toEqual([]);
        evidence.push({preset,initial,changed});
        await writeFile(info.outputPath('telemetry-narrow-gr-ray-transitions.json'),JSON.stringify(evidence,null,2));
        expect(initial).toHaveLength(preset==='compact-star'?123:135);expect(changed).toHaveLength(initial.length);
        for(const [name,rows] of [['initial',initial],['changed',changed]]){
            expect(rows.filter(row=>row.text.some(text=>text.clipped)).map(row=>({id:row.id,text:row.text})),`${preset} ${name} text remains unclipped`).toEqual([]);
        }
        for(const row of changed)unchanged(row.card,initial.find(item=>item.id===row.id).card,`${preset} sight-ray change ${row.id}`);
        await panel(page).locator('[data-observer-telemetry-section="distance"] summary').scrollIntoViewIfNeeded();
        await page.screenshot({path:info.outputPath(`telemetry-narrow-${preset}-distance.png`)});
    }
});

test.describe('fluid typography on a high DPI display',()=>{
    test.use({deviceScaleFactor:2});
    test('numeric values, titles and units grow and shrink with their column while remaining readable',async({page},info)=>{
        await page.setViewportSize({width:1440,height:900});await ready(page);
        const evidence=[];
        for(const width of [400,500,590,500,400]){
            await resizeWidth(page,width);
            await page.evaluate(()=>new Promise(resolve=>window.requestAnimationFrame(()=>window.requestAnimationFrame(resolve))));
            const result=await panel(page).evaluate(panel=>{
                const card=panel.querySelector('[data-observer-telemetry="coordinate-time"]').closest('.observer-telemetry-reading');
                const font=selector=>parseFloat(window.getComputedStyle(card.querySelector(selector)).fontSize);
                return {width:panel.getBoundingClientRect().width,number:font('.observer-telemetry-value'),label:font('dt'),unit:font('.observer-telemetry-unit'),
                    title:parseFloat(window.getComputedStyle(panel.querySelector('.observer-telemetry-group-title')).fontSize),dpr:window.devicePixelRatio,
                    columns:new Set([...panel.querySelectorAll('[data-observer-telemetry-section]')].map(group=>Math.round(group.getBoundingClientRect().left))).size};
            });
            evidence.push(result);
            await page.screenshot({path:info.outputPath(`telemetry-fluid-${width}-${evidence.length}-dpr2.png`)});
        }
        await writeFile(info.outputPath('telemetry-fluid-fonts-dpr2.json'),JSON.stringify(evidence,null,2));
        expect(evidence.every(item=>item.dpr===2&&item.columns===1)).toBe(true);
        for(const item of evidence){expect(item.number).toBeGreaterThanOrEqual(15);expect(item.label).toBeGreaterThanOrEqual(11);expect(item.unit).toBeGreaterThanOrEqual(11);}
        for(const key of ['number','label','unit','title']){
            expect(evidence[2][key],`${key} grows as its one-column panel grows`).toBeGreaterThan(evidence[0][key]+.4);
            expect(Math.abs(evidence[4][key]-evidence[0][key]),`${key} restores at the smaller width`).toBeLessThanOrEqual(.1);
            expect(Math.abs(evidence[3][key]-evidence[1][key]),`${key} restores at the intermediate width`).toBeLessThanOrEqual(.1);
        }
    });
});

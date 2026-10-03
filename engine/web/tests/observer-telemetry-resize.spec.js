/* global window, document */
import {test,expect} from '@playwright/test';
import {writeFile} from 'node:fs/promises';
import {gotoAndReady,openObserverWorkspace} from './_helpers.js';

const telemetry=page=>page.locator('[data-observer-panel="telemetry"]');
const resizeHandle=page=>page.getByRole('button',{name:'Resize telemetry panel',exact:true});

async function ready(page){
    await gotoAndReady(page,{path:'/?engine=wasm&lattice=9'});await openObserverWorkspace(page);
    await page.waitForFunction(()=>!!window.__FTD_DEV__.registry.get('observerWorkspace')?.snapshot);
    await page.evaluate(async()=>{
        const w=window.__FTD_DEV__.registry.get('observerWorkspace');
        w.setSetting('renderScale',.25);w.setSetting('autoQuality',false);await w.command({type:'pause'});
        window.__telemetryResizePointerId=null;
        document.addEventListener('pointerdown',event=>{
            if(event.target.closest?.('[data-observer-telemetry-resize]'))window.__telemetryResizePointerId=event.pointerId;
        },{capture:true});
    });
}

async function openTelemetry(page){
    await page.locator('[data-observer-panel-tab="telemetry"]').click();
    await expect(telemetry(page)).toBeVisible();await expect(resizeHandle(page)).toBeVisible();
}

async function geometry(page){
    const bounds=await telemetry(page).boundingBox();expect(bounds).not.toBeNull();return bounds;
}

async function drag(page,dx,dy){
    const bounds=await resizeHandle(page).boundingBox();expect(bounds).not.toBeNull();
    const x=bounds.x+bounds.width/2,y=bounds.y+bounds.height/2;
    await page.mouse.move(x,y);await page.mouse.down();
    try{await page.mouse.move(x+dx,y+dy,{steps:8});}finally{await page.mouse.up();}
}

function sameSize(actual,expected){
    expect(Math.abs(actual.width-expected.width)).toBeLessThanOrEqual(2);
    expect(Math.abs(actual.height-expected.height)).toBeLessThanOrEqual(2);
}

async function layout(page){
    return telemetry(page).evaluate(panel=>{
        const body=panel.querySelector('.observer-panel-body'),bodyBounds=body.getBoundingClientRect();
        const outputs=[...panel.querySelectorAll('[data-observer-telemetry]')];
        const clipped=outputs.filter(output=>{
            const card=output.closest('.observer-telemetry-reading'),bounds=card.getBoundingClientRect();
            return [output,card.querySelector('dt')].some(node=>{
                const range=document.createRange();range.selectNodeContents(node);
                return [...range.getClientRects()].some(r=>r.width>0&&(r.left<bounds.left-1||r.right>bounds.right+1||r.top<bounds.top-1||r.bottom>bounds.bottom+1));
            });
        }).map(output=>output.dataset.observerTelemetry);
        const unreachable=outputs.filter(output=>{
            const r=output.closest('.observer-telemetry-reading').getBoundingClientRect();
            return r.width<=0||r.height<=0||r.left<bodyBounds.left-1||r.right>bodyBounds.right+1
                ||r.top-bodyBounds.top+body.scrollTop<-1||r.bottom-bodyBounds.top+body.scrollTop>body.scrollHeight+1;
        }).map(output=>output.dataset.observerTelemetry);
        const controls=[...document.querySelectorAll('.observer-transport button,[data-observer-ui-toggle]'),panel.querySelector('[data-observer-action="close-panel"]')].filter(Boolean);
        const obscured=controls.filter(button=>{
            const r=button.getBoundingClientRect(),hit=document.elementFromPoint(r.left+r.width/2,r.top+r.height/2);
            return r.width<=0||r.height<=0||!(hit===button||button.contains(hit));
        }).map(button=>button.textContent);
        const handle=panel.querySelector('[data-observer-telemetry-resize]'),handleBounds=handle.getBoundingClientRect();
        const hit=document.elementFromPoint(handleBounds.left+handleBounds.width/2,handleBounds.top+handleBounds.height/2);
        return {readings:outputs.length,clipped,unreachable,obscured,handleExposed:hit===handle||handle.contains(hit),
            columns:new Set([...panel.querySelectorAll('[data-observer-telemetry-section]')].map(group=>Math.round(group.getBoundingClientRect().left))).size,
            viewport:{width:window.innerWidth,height:window.innerHeight},dpr:window.devicePixelRatio,
            overflow:{panel:panel.scrollWidth-panel.clientWidth,body:body.scrollWidth-body.clientWidth,document:document.documentElement.scrollWidth-window.innerWidth},
            gpu:window.__FTD_DEV__.registry.get('observerWorkspace').renderer.diagnostics.gpu};
    });
}

async function usable(page){
    const bounds=await geometry(page),result=await layout(page);
    expect(bounds.x).toBeGreaterThanOrEqual(0);expect(bounds.y).toBeGreaterThanOrEqual(0);
    expect(bounds.x+bounds.width).toBeLessThanOrEqual(result.viewport.width+1);
    expect(bounds.y+bounds.height).toBeLessThanOrEqual(result.viewport.height+1);
    expect(result.readings).toBe(123);expect(result.clipped).toEqual([]);expect(result.unreachable).toEqual([]);
    expect(result.overflow).toEqual({panel:0,body:0,document:0});expect(result.obscured).toEqual([]);expect(result.handleExposed).toBe(true);
    // The toolbar intentionally scrolls horizontally on short/narrow screens.
    // Each tab must become an exposed normal-click target when revealed.
    const tabs=page.locator('[data-observer-panel-tab]'),reachableTabs=[];
    for(let i=0;i<await tabs.count();i++){
        const tab=tabs.nth(i);await tab.scrollIntoViewIfNeeded();
        const exposed=await tab.evaluate(button=>{
            const r=button.getBoundingClientRect(),hit=document.elementFromPoint(r.left+r.width/2,r.top+r.height/2);
            return {name:button.textContent,visible:r.left>=0&&r.right<=window.innerWidth&&r.top>=0&&r.bottom<=window.innerHeight,
                exposed:hit===button||button.contains(hit)};
        });
        expect(exposed.visible,`${exposed.name} remains inside viewport after reveal`).toBe(true);
        expect(exposed.exposed,`${exposed.name} receives pointer input after reveal`).toBe(true);
        reachableTabs.push(exposed);
    }
    const groups=telemetry(page).locator('[data-observer-telemetry-section]');
    for(let i=0;i<await groups.count();i++){
        const last=groups.nth(i).locator('[data-observer-telemetry]').last();
        await last.scrollIntoViewIfNeeded();await expect(last).toBeInViewport();
    }
    return {...result,bounds,reachableTabs};
}

async function noCapture(page){
    expect(await page.evaluate(()=>document.pointerLockElement)).toBeNull();
    expect(await page.evaluate(()=>window.__telemetryResizePointerId)).not.toBeNull();
    expect(await page.locator('[data-observer-telemetry-resize]').evaluate(handle=>
        handle.hasPointerCapture(window.__telemetryResizePointerId))).toBe(false);
}

test('mouse resizing reflows the telemetry grid while playback and observer clocks continue',async({page},info)=>{
    await page.setViewportSize({width:1920,height:1080});await ready(page);
    await page.getByRole('button',{name:'Play',exact:true}).click();await openTelemetry(page);
    const start=await geometry(page),initial=await layout(page);
    const before=await page.evaluate(()=>{const s=window.__FTD_DEV__.registry.get('observerWorkspace').snapshot;return {time:s.time,proper:s.observer.properTime};});
    await drag(page,-220,-90);
    const small=await geometry(page);
    expect(Math.abs(small.width-(start.width-440))).toBeLessThanOrEqual(2);
    expect(Math.abs(small.height-(start.height-90))).toBeLessThanOrEqual(2);
    const narrow=await usable(page);expect(narrow.columns).toBeLessThan(initial.columns);
    await telemetry(page).locator('[data-observer-telemetry-section]').first().locator('summary').scrollIntoViewIfNeeded();
    await page.screenshot({path:info.outputPath('telemetry-resized-narrow.png')});
    await drag(page,180,60);
    const large=await geometry(page);
    expect(Math.abs(large.width-(small.width+360))).toBeLessThanOrEqual(2);
    expect(Math.abs(large.height-(small.height+60))).toBeLessThanOrEqual(2);
    const wide=await usable(page);expect(wide.columns).toBeGreaterThan(narrow.columns);
    await page.waitForFunction(previous=>{
        const s=window.__FTD_DEV__.registry.get('observerWorkspace').snapshot;
        return s.playing&&s.time>previous.time+.1&&s.observer.properTime>previous.proper+.1;
    },before);
    expect(await page.evaluate(()=>window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.playing)).toBe(true);
    await noCapture(page);
    await telemetry(page).locator('[data-observer-telemetry-section]').first().locator('summary').scrollIntoViewIfNeeded();
    await page.screenshot({path:info.outputPath('telemetry-resized-wide.png')});
    await writeFile(info.outputPath('mouse-resize.json'),JSON.stringify({start,small,large,narrow,wide},null,2));
});

test('keyboard resizing preserves focus and preferred size without changing other drawers',async({page},info)=>{
    await page.setViewportSize({width:1440,height:900});await ready(page);await openTelemetry(page);
    const natural=await geometry(page),handle=resizeHandle(page);await handle.focus();
    await page.keyboard.press('ArrowLeft');await page.keyboard.press('Shift+ArrowLeft');
    await page.keyboard.press('ArrowUp');await page.keyboard.press('Shift+ArrowUp');
    const preferred=await geometry(page);
    expect(Math.abs(preferred.width-(natural.width-100))).toBeLessThanOrEqual(2);
    expect(Math.abs(preferred.height-(natural.height-100))).toBeLessThanOrEqual(2);
    await expect(handle).toBeFocused();await expect(handle).toHaveAttribute('aria-describedby',/observer-telemetry-resize-description/);
    await expect(page.locator('#observer-telemetry-resize-description')).toContainText(`Width ${Math.round(preferred.width)} CSS pixels; height ${Math.round(preferred.height)} CSS pixels`);
    const tooltip=page.getByRole('tooltip');await expect(tooltip).toBeVisible();
    await expect(tooltip).toContainText(/drag/i);await expect(tooltip).toContainText(/arrow/i);await expect(tooltip).toContainText(/Home/);
    expect(await page.evaluate(()=>{const w=window.__FTD_DEV__.registry.get('observerWorkspace');return {playing:w.snapshot.playing,move:w.input.sample().move};})).toEqual({playing:false,move:[0,0,0]});
    await page.getByRole('button',{name:'Close controls',exact:true}).click();await expect(handle).toBeHidden();
    await openTelemetry(page);sameSize(await geometry(page),preferred);
    await page.locator('[data-observer-panel-tab="camera"]').click();await expect(handle).toBeHidden();await expect(tooltip).toBeHidden();
    const camera=await page.locator('[data-observer-panel="camera"]').boundingBox();expect(camera.width).toBeLessThanOrEqual(460);
    expect(1440-camera.x-camera.width).toBeLessThanOrEqual(32);
    await page.locator('[data-observer-panel-tab="telemetry"]').click();sameSize(await geometry(page),preferred);
    await handle.focus();await page.keyboard.press('Home');sameSize(await geometry(page),natural);await expect(handle).toBeFocused();
    await page.keyboard.press('Shift+ArrowLeft');expect((await geometry(page)).width).toBeLessThan(natural.width-50);
    await handle.dblclick();sameSize(await geometry(page),natural);await noCapture(page);
    await writeFile(info.outputPath('keyboard-resize.json'),JSON.stringify({natural,preferred,camera},null,2));
});

test.describe('responsive and touch telemetry resizing at device scale 2',()=>{
    test.use({deviceScaleFactor:2,hasTouch:true});

    test('viewport clamps preserve preferred dimensions and keep all readings and controls reachable',async({page},info)=>{
        await page.setViewportSize({width:1920,height:1080});await ready(page);await openTelemetry(page);
        await drag(page,-100,-30);const preferred=await geometry(page),evidence=[];
        for(const viewport of [{width:390,height:844},{width:800,height:480},{width:320,height:600},{width:1920,height:1080}]){
            await page.setViewportSize(viewport);
            const current=await usable(page);expect(current.dpr).toBe(2);evidence.push(current);
            if(viewport.width<preferred.width)expect(current.bounds.width).toBeLessThan(preferred.width);
            await telemetry(page).locator('[data-observer-telemetry-section]').first().locator('summary').scrollIntoViewIfNeeded();
            await page.screenshot({path:info.outputPath(`telemetry-resize-clamp-${viewport.width}x${viewport.height}-dpr2.png`)});
        }
        sameSize(await geometry(page),preferred);await noCapture(page);
        await writeFile(info.outputPath('viewport-clamps-dpr2.json'),JSON.stringify({preferred,evidence},null,2));
    });

    test('actual touch pointer capture resizes the mobile panel without scrolling or capturing navigation',async({page},info)=>{
        await page.setViewportSize({width:390,height:844});await ready(page);await openTelemetry(page);
        const before=await geometry(page),bounds=await resizeHandle(page).boundingBox();
        const scroll=await telemetry(page).locator('.observer-panel-body').evaluate(body=>body.scrollTop);
        const cdp=await page.context().newCDPSession(page),x=bounds.x+bounds.width/2,y=bounds.y+bounds.height/2;
        await page.evaluate(()=>{
            window.__telemetryResizePointerTypes=[];
            document.addEventListener('pointerdown',event=>{if(event.target.closest?.('[data-observer-telemetry-resize]'))window.__telemetryResizePointerTypes.push(event.pointerType);},{capture:true});
        });
        try{
            await cdp.send('Input.dispatchTouchEvent',{type:'touchStart',touchPoints:[{x,y,id:1,radiusX:2,radiusY:2,force:1}]});
            await expect.poll(()=>resizeHandle(page).evaluate(handle=>handle.hasPointerCapture(window.__telemetryResizePointerId))).toBe(true);
            await cdp.send('Input.dispatchTouchEvent',{type:'touchMove',touchPoints:[{x:x-30,y:y-80,id:1,radiusX:2,radiusY:2,force:1}]});
            await cdp.send('Input.dispatchTouchEvent',{type:'touchEnd',touchPoints:[]});
        }finally{await cdp.detach();}
        const after=await geometry(page);expect(after.width).toBeLessThan(before.width-20);expect(after.height).toBeLessThan(before.height-50);
        expect(await page.evaluate(()=>window.__telemetryResizePointerTypes)).toContain('touch');
        expect(await telemetry(page).locator('.observer-panel-body').evaluate(body=>body.scrollTop)).toBe(scroll);
        await noCapture(page);const result=await usable(page);
        await telemetry(page).locator('[data-observer-telemetry-section]').first().locator('summary').scrollIntoViewIfNeeded();
        await page.screenshot({path:info.outputPath('telemetry-touch-resize-dpr2.png')});
        await writeFile(info.outputPath('touch-resize-dpr2.json'),JSON.stringify({before,after,result},null,2));
    });
});

test('closing, changing viewport and disposing during a drag release capture and stop later resizing',async({page})=>{
    const errors=[];page.on('pageerror',error=>errors.push(error.message));
    await page.setViewportSize({width:1920,height:1080});await ready(page);await openTelemetry(page);
    const begin=async()=>{
        const r=await resizeHandle(page).boundingBox();await page.mouse.move(r.x+r.width/2,r.y+r.height/2);await page.mouse.down();
        await expect.poll(()=>resizeHandle(page).evaluate(handle=>handle.hasPointerCapture(window.__telemetryResizePointerId))).toBe(true);
        await page.mouse.move(r.x+r.width/2-60,r.y+r.height/2-30,{steps:4});
    };
    await begin();const preferred=await geometry(page);
    await page.evaluate(()=>window.__FTD_DEV__.registry.get('observerWorkspace').ui.closePanel());
    await page.mouse.move(200,200);await page.mouse.up();await expect(resizeHandle(page)).toBeHidden();
    await openTelemetry(page);sameSize(await geometry(page),preferred);await noCapture(page);
    await begin();await page.setViewportSize({width:1440,height:900});
    const clamped=await geometry(page);await page.mouse.move(200,200);await page.mouse.up();sameSize(await geometry(page),clamped);await noCapture(page);
    await begin();await page.evaluate(()=>window.__FTD_DEV__.registry.get('observerHost').dispose());
    await page.mouse.move(300,300);await page.mouse.up();
    await expect(page.locator('[data-observer-telemetry-resize]')).toHaveCount(0);
    expect(await page.evaluate(()=>document.pointerLockElement)).toBeNull();expect(errors).toEqual([]);
});

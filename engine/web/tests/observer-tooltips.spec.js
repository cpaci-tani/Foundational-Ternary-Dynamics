/* global window, document, getComputedStyle */
import {test,expect} from '@playwright/test';
import {writeFile} from 'node:fs/promises';
import {gotoAndReady,openObserverWorkspace} from './_helpers.js';

async function ready(page){
    await gotoAndReady(page,{path:'/?engine=wasm&lattice=9'});await openObserverWorkspace(page);
    await page.waitForFunction(()=>!!window.__FTD_DEV__.registry.get('observerWorkspace')?.snapshot);
    await page.evaluate(async()=>{
        const w=window.__FTD_DEV__.registry.get('observerWorkspace');
        w.setSetting('renderScale',.25);w.setSetting('autoQuality',false);await w.command({type:'pause'});
    });
}

async function hoverSceneInformation(page,locator){
    const bounds=await locator.boundingBox();
    expect(bounds).not.toBeNull();
    await page.mouse.move(bounds.x+bounds.width/2,bounds.y+bounds.height/2);
}

test('every dynamic panel route covers controls, categories and informational readouts',async({page},info)=>{
    await ready(page);
    const evidence=[];
    for(const profile of ['sr','playground']){
        await page.evaluate(async profile=>{
            const w=window.__FTD_DEV__.registry.get('observerWorkspace');
            if(w.snapshot.profile!==profile)await w.command({type:'profile',profile});
            await w.command({type:'pause'});
            const source=w.snapshot.entities.find(e=>e.alive&&(profile==='sr'?e.shape==='clock':e.bodyType==='dynamic'));
            if(source)await w.action('select',{id:source.id});
        },profile);
        for(const panel of ['telemetry','objects','forcegun','world','camera','layers','phenomena','experiments','storage','lattice','help']){
            await page.locator('[data-observer-panel-tab="'+panel+'"]').click();
            await expect(page.locator('[data-observer-panel="'+panel+'"]')).toBeVisible();
            const inventory=await page.evaluate(()=>{
                const w=window.__FTD_DEV__.registry.get('observerWorkspace');
                return w.ui.tooltipInventory();
            });
            expect(inventory.length).toBeGreaterThan(20);
            expect(inventory.filter(row=>!row.covered)).toEqual([]);
            const fields=page.locator('[data-observer-panel="'+panel+'"] input,[data-observer-panel="'+panel+'"] select,[data-observer-panel="'+panel+'"] button');
            expect(await fields.evaluateAll(nodes=>nodes.every(node=>node.hasAttribute('data-observer-tooltip')))).toBe(true);
            evidence.push({profile,panel,count:inventory.length,missing:inventory.filter(row=>!row.covered)});
        }
    }
    await writeFile(info.outputPath('observer-tooltip-coverage.json'),JSON.stringify(evidence,null,2));
});

test('hover and keyboard explanations render mathematics with an offline MathML fallback',async({page},info)=>{
    await ready(page);await page.locator('[data-observer-panel-tab="camera"]').click();
    const fov=page.getByLabel('Field of view · degrees',{exact:true});await fov.hover();
    const tooltip=page.getByRole('tooltip');await expect(tooltip).toBeVisible();await expect(tooltip).toContainText('does not change velocity');
    await expect(fov).toHaveAttribute('aria-describedby','observer-explanation');
    const bounds=await tooltip.boundingBox();await page.mouse.move(bounds.x+bounds.width/2,bounds.y+bounds.height/2);
    await page.waitForTimeout(220);await expect(tooltip).toBeVisible();
    await page.locator('[data-observer-panel-tab="telemetry"]').click();
    const gamma=page.locator('[data-observer-telemetry="gamma"]');await gamma.focus();
    await expect(tooltip).toContainText('Lorentz factor');await expect(tooltip.locator('math')).toHaveCount(1);
    expect(await tooltip.textContent()).not.toMatch(/\[(?:AXIOM|THEOREM|PARAMETRIC|IMPOSED)\]/);
    await page.screenshot({path:info.outputPath('observer-keyboard-math-tooltip.png')});
    await page.evaluate(()=>{window.__observerTooltipKatex=window.katex;window.katex=undefined;});
    try{
        await page.locator('[data-observer-telemetry="clock-rate"]').focus();
        await expect(tooltip.locator('.observer-tooltip-equation > math')).toHaveCount(1);
        await expect(tooltip.locator('math')).toHaveAttribute('aria-label',/Proper time increment/);
        expect(await tooltip.textContent()).not.toMatch(/\\(?:frac|sqrt|gamma|tau)/);
    }finally{await page.evaluate(()=>{window.katex=window.__observerTooltipKatex;delete window.__observerTooltipKatex;});}
    await page.keyboard.press('Escape');await expect(tooltip).toBeHidden();
    await expect(page.locator('[data-observer-panel="telemetry"]')).toBeVisible();
    await expect(gamma).not.toHaveAttribute('aria-describedby','observer-explanation');
});

test('pointer-transparent HUD clocks and motion show their own hover explanations',async({page})=>{
    await ready(page);
    const gamma=page.locator('[data-observer-metric="gamma"]');
    expect(await gamma.evaluate(node=>getComputedStyle(node).pointerEvents)).toBe('none');
    await hoverSceneInformation(page,gamma);
    await expect(page.getByRole('tooltip')).toContainText('Lorentz factor');
    await expect(page.getByRole('tooltip').locator('math')).toHaveCount(1);
    await page.mouse.move(900,450);
    await hoverSceneInformation(page,page.locator('[data-observer-worldline]'));
    await expect(page.getByRole('tooltip')).toContainText('Explicit relocation starts a new origin');
    await page.mouse.move(900,450);
    await hoverSceneInformation(page,page.locator('[data-observer-history-coverage]'));
    await expect(page.getByRole('tooltip')).toContainText('Earlier unavailable events are not replaced');
});

test('canvas capture, hidden interface and disposal dismiss explanations without altering movement ownership',async({page})=>{
    await ready(page);await page.locator('[data-observer-panel-tab="telemetry"]').click();
    await page.locator('[data-observer-telemetry="gamma"]').focus();await expect(page.getByRole('tooltip')).toBeVisible();
    await page.getByRole('button',{name:'Hide interface',exact:true}).click();await expect(page.getByRole('tooltip')).toBeHidden();
    await page.getByRole('button',{name:'Show interface',exact:true}).click();
    await page.locator('.observer-canvas').click({position:{x:700,y:350}});
    await expect.poll(()=>page.evaluate(()=>document.pointerLockElement===window.__FTD_DEV__.registry.get('observerWorkspace').renderer.canvas)).toBe(true);
    await page.mouse.move(760,360);await page.waitForTimeout(250);await expect(page.getByRole('tooltip')).toBeHidden();
    await page.evaluate(()=>document.exitPointerLock());
    await page.locator('[data-observer-panel-tab="camera"]').click();
    await page.getByLabel('Field of view · degrees',{exact:true}).focus();await expect(page.getByRole('tooltip')).toBeVisible();
    await page.evaluate(()=>window.__FTD_DEV__.registry.get('observerHost').dispose());
    await expect(page.locator('#observer-explanation')).toHaveCount(0);
});

test('a focused setting explanation never blocks the adjacent Save world action',async({page})=>{
    await ready(page);await page.locator('[data-observer-panel-tab="storage"]').click();
    await page.getByLabel('World name',{exact:true}).fill('Tooltip permits adjacent save');
    await page.getByLabel('World name',{exact:true}).focus();
    await expect(page.getByRole('tooltip')).toBeVisible();
    expect(await page.getByRole('tooltip').evaluate(node=>getComputedStyle(node).pointerEvents)).toBe('none');
    await page.getByRole('button',{name:'Save world',exact:true}).click();
    await expect(page.locator('.observer-save')).toContainText('Tooltip permits adjacent save');
});

test('black-hole explanations distinguish exterior guard, horizon, sky and shadow geometry',async({page},info)=>{
    await ready(page);
    await page.locator('[data-observer-panel-tab="experiments"]').click();
    await page.locator('[data-observer-action="experiment"][data-value="black-hole"]').click();
    await expect(page.locator('[data-observer-compact-star]')).toContainText('Black-hole exterior');
    await expect(page.locator('[data-observer-panel="experiments"]')).toContainText('exterior numerical guard');
    await page.locator('[data-observer-history-coverage]').focus();
    await expect(page.getByRole('tooltip')).toContainText('independent of the empty entity-history interval');
    await page.locator('[data-observer-panel-tab="telemetry"]').click();
    await page.locator('[data-observer-telemetry="optical-status"]').focus();
    await expect(page.getByRole('tooltip')).toContainText('without a source-entity hit or retained source clock');
    await page.locator('[data-observer-telemetry="gr-frequency-ratio"]').focus();
    await expect(page.getByRole('tooltip')).toContainText('Static emission lapse divided by static reception lapse');
    await expect(page.getByRole('tooltip').locator('math')).toContainText('grav');
    await page.locator('[data-observer-telemetry="local-doppler-ratio"]').focus();
    await expect(page.getByRole('tooltip')).toContainText('opposite the incoming photon propagation');
    await expect(page.getByRole('tooltip').locator('math')).toContainText('local');
    await page.locator('[data-observer-telemetry="observer-exterior-guard"]').focus();
    await expect(page.getByRole('tooltip')).toContainText('does not mean a horizon crossing');
    await page.locator('[data-observer-telemetry="critical-impact"]').focus();
    await expect(page.getByRole('tooltip').locator('math')).toHaveCount(1);
    await expect(page.getByRole('tooltip')).toContainText('unstable photon sphere');
    await page.locator('[data-observer-telemetry="static-shadow-angle"]').focus();
    await expect(page.getByRole('tooltip')).toContainText('inside it the angle is pi minus');
    await page.locator('[data-observer-telemetry="proper-radial-distance"]').focus();
    await expect(page.getByRole('tooltip').locator('math')).toHaveCount(1);
    await expect(page.locator('[data-observer-telemetry="source-clock"]')).toHaveText('—');
    await page.screenshot({path:info.outputPath('black-hole-math-tooltip.png')});
});

test('narrow-screen hover explanations stay inside the viewport and preserve readable notes',async({page},info)=>{
    await page.setViewportSize({width:390,height:844});await ready(page);
    await page.locator('[data-observer-panel-tab="telemetry"]').click();
    await page.locator('[data-observer-telemetry="gamma"]').focus();
    const tooltip=page.getByRole('tooltip');await expect(tooltip).toBeVisible();
    const bounds=await tooltip.boundingBox();expect(bounds.x).toBeGreaterThanOrEqual(0);expect(bounds.y).toBeGreaterThanOrEqual(0);
    expect(bounds.x+bounds.width).toBeLessThanOrEqual(390);expect(bounds.y+bounds.height).toBeLessThanOrEqual(844);
    await expect(tooltip.locator('math')).toHaveCount(1);
    await page.screenshot({path:info.outputPath('mobile-math-tooltip.png')});
});

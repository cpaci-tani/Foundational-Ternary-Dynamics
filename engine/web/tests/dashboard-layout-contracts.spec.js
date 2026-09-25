import { test, expect } from '@playwright/test';
import { bootDashboard, switchMode } from './_helpers.js';
import { assertControlsReachable } from './visual-assertions.js';

const SIZES = [[320,568],[390,844],[667,375],[767,800],[768,800],[1024,768],[1279,800],[1280,800],[1440,900],[1920,1080],[1280,500]];

test('public toolbar actions receive the pointer at every audit breakpoint', async ({ page }) => {
    test.setTimeout(120_000);
    await page.addInitScript(() => localStorage.setItem('ftd-gpu-card-dismissed','1'));
    await bootDashboard(page);
    await expect(page.locator('#btn-fluid')).toHaveCount(0);
    await expect(page.locator('.tab[data-panel="fluid"]')).toHaveCount(1);
    for (const mode of ['lattice','particles','atoms','molecules','planetary','cosmic']) {
        if (mode!=='lattice') await switchMode(page,mode);
        for(const [width,height] of SIZES) {
            await page.setViewportSize({width,height});
            await page.waitForTimeout(120);
            await assertControlsReachable(page,
                '#btn-settings,#btn-toolbar-menu,.validity-status',
                `${mode} ${width}x${height}`, {scrollToolbar:true});
        }
    }
});

test('short-screen GPU card close control stays reachable', async ({ page }) => {
    await page.route('**/api/gpu-server/status',route=>route.fulfill({json:{running:false,exeExists:true,exeSize:1200,port:9100}}));
    await page.addInitScript(()=>localStorage.removeItem('ftd-gpu-card-dismissed'));
    await page.setViewportSize({width:667,height:375});
    await bootDashboard(page);
    await assertControlsReachable(page,'#gpu-card-close','short landscape startup');
    await page.locator('#gpu-card-close').click();
    await expect(page.locator('#gpu-server-card')).toBeHidden();
});

test('Atlas has a usable bounded center at phone and landscape sizes', async ({ page }) => {
    await page.goto('/fields-atlas.html');
    await page.waitForFunction(()=>window.__ftdAtlas?.ready);
    for(const [width,height] of [[320,568],[390,844],[667,375],[1440,900]]) {
        await page.setViewportSize({width,height});await page.waitForTimeout(120);
        const box=await page.locator('#atlas-canvas').boundingBox();
        expect(box.width).toBeGreaterThan(100);expect(box.height).toBeGreaterThan(80);
        expect(box.x).toBeGreaterThanOrEqual(-1);expect(box.x+box.width).toBeLessThanOrEqual(width+1);
        expect(await page.evaluate(()=>document.documentElement.scrollWidth-innerWidth)).toBeLessThanOrEqual(2);
        await assertControlsReachable(page,'#atlas-top button',`Atlas ${width}x${height}`);
    }
});

test('real tab drag and viewport rotation keep floating controls inside the viewport', async ({ page }) => {
    await page.addInitScript(()=>localStorage.setItem('ftd-gpu-card-dismissed','1'));
    await page.setViewportSize({width:1440,height:900});await bootDashboard(page);
    await page.evaluate(async()=>{
        (await import('/js/ui/shell/panel-mount-state.js')).writePanelMount('left');
        window.__ftdCtx.appShell.panelDock.activate('controls');
    });
    const tab=page.locator('#tab-bar .tab[data-panel="controls"]');await tab.scrollIntoViewIfNeeded();
    const r=await tab.boundingBox();
    await page.mouse.move(r.x+r.width/2,r.y+r.height/2);await page.mouse.down();
    await page.mouse.move(r.x+r.width/2+30,r.y+r.height/2,{steps:4});await page.mouse.up();
    for(const [width,height] of [[1440,900],[390,844],[667,375],[320,568]]){
        await page.setViewportSize({width,height});await page.waitForTimeout(150);
        const box=await page.locator('.floating-window[data-panel-id="controls"]').boundingBox();
        expect(box.x).toBeGreaterThanOrEqual(0);expect(box.y).toBeGreaterThanOrEqual(0);
        expect(box.x+box.width).toBeLessThanOrEqual(width+1);expect(box.y+box.height).toBeLessThanOrEqual(height+1);
        await assertControlsReachable(page,'.floating-window[data-panel-id="controls"] .floating-window-header button',`float ${width}x${height}`);
    }
});

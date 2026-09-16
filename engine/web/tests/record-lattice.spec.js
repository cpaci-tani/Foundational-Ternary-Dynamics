import {test, expect} from '@playwright/test';
import {gotoAndReady} from './_helpers.js';
const read = page => page.evaluate(async () => {
    const {getActiveScale0Bridge} = await import('/js/scales/scale0/state/store.js');
    const owner = getActiveScale0Bridge(window.__ftdCtx);
    return {finite: !!owner?.isFiniteRecord, ready: owner?.ready, busy: owner?.busy, failed: owner?.failed,
        rawTick: owner?.currentTick?.(),
        ...owner?.getProvenance?.(), diag: owner?.isFiniteRecord ? owner.getDiagnostics() : null};
});
async function ready(page, id) {
    await expect.poll(async () => { const s = await read(page); return s.ready && !s.busy && s.scenario; }, {timeout: 40000}).toBe(id);
}
async function load(page, id = 'record-relation') {
    await gotoAndReady(page, {path: '/?engine=wasm', timeout: 90000});
    await expect(page.locator('#scenario-select option[value="record-relation"]')).toHaveCount(1, {timeout: 30000});
    await page.evaluate(async () => {
        window.__originalLatticeScene = window.__ftdCtx.viewport.scene;
        window.__previousLatticeOwner = (await import('/js/scales/scale0/state/store.js')).getActiveScale0Bridge(window.__ftdCtx);
    });
    await page.selectOption('#scenario-select', id); await ready(page, id);
}
test('registered records use the normal scenario menu, scene, field volume, inspector and playback', async ({page}, info) => {
    const errors = []; page.on('pageerror', e => errors.push(e.message));
    await load(page);
    expect(await page.locator('#scenario-select option[value^="record-"]').count()).toBe(629);
    expect(await page.locator('#lattice-law, #phi-lattice-panel, iframe').count()).toBe(0);
    const first = await read(page);
    expect(await page.evaluate(() => !window.__previousLatticeOwner.isWorker || window.__previousLatticeOwner.disposed)).toBe(true);
    await expect.poll(() => page.evaluate(() => {
        const v = window.__ftdCtx.viewport;
        return v.scene === window.__originalLatticeScene && v.showFlux &&
            v.scene.children.includes(v._fluxRenderer._fluxVolume) && v._fluxRenderer._fluxVolume.geometry.drawRange.count > 0;
    })).toBe(true);
    await page.click('#btn-step');
    await expect.poll(async () => (await read(page)).microtick).toBe('1');
    await page.evaluate(() => {
        const c = window.__ftdCtx;
        c.appShell.panelDock.setCollapsed(false); c.appShell.panelDock.activate('controls');
    });
    await expect(page.locator('#record-observation-card')).toBeVisible();
    await expect(page.locator('#toggle-state-field')).not.toHaveClass(/is-inapplicable/);
    await expect(page.locator('#toggle-e-field')).toHaveClass(/is-inapplicable/);
    await page.click('#toggle-state-field');
    await page.selectOption('#record-quantity', 'incidence');
    const observed = await read(page);
    expect(observed.microtick).toBe('1'); expect(observed.ownerId).toBe(first.ownerId);
    await page.selectOption('#record-quantity', 'tokens');
    await page.click('#btn-play');
    await expect.poll(async () => Number((await read(page)).microtick)).toBeGreaterThan(1);
    await page.click('#btn-play'); await ready(page, 'record-relation');
    await page.click('#btn-reset'); await ready(page, 'record-relation');
    expect((await read(page)).microtick).toBe('0'); expect((await read(page)).checkpoint_sha256).toBe(first.checkpoint_sha256);
    await page.evaluate(() => {
        const c = window.__ftdCtx; c.appShell.panelDock.activate('inspector');
        c.inspector.selectLatticePosition({x: 4, y: 4, z: 4});
    });
    await expect(page.locator('#insp-moore-grid')).toContainText('Integer incidence Q');
    await page.evaluate(() => window.__ftdCtx.appShell.panelDock.activate('controls'));
    await page.setViewportSize({width: 1600, height: 1000}); await page.mouse.move(1200, 240);
    await page.screenshot({path: info.outputPath('web-lattice-records.png')});
    expect(errors).toEqual([]);
});

test('family cases, size/reset, queued ticks and return to existing scenarios share the worker slot', async ({page}) => {
    const errors = []; page.on('pageerror', e => errors.push(e.message));
    await load(page, 'record-witness');
    await page.evaluate(async () => {
        const {getActiveScale0Bridge} = await import('/js/scales/scale0/state/store.js');
        window.__oldRecordOwner = getActiveScale0Bridge(window.__ftdCtx);
        await Promise.all([window.__oldRecordOwner.advance(4), window.__oldRecordOwner.advance(65)]);
    });
    expect((await read(page)).microtick).toBe('69');
    await page.selectOption('#lattice-size', '3'); await ready(page, 'record-witness');
    expect((await read(page)).microtick).toBe('0');
    expect(await page.evaluate(() => window.__oldRecordOwner.disposed)).toBe(true);
    const ids = await page.locator('#scenario-select optgroup').evaluateAll(groups => groups.filter(g => /Registered/.test(g.label)).map(g => g.querySelector('option').value));
    expect(ids.length).toBe(3);
    for (const id of ids) { await page.selectOption('#scenario-select', id); await ready(page, id); await page.click('#btn-step'); await expect.poll(async () => (await read(page)).microtick).toBe('1'); }
    await page.selectOption('#scenario-select', 'flux-pulse');
    await expect.poll(async () => (await read(page)).finite).toBe(false);
    await expect.poll(async () => (await read(page)).ready, {timeout: 30000}).toBe(true);
    const previousTick = (await read(page)).rawTick;
    await page.click('#btn-step'); await expect.poll(async () => (await read(page)).rawTick).toBeGreaterThan(previousTick);
    await expect(page.locator('#record-observation-card')).not.toBeVisible();
    await page.selectOption('#scenario-select', 'record-sparse'); await ready(page, 'record-sparse');
    await page.selectOption('#engine-mode', 'particles');
    await expect.poll(async () => (await read(page)).finite).toBe(false);
    expect(errors).toEqual([]);
});

test('failure and exit during checkpoint loading never start an effective fallback', async ({page}) => {
    await page.route('**/api/lattice/records/checkpoint?**', async route => {
        const response = await route.fetch(); const body = await response.body(); body[0] ^= 1;
        await route.fulfill({response, body});
    });
    await gotoAndReady(page, {path: '/?engine=wasm', timeout: 90000});
    await expect(page.locator('#scenario-select option[value="record-relation"]')).toHaveCount(1, {timeout: 30000});
    await page.selectOption('#scenario-select', 'record-relation');
    await expect.poll(async () => (await read(page)).failed, {timeout: 30000}).toBe(true);
    expect((await read(page)).finite).toBe(true);
    await page.unroute('**/api/lattice/records/checkpoint?**');
    let release, entered;
    const gate = new Promise(r => {release = r;}); const pending = new Promise(r => {entered = r;});
    await page.route('**/api/lattice/records/checkpoint?**', async route => {entered(); await gate; await route.abort().catch(() => {});});
    await page.click('#btn-reset'); await pending;
    await page.selectOption('#scenario-select', 'flux-pulse'); release();
    await expect.poll(async () => (await read(page)).finite).toBe(false);
});

test('collapsible searchable picker retains the existing scenario command', async ({page}, info) => {
    await load(page);
    const tick = (await read(page)).microtick;
    await page.click('#scenario-picker > summary');
    for (const theme of ['dark', 'light']) {
        await page.evaluate(theme => document.documentElement.dataset.theme = theme, theme);
        const colors = await page.locator('.scenario-picker-menu').evaluate(el => {
            const rgb = s => s.match(/[\d.]+/g).slice(0,3).map(Number).map(v => {v/=255;return v<=.04045?v/12.92:((v+.055)/1.055)**2.4;});
            const luminance = s => {const c=rgb(s);return c[0]*.2126+c[1]*.7152+c[2]*.0722;};
            const background=luminance(getComputedStyle(el).backgroundColor);
            const foreground=luminance(getComputedStyle(el.querySelector('.scenario-picker-group>summary')).color);
            return {contrast:(Math.max(background,foreground)+.05)/(Math.min(background,foreground)+.05),height:el.getBoundingClientRect().height};
        });
        expect(colors.contrast).toBeGreaterThan(4.5);expect(colors.height).toBeLessThanOrEqual(481);
        await page.screenshot({path:info.outputPath(`scenario-picker-${theme}.png`)});
    }
    const groups = page.locator('.scenario-picker-group');
    expect(await groups.count()).toBeGreaterThan(3);
    expect(await page.locator('.scenario-picker-group[open]').count()).toBe(0);
    await groups.first().locator('summary').click();
    await expect(groups.first()).toHaveAttribute('open','');
    await groups.first().locator('summary').click();
    await expect(groups.first()).not.toHaveAttribute('open','');
    await page.locator('#scenario-picker input').fill('record-no-such-name');
    await expect(page.locator('.scenario-picker-empty')).toBeVisible();
    await page.locator('#scenario-picker input').fill('witness');
    await expect(page.locator('[data-scenario="record-witness"]')).toBeVisible();
    expect((await read(page)).microtick).toBe(tick);
    await page.click('[data-scenario="record-witness"]'); await ready(page,'record-witness');
    await expect(page.locator('#scenario-picker')).not.toHaveAttribute('open','');
    await expect(page.locator('#scenario-picker > summary')).toContainText(/witness/i);
});

test('all sidepanels follow preparation ownership, passive reads, failures and restoration', async ({page}) => {
    const errors=[];page.on('pageerror',e=>errors.push(e.message));await load(page);
    const coverage = await page.evaluate(async()=>{
        const {getPanelsForScale}=await import('/js/ui/scale-registry/panel-registry.js');
        const {RECORD_PANEL_CONTRACTS}=await import('/js/scales/scale0/ui/controls/record-panel-model.js');
        const catalog=await(await fetch('/api/lattice/records/catalog')).json();
        return {panels:getPanelsForScale(0).filter(p=>RECORD_PANEL_CONTRACTS[p.id]!=='existing').map(p=>p.id),count:catalog.scenarios.length};
    });
    expect(coverage.count).toBe(629);
    const families=await page.locator('#scenario-select optgroup').evaluateAll(groups=>groups.filter(g=>g.label.startsWith('Finite records')).map(g=>g.querySelector('option').value));
    for(const id of new Set(['record-relation',...families])){
        await page.selectOption('#scenario-select',id);await ready(page,id);
        const before=await page.evaluate(async()=>{
            const {getActiveScale0Bridge}=await import('/js/scales/scale0/state/store.js');return getActiveScale0Bridge(window.__ftdCtx).getDynamicalStateDigest();
        });
        for(const panel of coverage.panels){
            await page.evaluate(id=>{const c=window.__ftdCtx;c.appShell.panelDock.setCollapsed(false);c.appShell.panelDock.activate(id);},panel);
            const body=page.locator(`#panel-${panel} > .record-panel-observation`);
            await expect(body).toBeVisible();await expect(body).toHaveAttribute('data-tick','0');
            await expect(body).not.toContainText('Loading complete records');
        }
        const after=await page.evaluate(async()=>{
            const {getActiveScale0Bridge}=await import('/js/scales/scale0/state/store.js');return getActiveScale0Bridge(window.__ftdCtx).getDynamicalStateDigest();
        });expect(after).toBe(before);
    }
    await page.evaluate(()=>{const dock=window.__ftdCtx.appShell.panelDock;dock.activate('time');window.__recordTimeWindow=dock.floatPanel('time',80,80);dock.activate('charts');});
    await expect(page.locator('#panel-time > .record-panel-observation')).toBeVisible();
    await page.click('#btn-step');await expect(page.locator('#panel-time > .record-panel-observation')).toHaveAttribute('data-tick','1');
    await page.evaluate(()=>window.__recordTimeWindow.toggleCollapse());
    await page.click('#btn-step');await expect.poll(async()=>(await read(page)).microtick).toBe('2');
    await expect(page.locator('#panel-time > .record-panel-observation')).toHaveAttribute('data-tick','1');
    await page.evaluate(()=>window.__recordTimeWindow.toggleCollapse());
    await expect(page.locator('#panel-time > .record-panel-observation')).toHaveAttribute('data-tick','2');
    await page.evaluate(async()=>{const {getActiveScale0Bridge}=await import('/js/scales/scale0/state/store.js');getActiveScale0Bridge(window.__ftdCtx).fail(new Error('Injected panel failure'));});
    await expect(page.locator('#panel-time > .record-panel-observation')).toContainText('Runtime failed');
    await page.selectOption('#scenario-select','flux-pulse');await expect.poll(async()=>(await read(page)).finite).toBe(false);
    await expect(page.locator('[data-record-observation]')).toHaveCount(0);
    await expect(page.locator('.record-panel-observation')).toHaveCount(0);
    expect(errors).toEqual([]);
});

test('volume controls follow every preparation family through ticking, reset and resize', async ({page}) => {
    await load(page,'record-sparse');
    const digest=()=>page.evaluate(async()=>{const {getActiveScale0Bridge}=await import('/js/scales/scale0/state/store.js');return getActiveScale0Bridge(window.__ftdCtx).getDynamicalStateDigest();});
    const initialDigest=await digest();
    const errors=[];page.on('pageerror',e=>errors.push(e.message));
    const families=await page.locator('#scenario-select optgroup').evaluateAll(groups=>groups.filter(g=>g.label.startsWith('Finite records')).map(g=>g.querySelector('option').value));
    await page.evaluate(()=>{
        for(const [id,value] of [['flux-opacity','.43'],['flux-point-scale','1.8'],['flux-threshold','.1'],['flux-scenario-scale','1.2'],['flux-lattice-spacing','1.25'],['wireframe-brightness','.4']]){
            const el=document.getElementById(id);el.value=value;el.dispatchEvent(new Event('input',{bubbles:true}));
        }
        for(const id of ['toggle-flux-glow','toggle-flux-organic']){const el=document.getElementById(id);if(el.classList.contains('active'))el.click();}
        const shape=document.getElementById('flux-shape-select');shape.value='2';shape.dispatchEvent(new Event('change',{bubbles:true}));
    });
    const visual = ()=>page.evaluate(async()=>{
        const {sliderPositionToFluxThreshold}=await import('/js/viewport/flux-threshold.js');
        const v=window.__ftdCtx.viewport,r=v._fluxRenderer;
        return {opacity:r._fluxVolume?.material.uniforms.uOpacity.value,shape:r._fluxVolume?.material.uniforms.shapeType.value,
            point:r._fluxPointScale,threshold:r._fluxThreshold,expected:sliderPositionToFluxThreshold(document.getElementById('flux-threshold').value),
            spacing:r._fluxVolume?.scale.x,scale:r._scenarioScale,organic:r._fluxOrganic,glow:r._fluxGlow};
    });
    await expect.poll(async()=>(await visual()).opacity).toBe(.43);
    expect(await digest()).toBe(initialDigest);
    for(const id of new Set(['record-sparse',...families])){
        await page.selectOption('#scenario-select',id);await ready(page,id);
        await expect.poll(async()=>{const v=await visual();return {...v,threshold:v.threshold.toExponential(2),expected:v.expected.toExponential(2)};}).toEqual({opacity:.43,shape:2,point:1.8,threshold:"2.00e-8",expected:"2.00e-8",spacing:1.25,scale:1.2,organic:false,glow:false});
        await page.evaluate(async()=>{const {getActiveScale0Bridge}=await import('/js/scales/scale0/state/store.js');await getActiveScale0Bridge(window.__ftdCtx).advance(4);});
        await expect.poll(()=>page.evaluate(async()=>{
            const {getActiveScale0Bridge}=await import('/js/scales/scale0/state/store.js');const o=getActiveScale0Bridge(window.__ftdCtx),r=window.__ftdCtx.viewport._fluxRenderer;
            return [...o.frame.volume].every((value,i)=>r._fluxActivation[i]===value);
        })).toBe(true);
    }
    await page.selectOption('#scenario-select','record-sparse');await ready(page,'record-sparse');
    await page.selectOption('#lattice-size','3');await ready(page,'record-sparse');
    await expect.poll(async()=>(await visual()).spacing).toBe(1.25);
    await page.click('#btn-reset');await ready(page,'record-sparse');
    await expect.poll(async()=>(await visual()).opacity).toBe(.43);
    await page.evaluate(()=>document.getElementById('toggle-flux-volume').click());
    await page.selectOption('#scenario-select','record-relation');await ready(page,'record-relation');
    expect(await page.evaluate(()=>window.__ftdCtx.viewport.showFlux)).toBe(false);
    await page.evaluate(()=>document.getElementById('toggle-flux-volume').click());
    await expect.poll(()=>page.evaluate(()=>window.__ftdCtx.viewport._fluxRenderer._fluxVolume.geometry.drawRange.count)).toBeGreaterThan(0);
    await page.selectOption('#scenario-select','flux-pulse');await expect.poll(async()=>(await read(page)).finite).toBe(false);
    await expect.poll(async()=>(await visual()).opacity).toBe(.43);
    expect(errors).toEqual([]);
});

test('all twenty mixed preparations load their registered observable and expose actual voxel dynamics', async ({page},info) => {
    test.setTimeout(180000);
    await load(page,'record-relation');
    const rows=await page.evaluate(async()=>(await(await fetch('/api/lattice/records/catalog')).json()).scenarios.filter(row=>/Registered mixed/.test(row.family)));
    expect(rows.length).toBe(20);
    const snapshot=()=>page.evaluate(async()=>{
        const {getActiveScale0Bridge}=await import('/js/scales/scale0/state/store.js');const o=getActiveScale0Bridge(window.__ftdCtx),r=window.__ftdCtx.viewport._fluxRenderer;
        const mask=r._fluxVolume?.geometry.getAttribute('particleVisibility').array;
        return {quantity:o.quantity,threshold:r._fluxThreshold,slider:document.getElementById('flux-threshold').value,
            visible:mask?[...mask].filter(v=>v===1).length:0,positive:[...o.frame.volume].filter(v=>v>0).length,
            aligned:[...o.frame.volume].every((v,i)=>r._fluxActivation[i]===v),tick:o.currentTick()};
    });
    for(const row of rows){
        // Reproduce stale |Q| selection: mixed incidence is zero initially.
        await page.evaluate(async()=>{const {getActiveScale0Bridge}=await import('/js/scales/scale0/state/store.js');getActiveScale0Bridge(window.__ftdCtx).setRecordQuantity('incidence');});
        await page.selectOption('#scenario-select',`record-${row.id}`);await ready(page,`record-${row.id}`);
        await expect.poll(async()=>{const s=await snapshot();return s.aligned&&s.visible===s.positive&&s.visible>0;}).toBe(true);
        const start=await snapshot();expect(start.quantity).toBe(row.observation);expect(start.threshold).toBeCloseTo(2e-8,14);expect(Number(start.slider)).toBe(.0001);
        expect(start.visible).toBe(row.observation==='field_tokens'?27:17**3);
        for(let step=1;step<=4;step++){
            await page.click('#btn-step');await expect.poll(async()=>{const s=await snapshot();return s.tick===step&&s.aligned&&s.visible===s.positive;}).toBe(true);
        }
    }
    await page.selectOption('#scenario-select','record-mixed_01');await ready(page,'record-mixed_01');
    await expect.poll(async()=>(await snapshot()).visible).toBe(27);
    await page.screenshot({path:info.outputPath('mixed-response-probe.png')});
    await page.evaluate(()=>{const el=document.getElementById('flux-threshold');el.value='0';el.dispatchEvent(new Event('input',{bubbles:true}));});
    await expect.poll(async()=>(await snapshot()).visible).toBe(17**3);
    await page.click('#btn-reset');await ready(page,'record-mixed_01');await expect.poll(async()=>(await snapshot()).visible).toBe(27);
    for(const id of ['flux-pulse','em-dipole']) {
        const exists=await page.locator(`#scenario-select option[value="${id}"]`).count();if(!exists)continue;
        await page.selectOption('#scenario-select',id);
        await expect.poll(()=>page.evaluate(()=>window.__ftdCtx.viewport._fluxRenderer._fluxThreshold)).toBeCloseTo(2e-8,14);
    }
});

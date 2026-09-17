import {test, expect} from '@playwright/test';
import {gotoAndReady} from './_helpers.js';
import {readFile} from 'node:fs/promises';

const read = page => page.evaluate(async () => {
    const store = await import('/js/scales/scale0/state/store.js');
    const owner = store.getActiveScale0Bridge(window.__ftdCtx);
    return {finite: !!owner?.isFiniteRecord, ready: owner?.ready && !owner?.busy,
        tick: owner?.currentTick?.(), ...owner?.getProvenance?.(),
        diagnostics: owner?.isFiniteRecord ? owner.getDiagnostics() : null,
        qualification: store.getScale0QualificationState()};
});
async function start(page) {
    await gotoAndReady(page, {path: '/?engine=wasm', timeout: 90000});
    await expect(page.locator('#seed-scenario option')).toHaveCount(772, {timeout: 30000});
    await page.selectOption('#scenario-select', 'record-relation');
    await expect.poll(async () => (await read(page)).ready, {timeout: 40000}).toBe(true);
    await page.evaluate(() => {const c = window.__ftdCtx; c.pauseSimulation(); c.appShell.panelDock.setCollapsed(false); c.appShell.panelDock.activate('seeding');});
    await expect(page.locator('#panel-seeding')).toBeVisible();
}
const digest = page => page.evaluate(async () => {
    const d=await (await import('/js/scales/scale0/state/store.js')).getActiveScale0Bridge(window.__ftdCtx).getDynamicalStateDigest();
    return d && typeof d==='object' ? `${d.hashLo}:${d.hashHi}:${d.tick}` : d;
});

// Every scientific property is a numeric propertyEditor (no <select>/<checkbox> for
// science, only for scenario navigation); this sets the underlying value directly
// and fires the same 'change' event the up/down steppers use.
async function setProperty(scope, label, value) {
    const input = scope.getByLabel(label, {exact: true});
    await input.fill(String(value));
    await input.dispatchEvent('change');
}

async function chooseDraft(page, id) {
    await page.selectOption('#seed-scenario', id);
    await expect(page.locator('#panel-seeding')).toHaveAttribute('data-preparation', id);
    await expect(page.locator('#panel-seeding')).toHaveAttribute('data-seed-ready', 'true');
}

test('native numeric edits follow dependent defaults, retain precise entries, and install and reset the actual owner', async ({page}, info) => {
    const errors=[]; page.on('pageerror', e=>errors.push(e.message));
    await start(page); await chooseDraft(page, 'flux-pulse');
    const panel=page.locator('#panel-seeding');
    await page.click('#seed-apply');
    await expect(page.locator('#seed-status')).toContainText('Applied registered preset', {timeout:60000});
    const original=await digest(page);
    expect((await read(page)).tick).toBe(0);
    await setProperty(panel,'Packet width',6);
    await expect.poll(async()=>Number(await panel.getByLabel('Carrier wavenumber',{exact:true}).inputValue())).toBeCloseTo(Math.PI/12,12);
    await setProperty(panel,'Carrier wavenumber',.35);
    await setProperty(panel,'Packet width',8);
    await expect(panel.getByLabel('Carrier wavenumber',{exact:true})).toHaveValue('0.35');
    await panel.getByRole('button',{name:'Reset Carrier wavenumber to preset',exact:true}).click();
    await expect.poll(async()=>Number(await panel.getByLabel('Carrier wavenumber',{exact:true}).inputValue())).toBeCloseTo(Math.PI/16,12);
    await setProperty(panel,'Packet amplitude',1);
    // Export during the dependent-default debounce must resolve this draft,
    // never the previous form's cached values.
    const exported = page.waitForEvent('download');
    await panel.getByRole('button',{name:'Export recipe',exact:true}).click();
    const envelope = JSON.parse(await readFile(await (await exported).path(),'utf8'));
    expect(envelope.resolvedProperties.find(p=>p.key==='packet.amplitude').value).toBe(1);
    expect(envelope.resolvedProperties.find(p=>p.key==='packet.carrierK').value).toBeCloseTo(Math.PI/16,12);
    const amplitude=panel.locator('[data-property-key="packet.amplitude"]');
    await expect(amplitude.locator('.seed-property-message')).toContainText('Outside the recommended');
    expect(Number(await amplitude.locator('input[type=range]').getAttribute('max'))).toBeGreaterThanOrEqual(1);
    await page.click('#seed-preview');
    await expect(page.locator('#seed-status')).toContainText('active lattice is unchanged',{timeout:60000});
    expect(await digest(page)).toBe(original);
    await page.click('#seed-apply');
    await expect(page.locator('#seed-status')).toContainText('Applied custom preparation',{timeout:60000});
    const edited=await digest(page); expect(edited).not.toBe(original);
    expect((await read(page)).qualification.status).toBe('suspended');
    expect((await read(page)).tick).toBe(0);
    await page.click('#btn-step'); await expect.poll(async()=>(await read(page)).tick).toBeGreaterThan(0);
    await page.click('#btn-reset');
    await expect.poll(()=>digest(page),{timeout:60000}).toBe(edited);
    await expect(panel.locator('.seed-recipe-summary')).toContainText('Custom preparation');
    for (const theme of ['dark','light']) {
        await page.evaluate(theme=>document.documentElement.dataset.theme=theme,theme);
        await panel.evaluate(el=>{el.style.width='300px';el.style.maxWidth='300px';});
        expect(await panel.evaluate(el=>el.scrollWidth<=el.clientWidth+1)).toBe(true);
        await page.screenshot({path:info.outputPath(`native-seeding-narrow-${theme}.png`)});
    }
    expect(errors).toEqual([]);
});

test('native scheduled reservoir edits reach the installed pump without later profile erasure', async ({page}) => {
    await start(page); await chooseDraft(page,'s0-cell-membrane-pumped');
    const panel=page.locator('#panel-seeding');
    await setProperty(panel,'Pump duration',7);
    await page.click('#seed-apply');
    await expect(page.locator('#seed-status')).toContainText('Applied custom preparation',{timeout:60000});
    const before=await digest(page);
    const installed=await page.evaluate(async()=>{
        const owner=(await import('/js/scales/scale0/state/store.js')).getActiveScale0Bridge(window.__ftdCtx);
        return owner.seedDescription.properties.find(p=>p.key==='protocol.pumpTicks').value;
    });
    expect(installed).toBe(7);
    await page.click('#btn-step'); await expect.poll(async()=>(await read(page)).tick).toBe(1);
    expect(await digest(page)).not.toBe(before);
    await page.click('#btn-reset'); await expect.poll(()=>digest(page),{timeout:60000}).toBe(before);
});

test('direct WASM applies native recipes when worker transport is disabled', async ({page}) => {
    await page.addInitScript(()=>{window.__ftdWasmWorker=false;});
    await start(page); await chooseDraft(page,'flux-pulse');
    await setProperty(page.locator('#panel-seeding'),'Packet amplitude',.42);
    await page.click('#seed-apply');
    await expect(page.locator('#seed-status')).toContainText('Applied custom preparation',{timeout:60000});
    const active=await page.evaluate(async()=>{
        const owner=(await import('/js/scales/scale0/state/store.js')).getActiveScale0Bridge(window.__ftdCtx);
        return {worker:!!owner.isWorker,amplitude:owner.seedDescription.properties.find(p=>p.key==='packet.amplitude').value,tick:owner.currentTick()};
    });
    expect(active).toEqual({worker:false,amplitude:.42,tick:0});
});

test('native category ingredients remain custom through preview, apply and reset, and rapid draft changes settle on the last selection', async ({page}) => {
    await start(page); await chooseDraft(page,'s0-seed-monopole');
    const panel=page.locator('#panel-seeding');
    await panel.getByRole('button',{name:'Open full category base'}).click();
    await expect(panel).toHaveAttribute('data-seed-ready','true');
    await expect(panel.locator('[data-component-id]')).toHaveCount(3);
    const ingredient=panel.locator('[data-component-id]').first();
    await setProperty(ingredient,'Ingredient enabled',1);
    await page.click('#seed-preview');
    await expect(page.locator('#seed-status')).toContainText('active lattice is unchanged',{timeout:60000});
    await page.click('#seed-apply');
    await expect(page.locator('#seed-status')).toContainText('Applied custom preparation',{timeout:60000});
    const state=await digest(page);
    const recipe=await page.evaluate(()=>window.__ftdCtx._appliedSeedRecipe);
    expect(recipe.blank).toBe(true); expect(recipe.components[0].enabled).toBe(true);
    expect(recipe.components.slice(1).every(c=>!c.enabled)).toBe(true);
    await page.click('#btn-step'); await expect.poll(async()=>(await read(page)).tick).toBeGreaterThan(0);
    await page.click('#btn-reset'); await expect.poll(()=>digest(page),{timeout:60000}).toBe(state);
    await page.evaluate(()=>{
        const select=document.getElementById('seed-scenario');
        for(const id of ['record-relation','s0-cell-membrane-transfer','flux-pulse']) {
            select.value=id;select.dispatchEvent(new Event('change',{bubbles:true}));
        }
    });
    await expect(panel).toHaveAttribute('data-preparation','flux-pulse');
    await expect(panel).toHaveAttribute('data-seed-ready','true');
    await expect(panel.getByLabel('Packet amplitude',{exact:true})).toBeVisible();
    expect(await digest(page)).toBe(state);
});

test('real panel previews without mutation, installs all record components and resets the applied recipe', async ({page}, info) => {
    const errors = []; page.on('pageerror', e => errors.push(e.message));
    await start(page);
    const before = await read(page), beforeDigest = await digest(page);
    const panel = page.locator('#panel-seeding');
    await page.selectOption('#seed-scenario', 'record-empty');
    await panel.getByRole('button', {name: 'Open full category base'}).click();
    await setProperty(panel, 'Lattice size', 7);
    const field = panel.locator('[data-component-id="base-field"]');
    await setProperty(field, 'Enabled', 1);
    await expect(field.getByLabel('Field channel', {exact: true})).toHaveAttribute('max', '383');
    await setProperty(field, 'Region shape', 0); // 'all'
    const relation = panel.locator('[data-component-id="base-relation"]');
    await setProperty(relation, 'Enabled', 1);
    await setProperty(relation, 'Region shape', 1);
    await setProperty(relation, 'Relation orientation', 8);
    await setProperty(relation, 'Slot', 1);
    await setProperty(relation, 'Phase', 3);
    await setProperty(relation, 'Polarity', -1);
    const marker = panel.locator('[data-component-id="base-manifestation"]');
    await setProperty(marker, 'Enabled', 1);
    await setProperty(marker, 'Region shape', 1);
    await setProperty(marker, 'Manifestation', -1);
    const layer = panel.locator('[data-component-id="base-collision"]');
    await setProperty(layer, 'Enabled', 1);
    await setProperty(layer, 'Region shape', 1);
    await setProperty(layer, 'Collision layer', 2);
    await page.click('#seed-preview');
    await expect(page.locator('#seed-status')).toContainText('343 field tokens · 1 relation tokens · 1 stored markers', {timeout: 40000});
    expect((await read(page)).ownerId).toBe(before.ownerId);
    expect(await digest(page)).toBe(beforeDigest);
    await page.click('#seed-apply');
    await expect(page.locator('#seed-status')).toContainText('Applied custom preparation', {timeout: 40000});
    const applied = await read(page), appliedDigest = await digest(page);
    expect(applied.ownerId).not.toBe(before.ownerId);
    expect(applied.tick).toBe(0);
    expect(applied.custom_preparation).toBe(true);
    expect(applied.qualification.status).toBe('suspended');
    expect(applied.qualification.lastMutation.reason).toBe('custom-seed');
    expect(applied.diagnostics.fieldTokens).toBe(343);
    expect(applied.diagnostics.relationTokens).toBe(1);
    await panel.getByRole('button',{name:'View summary',exact:true}).click();
    await panel.locator('.seed-recipe-section > summary').filter({hasText:'Resolved results'}).click();
    await expect(panel.locator('.seed-recipe-summary')).toContainText('Discretized geometry');
    await expect(panel.locator('.seed-recipe-summary')).toContainText('343 selected sites');
    await expect(panel.locator('.seed-recipe-summary')).toContainText('spatial overlap does not imply a shared record address');
    expect(applied.canonical_adoption).toBe(false);
    await expect(page.locator('#scenario-select')).toHaveValue('record-empty');
    await expect.poll(() => page.evaluate(() => window.__ftdCtx.viewport._fluxRenderer._fluxVolume.geometry.drawRange.count)).toBeGreaterThan(0);
    expect(await page.evaluate(() => window.__ftdCtx.viewport._fluxRenderer._fluxThreshold)).toBe(2e-8);
    await page.click('#btn-step');
    await expect.poll(async () => (await read(page)).tick).toBe(1);
    await page.click('#btn-reset');
    await expect.poll(async () => (await read(page)).ownerId).not.toBe(applied.ownerId);
    await expect.poll(async () => (await read(page)).ready).toBe(true);
    expect(await digest(page)).toBe(appliedDigest);
    expect((await read(page)).custom_preparation).toBe(true);
    // Editing and importing drafts do not reset the active owner.
    const download = page.waitForEvent('download');
    await panel.getByRole('button', {name: 'Export recipe'}).click();
    const path = await (await download).path();
    await panel.getByRole('button', {name: 'Restore preset', exact: true}).click();
    await expect(panel.locator('[data-component-id]')).toHaveCount(0);
    await page.locator('#seed-import').setInputFiles(path);
    await expect(page.locator('#seed-status')).toContainText('Imported draft');
    await expect(panel.locator('[data-component-id]')).toHaveCount(9);
    expect(await digest(page)).toBe(appliedDigest);
    await panel.getByLabel('Find properties').fill('provenance');
    await expect(panel.locator('.seed-body > details:visible')).toHaveCount(1);
    await panel.getByLabel('Find properties').fill('');
    await panel.getByRole('button', {name: 'Collapse all', exact: true}).focus();
    await page.keyboard.press('Space');
    await expect(panel.locator('.seed-body details[open]')).toHaveCount(0);
    expect(await page.evaluate(() => window.__ftdCtx.running)).toBe(false);
    await panel.getByRole('button', {name: 'Expand all', exact: true}).click();
    for (const theme of ['dark', 'light']) {
        await page.evaluate(theme => document.documentElement.dataset.theme = theme, theme);
        await page.setViewportSize({width: 1600, height: 1000});
        await panel.locator('.seed-header').scrollIntoViewIfNeeded();
        await page.screenshot({path: info.outputPath(`seeding-${theme}.png`)});
        expect(await panel.evaluate(el => el.scrollWidth <= el.clientWidth + 1)).toBe(true);
    }
    // A failed resize restores the toolbar and leaves the complete owner intact.
    const beforeResize = await read(page);
    await page.selectOption('#lattice-size', '3');
    await expect(page.locator('#lattice-size')).toHaveValue('7');
    expect((await read(page)).ownerId).toBe(beforeResize.ownerId);
    expect(await digest(page)).toBe(appliedDigest);
    await page.selectOption('#lattice-size', '9');
    await expect.poll(async () => (await read(page)).ownerId).not.toBe(beforeResize.ownerId);
    await expect.poll(async () => (await read(page)).ready).toBe(true);
    expect((await read(page)).diagnostics.fieldTokens).toBe(729);
    await page.click('#seed-run');
    await expect.poll(async () => (await read(page)).tick).toBeGreaterThan(0);
    await page.click('#btn-play');
    expect(errors).toEqual([]);
});

test('registered mixed presets preserve receipts and visibility; native presets restore the effective owner', async ({page}) => {
    await start(page);
    const ids = await page.locator('#seed-scenario optgroup').evaluateAll(groups => groups.filter(g => /mixed/.test(g.label)).map(g => g.querySelector('option').value));
    expect(ids).toHaveLength(2);
    for (const id of ids) {
        await page.selectOption('#seed-scenario', id);
        await page.click('#seed-apply');
        await expect(page.locator('#seed-status')).toContainText('Applied registered preset', {timeout: 40000});
        const state = await read(page);
        expect(state.scenario).toBe(id); expect(state.custom_preparation).toBe(false);
        expect(state.qualification.status).toBe('within-contract');
        const expected = await page.evaluate(async id => {
            const r = await fetch(`/api/lattice/records/checkpoint?scenario=${id.slice(7)}&size=17`);
            return r.headers.get('X-Checkpoint-SHA256');
        }, id);
        expect(state.checkpoint_sha256).toBe(expected);
        await expect.poll(() => page.evaluate(() => window.__ftdCtx.viewport._fluxRenderer._fluxVolume.geometry.drawRange.count)).toBeGreaterThan(0);
    }
    await page.selectOption('#seed-scenario', 'flux-pulse');
    await expect(page.locator('#seed-preview')).toHaveText('Preview seed');
    await page.click('#seed-apply');
    await expect(page.locator('#seed-status')).toContainText('Applied registered preset', {timeout: 40000});
    expect((await read(page)).finite).toBe(false);
    await page.click('#btn-step');
    await expect.poll(async () => (await read(page)).tick).toBeGreaterThan(0);
});

test('invalid input, preparation failure, cancellation and late responses retain the active lattice', async ({page}) => {
    await start(page);
    const before = await read(page), beforeDigest = await digest(page);
    const panel = page.locator('#panel-seeding');
    await panel.getByRole('button', {name: '+ Field channel', exact: true}).click();
    const c = panel.locator('[data-component-id]').first();
    await setProperty(c, 'Origin x', 999);
    await page.click('#seed-apply');
    await expect(page.locator('#seed-status')).toContainText('integer from');
    expect((await read(page)).ownerId).toBe(before.ownerId);
    await setProperty(c, 'Origin x', 1);
    await page.route('**/api/lattice/records/seed', route => route.fulfill({status: 400, contentType: 'application/json', body: '{"error":"Injected compile failure"}'}));
    await page.click('#seed-apply');
    await expect(page.locator('#seed-status')).toContainText('Injected compile failure');
    expect((await read(page)).ownerId).toBe(before.ownerId);
    expect(await digest(page)).toBe(beforeDigest);
    await page.unroute('**/api/lattice/records/seed');
    let release, entered;
    const gate = new Promise(r => {release = r;}), pending = new Promise(r => {entered = r;});
    await page.route('**/api/lattice/records/seed', async route => {entered(); await gate; await route.abort().catch(() => {});});
    await page.click('#seed-preview'); await pending;
    await panel.getByRole('button', {name: 'Cancel preparation'}).click();
    release();
    await expect(page.locator('#seed-status')).toHaveText('Preparation cancelled.');
    expect((await read(page)).ownerId).toBe(before.ownerId);
    expect(await digest(page)).toBe(beforeDigest);
    await page.unroute('**/api/lattice/records/seed');
    await page.locator('#seed-import').setInputFiles({name: 'bad.json', mimeType: 'application/json', buffer: Buffer.from('{"version":999}')});
    await expect(page.locator('#seed-status')).toContainText('unexpected or missing');
    expect(await digest(page)).toBe(beforeDigest);
    // Switching scales during compilation disposes the panel and detached owner.
    let releaseLate, enteredLate;
    const lateGate = new Promise(r => {releaseLate = r;}), latePending = new Promise(r => {enteredLate = r;});
    await page.route('**/api/lattice/records/seed', async route => {enteredLate(); await lateGate; await route.abort().catch(() => {});});
    await page.click('#seed-apply'); await latePending;
    await page.selectOption('#engine-mode', 'particles');
    releaseLate(); await page.unroute('**/api/lattice/records/seed');
    await expect(page.locator('#panel-seeding')).not.toBeVisible();
    await page.selectOption('#engine-mode', 'lattice');
    await page.evaluate(() => window.__ftdCtx.appShell.panelDock.activate('seeding'));
    await expect(page.locator('#seed-picker')).toBeVisible();
});

test('every registered form and category base exposes complete numeric editors without changing the live state', async ({page}) => {
    test.setTimeout(900000);
    await start(page);
    const before = await digest(page);
    const scenarios = await page.evaluate(async () => (await import('/js/scales/scale0/scenario-registry.js')).getScale0SeedingScenarios());
    const panel = page.locator('#panel-seeding'), categories = new Set();
    for (const row of scenarios) {
        await page.selectOption('#seed-scenario', row.id);
        await expect(panel).toHaveAttribute('data-preparation', row.id, {timeout: 60000});
        await expect(panel).toHaveAttribute('data-seed-ready', 'true', {timeout: 60000});
        const errors = await panel.evaluate(host => [...host.querySelectorAll('.seed-property')].flatMap(el => {
            const key=el.dataset.propertyKey, number=el.querySelector('input[type=number]');
            return !number || !number.dataset.uiTooltip?.includes('Recommended:') || !el.querySelector('.seed-property-help')
                || !el.querySelector('.seed-property-reset') || number.value === '' ? [key] : [];
        }));
        expect(errors, row.id).toEqual([]);
        expect(await panel.locator('.seed-property').count(), row.id).toBeGreaterThan(0);
        if (!categories.has(row.category)) {
            categories.add(row.category);
            await panel.getByRole('button', {name:'Open full category base'}).click();
            await expect(panel).toHaveAttribute('data-seed-base', 'true', {timeout: 60000});
            await expect(panel).toHaveAttribute('data-seed-ready', 'true', {timeout: 60000});
            const expected = row.backend === 'finite-records' ? 9 : scenarios.filter(s => s.backend !== 'finite-records' && s.category === row.category).length;
            await expect(panel.locator('[data-component-id]')).toHaveCount(expected);
            await expect(panel.locator('[data-component-id][data-enabled="false"]')).toHaveCount(expected);
            await panel.getByRole('button', {name: 'Restore preset', exact:true}).click();
            await expect(panel).toHaveAttribute('data-seed-base', 'false', {timeout: 60000});
        }
    }
    expect(scenarios).toHaveLength(772);
    expect(categories.size).toBe(22);
    expect(await digest(page)).toBe(before);
});

test('Escape dismisses inline help and the global tooltip without leaking simulation shortcuts', async ({page}) => {
    await start(page);
    const panel = page.locator('#panel-seeding');
    const help = panel.locator('.seed-property-help').first();
    await help.click();
    const explanation = panel.locator('.seed-help-text').first();
    await expect(explanation).toBeVisible();
    await page.keyboard.press('Escape');
    await expect(explanation).toBeHidden();
    expect(await page.evaluate(() => window.__ftdCtx.running)).toBe(false);
    // A held-focus number input still lets Space reach the input (not a play shortcut).
    const input = panel.locator('.seed-property input[type="number"]').first();
    await input.focus();
    await page.keyboard.press('ArrowUp');
    expect(await page.evaluate(() => window.__ftdCtx.running)).toBe(false);
});

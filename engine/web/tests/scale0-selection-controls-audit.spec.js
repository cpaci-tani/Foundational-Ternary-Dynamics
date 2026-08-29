// @ts-check
import { test, expect } from '@playwright/test';
import {
    attachConsoleWatcher,
    gotoAndReady,
    realErrors,
    switchMode,
} from './_helpers.js';

test.describe('Scale 0 Selection controls-card audit gate', () => {
    test.beforeEach(async ({ page }, testInfo) => {
        testInfo.setTimeout(120_000);
        page.setDefaultTimeout(30_000);
        await gotoAndReady(page);
        await page.waitForFunction(() => window.__ftdCtx?.fluxMock?.ready === true);
    });

    test('inventory, inspector handoff, clear semantics, and area resource reuse are exact', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const ctx = window.__ftdCtx;
            const card = document.getElementById('sel-card');
            const sceneCore = ctx.viewport._sceneCore;
            const ids = [
                'sel-x', 'sel-y', 'sel-z', 'sel-area-toggle', 'btn-select',
                'sel-area-controls', 'sel-radius', 'sel-radius-val',
            ];
            const initial = {
                missing: ids.filter((id) => !document.getElementById(id)),
                duplicates: ids.filter((id) => document.querySelectorAll(`#${id}`).length !== 1),
                title: card?.querySelector('.card-title')?.textContent,
                nodes: card?.querySelectorAll('*').length,
                inputs: card?.querySelectorAll('input').length,
                steppers: card?.querySelectorAll('.sel-coord-step').length,
                axes: card?.querySelectorAll('.sel-axis-btn').length,
                max: ['x', 'y', 'z'].map((axis) => document.getElementById(`sel-${axis}`).max),
                publicSelection: typeof ctx.inspector?.selectLatticePosition,
                voxelVisible: !!sceneCore._voxelHighlight?.visible,
                areaVisible: !!sceneCore._areaHighlight?.visible,
            };

            const x = document.getElementById('sel-x');
            x.value = '11';
            x.dispatchEvent(new Event('change', { bubbles: true }));
            const afterCoordinate = {
                position: sceneCore._voxelHighlight?.position.toArray(),
                voxelVisible: sceneCore._voxelHighlight?.visible,
                areaVisible: !!sceneCore._areaHighlight?.visible,
            };

            document.getElementById('sel-area-toggle').click();
            const area = sceneCore._areaHighlight;
            const geometry = area.geometry;
            const material = area.material;
            const firstArea = {
                radius: sceneCore._areaHighlightRadius,
                scale: area.scale.toArray(),
                position: area.position.toArray(),
                visible: area.visible,
            };

            const radius = document.getElementById('sel-radius');
            for (let i = 0; i < 400; i++) {
                radius.value = String(1 + (i % 10));
                radius.dispatchEvent(new Event('input', { bubbles: true }));
            }
            await new Promise((resolve) => requestAnimationFrame(resolve));
            const afterRadius = {
                sameArea: area === sceneCore._areaHighlight,
                sameGeometry: geometry === sceneCore._areaHighlight.geometry,
                sameMaterial: material === sceneCore._areaHighlight.material,
                radius: sceneCore._areaHighlightRadius,
                scale: sceneCore._areaHighlight.scale.toArray(),
                display: document.getElementById('sel-radius-val').textContent,
            };

            document.getElementById('sel-y').value = '7';
            document.getElementById('sel-z').value = '5';
            document.getElementById('btn-select').click();
            const afterSelect = {
                selected: ctx.inspector.getSelectedLatticePosition(),
                fields: ['x', 'y', 'z'].map((axis) => Number(document.getElementById(`sel-${axis}`).value)),
                voxelPosition: sceneCore._voxelHighlight.position.toArray(),
                areaPosition: sceneCore._areaHighlight.position.toArray(),
                voxelVisible: sceneCore._voxelHighlight.visible,
                areaVisible: sceneCore._areaHighlight.visible,
            };
            ctx.inspector.clearSelection();
            const afterClear = {
                selected: ctx.inspector.getSelectedLatticePosition(),
                voxelVisible: sceneCore._voxelHighlight.visible,
                areaVisible: sceneCore._areaHighlight.visible,
            };
            return { initial, afterCoordinate, firstArea, afterRadius, afterSelect, afterClear };
        });

        expect(result.initial).toEqual({
            missing: [], duplicates: [], title: 'Selection', nodes: expect.any(Number),
            inputs: 4, steppers: 6, axes: 6, max: ['32', '32', '32'],
            publicSelection: 'function', voxelVisible: false, areaVisible: false,
        });
        expect(result.initial.nodes).toBeGreaterThan(30);
        expect(result.afterCoordinate).toEqual({
            position: [11.5, 16.5, 16.5], voxelVisible: true, areaVisible: false,
        });
        expect(result.firstArea).toEqual({
            radius: 2, scale: [5, 5, 5], position: [11.5, 16.5, 16.5], visible: true,
        });
        expect(result.afterRadius).toEqual({
            sameArea: true, sameGeometry: true, sameMaterial: true,
            radius: 10, scale: [21, 21, 21], display: '10',
        });
        expect(result.afterSelect).toEqual({
            selected: { x: 11, y: 7, z: 5 }, fields: [11, 7, 5],
            voxelPosition: [11.5, 7.5, 5.5], areaPosition: [11.5, 7.5, 5.5],
            voxelVisible: true, areaVisible: true,
        });
        expect(result.afterClear).toEqual({ selected: null, voxelVisible: false, areaVisible: false });
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('input bursts coalesce, bounds reconcile, and stale-scale events cannot commit', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await page.goto('/js/scales/scale0/ui/controls/flux-volume.js', { waitUntil: 'domcontentloaded' });
        const result = await page.evaluate(async () => {
            const { createSelectionCard } = await import(
                '/js/scales/scale0/ui/controls/flux-volume.js?gate9-card=1'
            );
            const { wireScale0Controls } = await import(
                '/js/scales/scale0/ui/controls/wire.js?gate9-wire=1'
            );
            const host = document.createElement('div');
            host.id = 'panel-controls';
            const grid = document.createElement('div');
            grid.id = 'panel-controls-grid';
            grid.append(createSelectionCard());
            host.append(grid);
            document.body.append(host);

            const calls = [];
            let selected = null;
            const inspector = {
                getSelectedLatticePosition: () => selected,
                selectLatticePosition: (value) => {
                    selected = { ...value };
                    calls.push(['select', value.x, value.y, value.z]);
                    return true;
                },
            };
            const viewport = {
                setVoxelHighlight: (x, y, z, active) => calls.push(['voxel', x, y, z, active]),
                setAreaHighlight: (x, y, z, radius, active) => calls.push(['area', x, y, z, radius, active]),
            };
            const ctx = {
                bridge: { latticeSize: 33 }, viewport, inspector,
                engineMode: 'lattice', _loadGeneration: 1,
            };
            wireScale0Controls(ctx, { setLatticeNeedsUpload: () => {} });
            document.getElementById('sel-area-toggle').click();
            calls.length = 0;

            const card = document.getElementById('sel-card');
            const before = { nodes: card.querySelectorAll('*').length, inputs: card.querySelectorAll('input').length };
            const mutations = { records: 0, characterData: 0, added: 0, removed: 0 };
            const observer = new MutationObserver((list) => {
                for (const mutation of list) {
                    mutations.records++;
                    if (mutation.type === 'characterData') mutations.characterData++;
                    mutations.added += mutation.addedNodes?.length || 0;
                    mutations.removed += mutation.removedNodes?.length || 0;
                }
            });
            observer.observe(card, { subtree: true, childList: true, attributes: true, characterData: true });
            const radius = document.getElementById('sel-radius');
            for (let i = 0; i < 600; i++) {
                radius.value = '7';
                radius.dispatchEvent(new Event('input', { bubbles: true }));
            }
            const immediate = [...calls];
            await new Promise((resolve) => requestAnimationFrame(resolve));
            const afterFrame = [...calls];
            await new Promise((resolve) => requestAnimationFrame(resolve));
            const afterSecondFrame = [...calls];
            observer.disconnect();

            for (const axis of ['x', 'y', 'z']) document.getElementById(`sel-${axis}`).value = '32';
            selected = { x: 32, y: 32, z: 32 };
            calls.length = 0;
            ctx.syncScale0SelectionBounds(17);
            const reconciled = {
                fields: ['x', 'y', 'z'].map((axis) => Number(document.getElementById(`sel-${axis}`).value)),
                max: ['x', 'y', 'z'].map((axis) => document.getElementById(`sel-${axis}`).max),
                selected,
                calls: [...calls],
            };

            calls.length = 0;
            document.dispatchEvent(new CustomEvent('ftd:voxel-selected', {
                detail: { x: 3, y: 4, z: 5 },
            }));
            const syncedEvent = {
                fields: ['x', 'y', 'z'].map((axis) => Number(document.getElementById(`sel-${axis}`).value)),
                calls: [...calls],
            };
            calls.length = 0;
            document.dispatchEvent(new CustomEvent('ftd:voxel-selection-cleared'));
            const clearedCalls = [...calls];

            calls.length = 0;
            radius.value = '9';
            radius.dispatchEvent(new Event('input', { bubbles: true }));
            ctx.engineMode = 'particles';
            document.dispatchEvent(new CustomEvent('ftd:voxel-selected', {
                detail: { x: 8, y: 8, z: 8 },
            }));
            await new Promise((resolve) => requestAnimationFrame(resolve));
            const stale = {
                calls: [...calls],
                display: document.getElementById('sel-radius-val').textContent,
                fields: ['x', 'y', 'z'].map((axis) => Number(document.getElementById(`sel-${axis}`).value)),
            };
            return {
                before,
                after: { nodes: card.querySelectorAll('*').length, inputs: card.querySelectorAll('input').length },
                immediate, afterFrame, afterSecondFrame, mutations,
                display: document.getElementById('sel-radius-val').textContent,
                reconciled, syncedEvent, clearedCalls, stale,
            };
        });

        expect(result.immediate).toEqual([]);
        expect(result.afterFrame).toEqual([
            ['voxel', 16, 16, 16, true], ['area', 16, 16, 16, 7, true],
        ]);
        expect(result.afterSecondFrame).toEqual(result.afterFrame);
        expect(result.mutations).toEqual({ records: 1, characterData: 1, added: 0, removed: 0 });
        expect(result.before).toEqual(result.after);
        expect(result.reconciled).toEqual({
            fields: [16, 16, 16], max: ['16', '16', '16'], selected: { x: 16, y: 16, z: 16 },
            calls: [
                ['select', 16, 16, 16],
                ['voxel', 16, 16, 16, true], ['area', 16, 16, 16, 7, true],
            ],
        });
        expect(result.syncedEvent).toEqual({ fields: [3, 4, 5], calls: [['area', 3, 4, 5, 7, true]] });
        expect(result.clearedCalls).toEqual([['area', 0, 0, 0, 1, false]]);
        expect(result.stale).toEqual({ calls: [], display: '7', fields: [3, 4, 5] });
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('selection state is safely hidden across a Scale 1 round trip and reactivates on demand', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await page.evaluate(() => {
            document.getElementById('sel-area-toggle').click();
            document.getElementById('sel-x').value = '9';
            document.getElementById('sel-y').value = '8';
            document.getElementById('sel-z').value = '7';
            document.getElementById('btn-select').click();
        });
        await switchMode(page, 'particles');
        const inScale1 = await page.evaluate(() => ({
            mode: window.__ftdCtx.engineMode,
            voxel: !!window.__ftdCtx.viewport._sceneCore._voxelHighlight?.visible,
            area: !!window.__ftdCtx.viewport._sceneCore._areaHighlight?.visible,
        }));
        await switchMode(page, 'lattice');
        await page.waitForFunction(() => window.__ftdCtx?.fluxMock?.ready === true);
        const afterReentry = await page.evaluate(() => {
            const fields = ['x', 'y', 'z'].map((axis) => Number(document.getElementById(`sel-${axis}`).value));
            const toggle = document.getElementById('sel-area-toggle').dataset.active;
            const sceneCore = window.__ftdCtx.viewport._sceneCore;
            const beforeMove = { voxel: !!sceneCore._voxelHighlight?.visible, area: !!sceneCore._areaHighlight?.visible };
            document.querySelector('#sel-card .sel-axis-btn[data-axis="x"][data-dir="1"]').click();
            return {
                mode: window.__ftdCtx.engineMode, fields, toggle, beforeMove,
                afterMove: {
                    x: Number(document.getElementById('sel-x').value),
                    voxel: sceneCore._voxelHighlight.visible,
                    area: sceneCore._areaHighlight.visible,
                    voxelPosition: sceneCore._voxelHighlight.position.toArray(),
                    areaPosition: sceneCore._areaHighlight.position.toArray(),
                },
            };
        });

        expect(inScale1).toEqual({ mode: 'particles', voxel: false, area: false });
        expect(afterReentry).toEqual({
            mode: 'lattice', fields: [9, 8, 7], toggle: 'true',
            beforeMove: { voxel: false, area: false },
            afterMove: {
                x: 10, voxel: true, area: true,
                voxelPosition: [10.5, 8.5, 7.5], areaPosition: [10.5, 8.5, 7.5],
            },
        });
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});

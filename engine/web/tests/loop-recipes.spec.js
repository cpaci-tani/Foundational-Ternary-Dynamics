// @ts-check
/**
 * Loop recipes on the real WASM engine.
 *
 * engine/web/recipes/loop-repair-seed.json and loop-pair-seed.json are staged
 * exactly as the dashboard stages an imported recipe: parseRecipeEnvelope,
 * then nativeOverrides, then setupScenarioSeed on a detached bridge. The
 * assertions are the measured behaviours the recipes exist to show
 * (docs/superpowers/specs/2026-10-09-loop-recipes-design.md):
 *
 *   Loop repair  Gauss projection removes the longitudinal mode and leaves the
 *                loop content (the engine's own centred curl) unchanged.
 *   Loop pair    under the free wave map the coherent ring's circulation
 *                reverses sign between ticks 2 and 3, while the sign-scrambled
 *                control reads zero until the coherent ring's waves reach it.
 *
 * No dashboard simulation is started by this isolated document.
 */
import { test, expect } from '@playwright/test';

// The same two preparations as canned scenario ids. The native behavioural
// test (engine/tests/test_flux_cell_scenario_physics.cpp) is the test of
// record; this confirms the deployed WASM engine carries the constructors and
// gives the same physics, for every variant the dashboard can load.
const variants = [
    { id: 'wasm32', loader: 'ftd_core.js', factory: 'createFTDModule' },
    { id: 'wasm64', loader: 'ftd_core64.js', factory: 'createFTDModule64' },
    { id: 'wasm32-threads', loader: 'ftd_core_mt.js', factory: 'createFTDModuleMT' },
];

for (const variant of variants) {
    test(`${variant.id}: canned loop scenarios behave as on native`, async ({ page }, testInfo) => {
        test.setTimeout(180000);
        await page.goto('/wasm/build_info.json');
        await page.addScriptTag({ url: `/wasm/${variant.loader}` });
        const result = await page.evaluate(async variant => {
            if (variant.id === 'wasm32-threads' && !crossOriginIsolated) throw new Error('Threads require actual cross-origin isolation');
            const mod = await globalThis[variant.factory]({
                locateFile: file => `/wasm/${file}`,
                mainScriptUrlOrBlob: `${location.origin}/wasm/${variant.loader}`,
                print: () => {},
            });
            const N = 33, mid = 16;
            const make = id => {
                const rb = new mod.RenderBridge(N);
                if (!mod.setupScenario(rb, id)) { rb.delete(); throw new Error(`${id} is not in this engine build`); }
                return rb;
            };
            const active = rb => ['wave_propagation', 'gauss_projection', 'genesis', 'damping', 'coupling', 'forces', 'movement']
                .filter(name => mod.getToggle(rb, name));
            function norms(rb) {
                let flux = 0, curl = 0, div = 0, manifested = 0;
                for (let z = 0; z < N; z++) for (let y = 0; y < N; y++) for (let x = 0; x < N; x++) {
                    const v = mod.inspectVoxel(rb, x, y, z);
                    flux += v.fluxX * v.fluxX + v.fluxY * v.fluxY + v.fluxZ * v.fluxZ;
                    curl += v.curlX * v.curlX + v.curlY * v.curlY + v.curlZ * v.curlZ;
                    div += v.divJ * v.divJ;
                    if (v.state !== 0) manifested++;
                }
                return { flux, curl, div, manifested };
            }
            function circulation(rb, cx, cy, h, zc) {
                const J = (x, y) => mod.inspectVoxel(rb, x, y, zc);
                let gamma = 0;
                for (let x = cx - h; x < cx + h; x++) {
                    gamma += 0.5 * (J(x, cy - h).fluxX + J(x + 1, cy - h).fluxX);
                    gamma -= 0.5 * (J(x, cy + h).fluxX + J(x + 1, cy + h).fluxX);
                }
                for (let y = cy - h; y < cy + h; y++) {
                    gamma += 0.5 * (J(cx + h, y).fluxY + J(cx + h, y + 1).fluxY);
                    gamma -= 0.5 * (J(cx - h, y).fluxY + J(cx - h, y + 1).fluxY);
                }
                return gamma;
            }

            const repair = make('s0-cell-loop-repair');
            const repairActive = active(repair);
            const before = norms(repair);
            for (let t = 0; t < 50; t++) repair.tick();
            const after = norms(repair);
            repair.delete();

            const pair = make('s0-cell-loop-pair');
            const pairActive = active(pair);
            const coherent = [], control = [];
            for (let t = 0; t <= 8; t++) {
                if (t > 0) pair.tick();
                coherent.push(circulation(pair, 8, mid, 4, mid));
                control.push(circulation(pair, 24, mid, 4, mid));
            }
            pair.delete();
            return { repairActive, before, after, pairActive, coherent, control };
        }, variant);
        await testInfo.attach(`loop-scenarios-${variant.id}.json`, { body: JSON.stringify(result, null, 2), contentType: 'application/json' });

        // Loop repair. Sums of squares, as in the native test: ring 11.10 and
        // blob 14.46 at tick 0, the ring alone at tick 50.
        expect(result.repairActive).toEqual(['gauss_projection']);
        const { before, after } = result;
        expect(before.manifested + after.manifested).toBe(0);
        expect(before.flux).toBeGreaterThan(25.0);
        expect(before.flux).toBeLessThan(26.2);
        expect(Math.abs(after.curl / before.curl - 1)).toBeLessThan(1e-12);
        expect(after.div / before.div).toBeLessThan(1e-6);
        expect(after.flux).toBeGreaterThan(11.0);
        expect(after.flux).toBeLessThan(11.2);

        // Loop pair, by the square lattice loop of half-width 4.
        expect(result.pairActive).toEqual(['wave_propagation']);
        const { coherent, control } = result;
        expect(coherent[0]).toBeGreaterThan(5);
        expect(coherent[2]).toBeGreaterThan(0);
        expect(coherent[3]).toBeLessThan(0);
        expect(coherent[5]).toBeLessThan(-1);
        for (const gamma of control.slice(0, 5)) expect(Math.abs(gamma)).toBeLessThan(1e-3);
        for (const gamma of control) expect(Math.abs(gamma)).toBeLessThan(0.005 * coherent[0]);
    });
}

test('loop recipes stage and behave as specified', async ({ page }, testInfo) => {
    test.setTimeout(180000);
    await page.goto('/wasm/build_info.json');
    await page.addScriptTag({ url: '/wasm/ftd_core.js' });
    const result = await page.evaluate(async () => {
        const mod = await globalThis.createFTDModule({ locateFile: file => `/wasm/${file}`, print: () => {} });
        const { nativeOverrides } = await import('/js/seeding/native-runtime.js');
        const { parseRecipeEnvelope } = await import('/js/seeding/recipe.js');
        const { SCALE0_SCENARIOS } = await import('/js/scales/scale0/scenario-registry.js');

        async function stage(name) {
            const response = await fetch(`/recipes/${name}`);
            if (!response.ok) throw new Error(`${name}: HTTP ${response.status}`);
            const recipe = parseRecipeEnvelope(await response.text(), SCALE0_SCENARIOS);
            const rb = new mod.RenderBridge(recipe.size);
            const description = JSON.parse(mod.setupScenarioSeed(rb, recipe.scenarioId, nativeOverrides(recipe)));
            if (description.error) { rb.delete(); throw new Error(`${name}: ${description.error}`); }
            // Term toggles are described as Off/On choices; boundary, stencil
            // and bath settings are choices with other option labels.
            const isToggle = p => /^protocol\.[a-z_0-9]+$/.test(p.key) && p.type === 'choice'
                && p.options.length === 2 && p.options[0][1] === 'Off' && p.options[1][1] === 'On';
            const active = description.properties.filter(p => isToggle(p) && p.value === 1)
                .map(p => p.key.slice('protocol.'.length)).sort();
            return { rb, N: recipe.size, active };
        }

        // Whole-lattice norms from the engine's own per-voxel diagnostics.
        function norms(rb, N) {
            let flux = 0, curl = 0, div = 0, manifested = 0;
            for (let z = 0; z < N; z++) for (let y = 0; y < N; y++) for (let x = 0; x < N; x++) {
                const v = mod.inspectVoxel(rb, x, y, z);
                flux += v.fluxX * v.fluxX + v.fluxY * v.fluxY + v.fluxZ * v.fluxZ;
                curl += v.curlX * v.curlX + v.curlY * v.curlY + v.curlZ * v.curlZ;
                div += v.divJ * v.divJ;
                if (v.state !== 0) manifested++;
            }
            return { flux: Math.sqrt(flux), curl: Math.sqrt(curl), div: Math.sqrt(div), manifested };
        }

        // Circulation round the axis-aligned square of half-width h centred on
        // site (cx, cy) in the plane z = zc, counter-clockwise seen from +z,
        // by the trapezoid rule along lattice edges.
        function circulation(rb, cx, cy, h, zc) {
            const J = (x, y) => mod.inspectVoxel(rb, x, y, zc);
            let gamma = 0;
            for (let x = cx - h; x < cx + h; x++) {
                gamma += 0.5 * (J(x, cy - h).fluxX + J(x + 1, cy - h).fluxX);
                gamma -= 0.5 * (J(x, cy + h).fluxX + J(x + 1, cy + h).fluxX);
            }
            for (let y = cy - h; y < cy + h; y++) {
                gamma += 0.5 * (J(cx + h, y).fluxY + J(cx + h, y + 1).fluxY);
                gamma -= 0.5 * (J(cx - h, y).fluxY + J(cx - h, y + 1).fluxY);
            }
            return gamma;
        }

        const repair = await stage('loop-repair-seed.json');
        const before = norms(repair.rb, repair.N);
        for (let t = 0; t < 20; t++) repair.rb.tick();
        const after = norms(repair.rb, repair.N);
        repair.rb.delete();

        const pair = await stage('loop-pair-seed.json');
        const mid = (pair.N - 1) / 2;
        const coherent = [], control = [];
        for (let t = 0; t <= 8; t++) {
            if (t > 0) pair.rb.tick();
            coherent.push(circulation(pair.rb, 8, mid, 4, mid));
            control.push(circulation(pair.rb, 24, mid, 4, mid));
        }
        pair.rb.delete();

        return {
            repair: { active: repair.active, before, after },
            pair: { active: pair.active, coherent, control },
        };
    });

    await testInfo.attach('loop-recipes.json', { body: JSON.stringify(result, null, 2), contentType: 'application/json' });

    // Loop repair: projection is the only active term, and it is a repair of
    // the gradient part only.
    expect(result.repair.active).toEqual(['gauss_projection']);
    expect(result.repair.before.manifested).toBe(0);
    expect(result.repair.after.manifested).toBe(0);
    const { before, after } = result.repair;
    expect(Math.abs(after.curl / before.curl - 1)).toBeLessThan(1e-6);
    expect(after.div / before.div).toBeLessThan(1e-2);
    expect(after.flux / before.flux).toBeGreaterThan(0.38);
    expect(after.flux / before.flux).toBeLessThan(0.40);

    // Loop pair: the rings' own isolated free-wave profile.
    expect(result.pair.active).toEqual(['wave_propagation']);
    const { coherent, control } = result.pair;
    expect(coherent[0]).toBeGreaterThan(5);
    // The reversal: positive through tick 2, negative from tick 3.
    expect(coherent[2]).toBeGreaterThan(0);
    expect(coherent[3]).toBeLessThan(0);
    expect(coherent[5]).toBeLessThan(-1);
    // The control is exactly circulation-free while it is still causally
    // separate from the coherent ring, which covers the reversal ...
    for (const gamma of control.slice(0, 5)) expect(Math.abs(gamma)).toBeLessThan(1e-3);
    // ... and stays a clean control (under half a percent of the coherent
    // ring's starting value) until the coherent ring's waves arrive.
    for (const gamma of control) expect(Math.abs(gamma)).toBeLessThan(0.005 * coherent[0]);
});

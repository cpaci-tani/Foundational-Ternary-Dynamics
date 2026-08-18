// @ts-check
/**
 * Scale-0 side-panel request budgets.
 *
 * These tests intentionally use native-shaped bridge fakes: the important
 * regression is command fan-out, not a particular physics fixture.  The
 * values returned by the fakes are ordinary measured field/voxel records, so
 * the panels must retain real-data paths while reducing request cadence.
 */
import { test, expect } from '@playwright/test';

test('anisotropy consumes one bounded flux volume instead of 128 point reads', async ({ page }) => {
    await page.goto('/index_dag.html', { waitUntil: 'domcontentloaded' });
    const result = await page.evaluate(async () => {
        const { AnisotropyComponent } = await import(
            '/js/scales/scale0/ui/overlays/p1-observables/anisotropy.js?request-budget=1'
        );
        const axis = 4;
        const data = new Float32Array(axis ** 3);
        for (let i = 0; i < data.length; i++) data[i] = i + 1;
        let volumeReads = 0;
        let inspectReads = 0;
        const bridge = {
            isNativeGPU: true,
            latticeSize: 8,
            getScale0ParticleList: () => [{ id: 7, state: 1, x: 4, y: 4, z: 4 }],
            getFluxVolume: () => {
                volumeReads++;
                return { data, latticeSize: 8, stride: 2, axisCount: axis };
            },
            inspectVoxel: () => { inspectReads++; return { Emag: 99 }; },
        };
        const component = new AnisotropyComponent();
        component.mount(document.body);
        component.update(bridge, 0);
        component.update(bridge, 250);
        component.update(bridge, 1000);
        const text = component.element.textContent || '';
        component.unmount();
        return { volumeReads, inspectReads, text };
    });
    expect(result.volumeReads, 'native anisotropy is sampled at ≤1 Hz').toBe(2);
    expect(result.inspectReads, 'no 8×16 inspectVoxel fan-out').toBe(0);
    expect(result.text).toContain('stride 2a');
});

test('native lattice inspector stages Moore reads and stays quiet for a stable epoch', async ({ page }) => {
    await page.goto('/index_dag.html', { waitUntil: 'domcontentloaded' });
    const result = await page.evaluate(async () => {
        const { updateLatticeFields } = await import(
            '/js/inspector/scales/lattice.js?request-budget=1'
        );
        const makeField = () => document.createElement('span');
        const names = [
            'id', 'state', 'pos', 'spin', 'color', 'pair', 'locked', 'flux',
            'density', 'divj', 'curl', 'vel', 'speed', 'accel', 'eMag', 'bMag',
            'fCoulomb', 'fGravity', 'fMagnetic', 'fStrong', 'fExchange',
        ];
        const fields = Object.fromEntries(names.map((name) => [name, makeField()]));
        const grid = document.createElement('div');
        grid.id = 'insp-moore-grid';
        document.body.appendChild(grid);
        let inspectCalls = 0;
        let forceCalls = 0;
        const voxel = {
            state: 0, particleId: -1, spin: 0, color: 0, pairId: -1, locked: false,
            fluxX: 0.1, fluxY: 0, fluxZ: 0, density: 0.1, divJ: 0,
            curlX: 0, curlY: 0, curlZ: 0, velX: 0, velY: 0, velZ: 0,
            speed: 0, accelMag: 0, Emag: 0.1, Bmag: 0,
        };
        const target = {
            _selectedPos: { x: 8, y: 8, z: 8 },
            bridge: {
                isNativeGPU: true,
                latticeSize: 16,
                _visualEpoch: 4,
                inspectVoxel: () => { inspectCalls++; return { ...voxel }; },
                getForceAt: () => {
                    forceCalls++;
                    return { coulombMag: 0, gravityMag: 0, magneticMag: 0, strongMag: 0, exchangeMag: 0 };
                },
            },
            fields,
        };
        // First pass: centre plus only nine of 26 neighbours.
        updateLatticeFields(target);
        const first = { inspectCalls, forceCalls, cells: grid.querySelectorAll('div div div').length };
        // The render loop may call again immediately; no new native reads.
        updateLatticeFields(target);
        const immediate = { inspectCalls, forceCalls };
        // Drain the two remaining bounded neighbour batches without waiting in
        // real time; the epoch stays stable so the final call must be silent.
        target._latticeInspectionCache.lastRequestAt = -Infinity;
        updateLatticeFields(target);
        target._latticeInspectionCache.lastRequestAt = -Infinity;
        updateLatticeFields(target);
        const afterDrain = { inspectCalls, forceCalls, neighbours: target._latticeInspectionCache.neighbours.size };
        target._latticeInspectionCache.lastRequestAt = -Infinity;
        updateLatticeFields(target);
        const stable = { inspectCalls, forceCalls };
        grid.remove();
        return { first, immediate, afterDrain, stable };
    });
    expect(result.first.inspectCalls, 'centre + bounded nine-neighbour batch').toBe(10);
    expect(result.first.forceCalls).toBe(1);
    expect(result.immediate).toEqual({ inspectCalls: 10, forceCalls: 1 });
    expect(result.afterDrain.neighbours, 'all 26 real neighbours eventually resolve').toBe(26);
    expect(result.stable, 'no repeated reads for unchanged visual epoch').toEqual({ inspectCalls: 30, forceCalls: 3 });
});

test('default flux-slice rows mirror visual toggles except the flagship |J| slice', async ({ page }) => {
    await page.goto('/index_dag.html', { waitUntil: 'domcontentloaded' });
    const result = await page.evaluate(async () => {
        const { DEFAULT_FIELD_OVERRIDE, FIELD_DRIVERS } = await import(
            '/js/scales/scale0/ui/overlays/flux-slice-helpers.js?request-budget=1'
        );
        return {
            forcedOn: FIELD_DRIVERS.filter((driver) => DEFAULT_FIELD_OVERRIDE[driver.key] === 'on')
                .map((driver) => driver.key),
            mirrored: FIELD_DRIVERS.filter((driver) => DEFAULT_FIELD_OVERRIDE[driver.key] === null).length,
            total: FIELD_DRIVERS.length,
        };
    });
    expect(result.forcedOn).toEqual(['fluxJ']);
    expect(result.mirrored).toBe(result.total - 1);
});

test('Coulomb and PhysicsHarness retain an explicit high-resolution path while native UI probes are bounded', async ({ page }) => {
    await page.goto('/index_dag.html', { waitUntil: 'domcontentloaded' });
    const result = await page.evaluate(async () => {
        const { PhysicsHarness } = await import('/js/physics/physics-harness.js?request-budget=1');
        const { CoulombComponent } = await import(
            '/js/scales/scale0/ui/overlays/p1-observables/coulomb.js?request-budget=1'
        );
        const requestedStrides = [];
        const harness = new PhysicsHarness({
            latticeSize: 16,
            getEFieldSampled(stride) {
                requestedStrides.push(stride);
                return { positions: new Float32Array(0), vectors: new Float32Array(0), count: 0 };
            },
        });
        harness.sampleEFieldAlongRay({ x: 0, y: 0, z: 0 }, { x: 4, y: 0, z: 0 }, 4);
        harness.sampleEFieldAlongRay({ x: 0, y: 0, z: 0 }, { x: 4, y: 0, z: 0 }, 4, { stride: 3 });

        let probeCalls = 0;
        const bridge = {
            isNativeGPU: true,
            latticeSize: 64,
            sampleVAtRay() {
                probeCalls++;
                return { V: new Float32Array([1, 0.6, 0.2]), count: 3 };
            },
        };
        const particles = [
            { id: 1, state: 1, charge: 1, x: 10, y: 10, z: 10 },
            { id: 2, state: -1, charge: -1, x: 18, y: 10, z: 10 },
        ];
        const component = new CoulombComponent();
        component.mount(document.body);
        component.update(bridge, 0, particles, 'pair');
        component.update(bridge, 250, particles, 'pair');
        component.update(bridge, 1000, particles, 'pair');
        component.unmount();
        return { requestedStrides, probeCalls };
    });
    expect(result.requestedStrides, 'default remains high resolution; callers may opt into a bounded grid')
        .toEqual([1, 3]);
    expect(result.probeCalls, 'native Coulomb panel probes no faster than 1 Hz').toBe(2);
});

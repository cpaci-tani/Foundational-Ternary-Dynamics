import { test, expect } from '@playwright/test';
import { readFileSync } from 'node:fs';
import { attachConsoleWatcher, gotoAndReady, realErrors, switchMode } from './_helpers.js';

test.describe('Scale 4 Solar System', () => {
    test('canonical registry, navigation manifest, toolbar, and bridge admit the same dynamic scenarios', async ({ page }) => {
        const metadata = JSON.parse(readFileSync(new URL('../../config/scenarios/scale4.json', import.meta.url), 'utf8'));
        await page.goto('/js/constants.js');
        const result = await page.evaluate(async () => {
            const [registry, { PlanetaryMockBridge }] = await Promise.all([
                import('/js/scales/scale4/scenario-registry.js?scale4-registry-contract=1'),
                import('/js/bridge/mock-scale4.js?scale4-registry-contract=1'),
            ]);
            const select = document.createElement('select');
            registry.populateScale4ScenarioSelect(select);
            const options = [...select.options].map((option) => option.value);
            const scenarios = [];
            for (const scenario of registry.SCALE4_SCENARIOS) {
                const bridge = new PlanetaryMockBridge();
                bridge.setupScenario(scenario.id);
                const before = bridge.getBodies().map((body) => ({ id: body.id, x: body.x, y: body.y, z: body.z }));
                bridge.run(2);
                const after = bridge.getBodies();
                scenarios.push({
                    id: scenario.id,
                    actual: bridge.getDiagnostics().scenario,
                    beforeCount: before.length,
                    afterCount: after.length,
                    tick: bridge.getDiagnostics().tick,
                    time: bridge.getDiagnostics().timeYears,
                    finite: after.every((body) => ['mass', 'r', 'x', 'y', 'z', 'vx', 'vy', 'vz', 'ax', 'ay', 'az', 'rotationPhaseRad']
                        .every((key) => Number.isFinite(body[key]))),
                    forceAuditFinite: Object.values(bridge.getDiagnostics().forceAudit).every(Number.isFinite),
                });
            }
            const fallback = new PlanetaryMockBridge();
            fallback.setupScenario('retired-or-unknown-scale4-scenario');
            return {
                validation: registry.validateScale4ScenarioRegistry(),
                ids: registry.SCALE4_SCENARIOS.map((scenario) => scenario.id),
                options,
                selected: select.value,
                scenarios,
                fallback: fallback.getDiagnostics().scenario,
            };
        });

        expect(metadata).toMatchObject({
            scale: 4,
            name: 'Solar System',
            engineMode: 'planetary',
            selectId: 'planetary-scenario-select',
            registry: 'engine/web/js/scales/scale4/scenario-registry.js',
        });
        expect(metadata.scenarioCount).toBe(result.ids.length);
        expect(result.validation).toEqual({ ok: true, errors: [], count: result.ids.length });
        expect(result.options).toEqual(result.ids);
        expect(result.selected).toBe('planetary-solar');
        expect(result.fallback).toBe('planetary-solar');
        for (const scenario of result.scenarios) {
            expect(scenario.actual, scenario.id).toBe(scenario.id);
            expect(scenario.beforeCount, scenario.id).toBeGreaterThan(1);
            expect(scenario.tick, scenario.id).toBe(2);
            expect(scenario.time, scenario.id).toBeGreaterThan(0);
            expect(scenario.finite, scenario.id).toBe(true);
            expect(scenario.forceAuditFinite, scenario.id).toBe(true);
        }
    });

    test('default state is physical, finite, barycentric, and complete at the declared scope', async ({ page }) => {
        await page.goto('/js/constants.js');
        const state = await page.evaluate(async () => {
            const [{ PlanetaryMockBridge, orbitalElementsToState }, { G_HELIOCENTRIC }, reference, physics] = await Promise.all([
                import('/js/bridge/mock-scale4.js?solar-contract=1'),
                import('/js/constants.js'),
                import('/js/config/solar-system-data.js?solar-contract=1'),
                import('/js/config/solar-system-physics.js?solar-contract=1'),
            ]);
            const bridge = new PlanetaryMockBridge();
            bridge.setupScenario('planetary-solar');
            const bodies = bridge.getBodies();
            const sun = bodies.find((body) => body.name === 'Sun');
            const earth = bodies.find((body) => body.name === 'Earth');
            const planets = bodies.filter((body) => body.parentKey === 'sun' && body.type !== PlanetaryMockBridge.TYPE.DWARF_PLANET);
            const moons = bodies.filter((body) => body.type === PlanetaryMockBridge.TYPE.MOON);
            const moon = bodies.find((body) => body.key === 'moon');
            const earthReference = reference.SOLAR_SYSTEM_PLANETS.find((body) => body.key === 'earth');
            const mercuryReference = reference.SOLAR_SYSTEM_PLANETS.find((body) => body.key === 'mercury');
            const moonReference = reference.SOLAR_SYSTEM_MOONS.find((body) => body.key === 'moon');
            const emReferenceState = orbitalElementsToState(
                earthReference.orbit,
                reference.SUN_REFERENCE.massSolar,
                earthReference.massSolar,
                G_HELIOCENTRIC,
            );
            const emMass = earth.mass + moon.mass;
            const emBarycenter = {
                x: (earth.mass * earth.x + moon.mass * moon.x) / emMass,
                y: (earth.mass * earth.y + moon.mass * moon.y) / emMass,
                z: (earth.mass * earth.z + moon.mass * moon.z) / emMass,
                vx: (earth.mass * earth.vx + moon.mass * moon.vx) / emMass,
                vy: (earth.mass * earth.vy + moon.mass * moon.vy) / emMass,
                vz: (earth.mass * earth.vz + moon.mass * moon.vz) / emMass,
            };
            const momentum = bodies.reduce((p, body) => ({
                x: p.x + body.mass * body.vx,
                y: p.y + body.mass * body.vy,
                z: p.z + body.mass * body.vz,
            }), { x: 0, y: 0, z: 0 });
            return {
                G: bridge.G,
                expectedG: G_HELIOCENTRIC,
                names: planets.map((body) => body.name),
                bodyCount: bodies.length,
                moonCount: moons.length,
                earthDistance: Math.hypot(earth.x - sun.x, earth.y - sun.y, earth.z - sun.z),
                earthMoonDistance: Math.hypot(earth.x - moon.x, earth.y - moon.y, earth.z - moon.z),
                earthSpeed: Math.hypot(earth.vx - sun.vx, earth.vy - sun.vy, earth.vz - sun.vz),
                maxRadiusConversionError: Math.max(...bodies.map((body) => Math.abs(body.r - body.radiusKm / reference.KM_PER_AU))),
                kmPerAu: reference.KM_PER_AU,
                planetMeanRadiiKm: Object.fromEntries(reference.SOLAR_SYSTEM_PLANETS.map((body) => [body.key, body.radiusKm])),
                planetSiderealRotationDays: Object.fromEntries(reference.SOLAR_SYSTEM_PLANETS.map((body) => [body.key, body.rotationDays])),
                planetSemimajorAxesAu: Object.fromEntries(reference.SOLAR_SYSTEM_PLANETS.map((body) => [body.key, body.orbit.a])),
                sunRadiusKm: reference.SUN_REFERENCE.radiusKm,
                plutoRadiusKm: reference.SOLAR_SYSTEM_DWARFS.find((body) => body.key === 'pluto').radiusKm,
                moonSemimajorAu: moonReference.aKm / reference.KM_PER_AU,
                moonEccentricity: moonReference.e,
                emBarycenterPositionError: Math.hypot(
                    emBarycenter.x - sun.x - emReferenceState.x,
                    emBarycenter.y - sun.y - emReferenceState.y,
                    emBarycenter.z - sun.z - emReferenceState.z,
                ),
                emBarycenterVelocityError: Math.hypot(
                    emBarycenter.vx - sun.vx - emReferenceState.vx,
                    emBarycenter.vy - sun.vy - emReferenceState.vy,
                    emBarycenter.vz - sun.vz - emReferenceState.vz,
                ),
                momentum: Math.hypot(momentum.x, momentum.y, momentum.z),
                finite: bodies.every((body) => ['x', 'y', 'z', 'vx', 'vy', 'vz', 'ax', 'ay', 'az', 'mass'].every((key) => Number.isFinite(body[key]))),
                earthHillRadius: physics.hillRadiusAu(earth, sun),
                earthHillExpected: Math.hypot(earth.x - sun.x, earth.y - sun.y, earth.z - sun.z)
                    * Math.cbrt(earth.mass / (3 * sun.mass)),
                renderData: bridge.getPlanetaryData(),
                diagnostics: bridge.getDiagnostics(),
            };
        });

        expect(state.G).toBeCloseTo(state.expectedG, 12);
        expect(state.names).toEqual(['Mercury', 'Venus', 'Earth', 'Mars', 'Jupiter', 'Saturn', 'Uranus', 'Neptune']);
        expect(state.bodyCount).toBe(19);
        expect(state.moonCount).toBe(9);
        expect(state.earthDistance).toBeGreaterThan(0.98);
        expect(state.earthDistance).toBeLessThan(1.02);
        expect(state.earthMoonDistance).toBeGreaterThan(state.moonSemimajorAu * (1 - state.moonEccentricity));
        expect(state.earthMoonDistance).toBeLessThan(state.moonSemimajorAu * (1 + state.moonEccentricity));
        expect(state.maxRadiusConversionError).toBe(0);
        expect(state.kmPerAu).toBe(149_597_870.7);
        expect(state.planetMeanRadiiKm).toEqual({
            mercury: 2439.4,
            venus: 6051.8,
            earth: 6371.0084,
            mars: 3389.5,
            jupiter: 69_911,
            saturn: 58_232,
            uranus: 25_362,
            neptune: 24_622,
        });
        expect(state.planetSiderealRotationDays).toEqual({
            mercury: 58.6462,
            venus: -243.018,
            earth: 0.99726968,
            mars: 1.02595676,
            jupiter: 0.41354,
            saturn: 0.44401,
            uranus: -0.71833,
            neptune: 0.67125,
        });
        expect(state.planetSemimajorAxesAu).toEqual({
            mercury: 0.38709927,
            venus: 0.72333566,
            earth: 1.00000261,
            mars: 1.52371034,
            jupiter: 5.20288700,
            saturn: 9.53667594,
            uranus: 19.18916464,
            neptune: 30.06992276,
        });
        expect(state.sunRadiusKm).toBe(695_700);
        expect(state.plutoRadiusKm).toBe(1188.3);
        expect(state.emBarycenterPositionError).toBeLessThan(1e-15);
        expect(state.emBarycenterVelocityError).toBeLessThan(1e-14);
        expect(state.earthSpeed).toBeGreaterThan(6.1);
        expect(state.earthSpeed).toBeLessThan(6.6);
        expect(state.momentum).toBeLessThan(1e-16);
        expect(state.finite).toBe(true);
        expect(state.earthHillRadius).toBeCloseTo(state.earthHillExpected, 14);
        expect(state.renderData.gravityConstant).toBe(state.G);
        expect(state.renderData.gravityMode).toBe('physical');
        expect(state.renderData.physics.newtonianGravity).toBe(true);
        expect(state.diagnostics.gravityMode).toBe('physical');
        expect(state.diagnostics.timeStep.key).toBe('minute');
        expect(state.diagnostics.timeStep.seconds).toBe(60);
        expect(state.diagnostics.timeStep.integrationSubsteps).toBe(1);
        expect(state.diagnostics.dtDays).toBeCloseTo(1 / 1440, 12);
    });

    test('one public tick advances the selected minute, hour, or day interval', async ({ page }) => {
        await page.goto('/js/constants.js');
        const result = await page.evaluate(async () => {
            const { PlanetaryMockBridge } = await import('/js/bridge/mock-scale4.js?time-step-contract=1');
            const bridge = new PlanetaryMockBridge();
            bridge.setupScenario('planetary-solar');
            bridge.run(1);
            const minute = bridge.getDiagnostics();
            bridge.setTimeStep('hour');
            bridge.run(1);
            const hour = bridge.getDiagnostics();
            bridge.setTimeStep('day');
            bridge.run(1);
            const day = bridge.getDiagnostics();
            const invalidRejected = bridge.setTimeStep('week') === false;
            bridge.setupScenario('planetary-threebody');
            const natural = bridge.getDiagnostics();
            bridge.setupScenario('planetary-solar');
            const restored = bridge.getDiagnostics();
            return {
                minute: { tick: minute.tick, days: minute.timeDays, step: minute.timeStep },
                hour: { tick: hour.tick, days: hour.timeDays, step: hour.timeStep },
                day: { tick: day.tick, days: day.timeDays, step: day.timeStep },
                invalidRejected,
                natural: natural.timeStep,
                restored: restored.timeStep,
                finite: bridge.getBodies().every((body) => ['x', 'y', 'z', 'vx', 'vy', 'vz'].every((key) => Number.isFinite(body[key]))),
            };
        });

        expect(result.minute.tick).toBe(1);
        expect(result.minute.days).toBeCloseTo(1 / 1440, 12);
        expect(result.minute.step).toMatchObject({ key: 'minute', seconds: 60, integrationSubsteps: 1 });
        expect(result.hour.tick).toBe(2);
        expect(result.hour.days).toBeCloseTo(61 / 1440, 12);
        expect(result.hour.step).toMatchObject({ key: 'hour', seconds: 3600, integrationSubsteps: 5 });
        expect(result.day.tick).toBe(3);
        expect(result.day.days).toBeCloseTo(1 + 61 / 1440, 12);
        expect(result.day.step).toMatchObject({ key: 'day', seconds: 86400, integrationSubsteps: 100 });
        expect(result.invalidRejected).toBe(true);
        expect(result.natural).toMatchObject({
            key: 'natural', requestedKey: 'day', years: null, naturalTime: 0.01,
            integrationSubsteps: 100, naturalUnits: true,
        });
        expect(result.restored).toMatchObject({ key: 'day', seconds: 86400, integrationSubsteps: 100, naturalUnits: false });
        expect(result.finite).toBe(true);
    });

    test('every Solar System planet advances its sidereal spin by the selected elapsed time', async ({ page }) => {
        await page.goto('/js/constants.js');
        const result = await page.evaluate(async () => {
            const [{ PlanetaryMockBridge }, { siderealSpinRateRadYr }] = await Promise.all([
                import('/js/bridge/mock-scale4.js?rotation-clock=1'),
                import('/js/config/solar-system-physics.js?rotation-clock=1'),
            ]);
            const bridge = new PlanetaryMockBridge();
            bridge.setupScenario('planetary-solar');
            bridge.setPhysicsToggle('equilibriumTides', false);
            const planetTypes = new Set([
                PlanetaryMockBridge.TYPE.ROCKY_PLANET,
                PlanetaryMockBridge.TYPE.GAS_GIANT,
            ]);
            const planets = () => bridge.getBodies().filter((body) => planetTypes.has(body.type));
            const initial = Object.fromEntries(planets().map((body) => [body.key, {
                period: body.rotationDays,
                tilt: body.axialTiltDeg,
                rate: body.spinRateRadYr,
                expectedRate: siderealSpinRateRadYr(body),
            }]));
            const snapshots = [];
            for (const key of ['minute', 'hour', 'day']) {
                bridge.setTimeStep(key);
                bridge.run(1);
                snapshots.push({
                    key,
                    elapsedYears: bridge.getDiagnostics().timeYears,
                    phases: Object.fromEntries(planets().map((body) => [body.key, body.rotationPhaseRad])),
                    periods: Object.fromEntries(planets().map((body) => [body.key, body.rotationDays])),
                });
            }
            return { initial, snapshots };
        });

        const tau = 2 * Math.PI;
        const wrap = (phase) => phase % tau;
        expect(Object.keys(result.initial)).toEqual([
            'mercury', 'venus', 'earth', 'mars', 'jupiter', 'saturn', 'uranus', 'neptune',
        ]);
        for (const [key, initial] of Object.entries(result.initial)) {
            expect(initial.rate).toBeCloseTo(initial.expectedRate, 12);
            for (const snapshot of result.snapshots) {
                expect(snapshot.phases[key]).toBeCloseTo(wrap(initial.rate * snapshot.elapsedYears), 11);
                expect(snapshot.periods[key]).toBe(initial.period);
            }
        }
        expect(Math.sign(result.initial.earth.rate * Math.cos(result.initial.earth.tilt * Math.PI / 180))).toBe(1);
        expect(Math.sign(result.initial.venus.rate * Math.cos(result.initial.venus.tilt * Math.PI / 180))).toBe(-1);
        expect(Math.sign(result.initial.uranus.rate * Math.cos(result.initial.uranus.tilt * Math.PI / 180))).toBe(-1);
    });

    test('renderer uses the integrated physical spin phase without frame-time drift', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'planetary');
        const result = await page.evaluate(() => {
            const { bridge, _planetaryRenderer: renderer } = window.__ftdCtx.inspector;
            bridge.setPhysicsToggle('equilibriumTides', false);
            const samples = [];
            for (const key of ['minute', 'hour', 'day']) {
                bridge.setTimeStep(key);
                bridge.run(1);
                renderer.update(bridge.getPlanetaryData());
                samples.push(bridge.getBodies()
                    .filter((body) => body.type === 1 || body.type === 2)
                    .map((body) => ({
                        key: body.key,
                        statePhase: body.rotationPhaseRad,
                        renderPhase: renderer.getBodyVisualState(body.id).rotationPhaseRad,
                    })));
            }
            return samples;
        });

        for (const sample of result.flat()) {
            expect(sample.renderPhase).toBeCloseTo(sample.statePhase, 12);
        }
        expect(realErrors(errors)).toEqual([]);
    });

    test('the complete split-kick physics stack keeps the full Solar System finite for one year', async ({ page }) => {
        await page.goto('/js/constants.js');
        const result = await page.evaluate(async () => {
            const { PlanetaryMockBridge } = await import('/js/bridge/mock-scale4.js?solar-drift=1');
            const bridge = new PlanetaryMockBridge();
            bridge.setupScenario('planetary-solar');
            bridge.setTimeStep('day');
            const energy = () => {
                const bodies = bridge.getBodies();
                let ke = 0, pe = 0;
                for (const body of bodies) ke += 0.5 * body.mass * (body.vx ** 2 + body.vy ** 2 + body.vz ** 2);
                for (let i = 0; i < bodies.length; i++) {
                    for (let j = i + 1; j < bodies.length; j++) {
                        const a = bodies[i], b = bodies[j];
                        const r2 = (a.x - b.x) ** 2 + (a.y - b.y) ** 2 + (a.z - b.z) ** 2 + bridge.softeningSq;
                        pe -= bridge.G * a.mass * b.mass / Math.sqrt(r2);
                    }
                }
                return ke + pe;
            };
            const initial = energy();
            bridge.run(365);
            const final = energy();
            const bodies = bridge.getBodies();
            const sun = bodies.find((body) => body.key === 'sun');
            const earth = bodies.find((body) => body.key === 'earth');
            const diagnostics = bridge.getDiagnostics();
            return {
                drift: Math.abs((final - initial) / initial),
                finite: bodies.every((body) => ['x', 'y', 'z', 'vx', 'vy', 'vz'].every((key) => Number.isFinite(body[key]))),
                elapsedDays: diagnostics.timeDays,
                earthDistance: Math.hypot(earth.x - sun.x, earth.y - sun.y, earth.z - sun.z),
                bodyCount: bodies.length,
                stellarMassLost: diagnostics.stellarMassLost,
                ticks: diagnostics.tick,
            };
        });
        expect(result.finite).toBe(true);
        expect(result.elapsedDays).toBeCloseTo(365, 8);
        expect(result.bodyCount).toBe(19);
        expect(result.ticks).toBe(365);
        expect(result.earthDistance).toBeGreaterThan(0.97);
        expect(result.earthDistance).toBeLessThan(1.03);
        expect(result.drift).toBeLessThan(1e-7);
        expect(result.stellarMassLost).toBeGreaterThan(8e-14);
        expect(result.stellarMassLost).toBeLessThan(1e-13);
    });

    test('every non-destructive orbital scenario stays finite for one physical year', async ({ page }) => {
        await page.goto('/js/constants.js');
        const results = await page.evaluate(async () => {
            const { PlanetaryMockBridge } = await import('/js/bridge/mock-scale4.js?solar-year-matrix=1');
            const scenarios = [
                'planetary-solar', 'planetary-binary', 'planetary-mercury-relativity',
                'planetary-earth-moon-tides', 'exo-TRAPPIST-1', 'exo-Kepler-11',
                'exo-HR 8799', 'exo-Kepler-20',
            ];
            return scenarios.map((scenario) => {
                const bridge = new PlanetaryMockBridge();
                bridge.setupScenario(scenario);
                bridge.setTimeStep('day');
                bridge.run(365);
                const diag = bridge.getDiagnostics();
                return {
                    scenario,
                    days: diag.timeDays,
                    collisions: diag.collisionCount,
                    roche: diag.rocheDisruptionCount,
                    finite: bridge.getBodies().every((body) =>
                        ['mass', 'r', 'x', 'y', 'z', 'vx', 'vy', 'vz', 'ax', 'ay', 'az', 'rotationPhaseRad']
                            .every((key) => Number.isFinite(body[key]))),
                    auditFinite: Object.values(diag.forceAudit).every(Number.isFinite),
                };
            });
        });

        for (const result of results) {
            expect(result.days, result.scenario).toBeCloseTo(365, 8);
            expect(result.collisions, result.scenario).toBe(0);
            expect(result.roche, result.scenario).toBe(0);
            expect(result.finite, result.scenario).toBe(true);
            expect(result.auditFinite, result.scenario).toBe(true);
        }
    });

    test('renamed UI, hierarchy, overlays, inspection, and Scale 4 telemetry panels are wired', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'planetary');
        await expect(page.locator('#engine-mode option[value="planetary"]')).toHaveText('Scale 4 (Solar System)');
        await expect(page.locator('#planetary-scenario-select')).toHaveValue('planetary-solar');
        await expect(page.locator('#planetary-gravity-mode')).toHaveValue('physical');
        await expect(page.locator('#planetary-time-step')).toHaveValue('minute');
        await expect(page.locator('#planetary-layer-list [data-body-id]')).toHaveCount(19);
        await expect(page.locator('#planetary-live-bodies')).toContainText('19 modeled bodies');
        await expect(page.locator('#planetary-live-bodies')).toContainText('1 dwarf');
        await expect(page.locator('#status-time-label')).toHaveText('Day');
        await expect(page.locator('#status-particles-label')).toHaveText('Bodies');
        await expect(page.locator('#status-particles')).toHaveText('19');
        await expect(page.locator('#status-energy-label')).toHaveText('Mech E');
        await expect(page.locator('#status-energy')).not.toHaveText('—');
        await expect(page.locator('#planetary-system-title')).toHaveText('Our Solar System · J2000');
        await expect(page.locator('#planetary-ctrl-units')).toHaveText('AU · M☉ · Julian yr');
        await expect(page.locator('#planetary-ctrl-epoch')).toHaveText('J2000 approximate elements');
        await expect(page.locator('#planetary-scale-gauge')).toHaveText('1:1 AU · physical radii');
        await expect(page.locator('#planetary-size-mode')).toHaveCount(0);

        const panels = await page.locator('.tab:not(.hidden)').allTextContents();
        expect(panels.join(' ')).toContain('Solar System');
        expect(panels.join(' ')).toContain('Diagnostics');
        expect(panels.join(' ')).toContain('Telemetry Grid');
        expect(panels.join(' ')).toContain('Charts');

        await page.locator('.tab[data-panel="planetary"]').click();
        const earth = page.locator('#planetary-layer-list [data-body-id]').filter({ hasText: 'Earth' }).first();
        await earth.click();
        await expect(earth).toHaveClass(/is-selected/);
        await expect(page.locator('#planetary-insp-type')).toContainText('Earth');
        await expect(page.locator('#planetary-insp-speed')).toContainText('km/s');
        await expect(page.locator('#planetary-insp-radius')).toContainText('km ·');
        await expect(page.locator('#planetary-insp-radius')).toContainText('AU');

        await page.locator('#planetary-opt-labels').uncheck();
        await page.locator('#planetary-opt-labels').check();

        await page.locator('#btn-clear-inspector').click();
        await expect(page.locator('#planetary-insp-type')).toBeHidden();
        await expect(page.locator('#planetary-layer-list .is-selected')).toHaveCount(0);

        await page.locator('.tab[data-panel="planetary"]').click();
        const mercury = page.locator('#planetary-layer-list li[data-body-id]').filter({ hasText: 'Mercury' }).first();
        await mercury.press('Enter');
        await expect(page.locator('#planetary-insp-type')).toContainText('Mercury');
        await expect(mercury).toHaveClass(/is-selected/);

        const invalidSummaryChildren = await page.locator('#planetary-layer-list summary > li').count();
        expect(invalidSummaryChildren).toBe(0);

        expect(realErrors(errors)).toEqual([]);
    });

    test('time-step UI advances exact tick intervals and leaves playback speed independent', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'planetary');
        const stepSelect = page.locator('#planetary-time-step');
        await expect(stepSelect).toHaveValue('minute');
        await expect(page.locator('#planetary-ctrl-integrator')).toContainText('1 minute/tick · 1×1 min');

        await page.locator('#btn-step').click();
        await expect(page.locator('#planetary-live-time')).toContainText('tick 1 · J2000 + 0d 00:01');

        await stepSelect.selectOption('hour');
        await page.locator('#btn-step').click();
        await expect(page.locator('#planetary-live-time')).toContainText('tick 2 · J2000 + 0d 01:01');
        await expect(page.locator('#planetary-ctrl-integrator')).toContainText('1 hour/tick · 5×12 min');

        await page.evaluate(() => {
            const slider = document.getElementById('ticks-per-frame');
            slider.value = '75';
            slider.dispatchEvent(new Event('input', { bubbles: true }));
        });
        await expect.poll(() => page.evaluate(() => window.__ftdCtx.ticksPerFrame)).toBeCloseTo(10, 10);
        const speedBeforeStepChange = await page.evaluate(() => ({
            slider: document.getElementById('ticks-per-frame').value,
            display: document.getElementById('tpf-display').textContent,
            multiplier: window.__ftdCtx.ticksPerFrame,
        }));

        await stepSelect.selectOption('day');
        await page.locator('#btn-step').click();
        await expect(page.locator('#planetary-live-time')).toContainText('tick 3 · J2000 + 1d 01:01');
        await expect(page.locator('#planetary-ctrl-integrator')).toContainText('1 day/tick · 100×14.4 min');
        const speedAfterStepChange = await page.evaluate(() => ({
            slider: document.getElementById('ticks-per-frame').value,
            display: document.getElementById('tpf-display').textContent,
            multiplier: window.__ftdCtx.ticksPerFrame,
            diagnostic: window.__ftdCtx.inspector.bridge.getDiagnostics(),
        }));
        expect(speedAfterStepChange.slider).toBe(speedBeforeStepChange.slider);
        expect(speedAfterStepChange.display).toBe(speedBeforeStepChange.display);
        expect(speedAfterStepChange.multiplier).toBe(speedBeforeStepChange.multiplier);
        expect(speedAfterStepChange.diagnostic.tick).toBe(3);
        expect(speedAfterStepChange.diagnostic.timeDays).toBeCloseTo(1 + 61 / 1440, 12);

        await page.locator('#planetary-scenario-select').selectOption('planetary-threebody');
        await expect(stepSelect).toBeDisabled();
        await expect(stepSelect).toHaveValue('natural');
        await page.locator('#planetary-scenario-select').selectOption('planetary-solar');
        await expect(stepSelect).toBeEnabled();
        await expect(stepSelect).toHaveValue('day');
        expect(realErrors(errors)).toEqual([]);
    });

    test('every Scale 4 visual, camera, gravity, and transport control changes live state', async ({ page }) => {
        test.setTimeout(120_000);
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await page.evaluate(async () => {
            const { PlanetaryRenderer } = await import('/js/planetary-renderer.js?v=8');
            window.__scale4RendererCalls = [];
            const methods = [
                'setRenderOrbits', 'setRenderAxes', 'setRenderLabels', 'setRenderMoons',
                'setEclipticVisible', 'setRenderBelts', 'setRenderHabitableZone',
                'setRenderGravityField', 'setRenderAccelerationVectors', 'setRenderVelocityVectors',
                'setRenderHillSpheres', 'setRenderRocheLimits', 'setRenderCollisionShells',
            ];
            for (const method of methods) {
                const original = PlanetaryRenderer.prototype[method];
                PlanetaryRenderer.prototype[method] = function (...args) {
                    window.__scale4RendererCalls.push({ method, value: args[0] });
                    return original.apply(this, args);
                };
            }
        });
        await switchMode(page, 'planetary');
        await page.evaluate(() => { window.__scale4RendererCalls.length = 0; });

        const visualControls = [
            ['#planetary-opt-orbits', 'setRenderOrbits'],
            ['#planetary-opt-axes', 'setRenderAxes'],
            ['#planetary-opt-labels', 'setRenderLabels'],
            ['#planetary-opt-moons', 'setRenderMoons'],
            ['#planetary-opt-ecliptic', 'setEclipticVisible'],
            ['#planetary-opt-belts', 'setRenderBelts'],
            ['#planetary-opt-habitable', 'setRenderHabitableZone'],
            ['#planetary-opt-gravity-field', 'setRenderGravityField'],
            ['#planetary-opt-acceleration', 'setRenderAccelerationVectors'],
            ['#planetary-opt-velocity', 'setRenderVelocityVectors'],
            ['#planetary-opt-hill', 'setRenderHillSpheres'],
            ['#planetary-opt-roche', 'setRenderRocheLimits'],
            ['#planetary-opt-collision', 'setRenderCollisionShells'],
        ];
        for (const [selector, method] of visualControls) {
            const input = page.locator(selector);
            if (await input.isChecked()) await input.uncheck();
            else await input.check();
            const changedValue = await input.isChecked();
            await expect.poll(() => page.evaluate(
                ({ method, changedValue }) => window.__scale4RendererCalls
                    .some((call) => call.method === method && call.value === changedValue),
                { method, changedValue },
            )).toBe(true);
            if (changedValue) await input.uncheck();
            else await input.check();
        }

        await page.locator('#planetary-opt-gravity-field').check();
        await page.locator('#planetary-opt-hill').check();
        await page.locator('#planetary-opt-acceleration').check();
        const overlayAudit = await page.evaluate(() => {
            const renderer = window.__ftdCtx.inspector._planetaryRenderer;
            const status = renderer.getPhysicsOverlayStatus();
            const field = renderer._group.getObjectByName('scale4-gravity-field-strength');
            return {
                status,
                fieldFinite: [...field.geometry.getAttribute('position').array].every(Number.isFinite),
                fieldMeaning: field.userData.meaning,
                fieldMapping: field.userData.displayMapping,
            };
        });
        expect(overlayAudit.status.gravityField.visible).toBe(true);
        expect(overlayAudit.status.gravityField.samples).toBe(1089);
        expect(overlayAudit.status.gravityField.sourceCount).toBe(19);
        expect(overlayAudit.status.hillSpheres).toBeGreaterThan(0);
        expect(overlayAudit.status.accelerationVectors).toBeGreaterThan(0);
        expect(overlayAudit.fieldFinite).toBe(true);
        expect(overlayAudit.fieldMeaning).toContain('no finite cutoff');
        expect(overlayAudit.fieldMapping).toContain('logarithmic');
        await expect(page.locator('#planetary-physics-overlay-readout')).toContainText('1,089 samples');

        await page.locator('.tab[data-panel="planetary"]').click();
        await page.locator('#planetary-layer-list [data-body-id]').filter({ hasText: 'Earth' }).first().click();
        const physicalGauge = await page.evaluate(() => {
            const { viewport, inspector } = window.__ftdCtx;
            const bodies = inspector.bridge.getBodies();
            const renderer = inspector._planetaryRenderer;
            let maxPositionError = 0;
            let maxRadiusError = 0;
            let maxDistanceError = 0;
            for (const body of bodies) {
                const state = renderer.getBodyVisualState(body.id);
                const expected = [body.x, body.z, body.y];
                maxPositionError = Math.max(maxPositionError, ...state.position.toArray().map((value, index) => Math.abs(value - expected[index])));
                maxRadiusError = Math.max(maxRadiusError, Math.abs(state.radius - body.r));
                const parent = bodies.find((candidate) => candidate.id === body.parentId);
                if (parent) {
                    const parentState = renderer.getBodyVisualState(parent.id);
                    const simDistance = Math.hypot(body.x - parent.x, body.y - parent.y, body.z - parent.z);
                    maxDistanceError = Math.max(maxDistanceError, Math.abs(state.position.distanceTo(parentState.position) - simDistance));
                }
            }
            const earth = bodies.find((body) => body.key === 'earth');
            const earthState = renderer.getBodyVisualState(earth.id);
            return {
                maxPositionError,
                maxRadiusError,
                maxDistanceError,
                camera: viewport.camera.position.toArray(),
                target: viewport.controls.target.toArray(),
                cameraRadii: viewport.camera.position.distanceTo(earthState.position) / earth.r,
                nearRadiusRatio: viewport.camera.near / earth.r,
                minDistanceRadiusRatio: viewport.controls.minDistance / earth.r,
                overlayStatus: renderer.getPhysicsOverlayStatus(),
            };
        });
        expect(physicalGauge.maxPositionError).toBe(0);
        expect(physicalGauge.maxRadiusError).toBe(0);
        expect(physicalGauge.maxDistanceError).toBeLessThan(1e-14);
        expect(physicalGauge.cameraRadii).toBeCloseTo(7.5, 10);
        expect(physicalGauge.nearRadiusRatio).toBeCloseTo(0.02, 10);
        expect(physicalGauge.minDistanceRadiusRatio).toBeCloseTo(1.2, 10);
        expect(physicalGauge.overlayStatus.selectedOnly).toBe(true);
        expect(physicalGauge.overlayStatus.gravityField.sourceCount).toBe(1);
        expect(physicalGauge.overlayStatus.accelerationVectors).toBe(1);
        expect(physicalGauge.overlayStatus.hillSpheres).toBe(1);
        const focusedCamera = { camera: physicalGauge.camera, target: physicalGauge.target };
        await page.locator('#planetary-home-view').click();
        const systemCamera = await page.evaluate(() => ({
            camera: window.__ftdCtx.viewport.camera.position.toArray(),
            target: window.__ftdCtx.viewport.controls.target.toArray(),
            near: window.__ftdCtx.viewport.camera.near,
            minDistance: window.__ftdCtx.viewport.controls.minDistance,
        }));
        expect(systemCamera.camera).not.toEqual(focusedCamera.camera);
        expect(systemCamera.target).not.toEqual(focusedCamera.target);
        expect(systemCamera.near).toBe(0.001);
        expect(systemCamera.minDistance).toBe(0.01);

        const collapse = page.locator('#cs-viewport-overlay [aria-label="Collapse overlay"]');
        await collapse.click();
        await expect(page.locator('#cs-viewport-overlay')).toHaveClass(/is-collapsed/);
        await expect(collapse).toHaveAttribute('aria-expanded', 'false');
        await collapse.click();
        await expect(page.locator('#cs-viewport-overlay')).not.toHaveClass(/is-collapsed/);

        await page.locator('#planetary-gravity-mode').selectOption('presentation');
        await expect(page.locator('#planetary-overlay-status')).toContainText('Slow comparison gauge');
        await expect(page.locator('#planetary-ctrl-gravity')).toContainText('0.01 AU³');
        await page.locator('#planetary-gravity-mode').selectOption('physical');
        await expect(page.locator('#planetary-overlay-status')).toContainText('physical AU/M☉/yr');

        await page.locator('#btn-step').click();
        await expect(page.locator('#status-ptime')).not.toHaveText('0.00');
        await page.locator('#btn-reset').click();
        await expect(page.locator('#status-ptime')).toHaveText('0.00000');
        await page.locator('#btn-play').click();
        await expect(page.locator('#btn-play')).toHaveAttribute('data-paused', 'false');
        await expect.poll(async () => Number(await page.locator('#status-ptime').textContent())).toBeGreaterThan(0);
        await page.locator('#btn-play').click();
        await expect(page.locator('#btn-play')).toHaveAttribute('data-paused', 'true');

        expect(realErrors(errors)).toEqual([]);
    });

    test('a selected body remains centered while the simulated system moves', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'planetary');
        await page.locator('.tab[data-panel="planetary"]').click();
        await page.locator('#planetary-layer-list [data-body-id]').filter({ hasText: 'Earth' }).first().click();

        const before = await page.evaluate(() => {
            const { viewport, inspector } = window.__ftdCtx;
            const renderer = inspector._planetaryRenderer;
            const state = renderer.getBodyVisualState(inspector._selectedPlanetaryId);
            return {
                bodyId: inspector._selectedPlanetaryId,
                bodyPosition: state.position.toArray(),
                target: viewport.controls.target.toArray(),
                cameraOffset: viewport.camera.position.clone().sub(viewport.controls.target).toArray(),
                followId: renderer._followId,
            };
        });
        expect(before.bodyId).toBeGreaterThanOrEqual(0);
        expect(before.followId).toBe(before.bodyId);
        expect(Math.hypot(...before.target.map((value, index) => value - before.bodyPosition[index]))).toBeLessThan(1e-14);

        const after = await page.evaluate(() => {
            const { viewport, inspector } = window.__ftdCtx;
            const bridge = inspector.bridge;
            const renderer = inspector._planetaryRenderer;
            bridge.setTimeStep('day');
            bridge.run(10);
            renderer.update(bridge.getPlanetaryData());
            const state = renderer.getBodyVisualState(inspector._selectedPlanetaryId);
            const targetError = viewport.controls.target.distanceTo(state.position);
            const cameraOffset = viewport.camera.position.clone().sub(viewport.controls.target);
            return {
                bodyPosition: state.position.toArray(),
                targetError,
                cameraOffset: cameraOffset.toArray(),
                followId: renderer._followId,
            };
        });
        expect(Math.hypot(...after.bodyPosition.map((value, index) => value - before.bodyPosition[index]))).toBeGreaterThan(0.1);
        const offsetError = Math.hypot(...after.cameraOffset.map((value, index) => value - before.cameraOffset[index]));
        expect(after.targetError).toBeLessThan(1e-14);
        expect(offsetError).toBeLessThan(1e-12);
        expect(after.followId).toBe(before.bodyId);

        await page.locator('#planetary-home-view').click();
        const home = await page.evaluate(() => ({
            selectedId: window.__ftdCtx.inspector._selectedPlanetaryId,
            followId: window.__ftdCtx.inspector._planetaryRenderer._followId,
        }));
        expect(home).toEqual({ selectedId: -1, followId: -1 });
        expect(realErrors(errors)).toEqual([]);
    });

    test('scenario UI clears stale selection and reports the active model rather than stale Solar labels', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'planetary');
        await page.locator('.tab[data-panel="planetary"]').click();
        await page.locator('#planetary-layer-list [data-body-id="0"]').click();
        await expect(page.locator('#planetary-insp-type')).toContainText('Sun');

        await page.locator('#planetary-scenario-select').selectOption('planetary-roche-lab');
        await expect(page.locator('#planetary-inspector-content')).toBeHidden();
        await expect(page.locator('#btn-clear-inspector')).toBeDisabled();
        await expect(page.locator('#planetary-layer-list .is-selected')).toHaveCount(0);
        await expect(page.locator('#planetary-system-title')).toHaveText('Saturn · Roche Disruption');
        await expect(page.locator('#planetary-ctrl-epoch')).toHaveText('Designed experiment initial state');
        await expect(page.locator('#planetary-live-time')).toContainText('elapsed');

        await page.locator('#btn-step').click();
        await expect(page.locator('#planetary-live-bodies')).toContainText('8 debris');
        await expect(page.locator('#planetary-live-bodies')).not.toContainText('8 moons');
        await page.locator('#planetary-opt-moons').uncheck();
        await expect(page.locator('#planetary-layer-list [data-body-id]')).toHaveCount(9);
        const debrisVisibility = await page.evaluate(() => {
            const inspector = window.__ftdCtx.inspector;
            const debrisIds = new Set(inspector.bridge.getBodies()
                .filter((body) => body.rocheFragment).map((body) => body.id));
            const nodes = inspector._planetaryRenderer._nodes.filter((node) => debrisIds.has(node.id));
            return { count: nodes.length, visible: nodes.filter((node) => node.group.visible).length };
        });
        expect(debrisVisibility).toEqual({ count: 8, visible: 8 });

        await page.locator('#planetary-scenario-select').selectOption('planetary-threebody');
        await expect(page.locator('#planetary-ctrl-gravity')).toHaveText('G = 1 · natural units');
        await expect(page.locator('#planetary-ctrl-units')).toHaveText('natural units');
        await expect(page.locator('#planetary-ctrl-integrator')).toContainText('natural time');
        await expect(page.locator('#planetary-live-time')).toContainText('natural time');

        await page.locator('#planetary-scenario-select').selectOption('exo-Kepler-20');
        await expect(page.locator('#planetary-ctrl-epoch')).toHaveText('NASA PSCompPars · 2026-09-04');
        await expect(page.locator('#planetary-ctrl-appearance')).toContainText('not observed true color');
        await expect(page.locator('#cs-viewport-overlay .scale-overlay-footnote')).toContainText('NASA Exoplanet Archive scale data');
        await expect(page.locator('#planetary-geometry-contract')).toContainText('Uncompressed 1:1 AU geometry');

        await page.locator('#planetary-scenario-select').selectOption('planetary-radiation-lab');
        await expect(page.locator('#planetary-live-bodies')).toContainText('3 small bodies');
        await expect(page.locator('#planetary-live-bodies')).not.toContainText('3 planets');

        const lifecycle = await page.evaluate(async () => (await import('/js/scales/scale4/controller.js?v=12')).getScale4RuntimeAudit());
        expect(lifecycle).toMatchObject({
            hasBridge: true,
            hasRenderer: true,
            trackedThreeObjects: 1,
            loopActive: true,
            scenario: 'planetary-radiation-lab',
        });

        expect(realErrors(errors)).toEqual([]);
    });

    test('effective physics formulas have the expected sign, scale, and epistemic contract', async ({ page }) => {
        await page.goto('/js/constants.js');
        const result = await page.evaluate(async () => {
            const [{ PlanetaryMockBridge }, physics] = await Promise.all([
                import('/js/bridge/mock-scale4.js?solar-physics-contract=1'),
                import('/js/config/solar-system-physics.js?solar-physics-contract=1'),
            ]);
            const bridge = new PlanetaryMockBridge();
            bridge.setupScenario('planetary-earth-moon-tides');
            const earth = bridge.getBodies().find((body) => body.key === 'earth');
            const moon = bridge.getBodies().find((body) => body.key === 'moon');
            const tideMPerYear = moon.tidalMigrationAuPerYear * physics.AU_METERS;
            const onePn = physics.schwarzschild1PNAcceleration(
                { x: 0.387, y: 0, z: 0 },
                { x: 0, y: 10.1, z: 0 },
                bridge.G,
            );
            const j2 = physics.j2Acceleration(
                { x: 0.01, y: 0, z: 0 }, bridge.G * earth.mass, earth.r, earth.j2,
            );
            const radiation = physics.radiationAcceleration(
                { x: 1, y: 0, z: 0 }, { x: 0, y: 2 * Math.PI, z: 0 }, bridge.G, 0.1, 0.35,
            );
            return {
                available: physics.SOLAR_PHYSICS_DEFINITIONS.length,
                defaultsOn: Object.values(physics.DEFAULT_SOLAR_PHYSICS).every(Boolean),
                statusesHonest: physics.SOLAR_PHYSICS_DEFINITIONS.every((item) => ['CORE', 'PARAMETRIC', 'IMPOSED'].includes(item.status)),
                onePnFinite: Object.values(onePn).every(Number.isFinite),
                onePnRadialOutwardCorrection: onePn.x > 0,
                j2EquatorialInward: j2.x < 0,
                radiationPressureOutward: radiation.pressure.x > 0,
                radiationDragOpposesMotion: radiation.drag.y < 0,
                tideMPerYear,
                rocheAboveSurface: physics.fluidRocheLimitAu(earth, moon) > earth.r,
            };
        });

        expect(result.available).toBe(10);
        expect(result.defaultsOn).toBe(true);
        expect(result.statusesHonest).toBe(true);
        expect(result.onePnFinite).toBe(true);
        expect(result.onePnRadialOutwardCorrection).toBe(true);
        expect(result.j2EquatorialInward).toBe(true);
        expect(result.radiationPressureOutward).toBe(true);
        expect(result.radiationDragOpposesMotion).toBe(true);
        expect(result.tideMPerYear).toBeGreaterThan(0.02);
        expect(result.tideMPerYear).toBeLessThan(0.05);
        expect(result.rocheAboveSurface).toBe(true);
    });

    test('physics toggles are effective and conservative corrections preserve material momentum', async ({ page }) => {
        await page.goto('/js/constants.js');
        const result = await page.evaluate(async () => {
            const { PlanetaryMockBridge } = await import('/js/bridge/mock-scale4.js?solar-toggle-contract=1');
            const bridge = new PlanetaryMockBridge();
            bridge.setupScenario('planetary-solar');
            const allOn = bridge.getDiagnostics();
            for (const key of ['relativity1PN', 'oblatenessJ2', 'equilibriumTides', 'radiationForces', 'atmosphericDrag', 'stellarMassLoss']) {
                bridge.setPhysicsToggle(key, false);
            }
            const disabled = bridge.getDiagnostics();
            const momentum = () => bridge.getBodies().reduce((sum, body) => ({
                x: sum.x + body.mass * body.vx,
                y: sum.y + body.mass * body.vy,
                z: sum.z + body.mass * body.vz,
            }), { x: 0, y: 0, z: 0 });
            const before = momentum();
            bridge.run(20);
            const after = momentum();
            return {
                activeAll: allOn.activePhysicsCount,
                applicableAll: allOn.applicablePhysicsCount,
                nonzeroAll: [
                    allOn.forceAudit.relativityAcceleration,
                    allOn.forceAudit.j2Acceleration,
                    allOn.forceAudit.radiationAcceleration,
                    allOn.forceAudit.tideAcceleration,
                ].every((value) => value > 0),
                disabledAudit: disabled.forceAudit,
                momentumChange: Math.hypot(after.x - before.x, after.y - before.y, after.z - before.z),
                finite: bridge.getBodies().every((body) => ['x', 'y', 'z', 'vx', 'vy', 'vz'].every((key) => Number.isFinite(body[key]))),
            };
        });

        expect(result.activeAll).toBe(10);
        expect(result.applicableAll).toBe(10);
        expect(result.nonzeroAll).toBe(true);
        expect(result.disabledAudit.relativityAcceleration).toBe(0);
        expect(result.disabledAudit.j2Acceleration).toBe(0);
        expect(result.disabledAudit.radiationAcceleration).toBe(0);
        expect(result.disabledAudit.tideAcceleration).toBe(0);
        expect(result.disabledAudit.atmosphereAcceleration).toBe(0);
        expect(result.momentumChange).toBeLessThan(1e-16);
        expect(result.finite).toBe(true);
    });

    test('kernel dependencies, radiation channels, mass-loss ledger, and telemetry stay truthful', async ({ page }) => {
        await page.goto('/js/constants.js');
        const result = await page.evaluate(async () => {
            const [{ PlanetaryMockBridge }, { telemetryHub }, { charts }, { sections }] = await Promise.all([
                import('/js/bridge/mock-scale4.js?solar-dynamic-audit=1'),
                import('/js/telemetry-hub.js?solar-dynamic-audit=1'),
                import('/js/ui/panels/charts-panel/descriptors/scale4.js?solar-dynamic-audit=1'),
                import('/js/ui/panels/diagnostics-panel/descriptors/scale4.js?solar-dynamic-audit=1'),
            ]);

            const dependencyBridge = new PlanetaryMockBridge();
            dependencyBridge.setupScenario('planetary-solar');
            dependencyBridge.setPhysicsToggle('newtonianGravity', false);
            const dependencyStatus = Object.fromEntries(dependencyBridge.getPhysicsStatus()
                .map((item) => [item.key, { requested: item.requested, applicable: item.applicable, active: item.active, reason: item.reason }]));

            const radiationBridge = new PlanetaryMockBridge();
            radiationBridge.setupScenario('planetary-radiation-lab');
            radiationBridge.run(1);
            const radiationOn = radiationBridge.getDiagnostics().forceAudit;
            radiationBridge.setPhysicsToggle('solarWindDrag', false);
            const windOff = radiationBridge.getDiagnostics().forceAudit;
            radiationBridge.setPhysicsToggle('solarWindDrag', true);
            radiationBridge.setPhysicsToggle('radiationForces', false);
            const radiationOffStatus = Object.fromEntries(radiationBridge.getPhysicsStatus().map((item) => [item.key, item]));

            const massBridge = new PlanetaryMockBridge();
            massBridge.setupScenario('planetary-solar');
            massBridge.setTimeStep('day');
            massBridge.run(1);
            const massLostBeforeDisable = massBridge.getDiagnostics().stellarMassLost;
            massBridge.setPhysicsToggle('stellarMassLoss', false);
            const massLostAfterDisable = massBridge.getDiagnostics().stellarMassLost;

            const binaryBridge = new PlanetaryMockBridge();
            binaryBridge.setupScenario('planetary-binary');
            binaryBridge.run(1);
            const binaryStatus = Object.fromEntries(binaryBridge.getPhysicsStatus().map((item) => [item.key, item]));
            const circumbinary = binaryBridge.getBodies().find((body) => body.key === 'circumbinary');

            telemetryHub.resetScale(4);
            telemetryHub.collectScale4(binaryBridge);
            const chartBuffers = charts.flatMap((chart) => chart.series.map((series) => series.buffer));
            const diagnosticTrends = sections.flatMap((section) => section.rows)
                .map((row) => row.trend).filter(Boolean);
            const missingBuffers = [...new Set([...chartBuffers, ...diagnosticTrends])]
                .filter((name) => typeof telemetryHub[name]?.last !== 'function');
            const resolvePath = (path) => path.split('.').reduce((value, key) => value?.[key], telemetryHub);
            const missingSources = sections.flatMap((section) => section.rows)
                .filter((row) => row.source && resolvePath(row.source) === undefined)
                .map((row) => row.source);
            return {
                dependencyStatus,
                dependencyAudit: dependencyBridge.getDiagnostics().forceAudit,
                radiationOn,
                windOff,
                radiationOffStatus,
                massLostBeforeDisable,
                massLostAfterDisable,
                binary1pn: binaryStatus.relativity1PN,
                circumbinaryParentId: circumbinary.parentId,
                binaryWind: binaryBridge.getDiagnostics().forceAudit.solarWindDragAcceleration,
                telemetry: {
                    newtonian: telemetryHub.plNewtonianAccel.last(),
                    radiationPressure: telemetryHub.plRadiationPressureAccel.last(),
                    prDrag: telemetryHub.plPrDragAccel.last(),
                    solarWind: telemetryHub.plSolarWindAccel.last(),
                    dissipationPower: telemetryHub.plDissipationPower.last(),
                    massLoss: telemetryHub.plMassLoss.last(),
                },
                missingBuffers,
                missingSources,
            };
        });

        for (const key of ['relativity1PN', 'oblatenessJ2', 'equilibriumTides']) {
            expect(result.dependencyStatus[key].requested, key).toBe(true);
            expect(result.dependencyStatus[key].applicable, key).toBe(false);
            expect(result.dependencyStatus[key].active, key).toBe(false);
            expect(result.dependencyStatus[key].reason, key).toContain('Newtonian');
        }
        expect(result.dependencyAudit.newtonianAcceleration).toBe(0);
        expect(result.dependencyAudit.relativityAcceleration).toBe(0);
        expect(result.dependencyAudit.j2Acceleration).toBe(0);
        expect(result.dependencyAudit.tideAcceleration).toBe(0);
        expect(result.radiationOn.radiationPressureAcceleration).toBeGreaterThan(0);
        expect(result.radiationOn.poyntingRobertsonDragAcceleration).toBeGreaterThan(0);
        expect(result.radiationOn.solarWindDragAcceleration).toBeGreaterThan(0);
        expect(result.windOff.radiationPressureAcceleration).toBeGreaterThan(0);
        expect(result.windOff.poyntingRobertsonDragAcceleration).toBeGreaterThan(0);
        expect(result.windOff.solarWindDragAcceleration).toBe(0);
        expect(result.radiationOffStatus.solarWindDrag.requested).toBe(true);
        expect(result.radiationOffStatus.solarWindDrag.applicable).toBe(false);
        expect(result.radiationOffStatus.solarWindDrag.active).toBe(false);
        expect(result.massLostBeforeDisable).toBeGreaterThan(0);
        expect(result.massLostAfterDisable).toBe(result.massLostBeforeDisable);
        expect(result.binary1pn.requested).toBe(true);
        expect(result.binary1pn.applicable).toBe(false);
        expect(result.binary1pn.active).toBe(false);
        expect(result.binary1pn.reason).toContain('exactly one luminous star');
        expect(result.circumbinaryParentId).toBe(-1);
        expect(result.binaryWind).toBeGreaterThan(0);
        expect(result.telemetry.newtonian).toBeGreaterThan(0);
        expect(result.telemetry.radiationPressure).toBeGreaterThan(0);
        expect(result.telemetry.prDrag).toBeGreaterThan(0);
        expect(result.telemetry.solarWind).toBeGreaterThan(0);
        expect(result.telemetry.dissipationPower).toBeGreaterThanOrEqual(0);
        expect(result.telemetry.massLoss).toBeGreaterThan(0);
        expect(result.missingBuffers).toEqual([]);
        expect(result.missingSources).toEqual([]);
    });

    test('every advertised physics kernel has a live enabled witness and a zeroed disabled path', async ({ page }) => {
        await page.goto('/js/constants.js');
        const witnesses = await page.evaluate(async () => {
            const { PlanetaryMockBridge } = await import('/js/bridge/mock-scale4.js?solar-kernel-witnesses=1');
            const cases = [
                ['newtonianGravity', 'planetary-solar', 1, (diag) => diag.forceAudit.newtonianAcceleration],
                ['relativity1PN', 'planetary-mercury-relativity', 1, (diag) => diag.forceAudit.relativityAcceleration],
                ['oblatenessJ2', 'planetary-earth-moon-tides', 1, (diag) => diag.forceAudit.j2Acceleration],
                ['equilibriumTides', 'planetary-earth-moon-tides', 1, (diag) => diag.forceAudit.tideAcceleration],
                ['radiationForces', 'planetary-radiation-lab', 1, (diag) => diag.forceAudit.radiationPressureAcceleration],
                ['solarWindDrag', 'planetary-radiation-lab', 1, (diag) => diag.forceAudit.solarWindDragAcceleration],
                ['stellarMassLoss', 'planetary-solar', 1, (diag) => diag.stellarMassLost, 'day'],
                ['atmosphericDrag', 'planetary-atmosphere-entry', 1, (diag) => diag.forceAudit.atmosphereAcceleration],
                ['finiteBodyCollisions', 'planetary-impact-lab', 15, (diag) => diag.collisionCount],
                ['rocheDisruption', 'planetary-roche-lab', 2, (diag) => diag.rocheDisruptionCount],
            ];
            return cases.map(([key, scenario, ticks, metric, timeStep]) => {
                const enabled = new PlanetaryMockBridge();
                enabled.setupScenario(scenario);
                if (timeStep) enabled.setTimeStep(timeStep);
                const enabledActive = enabled.getPhysicsStatus().find((item) => item.key === key)?.active;
                enabled.run(ticks);
                const disabled = new PlanetaryMockBridge();
                disabled.setPhysicsToggle(key, false);
                disabled.setupScenario(scenario);
                if (timeStep) disabled.setTimeStep(timeStep);
                const disabledActive = disabled.getPhysicsStatus().find((item) => item.key === key)?.active;
                disabled.run(ticks);
                return {
                    key,
                    enabled: metric(enabled.getDiagnostics()),
                    disabled: metric(disabled.getDiagnostics()),
                    enabledActive,
                    disabledActive,
                };
            });
        });

        expect(witnesses.map((witness) => witness.key)).toEqual([
            'newtonianGravity', 'relativity1PN', 'oblatenessJ2', 'equilibriumTides',
            'radiationForces', 'solarWindDrag', 'stellarMassLoss', 'atmosphericDrag',
            'finiteBodyCollisions', 'rocheDisruption',
        ]);
        for (const witness of witnesses) {
            expect(witness.enabledActive, `${witness.key} enabled status`).toBe(true);
            expect(witness.disabledActive, `${witness.key} disabled status`).toBe(false);
            expect(witness.enabled, `${witness.key} enabled witness`).toBeGreaterThan(0);
            expect(witness.disabled, `${witness.key} disabled witness`).toBe(0);
        }
    });

    test('radiation, atmosphere, Roche, and impact laboratories exercise real event paths', async ({ page }) => {
        await page.goto('/js/constants.js');
        const result = await page.evaluate(async () => {
            const { PlanetaryMockBridge } = await import('/js/bridge/mock-scale4.js?solar-labs=1');
            const runLab = (scenario, runs) => {
                const bridge = new PlanetaryMockBridge();
                bridge.setupScenario(scenario);
                const initialMass = bridge.getBodies().reduce((sum, body) => sum + body.mass, 0);
                const initialMomentum = bridge.getBodies().reduce((sum, body) => ({
                    x: sum.x + body.mass * body.vx,
                    y: sum.y + body.mass * body.vy,
                    z: sum.z + body.mass * body.vz,
                }), { x: 0, y: 0, z: 0 });
                bridge.run(runs);
                const bodies = bridge.getBodies();
                const finalMomentum = bodies.reduce((sum, body) => ({
                    x: sum.x + body.mass * body.vx,
                    y: sum.y + body.mass * body.vy,
                    z: sum.z + body.mass * body.vz,
                }), { x: 0, y: 0, z: 0 });
                return {
                    diag: bridge.getDiagnostics(), bodies, events: bridge.getEvents(), initialMass,
                    finalMass: bodies.reduce((sum, body) => sum + body.mass, 0),
                    momentumChange: Math.hypot(
                        finalMomentum.x - initialMomentum.x,
                        finalMomentum.y - initialMomentum.y,
                        finalMomentum.z - initialMomentum.z,
                    ),
                };
            };
            const radiation = runLab('planetary-radiation-lab', 2);
            const atmosphere = runLab('planetary-atmosphere-entry', 2);
            const roche = runLab('planetary-roche-lab', 2);
            const impact = runLab('planetary-impact-lab', 15);
            return {
                radiationBetas: radiation.bodies.filter((body) => body.radiationBeta > 0).map((body) => body.radiationBeta),
                radiationAcceleration: radiation.diag.forceAudit.radiationAcceleration,
                atmosphereEntries: atmosphere.diag.atmosphereEntryCount,
                atmosphereDrag: atmosphere.diag.forceAudit.atmosphereAcceleration,
                rocheEvents: roche.diag.rocheDisruptionCount,
                rocheBodies: roche.diag.bodyCount,
                rocheFragments: roche.bodies.filter((body) => body.rocheFragment).length,
                impactEvents: impact.diag.collisionCount,
                impactBodies: impact.diag.bodyCount,
                impactDissipation: impact.diag.dissipatedEnergy,
                rocheMassError: Math.abs(roche.finalMass - roche.initialMass),
                impactMassError: Math.abs(impact.finalMass - impact.initialMass),
                impactMomentumChange: impact.momentumChange,
                finite: [radiation, atmosphere, roche, impact].every((lab) => lab.bodies.every((body) => ['x', 'y', 'z', 'vx', 'vy', 'vz', 'mass'].every((key) => Number.isFinite(body[key])))),
            };
        });

        expect(result.radiationBetas.length).toBe(3);
        expect(result.radiationBetas[0]).toBeGreaterThan(result.radiationBetas[1]);
        expect(result.radiationBetas[1]).toBeGreaterThan(result.radiationBetas[2]);
        expect(result.radiationAcceleration).toBeGreaterThan(0);
        expect(result.atmosphereEntries).toBe(1);
        expect(result.atmosphereDrag).toBeGreaterThan(0);
        expect(result.rocheEvents).toBe(1);
        expect(result.rocheBodies).toBe(9);
        expect(result.rocheFragments).toBe(8);
        expect(result.impactEvents).toBe(1);
        expect(result.impactBodies).toBe(1);
        expect(result.impactDissipation).toBeGreaterThan(0);
        expect(result.rocheMassError).toBeLessThan(1e-20);
        expect(result.impactMassError).toBeLessThan(1e-30);
        expect(result.impactMomentumChange).toBeLessThan(1e-30);
        expect(result.finite).toBe(true);
    });

    test('every example system preserves catalog scale and uses classified modeled colors', async ({ page }) => {
        await page.goto('/js/constants.js');
        const audit = await page.evaluate(async () => {
            const [{ PlanetaryMockBridge }, catalog, reference] = await Promise.all([
                import('/js/bridge/mock-scale4.js?exoplanet-contract=1'),
                import('/js/config/exoplanet-seeds.js?v=2'),
                import('/js/config/solar-system-data.js?v=4'),
            ]);
            return Object.entries(catalog.EXOPLANET_SYSTEMS).map(([name, system]) => {
                const bridge = new PlanetaryMockBridge();
                bridge.setupScenario(`exo-${name}`);
                const bodies = bridge.getBodies();
                const star = bodies.find((body) => body.type === PlanetaryMockBridge.TYPE.STAR);
                const planets = bodies.filter((body) => body.parentId === star.id);
                const totalMass = bodies.reduce((sum, body) => sum + body.mass, 0);
                const center = bodies.reduce((acc, body) => ({
                    x: acc.x + body.mass * body.x,
                    y: acc.y + body.mass * body.y,
                    z: acc.z + body.mass * body.z,
                    vx: acc.vx + body.mass * body.vx,
                    vy: acc.vy + body.mass * body.vy,
                    vz: acc.vz + body.mass * body.vz,
                }), { x: 0, y: 0, z: 0, vx: 0, vy: 0, vz: 0 });
                return {
                    name,
                    count: planets.length,
                    hostMass: star.mass,
                    expectedHostMass: system.host.massSolar,
                    hostRadiusKm: star.radiusKm,
                    expectedHostRadiusKm: system.host.radiusSolar * reference.SUN_REFERENCE.radiusKm,
                    hostTemperatureK: star.temperatureK,
                    expectedHostTemperatureK: system.host.temperatureK,
                    hostColor: star.color,
                    expectedHostColor: system.host.color,
                    maxRadiusError: Math.max(...planets.map((body) => Math.abs(body.r - body.radiusKm / reference.KM_PER_AU))),
                    maxSemimajorError: Math.max(...planets.map((body, index) => Math.abs(body.orbit.a - system.planets[index].semiMajorAxisAu))),
                    maxPeriapsisError: Math.max(...planets.map((body, index) => {
                        const distance = Math.hypot(body.x - star.x, body.y - star.y, body.z - star.z);
                        return Math.abs(distance - system.planets[index].semiMajorAxisAu * (1 - system.planets[index].eccentricity));
                    })),
                    maxMassError: Math.max(...planets.map((body, index) => Math.abs(body.mass - system.planets[index].massEarth * catalog.EARTH_MASS_SOLAR))),
                    centerMagnitude: Math.hypot(center.x, center.y, center.z) / totalMass,
                    momentumMagnitude: Math.hypot(center.vx, center.vy, center.vz),
                    appearanceValid: planets.every((body) => body.style >= 13 && body.style <= 15
                        && /^#[0-9a-f]{6}$/i.test(body.color)
                        && body.colorBasis.includes('not observed true color')),
                    rockyTypesValid: planets.every((body) => body.appearanceClass === 'rocky'
                        ? body.type === PlanetaryMockBridge.TYPE.ROCKY_PLANET
                        : body.type === PlanetaryMockBridge.TYPE.GAS_GIANT),
                    imposedEccentricities: planets.filter((body) => body.catalogEccentricity == null)
                        .map((body) => ({ name: body.name, e: body.eccentricity, basis: body.eccentricityBasis })),
                    ordered: planets.every((body, index) => index === 0 || body.orbit.a > planets[index - 1].orbit.a),
                    provenance: bridge.getPlanetaryData().provenance,
                };
            });
        });

        expect(audit.map((entry) => [entry.name, entry.count])).toEqual([
            ['TRAPPIST-1', 7], ['Kepler-11', 6], ['HR 8799', 4], ['Kepler-20', 6],
        ]);
        for (const system of audit) {
            expect(system.hostMass, system.name).toBe(system.expectedHostMass);
            expect(system.hostRadiusKm, system.name).toBe(system.expectedHostRadiusKm);
            expect(system.hostTemperatureK, system.name).toBe(system.expectedHostTemperatureK);
            expect(system.hostColor, system.name).toBe(system.expectedHostColor);
            expect(system.maxRadiusError, system.name).toBe(0);
            expect(system.maxSemimajorError, system.name).toBe(0);
            expect(system.maxPeriapsisError, system.name).toBeLessThan(1e-13);
            expect(system.maxMassError, system.name).toBe(0);
            expect(system.centerMagnitude, system.name).toBeLessThan(1e-14);
            expect(system.momentumMagnitude, system.name).toBeLessThan(1e-14);
            expect(system.appearanceValid, system.name).toBe(true);
            expect(system.rockyTypesValid, system.name).toBe(true);
            expect(system.ordered, system.name).toBe(true);
            expect(system.provenance.epistemic, system.name).toBe('[PARAMETRIC]');
            expect(system.provenance.appearance, system.name).toContain('not observed true color');
        }
        expect(audit.flatMap((system) => system.imposedEccentricities)).toEqual([{
            name: 'HR 8799 b',
            e: 0,
            basis: '[IMPOSED] circular initialization because the catalog composite value is unavailable',
        }]);
    });

    test('physics controls expose all kernels, persist requests, and explain standby state', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'planetary');
        await expect(page.locator('[data-solar-physics]')).toHaveCount(10);
        await expect(page.locator('[data-solar-physics]:checked')).toHaveCount(10);
        await expect(page.locator('#planetary-physics-summary')).toContainText('10 active · 10 applicable');
        const exoplanetContracts = await page.evaluate(async () => {
            const { EXOPLANET_SEEDS } = await import('/js/config/exoplanet-seeds.js');
            const selected = [...document.querySelectorAll('#planetary-scenario-select option[value^="exo-"]')]
                .map((option) => option.value.slice(4));
            return selected.map((name) => ({ name, records: EXOPLANET_SEEDS[name]?.length || 0 }));
        });
        expect(exoplanetContracts.every((entry) => entry.records > 0)).toBe(true);
        expect(exoplanetContracts.some((entry) => entry.name === 'Kepler-90')).toBe(false);

        const toggleAudit = await page.evaluate(() => [...document.querySelectorAll('[data-solar-physics]')]
            .map((input) => {
                const key = input.dataset.solarPhysics;
                input.click();
                const offState = document.getElementById(`planetary-physics-state-${key}`)?.textContent;
                const offSummary = document.getElementById('planetary-physics-summary')?.textContent;
                input.click();
                const onState = document.getElementById(`planetary-physics-state-${key}`)?.textContent;
                const onSummary = document.getElementById('planetary-physics-summary')?.textContent;
                return { key, offState, offSummary, onState, onSummary };
            }));
        for (const result of toggleAudit) {
            expect(result.offState, result.key).toBe('off');
            expect(result.offSummary, result.key).not.toContain('10 active');
            expect(result.onState, result.key).toBe('active');
            expect(result.onSummary, result.key).toContain('10 active · 10 applicable');
        }

        const gravityKernel = page.locator('#planetary-physics-newtonianGravity');
        await gravityKernel.uncheck();
        await expect.poll(() => page.evaluate(() => window.__ftdCtx.inspector
            ._planetaryRenderer.getPhysicsOverlayStatus().gravityField.visible)).toBe(false);
        for (const dependent of ['relativity1PN', 'oblatenessJ2', 'equilibriumTides']) {
            await expect(page.locator(`#planetary-physics-state-${dependent}`)).toHaveText('standby');
        }
        await gravityKernel.check();
        await expect.poll(() => page.evaluate(() => window.__ftdCtx.inspector
            ._planetaryRenderer.getPhysicsOverlayStatus().gravityField.visible)).toBe(true);

        const relativity = page.locator('#planetary-physics-relativity1PN');
        await relativity.uncheck();
        await expect(page.locator('#planetary-physics-state-relativity1PN')).toHaveText('off');
        await page.locator('#planetary-scenario-select').selectOption('planetary-threebody');
        await expect(relativity).not.toBeChecked();
        await page.locator('#planetary-physics-enable-all').click();
        await expect(relativity).toBeChecked();
        await expect(page.locator('#planetary-physics-state-relativity1PN')).toHaveText('standby');
        await expect(page.locator('#planetary-physics-summary')).toContainText('2 active · 2 applicable · 10 available');

        await page.locator('#planetary-scenario-select').selectOption('planetary-roche-lab');
        await expect(page.locator('#planetary-layer-list [data-body-id]')).toHaveCount(2);
        await page.locator('#planetary-opt-roche').check();
        await expect.poll(() => page.evaluate(() => window.__ftdCtx.inspector
            ._planetaryRenderer.getPhysicsOverlayStatus().rocheLimits)).toBe(1);
        await page.locator('#planetary-physics-rocheDisruption').uncheck();
        await expect.poll(() => page.evaluate(() => window.__ftdCtx.inspector
            ._planetaryRenderer.getPhysicsOverlayStatus().rocheLimits)).toBe(0);
        await page.locator('#planetary-physics-rocheDisruption').check();
        await expect.poll(() => page.evaluate(() => window.__ftdCtx.inspector
            ._planetaryRenderer.getPhysicsOverlayStatus().rocheLimits)).toBe(1);
        await page.locator('#btn-step').click();
        await expect(page.locator('#planetary-layer-list [data-body-id]')).toHaveCount(9);
        await expect(page.locator('#planetary-audit-roche')).toHaveText('1');
        await expect(page.locator('#planetary-audit-latest-event')).toContainText('fluid Roche threshold');
        await expect(page.locator('#planetary-live-bodies')).toContainText('8 debris');
        expect(realErrors(errors)).toEqual([]);
    });
});

import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { BOHR_RADIUS_M, E_REST, FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M, K_B } from '../js/constants.js';
import { formatLength as engineLength } from '../js/units.js';
import { PE_VIS_BOUNDARY_R } from '../js/viewport/constants.js';
import { CosmicMockBridge } from '../js/bridge/mock-scale5.js';
import {
    formatLength,
    formatGaugeLength,
    formatEnergy,
    pointAttributeSize,
    pointSpritePixels,
    pointSizeColor,
    activationEnergyEv,
    energyStringPath,
    energyAreaPath,
    clockEnvironmentStep,
    voxelClockPhase,
    globalClockPhase,
    relativeClockEnvironment,
    formatClockVersus,
    manifestedSiteName,
    anchorRuler,
    lengthGauge,
    rulerInsets,
    RULER_END_LABEL_PX,
    RULER_MIN_WIDTH_PX,
    latticeScale,
    niceStep,
    viewportScale,
    visibleLatticeSpan,
    VOXEL_LENGTH_M,
} from '../js/ui/components/live-rulers/measure.js';

const camera = { fovDeg: 45, viewWidth: 1000, viewHeight: 500 };

test('zooming out and shrinking scenario scale both widen the visible lattice', () => {
    const near = visibleLatticeSpan({ ...camera, distance: 40, scenarioScale: 1 });
    const far = visibleLatticeSpan({ ...camera, distance: 80, scenarioScale: 1 });
    const shrunk = visibleLatticeSpan({ ...camera, distance: 40, scenarioScale: 0.5 });
    assert.ok(far.latticeWidth > near.latticeWidth * 1.9);
    assert.ok(Math.abs(shrunk.latticeWidth - near.latticeWidth * 2) < 1e-9);
    assert.ok(shrunk.pixelsPerVoxel < near.pixelsPerVoxel);
});

test('a single voxel in view uses the discrete one-voxel step', () => {
    assert.equal(niceStep(1, 8), 1);
    assert.equal(niceStep(1.6, 8), 1);
    const scale = viewportScale(1);
    assert.equal(scale.step, 1);
    assert.deepEqual(scale.ticks, [0, 1]);
    const bar = latticeScale(33, 1000, 1000);
    assert.equal(bar.voxels, 1);
});

test('inside one voxel the viewport ruler keeps subdividing', () => {
    assert.ok(niceStep(0.4, 8) < 1);
    const scale = viewportScale(0.4);
    assert.ok(scale.step < 1);
    assert.ok(scale.ticks.length >= 3);
});

test('lattice bar follows lattice size when the whole lattice fits', () => {
    const small = latticeScale(17, 8, 1000);
    const large = latticeScale(65, 8, 1000);
    assert.equal(small.voxels, 17);
    assert.equal(small.latticeSize, 17);
    assert.equal(large.latticeSize, 65);
    assert.ok(large.voxels <= 65);
    assert.ok(large.px <= 900);
});

test('the viewport ruler stops at the inner edges of the side panels', () => {
    const view = { left: 0, right: 1000, top: 0, bottom: 600, width: 1000, height: 600 };
    const open = rulerInsets(view, [
        { left: 0, right: 280, top: 20, bottom: 560, width: 280, height: 540 },
        { left: 680, right: 1000, top: 8, bottom: 520, width: 320, height: 512 },
    ]);
    assert.equal(open.left, 288);
    const insetRail = rulerInsets(view, [
        { left: 52, right: 280, top: 20, bottom: 560, width: 228, height: 540 },
    ]);
    assert.equal(insetRail.left, 288);
    assert.equal(open.right, 328);
    const collapsed = rulerInsets(view, [
        { left: 776, right: 1000, top: 8, bottom: 48, width: 224, height: 40 },
    ]);
    assert.equal(collapsed.right, 232);
    const bottom = rulerInsets(view, [
        { left: 0, right: 1000, top: 400, bottom: 600, width: 1000, height: 200 },
    ]);
    assert.equal(bottom.left, 8);
    assert.equal(bottom.right, 8);
});

test('the viewport ruler takes the widest open stretch of its own row', () => {
    const view = { left: 0, right: 2000, top: 71, bottom: 745, width: 2000, height: 674 };
    const rect = (left, right, top, bottom) => ({ left, right, top, bottom, width: right - left, height: bottom - top });
    // A collapsed dock leaves only its icon rail and grip: the ruler starts beside them.
    const rail = rulerInsets(view, [rect(12, 56, 83, 705), rect(50, 62, 83, 705), rect(1745, 1988, 79, 121)]);
    assert.equal(rail.left, 70);
    assert.equal(rail.right, 263);
    assert.equal(rail.width, 2000 - 70 - 263);
    assert.equal(rail.hidden, false);
    // The play bar docked in the top row is an obstacle like any other.
    const playBar = rulerInsets(view, [rect(13, 585, 85, 131), rect(1640, 1988, 79, 705)]);
    assert.equal(playBar.left, 593);
    assert.equal(playBar.right, 368);
    // Something in the middle of the row splits it; the wider side wins.
    const middle = rulerInsets(view, [rect(600, 800, 80, 120)]);
    assert.equal(middle.left, 808);
    assert.equal(middle.right, 8);
    // What starts below the row is not in the way, however tall it is.
    const below = rulerInsets(view, [rect(0, 400, 71 + 70, 745)]);
    assert.equal(below.left, 8);
    // Too little room: the ruler is hidden rather than squeezed.
    const cramped = rulerInsets(view, [rect(0, 900, 80, 700), rect(1100, 2000, 80, 700)]);
    assert.equal(cramped.width, 184);
    assert.equal(cramped.hidden, true);
    assert.equal(rulerInsets(view, [rect(0, 800, 80, 700), rect(1100, 2000, 80, 700)]).hidden, false);
    assert.ok(RULER_MIN_WIDTH_PX > RULER_END_LABEL_PX, 'a ruler that is shown always has room for its end label');
});

test('ticks are as many as the width can label, and none sits under the end label', () => {
    assert.deepEqual(viewportScale(100).ticks, [0, 20, 40, 60, 80, 100], 'default is unchanged');
    // A narrow ruler asks for fewer divisions.
    assert.deepEqual(viewportScale(100, 3).ticks, [0, 50, 100]);
    // 96.4 is too close to 100 for both labels; with clearance it is dropped.
    assert.deepEqual(viewportScale(104, 8).ticks, [0, 20, 40, 60, 80, 100, 104]);
    assert.deepEqual(viewportScale(104, 8, 0.12).ticks, [0, 20, 40, 60, 80, 104]);
    const scale = viewportScale(180.9, 8, RULER_END_LABEL_PX / 1667);
    assert.deepEqual(scale.ticks, [0, 50, 100, 150, 180.9]);
});

test('the bracket moves from the nearest voxel to the lattice to the quasi-domain', () => {
    const close = anchorRuler({ domainUnits: 33, pixelsPerUnit: 100, viewPx: 1000, subject: 'lattice' });
    assert.equal(close.subject, 'voxel');
    assert.equal(close.units, 1);
    const body = anchorRuler({ domainUnits: 33, pixelsPerUnit: 4, viewPx: 1000, subject: 'lattice' });
    assert.equal(body.subject, 'lattice');
    assert.equal(body.px, 132);
    const popped = anchorRuler({ domainUnits: 33, pixelsPerUnit: 4 / 33, viewPx: 1600, subject: 'lattice' });
    assert.equal(popped.subject, 'quasi-domain');
    assert.equal(popped.units, 33 * Math.sqrt(3));
    assert.ok(Math.abs(popped.px - 4 * Math.sqrt(3)) < 1e-6);
    assert.ok(popped.opacity > 0 && popped.opacity < 1);
    const early = anchorRuler({ domainUnits: 33, pixelsPerUnit: 1, viewPx: 1600, subject: 'lattice', quasiUnits: 1000 });
    assert.equal(early.subject, 'quasi-domain');
    assert.equal(early.units, 1000);
    assert.ok(early.px > 33);
    const stillLattice = anchorRuler({ domainUnits: 33, pixelsPerUnit: 4, viewPx: 1600, subject: 'lattice', quasiUnits: 1000 });
    assert.equal(stillLattice.subject, 'lattice');
    const shell = anchorRuler({ domainUnits: 33, pixelsPerUnit: 0.14, viewPx: 1600, subject: 'lattice', quasiUnits: 400 });
    assert.equal(shell.subject, 'quasi-domain');
    assert.equal(shell.units, 400);
    assert.equal(shell.opacity, 1);
    const gone = anchorRuler({ domainUnits: 33, pixelsPerUnit: 0.001, viewPx: 1600, subject: 'cloud' });
    assert.equal(gone.hidden, true);
    assert.equal(lengthGauge('planetary').subject, 'system');
    assert.equal(lengthGauge('cosmic').subject, 'cosmic');
});

test('a voxel ruler matches the flux point sprite and colors by point scale', () => {
    assert.equal(pointAttributeSize(1), 10);
    assert.equal(pointAttributeSize(0.1), 1.9);
    assert.equal(pointAttributeSize(3), 28);
    const near = pointSpritePixels(10, 1);
    const far = pointSpritePixels(10, 60);
    assert.ok(near > far);
    assert.equal(pointSpritePixels(100, 0.1), 512);
    const restEv = activationEnergyEv(Math.sqrt(2 * E_REST));
    assert.ok(Math.abs(restEv - K_B * 1e6) / (K_B * 1e6) < 1e-9);
    assert.equal(manifestedSiteName(0), '');
    assert.equal(manifestedSiteName(1), 'positive');
    assert.equal(manifestedSiteName(-1), 'negative');
    assert.equal(manifestedSiteName(2), 'locked positive');
    assert.equal(clockEnvironmentStep(0, 1), 0);
    assert.equal(clockEnvironmentStep(4, 0), 0);
    assert.ok(clockEnvironmentStep(4, 1) > 0);
    assert.ok(clockEnvironmentStep(4, -1) < 0);
    assert.equal(voxelClockPhase(0, 0), 0);
    assert.equal(voxelClockPhase(10, 0), globalClockPhase(10));
    assert.notEqual(voxelClockPhase(10, 0.2), voxelClockPhase(10, 0));
    assert.ok(voxelClockPhase(3, 0.2) >= 0 && voxelClockPhase(3, 0.2) < 1);
    assert.equal(relativeClockEnvironment(5, 5, 4), 0);
    assert.equal(relativeClockEnvironment(9, 5, 4), 1);
    assert.equal(formatClockVersus(0, 0), '0.000 vs tick 0 (+0.000)');
    const path = energyStringPath([1, 3], 1, 3);
    assert.match(path, /^M0\.00 14\.00L100\.00 2\.00$/);
    assert.equal(energyAreaPath([1, 3], 1, 3), 'M0.00 14.00L100.00 2.00L100.00 16L0.00 16Z');
    assert.equal(pointSizeColor(0.1), 'hsl(240.0 78% 58%)');
    assert.equal(pointSizeColor(3), 'hsl(0.0 78% 58%)');
});

test('one voxel is the electron-primary Planck length, written in scientific notation', () => {
    assert.equal(VOXEL_LENGTH_M, FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M);
    assert.ok(Math.abs(VOXEL_LENGTH_M - 1.6131463e-35) / 1.6131463e-35 < 1e-6);
    assert.equal(formatLength(0), '0');
    assert.equal(formatLength(1), '1.613×10⁻³⁵ m');
    assert.equal(formatEnergy(0), '0 J');
    assert.equal(formatEnergy(K_B * 1e6), '8.187×10⁻¹⁴ J');
    assert.equal(formatEnergy(0.4), '6.409×10⁻²⁰ J');
    assert.match(formatLength(33), /^5\.323×10⁻³⁴ m$/);
    assert.doesNotMatch(formatLength(1), /vx/);
    const fine = viewportScale(0.04);
    const labels = fine.ticks.map((value) => formatLength(value, fine.step));
    assert.equal(new Set(labels).size, labels.length);
});

const OFF_LATTICE = ['particles', 'atoms', 'molecules', 'planetary', 'cosmic'];

test('each scale reads in the length unit its own engine declares', () => {
    const lattice = lengthGauge('lattice');
    assert.equal(lattice.metresPerUnit, FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M);
    assert.equal(lengthGauge(undefined), lattice);
    // Scales 2 and 3 run one atom engine. Its unit layer turns a simulation
    // unit into ångströms through the Bohr radius, so the ruler must agree.
    const engineAtomUnit = engineLength(1, 2);
    assert.equal(engineAtomUnit.unit, 'Å');
    for (const mode of ['atoms', 'molecules']) {
        const gauge = lengthGauge(mode);
        assert.equal(gauge.metresPerUnit, BOHR_RADIUS_M);
        assert.ok(Math.abs(gauge.metresPerUnit - engineAtomUnit.value * 1e-10) / BOHR_RADIUS_M < 1e-12);
        assert.equal(gauge.unit, 'a₀');
    }
    assert.equal(lengthGauge('planetary').metresPerUnit, 1.495978707e11);
    assert.equal(lengthGauge('planetary').unit, 'AU');
    // Scales 1 and 5 report lengths in lattice units and convert them to
    // metres nowhere, so the ruler carries no metre length for them either.
    assert.equal(engineLength(1, 1).unit, 'lu');
    for (const mode of ['particles', 'cosmic']) {
        const gauge = lengthGauge(mode);
        assert.equal(gauge.metresPerUnit, null);
        assert.equal(gauge.unit, 'lu');
    }
});

test('the drawn body on each scale is an extent its engine defines', () => {
    assert.equal(lengthGauge('lattice').domainUnits, null);
    assert.equal(lengthGauge('particles').domainUnits, 2 * PE_VIS_BOUNDARY_R);
    assert.equal(lengthGauge('particles').subject, 'reference shell');
    const atomEngine = readFileSync(new URL('../js/bridge/mock-atom-engine.js', import.meta.url), 'utf8');
    const containment = atomEngine.match(/_reflectIntoBoundary\(a, 0, 0, 0, (\d+(?:\.\d+)?)\)/);
    assert.ok(containment, 'the atom engine reflects atoms into an origin-centred sphere');
    for (const mode of ['atoms', 'molecules']) {
        assert.equal(lengthGauge(mode).domainUnits, 2 * Number(containment[1]));
        assert.equal(lengthGauge(mode).subject, 'boundary');
    }
    assert.equal(lengthGauge('cosmic').domainUnits, new CosmicMockBridge().getRuntimeParams().boxSize);
    assert.equal(lengthGauge('planetary').domainUnits, 30);
});

test('a length is written in metres only where the engine declares a metre length', () => {
    assert.equal(formatGaugeLength(lengthGauge('lattice'), 33, 1), '5.323×10⁻³⁴ m');
    assert.equal(formatGaugeLength(lengthGauge('atoms'), 1, 1), '5.292×10⁻¹¹ m');
    assert.equal(formatGaugeLength(lengthGauge('molecules'), 70, 70), '3.704×10⁻⁹ m');
    assert.equal(formatGaugeLength(lengthGauge('planetary'), 1, 1), '1.496×10¹¹ m');
    for (const mode of ['particles', 'cosmic']) {
        const gauge = lengthGauge(mode);
        assert.equal(formatGaugeLength(gauge, 0, 5), '0');
        assert.equal(formatGaugeLength(gauge, 5, 5), '5 lu');
        assert.equal(formatGaugeLength(gauge, 46.2371, 5), '46.24 lu');
        assert.equal(formatGaugeLength(gauge, 0.015, 0.005), '0.015 lu');
        assert.equal(formatGaugeLength(gauge, 2.5e7, 5e6), '2.500×10⁷ lu');
        assert.equal(formatGaugeLength(gauge, 4e-5, 5e-6), '4.000×10⁻⁵ lu');
        assert.equal(formatGaugeLength(gauge, NaN), '—');
        const fine = viewportScale(0.04);
        const labels = fine.ticks.map((value) => formatGaugeLength(gauge, value, fine.step));
        assert.equal(new Set(labels).size, labels.length);
        assert.ok(labels.every((label) => !/ m$/.test(label)));
    }
});

test('each ruler tooltip states its own scale and marks the unit as an input', () => {
    const lattice = lengthGauge('lattice');
    assert.match(lattice.viewTitle, /One voxel is the electron-primary Planck length/);
    assert.match(lattice.bodyTitle, /One voxel is the electron-primary Planck length/);
    for (const mode of OFF_LATTICE) {
        const gauge = lengthGauge(mode);
        for (const title of [gauge.viewTitle, gauge.bodyTitle]) {
            assert.doesNotMatch(title, /voxel|Planck/, `${mode} is not drawn in voxels`);
            assert.doesNotMatch(title, /deriv/i, `${mode} unit is not presented as derived`);
        }
    }
    for (const mode of ['atoms', 'molecules']) {
        assert.match(lengthGauge(mode).viewTitle, /in metres/);
        assert.match(lengthGauge(mode).viewTitle, /Bohr radius/);
        assert.match(lengthGauge(mode).viewTitle, /input/);
    }
    assert.match(lengthGauge('planetary').viewTitle, /astronomical unit/);
    for (const mode of ['particles', 'cosmic']) {
        assert.match(lengthGauge(mode).viewTitle, /lattice units \(lu\)/);
        assert.match(lengthGauge(mode).viewTitle, /no metre/);
    }
    assert.notEqual(lengthGauge('particles').viewTitle, lengthGauge('cosmic').viewTitle);
});

test('off the lattice the closest bracket is one engine unit, not a voxel', () => {
    for (const mode of OFF_LATTICE) {
        const gauge = lengthGauge(mode);
        const close = anchorRuler({
            domainUnits: gauge.domainUnits, pixelsPerUnit: 100, viewPx: 1000,
            subject: gauge.subject, unitSubject: gauge.unitSubject,
        });
        assert.equal(close.units, 1);
        assert.equal(close.subject, gauge.unitSubject);
        assert.notEqual(close.subject, 'voxel');
    }
    assert.equal(lengthGauge('lattice').unitSubject, 'voxel');
    assert.equal(lengthGauge('atoms').unitSubject, 'Bohr radius');
});

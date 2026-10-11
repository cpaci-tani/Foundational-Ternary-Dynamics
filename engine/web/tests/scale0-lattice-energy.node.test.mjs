import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { E_REST, J_PER_EV, K_B } from '../js/constants.js';
import { readScale0LatticeEnergy } from '../js/telemetry/scale0-read.js';
import {
    activationEnergyEv, ENERGY_TITLES, formatEnergy, latticeEnergyEv, placeString,
} from '../js/ui/components/live-rulers/measure.js';

const current = (tick, extra = {}) => ({ tick, stale: false, status: 'available', sourceEpoch: 4, ...extra });

test('one engine energy unit is 3 MeV, so one rest energy reads as the electron rest energy', () => {
    assert.ok(Math.abs(latticeEnergyEv(1) - 3e6) < 1e-6);
    assert.ok(Math.abs(latticeEnergyEv(E_REST) - K_B * 1e6) / (K_B * 1e6) < 1e-12);
    assert.equal(formatEnergy(latticeEnergyEv(E_REST)), '8.187×10⁻¹⁴ J');
    assert.equal(latticeEnergyEv(NaN), 0);
    // The voxel figure uses the same conversion.
    assert.equal(activationEnergyEv(2), latticeEnergyEv(2));
    assert.ok(Math.abs(latticeEnergyEv(2.5) * J_PER_EV - 2.5 * 3e6 * J_PER_EV) < 1e-24);
});

test('the lattice total is the dynamic channel plus the rest energy of the manifested sites', () => {
    const diag = { dynamicEnergy: 12.5, manifested: 4, totalEnergy: 999, energySampleSource: 'per-tick-ledger' };
    const reading = readScale0LatticeEnergy(diag, null, { diagMeta: current(40), auditMeta: null });
    assert.deepEqual(reading, { value: 12.5 + 4 * E_REST, tick: 40, epoch: 4, source: 'per-tick-ledger' });
    // An empty lattice is a reading of zero, not a missing one.
    const empty = readScale0LatticeEnergy({ dynamicEnergy: 0, manifested: 0 }, null, { diagMeta: current(1) });
    assert.equal(empty.value, 0);
});

test('an audit of the same tick is used as the engine reports it', () => {
    const diag = { dynamicEnergy: 12.5, manifested: 4 };
    const audit = { totalEnergy: 13.25, dynamicEnergy: 12.5 };
    const reading = readScale0LatticeEnergy(diag, audit, { diagMeta: current(40), auditMeta: current(40) });
    assert.equal(reading.value, 13.25);
    assert.equal(reading.source, 'same-tick-audit');
});

test('an audit of another tick is never mixed into the current total', () => {
    const diag = { dynamicEnergy: 12.5, manifested: 4 };
    const audit = { totalEnergy: 500, dynamicEnergy: 499 };
    const older = readScale0LatticeEnergy(diag, audit, { diagMeta: current(40), auditMeta: current(36) });
    assert.equal(older.value, 12.5 + 4 * E_REST);
    assert.equal(older.tick, 40);
    const stale = readScale0LatticeEnergy(diag, audit, { diagMeta: current(40), auditMeta: current(40, { stale: true }) });
    assert.equal(stale.value, 12.5 + 4 * E_REST);
});

test('without a current observation there is no reading', () => {
    const diag = { dynamicEnergy: 12.5, manifested: 4 };
    assert.equal(readScale0LatticeEnergy(diag, null, { diagMeta: null }), null);
    assert.equal(readScale0LatticeEnergy(diag, null, { diagMeta: current(40, { stale: true }) }), null);
    assert.equal(readScale0LatticeEnergy({ dynamicEnergy: 12.5 }, null, { diagMeta: current(40) }), null);
    // diagnostics.totalEnergy is a different quantity and is never the total.
    assert.equal(readScale0LatticeEnergy({ totalEnergy: 7, manifested: 0 }, null, { diagMeta: current(40) }), null);
});

test('the figure steps aside where the clock face stands over the bracket', () => {
    const strip = { centre: 500, top: 100, lineWidth: 148, valueSize: 16, viewWidth: 1000 };
    // Box: 148 + 18 + 160 = 326 wide, so it starts at 337 and its ink ends at 337 + 278 = 615.
    assert.deepEqual(placeString(strip, null), { shift: 0, compact: false });
    assert.equal(placeString(strip, { x: 500, y: 300, r: 40 }).shift, 0, 'a clock below the strip is no obstacle');
    assert.equal(placeString(strip, { x: 900, y: 110, r: 40 }).shift, 0, 'nor one beside it');
    const left = placeString(strip, { x: 500, y: 110, r: 40 }).shift;
    assert.equal(left, (500 - 40 - 10) - 615, 'the ink ends a gap before the disc');
    assert.ok(337 + left >= 8);
    // With no room on the left it goes to the right of the disc instead.
    const right = placeString({ ...strip, centre: 200 }, { x: 200, y: 110, r: 40 }).shift;
    assert.equal(right, (200 + 40 + 10) - 37);
});

test('the figure stays inside the view, and a phone-width view shows it without the line', () => {
    // A bracket centred near the right edge: the strip is pulled back inside.
    const edge = placeString({ centre: 950, top: 100, viewWidth: 1000 });
    assert.equal(edge.compact, false);
    assert.equal(950 - 163 + edge.shift + 326, 1000 - 8);
    // A 320 px view cannot carry the 326 px strip: the line is dropped and the figure fits.
    const phone = placeString({ centre: 160, top: 100, viewWidth: 320 });
    assert.equal(phone.compact, true);
    const box = 7 * 16;
    assert.ok(160 - box / 2 + phone.shift >= 8 && 160 + box / 2 + phone.shift <= 320 - 8);
    // With the clock in the way and no room to clear it, staying inside the view wins.
    const crowded = placeString({ centre: 160, top: 100, viewWidth: 320 }, { x: 160, y: 110, r: 120 });
    assert.ok(160 - box / 2 + crowded.shift >= 8 && 160 + box / 2 + crowded.shift <= 320 - 8);
    // A lattice wider than the view: the bracket's centre may be off screen, the figure is not.
    const off = placeString({ centre: 700, top: 100, viewWidth: 390 });
    assert.equal(off.compact, false);
    assert.equal(700 - 163 + off.shift + 326, 390 - 8);
});

test('the figure is hidden when it has no reading, and says what it is', () => {
    const css = readFileSync(new URL('../css/ui/components/live-rulers.css', import.meta.url), 'utf8');
    assert.match(css, /\.live-ruler-string\[hidden\]\s*\{\s*display:\s*none;/);
    const mount = readFileSync(new URL('../js/ui/components/live-rulers/mount.js', import.meta.url), 'utf8');
    assert.match(mount, /anchor\.subject === 'lattice'[\s\S]{0,400}reading\.latticeEnergy/);
    assert.match(ENERGY_TITLES.lattice, /^Total energy of the lattice, in joules/);
    assert.match(ENERGY_TITLES.voxel, /not a share of the lattice total/);
});

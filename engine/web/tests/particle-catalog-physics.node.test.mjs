import test from 'node:test';
import assert from 'node:assert/strict';
import {
    getAllParticles, getById, getSimulableParticles, getCatalogSimulationSupport,
} from '../js/particle-catalog.js';
import { ALPHA, M_E, M_E_PHYS, M_PLANCK_MEV } from '../js/constants.js';

const entries = getAllParticles();
const flavors = ['nu_e', 'antinu_e', 'nu_mu', 'antinu_mu', 'nu_tau', 'antinu_tau'];

// Independent conventional reference assignments, rather than comparing one
// catalog transformation against another copy of that transformation.
const quantumNumbers = [
    ['electron', -1, 0.5], ['positron', 1, 0.5],
    ['muon', -1, 0.5], ['antimuon', 1, 0.5],
    ['tau', -1, 0.5], ['antitau', 1, 0.5],
    ...flavors.map(id => [id, 0, 0.5]),
    ['up', 2 / 3, 0.5], ['anti_up', -2 / 3, 0.5],
    ['down', -1 / 3, 0.5], ['anti_down', 1 / 3, 0.5],
    ['strange', -1 / 3, 0.5], ['anti_strange', 1 / 3, 0.5],
    ['charm', 2 / 3, 0.5], ['anti_charm', -2 / 3, 0.5],
    ['bottom', -1 / 3, 0.5], ['anti_bottom', 1 / 3, 0.5],
    ['top', 2 / 3, 0.5], ['anti_top', -2 / 3, 0.5],
    ['photon', 0, 1], ['gluon', 0, 1], ['w_plus', 1, 1],
    ['w_minus', -1, 1], ['z_boson', 0, 1], ['higgs', 0, 0],
    ['proton', 1, 0.5], ['antiproton', -1, 0.5],
    ['neutron', 0, 0.5], ['antineutron', 0, 0.5],
    ['lambda', 0, 0.5], ['antilambda', 0, 0.5],
    ['sigma_plus', 1, 0.5], ['sigma_zero', 0, 0.5], ['sigma_minus', -1, 0.5],
    ['xi_zero', 0, 0.5], ['xi_minus', -1, 0.5],
    ['omega_minus', -1, 1.5], ['delta_pp', 2, 1.5],
    ['pion_plus', 1, 0], ['pion_minus', -1, 0], ['pion_zero', 0, 0],
    ['kaon_plus', 1, 0], ['kaon_minus', -1, 0],
    ['kaon_zero', 0, 0], ['antikaon_zero', 0, 0], ['eta', 0, 0],
    ['rho', 0, 1], ['jpsi', 0, 1], ['upsilon', 0, 1],
];

test('all 54 reference entries retain independently assigned charges and spins', () => {
    assert.equal(entries.length, 54);
    assert.equal(new Set(entries.map(p => p.id)).size, 54);
    assert.deepEqual(entries.map(p => p.id).sort(), quantumNumbers.map(([id]) => id).sort());
    for (const [id, charge, spin] of quantumNumbers) {
        const p = getById(id);
        assert.equal(p.charge, charge, id);
        assert.equal(p.spin, spin, id);
    }
    for (const [ids, generation] of [
        [['electron', 'positron', 'nu_e', 'antinu_e', 'up', 'anti_up', 'down', 'anti_down'], 1],
        [['muon', 'antimuon', 'nu_mu', 'antinu_mu', 'charm', 'anti_charm', 'strange', 'anti_strange'], 2],
        [['tau', 'antitau', 'nu_tau', 'antinu_tau', 'top', 'anti_top', 'bottom', 'anti_bottom'], 3],
    ]) for (const id of ids) assert.equal(getById(id).generation, generation, id);
    assert.equal(getById('gluon').color_charge, 'octet');
    assert.equal(getById('up').color_charge, 'r/g/b');
    assert.equal(getById('anti_up').color_charge, 'r̄/ḡ/b̄');
});

test('listed antiparticles are reciprocal; omitted partners are not self-conjugates', () => {
    const omitted = ['sigma_plus', 'sigma_zero', 'sigma_minus', 'xi_zero', 'xi_minus', 'omega_minus', 'delta_pp'];
    assert.deepEqual(entries.filter(p => p.antiparticle === null).map(p => p.id), omitted);
    for (const p of entries) {
        if (p.antiparticle === null) {
            assert.equal(p.antiparticle_status, 'not-listed');
            assert.match(p.antiparticle_note, /exists.*omitted/);
            continue;
        }
        const anti = getById(p.antiparticle);
        assert.equal(anti.antiparticle, p.id);
        assert.equal(anti.mass_mev, p.mass_mev);
        assert.equal(anti.spin, p.spin);
        assert.equal(anti.charge + p.charge, 0);
        assert.equal(anti.baryon + p.baryon, 0);
        assert.equal(anti.lepton + p.lepton, 0);
        assert.equal(p.antiparticle_status, p.antiparticle === p.id ? 'self-conjugate' : 'listed-partner');
    }
});

test('flavor states have unavailable masses, not fictitious mass measurements or bounds', () => {
    assert.deepEqual(entries.filter(p => p.mass_mev === null).map(p => p.id), flavors);
    for (const id of flavors) {
        const p = getById(id);
        assert.equal(p.mass_status, 'flavor-superposition');
        assert.equal(p.mass_basis, 'flavor');
        assert.match(p.mass_note, /superposition of mass eigenstates/);
        assert.match(p.antiparticle_note, /Dirac\/Majorana.*unresolved/);
        assert.equal(p.ftd_mass_mev, null);
        assert.equal(getCatalogSimulationSupport(p).supported, false);
    }
});

test('physical reference masses and model anchor remain distinct and provenance is present', () => {
    assert.equal(getById('electron').mass_mev, M_E_PHYS);
    assert.equal(getById('positron').mass_mev, M_E_PHYS);
    assert.notEqual(M_E_PHYS, M_E);
    for (const p of entries) {
        assert.ok(p.mass_status && p.mass_basis && p.mass_note, p.id);
        assert.ok(p.mass_source === null || /^https:\/\//.test(p.mass_source), p.id);
        if (p.mass_mev !== null) assert.ok(Number.isFinite(p.mass_mev) && p.mass_mev >= 0, p.id);
    }
    for (const id of ['up', 'anti_up', 'down', 'strange', 'charm', 'bottom']) {
        assert.equal(getById(id).mass_basis, 'MSbar');
        assert.match(getById(id).mass_note, /not a constituent mass/);
    }
    assert.match(getById('up').mass_note, /2 GeV/);
    assert.equal(getById('rho').mass_status, 'approximate-reference');
    assert.equal(getById('rho').mass_mev, 770);
    assert.equal(getById('top').mass_basis, 'unspecified-top-scheme');
    assert.match(getById('eta').composition, /octet–singlet mixture/);
});

test('only five canonical motivating relations and their partners have computed residuals', () => {
    // Expanded independent coefficients make changes to integer recipes visible.
    const expected = {
        electron: M_PLANCK_MEV * Math.sqrt(2 * Math.PI) * 16 / 3 * ALPHA ** 11,
        muon: M_E * 207,
        tau: M_E * 3477,
        proton: M_E * (13 / ALPHA + 55),
        higgs: M_E * 13 / ALPHA ** 2,
    };
    const partner = { positron: 'electron', antimuon: 'muon', antitau: 'tau', antiproton: 'proton' };
    const eligible = [...Object.keys(expected), ...Object.keys(partner)].sort();
    assert.deepEqual(entries.filter(p => p.ftd_mass_mev !== null).map(p => p.id).sort(), eligible);
    for (const p of entries) {
        const id = partner[p.id] || p.id;
        if (!Object.hasOwn(expected, id)) {
            for (const key of ['ftd_formula', 'ftd_mass_mev', 'ftd_accuracy', 'ftd_status', 'ftd_source']) {
                assert.equal(p[key], null, `${p.id}:${key}`);
            }
            continue;
        }
        assert.ok(Math.abs(p.ftd_mass_mev / expected[id] - 1) < 1e-14, p.id);
        const error = 100 * Math.abs(p.ftd_mass_mev - p.mass_mev) / p.mass_mev;
        assert.ok(Math.abs(p.ftd_accuracy - error) < 1e-12, p.id);
        assert.equal(p.ftd_status, ['electron', 'proton'].includes(id) ? 'selection' : 'parametric');
        assert.ok(p.ftd_source.startsWith('docs/theory/'));
        assert.match(p.ftd_note, /not a precision, confidence level, or recovered particle identity/);
    }
});

test('current PE injection rejects fractional charges and unsupported reference states', () => {
    for (const p of entries.filter(p => p.category === 'quarks')) {
        assert.deepEqual(getCatalogSimulationSupport(p), {
            supported: false,
            reason: 'Fractional quark charge is not representable by the current integer-charge particle engine.',
        });
    }
    for (const id of ['photon', 'gluon', 'neutron', 'z_boson', 'higgs', ...flavors]) {
        assert.equal(getCatalogSimulationSupport(getById(id)).supported, false, id);
    }
    for (const id of ['electron', 'positron', 'muon', 'proton', 'delta_pp', 'pion_plus']) {
        assert.equal(getCatalogSimulationSupport(getById(id)).supported, true, id);
    }
    for (const charge of [-129, 128, Number.MAX_SAFE_INTEGER]) {
        const result = getCatalogSimulationSupport({ mass_mev: 1, charge });
        assert.equal(result.supported, false, `charge ${charge}`);
        assert.match(result.reason, /signed 8-bit range/);
    }
    for (const charge of [-128, 127]) {
        assert.equal(getCatalogSimulationSupport({ mass_mev: 1, charge }).supported, true,
            `native charge endpoint ${charge}`);
    }
    for (const invalid of [null, undefined, {}, { mass_mev: 1, charge: NaN },
        { mass_mev: null, charge: 1 }, { mass_mev: Infinity, charge: 1 },
        { mass_mev: -1, charge: 1 }, { mass_mev: '1', charge: 1 }]) {
        const result = getCatalogSimulationSupport(invalid);
        assert.equal(result.supported, false);
        assert.ok(result.reason);
    }
    assert.deepEqual(getSimulableParticles().map(p => p.id),
        entries.filter(p => Number.isFinite(p.mass_mev) && p.mass_mev > 0
            && Number.isInteger(p.charge) && p.charge >= -128 && p.charge <= 127
            && p.charge !== 0).map(p => p.id));
});

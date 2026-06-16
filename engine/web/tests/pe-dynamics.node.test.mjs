// Node unit test: multi-body equilibrium orbit batch seeding.
// Run: node engine/web/tests/pe-dynamics.node.test.mjs
import assert from 'node:assert/strict';
import { createParticleEngine } from '../js/bridge/mock-particle-engine.js';
import { seedAtomicIon, seedBinaryOrbit } from '../js/scales/scale1/pe-dynamics.js';
import { computeForceOnParticle, peTogglesFromState } from '../js/bridge/pe-force-kernel.js';

function makeBridge() {
    const state = { _pe: null, _peParticleTypes: null };
    const engine = createParticleEngine(state);
    engine.initPE();
    const bridge = {
        ...engine,
        peApplyEquilibriumOrbit: (id, opts) => engine.peApplyEquilibriumOrbit(id, opts),
        peApplyEquilibriumOrbitBatch: (entries) => engine.peApplyEquilibriumOrbitBatch(entries),
        peAddParticle: (...args) => engine.peAddParticle(...args),
    };
    return { state, bridge };
}

/** m v²/r vs radial inward force — should match within tolerance for equilibrium IC. */
function assertOrbitBalance(state, idx, center = [0, 0, 0], tol = 0.02) {
    const pe = state._pe;
    const p = pe.particles[idx];
    const rx = p.x - center[0];
    const ry = p.y - center[1];
    const rz = p.z - center[2];
    const r = Math.sqrt(rx * rx + ry * ry + rz * rz);
    assert.ok(r > 1e-6);

    const savedV = [p.vx, p.vy, p.vz];
    p.vx = 0; p.vy = 0; p.vz = 0;
    const f = computeForceOnParticle(pe.particles, idx, peTogglesFromState(pe), pe.soft);
    p.vx = savedV[0]; p.vy = savedV[1]; p.vz = savedV[2];

    const rHatX = rx / r, rHatY = ry / r, rHatZ = rz / r;
    const fInward = -(f.fx * rHatX + f.fy * rHatY + f.fz * rHatZ);
    const centripetal = p.mass * (p.vx * p.vx + p.vy * p.vy + p.vz * p.vz) / r;
    const relErr = Math.abs(centripetal - fInward) / Math.max(fInward, 1e-30);
    assert.ok(relErr < tol, `orbit balance idx=${idx} relErr=${relErr}`);
}

const MP = 1836.15;
const ME = 1.0;
const RE = 0.1;

// Helium: both electrons should satisfy force balance with full 3-body field
{
    const { state, bridge } = makeBridge();
    seedAtomicIon(bridge, { Z: 2, A: 4, mp: MP, me: ME, RE, r: 4, electrons: 2 });
    assert.equal(state._pe.particles.length, 3);
    assertOrbitBalance(state, 1);
    assertOrbitBalance(state, 2);
}

// Positronium: both leptons balanced against mutual attraction
{
    const { state, bridge } = makeBridge();
    seedBinaryOrbit(bridge, {
        catalogA: 'electron', chargeA: -1, massA: ME,
        catalogB: 'positron', chargeB: 1, massB: ME,
        separation: 10, RE,
    });
    assertOrbitBalance(state, 0);
    assertOrbitBalance(state, 1);
}

console.log('pe-dynamics OK');

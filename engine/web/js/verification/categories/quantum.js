/**
 * Verification Lab — Quantum category.
 *
 * Each experiment returns a SCALAR per trial so aggregation is trivial.
 * Complex analyses (histograms, FFT, fringe visibility) still live in
 * quantum-lab.js and can be surfaced as "expand details" views later.
 */

import { K_B } from '../../constants.js';

// ── Shared helpers ────────────────────────────────────────────────

function particleCount(bridge) {
    const d = bridge.getParticleData?.();
    return d ? (d.positions?.length || 0) / 3 : 0;
}

function radialMean(bridge) {
    const d = bridge.getParticleData?.();
    if (!d?.positions) return 0;
    const N = bridge.latticeSize, mid = N / 2;
    const count = d.positions.length / 3;
    if (count === 0) return 0;
    let sum = 0;
    for (let i = 0; i < count; i++) {
        const px = d.positions[3 * i]     - mid;
        const py = d.positions[3 * i + 1] - mid;
        const pz = d.positions[3 * i + 2] - mid;
        sum += Math.sqrt(px * px + py * py + pz * pz);
    }
    return sum / count;
}

function fluxAtCenter(bridge) {
    const N = bridge.latticeSize, mid = N / 2;
    const v = bridge.inspectVoxel?.(mid, mid, mid);
    if (!v) return 0;
    return Math.sqrt((v.jx || 0) ** 2 + (v.jy || 0) ** 2 + (v.jz || 0) ** 2);
}

// ── Experiments ───────────────────────────────────────────────────

export const QUANTUM_EXPERIMENTS = [
    {
        id: 'quantum-born-rule',
        name: 'Born rule',
        category: 'quantum',
        epistemicTag: 'EMERGENT',
        description: 'Mean radial spread should stabilise near the Gaussian-envelope σ ≈ 4.',
        scenarioId: 'quantum-born-rule',
        overlays: ['toggle-psi-squared'],
        defaultTrials: 40,
        defaultTicksPerTrial: 80,
        resetFn: (bridge) => { /* scenario re-setup handled by runner */ },
        measureFn: (bridge) => radialMean(bridge),
        theoryFn: () => ({ value: 4.0, units: 'lattice units' }),
        tolerance: { relative: 0.25 },
        formatter: (v) => v.toFixed(2) + ' lu',
    },

    {
        id: 'quantum-tunnel',
        name: 'Tunneling transmission',
        category: 'quantum',
        epistemicTag: 'EMERGENT',
        description: 'Fraction of particles observed on the far side of the barrier.',
        scenarioId: 'quantum-tunnel',
        overlays: ['toggle-psi-squared'],
        defaultTrials: 30,
        defaultTicksPerTrial: 150,
        resetFn: () => {},
        measureFn: (bridge) => {
            const d = bridge.getParticleData?.();
            if (!d?.positions) return 0;
            const N = bridge.latticeSize, mid = N / 2;
            const count = d.positions.length / 3;
            if (count === 0) return 0;
            let past = 0;
            for (let i = 0; i < count; i++) {
                if (d.positions[3 * i] > mid + 4) past++;  // past barrier on +x side
            }
            return past / count;
        },
        theoryFn: () => ({ value: 0.05, units: '' }),
        tolerance: { relative: 0.8 },  // wide — WKB approximation, scenario-dependent
        formatter: (v) => (v * 100).toFixed(1) + '%',
    },

    {
        id: 'quantum-entangle',
        name: 'Singlet correlation',
        category: 'quantum',
        epistemicTag: 'SELECTION',
        description: 'Sign-correlation between +1 and −1 particles separated by < 6 lu.',
        scenarioId: 'quantum-entangle',
        overlays: ['toggle-chirality', 'toggle-phase'],
        defaultTrials: 50,
        defaultTicksPerTrial: 100,
        resetFn: () => {},
        measureFn: (bridge) => {
            const d = bridge.getParticleData?.();
            if (!d?.positions || !d.states) return 0;
            const count = d.positions.length / 3;
            if (count < 2) return 0;
            let paired = 0, antialigned = 0;
            for (let i = 0; i < count; i++) {
                for (let j = i + 1; j < count; j++) {
                    const dx = d.positions[3 * i]     - d.positions[3 * j];
                    const dy = d.positions[3 * i + 1] - d.positions[3 * j + 1];
                    const dz = d.positions[3 * i + 2] - d.positions[3 * j + 2];
                    if (dx * dx + dy * dy + dz * dz > 36) continue;
                    paired++;
                    if ((d.states[i] || 0) * (d.states[j] || 0) < 0) antialigned++;
                }
            }
            return paired > 0 ? antialigned / paired : 0;
        },
        theoryFn: () => ({ value: 0.85, units: '' }),  // high anti-correlation expected
        tolerance: { relative: 0.15 },
        formatter: (v) => v.toFixed(3),
    },

    {
        id: 'quantum-zeno',
        name: 'Zeno decay stability',
        category: 'quantum',
        epistemicTag: 'EMERGENT',
        description: 'Central flux retention under short tick windows (quantum Zeno effect).',
        scenarioId: 'quantum-zeno',
        overlays: ['toggle-psi-squared'],
        defaultTrials: 20,
        defaultTicksPerTrial: 60,
        resetFn: () => {},
        measureFn: (bridge) => fluxAtCenter(bridge),
        theoryFn: () => ({ value: K_B * 0.6, units: 'flux' }),
        tolerance: { relative: 0.35 },
        formatter: (v) => v.toFixed(3),
    },

    {
        id: 'quantum-double-slit',
        name: 'Double-slit interference',
        category: 'quantum',
        epistemicTag: 'EMERGENT',
        description: 'Particle-count asymmetry on the detector screen — fringes if > 0.',
        scenarioId: 'quantum-double-slit',
        overlays: ['toggle-psi-squared', 'toggle-phase'],
        defaultTrials: 30,
        defaultTicksPerTrial: 150,
        resetFn: () => {},
        measureFn: (bridge) => {
            // Scan a thin Z-slice at the detector plane (x = N - 4) and count
            // flux concentration at fringe vs trough columns.
            const N = bridge.latticeSize;
            const xDetector = N - 4;
            let peak = 0, trough = 0;
            for (let z = 2; z < N - 2; z++) {
                const v = bridge.inspectVoxel?.(xDetector, N / 2, z);
                if (!v) continue;
                const mag = Math.sqrt((v.jx || 0) ** 2 + (v.jy || 0) ** 2 + (v.jz || 0) ** 2);
                // Every other z-column is a fringe peak in a canonical double slit
                if (z % 4 < 2) peak += mag; else trough += mag;
            }
            if (peak + trough < 1e-9) return 0;
            return (peak - trough) / (peak + trough);
        },
        theoryFn: () => ({ value: 0.5, units: 'visibility' }),
        tolerance: { relative: 0.6 },  // loose — depends on scenario geometry
        formatter: (v) => v.toFixed(3),
    },

    {
        id: 'quantum-well',
        name: 'Energy well confinement',
        category: 'quantum',
        epistemicTag: 'EMERGENT',
        description: 'Fraction of flux that stays bound in the central well over N ticks.',
        scenarioId: 'quantum-well',
        overlays: ['toggle-psi-squared'],
        defaultTrials: 25,
        defaultTicksPerTrial: 120,
        resetFn: () => {},
        measureFn: (bridge) => {
            const N = bridge.latticeSize, mid = N / 2;
            let inside = 0, outside = 0;
            const R2_IN = 25;  // inside-well radius squared
            for (let x = 0; x < N; x++) {
                for (let y = 0; y < N; y++) {
                    for (let z = 0; z < N; z++) {
                        const v = bridge.inspectVoxel?.(x, y, z);
                        if (!v) continue;
                        const mag2 = (v.jx || 0) ** 2 + (v.jy || 0) ** 2 + (v.jz || 0) ** 2;
                        const dx = x - mid, dy = y - mid, dz = z - mid;
                        if (dx * dx + dy * dy + dz * dz < R2_IN) inside += mag2;
                        else outside += mag2;
                    }
                }
            }
            const total = inside + outside;
            return total > 0 ? inside / total : 0;
        },
        theoryFn: () => ({ value: 0.8, units: 'fraction bound' }),
        tolerance: { relative: 0.25 },
        formatter: (v) => (v * 100).toFixed(1) + '%',
    },

    {
        id: 'quantum-aharonov-bohm',
        name: 'Aharonov-Bohm phase',
        category: 'quantum',
        epistemicTag: 'SELECTION',
        description: 'Flux imbalance between the two ring arms encodes the enclosed-flux phase.',
        scenarioId: 'quantum-aharonov-bohm',
        overlays: ['toggle-phase'],
        defaultTrials: 30,
        defaultTicksPerTrial: 100,
        resetFn: () => {},
        measureFn: (bridge) => {
            const N = bridge.latticeSize, mid = N / 2;
            let upper = 0, lower = 0;
            // Sample along the y-axis at two Z offsets representing the two arms
            for (let x = mid - 8; x < mid + 8; x++) {
                const vu = bridge.inspectVoxel?.(x, mid, mid + 4);
                const vl = bridge.inspectVoxel?.(x, mid, mid - 4);
                if (vu) upper += Math.sqrt((vu.jx || 0) ** 2 + (vu.jy || 0) ** 2 + (vu.jz || 0) ** 2);
                if (vl) lower += Math.sqrt((vl.jx || 0) ** 2 + (vl.jy || 0) ** 2 + (vl.jz || 0) ** 2);
            }
            if (upper + lower < 1e-9) return 0;
            return (upper - lower) / (upper + lower);
        },
        theoryFn: () => ({ value: 0.3, units: 'asymmetry' }),
        tolerance: { relative: 0.5 },
        formatter: (v) => v.toFixed(3),
    },

    {
        id: 'quantum-casimir',
        name: 'Casimir pressure',
        category: 'quantum',
        epistemicTag: 'EMERGENT',
        description: 'Mean flux magnitude between two reflective plates.',
        scenarioId: 'quantum-casimir',
        overlays: ['toggle-light'],
        defaultTrials: 20,
        defaultTicksPerTrial: 200,
        resetFn: () => {},
        measureFn: (bridge) => {
            const N = bridge.latticeSize, mid = N / 2;
            // Scan the mid-cube between plates (assumed at x = mid ± 4)
            let sum = 0, n = 0;
            for (let x = mid - 3; x <= mid + 3; x++) {
                for (let y = mid - 3; y <= mid + 3; y++) {
                    for (let z = mid - 3; z <= mid + 3; z++) {
                        const v = bridge.inspectVoxel?.(x, y, z);
                        if (!v) continue;
                        sum += Math.sqrt((v.jx || 0) ** 2 + (v.jy || 0) ** 2 + (v.jz || 0) ** 2);
                        n++;
                    }
                }
            }
            return n > 0 ? sum / n : 0;
        },
        theoryFn: () => ({ value: 0.05, units: 'flux' }),
        tolerance: { relative: 0.6 },
        formatter: (v) => v.toFixed(4),
    },
];

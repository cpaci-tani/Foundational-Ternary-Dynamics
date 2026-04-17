/**
 * Verification Lab — Conservation category.
 *
 * These are [THEOREM] experiments. A lattice that conserves energy / momentum /
 * charge must see the drift stay within numerical tolerance (≤ 1-2%) over many
 * ticks. Violation implies a bug in the engine's update rules.
 *
 * Per-trial measurement = |final − initial| / |initial|  (relative drift).
 */

const FLUX_PULSE = 'flux-pulse';  // a canonical simple scenario
const SMALL_PAIR = 'electron-positron-pair';

function runNTicks(bridge, N) {
    if (bridge.run) bridge.run(N);
    else if (bridge.tick) for (let t = 0; t < N; t++) bridge.tick();
}

function captureDiagnostics(bridge) {
    const d = bridge.getDiagnostics?.();
    if (!d) return null;
    return {
        energy: d.totalEnergy ?? d.energy ?? 0,
        flux:   d.totalFlux ?? 0,
        manifested: d.manifestedCount ?? 0,
        charge: (d.chargePositive ?? 0) - (d.chargeNegative ?? 0),
    };
}

export const CONSERVATION_EXPERIMENTS = [
    {
        id: 'conservation-energy',
        name: 'Energy conservation',
        category: 'conservation',
        epistemicTag: 'THEOREM',
        description: 'Total energy should drift ≤ 1% over 500 ticks of undisturbed evolution.',
        scenarioId: FLUX_PULSE,
        overlays: [],
        defaultTrials: 5,
        defaultTicksPerTrial: 500,
        // The runner normally: setup → ticks → measure. We abuse resetFn to
        // capture the pre-run value so measureFn can compute the drift.
        resetFn: (bridge) => {
            bridge._verifStartEnergy = captureDiagnostics(bridge)?.energy ?? 0;
        },
        measureFn: (bridge) => {
            const start = bridge._verifStartEnergy || 0;
            const end   = captureDiagnostics(bridge)?.energy ?? 0;
            if (Math.abs(start) < 1e-9) return 0;
            return Math.abs(end - start) / Math.abs(start);
        },
        theoryFn: () => ({ value: 0, units: 'relative drift' }),
        tolerance: { absolute: 0.01 },
        formatter: (v) => (v * 100).toFixed(3) + '%',
    },

    {
        id: 'conservation-charge',
        name: 'Net charge conservation',
        category: 'conservation',
        epistemicTag: 'THEOREM',
        description: 'Net charge must not change during dynamics (strict zero drift).',
        scenarioId: SMALL_PAIR,
        overlays: [],
        defaultTrials: 5,
        defaultTicksPerTrial: 500,
        resetFn: (bridge) => {
            bridge._verifStartCharge = captureDiagnostics(bridge)?.charge ?? 0;
        },
        measureFn: (bridge) => {
            const start = bridge._verifStartCharge || 0;
            const end   = captureDiagnostics(bridge)?.charge ?? 0;
            return Math.abs(end - start);
        },
        theoryFn: () => ({ value: 0, units: 'q' }),
        tolerance: { absolute: 0 },
        formatter: (v) => v.toFixed(4) + ' q',
    },

    {
        id: 'conservation-flux',
        name: 'Flux magnitude stability',
        category: 'conservation',
        epistemicTag: 'SELECTION',
        description: 'Closed system: Σ|J| drift bounded. Relative drift over 500 ticks.',
        scenarioId: FLUX_PULSE,
        overlays: ['toggle-flux-volume'],
        defaultTrials: 5,
        defaultTicksPerTrial: 500,
        resetFn: (bridge) => {
            bridge._verifStartFlux = captureDiagnostics(bridge)?.flux ?? 0;
        },
        measureFn: (bridge) => {
            const start = bridge._verifStartFlux || 0;
            const end   = captureDiagnostics(bridge)?.flux ?? 0;
            if (Math.abs(start) < 1e-9) return 0;
            return Math.abs(end - start) / Math.abs(start);
        },
        theoryFn: () => ({ value: 0, units: 'relative drift' }),
        tolerance: { absolute: 0.05 },   // dissipation is allowed — softer tolerance
        formatter: (v) => (v * 100).toFixed(2) + '%',
    },

    {
        id: 'conservation-gauss',
        name: 'Gauss constraint (∥∇·J − s∥)',
        category: 'conservation',
        epistemicTag: 'THEOREM',
        description: '∥div(J) − ρ∥ should stay near zero under Gauss projection.',
        scenarioId: FLUX_PULSE,
        overlays: ['toggle-div-field'],
        defaultTrials: 5,
        defaultTicksPerTrial: 200,
        resetFn: () => {},
        measureFn: (bridge) => {
            const audit = bridge.getEnergyAudit?.();
            return audit?.gaussViolation ?? 0;
        },
        theoryFn: () => ({ value: 0, units: '' }),
        tolerance: { absolute: 0.05 },
        formatter: (v) => v.toFixed(5),
    },
];

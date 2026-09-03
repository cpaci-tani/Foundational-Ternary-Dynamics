/**
 * Deterministic-seed Scale 2 nuclear-reaction channels.
 *
 * [PARAMETRIC] These are effective reaction fixtures, not substrate-native
 * derivations of nuclear cross sections or tunnelling. Exact isotope pairs
 * become eligible only through their live swept trajectories. An
 * energy-dependent [PARAMETRIC] hazard then samples the reaction from a seeded
 * stream; no scenario or target identity can force an event. Product identity
 * and mass-channel Q are taken from evaluated atomic masses. Product momenta
 * are constructed so proton number, neutron number, momentum, and the
 * explicitly named kinetic-energy injection close numerically on every event.
 * U-235 keeps its selected-channel mass Q separate from the standard average
 * recoverable fission-energy budget; those are not interchangeable quantities.
 *
 * Atomic masses: NIST Atomic Weights and Isotopic Compositions (AME data).
 * U-235 channel: IAEA reference reaction
 *   235U + n -> 141Ba + 92Kr + 3n.
 */

import { M_P_PHYS, NEUTRON_PROTON_MASS_RATIO } from '../../constants.js';

export const ATOMIC_MASS_UNIT_MEV = 931.49410242;
export const FREE_NEUTRON_MASS_U = 1.00866491595;
export const MEV_TO_JOULE = 1.602176634e-13;

const MASS_U = Object.freeze({
    D2: 2.01410177812,
    T3: 3.0160492779,
    HE4: 4.00260325413,
    U235: 235.0439301,
    BA141: 140.9144033,
    KR92: 91.9261731,
});

const qFromAtomicMasses = (inputMassU, outputMassU) =>
    (inputMassU - outputMassU) * ATOMIC_MASS_UNIT_MEV;

export const NUCLEAR_REACTION_CHANNELS = Object.freeze({
    dt_fusion: Object.freeze({
        id: 'dt_fusion',
        label: '²H + ³H → ⁴He + n',
        kind: 'fusion',
        captureRadius: 0.8,
        referenceEnergyMeV: 0.064,
        collisionHazard: 1.5,
        reactants: Object.freeze([
            Object.freeze({ Z: 1, N: 1, label: '²H' }),
            Object.freeze({ Z: 1, N: 2, label: '³H' }),
        ]),
        products: Object.freeze([
            Object.freeze({ Z: 2, N: 2, label: '⁴He' }),
            Object.freeze({ Z: 0, N: 1, label: 'n' }),
        ]),
        qMeV: qFromAtomicMasses(
            MASS_U.D2 + MASS_U.T3,
            MASS_U.HE4 + FREE_NEUTRON_MASS_U,
        ),
        // Representative center-of-mass fuel energy for a hot D-T plasma.
        // This controls physical velocity, not how quickly the UI advances.
        incidentEnergyMeV: 0.020,
        energyBudget: Object.freeze({
            kineticMeV: qFromAtomicMasses(
                MASS_U.D2 + MASS_U.T3,
                MASS_U.HE4 + FREE_NEUTRON_MASS_U,
            ),
            chargedMeV: 3.52,
            neutronMeV: qFromAtomicMasses(
                MASS_U.D2 + MASS_U.T3,
                MASS_U.HE4 + FREE_NEUTRON_MASS_U,
            ) - 3.52,
            promptGammaMeV: 0,
            delayedHeatMeV: 0,
            totalRecoverableMeV: qFromAtomicMasses(
                MASS_U.D2 + MASS_U.T3,
                MASS_U.HE4 + FREE_NEUTRON_MASS_U,
            ),
        }),
        source: 'NIST evaluated atomic masses; IAEA D-T product-energy partition',
        sourceUrl: 'https://www-pub.iaea.org/MTCD/Publications/PDF/PUB1945_web.pdf',
    }),
    u235_fission: Object.freeze({
        id: 'u235_fission',
        label: '²³⁵U + n → ¹⁴¹Ba + ⁹²Kr + 3n',
        kind: 'fission',
        captureRadius: 1.0,
        referenceEnergyMeV: 2.53e-8,
        collisionHazard: 3.0,
        reactants: Object.freeze([
            Object.freeze({ Z: 92, N: 143, label: '²³⁵U' }),
            Object.freeze({ Z: 0, N: 1, label: 'n' }),
        ]),
        products: Object.freeze([
            Object.freeze({ Z: 56, N: 85, label: '¹⁴¹Ba' }),
            Object.freeze({ Z: 36, N: 56, label: '⁹²Kr' }),
            Object.freeze({ Z: 0, N: 1, label: 'n' }),
            Object.freeze({ Z: 0, N: 1, label: 'n' }),
            Object.freeze({ Z: 0, N: 1, label: 'n' }),
        ]),
        qMeV: qFromAtomicMasses(
            MASS_U.U235 + FREE_NEUTRON_MASS_U,
            MASS_U.BA141 + MASS_U.KR92 + 3 * FREE_NEUTRON_MASS_U,
        ),
        // 0.0253 eV reference thermal neutron at 2200 m/s.
        incidentEnergyMeV: 2.53e-8,
        // [PARAMETRIC] Rounded IAEA one-group U-235 energy distribution.
        // The selected Ba/Kr mass channel has Q=173.280 MeV, while an average
        // thermal fission deposits about 200 MeV after prompt capture-gamma
        // and delayed beta/gamma energy. Neutrino energy is excluded from the
        // recoverable budget. These are distinct, intentionally named ledgers.
        energyBudget: Object.freeze({
            kineticMeV: 173,
            chargedMeV: 168,
            neutronMeV: 5,
            promptGammaMeV: 13,
            delayedHeatMeV: 14,
            totalRecoverableMeV: 200,
        }),
        fragmentKineticFraction: 168 / 173,
        source: 'NIST evaluated atomic masses; IAEA average U-235 fission-energy partition',
        sourceUrl: 'https://gnssn.iaea.org/main/bptc/BPTC%20Module%20Documents/Module01%20Nuclear%20physics%20and%20reactor%20theory.pdf',
    }),
});

export function getNuclearReactionChannel(id) {
    return NUCLEAR_REACTION_CHANNELS[id] || null;
}

export function isotopeMassSim(Z, N) {
    return Z + N * NEUTRON_PROTON_MASS_RATIO;
}

/**
 * Physical incident velocities in c-normalized simulation coordinates.
 * Scenario presets and live injectors choose positions independently; this
 * helper does not accelerate a trajectory to make playback fire sooner.
 */
export function incidentVelocities(channelId) {
    const channel = getNuclearReactionChannel(channelId);
    if (!channel) return null;
    const [a, b] = channel.reactants;
    const m0 = isotopeMassSim(a.Z, a.N);
    const m1 = isotopeMassSim(b.Z, b.N);
    const kineticSim = channel.incidentEnergyMeV / M_P_PHYS;
    if (channel.kind === 'fusion') {
        const p = Math.sqrt(Math.max(0, 2 * kineticSim / (1 / m0 + 1 / m1)));
        return Object.freeze([
            Object.freeze({ vx: p / m0, vy: 0, vz: 0 }),
            Object.freeze({ vx: -p / m1, vy: 0, vz: 0 }),
        ]);
    }
    const projectileSpeed = Math.sqrt(Math.max(0, 2 * kineticSim / m1));
    return Object.freeze([
        Object.freeze({ vx: 0, vy: 0, vz: 0 }),
        Object.freeze({ vx: projectileSpeed, vy: 0, vz: 0 }),
    ]);
}

function counts(rows) {
    return rows.reduce((sum, row) => ({
        protons: sum.protons + row.Z,
        neutrons: sum.neutrons + row.N,
    }), { protons: 0, neutrons: 0 });
}

function norm(v) {
    const m = Math.hypot(v.x, v.y, v.z);
    return m > 1e-15
        ? { x: v.x / m, y: v.y / m, z: v.z / m }
        : { x: 1, y: 0, z: 0 };
}

function cross(a, b) {
    return {
        x: a.y * b.z - a.z * b.y,
        y: a.z * b.x - a.x * b.z,
        z: a.x * b.y - a.y * b.x,
    };
}

function addScaled(a, b, scale) {
    return { x: a.x + b.x * scale, y: a.y + b.y * scale, z: a.z + b.z * scale };
}

const clamp01 = value => Math.max(0, Math.min(1, value));

/**
 * Relative center-of-mass kinetic energy for a live pair.
 * Atom masses and velocities are in the AtomEngine's proton-mass/c units.
 */
export function relativeCollisionEnergyMeV(first, second) {
    const relativeSpeed = Math.hypot(
        second.vx - first.vx,
        second.vy - first.vy,
        second.vz - first.vz,
    );
    const reducedMass = first.mass * second.mass / Math.max(first.mass + second.mass, 1e-30);
    return 0.5 * reducedMass * relativeSpeed * relativeSpeed * M_P_PHYS;
}

/**
 * [PARAMETRIC] Unitless response curves used by the finite visual laboratory.
 * They retain the two important qualitative dependencies without pretending
 * that browser length units are barns: D-T reactivity has a broad hot-plasma
 * peak, while U-235 neutron capture is strongest near the thermal reference
 * and retains a non-zero fast-neutron floor. `scale` is an explicit laboratory
 * coefficient, not a derived cross section.
 */
export function nuclearReactionProbability(channelId, energyMeV, exposure = 1, scale = 1) {
    const channel = getNuclearReactionChannel(channelId);
    if (!channel || scale <= 0 || exposure <= 0) return 0;
    const energy = Math.max(Number(energyMeV) || 0, 1e-15);
    let response = 0;
    if (channel.kind === 'fusion') {
        const logRatio = Math.log(energy / channel.referenceEnergyMeV);
        response = Math.exp(-0.5 * (logRatio / 1.15) ** 2);
    } else {
        const inverseVelocity = Math.sqrt(channel.referenceEnergyMeV / energy);
        response = Math.max(0.025, Math.min(8, inverseVelocity));
    }
    return clamp01(1 - Math.exp(-channel.collisionHazard * scale * response * clamp01(exposure)));
}

function sweptPairGeometry(first, second, previousById) {
    const current = {
        x: second.x - first.x,
        y: second.y - first.y,
        z: second.z - first.z,
    };
    const priorFirst = previousById?.get(first.id);
    const priorSecond = previousById?.get(second.id);
    if (!priorFirst || !priorSecond) {
        return { distance: Math.hypot(current.x, current.y, current.z), axis: norm(current), sweepT: 1 };
    }
    const prior = {
        x: priorSecond.x - priorFirst.x,
        y: priorSecond.y - priorFirst.y,
        z: priorSecond.z - priorFirst.z,
    };
    const delta = { x: current.x - prior.x, y: current.y - prior.y, z: current.z - prior.z };
    const denom = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
    const sweepT = denom > 1e-30
        ? clamp01(-(prior.x * delta.x + prior.y * delta.y + prior.z * delta.z) / denom)
        : 1;
    const closest = addScaled(prior, delta, sweepT);
    return { distance: Math.hypot(closest.x, closest.y, closest.z), axis: norm(closest), sweepT };
}

function matchReactants(atoms, channel, options = {}) {
    let best = null;
    const radius = channel.captureRadius * Math.max(0.1, Number(options.collisionRadiusScale) || 1);
    const previousById = options.previousById instanceof Map ? options.previousById : null;
    for (const first of atoms) {
        if (first.Z !== channel.reactants[0].Z || first.N !== channel.reactants[0].N) continue;
        for (const second of atoms) {
            if (second.id === first.id || second.Z !== channel.reactants[1].Z || second.N !== channel.reactants[1].N) continue;
            const geometry = sweptPairGeometry(first, second, previousById);
            if (geometry.distance > radius) continue;
            const dx = second.x - first.x, dy = second.y - first.y, dz = second.z - first.z;
            const dvx = second.vx - first.vx, dvy = second.vy - first.vy, dvz = second.vz - first.vz;
            const approaching = dx * dvx + dy * dvy + dz * dvz <= 0;
            // A swept crossing stays eligible even when the pair has moved
            // apart by the end of this tick; a merely overlapping pair must be
            // approaching so products cannot immediately recapture one another.
            if (!approaching && geometry.sweepT >= 1) continue;
            const energyMeV = relativeCollisionEnergyMeV(first, second);
            const penetration = clamp01(1 - geometry.distance / radius);
            const exposure = Math.max(0.02, penetration);
            const probability = nuclearReactionProbability(
                channel.id,
                energyMeV,
                exposure,
                Number.isFinite(Number(options.reactivityScale)) ? Number(options.reactivityScale) : 1,
            );
            const sample = typeof options.sampleForPair === 'function'
                ? clamp01(options.sampleForPair(first.id, second.id))
                : 0;
            if (sample >= probability) continue;
            const candidate = {
                first,
                second,
                axis: geometry.axis,
                distance: geometry.distance,
                sweepT: geometry.sweepT,
                collisionEnergyMeV: energyMeV,
                reactionProbability: probability,
                randomSample: sample,
            };
            if (!best || candidate.sweepT < best.sweepT ||
                (candidate.sweepT === best.sweepT && candidate.distance < best.distance) ||
                (candidate.sweepT === best.sweepT && candidate.distance === best.distance &&
                    `${candidate.first.id}:${candidate.second.id}` < `${best.first.id}:${best.second.id}`)) best = candidate;
        }
    }
    return best;
}

function momentumAndEnergy(atoms) {
    let mass = 0, px = 0, py = 0, pz = 0, ke = 0;
    for (const atom of atoms) {
        mass += atom.mass;
        px += atom.mass * atom.vx;
        py += atom.mass * atom.vy;
        pz += atom.mass * atom.vz;
        ke += 0.5 * atom.mass * (atom.vx ** 2 + atom.vy ** 2 + atom.vz ** 2);
    }
    return { mass, px, py, pz, ke };
}

function twoBodyMomenta(products, internalKE, axis) {
    const m0 = isotopeMassSim(products[0].Z, products[0].N);
    const m1 = isotopeMassSim(products[1].Z, products[1].N);
    const p = Math.sqrt(Math.max(0, 2 * internalKE / (1 / m0 + 1 / m1)));
    return [
        { x: axis.x * p, y: axis.y * p, z: axis.z * p },
        { x: -axis.x * p, y: -axis.y * p, z: -axis.z * p },
    ];
}

function fissionMomenta(products, internalKE, axis, fragmentFraction) {
    const m0 = isotopeMassSim(products[0].Z, products[0].N);
    const m1 = isotopeMassSim(products[1].Z, products[1].N);
    const fragmentKE = internalKE * fragmentFraction;
    const neutronKE = internalKE - fragmentKE;
    const pFrag = Math.sqrt(Math.max(0, 2 * fragmentKE / (1 / m0 + 1 / m1)));

    const reference = Math.abs(axis.z) < 0.8 ? { x: 0, y: 0, z: 1 } : { x: 0, y: 1, z: 0 };
    const transverseA = norm(cross(axis, reference));
    const transverseB = norm(cross(axis, transverseA));
    const neutronMass = isotopeMassSim(0, 1);
    const pNeutron = Math.sqrt(Math.max(0, 2 * neutronMass * neutronKE / 3));
    const momenta = [
        { x: axis.x * pFrag, y: axis.y * pFrag, z: axis.z * pFrag },
        { x: -axis.x * pFrag, y: -axis.y * pFrag, z: -axis.z * pFrag },
    ];
    for (let i = 0; i < 3; i++) {
        const angle = 2 * Math.PI * i / 3;
        momenta.push({
            x: pNeutron * (Math.cos(angle) * transverseA.x + Math.sin(angle) * transverseB.x),
            y: pNeutron * (Math.cos(angle) * transverseA.y + Math.sin(angle) * transverseB.y),
            z: pNeutron * (Math.cos(angle) * transverseA.z + Math.sin(angle) * transverseB.z),
        });
    }
    return momenta;
}

/**
 * Return a closed reaction event, or null until the prepared reactants meet.
 * The caller owns atom replacement so this pure routine is independently
 * testable and cannot partially mutate an engine state.
 */
export function evaluateNuclearReaction(channelId, atoms, tick, options = {}) {
    const channel = getNuclearReactionChannel(channelId);
    if (!channel) return null;
    const match = matchReactants(atoms, channel, options);
    if (!match) return null;

    const inputs = [match.first, match.second];
    const before = momentumAndEnergy(inputs);
    const comVelocity = {
        x: before.px / before.mass,
        y: before.py / before.mass,
        z: before.pz / before.mass,
    };
    const comKE = 0.5 * before.mass *
        (comVelocity.x ** 2 + comVelocity.y ** 2 + comVelocity.z ** 2);
    const kineticReleaseMeV = channel.energyBudget.kineticMeV;
    const kineticReleaseSim = kineticReleaseMeV / M_P_PHYS;
    const internalKE = Math.max(0, before.ke - comKE) + kineticReleaseSim;
    const momenta = channel.kind === 'fusion'
        ? twoBodyMomenta(channel.products, internalKE, match.axis)
        : fissionMomenta(channel.products, internalKE, match.axis, channel.fragmentKineticFraction);

    const center = {
        x: (match.first.mass * match.first.x + match.second.mass * match.second.x) / before.mass,
        y: (match.first.mass * match.first.y + match.second.mass * match.second.y) / before.mass,
        z: (match.first.mass * match.first.z + match.second.mass * match.second.z) / before.mass,
    };
    const products = channel.products.map((spec, index) => {
        const mass = isotopeMassSim(spec.Z, spec.N);
        const direction = norm(momenta[index]);
        const offset = index < 2 ? 0.55 : 0.35;
        return {
            ...spec,
            x: center.x + direction.x * offset,
            y: center.y + direction.y * offset,
            z: center.z + direction.z * offset,
            vx: comVelocity.x + momenta[index].x / mass,
            vy: comVelocity.y + momenta[index].y / mass,
            vz: comVelocity.z + momenta[index].z / mass,
            mass,
        };
    });

    const after = momentumAndEnergy(products);
    const inputCounts = counts(channel.reactants);
    const outputCounts = counts(channel.products);
    return {
        channel: channel.id,
        label: channel.label,
        kind: channel.kind,
        source: channel.source,
        tick,
        center,
        axis: { ...match.axis },
        inputIds: inputs.map(atom => atom.id),
        products,
        qMeV: channel.qMeV,
        qSim: channel.qMeV / M_P_PHYS,
        kineticReleaseSim,
        incidentEnergyMeV: channel.incidentEnergyMeV,
        collisionEnergyMeV: match.collisionEnergyMeV,
        collisionDistance: match.distance,
        reactionProbability: match.reactionProbability,
        randomSample: match.randomSample,
        kineticReleaseMeV,
        chargedKineticMeV: channel.energyBudget.chargedMeV,
        neutronKineticMeV: channel.energyBudget.neutronMeV,
        promptGammaMeV: channel.energyBudget.promptGammaMeV,
        delayedHeatMeV: channel.energyBudget.delayedHeatMeV,
        totalReleasedMeV: channel.energyBudget.totalRecoverableMeV,
        totalReleasedJoule: channel.energyBudget.totalRecoverableMeV * MEV_TO_JOULE,
        protonResidual: outputCounts.protons - inputCounts.protons,
        // Nuclear electric charge is carried by proton number in these
        // neutral-atom fixtures; the electron count also balances for both
        // channels, so this is the complete charge ledger residual.
        chargeResidual: outputCounts.protons - inputCounts.protons,
        neutronResidual: outputCounts.neutrons - inputCounts.neutrons,
        momentumBefore: { x: before.px, y: before.py, z: before.pz },
        momentumAfter: { x: after.px, y: after.py, z: after.pz },
        momentumResidual: Math.hypot(after.px - before.px, after.py - before.py, after.pz - before.pz),
        kineticBeforeSim: before.ke,
        kineticAfterSim: after.ke,
        energyResidualMeV: (after.ke - before.ke) * M_P_PHYS - kineticReleaseMeV,
        totalLedgerResidualMeV: kineticReleaseMeV + channel.energyBudget.promptGammaMeV +
            channel.energyBudget.delayedHeatMeV - channel.energyBudget.totalRecoverableMeV,
    };
}

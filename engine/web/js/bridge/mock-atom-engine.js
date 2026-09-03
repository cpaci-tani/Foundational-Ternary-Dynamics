/**
 * Scale-2 Atom Engine (AE) — MockBridge side only.
 *
 * Atomic molecular dynamics with ionic (Coulomb), vdW (LJ 12-6), covalent
 * bond spring, H-bond, dipole-dipole, and VSEPR angle-strain forces.
 * Includes Berendsen thermostat, electronegativity-sensitive auto-bonding,
 * 1-2 / 1-3 exclusion, and a Velocity Verlet integrator.
 *
 * Extracted from `bridge-init.js` during the bridge modularization pass and
 * subsequently hardened as the production browser AtomEngine. The live
 * `state` reference owns all mutable engine state.
 *
 * STATE CONTRACT — `state` must be the MockBridge instance (not a
 * destructured copy), exposing:
 *   Read:
 *     _boundaryShape: string
 *     _reflectIntoBoundary(p, cx, cy, cz, R): void
 *   Read + write (created/managed here):
 *     _ae                : { atoms, bonds, nextId, tick, dt, soft,
 *                            damping, bonding, ionic, vdw, bonds_force,
 *                            speed_limit, h_bonds, angle_strain,
 *                            dipole_dipole, thermostat, thermostat_temp,
 *                            electronegativity }
 *     _aeBondSet         : Set<number>            (bond-pair lookup, rebuilt per tick)
 *     _aeIdToIdx         : Map<number, number>    (atom.id -> array index)
 *     _aeNeighborSets    : Array<Set<number>>     (per-atom bonded partner IDs)
 *
 * Bond-pair numeric key: `lo * 100000 + hi`. Safe as long as atom IDs stay
 * below 100000 (typical simulations have <1000 atoms).
 */

import { M_P_PHYS, NEUTRON_PROTON_MASS_RATIO } from '../constants.js';
import {
    AE_K_COULOMB, AE_K_BOND, AE_SPEED_MAX,
    AE_H_BOND_EPS, AE_K_ANGLE, AE_THERMOSTAT_TAU,
    computeAtomicProps,
} from '../atomic-props.js';
import { cpkColor, defaultNeutronCount as elemNeutrons, maxBonds as elemMaxBonds } from '../elements.js';
import { debugLog } from '../core/log.js';
import {
    evaluateNuclearReaction,
    getNuclearReactionChannel,
    incidentVelocities,
    MEV_TO_JOULE,
} from '../scales/scale2/nuclear-reactions.js';
import {
    valenceElectrons as _valenceElectrons,
    covalentValence as _covalentValence,
    AROMATIC_ORDER,
    MAX_BOND_ORDER,
} from './mock-atom-valence.js';

const AE_LIMITS = Object.freeze({
    dt: Object.freeze([0.001, 0.5]),
    soft: Object.freeze([0.01, 10.0]),
    thermostat_temp: Object.freeze([1e-6, 1e6]),
});
const AE_FORCE_MAX = 50.0;
const AE_FORCE_COMPONENTS = Object.freeze(['ionic', 'vdw', 'bond', 'hbond', 'angle', 'dipole']);

// Atomic properties (mass in proton units, LJ radius/ε/σ, max bonds,
// electronegativity) come from the canonical atomic-props.js
// implementation. A stale LOCAL shadow copy lived here until 2026-06-10;
// it still carried the pre-Theme-D3 electronegativity bug (log-formula
// for ALL Z>18 — χ(Fe)≈2.48 vs Pauling 1.83) and a hand-rounded 1.001
// neutron/proton ratio. De-duplicated so the engine and the rest of the
// app share one table-first, PDG-ratio implementation.

/**
 * Build the atom-engine provider bound to the given bridge-like state.
 *
 * @param {object} state - MockBridge instance (live reference).
 */
export function createAtomEngine(state) {

    function _aeNewNuclearState(config = {}) {
        const channel = typeof config === 'string' ? config : (config.channel || '');
        const mode = typeof config === 'object' && config.mode ? config.mode : 'single';
        const finiteOr = (value, fallback) => Number.isFinite(Number(value)) ? Number(value) : fallback;
        const eventLimit = mode === 'single' ? 1
            : Math.max(1, Math.floor(Number(config.eventLimit) || Number.MAX_SAFE_INTEGER));
        return {
            channel,
            mode,
            phase: channel ? 'armed' : 'disabled',
            event_limit: eventLimit,
            event_weight: Math.max(1, Number(config.eventWeight) || 1),
            // k_effective is measured from live neutron births and losses. It
            // is intentionally never accepted as a scenario input.
            k_effective: 0,
            reactivity_scale: Math.max(0, Math.min(20, finiteOr(config.reactivityScale, 1))),
            collision_radius_scale: Math.max(0.25, Math.min(4, finiteOr(config.collisionRadiusScale, 1))),
            transport_radius: Math.max(2, Math.min(100, finiteOr(config.transportRadius, 18))),
            boundary_mode: config.boundaryMode === 'reflect' ? 'reflect' : 'leak',
            moderator_strength: Math.max(0, Math.min(1, finiteOr(config.moderatorStrength, 0))),
            absorber_strength: Math.max(0, Math.min(1, finiteOr(config.absorberStrength, 0))),
            source_enabled: !!config.sourceEnabled,
            source_rate: Math.max(0, Math.min(4, finiteOr(config.sourceRate, 0))),
            source_energy_mev: Math.max(1e-12, Math.min(20, finiteOr(config.sourceEnergyMeV, 2.53e-8))),
            source_accumulator: 0,
            particle_limit: Math.max(32, Math.min(2048, Math.floor(finiteOr(config.particleLimit, 512)))),
            source_saturated: false,
            random_seed: (Number(config.seed) >>> 0) || 0x5eed235,
            neutron_containment: Math.max(0, Math.min(1, Number(config.neutronContainment) || 0)),
            gamma_containment: Math.max(0, Math.min(1, Number(config.gammaContainment) || 0)),
            event_count: 0,
            represented_event_count: 0,
            event_tick: -1,
            generation: 0,
            fuel_initial: 0,
            fuel_remaining: 0,
            effects: [],
            history: [],
            leaked_neutrons: 0,
            absorbed_neutrons: 0,
            scattered_neutrons: 0,
            source_neutrons: 0,
            fission_neutron_births: 0,
            neutron_fission_losses: 0,
            released_mev: 0,
            microscopic_released_mev: 0,
            released_joule: 0,
            deposited_mev: 0,
            in_transit_mev: 0,
            escaped_mev: 0,
            kinetic_mev: 0,
            charged_mev: 0,
            neutron_mev: 0,
            prompt_gamma_mev: 0,
            delayed_heat_mev: 0,
            last_event: null,
        };
    }

    function initAE() {
        state._ae = {
            atoms: [], bonds: [], nextId: 0, tick: 0,
            dt: 0.1,       // Larger dt for visible dynamics in sim units
            soft: 0.3,     // Softening in Bohr radii
            damping: false, bonding: true,
            ionic: true,
            vdw: true,
            bonds_force: true,
            speed_limit: true,
            // Phase 3 forces (all off by default)
            h_bonds: false,
            angle_strain: false,
            dipole_dipole: false,
            thermostat: false,
            thermostat_temp: 1.0,
            electronegativity: false,
            force_clamped_last: false,
            force_clamp_scale: 1.0,
            force_clamp_events: 0,
            nuclear: _aeNewNuclearState(),
            last_error: '',
        };
    }

    function resetAE() {
        if (state._ae) {
            state._ae.atoms = [];
            state._ae.bonds = [];
            state._ae.nextId = 0;
            state._ae.tick = 0;
            state._ae.force_clamped_last = false;
            state._ae.force_clamp_scale = 1.0;
            state._ae.force_clamp_events = 0;
            state._ae.nuclear = _aeNewNuclearState();
            state._ae.last_error = '';
        }
    }

    function _aeReject(message) {
        if (state._ae) state._ae.last_error = message;
        return false;
    }

    function _aeSetBounded(name, value) {
        if (!state._ae || !Number.isFinite(value)) return _aeReject(`${name} must be finite`);
        const [lo, hi] = AE_LIMITS[name];
        state._ae[name] = Math.max(lo, Math.min(hi, value));
        state._ae.last_error = '';
        return true;
    }

    function _aeStateIssue() {
        if (!state._ae) return 'atom engine is not initialized';
        const ae = state._ae;
        if (!Number.isFinite(ae.dt) || ae.dt <= 0) return 'time step must be finite and positive';
        if (!Number.isFinite(ae.soft) || ae.soft <= 0) return 'softening must be finite and positive';
        const ids = new Set();
        const byId = new Map();
        for (const a of ae.atoms) {
            if (!Number.isInteger(a.id) || ids.has(a.id)) return 'atom IDs must be unique integers';
            ids.add(a.id); byId.set(a.id, a);
            if (!Number.isInteger(a.Z) || a.Z < 0 || a.Z > 118 || (a.Z === 0 && a.N !== 1))
                return `atom ${a.id} has invalid atomic number/isotope`;
            if (!Number.isInteger(a.N) || a.N < 0) return `atom ${a.id} has invalid neutron count`;
            if (!Number.isFinite(a.mass) || a.mass <= 0) return `atom ${a.id} has invalid mass`;
            for (const key of ['charge', 'q_frac', 'x', 'y', 'z', 'vx', 'vy', 'vz', 'ax', 'ay', 'az']) {
                if (!Number.isFinite(a[key])) return `atom ${a.id} has non-finite ${key}`;
            }
        }
        for (const a of ae.atoms) {
            const partners = new Set();
            for (const b of a.bonds) {
                if (!Number.isInteger(b.partner_id) || b.partner_id === a.id || !byId.has(b.partner_id))
                    return `atom ${a.id} has an invalid bond partner`;
                if (partners.has(b.partner_id)) return `atom ${a.id} has a duplicate bond`;
                partners.add(b.partner_id);
                if (!Number.isFinite(b.r_eq) || b.r_eq <= 0 || !Number.isFinite(b.k_bond) || b.k_bond < 0 ||
                    !Number.isFinite(b.order) || b.order <= 0)
                    return `atom ${a.id} has invalid bond parameters`;
                if (!byId.get(b.partner_id).bonds.some(back => back.partner_id === a.id))
                    return `bond ${a.id}-${b.partner_id} is not reciprocal`;
            }
        }
        const nuclear = ae.nuclear;
        if (nuclear) {
            for (const key of [
                'event_weight', 'k_effective', 'reactivity_scale', 'collision_radius_scale',
                'transport_radius', 'moderator_strength', 'absorber_strength', 'source_rate',
                'source_energy_mev', 'source_accumulator', 'particle_limit', 'neutron_containment', 'gamma_containment',
                'event_count', 'represented_event_count', 'generation', 'fuel_initial',
                'fuel_remaining', 'released_mev', 'microscopic_released_mev',
                'released_joule', 'deposited_mev', 'in_transit_mev', 'escaped_mev',
                'kinetic_mev', 'charged_mev', 'neutron_mev', 'prompt_gamma_mev', 'delayed_heat_mev',
                'leaked_neutrons', 'absorbed_neutrons', 'scattered_neutrons',
                'source_neutrons', 'fission_neutron_births', 'neutron_fission_losses',
            ]) {
                if (!Number.isFinite(nuclear[key]) || nuclear[key] < 0) {
                    return `nuclear ledger has invalid ${key}`;
                }
            }
            if (!['leak', 'reflect'].includes(nuclear.boundary_mode)) {
                return 'nuclear transport has an invalid boundary mode';
            }
        }
        return '';
    }

    function _aeDynamicSnapshot() {
        return {
            atoms: state._ae.atoms.map(a => ({
                ...a,
                bonds: a.bonds.map(bond => ({ ...bond })),
            })),
            nextId: state._ae.nextId,
            nuclear: {
                ...state._ae.nuclear,
                last_event: state._ae.nuclear.last_event
                    ? { ...state._ae.nuclear.last_event }
                    : null,
                effects: state._ae.nuclear.effects.map(item => ({
                    ...item,
                    neutronDirections: item.neutronDirections?.map(direction => ({ ...direction })) || [],
                })),
                history: state._ae.nuclear.history.map(item => ({
                    ...item,
                    neutronDirections: item.neutronDirections?.map(direction => ({ ...direction })) || [],
                })),
            },
        };
    }

    function _aeRestoreDynamic(snapshot) {
        state._ae.atoms = snapshot.atoms.map(a => ({
            ...a,
            bonds: a.bonds.map(bond => ({ ...bond })),
        }));
        state._ae.nextId = snapshot.nextId;
        state._ae.nuclear = snapshot.nuclear;
    }

    function aeAddAtom(Z, x, y, z, vx = 0, vy = 0, vz = 0, charge = 0, N = -1) {
        if (!state._ae) initAE();
        const explicitNeutron = Z === 0 && N === 1;
        if (!Number.isInteger(Z) || (!explicitNeutron && (Z < 1 || Z > 118)) ||
            ![x, y, z, vx, vy, vz, charge].every(Number.isFinite) ||
            !(N === -1 || (Number.isInteger(N) && N >= 0))) {
            _aeReject('aeAddAtom received invalid atomic or finite-state data');
            return -1;
        }
        const neutrons = N >= 0 ? N : elemNeutrons(Z);
        const props = computeAtomicProps(Z, neutrons);
        if (!Number.isFinite(props.mass) || props.mass <= 0) {
            _aeReject(`aeAddAtom could not construct a positive finite mass for Z=${Z}`);
            return -1;
        }
        const id = state._ae.nextId++;
        state._ae.atoms.push({
            id, Z, N: neutrons, charge, mass: props.mass, radius: props.radius,
            vdw_epsilon: props.vdw_epsilon, vdw_sigma: props.vdw_sigma,
            max_bonds: props.max_bonds, bonds: [],
            electronegativity: props.electronegativity,
            valence_electrons: _valenceElectrons(Z),
            alpha_pol: props.alpha_pol, e_ion: props.e_ion, 
            e_aff: props.e_aff, sigma_scatter: props.sigma_scatter,
            z_eff: props.closure_context ? props.closure_context.z_eff : 0,
            dipole_x: 0, dipole_y: 0, dipole_z: 0, q_frac: charge,
            x, y, z, vx, vy, vz, ax: 0, ay: 0, az: 0, locked: false
        });
        return id;
    }

    function aeConfigureNuclearReaction(config = '') {
        if (!state._ae) initAE();
        const channelId = typeof config === 'string' ? config : (config?.channel || '');
        if (channelId && !getNuclearReactionChannel(channelId)) {
            return _aeReject(`unknown nuclear reaction channel: ${channelId}`);
        }
        state._ae.nuclear = _aeNewNuclearState(
            typeof config === 'string' ? { channel: config } : config,
        );
        state._ae.last_error = '';
        return true;
    }

    function _aeNuclearSample(...keys) {
        const nuclear = state._ae.nuclear;
        let x = nuclear.random_seed >>> 0;
        for (let i = 0; i < keys.length; i++) {
            x ^= Math.imul((Number(keys[i]) >>> 0) + i + 1, [0x9e3779b1, 0x85ebca6b, 0xc2b2ae35][i % 3]);
            x >>>= 0;
        }
        x ^= x >>> 16;
        x = Math.imul(x, 0x7feb352d) >>> 0;
        x ^= x >>> 15;
        x = Math.imul(x, 0x846ca68b) >>> 0;
        x ^= x >>> 16;
        return (x >>> 0) / 4294967296;
    }

    function _aeNuclearDirection(...keys) {
        const z = 2 * _aeNuclearSample(...keys, 17) - 1;
        const phi = 2 * Math.PI * _aeNuclearSample(...keys, 31);
        const radial = Math.sqrt(Math.max(0, 1 - z * z));
        return { x: radial * Math.cos(phi), y: radial * Math.sin(phi), z };
    }

    function aeSetNuclearEnvironment(patch = {}) {
        if (!state._ae) initAE();
        const nuclear = state._ae.nuclear;
        if (!nuclear) return false;
        const bounded = (key, sourceKey, lo, hi) => {
            if (!(sourceKey in patch)) return;
            const value = Number(patch[sourceKey]);
            if (Number.isFinite(value)) nuclear[key] = Math.max(lo, Math.min(hi, value));
        };
        bounded('reactivity_scale', 'reactivityScale', 0, 20);
        bounded('collision_radius_scale', 'collisionRadiusScale', 0.25, 4);
        bounded('transport_radius', 'transportRadius', 2, 100);
        bounded('moderator_strength', 'moderatorStrength', 0, 1);
        bounded('absorber_strength', 'absorberStrength', 0, 1);
        bounded('source_rate', 'sourceRate', 0, 4);
        bounded('source_energy_mev', 'sourceEnergyMeV', 1e-12, 20);
        if ('sourceEnabled' in patch) nuclear.source_enabled = !!patch.sourceEnabled;
        if ('boundaryMode' in patch) nuclear.boundary_mode = patch.boundaryMode === 'reflect' ? 'reflect' : 'leak';
        state._ae.last_error = '';
        return true;
    }

    function _aeTagNeutron(id, generation = 0, source = '') {
        const atom = state._ae.atoms.find(item => item.id === id);
        if (!atom || atom.Z !== 0 || atom.N !== 1) return;
        atom.nuclear_generation = Math.max(0, Math.floor(generation));
        atom.nuclear_source = source;
    }

    function aeInjectNuclearParticle(kind = 'neutron') {
        if (!state._ae?.nuclear?.channel) return _aeReject('enable a nuclear channel before injecting reactants');
        const nuclear = state._ae.nuclear;
        const required = kind === 'dt-pair' ? 2 : 1;
        if (state._ae.atoms.length + required > nuclear.particle_limit) {
            nuclear.source_saturated = true;
            return _aeReject(`nuclear particle limit ${nuclear.particle_limit} reached`);
        }
        nuclear.source_saturated = false;
        const ordinal = state._ae.nextId + state._ae.tick * 4099;
        if (kind === 'neutron') {
            const neutronMass = NEUTRON_PROTON_MASS_RATIO;
            const speed = Math.sqrt(2 * nuclear.source_energy_mev / Math.max(M_P_PHYS * neutronMass, 1e-30));
            const radius = nuclear.transport_radius * 0.82;
            const spreadY = (_aeNuclearSample(ordinal, 1) - 0.5) * nuclear.transport_radius * 0.3;
            const spreadZ = (_aeNuclearSample(ordinal, 2) - 0.5) * nuclear.transport_radius * 0.3;
            const directionLength = Math.hypot(radius, spreadY, spreadZ) || 1;
            const direction = { x: radius / directionLength, y: -spreadY / directionLength, z: -spreadZ / directionLength };
            const id = aeAddAtom(0, -radius, spreadY, spreadZ,
                direction.x * speed, direction.y * speed, direction.z * speed, 0, 1);
            if (id < 0) return false;
            _aeTagNeutron(id, 0, 'source');
            nuclear.source_neutrons++;
            return id;
        }
        if (kind === 'dt-pair') {
            const velocities = incidentVelocities('dt_fusion');
            const axis = _aeNuclearDirection(ordinal, 3);
            const center = _aeNuclearDirection(ordinal, 4);
            const centerScale = nuclear.transport_radius * 0.2 * _aeNuclearSample(ordinal, 5);
            const half = getNuclearReactionChannel('dt_fusion').captureRadius *
                nuclear.collision_radius_scale * 0.42;
            const cx = center.x * centerScale, cy = center.y * centerScale, cz = center.z * centerScale;
            const dSpeed = Math.hypot(velocities[0].vx, velocities[0].vy, velocities[0].vz);
            const tSpeed = Math.hypot(velocities[1].vx, velocities[1].vy, velocities[1].vz);
            const dId = aeAddAtom(1, cx - axis.x * half, cy - axis.y * half, cz - axis.z * half,
                axis.x * dSpeed, axis.y * dSpeed, axis.z * dSpeed, 0, 1);
            const tId = aeAddAtom(1, cx + axis.x * half, cy + axis.y * half, cz + axis.z * half,
                -axis.x * tSpeed, -axis.y * tSpeed, -axis.z * tSpeed, 0, 2);
            return dId >= 0 && tId >= 0 ? dId : false;
        }
        if (kind === 'u235') {
            let position = { x: 0, y: 0, z: 0 };
            const spacing = Math.max(1.4, 1.3 * getNuclearReactionChannel('u235_fission').captureRadius *
                nuclear.collision_radius_scale);
            const candidates = [];
            for (let x = -2; x <= 2; x++) for (let y = -2; y <= 2; y++) for (let z = -2; z <= 2; z++) {
                candidates.push({ x: x * spacing, y: y * spacing, z: z * spacing, r2: x * x + y * y + z * z });
            }
            candidates.sort((a, b) => a.r2 - b.r2 || a.x - b.x || a.y - b.y || a.z - b.z);
            const free = candidates.find(candidate => state._ae.atoms.every(atom => atom.Z !== 92 ||
                Math.hypot(atom.x - candidate.x, atom.y - candidate.y, atom.z - candidate.z) > spacing * 0.8));
            if (free) {
                position = free;
            } else {
                const direction = _aeNuclearDirection(ordinal, 7);
                const radius = nuclear.transport_radius * 0.65 * Math.cbrt(_aeNuclearSample(ordinal, 8));
                position = { x: direction.x * radius, y: direction.y * radius, z: direction.z * radius };
            }
            const id = aeAddLockedAtom(92, position.x, position.y, position.z, 0, 143);
            nuclear.fuel_initial++;
            nuclear.fuel_remaining++;
            return id;
        }
        return _aeReject(`unknown nuclear injection: ${kind}`);
    }

    function _aeEmitNuclearSource() {
        const nuclear = state._ae?.nuclear;
        if (!nuclear?.channel || !nuclear.source_enabled || nuclear.source_rate <= 0) return;
        nuclear.source_accumulator = Math.min(4, nuclear.source_accumulator + nuclear.source_rate);
        let emitted = 0;
        while (nuclear.source_accumulator >= 1 && emitted < 4) {
            if (aeInjectNuclearParticle(nuclear.channel === 'dt_fusion' ? 'dt-pair' : 'neutron') === false) {
                nuclear.source_accumulator = Math.min(1, nuclear.source_accumulator);
                break;
            }
            nuclear.source_accumulator -= 1;
            emitted++;
        }
    }

    function _aeRefreshMeasuredK() {
        const nuclear = state._ae.nuclear;
        const resolved = nuclear.neutron_fission_losses + nuclear.absorbed_neutrons + nuclear.leaked_neutrons;
        nuclear.k_effective = resolved > 0 ? nuclear.fission_neutron_births / resolved : 0;
    }

    function _aeApplyNuclearEvent(event, tick, generation = 0) {
        const nuclear = state._ae.nuclear;

        const consumed = new Set(event.inputIds);
        for (let i = state._ae.atoms.length - 1; i >= 0; i--) {
            if (consumed.has(state._ae.atoms[i].id)) state._ae.atoms.splice(i, 1);
        }
        for (const atom of state._ae.atoms) {
            atom.bonds = atom.bonds.filter(bond => !consumed.has(bond.partner_id));
        }

        const productIds = [];
        for (const product of event.products) {
            productIds.push(aeAddAtom(
                product.Z, product.x, product.y, product.z,
                product.vx, product.vy, product.vz, 0, product.N,
            ));
        }
        if (productIds.some(id => id < 0)) return false;
        let emittedNeutrons = 0;
        for (let i = 0; i < event.products.length; i++) {
            if (event.products[i].Z !== 0 || event.products[i].N !== 1) continue;
            emittedNeutrons++;
            _aeTagNeutron(productIds[i], event.kind === 'fission' ? generation + 1 : 0, event.kind);
        }
        if (event.kind === 'fission') {
            nuclear.neutron_fission_losses++;
            nuclear.fission_neutron_births += emittedNeutrons;
        }

        nuclear.event_count++;
        nuclear.represented_event_count += nuclear.event_weight;
        nuclear.event_tick = tick;
        nuclear.generation = Math.max(nuclear.generation, generation);
        nuclear.microscopic_released_mev += event.totalReleasedMeV;
        nuclear.released_mev += event.totalReleasedMeV * nuclear.event_weight;
        nuclear.released_joule = nuclear.released_mev * MEV_TO_JOULE;
        nuclear.kinetic_mev += event.kineticReleaseMeV * nuclear.event_weight;
        nuclear.charged_mev += event.chargedKineticMeV * nuclear.event_weight;
        nuclear.neutron_mev += event.neutronKineticMeV * nuclear.event_weight;
        nuclear.prompt_gamma_mev += event.promptGammaMeV * nuclear.event_weight;
        nuclear.delayed_heat_mev += event.delayedHeatMeV * nuclear.event_weight;
        const neutronDirections = event.products
            .filter(product => product.Z === 0 && product.N === 1)
            .map((product) => {
                const magnitude = Math.hypot(product.vx, product.vy, product.vz);
                return magnitude > 1e-12
                    ? { x: product.vx / magnitude, y: product.vy / magnitude, z: product.vz / magnitude }
                    : { x: 1, y: 0, z: 0 };
            });
        const record = {
            ordinal: nuclear.event_count,
            tick,
            generation,
            x: event.center.x,
            y: event.center.y,
            z: event.center.z,
            axisX: event.axis?.x ?? 1,
            axisY: event.axis?.y ?? 0,
            axisZ: event.axis?.z ?? 0,
            weight: nuclear.event_weight,
            totalMeV: event.totalReleasedMeV * nuclear.event_weight,
            chargedMeV: event.chargedKineticMeV * nuclear.event_weight,
            neutronMeV: event.neutronKineticMeV * nuclear.event_weight,
            gammaMeV: event.promptGammaMeV * nuclear.event_weight,
            delayedMeV: event.delayedHeatMeV * nuclear.event_weight,
            collisionEnergyMeV: event.collisionEnergyMeV,
            reactionProbability: event.reactionProbability,
            neutronDirections,
        };
        nuclear.history.push(record);
        nuclear.effects.push({ ...record, kind: event.kind });
        if (nuclear.effects.length > 256) nuclear.effects.splice(0, nuclear.effects.length - 256);
        nuclear.last_event = {
            ...event,
            generation,
            productIds,
            products: event.products.map(({ Z, N, label }) => ({ Z, N, label })),
        };
        _aeRefreshMeasuredK();
        return true;
    }

    function _aeNuclearTransportForEvent(event, tick, nuclear) {
        const progress = (age, tau) => 1 - Math.exp(-Math.max(0, age) / tau);
        const age = tick - event.tick;
        const chargedP = progress(age, 12);
        const neutronP = progress(age, 60);
        const gammaP = progress(age, 35);
        const delayedP = progress(age, 180);
        const deposited = event.chargedMeV * chargedP
            + event.neutronMeV * nuclear.neutron_containment * neutronP
            + event.gammaMeV * nuclear.gamma_containment * gammaP
            + event.delayedMeV * delayedP;
        const escaped = event.neutronMeV * (1 - nuclear.neutron_containment) * neutronP
            + event.gammaMeV * (1 - nuclear.gamma_containment) * gammaP;
        return { deposited, escaped };
    }

    function _aeUpdateNuclearTransport(tick) {
        const nuclear = state._ae.nuclear;
        let deposited = 0, escaped = 0;
        for (const event of nuclear.history) {
            const transport = _aeNuclearTransportForEvent(event, tick, nuclear);
            deposited += transport.deposited;
            escaped += transport.escaped;
        }
        nuclear.deposited_mev = deposited;
        nuclear.escaped_mev = escaped;
        nuclear.in_transit_mev = Math.max(0, nuclear.released_mev - deposited - escaped);
    }

    function _aeRemoveNuclearAtoms(ids) {
        if (ids.size === 0) return;
        for (let i = state._ae.atoms.length - 1; i >= 0; i--) {
            if (ids.has(state._ae.atoms[i].id)) state._ae.atoms.splice(i, 1);
        }
        for (const atom of state._ae.atoms) {
            atom.bonds = atom.bonds.filter(bond => !ids.has(bond.partner_id));
        }
    }

    function _aeProcessNeutronEnvironment(tick) {
        const nuclear = state._ae.nuclear;
        const remove = new Set();
        for (const neutron of state._ae.atoms) {
            if (neutron.Z !== 0 || neutron.N !== 1) continue;
            const radius = Math.hypot(neutron.x, neutron.y, neutron.z);
            if (radius > nuclear.transport_radius) {
                if (nuclear.boundary_mode === 'reflect') {
                    const nx = neutron.x / radius, ny = neutron.y / radius, nz = neutron.z / radius;
                    const radialVelocity = neutron.vx * nx + neutron.vy * ny + neutron.vz * nz;
                    if (radialVelocity > 0) {
                        neutron.vx -= 2 * radialVelocity * nx;
                        neutron.vy -= 2 * radialVelocity * ny;
                        neutron.vz -= 2 * radialVelocity * nz;
                    }
                    const inside = nuclear.transport_radius * (1 - 1e-6);
                    neutron.x = nx * inside; neutron.y = ny * inside; neutron.z = nz * inside;
                } else {
                    remove.add(neutron.id);
                    nuclear.leaked_neutrons++;
                    continue;
                }
            }
            const absorbP = 1 - Math.exp(-nuclear.absorber_strength * state._ae.dt * 0.75);
            if (_aeNuclearSample(tick, neutron.id, 71) < absorbP) {
                remove.add(neutron.id);
                nuclear.absorbed_neutrons++;
                continue;
            }
            const scatterP = 1 - Math.exp(-nuclear.moderator_strength * state._ae.dt * 0.5);
            if (_aeNuclearSample(tick, neutron.id, 83) < scatterP) {
                const direction = _aeNuclearDirection(tick, neutron.id, nuclear.scattered_neutrons);
                const speed = Math.hypot(neutron.vx, neutron.vy, neutron.vz) *
                    Math.sqrt(Math.max(0.05, 1 - 0.55 * nuclear.moderator_strength));
                neutron.vx = direction.x * speed;
                neutron.vy = direction.y * speed;
                neutron.vz = direction.z * speed;
                nuclear.scattered_neutrons++;
            }
        }
        _aeRemoveNuclearAtoms(remove);
        _aeRefreshMeasuredK();
    }

    function _aeProcessNuclearReaction(tick, previousById = null) {
        const nuclear = state._ae.nuclear;
        if (!nuclear?.channel) return false;
        if (nuclear.fuel_initial === 0 && nuclear.channel === 'u235_fission') {
            nuclear.fuel_initial = state._ae.atoms.filter(atom => atom.Z === 92 && atom.N === 143).length;
        }

        _aeProcessNeutronEnvironment(tick);
        let changed = false;
        let processed = 0;
        while (processed < 32 && nuclear.event_count < nuclear.event_limit) {
            const event = evaluateNuclearReaction(nuclear.channel, state._ae.atoms, tick, {
                previousById,
                collisionRadiusScale: nuclear.collision_radius_scale,
                reactivityScale: nuclear.reactivity_scale,
                sampleForPair: (firstId, secondId) =>
                    _aeNuclearSample(tick, firstId, secondId, nuclear.event_count, 53),
            });
            if (!event) break;
            const inputNeutron = state._ae.atoms.find(atom =>
                event.inputIds.includes(atom.id) && atom.Z === 0 && atom.N === 1);
            const generation = inputNeutron?.nuclear_generation || 0;
            if (!_aeApplyNuclearEvent(event, tick, generation)) break;
            changed = true;
            processed++;
        }

        nuclear.fuel_remaining = nuclear.channel === 'u235_fission'
            ? state._ae.atoms.filter(atom => atom.Z === 92 && atom.N === 143).length
            : Math.max(0, nuclear.event_limit - nuclear.event_count);
        const liveNeutrons = state._ae.atoms.filter(atom => atom.Z === 0 && atom.N === 1).length;
        if (nuclear.mode === 'chain' || nuclear.mode === 'sandbox') {
            nuclear.phase = nuclear.event_count >= nuclear.event_limit ? 'event-limit'
                : nuclear.fuel_remaining === 0 ? 'fuel-depleted'
                : liveNeutrons > 0 || nuclear.source_enabled ? (nuclear.event_count > 0 ? 'multiplying' : 'transport')
                    : nuclear.event_count > 0 ? 'extinct' : 'armed';
        } else {
            nuclear.phase = nuclear.event_count >= nuclear.event_limit ? 'complete'
                : nuclear.event_count > 0 ? 'reacting' : 'armed';
        }
        nuclear.events_per_100_ticks = nuclear.history.filter(event => tick - event.tick < 100).length;
        _aeUpdateNuclearTransport(tick);
        return changed;
    }

    function aeAddLockedAtom(Z, x, y, z, charge = 0, N = -1) {
        const id = aeAddAtom(Z, x, y, z, 0, 0, 0, charge, N);
        if (state._ae && id >= 0) {
            state._ae.atoms[state._ae.atoms.length - 1].locked = true;
        }
        return id;
    }

    function aeCreateBond(idA, idB, order = 1) {
        if (!state._ae || !Number.isInteger(idA) || !Number.isInteger(idB) || idA === idB ||
            !Number.isFinite(order) || order <= 0 || order > MAX_BOND_ORDER) return false;
        const a = state._ae.atoms.find(at => at.id === idA);
        const b = state._ae.atoms.find(at => at.id === idB);
        if (!a || !b || a.bonds.some(bond => bond.partner_id === idB)) return false;
        const sig_avg = (a.vdw_sigma + b.vdw_sigma) / 2;
        const r_eq = sig_avg * Math.pow(2, 1.0 / 6.0) / order;
        const eps_mix = Math.sqrt(a.vdw_epsilon * b.vdw_epsilon);
        const k_bond = AE_K_BOND * eps_mix / (r_eq * r_eq);
        a.bonds.push({ partner_id: idB, r_eq, k_bond, order });
        b.bonds.push({ partner_id: idA, r_eq, k_bond, order });
        return true;
    }

    /**
     * Build bond lookup structures for O(1) bond checks and partner lookups.
     * Called once per force evaluation to avoid O(bonds) scans in the inner loop.
     *
     * CAUTION: The bond key formula `lo * 100000 + hi` assumes atom IDs < 100000.
     * Since _ae.nextId increments monotonically and typical simulations have
     * < 1000 atoms, this is safe. If atom IDs ever exceed 100000, collisions
     * would cause false bond-pair matches. Use string keys as fallback if needed.
     */
    function _aeBuildBondLookup() {
        const atoms = state._ae.atoms;
        const bondSet = new Set();
        const idToIdx = new Map();
        const neighborSets = new Array(atoms.length);

        for (let i = 0; i < atoms.length; i++) {
            idToIdx.set(atoms[i].id, i);
            const ns = new Set();
            for (const b of atoms[i].bonds) {
                const lo = Math.min(atoms[i].id, b.partner_id);
                const hi = Math.max(atoms[i].id, b.partner_id);
                bondSet.add(lo * 100000 + hi);
                ns.add(b.partner_id);
            }
            neighborSets[i] = ns;
        }
        state._aeBondSet = bondSet;
        state._aeIdToIdx = idToIdx;
        state._aeNeighborSets = neighborSets;
    }

    function _aeIsBonded(id_a, id_b) {
        const lo = Math.min(id_a, id_b), hi = Math.max(id_a, id_b);
        return state._aeBondSet.has(lo * 100000 + hi);
    }

    function _aeIs13(i, j) {
        const nsI = state._aeNeighborSets[i];
        const nsJ = state._aeNeighborSets[j];
        for (const pid of nsI) {
            if (nsJ.has(pid)) return true;
        }
        return false;
    }

    function _aeComputeDipoleMoments(updateCharges = true) {
        const atoms = state._ae.atoms;
        // Visualization may request dipoles while charge equilibration is off.
        // In that case dipole inspection must not mutate the dynamical charges.
        for (const a of atoms) {
            a.dipole_x = 0; a.dipole_y = 0; a.dipole_z = 0;
            if (updateCharges) a.q_frac = a.charge;
        }
        
        for (const a of atoms) {
            for (const bond of a.bonds) {
                const jIdx = state._aeIdToIdx.get(bond.partner_id);
                if (jIdx === undefined) continue;
                const aj = atoms[jIdx];
                const chi_diff = aj.electronegativity - a.electronegativity;
                
                // [IMPOSED] QEq-like transfer. Apply only when the explicit
                // electronegativity dynamics toggle is active.
                if (updateCharges && state._ae.electronegativity)
                    a.q_frac += 0.5 * chi_diff;

                if (Math.abs(chi_diff) < 1e-10) continue;
                a.dipole_x += (aj.x - a.x) * chi_diff;
                a.dipole_y += (aj.y - a.y) * chi_diff;
                a.dipole_z += (aj.z - a.z) * chi_diff;
            }
        }
    }

    function _aeEquilibriumAngle(atom) {
        const nbonds = atom.bonds.length;
        const lonePairs = Math.max(0, Math.floor((atom.valence_electrons - nbonds) / 2));
        const steric = nbonds + lonePairs;
        if (steric === 2) return Math.PI;
        if (steric === 3) return 2 * Math.PI / 3;
        if (steric === 4) {
            if (lonePairs === 0) return Math.acos(-1 / 3);
            if (lonePairs === 1) return 107 * Math.PI / 180;
            return 104.5 * Math.PI / 180;
        }
        return Math.acos(-1 / 3);
    }

    function _aeComputeForce(i, parts = null) {
        const atoms = state._ae.atoms;
        const ai = atoms[i];
        let fx = 0, fy = 0, fz = 0;
        const soft2 = state._ae.soft * state._ae.soft;
        const add = (component, x, y, z) => {
            fx += x; fy += y; fz += z;
            if (parts) {
                parts[component].fx += x;
                parts[component].fy += y;
                parts[component].fz += z;
            }
        };

        for (let j = 0; j < atoms.length; j++) {
            if (j === i) continue;
            const aj = atoms[j];
            const dx = aj.x - ai.x, dy = aj.y - ai.y, dz = aj.z - ai.z;
            const r2 = dx * dx + dy * dy + dz * dz + soft2;
            const r = Math.sqrt(r2);
            if (r < 1e-20) continue;
            const rx = dx / r, ry = dy / r, rz = dz / r;

            // 1-2 exclusion: bonded pairs use spring instead of LJ (O(1) lookup)
            const isBonded = _aeIsBonded(ai.id, aj.id);

            // 1-3 exclusion: atoms sharing a bonded partner (O(bonds) via Set)
            const is13 = !isBonded && _aeIs13(i, j);

            // Ionic (Coulomb) — skip for bonded and 1-3 pairs
            if (state._ae.ionic && !isBonded && !is13 && (Math.abs(ai.q_frac) > 1e-6 || Math.abs(aj.q_frac) > 1e-6)) {
                const f_ionic = -AE_K_COULOMB * ai.q_frac * aj.q_frac / r2;
                add('ionic', f_ionic * rx, f_ionic * ry, f_ionic * rz);
            }

            // Van der Waals (LJ 12-6) — skip for bonded and 1-3 pairs
            if (state._ae.vdw && !isBonded && !is13) {
                const eps_mix = Math.sqrt(ai.vdw_epsilon * aj.vdw_epsilon);
                const sig_mix = (ai.vdw_sigma + aj.vdw_sigma) / 2;
                const sr = sig_mix / r;
                const sr6 = sr * sr * sr * sr * sr * sr;
                const sr12 = sr6 * sr6;
                const f_vdw = -24.0 * eps_mix * (2.0 * sr12 - sr6) / r;
                add('vdw', f_vdw * rx, f_vdw * ry, f_vdw * rz);
            }

            // H-bonds: LJ 10-12 + cos²(θ_DHA) angular factor
            if (state._ae.h_bonds) {
                const isElecNeg = (Z) => Z === 7 || Z === 8 || Z === 9;
                const hbondForce = (hAtom, acceptor, _hIdx, aIdx) => {
                    let donorIdx = -1;
                    for (const b of hAtom.bonds) {
                        const didx = state._aeIdToIdx.get(b.partner_id);
                        if (didx !== undefined && isElecNeg(atoms[didx].Z)) { donorIdx = didx; break; }
                    }
                    if (donorIdx < 0 || donorIdx === aIdx) return;
                    const sig_hb = (hAtom.vdw_sigma + acceptor.vdw_sigma) / 2;
                    if (sig_hb <= 0 || r < 1e-10) return;
                    const shr = sig_hb / r;
                    const shr10 = Math.pow(shr, 10);
                    const shr12 = shr10 * shr * shr;
                    // V = eps*(5*(sigma/r)^12 - 6*(sigma/r)^10).
                    // Since r_hat points from the current atom to its partner,
                    // the force on the current atom is +dV/dr * r_hat.
                    const f_rad = AE_H_BOND_EPS * 60.0 * (shr10 - shr12) / r;
                    const donor = atoms[donorIdx];
                    const dhx = hAtom.x - donor.x, dhy = hAtom.y - donor.y, dhz = hAtom.z - donor.z;
                    const hax = acceptor.x - hAtom.x, hay = acceptor.y - hAtom.y, haz = acceptor.z - hAtom.z;
                    const dh_mag = Math.sqrt(dhx*dhx + dhy*dhy + dhz*dhz);
                    const ha_mag = Math.sqrt(hax*hax + hay*hay + haz*haz);
                    let cos_theta = 1.0;
                    if (dh_mag > 1e-30 && ha_mag > 1e-30)
                        cos_theta = (dhx*hax + dhy*hay + dhz*haz) / (dh_mag * ha_mag);
                    const ang = cos_theta * cos_theta;
                    add('hbond', f_rad * ang * rx, f_rad * ang * ry, f_rad * ang * rz);
                };
                if (ai.Z === 1 && isElecNeg(aj.Z)) hbondForce(ai, aj, i, j);
                if (aj.Z === 1 && isElecNeg(ai.Z)) hbondForce(aj, ai, j, i);
            }

            // Dipole-dipole: 1/r^5 interaction between pre-computed molecular dipoles
            if (state._ae.dipole_dipole) {
                const mi_x = ai.dipole_x, mi_y = ai.dipole_y, mi_z = ai.dipole_z;
                const mj_x = aj.dipole_x, mj_y = aj.dipole_y, mj_z = aj.dipole_z;
                const mi_mag2 = mi_x*mi_x + mi_y*mi_y + mi_z*mi_z;
                const mj_mag2 = mj_x*mj_x + mj_y*mj_y + mj_z*mj_z;
                if (mi_mag2 > 1e-60 && mj_mag2 > 1e-60 && r > 1e-10) {
                    const mi_dot_r = mi_x*rx + mi_y*ry + mi_z*rz;
                    const mj_dot_r = mj_x*rx + mj_y*ry + mj_z*rz;
                    const mi_dot_mj = mi_x*mj_x + mi_y*mj_y + mi_z*mj_z;
                    // P1 (2026-07-26). Two defects, both fixed here:
                    //   1. rx,ry,rz are ALREADY unit vectors (divided by r above),
                    //      so the extra /r2 inside t1 made the bracket
                    //      dimensionally inhomogeneous, and coeff carried one
                    //      power of r too many (3k/r^5 instead of 3k/r^4).
                    //   2. Every bracket term had the wrong sign -- the old
                    //      expression was exactly the NEGATIVE of the standard
                    //      dipole-dipole force.
                    // Net effect: head-to-tail dipoles were correct only at r=1
                    // by coincidence, ZERO at r = sqrt(5/3) = 1.291, and
                    // repulsive beyond, with a spurious r^-5 tail.
                    //
                    // Canonical form (identical to atom_forces.cpp and to the
                    // already-correct pe-force-kernel.js):
                    //   F = (3k/r^4) [ (mi.mj) rhat + (mi.rhat) mj
                    //                  + (mj.rhat) mi - 5 (mi.rhat)(mj.rhat) rhat ]
                    const coeff = 3.0 * AE_K_COULOMB / (r2 * r2);
                    const t5 = 5.0 * mi_dot_r * mj_dot_r;
                    // The bracket is the force on j for R = r_j-r_i; the
                    // force on the current atom i is its negative.  This
                    // matches the corrected native AtomEngine assignment.
                    add('dipole',
                        -coeff * (mi_dot_mj*rx + mi_dot_r*mj_x + mj_dot_r*mi_x - t5*rx),
                        -coeff * (mi_dot_mj*ry + mi_dot_r*mj_y + mj_dot_r*mi_y - t5*ry),
                        -coeff * (mi_dot_mj*rz + mi_dot_r*mj_z + mj_dot_r*mi_z - t5*rz));
                }
            }
        }

        // Bond spring forces (O(1) partner lookup via Map)
        if (state._ae.bonds_force) {
            for (const bond of ai.bonds) {
                const jIdx = state._aeIdToIdx.get(bond.partner_id);
                const aj = jIdx !== undefined ? atoms[jIdx] : null;
                if (!aj) continue;
                const dx = aj.x - ai.x, dy = aj.y - ai.y, dz = aj.z - ai.z;
                const r = Math.sqrt(dx * dx + dy * dy + dz * dz + soft2);
                if (r < 1e-20) continue;
                const rx = dx / r, ry = dy / r, rz = dz / r;
                const dr = r - bond.r_eq;
                const f_bond = bond.k_bond * dr;
                add('bond', f_bond * rx, f_bond * ry, f_bond * rz);
            }
        }

        // Angle strain / VSEPR (3-body): restoring force toward equilibrium angles
        // Force on central atom i only; terminal atoms get Newton's 3rd in _aeComputeAllForces
        if (state._ae.angle_strain && ai.bonds.length >= 2) {
            for (let b1 = 0; b1 < ai.bonds.length; b1++) {
                for (let b2 = b1 + 1; b2 < ai.bonds.length; b2++) {
                    const j1 = state._aeIdToIdx.get(ai.bonds[b1].partner_id);
                    const j2 = state._aeIdToIdx.get(ai.bonds[b2].partner_id);
                    if (j1 === undefined || j2 === undefined) continue;
                    const a1 = atoms[j1], a2 = atoms[j2];
                    const r1x = a1.x - ai.x, r1y = a1.y - ai.y, r1z = a1.z - ai.z;
                    const r2x = a2.x - ai.x, r2y = a2.y - ai.y, r2z = a2.z - ai.z;
                    const m1 = Math.sqrt(r1x*r1x + r1y*r1y + r1z*r1z);
                    const m2 = Math.sqrt(r2x*r2x + r2y*r2y + r2z*r2z);
                    if (m1 < 1e-30 || m2 < 1e-30) continue;

                    let cos_t = (r1x*r2x + r1y*r2y + r1z*r2z) / (m1 * m2);
                    cos_t = Math.max(-1, Math.min(1, cos_t));
                    const theta = Math.acos(cos_t);

                    const theta_eq = _aeEquilibriumAngle(ai);

                    const sin_t = Math.sin(theta);
                    if (Math.abs(sin_t) < 1e-15) continue;
                    const dV = AE_K_ANGLE * (theta - theta_eq);

                    const r1hx = r1x/m1, r1hy = r1y/m1, r1hz = r1z/m1;
                    const r2hx = r2x/m2, r2hy = r2y/m2, r2hz = r2z/m2;
                    let p1x = r2hx - cos_t*r1hx, p1y = r2hy - cos_t*r1hy, p1z = r2hz - cos_t*r1hz;
                    const pm1 = Math.sqrt(p1x*p1x + p1y*p1y + p1z*p1z);
                    if (pm1 < 1e-30) continue;
                    p1x /= pm1; p1y /= pm1; p1z /= pm1;
                    let p2x = r1hx - cos_t*r2hx, p2y = r1hy - cos_t*r2hy, p2z = r1hz - cos_t*r2hz;
                    const pm2 = Math.sqrt(p2x*p2x + p2y*p2y + p2z*p2z);
                    if (pm2 < 1e-30) continue;
                    p2x /= pm2; p2y /= pm2; p2z /= pm2;

                    // p1/p2 are normalized after their raw magnitude
                    // sin(theta) is measured, so the analytic gradient is
                    // dV/m rather than dV/(m*sin(theta)).
                    const fj1 = dV / m1;
                    const fj2 = dV / m2;
                    add('angle',
                        -(fj1 * p1x + fj2 * p2x),
                        -(fj1 * p1y + fj2 * p2y),
                        -(fj1 * p1z + fj2 * p2z));
                }
            }
        }

        return { fx, fy, fz };
    }

    /**
     * Infer covalent bond orders (single / double / triple / aromatic) from
     * geometry + valence saturation. Audit P0-12/P0-13: the auto-bonder only
     * knows connectivity, so without this pass every bond renders as order 1
     * and the multi-order molecules (O₂, N₂, CO₂, ethylene, acetylene,
     * benzene, carbonyls) look identical to single-bonded ones.
     *
     * Rule (two signals, run after all bonds exist):
     *   1. Valence saturation (primary): each atom targets a total bond order
     *      equal to its covalent valence v(Z). Its residual = v − degree is the
     *      number of extra order-units it still needs. A bond between two atoms
     *      that both still have residual capacity is promoted; this is what
     *      turns the single O–O into O=O, the single N–N into N≡N, each C–O in
     *      CO₂ into C=O, the C–C in ethylene into C=C, and in acetylene into C≡C.
     *   2. Distance ordering (tie-breaker): bonds are promoted shortest-first
     *      (smallest r/r_eq), so when an atom has more candidate partners than
     *      residual capacity the geometrically tighter (genuinely multiple)
     *      bond wins. Multiply-bonded atoms are placed closer in molecules.js.
     *
     * Aromatic rings (e.g. benzene) are detected first: a maximal set of
     * carbons each with residual exactly 1 that forms a closed cycle (every
     * member has ≥2 ring neighbours in the set) has all its intra-set bonds
     * marked aromatic (order AROMATIC_ORDER) and its residual cleared, so the
     * ring renders as a uniform delocalised ring rather than a Kekulé
     * single/double alternation.
     *
     * Orders are written to BOTH directed half-edges (ai→aj and aj→ai).
     * Idempotent: resets every order to 1 before re-inferring, so it is safe
     * to call after each auto-bonding pass.
     *
     * KNOWN LIMITATION: molecules whose hand-built geometry is an incomplete
     * fragment (the 8-atom 'diamond' cell, parts of 'caffeine') leave some
     * carbons under-coordinated — they are missing single-bond neighbours that
     * a full crystal/ring would supply. Valence saturation then reads that
     * missing connectivity as unsaturation and may promote a normal single
     * bond to double/triple. This is a geometry-completeness artifact in those
     * two non-canonical molecules, not an inference error; every advertised
     * multiple bond in the diatomics, CO₂, the alkenes/alkynes, the carbonyls,
     * and benzene is inferred correctly. (See AUDIT_WEB_ENGINE_2026-05-27 H-11.)
     */
    function _aeInferBondOrders() {
        if (!state._ae) return;
        const atoms = state._ae.atoms;
        if (atoms.length === 0) return;

        const idToIdx = new Map();
        for (let i = 0; i < atoms.length; i++) idToIdx.set(atoms[i].id, i);

        // Reset all directed half-edges to single before inferring.
        for (const a of atoms) {
            for (const b of a.bonds) b.order = 1;
        }

        // Build the undirected bond list (one entry per pair, i < j by index).
        const bonds = [];
        for (let i = 0; i < atoms.length; i++) {
            const a = atoms[i];
            for (const b of a.bonds) {
                const j = idToIdx.get(b.partner_id);
                if (j === undefined || j <= i) continue;
                const aj = atoms[j];
                const dx = aj.x - a.x, dy = aj.y - a.y, dz = aj.z - a.z;
                const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
                const ratio = b.r_eq > 0 ? r / b.r_eq : 1;
                bonds.push({ i, j, ratio });
            }
        }
        if (bonds.length === 0) return;

        // Per-atom residual valence = covalent valence − bond degree.
        const degree = new Array(atoms.length).fill(0);
        for (const e of bonds) { degree[e.i]++; degree[e.j]++; }
        const residual = new Array(atoms.length).fill(0);
        for (let i = 0; i < atoms.length; i++) {
            residual[i] = Math.max(0, _covalentValence(atoms[i].Z) - degree[i]);
        }

        // ── Aromatic-ring detection ────────────────────────────────────────
        // Candidates: carbons with residual exactly 1. A candidate is in a ring
        // iff it has ≥2 bonds to other candidates. Iteratively drop candidates
        // with <2 candidate-neighbours (peel chains/leaves); what survives is
        // the set of closed-cycle aromatic carbons.
        const isCandidate = new Array(atoms.length).fill(false);
        for (let i = 0; i < atoms.length; i++) {
            if (atoms[i].Z === 6 && residual[i] === 1) isCandidate[i] = true;
        }
        // Adjacency among candidates.
        const candAdj = new Map(); // idx -> Set of candidate neighbour idxs
        const ensure = (k) => { if (!candAdj.has(k)) candAdj.set(k, new Set()); return candAdj.get(k); };
        for (const e of bonds) {
            if (isCandidate[e.i] && isCandidate[e.j]) {
                ensure(e.i).add(e.j);
                ensure(e.j).add(e.i);
            }
        }
        let changed = true;
        while (changed) {
            changed = false;
            for (let i = 0; i < atoms.length; i++) {
                if (!isCandidate[i]) continue;
                const nbrs = candAdj.get(i);
                const live = nbrs ? [...nbrs].filter(k => isCandidate[k]).length : 0;
                if (live < 2) { isCandidate[i] = false; changed = true; }
            }
        }
        // Mark intra-ring bonds aromatic; clear residual of ring atoms.
        const aromaticBond = new Array(bonds.length).fill(false);
        for (let bi = 0; bi < bonds.length; bi++) {
            const e = bonds[bi];
            if (isCandidate[e.i] && isCandidate[e.j]) {
                aromaticBond[bi] = true;
            }
        }
        for (let i = 0; i < atoms.length; i++) {
            if (isCandidate[i]) residual[i] = 0;
        }

        // ── Greedy valence-saturation promotion (shortest bond first) ──────
        const order = new Array(bonds.length).fill(1);
        const idxOrder = bonds.map((_, k) => k).sort((p, q) => bonds[p].ratio - bonds[q].ratio);
        for (const bi of idxOrder) {
            if (aromaticBond[bi]) continue;
            const e = bonds[bi];
            while (residual[e.i] > 0 && residual[e.j] > 0 && order[bi] < MAX_BOND_ORDER) {
                order[bi]++;
                residual[e.i]--;
                residual[e.j]--;
            }
        }

        // ── Write orders back to both directed half-edges ──────────────────
        const finalOrder = (bi) => aromaticBond[bi] ? AROMATIC_ORDER : order[bi];
        for (let bi = 0; bi < bonds.length; bi++) {
            const e = bonds[bi];
            const ai = atoms[e.i], aj = atoms[e.j];
            const o = finalOrder(bi);
            const hAB = ai.bonds.find(b => b.partner_id === aj.id);
            const hBA = aj.bonds.find(b => b.partner_id === ai.id);
            if (hAB) hAB.order = o;
            if (hBA) hBA.order = o;
        }
    }

    /**
     * Run auto-bonding logic without physics integration.
     * Call after loading a molecule to establish bonds before the first tick.
     */
    function aePreBond() {
        if (!state._ae || !state._ae.bonding) {
            debugLog('[FTD aePreBond] skipped — ae:', !!state._ae, 'bonding:', state._ae?.bonding);
            return;
        }
        const atoms = state._ae.atoms;
        let bondsCreated = 0;
        for (let i = 0; i < atoms.length; i++) {
            for (let j = i + 1; j < atoms.length; j++) {
                const ai = atoms[i], aj = atoms[j];
                if (ai.bonds.some(b => b.partner_id === aj.id)) continue;
                if (ai.bonds.length >= ai.max_bonds || aj.bonds.length >= aj.max_bonds) continue;
                const dx = aj.x - ai.x, dy = aj.y - ai.y, dz = aj.z - ai.z;
                const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
                const sig_avg = (ai.vdw_sigma + aj.vdw_sigma) / 2;
                if (r < 1.2 * sig_avg) {
                    const r_eq = sig_avg * Math.pow(2, 1.0 / 6.0);
                    const eps_mix = Math.sqrt(ai.vdw_epsilon * aj.vdw_epsilon);
                    const k_bond = AE_K_BOND * eps_mix / (r_eq * r_eq);
                    ai.bonds.push({ partner_id: aj.id, r_eq, k_bond, order: 1 });
                    aj.bonds.push({ partner_id: ai.id, r_eq, k_bond, order: 1 });
                    bondsCreated++;
                }
            }
        }
        // Infer double/triple/aromatic orders now that connectivity is set
        // (audit P0-12/P0-13). Must run after every bond exists so valence
        // saturation sees the full degree of each atom.
        _aeInferBondOrders();
        debugLog(`[FTD aePreBond] ${atoms.length} atoms, ${bondsCreated} bonds created`);
        for (const a of atoms) {
            debugLog(`  atom ${a.id} Z=${a.Z} pos=(${a.x.toFixed(2)},${a.y.toFixed(2)},${a.z.toFixed(2)}) bonds=${a.bonds.length}/${a.max_bonds} sigma=${a.vdw_sigma.toFixed(2)}`);
        }
    }

    function _aeComputeAllForces(capture = false) {
        const atoms = state._ae.atoms;
        _aeBuildBondLookup();

        // Always reset q_frac to the formal charge. Charge transfer is applied
        // only when enabled; dipoles are refreshed when their force or visual
        // inputs can affect this evaluation.
        _aeComputeDipoleMoments(true);

        const forces = new Array(atoms.length);
        const components = capture ? Object.fromEntries(
            AE_FORCE_COMPONENTS.map(name => [name, new Array(atoms.length)])) : null;
        for (let i = 0; i < atoms.length; i++) {
            let parts = null;
            if (capture) {
                parts = {};
                for (const name of AE_FORCE_COMPONENTS)
                    parts[name] = components[name][i] = { fx: 0, fy: 0, fz: 0 };
            }
            forces[i] = _aeComputeForce(i, parts);
        }

        // Angle strain: distribute Newton's-3rd-law forces to terminal atoms
        if (state._ae.angle_strain) {
            for (let i = 0; i < atoms.length; i++) {
                const ai = atoms[i];
                if (ai.bonds.length < 2) continue;
                for (let b1 = 0; b1 < ai.bonds.length; b1++) {
                    for (let b2 = b1 + 1; b2 < ai.bonds.length; b2++) {
                        const j1 = state._aeIdToIdx.get(ai.bonds[b1].partner_id);
                        const j2 = state._aeIdToIdx.get(ai.bonds[b2].partner_id);
                        if (j1 === undefined || j2 === undefined) continue;
                        const a1 = atoms[j1], a2 = atoms[j2];
                        const r1x = a1.x-ai.x, r1y = a1.y-ai.y, r1z = a1.z-ai.z;
                        const r2x = a2.x-ai.x, r2y = a2.y-ai.y, r2z = a2.z-ai.z;
                        const m1 = Math.sqrt(r1x*r1x+r1y*r1y+r1z*r1z);
                        const m2 = Math.sqrt(r2x*r2x+r2y*r2y+r2z*r2z);
                        if (m1 < 1e-30 || m2 < 1e-30) continue;
                        let cos_t = (r1x*r2x+r1y*r2y+r1z*r2z)/(m1*m2);
                        cos_t = Math.max(-1, Math.min(1, cos_t));
                        const theta = Math.acos(cos_t);
                        const theta_eq = _aeEquilibriumAngle(ai);
                        const sin_t = Math.sin(theta);
                        if (Math.abs(sin_t) < 1e-15) continue;
                        const dV = AE_K_ANGLE * (theta - theta_eq);
                        const r1hx=r1x/m1, r1hy=r1y/m1, r1hz=r1z/m1;
                        const r2hx=r2x/m2, r2hy=r2y/m2, r2hz=r2z/m2;
                        let p1x=r2hx-cos_t*r1hx, p1y=r2hy-cos_t*r1hy, p1z=r2hz-cos_t*r1hz;
                        const pm1=Math.sqrt(p1x*p1x+p1y*p1y+p1z*p1z);
                        if (pm1<1e-30) continue;
                        p1x/=pm1; p1y/=pm1; p1z/=pm1;
                        let p2x=r1hx-cos_t*r2hx, p2y=r1hy-cos_t*r2hy, p2z=r1hz-cos_t*r2hz;
                        const pm2=Math.sqrt(p2x*p2x+p2y*p2y+p2z*p2z);
                        if (pm2<1e-30) continue;
                        p2x/=pm2; p2y/=pm2; p2z/=pm2;
                        const fj1 = dV/m1, fj2 = dV/m2;
                        forces[j1].fx += fj1*p1x; forces[j1].fy += fj1*p1y; forces[j1].fz += fj1*p1z;
                        forces[j2].fx += fj2*p2x; forces[j2].fy += fj2*p2y; forces[j2].fz += fj2*p2z;
                        if (capture) {
                            components.angle[j1].fx += fj1*p1x;
                            components.angle[j1].fy += fj1*p1y;
                            components.angle[j1].fz += fj1*p1z;
                            components.angle[j2].fx += fj2*p2x;
                            components.angle[j2].fy += fj2*p2y;
                            components.angle[j2].fz += fj2*p2z;
                        }
                    }
                }
            }
        }

        // A per-particle clamp violates action-reaction symmetry. If a safety
        // limit is necessary, scale the complete force field uniformly so zero
        // net internal force remains zero and expose the intervention in
        // diagnostics instead of silently pretending the step is conservative.
        let maxMagnitude = 0;
        for (const f of forces)
            maxMagnitude = Math.max(maxMagnitude, Math.hypot(f.fx, f.fy, f.fz));
        const clampScale = maxMagnitude > AE_FORCE_MAX ? AE_FORCE_MAX / maxMagnitude : 1.0;
        if (clampScale < 1.0) {
            for (const f of forces) {
                f.fx *= clampScale; f.fy *= clampScale; f.fz *= clampScale;
            }
            if (capture) {
                for (const name of AE_FORCE_COMPONENTS) {
                    for (const f of components[name]) {
                        f.fx *= clampScale; f.fy *= clampScale; f.fz *= clampScale;
                    }
                }
            }
        }

        forces.components = components;
        forces.clamped = clampScale < 1.0;
        forces.clampScale = clampScale;
        return forces;
    }

    function aeTick() {
        if (!state._ae) return;
        const atoms = state._ae.atoms;
        const dt = state._ae.dt;
        const tickNum = state._ae.tick;
        const preIssue = _aeStateIssue();
        if (preIssue) {
            _aeReject(`pre-tick state rejected: ${preIssue}`);
            return false;
        }
        const snapshot = _aeDynamicSnapshot();
        _aeEmitNuclearSource();

        let forces = _aeComputeAllForces();
        let forceClamped = forces.clamped;
        let clampScale = forces.clampScale;

        // Debug: log first 3 ticks
        if (tickNum < 3) {
            debugLog(`[FTD aeTick #${tickNum}] dt=${dt} atoms=${atoms.length}`);
            for (let i = 0; i < Math.min(atoms.length, 4); i++) {
                const a = atoms[i], f = forces[i];
                debugLog(`  atom ${a.id}: pos=(${a.x.toFixed(3)},${a.y.toFixed(3)},${a.z.toFixed(3)}) vel=(${a.vx.toFixed(4)},${a.vy.toFixed(4)},${a.vz.toFixed(4)}) force=(${f.fx.toFixed(4)},${f.fy.toFixed(4)},${f.fz.toFixed(4)}) bonds=${a.bonds.length}`);
            }
        }

        // Half-kick
        for (let i = 0; i < atoms.length; i++) {
            const a = atoms[i];
            if (a.locked) continue;
            const hdt = dt * 0.5 / a.mass;
            a.vx += forces[i].fx * hdt;
            a.vy += forces[i].fy * hdt;
            a.vz += forces[i].fz * hdt;
        }

        // Drift
        for (const a of atoms) {
            if (a.locked) continue;
            a.x += a.vx * dt;
            a.y += a.vy * dt;
            a.z += a.vz * dt;
        }

        // Boundary containment (AE mode: origin-centered, radius 35)
        if (state._boundaryShape !== 'cube' && state._boundaryShape !== 'none') {
            for (const a of atoms) {
                if (a.locked) continue;
                state._reflectIntoBoundary(a, 0, 0, 0, 35);
            }
        }

        forces = _aeComputeAllForces();
        forceClamped = forceClamped || forces.clamped;
        clampScale = Math.min(clampScale, forces.clampScale);

        // Half-kick again
        for (let i = 0; i < atoms.length; i++) {
            const a = atoms[i];
            if (a.locked) continue;
            const hdt = dt * 0.5 / a.mass;
            a.vx += forces[i].fx * hdt;
            a.vy += forces[i].fy * hdt;
            a.vz += forces[i].fz * hdt;
        }

        if (tickNum < 3) {
            for (let i = 0; i < Math.min(atoms.length, 4); i++) {
                const a = atoms[i];
                debugLog(`  atom ${a.id} after tick: pos=(${a.x.toFixed(3)},${a.y.toFixed(3)},${a.z.toFixed(3)}) vel=(${a.vx.toFixed(4)},${a.vy.toFixed(4)},${a.vz.toFixed(4)})`);
            }
        }

        // Damping
        if (state._ae.damping) {
            const d = Math.max(0, 1 - 0.02 * dt);
            for (const a of atoms) {
                if (a.locked) continue;
                a.vx *= d; a.vy *= d; a.vz *= d;
            }
        }

        // Berendsen thermostat
        if (state._ae.thermostat && state._ae.thermostat_temp > 0) {
            let ke = 0, n_free = 0;
            for (const a of atoms) {
                if (!a.locked) {
                    ke += 0.5 * a.mass * (a.vx*a.vx + a.vy*a.vy + a.vz*a.vz);
                    n_free++;
                }
            }
            if (n_free > 0) {
                const T_current = 2.0 * ke / (3.0 * n_free);
                if (T_current > 1e-30) {
                    const lambdaSq = Math.max(0, 1.0 + dt / AE_THERMOSTAT_TAU
                        * (state._ae.thermostat_temp / T_current - 1.0));
                    const lam = Math.sqrt(lambdaSq);
                    for (const a of atoms) {
                        if (!a.locked) { a.vx *= lam; a.vy *= lam; a.vz *= lam; }
                    }
                }
            }
        }

        // Apply the causal display-model speed ceiling after every velocity
        // modifier, including the thermostat. Otherwise a hot target can
        // immediately undo the ceiling in the same tick.
        if (state._ae.speed_limit) {
            for (const a of atoms) {
                if (a.locked) continue;
                const speed = Math.hypot(a.vx, a.vy, a.vz);
                if (speed > AE_SPEED_MAX) {
                    const s = AE_SPEED_MAX / speed;
                    a.vx *= s; a.vy *= s; a.vz *= s;
                }
            }
        }

        // Auto-bonding + bond breaking
        if (state._ae.bonding) {
            for (let i = 0; i < atoms.length; i++) {
                for (let j = i + 1; j < atoms.length; j++) {
                    const ai = atoms[i], aj = atoms[j];
                    if (ai.bonds.some(b => b.partner_id === aj.id)) continue;
                    if (ai.bonds.length >= ai.max_bonds || aj.bonds.length >= aj.max_bonds) continue;
                    const dx = aj.x - ai.x, dy = aj.y - ai.y, dz = aj.z - ai.z;
                    const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
                    const sig_avg = (ai.vdw_sigma + aj.vdw_sigma) / 2;
                    // [IMPOSED] auto-bond capture radius 1.2·σ_avg (and the
                    // 0.2·Δχ electronegativity widening) are empirical
                    // visualization tunings, not literature bond criteria.
                    let bond_threshold = 1.2 * sig_avg;
                    if (state._ae.electronegativity) {
                        const chi_diff = Math.abs(ai.electronegativity - aj.electronegativity);
                        bond_threshold *= (1.0 + 0.2 * chi_diff);
                    }
                    if (r < bond_threshold) {
                        const r_eq = sig_avg * Math.pow(2, 1.0 / 6.0);
                        const eps_mix = Math.sqrt(ai.vdw_epsilon * aj.vdw_epsilon);
                        const k_bond = AE_K_BOND * eps_mix / (r_eq * r_eq);
                        ai.bonds.push({ partner_id: aj.id, r_eq, k_bond, order: 1 });
                        aj.bonds.push({ partner_id: ai.id, r_eq, k_bond, order: 1 });
                    }
                }
            }
            // Bond breaking — break only when stretched far beyond equilibrium.
            // [IMPOSED] the 3.5·r_eq break threshold is an empirical stability
            // tuning (prevents flicker re-bonding), not a physical dissociation
            // criterion.
            for (const a of atoms) {
                a.bonds = a.bonds.filter(b => {
                    const jIdx = state._aeIdToIdx.get(b.partner_id);
                    if (jIdx === undefined) return false;
                    const partner = atoms[jIdx];
                    const dx = partner.x - a.x, dy = partner.y - a.y, dz = partner.z - a.z;
                    const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
                    return r <= 3.5 * b.r_eq;
                });
            }
            // Re-infer bond orders after connectivity may have changed this
            // tick (audit P0-12/P0-13). Idempotent — resets to single first.
            _aeInferBondOrders();
        }

        // Nuclear reaction channels are separate from the molecular force
        // field. Exact isotope pairs become eligible through their current or
        // swept trajectories, then a deterministic-seed [PARAMETRIC] hazard
        // decides whether a momentum/Q-closed reaction occurs. Scenario IDs
        // never participate in this tick path.
        const previousById = new Map(snapshot.atoms.map(atom => [atom.id, atom]));
        _aeProcessNuclearReaction(tickNum + 1, previousById);

        const postIssue = _aeStateIssue();
        if (postIssue) {
            _aeRestoreDynamic(snapshot);
            _aeReject(`tick rolled back: ${postIssue}`);
            return false;
        }
        state._ae.force_clamped_last = forceClamped;
        state._ae.force_clamp_scale = clampScale;
        if (forceClamped) state._ae.force_clamp_events++;
        state._ae.last_error = '';
        state._ae.tick++;
        return true;
    }

    function aeGetAtomData() {
        if (!state._ae) return { positions: new Float32Array(0), colors: new Float32Array(0), sizes: new Float32Array(0), atomicNums: new Int32Array(0), neutronCounts: new Int32Array(0), charges: new Float32Array(0), ids: new Int32Array(0), bonds: new Int32Array(0), bondOrders: new Float32Array(0), bondCount: 0, count: 0 };
        const atoms = state._ae.atoms;
        const count = atoms.length;
        const positions = new Float32Array(count * 3);
        const colors = new Float32Array(count * 3);
        const sizes = new Float32Array(count);
        const atomicNums = new Int32Array(count);
        const neutronCounts = new Int32Array(count);
        const charges = new Float32Array(count);
        const ids = new Int32Array(count);

        let bondCount = 0;
        for (const a of atoms) {
            for (const b of a.bonds) {
                if (b.partner_id > a.id) bondCount++;
            }
        }
        const bonds = new Int32Array(bondCount * 2);
        // Float (not Int) so the aromatic sentinel order 1.5 survives — the
        // renderer distinguishes 1.5 ≤ order < 2 as aromatic (audit P0-13).
        const bondOrders = new Float32Array(bondCount);

        for (let i = 0; i < count; i++) {
            const a = atoms[i];
            positions[i * 3] = a.x;
            positions[i * 3 + 1] = a.y;
            positions[i * 3 + 2] = a.z;
            const [cr, cg, cb] = cpkColor(a.Z);
            colors[i * 3] = cr; colors[i * 3 + 1] = cg; colors[i * 3 + 2] = cb;
            sizes[i] = 6.0 + a.radius * 10.0;
            if (sizes[i] > 60) sizes[i] = 60;
            atomicNums[i] = a.Z;
            neutronCounts[i] = a.N;
            charges[i] = a.q_frac;
            ids[i] = a.id;
        }

        let bi = 0;
        for (const a of atoms) {
            for (const b of a.bonds) {
                if (b.partner_id > a.id) {
                    bonds[bi * 2] = a.id;
                    bonds[bi * 2 + 1] = b.partner_id;
                    bondOrders[bi] = b.order || 1;
                    bi++;
                }
            }
        }

        return { positions, colors, sizes, atomicNums, neutronCounts, charges, ids, bonds, bondOrders, bondCount, count };
    }

    function aeGetFieldSources() {
        if (!state._ae) return { positions: new Float32Array(0), charges: new Float32Array(0), count: 0 };
        const atoms = state._ae.atoms;
        const n = atoms.length;
        const positions = new Float32Array(n * 3);
        const charges = new Float32Array(n);
        for (let i = 0; i < n; i++) {
            positions[i * 3] = atoms[i].x;
            positions[i * 3 + 1] = atoms[i].y;
            positions[i * 3 + 2] = atoms[i].z;
            charges[i] = atoms[i].q_frac;
        }
        return { positions, charges, count: n };
    }

    function aeGetDiagnostics() {
        if (!state._ae) return { tick: 0, atomCount: 0, bondCount: 0, totalKE: 0, totalPEIonic: 0, totalPEVdw: 0, totalPEBond: 0, totalPEAngle: 0, totalEnergy: 0, momentumX: 0, momentumY: 0, momentumZ: 0, temperature: 0, energyComplete: true, energyConservative: false, energyStatus: 'complete-driven', forceClamped: false, forceClampScale: 1, forceClampEvents: 0, nuclear: null, lastError: '' };
        const atoms = state._ae.atoms;
        let ke = 0, pe_ionic = 0, pe_vdw = 0, pe_bond = 0, pe_angle = 0;
        let freeKE = 0, freeAtoms = 0;
        let px = 0, py = 0, pz = 0;
        const soft2 = state._ae.soft * state._ae.soft;

        for (const a of atoms) {
            const v2 = a.vx * a.vx + a.vy * a.vy + a.vz * a.vz;
            const atomKE = 0.5 * a.mass * v2;
            ke += atomKE;
            px += a.mass * a.vx; py += a.mass * a.vy; pz += a.mass * a.vz;
            if (!a.locked) {
                freeKE += atomKE;
                freeAtoms++;
            }
        }

        _aeBuildBondLookup();
        _aeComputeDipoleMoments(true);
        for (let i = 0; i < atoms.length; i++) {
            for (let j = i + 1; j < atoms.length; j++) {
                const ai = atoms[i], aj = atoms[j];
                const dx = aj.x - ai.x, dy = aj.y - ai.y, dz = aj.z - ai.z;
                const r = Math.sqrt(dx * dx + dy * dy + dz * dz + soft2);
                const isBonded = _aeIsBonded(ai.id, aj.id);
                const is13 = !isBonded && _aeIs13(i, j);
                if (state._ae.ionic && !isBonded && !is13 &&
                    (Math.abs(ai.q_frac) > 1e-6 || Math.abs(aj.q_frac) > 1e-6)) {
                    pe_ionic += AE_K_COULOMB * ai.q_frac * aj.q_frac / r;
                }
                if (state._ae.vdw && !isBonded && !is13) {
                    const eps_mix = Math.sqrt(ai.vdw_epsilon * aj.vdw_epsilon);
                    const sig_mix = (ai.vdw_sigma + aj.vdw_sigma) / 2;
                    const sr = sig_mix / r; const sr6 = sr ** 6; const sr12 = sr6 * sr6;
                    pe_vdw += 4.0 * eps_mix * (sr12 - sr6);
                }
            }
        }

        // Bond PE — build id→atom map ONCE (F-10) so the partner lookup in the
        // bond loop is O(1) instead of an O(N) Array.find per bond (was O(N²)).
        const idToAtom = new Map();
        for (let i = 0; i < atoms.length; i++) idToAtom.set(atoms[i].id, atoms[i]);
        if (state._ae.bonds_force) {
            const counted = new Set();
            for (const a of atoms) {
                for (const b of a.bonds) {
                    const key = Math.min(a.id, b.partner_id) + ',' + Math.max(a.id, b.partner_id);
                    if (counted.has(key)) continue;
                    counted.add(key);
                    const partner = idToAtom.get(b.partner_id);
                    if (!partner) continue;
                    const dx = partner.x - a.x, dy = partner.y - a.y, dz = partner.z - a.z;
                    const r = Math.sqrt(dx * dx + dy * dy + dz * dz + soft2);
                    const dr = r - b.r_eq;
                    pe_bond += 0.5 * b.k_bond * dr * dr;
                }
            }
        }

        if (state._ae.angle_strain) {
            for (let i = 0; i < atoms.length; i++) {
                const center = atoms[i];
                if (center.bonds.length < 2) continue;
                const thetaEq = _aeEquilibriumAngle(center);
                for (let b1 = 0; b1 < center.bonds.length; b1++) {
                    for (let b2 = b1 + 1; b2 < center.bonds.length; b2++) {
                        const j1 = state._aeIdToIdx.get(center.bonds[b1].partner_id);
                        const j2 = state._aeIdToIdx.get(center.bonds[b2].partner_id);
                        if (j1 === undefined || j2 === undefined) continue;
                        const a1 = atoms[j1], a2 = atoms[j2];
                        const r1x = a1.x-center.x, r1y = a1.y-center.y, r1z = a1.z-center.z;
                        const r2x = a2.x-center.x, r2y = a2.y-center.y, r2z = a2.z-center.z;
                        const m1 = Math.hypot(r1x, r1y, r1z), m2 = Math.hypot(r2x, r2y, r2z);
                        if (m1 < 1e-30 || m2 < 1e-30) continue;
                        const cosTheta = Math.max(-1, Math.min(1,
                            (r1x*r2x + r1y*r2y + r1z*r2z) / (m1*m2)));
                        const dTheta = Math.acos(cosTheta) - thetaEq;
                        pe_angle += 0.5 * AE_K_ANGLE * dTheta * dTheta;
                    }
                }
            }
        }

        let bondCount = 0;
        for (const a of atoms) {
            for (const b of a.bonds) { if (b.partner_id > a.id) bondCount++; }
        }

        // Equipartition proxy in SIM UNITS (implicit k_B = 1), NOT kelvin.
        // No Boltzmann conversion is applied — this is the bare 2⟨KE⟩/(3N)
        // statistic. The UI relabels it "(sim)" (audit P0-10); do not append
        // a "K" suffix or treat this as an SI temperature downstream.
        const T = freeAtoms > 0 ? 2.0 * freeKE / (3.0 * freeAtoms) : 0;
        // H-bond and induced-dipole kernels intentionally omit terms from the
        // full coordinate gradient, so no complete scalar potential is claimed
        // while either is active. Angle strain is now tracked exactly.
        const energyComplete = !(state._ae.h_bonds || state._ae.dipole_dipole);
        const nuclearActive = !!state._ae.nuclear?.channel;
        const energyConservative = energyComplete && !nuclearActive && !(state._ae.damping || state._ae.thermostat ||
            state._ae.bonding || state._ae.speed_limit || state._ae.electronegativity ||
            state._ae.force_clamped_last);
        const energyStatus = nuclearActive
            ? `reaction-${state._ae.nuclear.phase}`
            : !energyComplete
            ? 'partial-untracked-potential'
            : (energyConservative ? 'complete-conservative' : 'complete-driven');

        return {
            tick: state._ae.tick, atomCount: atoms.length, bondCount,
            totalKE: ke, totalPEIonic: pe_ionic, totalPEVdw: pe_vdw, totalPEBond: pe_bond,
            totalPEAngle: pe_angle,
            totalEnergy: ke + pe_ionic + pe_vdw + pe_bond + pe_angle,
            momentumX: px, momentumY: py, momentumZ: pz, temperature: T,
            energyComplete, energyConservative, energyStatus,
            forceClamped: state._ae.force_clamped_last,
            forceClampScale: state._ae.force_clamp_scale,
            forceClampEvents: state._ae.force_clamp_events,
            nuclear: aeGetNuclearDiagnostics(),
            lastError: state._ae.last_error || 'ok',
        };
    }

    function aeGetNuclearDiagnostics() {
        if (!state._ae?.nuclear?.channel) return null;
        const nuclear = state._ae.nuclear;
        const channel = getNuclearReactionChannel(nuclear.channel);
        const event = nuclear.last_event;
        const transportResidual = nuclear.released_mev - (nuclear.deposited_mev +
            nuclear.in_transit_mev + nuclear.escaped_mev);
        return {
            channel: nuclear.channel,
            label: channel?.label || nuclear.channel,
            kind: channel?.kind || '',
            phase: nuclear.phase,
            mode: nuclear.mode,
            eventCount: nuclear.event_count,
            representedEventCount: nuclear.represented_event_count,
            eventTick: nuclear.event_tick,
            eventWeight: nuclear.event_weight,
            generation: nuclear.generation,
            kEffective: nuclear.k_effective,
            reactivityScale: nuclear.reactivity_scale,
            collisionRadiusScale: nuclear.collision_radius_scale,
            transportRadius: nuclear.transport_radius,
            boundaryMode: nuclear.boundary_mode,
            moderatorStrength: nuclear.moderator_strength,
            absorberStrength: nuclear.absorber_strength,
            sourceEnabled: nuclear.source_enabled,
            sourceRate: nuclear.source_rate,
            sourceEnergyMeV: nuclear.source_energy_mev,
            particleLimit: nuclear.particle_limit,
            sourceSaturated: nuclear.source_saturated,
            neutronContainment: nuclear.neutron_containment,
            gammaContainment: nuclear.gamma_containment,
            fuelInitial: nuclear.fuel_initial,
            fuelRemaining: nuclear.fuel_remaining,
            liveNeutrons: state._ae.atoms.filter(atom => atom.Z === 0 && atom.N === 1).length,
            eventRatePer100Ticks: nuclear.events_per_100_ticks || 0,
            leakedNeutrons: nuclear.leaked_neutrons,
            absorbedNeutrons: nuclear.absorbed_neutrons,
            scatteredNeutrons: nuclear.scattered_neutrons,
            sourceNeutrons: nuclear.source_neutrons,
            fissionNeutronBirths: nuclear.fission_neutron_births,
            neutronFissionLosses: nuclear.neutron_fission_losses,
            qMeV: channel?.qMeV || 0,
            incidentEnergyMeV: channel?.incidentEnergyMeV || 0,
            kineticReleaseMeV: channel?.energyBudget?.kineticMeV || 0,
            recoverablePerEventMeV: channel?.energyBudget?.totalRecoverableMeV || 0,
            microscopicReleasedMeV: nuclear.microscopic_released_mev,
            releasedMeV: nuclear.released_mev,
            releasedJoule: nuclear.released_joule,
            depositedMeV: nuclear.deposited_mev,
            inTransitMeV: nuclear.in_transit_mev,
            escapedMeV: nuclear.escaped_mev,
            kineticMeV: nuclear.kinetic_mev,
            chargedMeV: nuclear.charged_mev,
            neutronMeV: nuclear.neutron_mev,
            promptGammaMeV: nuclear.prompt_gamma_mev,
            delayedHeatMeV: nuclear.delayed_heat_mev,
            protonResidual: event?.protonResidual || 0,
            chargeResidual: event?.chargeResidual || 0,
            neutronResidual: event?.neutronResidual || 0,
            momentumResidual: event?.momentumResidual || 0,
            energyResidualMeV: event?.energyResidualMeV || 0,
            totalLedgerResidualMeV: event?.totalLedgerResidualMeV || 0,
            transportResidualMeV: transportResidual,
            transportResidualFraction: transportResidual / Math.max(Math.abs(nuclear.released_mev), 1),
            kineticBeforeSim: event?.kineticBeforeSim || 0,
            kineticAfterSim: event?.kineticAfterSim || 0,
            collisionEnergyMeV: event?.collisionEnergyMeV || 0,
            reactionProbability: event?.reactionProbability || 0,
            source: channel?.source || '',
        };
    }

    function aeGetNuclearVisuals() {
        if (!state._ae?.nuclear?.channel) return null;
        const nuclear = state._ae.nuclear;
        const tick = state._ae.tick;
        return {
            tick,
            mode: nuclear.mode,
            phase: nuclear.phase,
            transportRadius: nuclear.transport_radius,
            boundaryMode: nuclear.boundary_mode,
            effects: nuclear.effects.map((event) => {
                const transport = _aeNuclearTransportForEvent(event, tick, nuclear);
                return {
                    ...event,
                    neutronDirections: event.neutronDirections?.map(direction => ({ ...direction })) || [],
                    simAge: Math.max(0, tick - event.tick),
                    depositedMeV: transport.deposited,
                    depositedFraction: transport.deposited / Math.max(event.totalMeV, 1e-12),
                };
            }),
            // Neutrons are ordinary live particles now; no target-assignment
            // flight overlay exists. The renderer obtains their positions from
            // aeGetAtomData like every other particle.
            flights: [],
            depositedMeV: nuclear.deposited_mev,
            escapedMeV: nuclear.escaped_mev,
            inTransitMeV: nuclear.in_transit_mev,
        };
    }

    /**
     * Exact decomposition of the force field used by the integrator. `net` is
     * the actual post-safety force, including H-bond, angle and dipole terms;
     * it is no longer a three-channel visual approximation.
     */
    function aeGetForceDecomposition(want) {
        const empty = () => new Float32Array(0);
        if (!state._ae) return {
            ionic: empty(), vdw: empty(), bond: empty(), hbond: empty(),
            angle: empty(), dipole: empty(), net: empty(), count: 0,
            clamped: false, clampScale: 1,
        };
        const forces = _aeComputeAllForces(true);
        const n = state._ae.atoms.length;
        const result = { count: n, clamped: forces.clamped, clampScale: forces.clampScale };
        const wantNet = !want || !!want.net;
        for (const name of AE_FORCE_COMPONENTS) {
            const include = !want || wantNet || !!want[name];
            const out = new Float32Array(n * 3);
            if (include) {
                for (let i = 0; i < n; i++) {
                    const f = forces.components[name][i];
                    out[i*3] = f.fx; out[i*3+1] = f.fy; out[i*3+2] = f.fz;
                }
            }
            result[name] = out;
        }
        const net = new Float32Array(n * 3);
        if (wantNet) {
            for (let i = 0; i < n; i++) {
                net[i*3] = forces[i].fx;
                net[i*3+1] = forces[i].fy;
                net[i*3+2] = forces[i].fz;
            }
        }
        result.net = net;
        return result;
    }

    function aeSetDt(dt)              { return _aeSetBounded('dt', Number(dt)); }
    function aeGetDt()                 { return state._ae ? state._ae.dt : 0.01; }
    function aeSetSoftening(s)        { return _aeSetBounded('soft', Number(s)); }
    function aeSetDamping(e)          { if (!state._ae) return false; state._ae.damping = !!e; return true; }
    function aeSetBonding(e)          { if (!state._ae) return false; state._ae.bonding = !!e; return true; }
    function aeSetIonic(e)            { if (!state._ae) return false; state._ae.ionic = !!e; return true; }
    function aeSetVdw(e)              { if (!state._ae) return false; state._ae.vdw = !!e; return true; }
    function aeSetBondsForce(e)       { if (!state._ae) return false; state._ae.bonds_force = !!e; return true; }
    function aeSetSpeedLimit(e)       { if (!state._ae) return false; state._ae.speed_limit = !!e; return true; }
    function aeSetHBonds(e)           { if (!state._ae) return false; state._ae.h_bonds = !!e; return true; }
    function aeSetAngleStrain(e)      { if (!state._ae) return false; state._ae.angle_strain = !!e; return true; }
    function aeSetDipoleDipole(e)     { if (!state._ae) return false; state._ae.dipole_dipole = !!e; return true; }
    function aeSetThermostat(e)       { if (!state._ae) return false; state._ae.thermostat = !!e; return true; }
    function aeSetThermostatTemp(t)   { return _aeSetBounded('thermostat_temp', Number(t)); }
    function aeSetElectronegativity(e){ if (!state._ae) return false; state._ae.electronegativity = !!e; return true; }
    function aeAtomCount()            { return state._ae ? state._ae.atoms.length : 0; }
    function aeClear()                { resetAE(); }

    /**
     * Per-atom velocities for the velocity-vector overlay. Positions are
     * already in every aeGetAtomData() frame, so only velocities ship here.
     */
    function aeGetVelocities() {
        if (!state._ae) return { velocities: new Float32Array(0), count: 0 };
        const atoms = state._ae.atoms;
        const velocities = new Float32Array(atoms.length * 3);
        for (let i = 0; i < atoms.length; i++) {
            velocities[i * 3]     = atoms[i].vx;
            velocities[i * 3 + 1] = atoms[i].vy;
            velocities[i * 3 + 2] = atoms[i].vz;
        }
        return { velocities, count: atoms.length };
    }

    /**
     * Per-atom dipole moments for the dipole-arrow overlay. Recomputes the
     * bond-χ-difference dipoles directly so arrows work even when the
     * dipole-dipole FORCE toggle is off (the force path only refreshes
     * them when enabled).
     */
    function aeGetDipoles() {
        if (!state._ae) return { dipoles: new Float32Array(0), count: 0 };
        _aeBuildBondLookup();
        _aeComputeDipoleMoments(false);
        const atoms = state._ae.atoms;
        const dipoles = new Float32Array(atoms.length * 3);
        for (let i = 0; i < atoms.length; i++) {
            dipoles[i * 3]     = atoms[i].dipole_x || 0;
            dipoles[i * 3 + 1] = atoms[i].dipole_y || 0;
            dipoles[i * 3 + 2] = atoms[i].dipole_z || 0;
        }
        return { dipoles, count: atoms.length };
    }

    /**
     * Donor-H···acceptor pairs for the dashed H-bond line overlay.
     * ELIGIBILITY mirrors hbondForce exactly: H (Z=1) covalently bonded to
     * an electronegative donor (N/O/F), candidate acceptor electronegative,
     * not the donor itself, not covalently bonded to the H.
     * [VISUALIZATION] display gates on top (the force itself has no hard
     * cutoff): H···A range ≤ 4.0·σ_hb and cos²θ_DHA ≥ 0.25 with the
     * D→H · H→A alignment positive — bounds drawing to geometrically
     * meaningful H-bonds. 4.0·σ_hb covers the ae-water-dimer SEED
     * separation (10.6 at σ_hb = 3) so the forming bond is annotated from
     * tick 0; the angular gate excludes the away-facing H (15.2, cos<0.5).
     * @returns {{segments: Float32Array, count: number}} count line
     *          segments, 6 floats each (hx,hy,hz, ax,ay,az).
     */
    function aeGetHBondPairs() {
        if (!state._ae) return { segments: new Float32Array(0), count: 0 };
        _aeBuildBondLookup();
        const atoms = state._ae.atoms;
        const isElecNeg = (Z) => Z === 7 || Z === 8 || Z === 9;
        const segs = [];
        for (let i = 0; i < atoms.length; i++) {
            const h = atoms[i];
            if (h.Z !== 1) continue;
            let donorIdx = -1;
            for (const b of h.bonds) {
                const didx = state._aeIdToIdx.get(b.partner_id);
                if (didx !== undefined && isElecNeg(atoms[didx].Z)) { donorIdx = didx; break; }
            }
            if (donorIdx < 0) continue;
            const donor = atoms[donorIdx];
            for (let j = 0; j < atoms.length; j++) {
                if (j === i || j === donorIdx) continue;
                const a = atoms[j];
                if (!isElecNeg(a.Z)) continue;
                if (_aeIsBonded(h.id, a.id)) continue;
                const dx = a.x - h.x, dy = a.y - h.y, dz = a.z - h.z;
                const r = Math.sqrt(dx * dx + dy * dy + dz * dz);
                const sig_hb = (h.vdw_sigma + a.vdw_sigma) / 2;
                if (!(r > 1e-10) || sig_hb <= 0) continue;
                if (r > 4.0 * sig_hb) continue;
                const dhx = h.x - donor.x, dhy = h.y - donor.y, dhz = h.z - donor.z;
                const dh_mag = Math.sqrt(dhx * dhx + dhy * dhy + dhz * dhz);
                if (dh_mag < 1e-30) continue;
                const cos_theta = (dhx * dx + dhy * dy + dhz * dz) / (dh_mag * r);
                if (cos_theta <= 0 || cos_theta * cos_theta < 0.25) continue;
                segs.push(h.x, h.y, h.z, a.x, a.y, a.z);
            }
        }
        return { segments: new Float32Array(segs), count: segs.length / 6 };
    }

    /**
     * Snapshot of the live AE runtime parameters + physics toggle states.
     * Read by telemetryHub.collectScale2 so the diagnostics panel reports
     * engine truth (not DOM checkbox state). Mirrors peGetToggle's role on
     * Scale 1, returned as one object because AE toggles are only ever
     * consumed together.
     */
    function aeGetRuntimeState() {
        if (!state._ae) return null;
        const ae = state._ae;
        return {
            dt: ae.dt,
            softening: ae.soft,
            thermostatTemp: ae.thermostat_temp,
            forceClamped: ae.force_clamped_last,
            forceClampScale: ae.force_clamp_scale,
            forceClampEvents: ae.force_clamp_events,
            lastError: ae.last_error,
            nuclear: aeGetNuclearDiagnostics(),
            toggles: {
                ionic: !!ae.ionic,
                vdw: !!ae.vdw,
                bonds_force: !!ae.bonds_force,
                bonding: !!ae.bonding,
                damping: !!ae.damping,
                speed_limit: !!ae.speed_limit,
                h_bonds: !!ae.h_bonds,
                angle_strain: !!ae.angle_strain,
                dipole_dipole: !!ae.dipole_dipole,
                thermostat: !!ae.thermostat,
                electronegativity: !!ae.electronegativity,
            },
        };
    }

    function aeInspectAtom(id) {
        if (!state._ae) return null;
        const a = state._ae.atoms.find(at => at.id === id);
        if (!a) return null;
        const mass = a.Z + a.N * NEUTRON_PROTON_MASS_RATIO;  // proton-mass units (PDG ratio)
        const speed = Math.sqrt(a.vx * a.vx + a.vy * a.vy + a.vz * a.vz);
        const ke = 0.5 * mass * speed * speed;

        const bondInfo = a.bonds.map(b => {
            const p = state._ae.atoms.find(at => at.id === b.partner_id);
            if (!p) return null;
            const dx = p.x - a.x, dy = p.y - a.y, dz = p.z - a.z;
            const dist = Math.sqrt(dx * dx + dy * dy + dz * dz);
            return { partnerId: b.partner_id, partnerZ: p.Z, dist, r_eq: b.r_eq, order: b.order };
        }).filter(Boolean);

        let nearestId = -1, nearestDist = Infinity, nearestZ = 0;
        const bondSet = new Set(a.bonds.map(b => b.partner_id));
        for (const other of state._ae.atoms) {
            if (other.id === id || bondSet.has(other.id)) continue;
            const dx = other.x - a.x, dy = other.y - a.y, dz = other.z - a.z;
            const d = Math.sqrt(dx * dx + dy * dy + dz * dz);
            if (d < nearestDist) { nearestDist = d; nearestId = other.id; nearestZ = other.Z; }
        }

        // Net force magnitude — must rebuild bond lookups first.
        _aeBuildBondLookup();
        const idx = state._ae.atoms.indexOf(a);
        const forces = _aeComputeAllForces(false);
        const f = forces[idx];
        const fNetMag = Math.sqrt(f.fx * f.fx + f.fy * f.fy + f.fz * f.fz);

        // Bond-connected observer component. This is derived from the live
        // topology for inspection only and never feeds back into dynamics.
        const byId = new Map(state._ae.atoms.map(atom => [atom.id, atom]));
        const memberIds = [];
        const pending = [a.id];
        const visited = new Set();
        while (pending.length > 0) {
            const currentId = pending.pop();
            if (visited.has(currentId)) continue;
            const current = byId.get(currentId);
            if (!current) continue;
            visited.add(currentId);
            memberIds.push(currentId);
            for (const bond of current.bonds) {
                if (!visited.has(bond.partner_id)) pending.push(bond.partner_id);
            }
        }
        memberIds.sort((lhs, rhs) => lhs - rhs);
        let componentMass = 0;
        let componentCharge = 0;
        let componentKE = 0;
        let centerX = 0, centerY = 0, centerZ = 0;
        for (const memberId of memberIds) {
            const atom = byId.get(memberId);
            const atomMass = atom.Z + atom.N * NEUTRON_PROTON_MASS_RATIO;
            const atomSpeed2 = atom.vx * atom.vx + atom.vy * atom.vy + atom.vz * atom.vz;
            componentMass += atomMass;
            componentCharge += atom.q_frac;
            componentKE += 0.5 * atomMass * atomSpeed2;
            centerX += atomMass * atom.x;
            centerY += atomMass * atom.y;
            centerZ += atomMass * atom.z;
        }
        if (componentMass > 0) {
            centerX /= componentMass;
            centerY /= componentMass;
            centerZ /= componentMass;
        }

        return {
            id, Z: a.Z, N: a.N, charge: a.q_frac, mass, radius: a.radius,
            locked: a.locked, sigma: a.vdw_sigma, epsilon: a.vdw_epsilon,
            maxBonds: a.max_bonds,
            x: a.x, y: a.y, z: a.z,
            vx: a.vx, vy: a.vy, vz: a.vz,
            speed, ke, bonds: bondInfo,
            nearestId, nearestDist, nearestZ, fNetMag,
            alpha_pol: a.alpha_pol, e_ion: a.e_ion, e_aff: a.e_aff,
            sigma_scatter: a.sigma_scatter, z_eff: a.z_eff,
            component: {
                count: memberIds.length,
                members: memberIds,
                centerX, centerY, centerZ,
                mass: componentMass,
                charge: componentCharge,
                ke: componentKE,
            },
        };
    }

    return {
        initAE, resetAE,
        aeAddAtom, aeAddLockedAtom, aeCreateBond,
        _aeBuildBondLookup, _aeIsBonded, _aeIs13,
        _aeComputeDipoleMoments, _aeComputeForce, _aeComputeAllForces,
        aePreBond, aeTick,
        aeGetAtomData, aeGetFieldSources, aeGetDiagnostics, aeGetForceDecomposition,
        aeSetDt, aeGetDt, aeSetSoftening, aeSetDamping, aeSetBonding,
        aeSetIonic, aeSetVdw, aeSetBondsForce, aeSetSpeedLimit,
        aeSetHBonds, aeSetAngleStrain, aeSetDipoleDipole,
        aeSetThermostat, aeSetThermostatTemp, aeSetElectronegativity,
        aeAtomCount, aeClear, aeInspectAtom, aeGetRuntimeState,
        aeConfigureNuclearReaction, aeSetNuclearEnvironment, aeInjectNuclearParticle,
        aeGetNuclearDiagnostics, aeGetNuclearVisuals,
        aeGetVelocities, aeGetDipoles, aeGetHBondPairs,
    };
}

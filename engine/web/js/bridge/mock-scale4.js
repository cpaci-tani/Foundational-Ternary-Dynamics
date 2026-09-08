/**
 * SolarSystemBridge — JS-only N-body simulation for Solar System scale (Scale 4).
 *
 * Architecture follows an effective celestial-mechanics force stack:
 *   1. direct Newton gravity
 *   2. toggle-gated 1PN, J2, tides, radiation/drag, and atmosphere kernels
 *   3. finite-body collision and Roche-event resolution
 *   4. split-kick Velocity-Verlet integration
 *
 * Unit system (positions/masses are heliocentric in both gravity modes):
 * - Distance: Astronomical Units (AU)
 * - Mass: Solar Masses (M_sun)
 * - Time: Earth Years (yr)
 * - Velocity: AU / yr
 *
 * Gravitational-constant mode:
 * - 'physical' (DEFAULT): G = G_HELIOCENTRIC = 4π² ≈ 39.478. Earth's
 *   circular orbit at a=1 AU has the correct period T = 1 yr; orbits are
 *   period-faithful in the adopted Newtonian model.
 * - 'presentation': G = G_N = 0.01. This retained comparison mode is a slow
 *   visual gauge and explicitly not AU/M_sun/yr-faithful.
 * The figure-8 three-body scenario always overrides to G=1 regardless of mode
 * (Chenciner–Montgomery natural units).
 */

import { EXOPLANET_SYSTEMS } from '../config/exoplanet-seeds.js?v=2';
import { G_HELIOCENTRIC, G_N } from '../constants.js';
import { getScale4Scenario, SCALE4_DEFAULT_SCENARIO } from '../scales/scale4/scenario-registry.js?v=1';
import {
    DAYS_PER_YEAR,
    KM_PER_AU,
    SOLAR_SYSTEM_DWARFS,
    SOLAR_SYSTEM_MOONS,
    SOLAR_SYSTEM_PLANETS,
    SOLAR_SYSTEM_PROVENANCE,
    SUN_REFERENCE,
} from '../config/solar-system-data.js?v=4';
import {
    DEFAULT_SOLAR_MASS_LOSS_PER_YEAR,
    DEFAULT_SOLAR_PHYSICS,
    SOLAR_MASS_KG,
    SOLAR_PHYSICS_DEFINITIONS,
    SOLAR_WIND_TO_PR_DRAG,
    atmosphereDragAcceleration,
    equilibriumTideAcceleration,
    fluidRocheLimitAu,
    j2Acceleration,
    radiationAcceleration,
    radiationBeta,
    schwarzschild1PNAcceleration,
    siderealSpinRateRadYr,
    signedSiderealPeriodDays,
    spinAxis,
} from '../config/solar-system-physics.js?v=4';

// Gravity-constant presets for the Scale-4 toggle. Both are named exports of
// constants.js; do not inline the numbers.
const G_BY_MODE = {
    physical:   G_HELIOCENTRIC, // 4π²  — Keplerian AU/M_sun/yr, period-faithful
    presentation: G_N,          // 0.01 — retained slow visual comparison
    decorative: G_N,            // backwards-compatible alias
};
const DEFAULT_GRAVITY_MODE = 'physical';
const SECONDS_PER_DAY = 86_400;
const DEFAULT_MAX_INTEGRATION_STEP_YEARS = 0.01 / DAYS_PER_YEAR;
const ROCHE_FRAGMENT_COLLISION_COOLDOWN_YEARS = 5 / DAYS_PER_YEAR;

const BINARY_SYSTEM_PROVENANCE = Object.freeze({
    epoch: 'Designed experiment t=0',
    orbitalSource: 'Imposed equal-mass circular binary plus circumbinary test orbit',
    dynamics: 'Newtonian N-body plus applicable toggle-gated effective perturbations, split-kick Verlet',
    perturbationSource: 'Standard J2, tide, radiation/drag, collision, and Roche approximations; full binary-star EIH 1PN is not implemented',
    epistemic: '[IMPOSED] designed comparison state; not an FTD derivation',
});

const FIGURE_EIGHT_PROVENANCE = Object.freeze({
    epoch: 'Dimensionless experiment t=0',
    orbitalSource: 'Chenciner-Montgomery figure-eight initial data',
    dynamics: 'Newtonian three-body dynamics in G=1 natural units, split-kick Verlet',
    perturbationSource: 'No applicable single-parent perturbation kernels in this natural-unit state',
    epistemic: '[PARAMETRIC] imported reference solution; not an FTD derivation',
});

export const PLANETARY_TIME_STEPS = Object.freeze({
    minute: Object.freeze({ key: 'minute', label: '1 minute', seconds: 60, years: 1 / (DAYS_PER_YEAR * 24 * 60) }),
    hour: Object.freeze({ key: 'hour', label: '1 hour', seconds: 3_600, years: 1 / (DAYS_PER_YEAR * 24) }),
    day: Object.freeze({ key: 'day', label: '1 day', seconds: SECONDS_PER_DAY, years: 1 / DAYS_PER_YEAR }),
});
export const DEFAULT_PLANETARY_TIME_STEP = 'minute';

const DEG = Math.PI / 180;
const MAX_EVENT_LOG = 120;
const MAX_DYNAMIC_BODIES = 256;
const TAU = 2 * Math.PI;

function wrapRotationPhase(phase) {
    if (!Number.isFinite(phase)) return 0;
    const wrapped = phase % TAU;
    return Object.is(wrapped, -0) ? 0 : wrapped;
}

function vectorMagnitude(v) {
    return Math.hypot(v.x || 0, v.y || 0, v.z || 0);
}

function freshForceAudit() {
    return {
        newtonianAcceleration: 0,
        relativityAcceleration: 0,
        j2Acceleration: 0,
        radiationAcceleration: 0,
        radiationPressureAcceleration: 0,
        poyntingRobertsonDragAcceleration: 0,
        solarWindDragAcceleration: 0,
        tideAcceleration: 0,
        atmosphereAcceleration: 0,
        instantaneousDissipationPower: 0,
    };
}

function solveEccentricAnomaly(meanAnomaly, eccentricity) {
    let E = meanAnomaly;
    for (let i = 0; i < 12; i++) {
        const f = E - eccentricity * Math.sin(E) - meanAnomaly;
        const fp = 1 - eccentricity * Math.cos(E);
        E -= f / fp;
    }
    return E;
}

/** Convert classical elements into a Cartesian state in the reference plane. */
export function orbitalElementsToState(orbit, centralMass = 1, bodyMass = 0, G = G_HELIOCENTRIC) {
    const a = orbit.a;
    const e = orbit.e || 0;
    const inc = (orbit.i || 0) * DEG;
    const node = (orbit.longNode || 0) * DEG;
    const peri = (orbit.longPeri || 0) * DEG;
    const mean = ((orbit.L || 0) - (orbit.longPeri || 0)) * DEG;
    const E = solveEccentricAnomaly(mean, e);
    const root = Math.sqrt(1 - e * e);
    const xp = a * (Math.cos(E) - e);
    const yp = a * root * Math.sin(E);
    const meanMotion = Math.sqrt(G * (centralMass + bodyMass) / (a * a * a));
    const dEdt = meanMotion / (1 - e * Math.cos(E));
    const vxp = -a * Math.sin(E) * dEdt;
    const vyp = a * root * Math.cos(E) * dEdt;

    const omega = peri - node;
    const cO = Math.cos(node), sO = Math.sin(node);
    const co = Math.cos(omega), so = Math.sin(omega);
    const ci = Math.cos(inc), si = Math.sin(inc);
    const A = cO * co - sO * so * ci;
    const B = -cO * so - sO * co * ci;
    const C = sO * co + cO * so * ci;
    const D = -sO * so + cO * co * ci;
    const E1 = so * si;
    const F = co * si;
    return {
        x: A * xp + B * yp, y: C * xp + D * yp, z: E1 * xp + F * yp,
        vx: A * vxp + B * vyp, vy: C * vxp + D * vyp, vz: E1 * vxp + F * vyp,
    };
}

export class PlanetaryMockBridge {
    constructor() {
        this._bodies = [];
        this._tick = 0;
        this._timeYears = 0;
        this._nextId = 0;
        // A public Scale-4 tick is a user-selected interval and defaults to one
        // minute. The Verlet solver can subdivide that interval without
        // changing the tick counter or simulated elapsed time contract.
        this._timeStepKey = DEFAULT_PLANETARY_TIME_STEP;
        this._timeStepYears = PLANETARY_TIME_STEPS[this._timeStepKey].years;
        this._maxIntegrationStepYears = DEFAULT_MAX_INTEGRATION_STEP_YEARS;
        this._dt = this._timeStepYears;
        this._integrationSubsteps = 1;
        this.softeningSq = 1e-18;

        // Physical AU/M_sun/yr dynamics are the default. Presentation mode is
        // retained only as an explicitly non-physical comparison.
        this._gravityMode = DEFAULT_GRAVITY_MODE;
        this.G = G_BY_MODE[this._gravityMode];
        this._physics = { ...DEFAULT_SOLAR_PHYSICS };
        this._forceAudit = freshForceAudit();
        this._events = [];
        this._nextEventId = 0;
        this._eventCounts = { collisions: 0, rocheDisruptions: 0, atmosphereEntries: 0 };
        this._dissipatedEnergy = 0;
        this._stellarMassLost = 0;
        this._pendingSpinTorques = new Map();
        this._atmosphereInside = new Set();
        this._bodyRevision = 0;
    }

    /**
     * Select the gravitational-constant mode. Takes effect on the next
     * setupScenario() (callers reload the scenario after switching).
     * @param {'presentation'|'decorative'|'physical'} mode
     */
    setGravityMode(mode) {
        if (!(mode in G_BY_MODE)) return;
        this._gravityMode = mode;
        this.G = G_BY_MODE[mode];
    }

    getGravityMode() {
        return this._gravityMode;
    }

    setTimeStep(key) {
        if (!Object.prototype.hasOwnProperty.call(PLANETARY_TIME_STEPS, key)) return false;
        this._timeStepKey = key;
        this._configureIntegrationClock();
        if (this._bodies.length) this._computeForces();
        return true;
    }

    getTimeStepStatus() {
        const naturalUnits = this._scenarioName === 'planetary-threebody';
        return {
            key: naturalUnits ? 'natural' : this._timeStepKey,
            requestedKey: this._timeStepKey,
            label: naturalUnits ? '0.01 natural time' : PLANETARY_TIME_STEPS[this._timeStepKey].label,
            seconds: naturalUnits ? null : PLANETARY_TIME_STEPS[this._timeStepKey].seconds,
            years: naturalUnits ? null : this._timeStepYears,
            naturalTime: naturalUnits ? this._timeStepYears : null,
            integrationSubsteps: this._integrationSubsteps,
            integrationDtYears: naturalUnits ? null : this._dt,
            integrationDtNatural: naturalUnits ? this._dt : null,
            integrationDtSeconds: naturalUnits ? null : this._dt * DAYS_PER_YEAR * SECONDS_PER_DAY,
            naturalUnits,
        };
    }

    _configureIntegrationClock() {
        if (this._scenarioName === 'planetary-threebody') {
            this._timeStepYears = 0.01;
            this._integrationSubsteps = 100;
            this._dt = this._timeStepYears / this._integrationSubsteps;
            return;
        }
        this._timeStepYears = PLANETARY_TIME_STEPS[this._timeStepKey].years;
        this._integrationSubsteps = Math.max(1, Math.ceil(
            this._timeStepYears / Math.max(this._maxIntegrationStepYears, 1e-15),
        ));
        this._dt = this._timeStepYears / this._integrationSubsteps;
    }

    setPhysicsToggle(key, enabled) {
        if (!(key in DEFAULT_SOLAR_PHYSICS)) return false;
        this._physics[key] = !!enabled;
        if (this._bodies.length) this._computeForces();
        return true;
    }

    setPhysicsToggles(values = {}) {
        for (const key of Object.keys(DEFAULT_SOLAR_PHYSICS)) {
            if (Object.prototype.hasOwnProperty.call(values, key)) this._physics[key] = !!values[key];
        }
        if (this._bodies.length) this._computeForces();
    }

    getPhysicsToggles() {
        return { ...this._physics };
    }

    _physicsApplicability(key) {
        const physicalUnits = this._scenarioName !== 'planetary-threebody' && this._gravityMode === 'physical';
        const byId = new Map(this._bodies.map((body) => [body.id, body]));
        const hasParentWhere = (predicate) => this._bodies.some((body) => {
            const parent = byId.get(body.parentId);
            return parent && predicate(body, parent);
        });
        const luminousStars = this._bodies.filter((body) => body.type === PlanetaryMockBridge.TYPE.STAR && body.luminosityW > 0);
        const luminousStar = this._dominantStar();
        const dependency = SOLAR_PHYSICS_DEFINITIONS.find((definition) => definition.key === key)?.requires
            ?.find((requiredKey) => !this._physics[requiredKey]);
        if (dependency) {
            const dependencyLabel = SOLAR_PHYSICS_DEFINITIONS.find((definition) => definition.key === dependency)?.label || dependency;
            return { applicable: false, reason: `Requires ${dependencyLabel} to be enabled.` };
        }
        switch (key) {
            case 'newtonianGravity':
                return { applicable: this._bodies.length > 1, reason: 'Requires at least two massive bodies.' };
            case 'relativity1PN':
                return { applicable: physicalUnits && luminousStars.length === 1 && !!luminousStar, reason: 'Requires physical AU/yr units and exactly one luminous star; multi-star 1PN requires the full EIH equations.' };
            case 'oblatenessJ2':
                return { applicable: physicalUnits && hasParentWhere((_, parent) => parent.j2 > 0), reason: 'Requires a parent with a configured J2.' };
            case 'equilibriumTides':
                return { applicable: physicalUnits && hasParentWhere((_, parent) => parent.loveK2 > 0 && parent.tidalQ > 0), reason: 'Requires a configured deformable parent and orbiting child.' };
            case 'radiationForces':
                return { applicable: physicalUnits && luminousStars.length > 0, reason: 'Requires at least one luminous star in physical units.' };
            case 'solarWindDrag':
                return { applicable: physicalUnits && luminousStars.length > 0, reason: 'Requires the radiation-force kernel and at least one luminous star.' };
            case 'stellarMassLoss':
                return { applicable: physicalUnits && this._bodies.some((body) => body.type === PlanetaryMockBridge.TYPE.STAR && body.massLossPerYear > 0), reason: 'Requires a star with a configured mass-loss rate.' };
            case 'atmosphericDrag':
                return { applicable: physicalUnits && hasParentWhere((_, parent) => parent.atmosphereDensityKgM3 > 0), reason: 'Requires a child crossing a configured parent atmosphere.' };
            case 'finiteBodyCollisions':
                return { applicable: this._bodies.filter((body) => body.r > 0).length > 1, reason: 'Requires at least two bodies with physical radii.' };
            case 'rocheDisruption':
                return { applicable: physicalUnits && hasParentWhere((body) => body.rocheEligible !== false), reason: 'Requires a finite-density child with a modeled parent.' };
            default:
                return { applicable: false, reason: 'Unknown physics kernel.' };
        }
    }

    _physicsActive(key) {
        return !!this._physics[key] && this._physicsApplicability(key).applicable;
    }

    getPhysicsStatus() {
        return SOLAR_PHYSICS_DEFINITIONS.map((definition) => {
            const { applicable, reason } = this._physicsApplicability(definition.key);
            const requested = !!this._physics[definition.key];
            return { ...definition, requested, applicable, active: requested && applicable, reason };
        });
    }

    getEvents() {
        return this._events.map((event) => ({ ...event }));
    }

    _recordEvent(category, message, details = {}) {
        this._events.unshift({
            id: `${this._nextEventId++}-${category}`,
            tick: this._tick,
            timeYears: this._timeYears,
            category,
            message,
            ...details,
        });
        if (this._events.length > MAX_EVENT_LOG) this._events.length = MAX_EVENT_LOG;
    }

    static TYPE = {
        STAR: 0,
        ROCKY_PLANET: 1,
        GAS_GIANT: 2,
        MOON: 3,
        ASTEROID: 4,
        DWARF_PLANET: 5,
    };

    addBody(type, mass, r, x, y, z, vx=0, vy=0, vz=0, seed=0, metadata={}) {
        const id = this._nextId++;
        const rotationDays = Number(metadata.rotationDays) || 0;
        const body = {
            id, type, mass, r,
            x, y, z, vx, vy, vz,
            ax: 0, ay: 0, az: 0,
            seed: seed || this._nextId * 997.3 + 17,
            initialMass: mass,
            createdTick: this._tick,
            createdTimeYears: this._timeYears,
            radiationQpr: 1,
            dragCoefficient: 2.2,
            rocheEligible: true,
            ...metadata,
            spinRateRadYr: Number.isFinite(metadata.spinRateRadYr)
                ? metadata.spinRateRadYr
                : siderealSpinRateRadYr(metadata),
            rotationPhaseRad: wrapRotationPhase(Number(metadata.rotationPhaseRad) || 0),
        };
        this._bodies.push(body);
        this._bodyRevision++;
        return id;
    }

    // ================================================================
    // SCENARIOS
    // ================================================================

    setupScenario(name) {
        const admitted = getScale4Scenario(name) || getScale4Scenario(SCALE4_DEFAULT_SCENARIO);
        name = admitted.id;
        this._bodies = [];
        this._nextId = 0;
        this._tick = 0;
        this._timeYears = 0;
        this._scenarioName = name;
        this._provenance = null;
        this._events = [];
        this._nextEventId = 0;
        this._eventCounts = { collisions: 0, rocheDisruptions: 0, atmosphereEntries: 0 };
        this._dissipatedEnergy = 0;
        this._stellarMassLost = 0;
        this._pendingSpinTorques.clear();
        this._atmosphereInside.clear();
        this._forceAudit = freshForceAudit();
        this._bodyRevision++;
        this._maxIntegrationStepYears = DEFAULT_MAX_INTEGRATION_STEP_YEARS;
        // Apply the currently-selected gravity mode (figure-8 scenario below
        // overrides to G=1 for its natural units, regardless of mode).
        this.G = G_BY_MODE[this._gravityMode];

        const T = PlanetaryMockBridge.TYPE;
        if (name === 'planetary-solar') {
            this._setupSolarSystem(T);
        } else if (name === 'planetary-mercury-relativity') {
            this._setupReferenceSubset(T, ['mercury']);
        } else if (name === 'planetary-earth-moon-tides') {
            this._setupEarthMoonLab(T);
        } else if (name === 'planetary-radiation-lab') {
            this._setupRadiationLab(T);
        } else if (name === 'planetary-atmosphere-entry') {
            this._setupAtmosphereEntryLab(T);
        } else if (name === 'planetary-roche-lab') {
            this._setupRocheLab(T);
        } else if (name === 'planetary-impact-lab') {
            this._setupImpactLab(T);
        } else if (name === 'planetary-binary') {
            this._provenance = BINARY_SYSTEM_PROVENANCE;
            // Twin suns
            const a = 2.0; 
            const M = 1.0;
            // v = sqrt(G*M/(4a))
            const v = Math.sqrt(this.G * M / (4 * a));
            const solarRadius = SUN_REFERENCE.radiusKm / KM_PER_AU;
            this.addBody(T.STAR, M, solarRadius, a, 0, 0, 0, v, 0, 1001, {
                ...SUN_REFERENCE, key: 'binary-a', name: 'Binary A', parentId: -1,
            });
            this.addBody(T.STAR, M, solarRadius, -a, 0, 0, 0, -v, 0, 1002, {
                ...SUN_REFERENCE, key: 'binary-b', name: 'Binary B', parentId: -1,
            });

            // Circumbinary planet
            const rp = 6.0;
            const vp = Math.sqrt(this.G * (2 * M) / rp);
            this.addBody(T.GAS_GIANT, 3.00e-6, 6371 / KM_PER_AU, rp, 0, 0, 0, vp, 0, 1003, {
                key: 'circumbinary', name: 'Circumbinary world', className: 'Circumbinary planet',
                radiusKm: 6371, parentId: -1, parentKey: 'binary barycenter', color: '#63b3ed',
            });
            this._recenterBarycentric();
        } else if (name === 'planetary-threebody') {
            this._provenance = FIGURE_EIGHT_PROVENANCE;
            // Figure-8 (Chenciner-Montgomery 2000). Requires G=1, so temporarily override.
            // The G=1.0 below is the intentional figure-8 unit convention
            // (Chenciner–Montgomery natural units) and is NOT a physics value.
            this.G = 1.0;
            this._maxIntegrationStepYears = 0.0001;
            this.addBody(T.STAR, 1.0, 0.02, 0.97000436, -0.24308753, 0, 0.466203685, 0.43236573, 0, 201, { key: 'figure8-a', name: 'Figure-8 A', parentId: -1, rocheEligible: false });
            this.addBody(T.STAR, 1.0, 0.02, -0.97000436, 0.24308753, 0, 0.466203685, 0.43236573, 0, 202, { key: 'figure8-b', name: 'Figure-8 B', parentId: -1, rocheEligible: false });
            this.addBody(T.STAR, 1.0, 0.02, 0, 0, 0, -2 * 0.466203685, -2 * 0.43236573, 0, 203, { key: 'figure8-c', name: 'Figure-8 C', parentId: -1, rocheEligible: false });
            this._threebody_G = 1.0; // keep G=1 for this scenario
        } else if (name.startsWith('exo-') && EXOPLANET_SYSTEMS) {
            const host = name.substring(4);
            const system = EXOPLANET_SYSTEMS[host];
            if (system?.planets?.length > 0) {
                const star = system.host;
                const M_star = star.massSolar;
                const radiusKm = star.radiusSolar * SUN_REFERENCE.radiusKm;
                const luminosityW = SUN_REFERENCE.luminosityW
                    * star.radiusSolar ** 2
                    * (star.temperatureK / SUN_REFERENCE.temperatureK) ** 4;
                this._provenance = system.provenance;
                this.addBody(T.STAR, M_star, radiusKm / KM_PER_AU, 0, 0, 0, 0, 0, 0, 1, {
                    ...star,
                    className: `${star.spectralType} exoplanet host star`,
                    radiusKm,
                    style: 0,
                    luminosityW,
                    luminosityBasis: 'Stefan-Boltzmann scaling from catalog radius and effective temperature',
                    parentId: -1,
                });

                // Add planets
                for (let i = 0; i < system.planets.length; i++) {
                    const p = system.planets[i];
                    const M_p = p.massSolar;
                    const a = p.semiMajorAxisAu;
                    const e = p.eccentricity;
                    const r = a * (1 - e);
                    // vis-viva at perihelion: v = sqrt(G*M*(1+e)/(a*(1-e)))
                    const v = Math.sqrt(this.G * (M_star + M_p) * (1 + e) / (a * (1 - e)));
                    const physicalRadius = p.radiusKm / KM_PER_AU;
                    
                    // Simple hash from string for deterministic traits
                    let nameHash = 0;
                    if (p.name) {
                        for(let c=0; c<p.name.length; c++) nameHash += p.name.charCodeAt(c) * (c+1);
                    }
                    
                    // Prevent catastrophic gravitational syzygy by distributing starting phase deterministicly
                    const theta = (nameHash % 360) * (Math.PI / 180);
                    const start_x = r * Math.cos(theta);
                    const start_y = r * Math.sin(theta);
                    const start_vx = -v * Math.sin(theta);
                    const start_vy = v * Math.cos(theta);

                    const bodyType = p.appearanceClass === 'rocky' ? T.ROCKY_PLANET : T.GAS_GIANT;
                    this.addBody(bodyType, M_p, physicalRadius, start_x, start_y, 0, start_vx, start_vy, 0, nameHash, {
                        ...p,
                        key: p.name.toLowerCase().replace(/\s+/g, '-'),
                        temperatureK: p.equilibriumTemperatureK,
                        orbit: { a, e, i: 0 }, parentKey: host.toLowerCase().replace(/\s+/g, '-'),
                        parentId: 0, rocheEligible: false,
                    });
                }
                this._recenterBarycentric();
            } else {
                this.addBody(T.STAR, 1.0, 1.0, 0, 0, 0, 0, 0, 0);
            }
        } else {
            // Fallback empty
            this.addBody(T.STAR, 1.0, 1.0, 0, 0, 0, 0, 0, 0);
        }
        
        this._configureIntegrationClock();
        this._computeForces();
        this._recordEvent('scenario', `Loaded ${name} with ${this._bodies.length} modeled bodies.`);
    }

    _setupReferenceSubset(T, planetKeys, moonKeys = []) {
        this._setupSolarSystem(T);
        const retained = new Set(['sun', ...planetKeys, ...moonKeys]);
        this._bodies = this._bodies.filter((body) => retained.has(body.key));
        this._bodyRevision++;
        this._recenterBarycentric();
    }

    _setupRadiationLab(T) {
        this._provenance = SOLAR_SYSTEM_PROVENANCE;
        const sunId = this.addBody(
            T.STAR, SUN_REFERENCE.massSolar, SUN_REFERENCE.radiusKm / KM_PER_AU,
            0, 0, 0, 0, 0, 0, 301, { ...SUN_REFERENCE, parentId: -1 },
        );
        const radiiMicron = [0.25, 1, 10];
        radiiMicron.forEach((radiusMicron, index) => {
            const radiusM = radiusMicron * 1e-6;
            const massKg = (4 / 3) * Math.PI * radiusM ** 3 * 3000;
            const massSolar = massKg / SOLAR_MASS_KG;
            const distance = 0.35 + index * 0.08;
            const phase = index * 2.1;
            const speed = Math.sqrt(this.G * SUN_REFERENCE.massSolar / distance);
            this.addBody(
                T.ASTEROID, massSolar, radiusM / 1000 / KM_PER_AU,
                distance * Math.cos(phase), distance * Math.sin(phase), 0,
                -speed * Math.sin(phase), speed * Math.cos(phase), 0,
                310 + index,
                {
                    key: `dust-${radiusMicron}`, name: `${radiusMicron} µm silicate grain`,
                    className: 'Radiation-force test grain', style: 12,
                    radiusKm: radiusM / 1000, parentId: sunId, parentKey: 'sun',
                    color: ['#fef3c7', '#fde68a', '#f59e0b'][index], rocheEligible: false,
                },
            );
        });
        this._recenterBarycentric();
    }

    _setupEarthMoonLab(T) {
        this._provenance = SOLAR_SYSTEM_PROVENANCE;
        const earth = SOLAR_SYSTEM_PLANETS.find((body) => body.key === 'earth');
        const moon = SOLAR_SYSTEM_MOONS.find((body) => body.key === 'moon');
        const earthId = this.addBody(
            T.ROCKY_PLANET, earth.massSolar, earth.radiusKm / KM_PER_AU,
            0, 0, 0, 0, 0, 0, 351, { ...earth, parentId: -1, rocheEligible: false },
        );
        const orbit = {
            a: moon.aKm / KM_PER_AU,
            e: moon.e,
            i: moon.i,
            L: moon.phaseDeg,
            longPeri: 0,
            longNode: (moon.phaseDeg * 0.61803398875) % 360,
        };
        const state = orbitalElementsToState(orbit, earth.massSolar, moon.massSolar, this.G);
        this.addBody(
            T.MOON, moon.massSolar, moon.radiusKm / KM_PER_AU,
            state.x, state.y, state.z, state.vx, state.vy, state.vz,
            352,
            {
                ...moon, orbit, parentId: earthId, parentKey: 'earth',
                rotationDays: Math.abs(moon.periodDays), axialTiltDeg: 0,
                atmosphere: 'Trace or none', magneticField: 'No known global field',
            },
        );
        this._recenterBarycentric();
    }

    _setupAtmosphereEntryLab(T) {
        this._provenance = SOLAR_SYSTEM_PROVENANCE;
        this._maxIntegrationStepYears = 1 / SECONDS_PER_DAY / DAYS_PER_YEAR;
        const earth = SOLAR_SYSTEM_PLANETS.find((body) => body.key === 'earth');
        const earthId = this.addBody(
            T.ROCKY_PLANET, earth.massSolar, earth.radiusKm / KM_PER_AU,
            0, 0, 0, 0, 0, 0, 401, { ...earth, parentId: -1, rocheEligible: false },
        );
        const altitudeKm = 90;
        const distance = (earth.radiusKm + altitudeKm) / KM_PER_AU;
        const entrySpeedAuYr = 7.8 * 86_400 * DAYS_PER_YEAR / KM_PER_AU;
        const capsuleMassSolar = 10_000 / SOLAR_MASS_KG;
        this.addBody(
            T.ASTEROID, capsuleMassSolar, 0.0015 / KM_PER_AU,
            distance, 0, 0, 0, entrySpeedAuYr, -0.04,
            402,
            {
                key: 'entry-capsule', name: 'Entry capsule', className: '10-ton atmospheric probe',
                style: 12, radiusKm: 0.0015, color: '#f97316', parentId: earthId,
                parentKey: 'earth', dragCoefficient: 1.1, rocheEligible: false,
            },
        );
    }

    _setupRocheLab(T) {
        this._provenance = SOLAR_SYSTEM_PROVENANCE;
        this._maxIntegrationStepYears = 0.0002 / DAYS_PER_YEAR;
        const saturn = SOLAR_SYSTEM_PLANETS.find((body) => body.key === 'saturn');
        const parentId = this.addBody(
            T.GAS_GIANT, saturn.massSolar, saturn.radiusKm / KM_PER_AU,
            0, 0, 0, 0, 0, 0, 501, { ...saturn, parentId: -1, rocheEligible: false },
        );
        const radiusKm = 180;
        const densityKgM3 = 900;
        const radiusM = radiusKm * 1000;
        const massSolar = ((4 / 3) * Math.PI * radiusM ** 3 * densityKgM3) / SOLAR_MASS_KG;
        const distance = 1.8 * saturn.radiusKm / KM_PER_AU;
        const speed = Math.sqrt(this.G * saturn.massSolar / distance);
        this.addBody(
            T.MOON, massSolar, radiusKm / KM_PER_AU,
            distance, 0, 0, 0, speed, 0, 502,
            {
                key: 'roche-moon', name: 'Roche-limit moon', className: 'Icy rubble satellite',
                style: 10, radiusKm, color: '#dbeafe', parentId, parentKey: 'saturn',
                rotationDays: 0.35, axialTiltDeg: 0,
            },
        );
    }

    _setupImpactLab(T) {
        this._provenance = SOLAR_SYSTEM_PROVENANCE;
        this._maxIntegrationStepYears = 1e-6;
        const radiusKm = 120;
        const densityKgM3 = 2800;
        const massSolar = ((4 / 3) * Math.PI * (radiusKm * 1000) ** 3 * densityKgM3) / SOLAR_MASS_KG;
        const separation = 1200 / KM_PER_AU;
        const speed = 0.7 * 86_400 * DAYS_PER_YEAR / KM_PER_AU;
        this.addBody(T.ASTEROID, massSolar, radiusKm / KM_PER_AU, -separation / 2, 0, 0, speed, 0, 0, 601, {
            key: 'impact-a', name: 'Impactor A', className: 'Rocky protoplanet', radiusKm, style: 4,
            color: '#fb923c', parentId: -1, rocheEligible: false,
        });
        this.addBody(T.ASTEROID, massSolar, radiusKm / KM_PER_AU, separation / 2, 0, 0, -speed, 0, 0, 602, {
            key: 'impact-b', name: 'Impactor B', className: 'Rocky protoplanet', radiusKm, style: 1,
            color: '#94a3b8', parentId: -1, rocheEligible: false,
        });
    }

    _setupSolarSystem(T) {
        this._provenance = SOLAR_SYSTEM_PROVENANCE;
        const ids = new Map();
        const sunId = this.addBody(
            T.STAR, SUN_REFERENCE.massSolar, SUN_REFERENCE.radiusKm / KM_PER_AU,
            0, 0, 0, 0, 0, 0, 101, { ...SUN_REFERENCE, parentId: -1 },
        );
        ids.set(SUN_REFERENCE.key, sunId);

        for (const ref of [...SOLAR_SYSTEM_PLANETS, ...SOLAR_SYSTEM_DWARFS]) {
            const state = orbitalElementsToState(ref.orbit, SUN_REFERENCE.massSolar, ref.massSolar, this.G);
            const type = ref.className === 'Dwarf planet'
                ? T.DWARF_PLANET
                : (ref.className.includes('giant') ? T.GAS_GIANT : T.ROCKY_PLANET);
            const id = this.addBody(
                type, ref.massSolar, ref.radiusKm / KM_PER_AU,
                state.x, state.y, state.z, state.vx, state.vy, state.vz,
                ref.key.split('').reduce((a, c) => a + c.charCodeAt(0), 0),
                { ...ref, parentId: sunId, parentKey: 'sun' },
            );
            ids.set(ref.key, id);
        }

        for (const ref of SOLAR_SYSTEM_MOONS) {
            const parentId = ids.get(ref.parent);
            const parent = this._bodies.find((body) => body.id === parentId);
            if (!parent) continue;
            const periodSign = Math.sign(ref.periodDays || 1);
            const orbit = {
                a: ref.aKm / KM_PER_AU,
                e: ref.e,
                i: ref.i,
                L: ref.phaseDeg,
                longPeri: 0,
                longNode: (ref.phaseDeg * 0.61803398875) % 360,
            };
            const state = orbitalElementsToState(orbit, parent.mass, ref.massSolar, this.G);
            if (periodSign < 0) {
                state.vx *= -1; state.vy *= -1; state.vz *= -1;
            }
            // JPL Table 1 publishes the Earth-Moon barycenter rather than the
            // Earth's geocenter. Split that reference state into geocentric
            // Earth and Moon states so their mass-weighted position/velocity
            // remains exactly at the imported EM-barycenter coordinate.
            if (parent.orbitReference === 'Earth-Moon barycenter') {
                const childMassFraction = ref.massSolar / (parent.mass + ref.massSolar);
                parent.x -= state.x * childMassFraction;
                parent.y -= state.y * childMassFraction;
                parent.z -= state.z * childMassFraction;
                parent.vx -= state.vx * childMassFraction;
                parent.vy -= state.vy * childMassFraction;
                parent.vz -= state.vz * childMassFraction;
            }
            this.addBody(
                T.MOON, ref.massSolar, ref.radiusKm / KM_PER_AU,
                parent.x + state.x, parent.y + state.y, parent.z + state.z,
                parent.vx + state.vx, parent.vy + state.vy, parent.vz + state.vz,
                ref.key.split('').reduce((a, c) => a + c.charCodeAt(0), 0),
                {
                    ...ref, orbit, parentId, parentKey: ref.parent,
                    rotationDays: ref.periodDays, axialTiltDeg: 0,
                    atmosphere: ref.key === 'titan' ? 'Dense nitrogen atmosphere' : 'Trace or none',
                    magneticField: ref.key === 'ganymede' ? 'Intrinsic global field' : 'No known global field',
                },
            );
        }

        this._recenterBarycentric();
    }

    _recenterBarycentric() {
        let m = 0, x = 0, y = 0, z = 0, vx = 0, vy = 0, vz = 0;
        for (const b of this._bodies) {
            m += b.mass;
            x += b.mass * b.x; y += b.mass * b.y; z += b.mass * b.z;
            vx += b.mass * b.vx; vy += b.mass * b.vy; vz += b.mass * b.vz;
        }
        if (!(m > 0)) return;
        x /= m; y /= m; z /= m; vx /= m; vy /= m; vz /= m;
        for (const b of this._bodies) {
            b.x -= x; b.y -= y; b.z -= z;
            b.vx -= vx; b.vy -= vy; b.vz -= vz;
        }
    }

    _dominantStar() {
        let dominant = null;
        for (const body of this._bodies) {
            if (body.type === PlanetaryMockBridge.TYPE.STAR && (!dominant || body.mass > dominant.mass)) dominant = body;
        }
        return dominant;
    }

    _relativeState(child, parent) {
        return {
            r: { x: child.x - parent.x, y: child.y - parent.y, z: child.z - parent.z },
            v: { x: child.vx - parent.vx, y: child.vy - parent.vy, z: child.vz - parent.vz },
        };
    }

    /** Apply a relative acceleration while preserving material linear momentum. */
    _applyRelativeAcceleration(child, parent, acceleration) {
        const totalMass = child.mass + parent.mass;
        if (!(totalMass > 0)) return;
        const childShare = parent.mass / totalMass;
        const parentShare = child.mass / totalMass;
        child.ax += acceleration.x * childShare;
        child.ay += acceleration.y * childShare;
        child.az += acceleration.z * childShare;
        parent.ax -= acceleration.x * parentShare;
        parent.ay -= acceleration.y * parentShare;
        parent.az -= acceleration.z * parentShare;
    }

    _auditAcceleration(channel, acceleration) {
        this._forceAudit[channel] = Math.max(this._forceAudit[channel], vectorMagnitude(acceleration));
    }

    _queueSpinTorque(bodyId, torque) {
        if (!Number.isFinite(torque) || torque === 0) return;
        this._pendingSpinTorques.set(bodyId, (this._pendingSpinTorques.get(bodyId) || 0) + torque);
    }

    _computeForces() {
        const N = this._bodies.length;
        const eps2 = this.softeningSq;
        const byId = new Map(this._bodies.map((body) => [body.id, body]));
        // Applicability can inspect the whole body set. Resolve each kernel
        // once per force sweep instead of rebuilding maps/scanning bodies from
        // inside the per-child loop.
        const activePhysics = Object.fromEntries(
            SOLAR_PHYSICS_DEFINITIONS.map(({ key }) => [key, this._physicsActive(key)]),
        );
        this._forceAudit = freshForceAudit();
        this._pendingSpinTorques = new Map();

        for (const body of this._bodies) body.ax = body.ay = body.az = 0;

        if (activePhysics.newtonianGravity) {
            for (let i = 0; i < N; i++) {
                const bi = this._bodies[i];
                for (let j = i + 1; j < N; j++) {
                    const bj = this._bodies[j];
                    const dx = bj.x - bi.x;
                    const dy = bj.y - bi.y;
                    const dz = bj.z - bi.z;
                    const r2 = dx * dx + dy * dy + dz * dz + eps2;
                    const rInv = 1 / Math.sqrt(r2);
                    const scale = this.G * rInv * rInv * rInv;
                    const onI = { x: scale * dx * bj.mass, y: scale * dy * bj.mass, z: scale * dz * bj.mass };
                    const onJ = { x: -scale * dx * bi.mass, y: -scale * dy * bi.mass, z: -scale * dz * bi.mass };
                    bi.ax += onI.x; bi.ay += onI.y; bi.az += onI.z;
                    bj.ax += onJ.x; bj.ay += onJ.y; bj.az += onJ.z;
                    this._auditAcceleration('newtonianAcceleration', onI);
                    this._auditAcceleration('newtonianAcceleration', onJ);
                }
            }
        }

        const star = this._dominantStar();
        if (star && activePhysics.relativity1PN) {
            const mu = this.G * star.mass;
            for (const body of this._bodies) {
                if (body === star || body.type === PlanetaryMockBridge.TYPE.STAR) continue;
                const { r, v } = this._relativeState(body, star);
                const acceleration = schwarzschild1PNAcceleration(r, v, mu);
                this._applyRelativeAcceleration(body, star, acceleration);
                this._auditAcceleration('relativityAcceleration', acceleration);
            }
        }

        for (const child of this._bodies) {
            const parent = byId.get(child.parentId);
            if (!parent || child === parent) continue;
            const { r, v } = this._relativeState(child, parent);

            if (activePhysics.oblatenessJ2 && parent.j2 > 0) {
                const acceleration = j2Acceleration(r, this.G * parent.mass, parent.r, parent.j2, spinAxis(parent));
                this._applyRelativeAcceleration(child, parent, acceleration);
                this._auditAcceleration('j2Acceleration', acceleration);
            }

            if (activePhysics.equilibriumTides) {
                const tidal = equilibriumTideAcceleration(child, parent, r, v, this.G);
                this._applyRelativeAcceleration(child, parent, tidal);
                this._auditAcceleration('tideAcceleration', tidal);
                this._queueSpinTorque(parent.id, -tidal.torque);
                this._forceAudit.instantaneousDissipationPower += tidal.power || 0;
                child.tidalMigrationAuPerYear = tidal.migrationAuPerYear || 0;
            } else {
                child.tidalMigrationAuPerYear = 0;
            }

            if (activePhysics.atmosphericDrag) {
                const drag = atmosphereDragAcceleration(child, parent, r, v, this._dt);
                this._applyRelativeAcceleration(child, parent, drag);
                this._auditAcceleration('atmosphereAcceleration', drag);
                this._forceAudit.instantaneousDissipationPower += drag.power || 0;
                child.localAtmosphereDensityKgM3 = drag.density || 0;
                child.liveAltitudeKm = drag.altitudeKm;
                if (drag.density > 0) {
                    const axis = spinAxis(parent);
                    const torque = child.mass * (
                        axis.x * (r.y * drag.z - r.z * drag.y)
                        + axis.y * (r.z * drag.x - r.x * drag.z)
                        + axis.z * (r.x * drag.y - r.y * drag.x)
                    );
                    this._queueSpinTorque(parent.id, -torque);
                    if (!this._atmosphereInside.has(child.id)) {
                        this._atmosphereInside.add(child.id);
                        this._eventCounts.atmosphereEntries++;
                        this._recordEvent('atmosphere', `${child.name || `Body ${child.id}`} entered ${parent.name || 'a parent'}'s modeled atmosphere.`);
                    }
                } else {
                    this._atmosphereInside.delete(child.id);
                }
            } else {
                child.localAtmosphereDensityKgM3 = 0;
                child.liveAltitudeKm = Infinity;
            }
        }

        const luminousStars = this._bodies.filter((body) => body.type === PlanetaryMockBridge.TYPE.STAR && body.luminosityW > 0);
        for (const body of this._bodies) body.radiationBeta = 0;
        if (luminousStars.length && activePhysics.radiationForces) {
            const windRatio = activePhysics.solarWindDrag ? SOLAR_WIND_TO_PR_DRAG : 0;
            for (const source of luminousStars) {
                const mu = this.G * source.mass;
                for (const body of this._bodies) {
                    if (body === source || body.type === PlanetaryMockBridge.TYPE.STAR) continue;
                    const { r, v } = this._relativeState(body, source);
                    const beta = radiationBeta(body, source);
                    body.radiationBeta = Math.max(body.radiationBeta, beta);
                    if (!(beta > 0)) continue;
                    const acceleration = radiationAcceleration(r, v, mu, beta, windRatio);
                    body.ax += acceleration.x;
                    body.ay += acceleration.y;
                    body.az += acceleration.z;
                    this._auditAcceleration('radiationAcceleration', acceleration);
                    this._auditAcceleration('radiationPressureAcceleration', acceleration.pressure);
                    this._auditAcceleration('poyntingRobertsonDragAcceleration', acceleration.prDrag);
                    this._auditAcceleration('solarWindDragAcceleration', acceleration.windDrag);
                    const dragPower = -body.mass * (
                        acceleration.drag.x * v.x + acceleration.drag.y * v.y + acceleration.drag.z * v.z
                    );
                    this._forceAudit.instantaneousDissipationPower += Math.max(0, dragPower);
                }
            }
        }
    }

    _applyStellarMassLoss() {
        if (!this._physicsActive('stellarMassLoss')) {
            return;
        }
        let totalLost = 0;
        const elapsedYears = this._timeYears;
        for (const star of this._bodies) {
            if (star.type !== PlanetaryMockBridge.TYPE.STAR) continue;
            const rate = Number(star.massLossPerYear) || DEFAULT_SOLAR_MASS_LOSS_PER_YEAR;
            if (!(rate > 0 && star.initialMass > 0)) continue;
            const targetMass = star.initialMass * Math.exp(-rate * elapsedYears);
            totalLost += star.initialMass * -Math.expm1(-rate * elapsedYears);
            star.mass = targetMass;
        }
        this._stellarMassLost = totalLost;
    }

    _integrateSpinReservoirs(dt) {
        for (const body of this._bodies) {
            const priorRate = Number.isFinite(body.spinRateRadYr) ? body.spinRateRadYr : 0;
            let nextRate = priorRate;
            const torque = this._pendingSpinTorques.get(body.id) || 0;
            const momentFactor = Number(body.momentFactor);
            if (torque !== 0 && momentFactor > 0 && body.mass > 0 && body.r > 0) {
                const inertia = momentFactor * body.mass * body.r * body.r;
                nextRate += torque * dt / inertia;
            }
            if (!Number.isFinite(nextRate)) nextRate = priorRate;
            body.spinRateRadYr = nextRate;
            body.rotationPhaseRad = wrapRotationPhase(
                (Number(body.rotationPhaseRad) || 0) + 0.5 * (priorRate + nextRate) * dt,
            );
            if (torque !== 0 && Math.abs(nextRate) > 1e-15) {
                body.rotationDays = signedSiderealPeriodDays(body, nextRate);
            }
        }
    }

    _resolveRocheDisruptions() {
        if (!this._physicsActive('rocheDisruption') || this._bodies.length + 7 > MAX_DYNAMIC_BODIES) return;
        const byId = new Map(this._bodies.map((body) => [body.id, body]));
        const candidate = this._bodies.find((child) => {
            if (child.rocheEligible === false || child.rocheFragment) return false;
            const parent = byId.get(child.parentId);
            if (!parent) return false;
            const distance = Math.hypot(child.x - parent.x, child.y - parent.y, child.z - parent.z);
            const limit = fluidRocheLimitAu(parent, child);
            return limit > 0 && distance < limit && distance > parent.r + child.r;
        });
        if (!candidate) return;

        const parent = byId.get(candidate.parentId);
        const radial = {
            x: candidate.x - parent.x,
            y: candidate.y - parent.y,
            z: candidate.z - parent.z,
        };
        const radialNorm = Math.max(vectorMagnitude(radial), 1e-15);
        radial.x /= radialNorm; radial.y /= radialNorm; radial.z /= radialNorm;
        const reference = Math.abs(radial.z) < 0.85 ? { x: 0, y: 0, z: 1 } : { x: 1, y: 0, z: 0 };
        const u = {
            x: radial.y * reference.z - radial.z * reference.y,
            y: radial.z * reference.x - radial.x * reference.z,
            z: radial.x * reference.y - radial.y * reference.x,
        };
        const uNorm = Math.max(vectorMagnitude(u), 1e-15);
        u.x /= uNorm; u.y /= uNorm; u.z /= uNorm;
        const w = {
            x: radial.y * u.z - radial.z * u.y,
            y: radial.z * u.x - radial.x * u.z,
            z: radial.x * u.y - radial.y * u.x,
        };
        const fragmentCount = 8;
        const fragmentMass = candidate.mass / fragmentCount;
        const fragmentRadius = candidate.r / Math.cbrt(fragmentCount);
        const ringRadius = candidate.r * 1.55;
        const escapeSpeed = Math.sqrt(Math.max(0, 2 * this.G * candidate.mass / Math.max(candidate.r, 1e-15)));
        const fragmentFamily = `${candidate.id}-${this._tick}`;
        this._bodies = this._bodies.filter((body) => body !== candidate);
        this._bodyRevision++;
        for (let index = 0; index < fragmentCount; index++) {
            const phase = 2 * Math.PI * index / fragmentCount;
            const ox = ringRadius * (Math.cos(phase) * u.x + Math.sin(phase) * w.x);
            const oy = ringRadius * (Math.cos(phase) * u.y + Math.sin(phase) * w.y);
            const oz = ringRadius * (Math.cos(phase) * u.z + Math.sin(phase) * w.z);
            const dispersion = escapeSpeed * 0.65;
            const tx = Math.cos(phase) * u.x + Math.sin(phase) * w.x;
            const ty = Math.cos(phase) * u.y + Math.sin(phase) * w.y;
            const tz = Math.cos(phase) * u.z + Math.sin(phase) * w.z;
            this.addBody(
                candidate.type, fragmentMass, fragmentRadius,
                candidate.x + ox, candidate.y + oy, candidate.z + oz,
                candidate.vx + dispersion * tx, candidate.vy + dispersion * ty, candidate.vz + dispersion * tz,
                candidate.seed + index + 1,
                {
                    key: `${candidate.key || 'body'}-fragment-${index + 1}`,
                    name: `${candidate.name || 'Body'} fragment ${index + 1}`,
                    className: 'Roche debris',
                    radiusKm: (candidate.radiusKm || candidate.r * KM_PER_AU) / Math.cbrt(fragmentCount),
                    style: candidate.style,
                    color: candidate.color,
                    temperatureK: candidate.temperatureK,
                    rotationDays: candidate.rotationDays,
                    axialTiltDeg: candidate.axialTiltDeg,
                    parentId: candidate.parentId,
                    parentKey: candidate.parentKey,
                    rocheEligible: false,
                    rocheFragment: true,
                    fragmentFamily,
                    collisionCooldownUntilTimeYears: this._timeYears + ROCHE_FRAGMENT_COLLISION_COOLDOWN_YEARS,
                },
            );
        }
        this._eventCounts.rocheDisruptions++;
        this._recordEvent(
            'roche',
            `${candidate.name || `Body ${candidate.id}`} crossed ${parent.name || 'its parent'}'s fluid Roche threshold and became ${fragmentCount} gravitating fragments.`,
            { parentId: parent.id, sourceBodyId: candidate.id, fragmentCount },
        );
    }

    _mergeBodies(first, second) {
        const primary = first.mass >= second.mass ? first : second;
        const secondary = primary === first ? second : first;
        const totalMass = first.mass + second.mass;
        if (!(totalMass > 0)) return;
        const vx = (first.mass * first.vx + second.mass * second.vx) / totalMass;
        const vy = (first.mass * first.vy + second.mass * second.vy) / totalMass;
        const vz = (first.mass * first.vz + second.mass * second.vz) / totalMass;
        const x = (first.mass * first.x + second.mass * second.x) / totalMass;
        const y = (first.mass * first.y + second.mass * second.y) / totalMass;
        const z = (first.mass * first.z + second.mass * second.z) / totalMass;
        const dv2 = (first.vx - second.vx) ** 2 + (first.vy - second.vy) ** 2 + (first.vz - second.vz) ** 2;
        const reducedMass = first.mass * second.mass / totalMass;
        const impactEnergy = 0.5 * reducedMass * dv2;
        const comparable = Math.min(first.mass, second.mass) / Math.max(first.mass, second.mass) > 0.1;

        primary.mass = totalMass;
        primary.initialMass = totalMass;
        primary.x = x; primary.y = y; primary.z = z;
        primary.vx = vx; primary.vy = vy; primary.vz = vz;
        primary.r = Math.cbrt(first.r ** 3 + second.r ** 3);
        primary.radiusKm = primary.r * KM_PER_AU;
        primary.name = comparable
            ? `${first.name || `Body ${first.id}`} + ${second.name || `Body ${second.id}`} remnant`
            : primary.name;
        primary.className = comparable ? 'Perfectly inelastic impact remnant' : primary.className;
        this._bodies = this._bodies.filter((body) => body !== secondary);
        this._bodyRevision++;
        this._dissipatedEnergy += impactEnergy;
        this._eventCounts.collisions++;
        this._recordEvent(
            'collision',
            `${first.name || `Body ${first.id}`} and ${second.name || `Body ${second.id}`} merged in a momentum-conserving inelastic impact.`,
            { survivorId: primary.id, removedId: secondary.id, impactEnergy },
        );
    }

    _resolveCollisions() {
        if (!this._physicsActive('finiteBodyCollisions')) return;
        let merged = true;
        while (merged) {
            merged = false;
            outer: for (let i = 0; i < this._bodies.length; i++) {
                const first = this._bodies[i];
                for (let j = i + 1; j < this._bodies.length; j++) {
                    const second = this._bodies[j];
                    if (first.fragmentFamily && first.fragmentFamily === second.fragmentFamily
                        && this._timeYears < Math.max(
                            first.collisionCooldownUntilTimeYears || 0,
                            second.collisionCooldownUntilTimeYears || 0,
                        )) continue;
                    const distance = Math.hypot(first.x - second.x, first.y - second.y, first.z - second.z);
                    if (distance <= first.r + second.r) {
                        this._mergeBodies(first, second);
                        merged = true;
                        break outer;
                    }
                }
            }
        }
    }

    run(ticks) {
        const count = Math.max(0, Math.floor(Number(ticks) || 0));
        for (let k = 0; k < count; k++) this.tick();
    }

    tick() {
        for (let substep = 0; substep < this._integrationSubsteps; substep++) {
            this._integrateSubstep();
        }
        this._tick++;
    }

    _integrateSubstep() {
        const dt = this._dt;
        const dt2 = 0.5 * dt;

        // V-Verlet kick 1 & drift
        for (const b of this._bodies) {
            b.vx += b.ax * dt2;
            b.vy += b.ay * dt2;
            b.vz += b.az * dt2;

            b.x += b.vx * dt;
            b.y += b.vy * dt;
            b.z += b.vz * dt;
        }

        // Positions now represent the end of this solver substep. Advance the
        // physical clock before time-dependent kernels and event timestamps.
        this._timeYears += dt;

        this._applyStellarMassLoss();
        this._resolveRocheDisruptions();
        this._resolveCollisions();

        // Recompute forces at new positions
        this._computeForces();

        // V-Verlet kick 2
        for (const b of this._bodies) {
            b.vx += b.ax * dt2;
            b.vy += b.ay * dt2;
            b.vz += b.az * dt2;
        }

        this._integrateSpinReservoirs(dt);
        this._dissipatedEnergy += this._forceAudit.instantaneousDissipationPower * dt;

    }

    getPlanetaryData() {
        // Just return a shallow clone array or flat array for renderer
        const flatBuffer = new Float32Array(this._bodies.length * 16);
        for(let i=0; i<this._bodies.length; i++) {
            const b = this._bodies[i];
            const off = i*16;
            flatBuffer[off+0] = b.x;
            flatBuffer[off+1] = b.y;
            flatBuffer[off+2] = b.z;
            flatBuffer[off+3] = b.type;
            flatBuffer[off+4] = b.mass;
            flatBuffer[off+5] = b.r;
            flatBuffer[off+6] = b.id;
            flatBuffer[off+7] = b.seed; // inject generated seed to fragment shaders map
            flatBuffer[off+8] = b.vx;
            flatBuffer[off+9] = b.vy;
            flatBuffer[off+10] = b.vz;
            flatBuffer[off+11] = b.ax;
            flatBuffer[off+12] = b.ay;
            flatBuffer[off+13] = b.az;
            flatBuffer[off+14] = b.parentId ?? -1;
            flatBuffer[off+15] = b.radiusKm || (b.r * KM_PER_AU);
        }
        return {
            count: this._bodies.length,
            buffer: flatBuffer,
            bodies: this._bodies,
            timeYears: this._timeYears,
            scenario: this._scenarioName,
            gravityConstant: this.G,
            gravityMode: this._gravityMode,
            physics: { ...this._physics },
            provenance: this._provenance,
            bodyRevision: this._bodyRevision,
        };
    }

    getBody(id) {
        return this._bodies.find((body) => body.id === id) || null;
    }

    getBodies() {
        return this._bodies.slice();
    }

    getDiagnostics() {
        const physicsStatus = this.getPhysicsStatus();
        const activePhysicsCount = physicsStatus.filter((item) => item.active).length;
        const applicablePhysicsCount = physicsStatus.filter((item) => item.applicable).length;
        return {
            tick: this._tick,
            bodyCount: this._bodies.length,
            starCount: this._bodies.filter((b) => b.type === PlanetaryMockBridge.TYPE.STAR).length,
            planetCount: this._bodies.filter((b) => b.type === PlanetaryMockBridge.TYPE.ROCKY_PLANET
                || b.type === PlanetaryMockBridge.TYPE.GAS_GIANT).length,
            dwarfPlanetCount: this._bodies.filter((b) => b.type === PlanetaryMockBridge.TYPE.DWARF_PLANET).length,
            moonCount: this._bodies.filter((b) => b.type === PlanetaryMockBridge.TYPE.MOON && !b.rocheFragment).length,
            smallBodyCount: this._bodies.filter((b) => b.type === PlanetaryMockBridge.TYPE.ASTEROID && !b.rocheFragment).length,
            debrisCount: this._bodies.filter((b) => b.rocheFragment).length,
            timeYears: this._timeYears,
            timeDays: this._timeYears * DAYS_PER_YEAR,
            dt: this._dt,
            dtDays: this._dt * DAYS_PER_YEAR,
            timeStep: this.getTimeStepStatus(),
            gravityMode: this._gravityMode,
            scenario: this._scenarioName,
            provenance: this._provenance,
            units: this._scenarioName === 'planetary-threebody' ? 'natural units' : 'AU · M☉ · Julian yr',
            integrator: 'split-kick Velocity Verlet',
            newtonianEnabled: this._physicsActive('newtonianGravity'),
            activePhysicsCount,
            applicablePhysicsCount,
            availablePhysicsCount: physicsStatus.length,
            physicsStatus,
            forceAudit: { ...this._forceAudit },
            dissipatedEnergy: this._dissipatedEnergy,
            stellarMassLost: this._stellarMassLost,
            collisionCount: this._eventCounts.collisions,
            rocheDisruptionCount: this._eventCounts.rocheDisruptions,
            atmosphereEntryCount: this._eventCounts.atmosphereEntries,
            eventCount: this._events.length,
            latestEvent: this._events[0] ? { ...this._events[0] } : null,
            bodyRevision: this._bodyRevision,
        };
    }
}

import { C_SPEED } from '../../../constants.js';

export const RF_LATTICE_WAVE_SCENARIO_ID = 's0-field-rf-lattice-wave';
export const LIGHT_LATTICE_WAVE_SCENARIO_ID = 's0-field-light-lattice-wave';
export const SOUND_LATTICE_WAVE_SCENARIO_ID = 's0-field-sound-lattice-wave';
export const SOUND_COLLISION_SCENARIO_ID = 's0-field-sound-collision';
export const SOUND_PROXY_SPEED_RATIO = 1 / 8;
export const SOUND_PROXY_SPEED = C_SPEED * SOUND_PROXY_SPEED_RATIO;

const COMPONENT_INDEX = Object.freeze({ x: 0, y: 1, z: 2 });
const SINGLE_SCENARIO_IDS = new Set([
    RF_LATTICE_WAVE_SCENARIO_ID,
    LIGHT_LATTICE_WAVE_SCENARIO_ID,
    SOUND_LATTICE_WAVE_SCENARIO_ID,
    SOUND_COLLISION_SCENARIO_ID,
]);
const SINGLE_WAVE_OVERRIDES = new Map();

export const WAVE_FAMILY_SCENARIO_IDS = Object.freeze([
    RF_LATTICE_WAVE_SCENARIO_ID,
    LIGHT_LATTICE_WAVE_SCENARIO_ID,
    SOUND_LATTICE_WAVE_SCENARIO_ID,
    SOUND_COLLISION_SCENARIO_ID,
]);

export const SINGLE_WAVE_LANES = Object.freeze({
    [RF_LATTICE_WAVE_SCENARIO_ID]: [
        {
            id: 'rf',
            label: 'n=1 transverse lattice mode',
            set: 'rf',
            carrier: 'transverse vector',
            modeN: 1,
            laneFrac: 0,
            sigmaFrac: 0.12,
            amp: 0.034,
            phase: 0.00,
            tag: 'RF',
            component: 'y',
            waveSpeed: C_SPEED,
        },
    ],
    [LIGHT_LATTICE_WAVE_SCENARIO_ID]: [
        {
            id: 'light_visible',
            label: 'n=6 transverse lattice mode',
            set: 'light',
            carrier: 'transverse vector',
            modeN: 6,
            laneFrac: 0,
            sigmaFrac: 0.10,
            amp: 0.032,
            phase: Math.PI * 0.15,
            tag: 'L',
            component: 'y',
            waveSpeed: C_SPEED,
        },
    ],
    [SOUND_LATTICE_WAVE_SCENARIO_ID]: [
        {
            id: 'sound_air_proxy',
            label: 'n=4 longitudinal seed (sound gate)',
            set: 'sound',
            carrier: 'longitudinal vector; no medium',
            modeN: 4,
            laneFrac: 0,
            sigmaFrac: 0.11,
            amp: 0.030,
            phase: Math.PI * 0.10,
            tag: 'snd',
            component: 'x',
            waveSpeed: SOUND_PROXY_SPEED,
            proxy: true,
        },
    ],
    [SOUND_COLLISION_SCENARIO_ID]: [
        {
            id: 'sound_left',
            label: 'Longitudinal packet L',
            set: 'sound',
            carrier: 'longitudinal vector; no medium',
            modeN: 4,
            laneFrac: 0,
            sigmaFrac: 0.11,
            amp: 0.030,
            phase: 0,
            tag: 'snL',
            component: 'x',
            waveSpeed: SOUND_PROXY_SPEED,
            pulseFrac: 0.15, // Make it a short pulse
            pulseCenterOffsetFrac: -0.25, // Start 25% to the left
            proxy: true,
        },
        {
            id: 'sound_right',
            label: 'Longitudinal packet R',
            set: 'sound',
            carrier: 'longitudinal vector; no medium',
            modeN: 4,
            laneFrac: 0,
            sigmaFrac: 0.11,
            amp: 0.030,
            phase: 0,
            tag: 'snR',
            component: 'x',
            waveSpeed: SOUND_PROXY_SPEED,
            speedMultiplier: -1,
            pulseFrac: 0.15, // Make it a short pulse
            pulseCenterOffsetFrac: 0.25, // Start 25% to the right
            proxy: true,
        },
    ],
});

function clampNumber(value, min, max, fallback) {
    const n = Number(value);
    if (!Number.isFinite(n)) return fallback;
    return Math.min(max, Math.max(min, n));
}

function singleWaveBaseLane(scenarioId) {
    return SINGLE_WAVE_LANES[scenarioId]?.[0] || null;
}

export function isSingleWaveScenario(scenarioId) {
    return SINGLE_SCENARIO_IDS.has(scenarioId);
}

export function getWaveScenarioDefaults(scenarioId) {
    const lane = singleWaveBaseLane(scenarioId);
    if (!lane) return null;
    return {
        modeN: lane.modeN,
        amp: lane.amp,
        sigmaFrac: lane.sigmaFrac ?? 0.10,
        pulseFrac: lane.pulseFrac ?? 1.00,
        phase: lane.phase ?? 0,
        speedRatio: lane.waveSpeed ? lane.waveSpeed / C_SPEED : 1,
    };
}

export function sanitizeWaveScenarioSettings(scenarioId, settings = {}) {
    const defaults = getWaveScenarioDefaults(scenarioId);
    if (!defaults) return null;
    const speedMin = scenarioId === SOUND_LATTICE_WAVE_SCENARIO_ID ? 0.04 : 1;
    const speedMax = scenarioId === SOUND_LATTICE_WAVE_SCENARIO_ID ? 0.5 : 1;
    return {
        modeN: Math.round(clampNumber(settings.modeN, 1, 24, defaults.modeN)),
        amp: clampNumber(settings.amp, 0.001, 0.120, defaults.amp),
        sigmaFrac: clampNumber(settings.sigmaFrac, 0.025, 0.240, defaults.sigmaFrac),
        pulseFrac: clampNumber(settings.pulseFrac, 0.120, 1.000, defaults.pulseFrac),
        phase: clampNumber(settings.phase, 0, Math.PI * 2, defaults.phase),
        speedRatio: clampNumber(settings.speedRatio, speedMin, speedMax, defaults.speedRatio),
    };
}

export function getWaveScenarioSettings(scenarioId) {
    const defaults = getWaveScenarioDefaults(scenarioId);
    if (!defaults) return null;
    return sanitizeWaveScenarioSettings(scenarioId, {
        ...defaults,
        ...(SINGLE_WAVE_OVERRIDES.get(scenarioId) || {}),
    });
}

export function setWaveScenarioSettings(scenarioId, patch = {}) {
    if (!isSingleWaveScenario(scenarioId)) return null;
    const next = sanitizeWaveScenarioSettings(scenarioId, {
        ...getWaveScenarioSettings(scenarioId),
        ...patch,
    });
    SINGLE_WAVE_OVERRIDES.set(scenarioId, next);
    return next;
}

export function resetWaveScenarioSettings(scenarioId) {
    SINGLE_WAVE_OVERRIDES.delete(scenarioId);
    return getWaveScenarioSettings(scenarioId);
}

export function waveFamilyLanesForScenario(scenarioId = RF_LATTICE_WAVE_SCENARIO_ID) {
    const lanes = SINGLE_WAVE_LANES[scenarioId];
    if (!lanes || lanes.length === 0) return [];
    const settings = getWaveScenarioSettings(scenarioId);
    return lanes.map(baseLane => ({
        ...baseLane,
        modeN: settings.modeN,
        amp: settings.amp,
        sigmaFrac: settings.sigmaFrac,
        pulseFrac: settings.pulseFrac,
        phase: baseLane.phaseOffset ? settings.phase + baseLane.phaseOffset : settings.phase,
        waveSpeed: baseLane.waveSpeed !== undefined ? baseLane.waveSpeed : C_SPEED * settings.speedRatio,
    }));
}

export function spectrumComparatorLaneParams(N, midF = (N - 1) / 2, scenarioId = RF_LATTICE_WAVE_SCENARIO_ID) {
    const span = Math.max(1, N - 1);
    return waveFamilyLanesForScenario(scenarioId).map((lane) => {
        const sigma = Math.max(1.15, N * (lane.sigmaFrac ?? 0.045));
        const modeN = Math.max(1, Math.min(Math.floor(N / 2) - 1, lane.modeN));
        const k = 2 * Math.PI * modeN / N;
        const seedWaveSpeed = lane.waveSpeed ?? C_SPEED;
        const sinHalfK = Math.abs(Math.sin(k / 2));
        const seedOmega = 2 * seedWaveSpeed * sinHalfK;
        const nativeOmega = 2 * Math.asin(C_SPEED * sinHalfK);
        const frequency = nativeOmega / (2 * Math.PI);
        const nativeGroupVelocity = C_SPEED * Math.cos(k / 2)
            / Math.sqrt(1 - C_SPEED * C_SPEED * sinHalfK * sinHalfK);
        const componentIndex = COMPONENT_INDEX[lane.component] ?? 1;
        return {
            ...lane,
            modeN,
            componentIndex,
            y: Math.round(midF + lane.laneFrac * span),
            z: Math.round(midF),
            sigma,
            pulseFrac: lane.pulseFrac ?? 1.0,
            pulseSigma: Math.max(1.5, N * (lane.pulseFrac ?? 1.0) * 0.5),
            pulseCenterOffsetFrac: lane.pulseCenterOffsetFrac ?? 0,
            pulseActive: (lane.pulseFrac ?? 1.0) < 0.985,
            amplitude: lane.amp,
            lambda: N / modeN,
            k,
            omega: nativeOmega,
            nativeOmega,
            seedOmega,
            frequency,
            phaseVelocity: k > 0 ? nativeOmega / k : 0,
            groupVelocity: nativeGroupVelocity,
            speedRatioToLight: nativeGroupVelocity / C_SPEED,
            seedSpeedRatioToLight: seedWaveSpeed / C_SPEED,
        };
    });
}

export function seedSpectrumComparator(harness, ctx, scenarioId = RF_LATTICE_WAVE_SCENARIO_ID) {
    const { N, midF } = ctx;
    harness.setToggle('wave_propagation', true);
    harness.setToggle('coupling', false);
    harness.setToggle('damping', false);
    harness.setToggle('selective_damping', false);
    harness.setToggle('genesis', false);
    harness.setToggle('gauss_projection', false);
    harness.setToggle('forces', false);
    harness.setToggle('movement', false);
    harness.setToggle('poisson_coulomb', false);
    harness.setToggle('lorentz_force', false);
    harness.setToggle('emergent_forces', false);

    const lanes = spectrumComparatorLaneParams(N, midF, scenarioId);
    for (const lane of lanes) {
        const cut = lane.sigma * 2.4;
        const cut2 = cut * cut;
        const pulseActive = lane.pulseFrac < 0.985;
        for (let z = Math.max(0, Math.floor(lane.z - cut)); z <= Math.min(N - 1, Math.ceil(lane.z + cut)); z++)
        for (let y = Math.max(0, Math.floor(lane.y - cut)); y <= Math.min(N - 1, Math.ceil(lane.y + cut)); y++)
        for (let x = 0; x < N; x++) {
            const dy = y - lane.y;
            const dz = z - lane.z;
            const r2 = dy * dy + dz * dz;
            if (r2 > cut2) continue;
            const centerOffsetX = (lane.pulseCenterOffsetFrac || 0) * N;
            const dx = x - (midF + centerOffsetX);
            const gx = pulseActive
                ? Math.exp(-(dx * dx) / (2 * lane.pulseSigma * lane.pulseSigma))
                : 1;
            const g = gx * Math.exp(-r2 / (2 * lane.sigma * lane.sigma));
            if (g < 1e-4) continue;
            const phase = lane.k * x + lane.phase;
            const j = lane.amp * g * Math.sin(phase);
            const direction = lane.speedMultiplier ?? 1;
            const w = lane.proxy
                ? direction * (-lane.seedOmega * lane.amp * g * Math.cos(phase))
                : lane.amp * g * ((1 - Math.cos(lane.nativeOmega)) * Math.sin(phase)
                    - direction * Math.sin(lane.nativeOmega) * Math.cos(phase));
            const jf = [0, 0, 0];
            const wv = [0, 0, 0];
            jf[lane.componentIndex] = j;
            wv[lane.componentIndex] = w;
            if (Math.abs(j) > 1e-12) harness.injectFlux(x, y, z, jf[0], jf[1], jf[2]);
            if (Math.abs(w) > 1e-12) harness.injectWaveVel(x, y, z, wv[0], wv[1], wv[2]);
        }
    }
}

// Reduce sparse vector samplers directly into the small lane accumulator set.
// The former implementation allocated one Map entry plus one three-number
// Array for every nonzero voxel, then scanned the lane volume and performed a
// hash lookup at every coordinate. At L=97 that created hundreds of thousands
// of short-lived objects every 250 ms. A single typed-array pass is exact for
// the same sparse samples (omitted vectors are defined as zero by the WASM
// sampler) and keeps the foreground panel update bounded.
function reduceVectorSample(sample, laneRows, N, { wave = false, stride = 1 } = {}) {
    if (!sample?.positions || !sample?.vectors) return;
    const positions = sample.positions;
    const vectors = sample.vectors;
    const count = Math.min(
        Math.max(0, Math.trunc(Number(sample.count) || 0)),
        Math.floor(positions.length / 3),
        Math.floor(vectors.length / 3),
    );
    const sign = wave ? -1 : 1;
    const volumeWeight = stride * stride * stride;
    const probeX = Math.round((N - 1) / 2);

    for (let i = 0; i < count; i++) {
        const offset = i * 3;
        const x = Math.floor(positions[offset]);
        const y = Math.floor(positions[offset + 1]);
        const z = Math.floor(positions[offset + 2]);
        const vx = sign * vectors[offset];
        const vy = sign * vectors[offset + 1];
        const vz = sign * vectors[offset + 2];
        const rawEnergy = 0.5 * (vx * vx + vy * vy + vz * vz);
        const vectorEnergy = rawEnergy * volumeWeight;
        const magnitude = Math.sqrt(2 * rawEnergy);

        for (const row of laneRows) {
            if (Math.abs(y - row.y) > row._band || Math.abs(z - row.z) > row._band) continue;
            const directional = row.componentIndex === 0 ? vx
                : (row.componentIndex === 1 ? vy : vz);
            if (wave) {
                row.waveEnergy += vectorEnergy;
                row.peakWaveVel = Math.max(row.peakWaveVel, magnitude);
                row.peakDirectionalWaveVel = Math.max(
                    row.peakDirectionalWaveVel,
                    Math.abs(directional),
                );
                if (x === probeX && y === row.y && z === row.z) {
                    row.sampleWaveVel = directional;
                    row.sampleWy = vy;
                }
            } else {
                row.fieldEnergy += vectorEnergy;
                row.peakFlux = Math.max(row.peakFlux, magnitude);
                row.peakDirectionalFlux = Math.max(row.peakDirectionalFlux, Math.abs(directional));
                if (y === row.y && z === row.z && x >= 0 && x < N) {
                    row._lineFlux[x] = directional;
                }
                if (x === probeX && y === row.y && z === row.z) {
                    row.sampleFlux = directional;
                    row.sampleJy = vy;
                }
            }
            row.energy += vectorEnergy;
            row._energyX += x * vectorEnergy;
        }
    }
}

function selectMetricStride(N) {
    if (N <= 33) return 1;
    const mid = Math.round((N - 1) / 2);
    const target = Math.max(2, Math.ceil(N / 33));
    // Prefer a stride that includes the exact center line used by probe and
    // harmonic readouts. All supported lattice sizes are odd, so `mid` is an
    // integer and a nearby divisor normally exists.
    for (let stride = target; stride <= Math.min(8, mid); stride++) {
        if (mid % stride === 0) return stride;
    }
    for (let stride = target - 1; stride >= 2; stride--) {
        if (mid % stride === 0) return stride;
    }
    return target;
}

export function getSpectrumComparatorMetrics(bridge, scenarioId = RF_LATTICE_WAVE_SCENARIO_ID) {
    if (typeof bridge?.getFluxVectorSampled !== 'function' || typeof bridge?.getEFieldSampled !== 'function') {
        return { active: false, reason: 'no field buffers' };
    }
    const N = bridge.latticeSize || 33;
    const sampleStride = selectMetricStride(N);
    const lanes = spectrumComparatorLaneParams(N, (N - 1) / 2, scenarioId);
    if (lanes.length === 0) {
        return { active: false, reason: 'not a wave-family scenario' };
    }
    const laneRows = lanes.map((lane) => {
        const band = Math.max(1, Math.ceil(lane.sigma * 2.4));
        return {
            id: lane.id,
            label: lane.label,
            set: lane.set,
            carrier: lane.carrier,
            tag: lane.tag,
            modeN: lane.modeN,
            component: lane.component,
            y: lane.y,
            z: lane.z,
            amplitude: lane.amplitude,
            sigma: lane.sigma,
            sigmaFrac: lane.sigmaFrac,
            pulseFrac: lane.pulseFrac,
            pulseSigma: lane.pulseSigma,
            pulseActive: lane.pulseFrac < 0.985,
            lambda: lane.lambda,
            frequency: lane.frequency,
            omega: lane.omega,
            phaseVelocity: lane.phaseVelocity,
            groupVelocity: lane.groupVelocity,
            speedRatioToLight: lane.speedRatioToLight,
            proxy: !!lane.proxy,
            fieldEnergy: 0,
            waveEnergy: 0,
            energy: 0,
            energyCentroidX: 0,
            peakFlux: 0,
            peakDirectionalFlux: 0,
            peakWaveVel: 0,
            peakDirectionalWaveVel: 0,
            sampleFlux: 0,
            sampleWaveVel: 0,
            sampleJy: 0,
            sampleWy: 0,
            harmonics: [],
            _band: band,
            _energyX: 0,
            _lineFlux: new Float64Array(N),
        };
    });

    // J = flux (live sample); W = wave_vel = -E (established convention, see
    // diagnostics_compute.cpp) since there is no direct wave_vel sampler.
    reduceVectorSample(
        bridge.getFluxVectorSampled(sampleStride),
        laneRows,
        N,
        { stride: sampleStride },
    );
    reduceVectorSample(
        bridge.getEFieldSampled(sampleStride),
        laneRows,
        N,
        { wave: true, stride: sampleStride },
    );

    let totalLaneEnergy = 0;
    for (const row of laneRows) {
        row.energyCentroidX = row.energy > 0 ? row._energyX / row.energy : 0;
        for (let m = 1; m <= 8; m++) {
            let re = 0;
            let im = 0;
            const kMode = 2 * Math.PI * m / N;
            for (let x = 0; x < N; x++) {
                const val = row._lineFlux[x];
                re += val * Math.cos(kMode * x);
                im -= val * Math.sin(kMode * x);
            }
            row.harmonics.push(Math.sqrt(re * re + im * im) * 2 * sampleStride / N);
        }
        totalLaneEnergy += row.energy;
        delete row._band;
        delete row._energyX;
        delete row._lineFlux;
    }

    const sets = new Map();
    for (const lane of laneRows) {
        lane.energyShare = totalLaneEnergy > 0 ? lane.energy / totalLaneEnergy : 0;
        const key = lane.set || 'other';
        const entry = sets.get(key) || {
            id: key,
            laneCount: 0,
            energy: 0,
            peakFlux: 0,
            carrier: lane.carrier,
            speedRatioToLight: lane.speedRatioToLight,
            labels: [],
        };
        entry.laneCount += 1;
        entry.energy += lane.energy;
        entry.peakFlux = Math.max(entry.peakFlux, lane.peakFlux);
        entry.labels.push(lane.label);
        sets.set(key, entry);
    }
    const setRows = Array.from(sets.values()).map((entry) => ({
        ...entry,
        energyShare: totalLaneEnergy > 0 ? entry.energy / totalLaneEnergy : 0,
    }));

    return {
        active: true,
        tick: bridge.currentTick?.() ?? 0,
        latticeSize: N,
        sampleStride,
        samplingMode: sampleStride === 1 ? 'exact' : 'stride-estimate',
        cSpeed: C_SPEED,
        scenarioId,
        singleScenario: lanes.length === 1,
        soundProxySpeed: SOUND_PROXY_SPEED,
        soundProxySpeedRatio: SOUND_PROXY_SPEED_RATIO,
        totalLaneEnergy,
        lanes: laneRows,
        sets: setRows,
        ratios: {},
        toggles: {
            wave_propagation: !!bridge.getToggle?.('wave_propagation'),
            damping: !!bridge.getToggle?.('damping'),
            genesis: !!bridge.getToggle?.('genesis'),
        },
    };
}

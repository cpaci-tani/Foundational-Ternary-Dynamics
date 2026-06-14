import { C_SPEED } from '../../constants.js';

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
            label: 'RF lattice wave',
            set: 'rf',
            carrier: 'EM transverse',
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
            label: 'Light lattice wave',
            set: 'light',
            carrier: 'EM transverse',
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
            label: 'Sound lattice proxy',
            set: 'sound',
            carrier: 'longitudinal medium',
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
            label: 'Sound proxy L',
            set: 'sound',
            carrier: 'longitudinal medium',
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
            label: 'Sound proxy R',
            set: 'sound',
            carrier: 'longitudinal medium',
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
        const waveSpeed = lane.waveSpeed ?? C_SPEED;
        const omega = 2 * waveSpeed * Math.abs(Math.sin(k / 2));
        const frequency = omega / (2 * Math.PI);
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
            omega,
            frequency,
            phaseVelocity: k > 0 ? omega / k : 0,
            groupVelocity: waveSpeed * Math.cos(k / 2),
            speedRatioToLight: waveSpeed / C_SPEED,
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
            const w = -lane.omega * lane.amp * g * Math.cos(phase);
            const jf = [0, 0, 0];
            const wv = [0, 0, 0];
            jf[lane.componentIndex] = j;
            wv[lane.componentIndex] = w;
            if (Math.abs(j) > 1e-12) harness.injectFlux(x, y, z, jf[0], jf[1], jf[2]);
            if (Math.abs(w) > 1e-12) harness.injectWaveVel(x, y, z, wv[0], wv[1], wv[2]);
        }
    }
}

export function getSpectrumComparatorMetrics(bridge, scenarioId = RF_LATTICE_WAVE_SCENARIO_ID) {
    if (!bridge?._fluxJ || !bridge?._fluxWV) {
        return { active: false, reason: 'no field buffers' };
    }
    const N = bridge.latticeSize || 33;
    const lanes = spectrumComparatorLaneParams(N, (N - 1) / 2, scenarioId);
    if (lanes.length === 0) {
        return { active: false, reason: 'not a wave-family scenario' };
    }
    const J = bridge._fluxJ;
    const WV = bridge._fluxWV;
    const idxOf = (x, y, z) => bridge._fluxIdx(x, y, z) * 3;
    let totalLaneEnergy = 0;

    const laneRows = lanes.map((lane) => {
        const band = Math.max(1, Math.ceil(lane.sigma * 2.4));
        let fieldEnergy = 0;
        let waveEnergy = 0;
        let peakFlux = 0;
        let peakDirectionalFlux = 0;
        let peakWaveVel = 0;
        let peakDirectionalWaveVel = 0;
        let sx = 0;
        let energy = 0;
        for (let z = Math.max(0, lane.z - band); z <= Math.min(N - 1, lane.z + band); z++)
        for (let y = Math.max(0, lane.y - band); y <= Math.min(N - 1, lane.y + band); y++)
        for (let x = 0; x < N; x++) {
            const base = idxOf(x, y, z);
            const jx = J[base] || 0;
            const jy = J[base + 1] || 0;
            const jz = J[base + 2] || 0;
            const wx = WV[base] || 0;
            const wy = WV[base + 1] || 0;
            const wz = WV[base + 2] || 0;
            const fE = 0.5 * (jx * jx + jy * jy + jz * jz);
            const wE = 0.5 * (wx * wx + wy * wy + wz * wz);
            const e = fE + wE;
            fieldEnergy += fE;
            waveEnergy += wE;
            energy += e;
            sx += x * e;
            const dirJ = J[base + lane.componentIndex] || 0;
            const dirW = WV[base + lane.componentIndex] || 0;
            peakFlux = Math.max(peakFlux, Math.sqrt(jx * jx + jy * jy + jz * jz));
            peakDirectionalFlux = Math.max(peakDirectionalFlux, Math.abs(dirJ));
            peakWaveVel = Math.max(peakWaveVel, Math.sqrt(wx * wx + wy * wy + wz * wz));
            peakDirectionalWaveVel = Math.max(peakDirectionalWaveVel, Math.abs(dirW));
        }
        totalLaneEnergy += energy;
        const probe = idxOf(Math.round((N - 1) / 2), lane.y, lane.z);

        // 1D Spatial DFT for Additive Synthesis
        const harmonics = [];
        const numHarmonics = 8;
        for (let m = 1; m <= numHarmonics; m++) {
            let re = 0, im = 0;
            const kMode = 2 * Math.PI * m / N;
            for (let x = 0; x < N; x++) {
                const base = idxOf(x, lane.y, lane.z);
                const val = J[base + lane.componentIndex] || 0;
                re += val * Math.cos(kMode * x);
                im -= val * Math.sin(kMode * x);
            }
            harmonics.push(Math.sqrt(re * re + im * im) * 2 / N);
        }

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
            fieldEnergy,
            waveEnergy,
            energy,
            energyCentroidX: energy > 0 ? sx / energy : 0,
            peakFlux,
            peakDirectionalFlux,
            peakWaveVel,
            peakDirectionalWaveVel,
            sampleFlux: J[probe + lane.componentIndex] || 0,
            sampleWaveVel: WV[probe + lane.componentIndex] || 0,
            sampleJy: J[probe + 1] || 0,
            sampleWy: WV[probe + 1] || 0,
            harmonics,
        };
    });

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
        tick: bridge._tick ?? bridge.currentTick?.() ?? 0,
        latticeSize: N,
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
            wave_propagation: !!bridge._toggles?.wave_propagation,
            damping: !!bridge._toggles?.damping,
            genesis: !!bridge._toggles?.genesis,
        },
    };
}

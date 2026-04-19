/**
 * Consciousness audio subsystem — WebAudio synthesis driven by FTD constants.
 *
 * Extracted from consciousness.js (ticket CE-1).
 *
 * Frequencies derived from FTD consciousness theory:
 *   1/α ≈ 137.036 Hz — strikingly close to Om/Aum (136.1 Hz)
 *   G* ≈ 2.959 — frequency ratio (nearly a perfect 12th = octave+fifth)
 *   Framework integers {3,4,7,13} — Pythagorean overtone partials
 *   θ_C ≈ 52.54° — consciousness phase angle
 *   cos²(θ_C) ≈ 0.37 — observable fraction (gain ratio)
 *   K_C ≈ 3.599 — consciousness threshold (frequency multiplier)
 *   C_MANDELBROT ≈ 0.338 — subharmonic ratio (deep bass)
 *
 * Historical sound traditions:
 *   Om/Aum drone (136.1 Hz) ≈ 1/α — the cosmic hum
 *   Schumann resonance (7.83 Hz) ≈ b₃=7 — Earth's EM cavity
 *   Pythagorean tuning: 3:2 (fifth), 4:3 (fourth) — framework integers
 *   Binaural beats: theta (4-8 Hz), alpha (8-13 Hz) — brainwave entrainment
 *   Tibetan singing bowls: beating overtones, long reverb
 *   Gamelan: metallic shimmer from paired detuned tones
 *   Overtone/throat singing: emphasized upper partials
 *   Indian tanpura: slow-evolving harmonic bath
 */

import * as THREE from 'three';
import {
    COS2_THETA_C, THETA_C_RAD, Y_REAL, Y_IMAG,
    K_C, C_MANDELBROT, G_STAR, ALPHA,
} from '../constants.js';

// ── Lemniscate-Alpha Curve ───────────────────────────────────────────

export class LemniscateAlphaCurve extends THREE.Curve {
    constructor(scale = 1) {
        super();
        this.scale = scale;
    }

    getPoint(t, optionalTarget = new THREE.Vector3()) {
        const a = t * 2 * Math.PI;
        const px = Math.cos(a) + 0.5 * Math.cos(2*a) + 0.5 * Math.cos(4*a) + 0.375 * Math.cos(8*a);
        const py = 2 * Math.sin(a) - Math.sin(2*a) + Math.sin(4*a) - 0.75 * Math.sin(8*a);
        return optionalTarget.set(px * this.scale, py * this.scale, 0);
    }
}

// ── FTD-Derived Audio System ─────────────────────────────────────────

export const FTD_FREQ  = 1.0 / ALPHA;                           // ≈ 137.036 Hz
export const FTD_SUB   = FTD_FREQ * C_MANDELBROT;               // ≈ 46.3 Hz
export const FTD_DELTA = Math.sqrt((4*G_STAR - 1)/(4*G_STAR));  // ≈ 0.957

/** Generate algorithmic reverb impulse response (no external files needed) */
export function createReverbIR(ctx, duration, decay) {
    const rate = ctx.sampleRate;
    const len = Math.floor(rate * duration);
    const buf = ctx.createBuffer(2, len, rate);
    for (let ch = 0; ch < 2; ch++) {
        const d = buf.getChannelData(ch);
        for (let i = 0; i < len; i++) {
            d[i] = (Math.random() * 2 - 1) * Math.pow(1 - i / len, decay);
        }
    }
    return buf;
}

/** Generate waveshaper curves for different timbres */
export function makeShaperCurve(type, drive, n = 4096) {
    const curve = new Float32Array(n);
    for (let i = 0; i < n; i++) {
        const x = (i * 2) / n - 1;
        switch (type) {
            case 'soft':   // Tanpura/drone: warm saturation
                curve[i] = Math.tanh(x * drive);
                break;
            case 'hard':   // Gamelan/metallic: aggressive clip
                curve[i] = Math.max(-1, Math.min(1, x * drive));
                break;
            case 'bowl': { // Singing bowl: odd-harmonic emphasis
                const s = Math.tanh(x * drive * 0.8);
                curve[i] = s + 0.15 * Math.sin(x * Math.PI * 3)
                             + 0.08 * Math.sin(x * Math.PI * 7);
                break;
            }
            default:
                curve[i] = x;
        }
    }
    return curve;
}

// ── Audio Profiles: Per-Scenario Sound Design ────────────────────────
// Each scenario maps to a unique sonic identity derived from FTD theory

export const AUDIO_PROFILES = {
    'cs-threshold': {
        // Phase Transition: silence → rising Om drone → Pythagorean bloom
        baseFreq: FTD_SUB,  // start at 46 Hz (sub-threshold)
        oscs: [
            { type: 'sine',     ratio: 1,    gain: 0.5  },
            { type: 'sine',     ratio: 3/2,  gain: 0, bloomMax: 0.20 },  // Pythagorean 5th
            { type: 'triangle', ratio: 4/3,  gain: 0, bloomMax: 0.15 },  // Pythagorean 4th
            { type: 'sine',     ratio: 7/4,  gain: 0, bloomMax: 0.10 },  // Harmonic 7th
            { type: 'sine',     ratio: 13/8, gain: 0, bloomMax: 0.08 },  // Tridecimal (N_EFF)
        ],
        filterFreq: 400, filterQ: 1.0,
        shaper: 'soft', drive: 1.5,
        reverbWet: 0.2, reverbTime: 2.5,
        lfoRate: 7.83, lfoDepth: 0.03,  // Schumann resonance
        pitchMode: 'threshold',
    },
    'cs-high-coupling': {
        // Dense Gamelan Shimmer: Balinese paired-detuned metallophones
        baseFreq: FTD_FREQ,
        oscs: [
            { type: 'sine',     ratio: 1,          gain: 0.25, detune: 0  },
            { type: 'sine',     ratio: 1,          gain: 0.25, detune: 3  },  // beating +3¢
            { type: 'triangle', ratio: G_STAR,      gain: 0.12, detune: 0  },
            { type: 'triangle', ratio: G_STAR,      gain: 0.12, detune: -4 },  // beating -4¢
            { type: 'sawtooth', ratio: K_C/G_STAR,  gain: 0.06, detune: 2  },
            { type: 'sine',     ratio: 13/8,        gain: 0.05, detune: -2 },  // tridecimal
        ],
        filterFreq: 3000, filterQ: 0.5,
        shaper: 'hard', drive: 2.5,
        reverbWet: 0.3, reverbTime: 1.5,
        lfoRate: 11, lfoDepth: 0.04,  // alpha-wave band
        pitchMode: 'subtle',
    },
    'cs-self-ref': {
        // Overtone Singing + Binaural Theta Beat (Tuvan throat singing)
        baseFreq: FTD_FREQ,
        oscs: [
            { type: 'sine', ratio: 1,  gain: 0.35, pan: -1.0 },                  // L ear drone
            { type: 'sine', ratio: 1,  gain: 0.35, pan: 1.0, freqOffset: 6 },    // R ear +6Hz → θ beat
            { type: 'sine', ratio: 7,  gain: 0.08 },   // 7th overtone (b₃ partial)
            { type: 'sine', ratio: 13, gain: 0.04 },   // 13th overtone (N_EFF partial)
        ],
        filterFreq: 1200, filterQ: 3.0,  // resonant peak for overtone emphasis
        shaper: 'bowl', drive: 2.0,
        reverbWet: 0.35, reverbTime: 3.0,
        lfoRate: THETA_C_RAD,  // θ_C ≈ 0.917 Hz — slow sLoop pulse
        lfoDepth: 0.06,
        pitchMode: 'none',
    },
    'cs-nested-sloop': {
        // Deep Tibetan Singing Bowl: double binaural beat
        baseFreq: FTD_FREQ / 2,  // 68.5 Hz sub-octave
        oscs: [
            { type: 'sine',     ratio: 1,          gain: 0.30, pan: -1.0 },                  // L fund
            { type: 'sine',     ratio: 1,          gain: 0.30, pan: 1.0, freqOffset: 4 },    // R +4Hz delta
            { type: 'sine',     ratio: Y_REAL,     gain: 0.10, pan: -0.6 },                  // Y_REAL overtone
            { type: 'sine',     ratio: Y_IMAG,     gain: 0.08, pan: 0.6, freqOffset: 7 },    // Y_IMAG +7Hz θ
            { type: 'triangle', ratio: K_C/G_STAR, gain: 0.06 },                             // mid shimmer
        ],
        filterFreq: 800, filterQ: 2.0,
        shaper: 'bowl', drive: 1.8,
        reverbWet: 0.5, reverbTime: 4.0,  // long tail
        lfoRate: COS2_THETA_C,  // ≈ 0.37 Hz — slow breathing
        lfoDepth: 0.08,
        pitchMode: 'none',
    },
    'cs-chirality': {
        // Stereo-Split Duality: L/R substrates in separate ears
        // Historical: antiphonal chant, call-and-response
        baseFreq: FTD_FREQ,
        oscs: [
            { type: 'sine',     ratio: 1.05,  gain: 0.30, pan: -0.85 },  // L substrate (slightly sharp)
            { type: 'triangle', ratio: 1.05,  gain: 0.12, pan: -0.85 },
            { type: 'sine',     ratio: 0.95,  gain: 0.30, pan: 0.85 },   // R substrate (slightly flat)
            { type: 'triangle', ratio: 0.95,  gain: 0.12, pan: 0.85 },
            { type: 'sine',     ratio: G_STAR, gain: 0.06 },             // center bridge tone
        ],
        filterFreq: 1800, filterQ: 1.0,
        shaper: 'soft', drive: 1.3,
        reverbWet: 0.25, reverbTime: 2.0,
        lfoRate: 3.0, lfoDepth: 0.05,
        pitchMode: 'subtle',
    },
    'cs-boundary-orbit': {
        // Mandelbrot Chaos: chaotic/cyclic texture modulated by |z|
        baseFreq: FTD_SUB,  // 46 Hz deep
        oscs: [
            { type: 'sawtooth', ratio: 1,         gain: 0.25 },
            { type: 'sine',     ratio: G_STAR,     gain: 0.15 },
            { type: 'triangle', ratio: C_MANDELBROT * 7, gain: 0.10 },  // b₃ × C_M partial
            { type: 'sine',     ratio: 4/3,        gain: 0.08 },
        ],
        filterFreq: 1000, filterQ: 2.5,
        shaper: 'hard', drive: 2.0,
        reverbWet: 0.3, reverbTime: 1.8,
        lfoRate: 5.0, lfoDepth: 0.05,
        pitchMode: 'mandelbrot',
    },
    'cs-entangled': {
        // Phase-Locked Mirror Pairs: quantum-correlated harmonics
        baseFreq: FTD_FREQ,
        oscs: [
            { type: 'sine',     ratio: 1,      gain: 0.30, pan: -0.5 },
            { type: 'sine',     ratio: 1,      gain: 0.30, pan: 0.5, detune: 1 },  // near-unison
            { type: 'triangle', ratio: G_STAR,  gain: 0.12, pan: -0.5 },
            { type: 'triangle', ratio: G_STAR,  gain: 0.12, pan: 0.5, detune: -1 },
        ],
        filterFreq: 1500, filterQ: 1.5,
        shaper: 'soft', drive: 1.5,
        reverbWet: 0.35, reverbTime: 2.5,
        lfoRate: 7.0, lfoDepth: 0.05,  // b₃ = 7 Hz (Schumann-adjacent)
        pitchMode: 'subtle',
    },
    'cs-flow': {
        // Rhythmic Pentatonic Shimmer: gamelan-like, active alpha-wave
        // Historical: Balinese kecak, metallic pentatonic
        baseFreq: FTD_FREQ,
        oscs: [
            { type: 'triangle', ratio: 1,    gain: 0.25 },
            { type: 'triangle', ratio: 3/2,  gain: 0.20, detune: 2 },   // 5th
            { type: 'sine',     ratio: 4/3,  gain: 0.15, detune: -2 },  // 4th
            { type: 'sawtooth', ratio: 7/4,  gain: 0.08 },              // 7th harmonic
        ],
        filterFreq: 2500, filterQ: 0.8,
        shaper: 'hard', drive: 1.8,
        reverbWet: 0.15, reverbTime: 1.0,  // dry, present
        lfoRate: 10, lfoDepth: 0.06,  // alpha wave 10 Hz
        pitchMode: 'subtle',
    },
    'cs-meditation': {
        // Tanpura Drone + Theta Binaural: subject-dominant stillness
        // Historical: Indian tanpura, Tibetan bowls, theta entrainment
        baseFreq: FTD_FREQ / 2,  // 68.5 Hz sub-octave
        oscs: [
            { type: 'sine',     ratio: 1,     gain: 0.35, pan: -1.0 },                  // L drone
            { type: 'sine',     ratio: 1,     gain: 0.35, pan: 1.0, freqOffset: 6 },    // R +6Hz theta
            { type: 'sine',     ratio: 3,     gain: 0.08 },   // 3rd harmonic
            { type: 'sine',     ratio: 4,     gain: 0.06 },   // 4th harmonic (tanpura character)
            { type: 'triangle', ratio: 7,     gain: 0.03 },   // 7th harmonic (b₃)
        ],
        filterFreq: 600, filterQ: 1.5,
        shaper: 'bowl', drive: 2.2,
        reverbWet: 0.5, reverbTime: 4.0,   // deep, spacious
        lfoRate: 0.1, lfoDepth: 0.04,  // glacial breathing
        pitchMode: 'none',
    },
    'cs-custom': {
        // Minimal Ambient: single quiet Om tone
        baseFreq: FTD_FREQ,
        oscs: [
            { type: 'sine', ratio: 1, gain: 0.3 },
        ],
        filterFreq: 2000, filterQ: 0.5,
        shaper: null, drive: 1.0,
        reverbWet: 0.15, reverbTime: 1.5,
        lfoRate: 2.0, lfoDepth: 0.02,
        pitchMode: 'none',
    },
};

// ── Audio graph construction ─────────────────────────────────────────

/**
 * Build the WebAudio graph for a given profile.
 * Mutates `engine` by assigning audio node fields.
 *
 * @param {object} engine - ConsciousnessEngine instance (receives _masterGain, _filter, etc.)
 * @param {string} profileName - profile key in AUDIO_PROFILES
 */
export function buildAudioGraph(engine, profileName) {
    const ctx = engine._audioCtx;
    const p = AUDIO_PROFILES[profileName] || AUDIO_PROFILES['cs-custom'];

    // ── Master output chain ──
    // masterGain → destination
    engine._masterGain = ctx.createGain();
    engine._masterGain.gain.value = 0.12;
    engine._masterGain.connect(ctx.destination);

    // ── Dry/Wet reverb split ──
    engine._dryGain = ctx.createGain();
    engine._dryGain.gain.value = 1.0 - p.reverbWet;
    engine._dryGain.connect(engine._masterGain);

    engine._wetGain = ctx.createGain();
    engine._wetGain.gain.value = p.reverbWet;
    engine._wetGain.connect(engine._masterGain);

    // Algorithmic reverb (convolver with generated IR)
    engine._convolver = ctx.createConvolver();
    engine._convolver.buffer = createReverbIR(ctx, p.reverbTime, 3.0);
    engine._convolver.connect(engine._wetGain);

    // ── Filter ──
    engine._filter = ctx.createBiquadFilter();
    engine._filter.type = 'lowpass';
    engine._filter.frequency.value = p.filterFreq;
    engine._filter.Q.value = p.filterQ;
    engine._filter.connect(engine._dryGain);
    engine._filter.connect(engine._convolver);

    // ── Waveshaper (optional overtone enrichment) ──
    const oscTarget = (() => {
        if (p.shaper) {
            engine._shaper = ctx.createWaveShaper();
            engine._shaper.curve = makeShaperCurve(p.shaper, p.drive);
            engine._shaper.oversample = '2x';
            engine._shaper.connect(engine._filter);
            return engine._shaper;
        }
        return engine._filter;
    })();

    // ── Oscillator bank ──
    for (let i = 0; i < p.oscs.length; i++) {
        const def = p.oscs[i];
        const osc = ctx.createOscillator();
        osc.type = def.type;

        let freq = p.baseFreq * def.ratio;
        if (def.freqOffset) freq += def.freqOffset;
        osc.frequency.value = freq;
        if (def.detune) osc.detune.value = def.detune;

        const gain = ctx.createGain();
        gain.gain.value = def.gain;
        osc.connect(gain);

        // Per-oscillator stereo panning (for binaural beats, chirality)
        let panner = null;
        if (def.pan !== undefined && def.pan !== 0) {
            panner = ctx.createStereoPanner();
            panner.pan.value = def.pan;
            gain.connect(panner);
            panner.connect(oscTarget);
        } else {
            gain.connect(oscTarget);
        }

        engine._oscBank.push({
            osc, gain, panner,
            ratio: def.ratio,
            freqOffset: def.freqOffset || 0,
            basePan: def.pan || 0,
            bloomMax: def.bloomMax || 0,
        });
        osc.start();
    }

    // ── LFO → master gain modulation ──
    if (p.lfoRate > 0) {
        engine._lfo = ctx.createOscillator();
        engine._lfo.frequency.value = p.lfoRate;
        engine._lfoGain = ctx.createGain();
        engine._lfoGain.gain.value = p.lfoDepth;
        engine._lfo.connect(engine._lfoGain);
        engine._lfoGain.connect(engine._masterGain.gain);
        engine._lfo.start();
    }
}

/**
 * Tear down and null out all audio nodes on the engine.
 */
export function teardownAudioGraph(engine) {
    // Stop all oscillators
    for (const o of engine._oscBank) {
        try { o.osc.stop(); } catch(e) {}
    }
    engine._oscBank = [];
    if (engine._lfo) { try { engine._lfo.stop(); } catch(e) {} engine._lfo = null; }
    // Nodes will be GC'd once disconnected
    engine._masterGain = null;
    engine._filter = null;
    engine._shaper = null;
    engine._convolver = null;
    engine._dryGain = null;
    engine._wetGain = null;
    engine._lfoGain = null;
    engine._audioProfile = null;
}

/**
 * Per-frame audio modulation based on engine state fields.
 */
export function updateAudio(engine) {
    if (!engine._audioEnabled || !engine._audioCtx || !engine._audioProfile) return;
    const ctx = engine._audioCtx;
    const t = ctx.currentTime;
    const p = AUDIO_PROFILES[engine._audioProfile];
    if (!p) return;

    const TAU = 0.1;       // smooth ramping time constant
    const TAU_SLOW = 0.3;  // slower ramps for pitch/reverb

    // ═══ Pitch modulation (scenario-specific) ═══
    switch (p.pitchMode) {
        case 'threshold': {
            // fluxRatio drives frequency: 46 Hz → 137 Hz → 493 Hz
            const r = Math.min(engine._fluxRatio, 2.0);
            const freqLow  = FTD_SUB;                            // 46 Hz
            const freqMid  = FTD_FREQ;                           // 137 Hz
            const freqHigh = FTD_FREQ * K_C / G_STAR;            // 167 Hz (K_C × base / G*)
            const baseFreq = r <= 1.0
                ? freqLow + (freqMid - freqLow) * r
                : freqMid + (freqHigh - freqMid) * (r - 1.0);

            for (const o of engine._oscBank) {
                o.osc.frequency.setTargetAtTime(
                    baseFreq * o.ratio + o.freqOffset, t, TAU_SLOW
                );
            }

            // Pythagorean overtones bloom as fluxRatio approaches/exceeds 1.0
            for (let i = 1; i < engine._oscBank.length; i++) {
                const maxG = engine._oscBank[i].bloomMax;
                if (maxG > 0) {
                    const bloom = Math.max(0, Math.min(1, (r - 0.5) * 2));
                    engine._oscBank[i].gain.gain.setTargetAtTime(
                        bloom * maxG, t, TAU_SLOW
                    );
                }
            }
            break;
        }
        case 'mandelbrot': {
            // Mandelbrot |z| (0–2) modulates pitch and waveshaper
            const z = Math.min(engine._mandelbrotZ, 2.0);
            const baseFreq = p.baseFreq * (1 + z * 0.5);
            for (const o of engine._oscBank) {
                o.osc.frequency.setTargetAtTime(
                    baseFreq * o.ratio + o.freqOffset, t, TAU * 0.5
                );
            }
            // Waveshaper drive follows |z| → more chaos = richer overtones
            if (engine._shaper && p.shaper) {
                engine._shaper.curve = makeShaperCurve(
                    p.shaper, p.drive * (0.5 + z)
                );
            }
            break;
        }
        case 'subtle': {
            // Gentle vibrato from flux energy
            const vib = 1.0 + (engine._fluxEnergy - 0.5) * 0.02;
            for (const o of engine._oscBank) {
                o.osc.frequency.setTargetAtTime(
                    p.baseFreq * o.ratio * vib + o.freqOffset, t, TAU
                );
            }
            break;
        }
        // 'none': no pitch modulation
    }

    // ═══ Filter modulation (fluxRatio opens the filter) ═══
    const fTarget = Math.min(200 + engine._fluxRatio * 2000, 8000);
    engine._filter.frequency.setTargetAtTime(fTarget, t, TAU);
    engine._filter.Q.setTargetAtTime(
        p.filterQ + engine._variance * 6, t, TAU
    );

    // ═══ Reverb wet/dry (consciousness adds spatial depth) ═══
    const wetTarget = engine._consciousnessI > 0
        ? Math.min(p.reverbWet + 0.15, 0.7)
        : p.reverbWet * 0.5;
    engine._wetGain.gain.setTargetAtTime(wetTarget, t, TAU_SLOW);
    engine._dryGain.gain.setTargetAtTime(1.0 - wetTarget, t, TAU_SLOW);

    // ═══ LFO rate modulation (curlMag adds energy) ═══
    if (engine._lfo) {
        engine._lfo.frequency.setTargetAtTime(
            p.lfoRate + engine._curlMag * 4, t, 0.2
        );
    }

    // ═══ Master gain (flux energy → volume) ═══
    engine._masterGain.gain.setTargetAtTime(
        0.04 + engine._fluxEnergy * 0.18, t, TAU
    );

    // ═══ Variance → detuning (oscillator pairs drift apart) ═══
    for (let i = 1; i < engine._oscBank.length; i++) {
        const baseDet = p.oscs[i]?.detune || 0;
        const varDet = engine._variance * 30 * (i % 2 === 0 ? 1 : -1);
        engine._oscBank[i].osc.detune.setTargetAtTime(
            baseDet + varDet, t, TAU
        );
    }
}

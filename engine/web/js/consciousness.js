/**
 * Consciousness Engine — holographic figure, sLoop ring, and audio synthesis
 * driven by lattice flux dynamics.
 *
 * Scale 4 in the FTD hierarchy: the meta-scale where physics meets consciousness.
 *
 * The holographic figure responds to real-time flux data:
 *   - Vertex deformation from flux spatial variance
 *   - Color shifts from flux polarity
 *   - Opacity from central flux density
 *   - Sound from flux energy / wave energy / curl
 *
 * The sLoop torus ring orbits at theta_C = 52.54 degrees — the consciousness
 * phase angle from the master quadratic with k = 1/2.
 */

import * as THREE from 'three';
import { FIGURE_REGISTRY } from './consciousness-figure.js';
import {
    COS2_THETA_C, THETA_C_RAD, THETA_C_DEG, Y_REAL, Y_IMAG,
    K_C, C_MANDELBROT, SIN2_THETA_C, G_STAR, ALPHA, K_B
} from './constants.js';

// ── Holographic Mesh Shader ──────────────────────────────────────────

const HOLO_VERT = `
    uniform float uTime;
    uniform float uDeformAmt;

    varying vec3 vNormal;
    varying vec3 vPosition;
    varying vec3 vWorldPosition;
    varying vec2 vUv;

    void main() {
        vNormal = normalize(normalMatrix * normal);
        vUv = uv;

        // Flux-driven vertex displacement (breathing + ripple)
        float displacement = sin(position.y * 3.0 + uTime * 2.0) * uDeformAmt;
        displacement += sin(position.x * 5.0 + uTime * 1.5) * uDeformAmt * 0.5;
        displacement += cos(position.z * 4.0 + uTime * 1.8) * uDeformAmt * 0.3;
        vec3 pos = position + normal * displacement;

        vec4 worldPos = modelMatrix * vec4(pos, 1.0);
        vWorldPosition = worldPos.xyz;
        vPosition = pos;
        gl_Position = projectionMatrix * viewMatrix * worldPos;
    }
`;

const HOLO_FRAG = `
    uniform float uTime;
    uniform float uFluxEnergy;
    uniform float uObservableFraction;
    uniform vec3  uBaseColor;
    uniform vec3  uSecondaryColor;
    uniform float uOpacity;
    uniform float uScanLineFreq;
    uniform float uScanLineSpeed;
    uniform float uFresnelPower;
    uniform float uGlitchScale;
    uniform float uIridescenceStr;

    varying vec3 vNormal;
    varying vec3 vPosition;
    varying vec3 vWorldPosition;
    varying vec2 vUv;

    void main() {
        // Fresnel rim glow
        vec3 viewDir = normalize(cameraPosition - vWorldPosition);
        float fresnel = 1.0 - abs(dot(viewDir, vNormal));
        fresnel = pow(fresnel, uFresnelPower);

        // Scan lines (scrolling upward)
        float scanLine = sin(vPosition.y * uScanLineFreq - uTime * uScanLineSpeed) * 0.5 + 0.5;
        scanLine = smoothstep(0.3, 0.7, scanLine);

        // Glitch noise (inversely proportional to flux stability)
        float glitch = fract(sin(dot(vUv * 100.0 + uTime * 0.5, vec2(12.9898, 78.233))) * 43758.5453);
        float glitchStrength = (0.05 * uGlitchScale) / (1.0 + uFluxEnergy * 2.0);

        // Iridescence (rainbow shift based on view angle)
        float iridescence = fract(fresnel * 2.0 + uTime * 0.1);
        vec3 iriColor = vec3(
            sin(iridescence * 6.2832) * 0.5 + 0.5,
            sin(iridescence * 6.2832 + 2.094) * 0.5 + 0.5,
            sin(iridescence * 6.2832 + 4.189) * 0.5 + 0.5
        );

        // Compose final color
        vec3 base = mix(uBaseColor, uSecondaryColor, fresnel);
        base = mix(base, iriColor, uIridescenceStr * uObservableFraction);
        base *= (0.7 + 0.3 * scanLine);
        base += glitch * glitchStrength;

        // Opacity: solid core, transparent edges with fresnel glow
        float alpha = uOpacity * (0.3 + 0.7 * (1.0 - fresnel));
        alpha += fresnel * 0.6;
        alpha *= (0.85 + 0.15 * scanLine);

        gl_FragColor = vec4(base, clamp(alpha, 0.0, 1.0));
    }
`;

// ── Particle Holographic Shader ──────────────────────────────────────

const PARTICLE_VERT = `
    attribute float aSize;
    uniform float uTime;
    uniform float uDeformAmt;

    varying vec3 vWorldPosition;
    varying float vSize;

    void main() {
        vSize = aSize;
        vec3 pos = position;
        // Gentle breathing on particles
        pos.y += sin(pos.x * 3.0 + uTime * 1.5) * uDeformAmt * 0.5;
        pos.x += cos(pos.y * 2.0 + uTime * 1.2) * uDeformAmt * 0.3;

        vec4 worldPos = modelMatrix * vec4(pos, 1.0);
        vWorldPosition = worldPos.xyz;

        vec4 mvPosition = modelViewMatrix * vec4(pos, 1.0);
        gl_PointSize = aSize * (800.0 / max(-mvPosition.z, 0.1));
        gl_PointSize = clamp(gl_PointSize, 2.0, 180.0);
        gl_Position = projectionMatrix * mvPosition;
    }
`;

const PARTICLE_FRAG = `
    uniform float uTime;
    uniform float uFluxEnergy;
    uniform vec3  uBaseColor;
    uniform vec3  uSecondaryColor;
    uniform float uOpacity;
    uniform float uScanLineFreq;
    uniform float uScanLineSpeed;

    varying vec3 vWorldPosition;
    varying float vSize;

    void main() {
        vec2 center = gl_PointCoord - vec2(0.5);
        float dist = length(center);

        // Soft Gaussian falloff — no hard discard, smooth gas cloud
        float gauss = exp(-dist * dist * 8.0);

        // Very subtle scan lines (reduced from mesh intensity)
        float scanLine = sin(vWorldPosition.y * uScanLineFreq * 0.3 - uTime * uScanLineSpeed * 0.5) * 0.5 + 0.5;
        scanLine = mix(0.85, 1.0, smoothstep(0.3, 0.7, scanLine));

        // Color: gentle shift from center to edge
        float edgeMix = smoothstep(0.0, 0.45, dist);
        vec3 base = mix(uBaseColor, uSecondaryColor, edgeMix * 0.5);
        base *= scanLine;

        // Inner glow (brighter core)
        float glow = exp(-dist * dist * 20.0) * 0.4;
        base += glow * uBaseColor;

        float alpha = gauss * uOpacity * (0.15 + 0.35 * uFluxEnergy);
        gl_FragColor = vec4(base, clamp(alpha, 0.0, 0.7));
    }
`;

// ── sLoop ring shader (golden) ───────────────────────────────────────

const SLOOP_FRAG = `
    uniform float uTime;
    uniform float uFluxEnergy;
    uniform float uPulse;

    varying vec3 vNormal;
    varying vec3 vPosition;
    varying vec3 vWorldPosition;
    varying vec2 vUv;

    void main() {
        vec3 viewDir = normalize(cameraPosition - vWorldPosition);
        float fresnel = 1.0 - abs(dot(viewDir, vNormal));
        fresnel = pow(fresnel, 1.5);

        float scanLine = sin(vPosition.x * 30.0 + vPosition.z * 30.0 - uTime * 4.0) * 0.5 + 0.5;

        vec3 gold = vec3(1.0, 0.84, 0.0);
        vec3 white = vec3(1.0, 0.95, 0.8);
        vec3 base = mix(gold, white, fresnel * 0.5);
        base *= (0.6 + 0.4 * scanLine);

        float alpha = (0.15 + 0.25 * fresnel + 0.15 * uPulse) * (0.7 + 0.3 * scanLine);
        alpha *= (0.5 + 0.5 * uFluxEnergy);

        gl_FragColor = vec4(base, clamp(alpha, 0.0, 0.85));
    }
`;

// ── Lemniscate-Alpha Curve ───────────────────────────────────────────

class LemniscateAlphaCurve extends THREE.Curve {
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
//
// Frequencies derived from FTD consciousness theory:
//   1/α ≈ 137.036 Hz — strikingly close to Om/Aum (136.1 Hz)
//   G* ≈ 2.959 — frequency ratio (nearly a perfect 12th = octave+fifth)
//   Framework integers {3,4,7,13} — Pythagorean overtone partials
//   θ_C ≈ 52.54° — consciousness phase angle
//   cos²(θ_C) ≈ 0.37 — observable fraction (gain ratio)
//   K_C ≈ 3.599 — consciousness threshold (frequency multiplier)
//   C_MANDELBROT ≈ 0.338 — subharmonic ratio (deep bass)
//
// Historical sound traditions:
//   Om/Aum drone (136.1 Hz) ≈ 1/α — the cosmic hum
//   Schumann resonance (7.83 Hz) ≈ b₃=7 — Earth's EM cavity
//   Pythagorean tuning: 3:2 (fifth), 4:3 (fourth) — framework integers
//   Binaural beats: theta (4-8 Hz), alpha (8-13 Hz) — brainwave entrainment
//   Tibetan singing bowls: beating overtones, long reverb
//   Gamelan: metallic shimmer from paired detuned tones
//   Overtone/throat singing: emphasized upper partials
//   Indian tanpura: slow-evolving harmonic bath

const FTD_FREQ  = 1.0 / ALPHA;                           // ≈ 137.036 Hz
const FTD_SUB   = FTD_FREQ * C_MANDELBROT;               // ≈ 46.3 Hz
const FTD_DELTA = Math.sqrt((4*G_STAR - 1)/(4*G_STAR));  // ≈ 0.957

// Generate algorithmic reverb impulse response (no external files needed)
function _createReverbIR(ctx, duration, decay) {
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

// Generate waveshaper curves for different timbres
function _makeShaperCurve(type, drive, n = 4096) {
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

const AUDIO_PROFILES = {
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

// ── ConsciousnessEngine ──────────────────────────────────────────────

export class ConsciousnessEngine {
    constructor(scene) {
        this._scene = scene;
        this._group = new THREE.Group();
        this._scene.add(this._group);

        this._figureMesh = null;
        this._figurePoints = null;
        this._descriptor = null;
        this._sloopRing = null;
        this._sloopPivot = null;
        this._figureType = 'plasmoid';
        this._startTime = performance.now() / 1000;

        // Audio state (scenario-aware synthesis engine)
        this._audioCtx = null;
        this._audioEnabled = false;
        this._audioProfile = null;   // current profile name
        this._masterGain = null;
        this._filter = null;
        this._shaper = null;         // WaveShaperNode for overtone enrichment
        this._convolver = null;      // ConvolverNode for spatial reverb
        this._dryGain = null;
        this._wetGain = null;
        this._oscBank = [];          // [{osc, gain, panner, ratio, freqOffset, basePan}]
        this._lfo = null;
        this._lfoGain = null;

        // Flux data (updated per frame)
        this._fluxEnergy = 0;
        this._waveEnergy = 0;
        this._variance = 0;
        this._curlMag = 0;
        this._centralDensity = 0;
        // Consciousness diagnostics (new — drives audio modulation)
        this._fluxRatio = 0;
        this._effTheta = THETA_C_DEG;
        this._consciousnessI = 0;
        this._mandelbrotZ = 0;

        this._buildFigure('plasmoid');
        this._buildSLoop();
    }

    // ── Figure Management ──

    _buildFigure(type) {
        // Dispose old
        if (this._figureMesh) {
            this._group.remove(this._figureMesh);
            this._figureMesh.geometry.dispose();
            this._figureMesh.material.dispose();
            this._figureMesh = null;
        }
        if (this._figurePoints) {
            this._group.remove(this._figurePoints);
            this._figurePoints.geometry.dispose();
            this._figurePoints.material.dispose();
            this._figurePoints = null;
        }

        // Look up builder
        const builder = FIGURE_REGISTRY.get(type) || FIGURE_REGISTRY.get('humanoid');
        const d = builder();
        this._descriptor = d;
        this._figureType = type;

        const overrides = d.shaderOverrides;

        // Build mesh if descriptor has geometry
        if (d.geometry && (d.type === 'mesh' || d.type === 'hybrid')) {
            const mat = new THREE.ShaderMaterial({
                vertexShader: HOLO_VERT,
                fragmentShader: HOLO_FRAG,
                uniforms: {
                    uTime:              { value: 0 },
                    uFluxEnergy:        { value: 0.3 },
                    uDeformAmt:         { value: 0.03 },
                    uObservableFraction:{ value: COS2_THETA_C },
                    uBaseColor:         { value: d.palette.primary.clone() },
                    uSecondaryColor:    { value: d.palette.secondary.clone() },
                    uOpacity:           { value: 0.7 },
                    uScanLineFreq:      { value: overrides.scanLineFreq },
                    uScanLineSpeed:     { value: overrides.scanLineSpeed },
                    uFresnelPower:      { value: overrides.fresnelPower },
                    uGlitchScale:       { value: overrides.glitchScale },
                    uIridescenceStr:    { value: overrides.iridescenceStr },
                },
                transparent: true,
                side: THREE.DoubleSide,
                depthWrite: false,
                blending: THREE.AdditiveBlending,
            });

            this._figureMesh = new THREE.Mesh(d.geometry, mat);
            this._figureMesh.scale.setScalar(d.scale);
            if (d.yOffset) this._figureMesh.position.y = d.yOffset;
            this._group.add(this._figureMesh);
        }

        // Build particles if descriptor has them
        if (d.particlePositions && d.particleCount > 0 && (d.type === 'points' || d.type === 'hybrid')) {
            const geom = new THREE.BufferGeometry();
            geom.setAttribute('position', new THREE.BufferAttribute(d.particlePositions, 3));
            geom.setAttribute('aSize', new THREE.BufferAttribute(d.particleSizes, 1));

            const pMat = new THREE.ShaderMaterial({
                vertexShader: PARTICLE_VERT,
                fragmentShader: PARTICLE_FRAG,
                uniforms: {
                    uTime:          { value: 0 },
                    uFluxEnergy:    { value: 0.3 },
                    uDeformAmt:     { value: 0.02 },
                    uBaseColor:     { value: d.palette.primary.clone() },
                    uSecondaryColor:{ value: d.palette.secondary.clone() },
                    uOpacity:       { value: 0.8 },
                    uScanLineFreq:  { value: overrides.scanLineFreq },
                    uScanLineSpeed: { value: overrides.scanLineSpeed },
                },
                transparent: true,
                depthWrite: false,
                blending: THREE.AdditiveBlending,
            });

            this._figurePoints = new THREE.Points(geom, pMat);
            this._figurePoints.scale.setScalar(d.scale);
            if (d.yOffset) this._figurePoints.position.y = d.yOffset;
            this._group.add(this._figurePoints);
        }
    }

    setFigureType(type) {
        if (type === this._figureType) return;
        this._buildFigure(type);
    }

    // ── sLoop Ring ──

    _buildSLoop() {
        this._sloopPivot = new THREE.Group();

        const curve = new LemniscateAlphaCurve(0.7);
        const ringGeom = new THREE.TubeGeometry(curve, 300, 0.05, 12, true);

        const ringMat = new THREE.ShaderMaterial({
            vertexShader: HOLO_VERT,
            fragmentShader: SLOOP_FRAG,
            uniforms: {
                uTime:      { value: 0 },
                uDeformAmt: { value: 0.005 },
                uFluxEnergy:{ value: 0.5 },
                uPulse:     { value: 0 },
            },
            transparent: true,
            side: THREE.DoubleSide,
            depthWrite: false,
            blending: THREE.AdditiveBlending,
        });

        this._sloopRing = new THREE.Mesh(ringGeom, ringMat);

        this._sloopPivot.rotation.x = Math.PI / 2 - THETA_C_RAD;
        this._sloopPivot.add(this._sloopRing);
        this._group.add(this._sloopPivot);
    }

    // ── Audio (Scenario-Aware Synthesis) ──

    enableAudio(profileName = 'cs-custom') {
        // If same profile already running, just resume
        if (this._audioCtx && this._audioProfile === profileName) {
            this._audioCtx.resume();
            this._audioEnabled = true;
            return;
        }

        // Create AudioContext if needed
        if (!this._audioCtx) {
            try {
                this._audioCtx = new (window.AudioContext || window.webkitAudioContext)();
            } catch (e) {
                console.warn('[CS] Web Audio not available:', e);
                return;
            }
        }
        this._audioCtx.resume();

        // Tear down old graph, build new one for this scenario
        this._teardownAudioGraph();
        this._buildAudioGraph(profileName);
        this._audioProfile = profileName;
        this._audioEnabled = true;
    }

    disableAudio() {
        this._teardownAudioGraph();
        if (this._audioCtx) this._audioCtx.suspend();
        this._audioEnabled = false;
    }

    _teardownAudioGraph() {
        // Stop all oscillators
        for (const o of this._oscBank) {
            try { o.osc.stop(); } catch(e) {}
        }
        this._oscBank = [];
        if (this._lfo) { try { this._lfo.stop(); } catch(e) {} this._lfo = null; }
        // Nodes will be GC'd once disconnected
        this._masterGain = null;
        this._filter = null;
        this._shaper = null;
        this._convolver = null;
        this._dryGain = null;
        this._wetGain = null;
        this._lfoGain = null;
        this._audioProfile = null;
    }

    _buildAudioGraph(profileName) {
        const ctx = this._audioCtx;
        const p = AUDIO_PROFILES[profileName] || AUDIO_PROFILES['cs-custom'];

        // ── Master output chain ──
        // masterGain → destination
        this._masterGain = ctx.createGain();
        this._masterGain.gain.value = 0.12;
        this._masterGain.connect(ctx.destination);

        // ── Dry/Wet reverb split ──
        this._dryGain = ctx.createGain();
        this._dryGain.gain.value = 1.0 - p.reverbWet;
        this._dryGain.connect(this._masterGain);

        this._wetGain = ctx.createGain();
        this._wetGain.gain.value = p.reverbWet;
        this._wetGain.connect(this._masterGain);

        // Algorithmic reverb (convolver with generated IR)
        this._convolver = ctx.createConvolver();
        this._convolver.buffer = _createReverbIR(ctx, p.reverbTime, 3.0);
        this._convolver.connect(this._wetGain);

        // ── Filter ──
        this._filter = ctx.createBiquadFilter();
        this._filter.type = 'lowpass';
        this._filter.frequency.value = p.filterFreq;
        this._filter.Q.value = p.filterQ;
        this._filter.connect(this._dryGain);
        this._filter.connect(this._convolver);

        // ── Waveshaper (optional overtone enrichment) ──
        const oscTarget = (() => {
            if (p.shaper) {
                this._shaper = ctx.createWaveShaper();
                this._shaper.curve = _makeShaperCurve(p.shaper, p.drive);
                this._shaper.oversample = '2x';
                this._shaper.connect(this._filter);
                return this._shaper;
            }
            return this._filter;
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

            this._oscBank.push({
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
            this._lfo = ctx.createOscillator();
            this._lfo.frequency.value = p.lfoRate;
            this._lfoGain = ctx.createGain();
            this._lfoGain.gain.value = p.lfoDepth;
            this._lfo.connect(this._lfoGain);
            this._lfoGain.connect(this._masterGain.gain);
            this._lfo.start();
        }
    }

    _updateAudio() {
        if (!this._audioEnabled || !this._audioCtx || !this._audioProfile) return;
        const ctx = this._audioCtx;
        const t = ctx.currentTime;
        const p = AUDIO_PROFILES[this._audioProfile];
        if (!p) return;

        const TAU = 0.1;       // smooth ramping time constant
        const TAU_SLOW = 0.3;  // slower ramps for pitch/reverb

        // ═══ Pitch modulation (scenario-specific) ═══
        switch (p.pitchMode) {
            case 'threshold': {
                // fluxRatio drives frequency: 46 Hz → 137 Hz → 493 Hz
                const r = Math.min(this._fluxRatio, 2.0);
                const freqLow  = FTD_SUB;                            // 46 Hz
                const freqMid  = FTD_FREQ;                           // 137 Hz
                const freqHigh = FTD_FREQ * K_C / G_STAR;            // 167 Hz (K_C × base / G*)
                const baseFreq = r <= 1.0
                    ? freqLow + (freqMid - freqLow) * r
                    : freqMid + (freqHigh - freqMid) * (r - 1.0);

                for (const o of this._oscBank) {
                    o.osc.frequency.setTargetAtTime(
                        baseFreq * o.ratio + o.freqOffset, t, TAU_SLOW
                    );
                }

                // Pythagorean overtones bloom as fluxRatio approaches/exceeds 1.0
                for (let i = 1; i < this._oscBank.length; i++) {
                    const maxG = this._oscBank[i].bloomMax;
                    if (maxG > 0) {
                        const bloom = Math.max(0, Math.min(1, (r - 0.5) * 2));
                        this._oscBank[i].gain.gain.setTargetAtTime(
                            bloom * maxG, t, TAU_SLOW
                        );
                    }
                }
                break;
            }
            case 'mandelbrot': {
                // Mandelbrot |z| (0–2) modulates pitch and waveshaper
                const z = Math.min(this._mandelbrotZ, 2.0);
                const baseFreq = p.baseFreq * (1 + z * 0.5);
                for (const o of this._oscBank) {
                    o.osc.frequency.setTargetAtTime(
                        baseFreq * o.ratio + o.freqOffset, t, TAU * 0.5
                    );
                }
                // Waveshaper drive follows |z| → more chaos = richer overtones
                if (this._shaper && p.shaper) {
                    this._shaper.curve = _makeShaperCurve(
                        p.shaper, p.drive * (0.5 + z)
                    );
                }
                break;
            }
            case 'subtle': {
                // Gentle vibrato from flux energy
                const vib = 1.0 + (this._fluxEnergy - 0.5) * 0.02;
                for (const o of this._oscBank) {
                    o.osc.frequency.setTargetAtTime(
                        p.baseFreq * o.ratio * vib + o.freqOffset, t, TAU
                    );
                }
                break;
            }
            // 'none': no pitch modulation
        }

        // ═══ Filter modulation (fluxRatio opens the filter) ═══
        const fTarget = Math.min(200 + this._fluxRatio * 2000, 8000);
        this._filter.frequency.setTargetAtTime(fTarget, t, TAU);
        this._filter.Q.setTargetAtTime(
            p.filterQ + this._variance * 6, t, TAU
        );

        // ═══ Reverb wet/dry (consciousness adds spatial depth) ═══
        const wetTarget = this._consciousnessI > 0
            ? Math.min(p.reverbWet + 0.15, 0.7)
            : p.reverbWet * 0.5;
        this._wetGain.gain.setTargetAtTime(wetTarget, t, TAU_SLOW);
        this._dryGain.gain.setTargetAtTime(1.0 - wetTarget, t, TAU_SLOW);

        // ═══ LFO rate modulation (curlMag adds energy) ═══
        if (this._lfo) {
            this._lfo.frequency.setTargetAtTime(
                p.lfoRate + this._curlMag * 4, t, 0.2
            );
        }

        // ═══ Master gain (flux energy → volume) ═══
        this._masterGain.gain.setTargetAtTime(
            0.04 + this._fluxEnergy * 0.18, t, TAU
        );

        // ═══ Variance → detuning (oscillator pairs drift apart) ═══
        for (let i = 1; i < this._oscBank.length; i++) {
            const baseDet = p.oscs[i]?.detune || 0;
            const varDet = this._variance * 30 * (i % 2 === 0 ? 1 : -1);
            this._oscBank[i].osc.detune.setTargetAtTime(
                baseDet + varDet, t, TAU
            );
        }
    }

    // ── Per-Frame Particle Animation ──

    _animateParticles(elapsed, dt) {
        if (!this._figurePoints || !this._descriptor) return;
        const d = this._descriptor;
        const posAttr = this._figurePoints.geometry.getAttribute('position');
        const sizeAttr = this._figurePoints.geometry.getAttribute('aSize');
        if (!posAttr) return;
        const pos = posAttr.array;
        const count = d.particleCount;

        switch (d.animationFn) {
            case 'plasmoid': {
                // Swirl along torus path
                const angles = d._torusAngles;
                const R = d._torusR || 2.0;
                const r = d._torusr || 1.0;
                if (!angles) break;
                const speed = 0.3 + this._fluxEnergy * 0.5;
                for (let i = 0; i < count; i++) {
                    angles[i*2] += speed * dt; // advance toroidal angle
                    const u = angles[i*2];
                    const v = angles[i*2+1];
                    const rr = r + Math.sin(u * 3 + elapsed) * 0.15;
                    pos[i*3]   = (R + rr * Math.cos(v)) * Math.cos(u);
                    pos[i*3+1] = rr * Math.sin(v);
                    pos[i*3+2] = (R + rr * Math.cos(v)) * Math.sin(u);
                }
                posAttr.needsUpdate = true;
                break;
            }

            case 'spirit': {
                // Dissolution particles drift downward, respawn at waist
                const driftSpeed = 0.8 + this._fluxEnergy * 0.5;
                for (let i = 0; i < count; i++) {
                    pos[i*3+1] -= driftSpeed * dt;
                    // Horizontal jitter
                    pos[i*3]   += (Math.random() - 0.5) * 0.02;
                    pos[i*3+2] += (Math.random() - 0.5) * 0.02;
                    // Respawn at top
                    if (pos[i*3+1] < -3.5) {
                        const angle = Math.random() * Math.PI * 2;
                        const rr = Math.random() * 0.3;
                        pos[i*3]   = rr * Math.cos(angle);
                        pos[i*3+1] = 0.5;
                        pos[i*3+2] = rr * Math.sin(angle);
                    }
                }
                posAttr.needsUpdate = true;
                break;
            }

            case 'yahweh': {
                // Fire particles drift upward, flicker size
                const upSpeed = 1.2 + this._fluxEnergy * 0.8;
                const sizes = sizeAttr.array;
                for (let i = 0; i < count; i++) {
                    pos[i*3+1] += upSpeed * dt;
                    // Slight horizontal wobble
                    pos[i*3]   += Math.sin(elapsed * 5 + i * 0.1) * 0.01;
                    pos[i*3+2] += Math.cos(elapsed * 4 + i * 0.13) * 0.01;
                    // Respawn at bottom
                    if (pos[i*3+1] > 5.0) {
                        const angle = Math.random() * Math.PI * 2;
                        const rr = Math.random() * 0.8;
                        pos[i*3]   = rr * Math.cos(angle);
                        pos[i*3+1] = -2.0;
                        pos[i*3+2] = rr * Math.sin(angle);
                    }
                    // Flicker size
                    sizes[i] = 0.06 + 0.12 * Math.abs(Math.sin(elapsed * 5 + i * 0.37));
                }
                posAttr.needsUpdate = true;
                sizeAttr.needsUpdate = true;
                break;
            }

            case 'death-cloud': {
                // Vortex rotation + outward drift
                const rotSpeed = 0.5 + this._fluxEnergy * 0.3;
                const expandSpeed = 0.3;
                for (let i = 0; i < count; i++) {
                    const x = pos[i*3];
                    const z = pos[i*3+2];
                    const r = Math.sqrt(x*x + z*z) || 0.01;
                    const angle = Math.atan2(z, x) + rotSpeed * dt;
                    const newR = r + expandSpeed * dt * 0.1;
                    pos[i*3]   = newR * Math.cos(angle);
                    pos[i*3+2] = newR * Math.sin(angle);
                    // Slight downward drift
                    pos[i*3+1] -= 0.1 * dt;
                    // Respawn when too far
                    if (newR > 3.5 || pos[i*3+1] < -4.0) {
                        const a = Math.random() * Math.PI * 2;
                        const rr = Math.random() * 0.3;
                        pos[i*3]   = rr * Math.cos(a);
                        pos[i*3+1] = 2.5 + Math.random() * 0.5;
                        pos[i*3+2] = rr * Math.sin(a);
                    }
                }
                posAttr.needsUpdate = true;
                break;
            }

            case 'mayan-sun': {
                // Particles drift outward along rays, respawn at center
                const driftSpeed = 0.6 + this._fluxEnergy * 0.4;
                const rayAngles = d._rayAngles;
                const rayDists  = d._rayDists;
                if (!rayAngles || !rayDists) break;
                for (let i = 0; i < count; i++) {
                    rayDists[i] += driftSpeed * dt;
                    if (rayDists[i] > 3.2) {
                        rayDists[i] = 1.2;
                    }
                    pos[i*3]   = rayDists[i] * Math.cos(rayAngles[i]);
                    pos[i*3+1] = rayDists[i] * Math.sin(rayAngles[i]);
                    pos[i*3+2] = (Math.random() - 0.5) * 0.1;
                }
                posAttr.needsUpdate = true;
                break;
            }

            case 'demiurge': {
                // Orbiting particles rotate around Y
                const orbSpeed = 0.3 + this._fluxEnergy * 0.2;
                for (let i = 0; i < count; i++) {
                    const x = pos[i*3];
                    const z = pos[i*3+2];
                    const angle = Math.atan2(z, x) + orbSpeed * dt;
                    const r = Math.sqrt(x*x + z*z) || 1.5;
                    pos[i*3]   = r * Math.cos(angle);
                    pos[i*3+2] = r * Math.sin(angle);
                    // Gentle vertical oscillation
                    pos[i*3+1] += Math.sin(elapsed * 2 + i * 0.1) * 0.003;
                }
                posAttr.needsUpdate = true;
                break;
            }
        }
    }

    // ── Per-Frame Update ──

    update(opts = {}) {
        const now = performance.now() / 1000;
        const elapsed = now - this._startTime;
        const dt = Math.min(1/30, now - (this._lastTime || now));
        this._lastTime = now;

        this._fluxEnergy = opts.fluxEnergy ?? 0.3;
        this._waveEnergy = opts.waveEnergy ?? 0.2;
        this._variance = opts.variance ?? 0.1;
        this._curlMag = opts.curlMag ?? 0.05;
        this._centralDensity = opts.centralDensity ?? 0.3;
        // Consciousness diagnostics (drive audio modulation)
        this._fluxRatio = opts.fluxRatio ?? 0;
        this._effTheta = opts.effTheta ?? THETA_C_DEG;
        this._consciousnessI = opts.consciousnessI ?? 0;
        this._mandelbrotZ = opts.mandelbrotZ ?? 0;
        const polarity = opts.polarity ?? 0;

        const d = this._descriptor;

        // ── Update figure mesh shader uniforms ──
        if (this._figureMesh && d) {
            const u = this._figureMesh.material.uniforms;
            u.uTime.value = elapsed;
            u.uFluxEnergy.value = this._fluxEnergy;
            u.uDeformAmt.value = 0.01 + this._variance * 0.12;
            u.uOpacity.value = 0.4 + this._centralDensity * 0.5;

            // Color shifts with polarity, using figure's palette
            const mix = (polarity + 1) / 2;
            u.uBaseColor.value.copy(d.palette.primary).lerp(d.palette.secondary, 1 - mix);
            u.uSecondaryColor.value.copy(d.palette.secondary).lerp(d.palette.primary, 1 - mix);

            // Slow idle rotation
            this._figureMesh.rotation.y = elapsed * 0.15;
        }

        // ── Update figure particles ──
        if (this._figurePoints && d) {
            const pu = this._figurePoints.material.uniforms;
            pu.uTime.value = elapsed;
            pu.uFluxEnergy.value = this._fluxEnergy;
            pu.uDeformAmt.value = 0.005 + this._variance * 0.05;
            pu.uOpacity.value = 0.5 + this._centralDensity * 0.4;

            const mix = (polarity + 1) / 2;
            pu.uBaseColor.value.copy(d.palette.primary).lerp(d.palette.secondary, 1 - mix);
            pu.uSecondaryColor.value.copy(d.palette.secondary).lerp(d.palette.primary, 1 - mix);

            // Rotate particles with mesh (except pure point figures)
            if (d.type === 'hybrid') {
                this._figurePoints.rotation.y = elapsed * 0.15;
            } else {
                this._figurePoints.rotation.y = elapsed * 0.08; // slower for pure clouds
            }
        }

        // ── Animate particles ──
        this._animateParticles(elapsed, dt);

        // ── Update sLoop ring ──
        if (this._sloopPivot) {
            const orbitPeriod = 2 * Math.PI / THETA_C_RAD;
            this._sloopPivot.rotation.y = (elapsed / orbitPeriod) * 2 * Math.PI;

            const ru = this._sloopRing.material.uniforms;
            ru.uTime.value = elapsed;
            ru.uFluxEnergy.value = this._fluxEnergy;
            ru.uPulse.value = 0.5 + 0.5 * Math.sin(elapsed * 2 * Math.PI * this._centralDensity);
        }

        // ── Update audio ──
        this._updateAudio();
    }

    // ── Cleanup ──

    dispose() {
        // Stop audio
        this._teardownAudioGraph();
        if (this._audioCtx) {
            this._audioCtx.close();
            this._audioCtx = null;
        }

        // Remove mesh
        if (this._figureMesh) {
            this._group.remove(this._figureMesh);
            this._figureMesh.geometry.dispose();
            this._figureMesh.material.dispose();
        }
        // Remove particles
        if (this._figurePoints) {
            this._group.remove(this._figurePoints);
            this._figurePoints.geometry.dispose();
            this._figurePoints.material.dispose();
        }
        // Remove sLoop
        if (this._sloopRing) {
            this._sloopPivot.remove(this._sloopRing);
            this._sloopRing.geometry.dispose();
            this._sloopRing.material.dispose();
        }
        if (this._sloopPivot) {
            this._group.remove(this._sloopPivot);
        }
        this._scene.remove(this._group);
    }
}

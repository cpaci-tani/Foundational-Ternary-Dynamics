/**
 * @file engine/web/js/audio/lattice-synth.js
 * @purpose Connects FTD wave telemetry into the Web Audio API to hear the lattice.
 */

export class LatticeSynth {
    constructor() {
        this.ctx = null;
        this.active = false;
        
        // The dimensionless lattice frequency is small (e.g. 0.01 - 0.1).
        // This scalar brings those wave cycles up into the human hearing range.
        this.hzScale = 22000;
        this._oscType = 'sine';
        
        // Master Bus
        this.masterGain = null;
        this.masterShaper = null;
        this.convolver = null;
        this.reverbMix = null;
        this.dryMix = null;

        // Dual channels for collision scenario
        this.channels = [
            this._createChannelState(),
            this._createChannelState()
        ];
        
        // Feature toggles
        this.detuneEnabled = true;
        this.filterEnabled = true;
        this.panningEnabled = true;
        this.tremoloEnabled = true;
        this.additiveEnabled = true;
        this.reverbEnabled = true;
        this.fmEnabled = true;
        this.masterVolume = 1.0;
    }

    _createChannelState() {
        return {
            osc: null,
            oscDetune: null,
            additiveOscs: [],
            additiveGains: [],
            fmOsc: null,
            fmGain: null,
            filter: null,
            panner: null,
            gain: null,
            _lastHz: -1,
            _lastVol: -1
        };
    }

    _createImpulseResponse(latticeSize) {
        if (!this.ctx) return null;
        const sampleRate = this.ctx.sampleRate;
        // Decay time from 0.5s to 3.0s based on lattice size
        const duration = Math.max(0.5, (latticeSize / 129) * 3.0); 
        const length = sampleRate * duration;
        const impulse = this.ctx.createBuffer(2, length, sampleRate);
        const left = impulse.getChannelData(0);
        const right = impulse.getChannelData(1);
        for (let i = 0; i < length; i++) {
            const decay = Math.exp(-i / (sampleRate * (duration / 4)));
            left[i] = (Math.random() * 2 - 1) * decay;
            right[i] = (Math.random() * 2 - 1) * decay;
        }
        return impulse;
    }

    async init() {
        if (!this.ctx) {
            const AudioContextClass = window.AudioContext || window.webkitAudioContext;
            if (!AudioContextClass) return;
            this.ctx = new AudioContextClass();

            // Master Bus
            this.masterGain = this.ctx.createGain();
            this.masterGain.gain.value = 1.0;
            this.masterGain.connect(this.ctx.destination);

            this.masterShaper = this.ctx.createWaveShaper();
            this.masterShaper.curve = this._makeDistortionCurve(0); // 0 = no distortion
            this.masterShaper.connect(this.masterGain);

            this.reverbMix = this.ctx.createGain();
            this.reverbMix.gain.value = 0;
            this.reverbMix.connect(this.masterShaper);

            this.convolver = this.ctx.createConvolver();
            this.convolver.buffer = this._createImpulseResponse(33);
            this.convolver.connect(this.reverbMix);

            this.dryMix = this.ctx.createGain();
            this.dryMix.gain.value = 1;
            this.dryMix.connect(this.masterShaper);

            // Init Channels
            for (const ch of this.channels) {
                ch.panner = this.ctx.createStereoPanner();
                ch.panner.pan.value = 0;
                ch.panner.connect(this.dryMix);
                ch.panner.connect(this.convolver);

                ch.gain = this.ctx.createGain();
                ch.gain.gain.value = 0;
                ch.gain.connect(ch.panner);

                ch.filter = this.ctx.createBiquadFilter();
                ch.filter.type = 'lowpass';
                ch.filter.frequency.value = 22000;
                ch.filter.connect(ch.gain);

                // Primary Osc
                ch.osc = this.ctx.createOscillator();
                ch.osc.type = this._oscType;
                ch.osc.connect(ch.filter);
                ch.osc.start();

                // Detune Osc
                ch.oscDetune = this.ctx.createOscillator();
                ch.oscDetune.type = this._oscType;
                ch.oscDetune.connect(ch.filter);
                ch.oscDetune.start();

                // Additive Harmonics
                for (let i = 0; i < 8; i++) {
                    const aOsc = this.ctx.createOscillator();
                    aOsc.type = this._oscType;
                    const aGain = this.ctx.createGain();
                    aGain.gain.value = 0;
                    aOsc.connect(aGain);
                    aGain.connect(ch.filter);
                    aOsc.start();
                    ch.additiveOscs.push(aOsc);
                    ch.additiveGains.push(aGain);
                }

                // FM Modulator
                ch.fmOsc = this.ctx.createOscillator();
                ch.fmOsc.type = 'sine';
                ch.fmGain = this.ctx.createGain();
                ch.fmGain.gain.value = 0;
                ch.fmOsc.connect(ch.fmGain);
                ch.fmGain.connect(ch.osc.frequency);
                ch.fmGain.connect(ch.oscDetune.frequency);
                for (const aOsc of ch.additiveOscs) {
                    ch.fmGain.connect(aOsc.frequency);
                }
                ch.fmOsc.start();
            }
        }

        if (this.ctx.state === 'suspended') {
            await this.ctx.resume();
        }
        
        this.active = true;
    }

    _makeDistortionCurve(amount) {
        if (amount <= 0) return new Float32Array([ -1, 1 ]);
        const k = amount * 100; // Drive
        const n_samples = 44100;
        const curve = new Float32Array(n_samples);
        const deg = Math.PI / 180;
        for (let i = 0; i < n_samples; ++i) {
            const x = i * 2 / n_samples - 1;
            curve[i] = (3 + k) * x * 20 * deg / (Math.PI + k * Math.abs(x));
        }
        return curve;
    }

    setOscType(type) {
        this._oscType = type;
        for (const ch of this.channels) {
            if (ch.osc) ch.osc.type = type;
            if (ch.oscDetune) ch.oscDetune.type = type;
            for (const aOsc of ch.additiveOscs) {
                if (aOsc) aOsc.type = type;
            }
        }
    }

    stop() {
        this.active = false;
        for (const ch of this.channels) {
            if (ch.gain && this.ctx) {
                ch.gain.gain.setTargetAtTime(0, this.ctx.currentTime, 0.05);
            }
        }
        if (this.ctx && this.ctx.state === 'running') {
            this.ctx.suspend();
        }
    }

    update(metrics) {
        if (!this.active || !this.ctx || !metrics || !metrics.active) {
            this._silenceAll();
            return;
        }

        const lanes = metrics.lanes?.filter(l => l.set === 'sound') || [];
        if (lanes.length === 0) {
            this._silenceAll();
            return;
        }

        // Global Effects
        this.masterGain.gain.setTargetAtTime(this.masterVolume, this.ctx.currentTime, 0.05);

        if (this.reverbEnabled) {
            this.reverbMix.gain.setTargetAtTime(0.5, this.ctx.currentTime, 0.1);
            this.dryMix.gain.setTargetAtTime(0.7, this.ctx.currentTime, 0.1);
            // Re-render impulse response if lattice size changes significantly
            if (!this._lastLatticeSize || Math.abs(this._lastLatticeSize - metrics.latticeSize) > 5) {
                this.convolver.buffer = this._createImpulseResponse(metrics.latticeSize);
                this._lastLatticeSize = metrics.latticeSize;
            }
        } else {
            this.reverbMix.gain.setTargetAtTime(0, this.ctx.currentTime, 0.1);
            this.dryMix.gain.setTargetAtTime(1.0, this.ctx.currentTime, 0.1);
        }

        // Collision Master Distortion
        if (lanes.length > 1) {
            // Very basic collision detection: if energy is high in the center
            const centerProbeE = (lanes[0].sampleFlux * lanes[0].sampleFlux) + (lanes[1].sampleFlux * lanes[1].sampleFlux);
            const distAmount = Math.min(1.0, centerProbeE * 10);
            // Throttle wave shaper updates to avoid clicks
            if (!this._lastDistAmount || Math.abs(this._lastDistAmount - distAmount) > 0.1) {
                this.masterShaper.curve = this._makeDistortionCurve(distAmount * 5);
                this._lastDistAmount = distAmount;
            }
        } else {
            if (this._lastDistAmount > 0) {
                this.masterShaper.curve = this._makeDistortionCurve(0);
                this._lastDistAmount = 0;
            }
        }

        // Process up to 2 lanes
        for (let i = 0; i < 2; i++) {
            const ch = this.channels[i];
            const lane = lanes[i];
            
            if (!lane) {
                if (ch.gain) ch.gain.gain.setTargetAtTime(0, this.ctx.currentTime, 0.05);
                continue;
            }

            const targetHz = (lane.frequency || 0) * this.hzScale;
            
            let rawAmp = lane.peakFlux || lane.amplitude || 0;
            if (this.tremoloEnabled) {
                rawAmp = Math.abs(lane.sampleWaveVel || 0) * 2.5; 
            }
            
            // Base volume down to leave headroom for additive & detune
            let gainScale = 15;
            if (this.detuneEnabled) gainScale /= 2;
            if (this.additiveEnabled) gainScale /= 2;
            const targetVol = Math.min(1.0, rawAmp * gainScale);

            // Filter Cutoff
            let targetCutoff = 22000;
            if (this.filterEnabled) {
                const e = Math.min(1.0, (lane.energy || 0) * 2.5);
                targetCutoff = 400 + (7600 * e);
            }

            // Spatial Panning
            let targetPan = 0;
            if (this.panningEnabled) {
                if (lanes.length > 1) {
                    // Hard pan collisions
                    targetPan = i === 0 ? -0.8 : 0.8;
                } else {
                    const N = metrics.latticeSize || 33;
                    const cx = lane.energyCentroidX || (N / 2);
                    targetPan = Math.max(-1, Math.min(1, ((cx / N) * 2) - 1));
                }
            }

            // Update Osc Frequencies
            if (Math.abs(ch._lastHz - targetHz) > 0.1) {
                ch.osc.frequency.setTargetAtTime(targetHz, this.ctx.currentTime, 0.05);
                
                if (this.detuneEnabled) {
                    ch.oscDetune.frequency.setTargetAtTime(targetHz * 1.015, this.ctx.currentTime, 0.05);
                } else {
                    ch.oscDetune.frequency.setTargetAtTime(targetHz, this.ctx.currentTime, 0.05);
                }

                // Additive Synth frequencies
                for (let h = 0; h < 8; h++) {
                    const hFreq = targetHz * (h + 1);
                    ch.additiveOscs[h].frequency.setTargetAtTime(Math.min(22000, hFreq), this.ctx.currentTime, 0.05);
                }

                ch._lastHz = targetHz;
            }

            // FM Synthesis
            if (this.fmEnabled) {
                // Modulator plays targetHz * 2 for metallic sounds
                ch.fmOsc.frequency.setTargetAtTime(targetHz * 2.1, this.ctx.currentTime, 0.05);
                const fmDepth = targetHz * 15 * Math.abs(lane.sampleFlux || 0);
                ch.fmGain.gain.setTargetAtTime(fmDepth, this.ctx.currentTime, 0.05);
            } else {
                ch.fmGain.gain.setTargetAtTime(0, this.ctx.currentTime, 0.05);
            }

            // Additive Harmonics Volumes
            if (this.additiveEnabled && lane.harmonics) {
                // Harmonic 0 is the fundamental, which ch.osc already handles.
                // We'll use additiveOscs for harmonics 2 through 9.
                for (let h = 0; h < 8; h++) {
                    // lane.harmonics has 8 slots (modes 1 through 8)
                    // If fundamental is mode 1, then mode 2 is harmonic 1
                    const mag = lane.harmonics[h] || 0;
                    const aVol = Math.min(1.0, mag * gainScale);
                    ch.additiveGains[h].gain.setTargetAtTime(aVol, this.ctx.currentTime, 0.05);
                }
            } else {
                for (let h = 0; h < 8; h++) {
                    ch.additiveGains[h].gain.setTargetAtTime(0, this.ctx.currentTime, 0.05);
                }
            }

            // Main Volume
            if (Math.abs(ch._lastVol - targetVol) > 0.001) {
                ch.gain.gain.setTargetAtTime(targetVol, this.ctx.currentTime, 0.05);
                ch._lastVol = targetVol;
            }

            // Continuous Effects
            ch.filter.frequency.setTargetAtTime(targetCutoff, this.ctx.currentTime, 0.05);
            ch.panner.pan.setTargetAtTime(targetPan, this.ctx.currentTime, 0.05);
        }
    }

    _silenceAll() {
        for (const ch of this.channels) {
            if (ch.gain && this.ctx) {
                ch.gain.gain.setTargetAtTime(0, this.ctx.currentTime, 0.05);
                ch._lastVol = 0;
            }
        }
    }
}

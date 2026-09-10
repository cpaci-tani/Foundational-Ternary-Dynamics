/**
 * @file engine/web/js/scales/scale0/ui/overlays/p1-observables/g2.js
 * @purpose Imported lepton g-2 reference and live scalar state-label display.
 */

import { BaseComponent } from '../../../../../core/component.js';
import { ALPHA, SCHWINGER_C2, A_E_CODATA, A_MU_CODATA } from '../../../../../constants.js';
import { cardStyle, titleStyle, tagBadge } from '../_card-helpers.js';

const PANEL_ID = 'p1-observables-panel';
const TWO_PI = 2.0 * Math.PI;

const TEMPLATE = `
    <section data-section="g2" style="${cardStyle(312)}">
        <div style="${titleStyle()}">Lepton g−2 · QED reference</div>
        <div ref="stats"></div>
        <div ref="precession" class="p1-g2-precession-box"></div>
    </section>
`;

export class G2Component extends BaseComponent {
    constructor(historyControl = null) {
        super(TEMPLATE);
        this.trackingState = null;
        this.bridgeRef = null;
        this.historyControl = historyControl;

        this._renderG2Section(this.refs.stats);

        // Click event delegation local to this component
        this.element.addEventListener('click', (e) => {
            const target = e.target.closest('button');
            if (!target) return;
            if (target.id === `${PANEL_ID}-g2-track-btn`) {
                if (this.bridgeRef) this._trackParticle(this.bridgeRef);
            } else if (target.id === `${PANEL_ID}-g2-untrack-btn`) {
                this._untrackParticle();
            }
        });
    }

    update(bridge, particles = null, provenance = null) {
        const generation = provenance?.generation ?? bridge?.lifecycleDebug?.configurationToken ?? null;
        const mutationEpoch = provenance?.mutationEpoch ?? null;
        const ready = !!bridge && provenance?.ready !== false;
        if (this.trackingState && (!ready || this.trackingState.owner !== bridge
            || this.trackingState.generation !== generation
            || this.trackingState.mutationEpoch !== mutationEpoch)) this._untrackParticle();
        this.bridgeRef = ready ? bridge : null;
        this._recordGeneration = generation;
        this._mutationEpoch = mutationEpoch;
        
        if (this.trackingState) {
            const particleList = particles || bridge.getScale0ParticleList?.() || [];
            const tracked = particleList.find((p) => p.id === this.trackingState.trackedId);
            if (tracked && Number.isFinite(tracked.spin) && Math.abs(tracked.spin) === 1
                && [tracked.x, tracked.y, tracked.z].every(Number.isFinite)) {
                this.trackingState.position = { x: tracked.x, y: tracked.y, z: tracked.z };
                this.trackingState.spin = Number.isFinite(tracked.spin) ? tracked.spin : null;
                // Scalar genesis spin is not a three-dimensional precession
                // observable. Do not manufacture zero measurements or histories.
            } else {
                // Tracked particle disappeared — auto-untrack
                const sam = this._getSpinArrowManager();
                if (sam) sam.untrack(this.trackingState.trackedId);
                this.trackingState = null;
            }
        }
        
        this._renderG2PrecessionSubsection(this.refs.precession, this.trackingState);
    }

    onUnmount() {
        this.invalidateTracking();
    }

    invalidateTracking() {
        this._untrackParticle();
        this.bridgeRef = null;
        this._renderG2PrecessionSubsection(this.refs.precession, null);
    }

    _getSpinArrowManager() {
        try {
            return window.__ftdCtx?.viewport?.spinArrowManager || null;
        } catch (_) { return null; }
    }

    _trackParticle(bridge) {
        if (bridge !== this.bridgeRef) return;
        const particles = bridge.getScale0ParticleList?.() || [];
        const tracked = particles.find((p) => (p.state ?? 0) !== 0 && Math.abs(p.spin) === 1
            && Number.isFinite(p.spin) && [p.x, p.y, p.z].every(Number.isFinite));
        if (!tracked) return;
        this._untrackParticle();

        this.trackingState = {
            owner: bridge,
            generation: this._recordGeneration ?? null,
            mutationEpoch: this._mutationEpoch ?? null,
            trackedId: tracked.id,
            position: { x: tracked.x, y: tracked.y, z: tracked.z },
            spin: tracked.spin,
        };

        const sam = this._getSpinArrowManager();
        if (sam) {
            const trackedId = tracked.id;
            sam.track(trackedId, {
                // The viewport asks these callbacks on every rendered frame.
                // Reading the bridge in each callback used to schedule a native
                // particle-frame request at render rate. The P1 panel refreshes
                // the real tracked snapshot above at its bounded cadence, so
                // the label display follows the sampled record at that cadence.
                getPosition: () => {
                    const state = this.trackingState;
                    if (!state || state.trackedId !== trackedId) return null;
                    const p = state.position;
                    return { x: p.x + 0.5, y: p.y + 0.5, z: p.z + 0.5 };
                },
                // Real genesis-assigned spin (Voxel::spin, added to the WASM
                // boundary 2026-07-14) flips the arrow between +z/-z. This is
                // still not a true 3D spin axis — the engine only tracks a
                // scalar +-1 per voxel, assigned once at manifestation and
                // never evolved — but it
                // is the REAL value instead of the previous hardcoded {sz:1}.
                getSpin: () => {
                    const state = this.trackingState;
                    if (!state || state.trackedId !== trackedId || !Number.isFinite(state.spin)) return null;
                    // Scalar state labels have no measured precession rate.
                    return { sx: 0, sy: 0, sz: Math.sign(state.spin), omega_z: 0 };
                },
                omegaDefault: 0,
            });
        }
    }

    _untrackParticle() {
        if (this.trackingState) {
            const sam = this._getSpinArrowManager();
            if (sam) sam.untrack(this.trackingState.trackedId);
            this.trackingState = null;
        }
    }

    _renderG2Section(container) {
        const a_e_first = ALPHA / TWO_PI;
        const a_e_two = SCHWINGER_C2 * (ALPHA / Math.PI) ** 2;
        const a_e_predicted = a_e_first + a_e_two;
        const relErrFirst = Math.abs(a_e_first - A_E_CODATA) / A_E_CODATA * 100;
        const relErrTwo = Math.abs(a_e_predicted - A_E_CODATA) / A_E_CODATA * 100;

        container.innerHTML = `
            <div class="p1-g2-stats">
                <div>α (CODATA calibration) = <span class="p1-g2-val">${ALPHA.toExponential(6)}</span></div>
                <div>1/α                  = <span class="p1-g2-val">${(1 / ALPHA).toFixed(6)}</span></div>
                <div class="p1-g2-border">
                    Schwinger first-order: a = α/(2π) = <span class="p1-g2-val">${a_e_first.toExponential(5)}</span>
                </div>
                <div>plus 2nd-order: ${SCHWINGER_C2.toFixed(4)}·(α/π)² = <span class="p1-g2-val-warning">${a_e_two.toExponential(2)}</span></div>
                <div>[PARAMETRIC] value (1+2 loop) = <span class="p1-g2-val">${a_e_predicted.toExponential(6)}</span>
                    <span class="p1-g2-desc">— imported QED coefficients evaluated with the calibrated α. This is a reference calculation.</span></div>
                <div class="p1-g2-border">
                    Stored a_e reference = <span class="p1-g2-val">${A_E_CODATA.toExponential(6)}</span>
                    <span class="p1-g2-desc">(imported experimental value)</span>
                </div>
                <div>Stored a_μ reference = <span class="p1-g2-val">${A_MU_CODATA.toExponential(6)}</span>
                    <span class="p1-g2-desc">Higher-order QED terms depend on lepton mass ratios; hadronic and electroweak terms also contribute. The displayed universal terms do not give a full muon prediction.</span>
                </div>
                <div style="margin-top:3px;">
                    rel err (1-loop only): <span class="p1-g2-val-warning">${relErrFirst.toFixed(3)}%</span>;
                    with 2-loop: <span class="p1-g2-val">${relErrTwo.toFixed(3)}%</span>
                </div>
            </div>
            <div class="p1-g2-footer">
                The displayed two-loop expression is the universal QED contribution. Mass-dependent loop terms are omitted; this calculation does not measure a lattice lepton's magnetic moment.
            </div>
        `;
    }

    _renderG2PrecessionSubsection(container, state) {
        if (!state || state.trackedId == null) {
            container.innerHTML = `
                <div class="p1-g2-untracked">
                    <div class="p1-g2-untracked-title">State spin-label tracking <span class="p1-g2-untracked-label">[awaiting tracking]</span></div>
                    <div class="p1-g2-untracked-desc">
                        Click <button id="${PANEL_ID}-g2-track-btn" type="button" class="p1-btn-track">Track first particle</button>
                        to display the scalar spin label of a manifested site. No three-dimensional spin or precession is measured by this engine.
                    </div>
                </div>
            `;
            return;
        }

        const { trackedId, position, spin } = state;

        container.innerHTML = `
            <div class="p1-g2-tracked">
                <div class="p1-g2-tracked-header">
                    <span style="font-weight:600;">State spin-label tracking</span>
                    <button id="${PANEL_ID}-g2-untrack-btn" type="button" class="p1-btn-untrack">untrack</button>
                </div>
                <div class="p1-g2-tracked-grid">
                    <span>${tagBadge('M')}id</span><span class="p1-g2-tracked-val">${trackedId}</span>
                    <span>${tagBadge('M')}position</span><span class="p1-g2-tracked-val">(${position.x.toFixed(1)}, ${position.y.toFixed(1)}, ${position.z.toFixed(1)})</span>
                    <span>${tagBadge('M')}scalar spin label</span><span class="p1-g2-tracked-val-accent">${Number.isFinite(spin) ? spin : 'unavailable'}</span>
                    <span>ω_measured</span><span class="p1-g2-tracked-val-warning">unavailable</span>
                    <span>residual</span><span class="p1-g2-tracked-val-warning">unavailable</span>
                </div>
                <div class="p1-g2-tracked-footer">
                    The arrow displays the scalar state label along a chosen display axis. This record supplies no measured spin precession or magnetic moment. The QED reference calculation above does not evolve the lattice.
                </div>
            </div>
        `;
    }
}

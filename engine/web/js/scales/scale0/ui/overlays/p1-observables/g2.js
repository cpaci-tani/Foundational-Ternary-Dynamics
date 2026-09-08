/**
 * @file engine/web/js/scales/scale0/ui/overlays/p1-observables/g2.js
 * @purpose Lepton g-2 (Schwinger) and live precession component.
 */

import { BaseComponent } from '../../../../../core/component.js';
import { ALPHA, SCHWINGER_C2, A_E_CODATA, A_MU_CODATA } from '../../../../../constants.js';
import { getParticleCharge } from '../../../../../physics/index.js';
import { cardStyle, titleStyle, tagBadge, formatExp } from '../_card-helpers.js';

const PANEL_ID = 'p1-observables-panel';
const TWO_PI = 2.0 * Math.PI;

const TEMPLATE = `
    <section data-section="g2" style="${cardStyle(312)}">
        <div style="${titleStyle()}">Lepton g−2 (Schwinger)</div>
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

    update(bridge, particles = null, tick = null) {
        this.bridgeRef = bridge;
        
        if (this.trackingState) {
            const particleList = particles || bridge.getScale0ParticleList?.() || [];
            const tracked = particleList.find((p) => p.id === this.trackingState.trackedId);
            if (tracked) {
                this.trackingState.position = { x: tracked.x, y: tracked.y, z: tracked.z };
                this.trackingState.spin = Number.isFinite(tracked.spin) ? tracked.spin : 1;
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
        this._untrackParticle();
    }

    _getSpinArrowManager() {
        try {
            return window.__ftdCtx?.viewport?.spinArrowManager || null;
        } catch (_) { return null; }
    }

    _trackParticle(bridge) {
        const particles = bridge.getScale0ParticleList?.() || [];
        const tracked = particles.find((p) => (p.state ?? 0) !== 0);
        if (!tracked) return;

        const bField = { x: 0, y: 0, z: 0.2 };
        const bMag = Math.sqrt(bField.x ** 2 + bField.y ** 2 + bField.z ** 2);
        const q = getParticleCharge(tracked, -1);
        const m_lep = 1.0;
        const a_e = ALPHA / (2 * Math.PI) + SCHWINGER_C2 * (ALPHA / Math.PI) ** 2;
        const omegaPredicted = Math.abs(q) * bMag / m_lep * (1 + a_e);

        this.trackingState = {
            trackedId: tracked.id,
            position: { x: tracked.x, y: tracked.y, z: tracked.z },
            bField,
            omegaPredicted,
            m_lepton_units: m_lep,
            q,
            spin: Number.isFinite(tracked.spin) ? tracked.spin : 1,
        };

        const sam = this._getSpinArrowManager();
        if (sam) {
            const trackedId = tracked.id;
            sam.track(trackedId, {
                // The viewport asks these callbacks on every rendered frame.
                // Reading the bridge in each callback used to schedule a native
                // particle-frame request at render rate. The P1 panel refreshes
                // the real tracked snapshot above at its bounded cadence, so
                // the arrow remains physical without becoming a telemetry pump.
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
                    const sReal = state && state.trackedId === trackedId && Number.isFinite(state.spin)
                        ? state.spin
                        : 1;
                    return { sx: 0, sy: 0, sz: sReal >= 0 ? 1 : -1, omega_z: omegaPredicted };
                },
                omegaDefault: omegaPredicted,
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
                <div>α (FTD ontic chain) = <span class="p1-g2-val">${ALPHA.toExponential(6)}</span></div>
                <div>1/α                  = <span class="p1-g2-val">${(1 / ALPHA).toFixed(6)}</span></div>
                <div class="p1-g2-border">
                    Schwinger first-order: a = α/(2π) = <span class="p1-g2-val">${a_e_first.toExponential(5)}</span>
                </div>
                <div>plus 2nd-order: ${SCHWINGER_C2.toFixed(4)}·(α/π)² = <span class="p1-g2-val-warning">${a_e_two.toExponential(2)}</span></div>
                <div>[PARAMETRIC] value (1+2 loop) = <span class="p1-g2-val">${a_e_predicted.toExponential(6)}</span>
                    <span class="p1-g2-desc">— standard multi-loop QED with FTD's α inserted. The loop functional form is imported; only α is FTD's, so this is evidence about α's <i>value</i>, not a substrate derivation of g−2.</span></div>
                <div class="p1-g2-border">
                    CODATA a_e = <span class="p1-g2-val">${A_E_CODATA.toExponential(6)}</span>
                    <span class="p1-g2-desc">(electron, measured to 0.13 ppt)</span>
                </div>
                <div>CODATA a_μ = <span class="p1-g2-val">${A_MU_CODATA.toExponential(6)}</span>
                    <span class="p1-g2-desc">(muon — shown for reference. The leading Schwinger term α/2π is mass-independent, but a_μ is NOT: hadronic and electroweak contributions scale with m² and dominate the a_μ−a_e difference. Do not read this row as an FTD prediction for the muon.)</span>
                </div>
                <div style="margin-top:3px;">
                    rel err (1-loop only): <span class="p1-g2-val-warning">${relErrFirst.toFixed(3)}%</span>;
                    with 2-loop: <span class="p1-g2-val">${relErrTwo.toFixed(3)}%</span>
                </div>
            </div>
            <div class="p1-g2-footer">
                QED's a = α/(2π) − 0.328·(α/π)² + 1.181·(α/π)³ − ··· is mass-independent through the universal series. This display verifies the chain α (ontic) → a_lepton (Schwinger).
            </div>
        `;
    }

    _renderG2PrecessionSubsection(container, state) {
        if (!state || state.trackedId == null) {
            container.innerHTML = `
                <div class="p1-g2-untracked">
                    <div class="p1-g2-untracked-title">Precession illustration <span class="p1-g2-untracked-label">[awaiting tracking]</span></div>
                    <div class="p1-g2-untracked-desc">
                        Click <button id="${PANEL_ID}-g2-track-btn" type="button" class="p1-btn-track">Track first particle</button>
                        to attach a spin illustration to the first manifested particle. Precession is not measured by this engine.
                    </div>
                </div>
            `;
            return;
        }

        const { trackedId, position, bField, omegaPredicted } = state;
        const bMag = Math.hypot(bField.x, bField.y, bField.z);

        container.innerHTML = `
            <div class="p1-g2-tracked">
                <div class="p1-g2-tracked-header">
                    <span style="font-weight:600;">Precession illustration</span>
                    <button id="${PANEL_ID}-g2-untrack-btn" type="button" class="p1-btn-untrack">untrack</button>
                </div>
                <div class="p1-g2-tracked-grid">
                    <span>${tagBadge('M')}id</span><span class="p1-g2-tracked-val">${trackedId}</span>
                    <span>${tagBadge('M')}position</span><span class="p1-g2-tracked-val">(${position.x.toFixed(1)}, ${position.y.toFixed(1)}, ${position.z.toFixed(1)})</span>
                    <span>${tagBadge('T')}imposed |B|</span><span class="p1-g2-tracked-val-accent">${formatExp(bMag)}</span>
                    <span>${tagBadge('T')}ω_reference</span><span class="p1-g2-tracked-val-accent">${formatExp(omegaPredicted)}</span>
                    <span>ω_measured</span><span class="p1-g2-tracked-val-warning">unavailable</span>
                    <span>residual</span><span class="p1-g2-tracked-val-warning">unavailable</span>
                </div>
                <div class="p1-g2-tracked-footer">
                    Imposed illustrative field |B| = 0.2 and mass = 1; imported precession formula ω_reference = (q·|B|/m)·(1+a_e). Arrow motion illustrates this formula. This engine has no measured spin-precession observable or residual.
                </div>
            </div>
        `;
    }
}

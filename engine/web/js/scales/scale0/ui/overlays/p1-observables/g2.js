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
const OMEGA_HISTORY_LEN = 60;

const TEMPLATE = `
    <section data-section="g2" style="${cardStyle(312)}">
        <div style="${titleStyle()}">Lepton g−2 (Schwinger)</div>
        <div ref="stats"></div>
        <div ref="precession" class="p1-g2-precession-box"></div>
    </section>
`;

export class G2Component extends BaseComponent {
    constructor() {
        super(TEMPLATE);
        this.trackingState = null;
        this.bridgeRef = null;

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

    update(bridge) {
        this.bridgeRef = bridge;
        
        if (this.trackingState) {
            const particles = bridge.getScale0ParticleList?.() || [];
            const tracked = particles.find((p) => p.id === this.trackingState.trackedId);
            if (tracked) {
                this.trackingState.position = { x: tracked.x, y: tracked.y, z: tracked.z };
                this.trackingState.omegaMeasured = 0;
                this.trackingState.omegaHistory.push(this.trackingState.omegaMeasured);
                while (this.trackingState.omegaHistory.length > OMEGA_HISTORY_LEN) {
                    this.trackingState.omegaHistory.shift();
                }
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
            omegaMeasured: 0,
            omegaHistory: [],
            m_lepton_units: m_lep,
            q,
        };

        const sam = this._getSpinArrowManager();
        if (sam) {
            const trackedId = tracked.id;
            sam.track(trackedId, {
                getPosition: () => {
                    const ps = bridge?.getScale0ParticleList?.() || [];
                    const p = ps.find((pp) => pp.id === trackedId);
                    return p ? { x: p.x + 0.5, y: p.y + 0.5, z: p.z + 0.5 } : null;
                },
                getSpin: () => ({ sx: 0, sy: 0, sz: 1, omega_z: omegaPredicted }),
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
                <div>FTD prediction (1+2 loop) = <span class="p1-g2-val">${a_e_predicted.toExponential(6)}</span></div>
                <div class="p1-g2-border">
                    CODATA a_e = <span class="p1-g2-val">${A_E_CODATA.toExponential(6)}</span>
                    <span class="p1-g2-desc">(electron, measured to 0.13 ppt)</span>
                </div>
                <div>CODATA a_μ = <span class="p1-g2-val">${A_MU_CODATA.toExponential(6)}</span>
                    <span class="p1-g2-desc">(muon — same Schwinger formula; mass-independent at QED order)</span>
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
                    <div class="p1-g2-untracked-title">Live precession <span class="p1-g2-untracked-label">[awaiting tracking]</span></div>
                    <div class="p1-g2-untracked-desc">
                        Click <button id="${PANEL_ID}-g2-track-btn" type="button" class="p1-btn-track">Track first particle</button>
                        to mount a 3D spin arrow on the first manifested particle and read its precession rate against the Schwinger prediction.
                    </div>
                </div>
            `;
            return;
        }

        const { trackedId, position, bField, omegaPredicted, omegaMeasured, omegaHistory } = state;
        const bMag = Math.sqrt(bField.x * bField.x + bField.y * bField.y + bField.z * bField.z);
        const residualPct = (omegaPredicted !== 0)
            ? Math.abs(omegaMeasured - omegaPredicted) / Math.abs(omegaPredicted) * 100
            : NaN;

        const W = 240, H = 48, m = { left: 8, right: 8, top: 6, bottom: 6 };
        const innerW = W - m.left - m.right;
        const innerH = H - m.top - m.bottom;
        let sparkSvg = `<svg viewBox="0 0 ${W} ${H}" style="width:100%;height:auto;display:block;">`;
        sparkSvg += `<rect x="${m.left}" y="${m.top}" width="${innerW}" height="${innerH}" fill="rgba(255,255,255,0.02)" stroke="var(--border-light, rgba(255,255,255,0.06))" stroke-width="0.5"/>`;
        if (omegaHistory && omegaHistory.length > 1) {
            const minV = Math.min(...omegaHistory, omegaPredicted);
            const maxV = Math.max(...omegaHistory, omegaPredicted);
            const span = (maxV - minV) || Math.abs(omegaPredicted) * 0.5 || 1e-9;
            const ypx = (v) => m.top + (1 - (v - minV) / span) * innerH;
            const yPred = ypx(omegaPredicted);
            sparkSvg += `<line x1="${m.left}" y1="${yPred.toFixed(1)}" x2="${m.left + innerW}" y2="${yPred.toFixed(1)}" stroke="var(--text-muted)" stroke-width="0.8" stroke-dasharray="3,3"/>`;
            let path = '';
            for (let i = 0; i < omegaHistory.length; i++) {
                const fx = i / Math.max(1, omegaHistory.length - 1);
                const x = (m.left + fx * innerW).toFixed(1);
                const y = ypx(omegaHistory[i]).toFixed(1);
                path += (i === 0 ? 'M' : 'L') + x + ',' + y;
            }
            sparkSvg += `<path d="${path}" stroke="var(--accent)" stroke-width="1.4" fill="none"/>`;
        } else {
            sparkSvg += `<text x="${m.left + innerW / 2}" y="${m.top + innerH / 2 + 4}" text-anchor="middle" fill="var(--text-muted)" font-size="10" font-style="italic">collecting samples…</text>`;
        }
        sparkSvg += `</svg>`;

        container.innerHTML = `
            <div class="p1-g2-tracked">
                <div class="p1-g2-tracked-header">
                    <span style="font-weight:600;">Live precession</span>
                    <button id="${PANEL_ID}-g2-untrack-btn" type="button" class="p1-btn-untrack">untrack</button>
                </div>
                <div class="p1-g2-tracked-grid">
                    <span>${tagBadge('M')}id</span><span class="p1-g2-tracked-val">${trackedId}</span>
                    <span>${tagBadge('M')}position</span><span class="p1-g2-tracked-val">(${position.x.toFixed(1)}, ${position.y.toFixed(1)}, ${position.z.toFixed(1)})</span>
                    <span>${tagBadge('D')}|B|</span><span class="p1-g2-tracked-val-accent">${formatExp(bMag)}</span>
                    <span>${tagBadge('D')}ω_predicted</span><span class="p1-g2-tracked-val-accent">${formatExp(omegaPredicted)}</span>
                    <span>${tagBadge('~M')}ω_measured</span><span class="p1-g2-tracked-val-warning">${formatExp(omegaMeasured)}</span>
                    <span>${tagBadge('M')}residual</span><span class="p1-g2-tracked-val-warning">${Number.isFinite(residualPct) ? residualPct.toFixed(1) + '%' : '—'}</span>
                </div>
                <div class="p1-g2-tracked-svg-box">${sparkSvg}</div>
                <div class="p1-g2-tracked-footer">
                    ${tagBadge('D')}ω_predicted = (q·|B|/m_lepton)·(1+a_e). 3D arrow rotates at this rate. ${tagBadge('~M')}ω_measured = 0 currently — engine has no spin-precession physics yet (particle.spin is randomly initialized at manifestation, no torque-from-B). Residual slot is reserved; once engine adds spin dynamics, the [~M] tag promotes to [M].
                </div>
            </div>
        `;
    }
}

// Scale Context — docked Scale-0 instrument (FTD-0306).
//
// The panel separates engine-acknowledged geometry, live telemetry, and
// conditional physical calibration. Size changes arrive only after the active
// engine accepts a configuration. The low-rate telemetry loop updates values;
// it never rebuilds the ruler, cards, or lattice-size rail.

import { rafCoordinator } from '../../../../lib/raf-coordinator.js';
import { isPanelLive } from '../../../../ui/panels/panel-visibility.js';
import { resolveActiveScale0BridgeFromWindow } from '../../state/store.js';
import {
    PLANCK_LENGTH_M, PLANCK_TIME_S,
    FTD_ELECTRON_PLANCK_RATIO, FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M,
    FTD_ELECTRON_PRIMARY_PLANCK_TIME_S, FTD_PLANCK_LENGTH_RELATIVE_ERROR,
    FTD_TICK_S, HBAR_C_MEV_M, M_PLANCK_GEV, K_B, K_GENESIS, M_E_PHYS, C_WAVE,
} from '../../../../constants.js';

const PANEL_ID = 'scale-context-panel';
export const SCALE0_LATTICE_SIZE_ACK_EVENT = 'ftd:scale0-lattice-size-ack';

const E_LHC_GEV = 13600.0;
const LHC_LEN_M = HBAR_C_MEV_M / (E_LHC_GEV * 1000.0);
const OMEGA_MAX = 2 * C_WAVE;
const PAIR_MEV = 2 * M_E_PHYS;
const GRB_E_GEV = 10.0;
const LOG_MIN = Math.log10(FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M);
const LOG_MAX = -9;
const INTEGER_FORMAT = new Intl.NumberFormat('en-US', { maximumFractionDigits: 0 });

function normalizeSize(value) {
    const size = Number(value);
    return Number.isInteger(size) && size > 0 ? size : null;
}

function sci(x, digits = 2) {
    if (!Number.isFinite(x) || x === 0) return '0';
    const [mantissa, exponentText] = x.toExponential(digits).split('e');
    const exponent = Number(exponentText);
    if (exponent === 0) return mantissa;
    const superscript = String(exponent)
        .replace('-', '⁻')
        .replace(/\d/g, (digit) => '⁰¹²³⁴⁵⁶⁷⁸⁹'[digit]);
    return `${mantissa}×10${superscript}`;
}

function row(label, ref, tip, { live = false } = {}) {
    return `<div class="sc-row${live ? ' sc-row--live' : ''}">
        <dt title="${tip}">${label}</dt>
        <dd data-sc-ref="${ref}">—</dd>
    </div>`;
}

function buildPanel() {
    const root = document.createElement('section');
    root.id = PANEL_ID;
    root.className = 'scale-context-shell';
    root.setAttribute('role', 'region');
    root.setAttribute('aria-labelledby', `${PANEL_ID}-title`);
    root.innerHTML = `
        <header class="sc-header">
            <div>
                <div class="sc-kicker">Scale 0 · live instrument</div>
                <h2 class="sc-title" id="${PANEL_ID}-title">Scale Context</h2>
                <p class="sc-copy">Computational extent and the default electron-primary SI mapping for the running lattice.</p>
            </div>
            <output class="sc-ack" data-sc-ref="ack" aria-live="polite" title="This value changes only after the active engine acknowledges a lattice configuration.">
                <span class="sc-ack-dot" aria-hidden="true"></span>
                <span><strong data-sc-ref="badge">L=—</strong><small>engine acknowledged</small></span>
            </output>
        </header>

        <section class="sc-hero" aria-labelledby="${PANEL_ID}-window-title">
            <div class="sc-section-heading">
                <div><span class="sc-eyebrow">Computational window</span><h3 id="${PANEL_ID}-window-title">Current lattice</h3></div>
                <span class="sc-tag">LIVE</span>
            </div>
            <div class="sc-dimension" data-sc-ref="dimension">— × — × —</div>
            <div class="sc-site-count" data-sc-ref="sites">Waiting for engine acknowledgement</div>
            <div class="sc-hero-metrics">
                <div><span>Edge calibration</span><strong data-sc-ref="edge-span">—</strong></div>
                <div><span>Size class</span><strong data-sc-ref="size-class">—</strong></div>
            </div>
        </section>

        <section class="sc-size-panel" aria-labelledby="${PANEL_ID}-sizes-title">
            <div class="sc-section-heading">
                <div><span class="sc-eyebrow">Supported profiles</span><h3 id="${PANEL_ID}-sizes-title">Lattice-size rail</h3></div>
                <span class="sc-tag sc-tag--muted">WASM → NATIVE</span>
            </div>
            <div class="sc-size-track" data-sc-ref="size-track" role="img" aria-label="Supported lattice sizes">
                <div class="sc-size-track-line" aria-hidden="true"><i></i></div>
                <ol class="sc-size-marks" data-sc-ref="size-marks"></ol>
            </div>
            <p class="sc-caption"><span class="sc-key sc-key--wasm"></span>browser-capable <span class="sc-key sc-key--native"></span>native-only ceiling</p>
        </section>

        <section class="sc-physical" aria-labelledby="${PANEL_ID}-physical-title">
            <div class="sc-section-heading">
                <div><span class="sc-eyebrow">Physical context</span><h3 id="${PANEL_ID}-physical-title">Log-length position</h3></div>
                <span class="sc-tag sc-tag--calibration">[SMC · ELECTRON-PRIMARY]</span>
            </div>
            <svg class="sc-ruler" data-sc-ref="ruler" viewBox="0 0 100 18" preserveAspectRatio="none" role="img"></svg>
            <div class="sc-ruler-legend" aria-hidden="true">
                <span><i class="sc-ruler-key sc-ruler-key--live"></i>lattice</span>
                <span><i class="sc-ruler-key sc-ruler-key--lhc"></i>LHC</span>
                <span><i></i>nuclear</span>
                <span><i></i>atomic</span>
            </div>
        </section>

        <div class="sc-card-grid">
            <section class="sc-card" aria-labelledby="${PANEL_ID}-geometry-title">
                <div class="sc-card-title"><h3 id="${PANEL_ID}-geometry-title">Geometry &amp; dimensional map</h3><span class="sc-tag">[SMC · DEFAULT GAUGE]</span></div>
                <dl>
                    ${row('Dimensionless mₑ/mP', 'electron-planck-ratio', 'Kα¹¹ with K = √(2π)·16/3. [SMC], with the K factor carrying [SELECTION].')}
                    ${row('FTD ℓP · 1 voxel', 'voxel-length', 'Electron-primary output: (ℏ/mₑc)·Kα¹¹. Conditional on the [SMC] mass ladder; not the CODATA input.')}
                    ${row('CODATA ℓP reference', 'codata-length', 'Empirical reference only. The signed percentage is (FTD/CODATA − 1)×100.')}
                    ${row('FTD tP = ℓP/c', 'planck-time', 'Electron-primary Planck time derived from the FTD length output and imported c.')}
                    ${row('CODATA tP reference', 'codata-time', 'Empirical Planck-time reference only; it is not used to compute the FTD global tick.')}
                    ${row('1 global tick', 'tick-time', 't_phys = ℓP/(√3·c) = tP^FTD/√3, using selected c_lat = 1/√3.')}
                    ${row('LHC resolves', 'lhc-length', 'ℏc / 13.6 TeV — one resolution element.')}
                    ${row('LHC element / lattice', 'lhc-gap', 'How many current lattice edge lengths fit into one LHC resolution element.', { live: true })}
                </dl>
            </section>

            <section class="sc-card" aria-labelledby="${PANEL_ID}-live-title">
                <div class="sc-card-title"><h3 id="${PANEL_ID}-live-title">Live occupancy</h3><span class="sc-tag sc-tag--live">LIVE</span></div>
                <dl>
                    ${row('Manifested sites', 'manifested-sites', 'Current engine diagnostic count of manifested lattice sites.', { live: true })}
                    ${row('Manifested energy', 'manifested-energy', 'Σ manifested sites × K_B — an engine aggregate. The mapping is [IMPOSED], not a particle identification.', { live: true })}
                    ${row('Largest tracked cluster', 'largest-cluster', 'N·K_B [SMC] — IDENT-NULL (FTD-0262): an energy context, not a named Standard Model particle.', { live: true })}
                </dl>
            </section>

            <section class="sc-card" aria-labelledby="${PANEL_ID}-energy-title">
                <div class="sc-card-title"><h3 id="${PANEL_ID}-energy-title">Energy context</h3><span class="sc-tag">[IMPOSED · SMC]</span></div>
                <dl>
                    ${row('UV cutoff ωmax', 'uv-cutoff', 'Zone edge 2/√3; physically approximately Planck scale under the calibration.')}
                    ${row('Manifestation K_GENESIS', 'genesis', '= 3·K_MANIFEST = 3·W_SC [SELECTION — ADOPTED, FTD-0388].')}
                    ${row('QED pair threshold 2mₑ', 'pair-threshold', 'Standard QED pair threshold, shown only as context; the 3-versus-2 difference is not claimed as a derivation.')}
                    ${row('Δv/c at 10 GeV', 'lv-delta', '(E/E_P)²/8 — no linear term [MEASURED structure, FTD-0299]; continued null results are the prediction (FP-3).')}
                </dl>
            </section>
        </div>

        <aside class="sc-note">
            <strong>Interpretation boundary.</strong> The engine simulates dimensionless substrate structure. The displayed length and time use the default electron-primary gauge and inherit its <b>[SMC]</b>/<b>[SELECTION]</b> status. CODATA values are references, not substituted FTD outputs; legacy Planck-primary remains an allowed alternative gauge. Cluster→MeV context remains <b>[SMC] / IDENT-NULL</b>, not a named-particle identification.
        </aside>`;
    return root;
}

function setText(panel, ref, value) {
    const node = panel.querySelector(`[data-sc-ref="${ref}"]`);
    if (node && node.textContent !== value) node.textContent = value;
}

function collectSizeProfiles(L) {
    const select = document.getElementById('lattice-size');
    const profiles = select
        ? [...select.options].map((option) => ({
            size: normalizeSize(option.value),
            nativeOnly: option.hasAttribute('data-native-only'),
            available: !option.disabled,
        })).filter((profile) => profile.size !== null)
        : [];
    if (L && !profiles.some((profile) => profile.size === L)) {
        profiles.push({ size: L, nativeOnly: L > 97, available: true });
        profiles.sort((a, b) => a.size - b.size);
    }
    return profiles;
}

function paintSizeRail(panel, L) {
    const profiles = collectSizeProfiles(L);
    const marks = panel.querySelector('[data-sc-ref="size-marks"]');
    const track = panel.querySelector('[data-sc-ref="size-track"]');
    if (!marks || !track || profiles.length === 0) return;

    const foundIndex = profiles.findIndex((profile) => profile.size === L);
    const activeIndex = Math.max(0, foundIndex);
    const progress = profiles.length > 1 ? activeIndex / (profiles.length - 1) : 0;
    track.style.setProperty('--sc-size-progress', `${(progress * 100).toFixed(2)}%`);
    track.dataset.activeIndex = String(activeIndex);
    track.setAttribute('aria-label', `Current lattice L=${L}. Supported profiles: ${profiles.map((p) => `L=${p.size}${p.nativeOnly ? ' native-only' : ''}`).join(', ')}.`);
    marks.replaceChildren(...profiles.map((profile) => {
        const item = document.createElement('li');
        item.className = 'sc-size-mark';
        item.classList.toggle('is-active', profile.size === L);
        item.classList.toggle('is-native', profile.nativeOnly);
        item.classList.toggle('is-unavailable', !profile.available);
        item.dataset.size = String(profile.size);
        item.setAttribute('aria-current', profile.size === L ? 'true' : 'false');
        item.title = profile.nativeOnly
            ? `L=${profile.size}: native GPU profile.`
            : `L=${profile.size}: browser-capable profile.`;
        item.innerHTML = `<i aria-hidden="true"></i><span>${profile.size}</span>`;
        return item;
    }));
}

function logPosition(metres) {
    return Math.max(0, Math.min(100,
        ((Math.log10(metres) - LOG_MIN) / (LOG_MAX - LOG_MIN)) * 100));
}

function paintPhysicalRuler(panel, L) {
    const svg = panel.querySelector('[data-sc-ref="ruler"]');
    if (!svg) return;
    const latticeM = L * FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M;
    const points = [
        { x: logPosition(latticeM), className: 'live', label: `current lattice L=${L}` },
        { x: logPosition(LHC_LEN_M), className: 'lhc', label: 'LHC resolution' },
        { x: logPosition(1e-15), className: 'reference', label: 'nuclear scale' },
        { x: logPosition(1e-10), className: 'reference', label: 'atomic scale' },
    ];
    const aria = `Log length context from one Planck length to atomic scale. The current L=${L} lattice spans ${sci(latticeM)} metres. LHC resolution is ${sci(LHC_LEN_M)} metres.`;
    svg.setAttribute('aria-label', aria);
    svg.replaceChildren();
    const namespace = 'http://www.w3.org/2000/svg';
    const axis = document.createElementNS(namespace, 'line');
    axis.setAttribute('x1', '1'); axis.setAttribute('y1', '9');
    axis.setAttribute('x2', '99'); axis.setAttribute('y2', '9');
    axis.setAttribute('class', 'sc-ruler-axis');
    svg.appendChild(axis);
    for (const point of points) {
        const marker = document.createElementNS(namespace, 'circle');
        marker.setAttribute('cx', String(Math.max(1, Math.min(99, point.x))));
        marker.setAttribute('cy', '9');
        marker.setAttribute('r', point.className === 'live' ? '2.2' : '1.45');
        marker.setAttribute('class', `sc-ruler-point sc-ruler-point--${point.className}`);
        const title = document.createElementNS(namespace, 'title');
        title.textContent = point.label;
        marker.appendChild(title);
        svg.appendChild(marker);
    }
}

function readTelemetry(getBridge) {
    const bridge = getBridge?.();
    const capability = bridge?.capabilities?.scale0 ?? null;
    let manifested = null;
    let maxClusterSize = 0;
    let knotTelemetryAvailable = false;
    try {
        const diagnostics = capability?.getScale0Diagnostics?.();
        if (Number.isFinite(diagnostics?.manifested)) manifested = diagnostics.manifested;
        const knot = capability?.getScale0KnotTelemetry?.();
        if (knot && knot.count && knot.size) {
            knotTelemetryAvailable = true;
            for (let index = 0; index < knot.count; index += 1) {
                maxClusterSize = Math.max(maxClusterSize, Number(knot.size[index]) || 0);
            }
        }
    } catch { /* bridge can be between configurations */ }
    return { manifested, maxClusterSize, knotTelemetryAvailable };
}

export function mountScaleContextPanel(host, getBridge) {
    if (!host) return null;
    document.getElementById(PANEL_ID)?.remove();
    const panel = buildPanel();
    host.appendChild(panel);
    let acknowledgedSize = null;
    let telemetrySignature = '';

    function paintStatic(nextSize) {
        const L = normalizeSize(nextSize);
        if (!L || L === acknowledgedSize) return false;
        acknowledgedSize = L;
        panel.dataset.latticeSize = String(L);
        setText(panel, 'badge', `L=${L}`);
        setText(panel, 'dimension', `${L} × ${L} × ${L}`);
        setText(panel, 'sites', `${INTEGER_FORMAT.format(L ** 3)} sites · ${L - 1} intervals per edge`);
        setText(panel, 'edge-span', `${sci(L * FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M)} m`);
        setText(panel, 'size-class', L > 97 ? 'native GPU profile' : 'browser-capable profile');
        setText(panel, 'electron-planck-ratio', sci(FTD_ELECTRON_PLANCK_RATIO, 3));
        setText(panel, 'voxel-length', `${sci(FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M, 4)} m`);
        setText(panel, 'codata-length', `${sci(PLANCK_LENGTH_M, 4)} m · ${FTD_PLANCK_LENGTH_RELATIVE_ERROR >= 0 ? '+' : ''}${(FTD_PLANCK_LENGTH_RELATIVE_ERROR * 100).toFixed(3)}%`);
        setText(panel, 'planck-time', `${sci(FTD_ELECTRON_PRIMARY_PLANCK_TIME_S, 4)} s`);
        setText(panel, 'codata-time', `${sci(PLANCK_TIME_S, 4)} s · reference`);
        setText(panel, 'tick-time', `${sci(FTD_TICK_S, 4)} s`);
        setText(panel, 'lhc-length', `${sci(LHC_LEN_M)} m`);
        setText(panel, 'lhc-gap', `${sci(LHC_LEN_M / (L * FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M))}× longer`);
        setText(panel, 'uv-cutoff', `${OMEGA_MAX.toFixed(3)} rad/tick`);
        setText(panel, 'genesis', `${K_GENESIS.toFixed(3)} MeV`);
        setText(panel, 'pair-threshold', `${PAIR_MEV.toFixed(3)} MeV`);
        const deltaV = Math.pow(GRB_E_GEV / M_PLANCK_GEV, 2) / 8;
        setText(panel, 'lv-delta', sci(deltaV, 1));
        paintSizeRail(panel, L);
        paintPhysicalRuler(panel, L);
        return true;
    }

    function paintTelemetry(force = false) {
        const { manifested, maxClusterSize, knotTelemetryAvailable } = readTelemetry(getBridge);
        const signature = `${manifested ?? 'null'}:${maxClusterSize}:${knotTelemetryAvailable}`;
        if (!force && signature === telemetrySignature) return false;
        telemetrySignature = signature;
        const manifestedSites = manifested === null ? '—' : INTEGER_FORMAT.format(manifested);
        const manifestedEnergy = manifested === null ? '— (start the sim)' : `≈ ${(manifested * K_B).toFixed(1)} MeV`;
        const cluster = maxClusterSize > 0
            ? `${INTEGER_FORMAT.format(maxClusterSize)} sites · ≈ ${(maxClusterSize * K_B).toFixed(1)} MeV`
            : (knotTelemetryAvailable ? 'none yet' : '— (enable Knots tracking)');
        setText(panel, 'manifested-sites', manifestedSites);
        setText(panel, 'manifested-energy', manifestedEnergy);
        setText(panel, 'largest-cluster', cluster);
        return true;
    }

    function update() {
        const bridgeSize = normalizeSize(getBridge?.()?.latticeSize);
        if (bridgeSize) paintStatic(bridgeSize);
        paintTelemetry(true);
    }

    function onSizeAcknowledged(event) {
        const eventSize = normalizeSize(event?.detail?.size);
        const activeSize = normalizeSize(getBridge?.()?.latticeSize);
        // A worker replacement can overlap the final callback from the bridge
        // it superseded. Never let that late acknowledgement repaint the panel
        // away from the bridge that currently owns Scale-0 physics.
        if (!eventSize || (activeSize && eventSize !== activeSize)) return;
        paintStatic(eventSize);
    }

    const initialSize = normalizeSize(getBridge?.()?.latticeSize);
    if (initialSize) paintStatic(initialSize);
    paintTelemetry(true);
    window.addEventListener(SCALE0_LATTICE_SIZE_ACK_EVENT, onSizeAcknowledged);

    const subscription = rafCoordinator.subscribe(`${PANEL_ID}-loop`, { hz: 2, cb: () => {
        if (!isPanelLive(host)) return;
        paintTelemetry();
    } });

    const api = {
        update,
        element: panel,
        get acknowledgedLatticeSize() { return acknowledgedSize; },
        dispose: () => {
            subscription.unsubscribe();
            window.removeEventListener(SCALE0_LATTICE_SIZE_ACK_EVENT, onSizeAcknowledged);
            if (window.__ftdScaleContextPanel === api) window.__ftdScaleContextPanel = null;
            panel.remove();
        },
    };
    window.__ftdScaleContextPanel = api;
    return api;
}

export function initScaleContextPanel() {
    if (typeof document === 'undefined') return null;
    if (typeof window !== 'undefined' && window.__ftdScaleContextPanel) return window.__ftdScaleContextPanel;
    const host = document.getElementById('panel-scale-context');
    if (!host) return null;
    return mountScaleContextPanel(host, () => resolveActiveScale0BridgeFromWindow());
}

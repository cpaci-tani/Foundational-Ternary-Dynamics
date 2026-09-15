// engine/web/js/scales/scale0/ui/overlays/native-transport-legend.js
//
// Honest one-paragraph readout for the native transport overlay (spec 2026-09-15
// native transport overlays, section 7). The current is exact only for the pure
// wave step; energy exchanged by other terms is described as exchange at sites,
// never as current.

const TERM_NAMES = Object.freeze([
    [1 << 0, 'thermostat'],
    [1 << 1, 'Gauss projection'],
    [1 << 2, 'damping'],
    [1 << 3, 'genesis'],
    [1 << 4, 'state-flux coupling'],
    [1 << 5, 'absorbing boundary'],
    [1 << 6, 'non-periodic boundary'],
    [1 << 7, 'flux-cell pump or port'],
]);
const CLOSES_BELOW = 1e-9;
const DEFAULT_THRESHOLD = 0.05;

export function describeNativeTransport(summary) {
    if (!summary) return 'Native transport: waiting for the engine.';
    const head = '[REFERENCE ENGINE] Exact energy transport of the v1 wave step on the 18 lattice links.';
    if (summary.status === 'unavailable') return `${head} Unavailable for this configuration: ${summary.reason}.`;
    if (summary.status !== 'ok') return `${head} Warming: the engine needs two consecutive ticks.`;
    const closure = Number(summary.closure) || 0;
    let balance;
    if (closure <= CLOSES_BELOW) {
        balance = `The energy balance closes at every site to ${closure.toExponential(1)} of the largest local change.`;
    } else {
        const terms = TERM_NAMES.filter(([bit]) => (summary.exchangeTerms >>> 0) & bit).map(([, name]) => name);
        balance = `The energy balance does not close: ${closure.toExponential(1)} of the largest local change is exchanged off the links`
            + (terms.length ? ` by ${terms.join(', ')}.` : '.');
    }
    let exchange = '';
    if (summary.exchangeHidden) {
        exchange = ' Off-link exchange sites are hidden while the thermostat runs, because it exchanges energy at nearly every site.';
    } else if (summary.exchangeSites > 0) {
        exchange = ` Magenta: ${summary.exchangeSites} sites exchanging energy off the links.`;
    }
    const knots = summary.knotCount > 0 ? ` White: ${summary.knotCount} manifested matter clusters.` : '';
    return `${head} ${summary.drawnLinks} links drawn. ${balance}${exchange}${knots}`;
}

export function renderNativeTransportLegend(summary) {
    if (typeof document === 'undefined') return;
    const panel = document.getElementById('native-transport-panel');
    const el = document.getElementById('native-transport-legend');
    if (panel) panel.hidden = !summary;
    if (el && summary) el.textContent = describeNativeTransport(summary);
}

// The user-adjustable draw threshold as a fraction of the frame maximum (spec
// section 7). Defaults to 5% when the control is absent or unreadable.
export function readNativeTransportThreshold() {
    if (typeof document === 'undefined') return DEFAULT_THRESHOLD;
    const pct = Number(document.getElementById('native-transport-threshold')?.value);
    return Number.isFinite(pct) && pct > 0 && pct <= 100 ? pct / 100 : DEFAULT_THRESHOLD;
}

/** Passive readout of the active lattice's published field observations.
 * The existing Scale-0 pipeline supplies aggregate values and provenance.
 * This instrument owns no sampler, preparation, state owner or frame loop.
 */
import { appRegistry } from '../../../../core/registry.js';

const QUANTITIES = Object.freeze([
    { id: 'energy', title: 'Quadratic field energy density', formula: '½|E|² + c²|B|²/2',
        units: 'Native quadratic-field units',
        scope: 'A quadratic field measure. It excludes other terms of the full Hamiltonian and is not gas thermal energy.' },
    { id: 'flux', title: 'Flux amplitude', formula: '|J|', units: 'Native flux-amplitude units',
        scope: 'Magnitude of the active lattice flux field; this is not material mass density.' },
    { id: 'flow', title: 'Field-energy flow', formula: '|S| · Poynting magnitude', units: 'Native Poynting units',
        scope: 'The existing Poynting observable describes field-energy flow, not fluid velocity.' },
]);

function format(value) {
    if (!Number.isFinite(value)) return 'Unavailable';
    if (value === 0) return '0';
    return Math.abs(value) < 0.001 || Math.abs(value) >= 1e5 ? value.toExponential(3) : value.toFixed(4);
}
function text(node, value) {
    const next = String(value);
    if (node.textContent !== next) node.textContent = next;
}
function ensureCss() {
    if (document.getElementById('fluid-panel-css')) return;
    const link = document.createElement('link');
    link.id = 'fluid-panel-css'; link.rel = 'stylesheet'; link.href = 'css/ui/components/fluid-panel.css';
    document.head.appendChild(link);
}
function card(quantity) {
    return `<section class="fluid-card fluid-card-${quantity.id}" aria-labelledby="fluid-${quantity.id}-title">
      <header><h3 id="fluid-${quantity.id}-title">${quantity.title}</h3><span class="fluid-formula">${quantity.formula}</span></header>
      <div class="fluid-mean"><strong data-fluid-${quantity.id}="mean">Unavailable</strong><span>sample mean</span></div>
      <p class="fluid-units">${quantity.units}</p>
      <dl class="fluid-card-details"><div><dt>Sample range</dt><dd data-fluid-${quantity.id}="range">—</dd></div>
      <div><dt>Samples</dt><dd data-fluid-${quantity.id}="count">—</dd></div></dl>
      <p class="fluid-note">${quantity.scope}</p>
    </section>`;
}

/** update({scenarioId,tick,source,energy?,flux?,flow?,sampling?,status})
 * Quantities are precomputed {min,max,mean,count}; no volume is scanned here.
 * sampling is {stride,count,approximate:true}. The ordinary scenario selector
 * and Visualization controls remain the sole preparation and display controls.
 */
export function initFluidPanel() {
    if (typeof document === 'undefined') return null;
    const existing = appRegistry.get('panel:fluid') || (typeof window !== 'undefined' && window.__ftdFluidPanel);
    if (existing) return existing;
    const host = document.getElementById('panel-fluid');
    if (!host) return null;
    ensureCss();
    const panel = document.createElement('section');
    panel.className = 'fluid-panel'; panel.dataset.state = 'empty';
    panel.setAttribute('aria-label', 'Active lattice field observations');
    panel.innerHTML = `
      <header class="fluid-header"><strong>Fluid</strong><span class="fluid-badge">Lattice fields</span></header>
      <p class="fluid-intro">Observe the current lattice. Choose a scene with the scenario selector and show field heatmaps through Visualization.</p>
      <p class="fluid-status" role="status" aria-live="polite">Waiting for field observations.</p>
      <dl class="fluid-provenance">
        <div><dt>Scenario</dt><dd data-fluid-meta="scenario">—</dd></div>
        <div><dt>Sample tick</dt><dd data-fluid-meta="tick">—</dd></div>
        <div><dt>Source</dt><dd data-fluid-meta="source">—</dd></div>
        <div><dt>Spatial sampling</dt><dd data-fluid-meta="sampling">—</dd></div>
      </dl>
      <p class="fluid-sampling-note" data-fluid-sampling-note>No sample has been published.</p>
      <div class="fluid-cards">${QUANTITIES.map(card).join('')}</div>
      <section class="fluid-material" aria-label="Material observables">
        <h3>Material observables</h3>
        <dl><div><dt>Pressure</dt><dd>Unavailable</dd></div>
          <div><dt>Viscosity</dt><dd>Unavailable</dd></div>
          <div><dt>Material transport</dt><dd>Unavailable</dd></div></dl>
        <p class="fluid-note">These require material observables supplied by the engine. Field plots alone do not establish them. Values use native lattice conventions; no SI conversion is applied.</p>
      </section>`;
    host.appendChild(panel);
    const status = panel.querySelector('.fluid-status');
    const samplingNote = panel.querySelector('[data-fluid-sampling-note]');
    const meta = Object.fromEntries([...panel.querySelectorAll('[data-fluid-meta]')].map(node => [node.dataset.fluidMeta, node]));
    const cards = Object.fromEntries(QUANTITIES.map(quantity => [quantity.id, {
        mean: panel.querySelector(`[data-fluid-${quantity.id}="mean"]`),
        range: panel.querySelector(`[data-fluid-${quantity.id}="range"]`),
        count: panel.querySelector(`[data-fluid-${quantity.id}="count"]`),
        host: panel.querySelector(`.fluid-card-${quantity.id}`),
    }]));
    let disposed = false, observation = null;
    const api = {
        update(next) {
            if (disposed) return;
            if (!next || next.tick == null || !next.source) { api.clear(next?.status || 'No current field observation.'); return; }
            observation = next; panel.dataset.state = 'observed';
            panel.dataset.sampleTick = String(next.tick); panel.dataset.source = String(next.source);
            text(status, next.status || 'Observed · active lattice fields');
            text(meta.scenario, next.scenarioId || 'Current scenario');
            text(meta.tick, next.tick); text(meta.source, next.source);
            const sampling = next.sampling || {};
            const stride = Number.isFinite(sampling.stride) && sampling.stride > 0 ? sampling.stride : null;
            const count = Number.isSafeInteger(sampling.count) && sampling.count >= 0 ? sampling.count : null;
            text(meta.sampling, [stride != null ? `Stride ${stride} lattice ${stride === 1 ? 'cell' : 'cells'}` : 'Stride unavailable',
                count != null ? `${count} points` : null].filter(Boolean).join(' · '));
            text(samplingNote, sampling.approximate === true
                ? 'Approximate floating-point field observations at the stated sample tick and spatial stride.'
                : 'Field observations at the stated sample tick and spatial stride.');
            for (const quantity of QUANTITIES) {
                const value = next[quantity.id], target = cards[quantity.id];
                const available = value && Number.isFinite(value.mean) && Number.isFinite(value.min)
                    && Number.isFinite(value.max) && Number.isSafeInteger(value.count) && value.count > 0;
                target.host.dataset.available = available ? 'true' : 'false';
                text(target.mean, available ? format(value.mean) : 'Unavailable');
                text(target.range, available ? `${format(value.min)} … ${format(value.max)}` : '—');
                text(target.count, available ? value.count : '—');
            }
        },
        clear(reason = 'Waiting for field observations.') {
            observation = null; panel.dataset.state = 'empty'; delete panel.dataset.sampleTick; delete panel.dataset.source;
            text(status, reason); for (const node of Object.values(meta)) text(node, '—');
            text(samplingNote, 'No current sample is published.');
            for (const target of Object.values(cards)) {
                target.host.dataset.available = 'false'; text(target.mean, 'Unavailable'); text(target.range, '—'); text(target.count, '—');
            }
        },
        dispose() {
            if (disposed) return; disposed = true; observation = null;
            if (appRegistry.get('panel:fluid') === api) appRegistry.unregister('panel:fluid');
            if (typeof window !== 'undefined' && window.__ftdFluidPanel === api) window.__ftdFluidPanel = null;
            panel.remove();
        },
        get observation() { return observation; },
    };
    appRegistry.register('panel:fluid', api);
    if (typeof window !== 'undefined') window.__ftdFluidPanel = api;
    return api;
}

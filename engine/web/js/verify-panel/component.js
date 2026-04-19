/**
 * Verify panel — evidence-scoreboard component.
 *
 * Renders a static view of engine/web/data/verify-manifest.json. No
 * simulations, no Monte Carlo, no pass/fail verdicts. See
 * docs/superpowers/specs/2026-04-18-verify-panel-redesign-design.md.
 */
import { getVerifyPanelTemplate } from './template.js';
import { renderHeader } from './header.js';
import { renderTier } from './tier.js';

// Resolve the manifest relative to this module, so the URL works whether
// the dev server roots at engine/web or at the project root. new URL(...)
// returns a full href that fetch() accepts; avoids the "/data/..." absolute
// path that only resolves when the docroot happens to be engine/web.
const MANIFEST_URL = new URL('../../data/verify-manifest.json', import.meta.url).href;

export class VerifyPanelComponent {
    constructor({ panelArea }) {
        this.panelArea = panelArea;
        this.manifest = null;
        this.activeFilter = 'all';
    }

    async init() {
        this._ensurePanel();
        this._bindFilters();
        this._bindExport();
        try {
            this.manifest = await this._fetchManifest();
            this._render();
        } catch (err) {
            this._renderError(err);
        }
        return this;
    }

    _ensurePanel() {
        if (!this.panelArea) return;
        if (this.panelArea.querySelector('#panel-verification-lab')) return;
        const tpl = document.createElement('template');
        tpl.innerHTML = getVerifyPanelTemplate().trim();
        this.panelArea.appendChild(tpl.content.firstElementChild);
    }

    async _fetchManifest() {
        const res = await fetch(MANIFEST_URL, { cache: 'no-store' });
        if (!res.ok) throw new Error(`manifest fetch failed: HTTP ${res.status}`);
        return await res.json();
    }

    _bindFilters() {
        const panel = this.panelArea?.querySelector('#panel-verification-lab');
        if (!panel) return;
        panel.querySelectorAll('.verify-filter').forEach((btn) => {
            btn.addEventListener('click', () => {
                this.activeFilter = btn.dataset.filter;
                panel.querySelectorAll('.verify-filter').forEach((b) => {
                    b.classList.toggle('active', b === btn);
                });
                this._renderTiers();
            });
        });
    }

    _bindExport() {
        const btn = this.panelArea?.querySelector('#verify-export-btn');
        if (!btn) return;
        btn.addEventListener('click', () => {
            if (!this.manifest) return;
            const blob = new Blob([JSON.stringify(this.manifest, null, 2)], { type: 'application/json' });
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = `verify-manifest-${this.manifest.build_stamp?.commit || 'unknown'}.json`;
            document.body.appendChild(a);
            a.click();
            a.remove();
            URL.revokeObjectURL(url);
        });
    }

    _render() {
        const panel = this.panelArea?.querySelector('#panel-verification-lab');
        if (!panel) return;
        const headerSlot = panel.querySelector('#verify-header-slot');
        if (headerSlot) headerSlot.innerHTML = renderHeader(this.manifest);
        this._renderTiers();
        this._wireRowAnchors();
    }

    _renderTiers() {
        const panel = this.panelArea?.querySelector('#panel-verification-lab');
        if (!panel || !this.manifest) return;
        const slot = panel.querySelector('#verify-tiers-slot');
        if (!slot) return;
        const tiers = this.manifest.tiers || {};
        const show = (t) => this.activeFilter === 'all' || this.activeFilter === t;
        const parts = [];
        if (show('hard')) parts.push(renderTier('hard', tiers.hard || []));
        if (show('parametric')) parts.push(renderTier('parametric', tiers.parametric || []));
        if (show('unpredicted')) parts.push(renderTier('unpredicted', tiers.unpredicted || []));
        slot.innerHTML = parts.join('\n');
        this._wireRowAnchors();
    }

    _wireRowAnchors() {
        const panel = this.panelArea?.querySelector('#panel-verification-lab');
        if (!panel) return;
        // Give each row an anchor id so header tensions can scroll to it.
        panel.querySelectorAll('.verify-row[data-row-id]').forEach((el) => {
            el.id = `verify-row-${el.dataset.rowId}`;
        });
    }

    _renderError(err) {
        const panel = this.panelArea?.querySelector('#panel-verification-lab');
        if (!panel) return;
        const slot = panel.querySelector('#verify-error-slot');
        if (!slot) return;
        slot.hidden = false;
        slot.textContent = `Failed to load verify manifest: ${err.message}. Regenerate with scripts/proofs/build_verify_manifest.py.`;
    }
}

export function initVerifyPanel({ panelArea }) {
    const component = new VerifyPanelComponent({ panelArea });
    component.init();  // fire-and-forget: errors render in the panel's error slot
    return component;
}

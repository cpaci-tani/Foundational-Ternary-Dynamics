/**
 * Scale 0 Visualization panel shell: what the panel does as a container.
 *
 * The layer switches and their click handlers are wired in
 * scale0/ui/bindings.js and are not touched here. This module adds:
 *
 *   1. Group collapse. Each `.s0-overlay-col` header opens and closes its
 *      group. Until the user chooses, Flux J and any group with a layer on
 *      are open. Several groups can be open; the choice is kept per group.
 *   2. The active strip. `#s0-overlay-active` shows a removable chip for every
 *      layer that is on, derived from the buttons' `.active` state by a
 *      MutationObserver so it cannot drift. A chip's × switches the layer off.
 *   3. The Rendering rows. Scalar and Vector are shown only while a layer
 *      they apply to is on.
 *   4. The filter. `#s0-overlay-search` hides layers that do not match and
 *      opens the groups that hold a match.
 *
 * Idempotent: a second call is a no-op (guarded by panel._shellInit).
 */

import { COL_TO_TOGGLES, SCALAR_RENDER_TOGGLES, VECTOR_STYLE_TOGGLES } from './presets.js';
import { initScale0StandardModelReferenceControl } from './standard-model.js?v=2';

// Under one prefix with the panel's own collapse key, ftd.overlay.scale0.collapsed.
const lsKey = (col) => `ftd.overlay.scale0.group.${col}.collapsed`;
const RETIRED_KEY_PREFIX = 'ftd.s0overlay.inspector.';
let refreshFrame = null;

/** On, and able to draw in the current scenario. */
function isOn(id) {
    const btn = document.getElementById(id);
    return !!btn && btn.classList.contains('active') && !btn.classList.contains('is-inapplicable');
}

function activeSignature() {
    return Object.values(COL_TO_TOGGLES).flat().filter(isOn).join('|');
}

function readCollapsed(col) {
    try {
        const v = localStorage.getItem(lsKey(col));
        if (v !== null) return v === '1';
    } catch { /* fall through to the default */ }
    // No stored choice: Flux J is open, and so is any group with a layer on.
    return col !== 'flux' && !(COL_TO_TOGGLES[col] || []).some(isOn);
}
function writeCollapsed(col, collapsed) {
    try { localStorage.setItem(lsKey(col), collapsed ? '1' : '0'); } catch { /* ignore */ }
}

function setColumnCollapsed(col, collapsed, { persist = false } = {}) {
    col.classList.toggle('is-collapsed', collapsed);
    col.querySelector('.s0-overlay-col-head')?.setAttribute('aria-expanded', collapsed ? 'false' : 'true');
    if (persist) writeCollapsed(col.dataset.col, collapsed);
}

export function initOverlayPanelShell() {
    const panel = document.getElementById('viewport-overlay');
    if (!panel || panel._shellInit) return;
    panel._shellInit = true;

    const body = panel.querySelector('.s0-overlay-body');
    const search = document.getElementById('s0-overlay-search');
    const searchClear = document.getElementById('s0-overlay-search-clear');
    if (!body) return;

    for (const button of body.querySelectorAll('.view-toggle')) {
        button.setAttribute('aria-pressed', button.classList.contains('active') ? 'true' : 'false');
    }
    initScale0StandardModelReferenceControl();
    // The groups were renamed with the regrouping; drop choices stored under the old names.
    try {
        for (const key of Object.keys(localStorage)) {
            if (key.startsWith(RETIRED_KEY_PREFIX)) localStorage.removeItem(key);
        }
    } catch { /* storage unavailable */ }

    // ── 1. Group collapse (per group, persisted) ─────────────────────────────
    for (const col of body.querySelectorAll('.s0-overlay-col')) {
        const head = col.querySelector('.s0-overlay-col-head');
        if (!head) continue;
        setColumnCollapsed(col, readCollapsed(col.dataset.col));
        head.setAttribute('role', 'button');
        head.setAttribute('tabindex', '0');
        const toggle = (ev) => {
            // The clear-× lives inside the head and owns its own handler — ignore it.
            if (ev.target.closest('.s0-overlay-col-clear')) return;
            const next = !col.classList.contains('is-collapsed');
            setColumnCollapsed(col, next, { persist: true });
        };
        head.addEventListener('click', toggle);
        head.addEventListener('keydown', (e) => {
            if (e.key === 'Enter' || e.key === ' ') { e.preventDefault(); toggle(e); }
        });
    }

    // ── 2. Active-overlays strip (derived from button .active state) ─────────
    refreshOverlayPanelShell();
    // Toggle buttons live inside `body`; watch their class changes so the strip
    // (a sibling of body, so its own edits don't re-trigger us) stays in sync.
    new MutationObserver((mutations) => {
        const activeChanged = mutations.map((mutation) => {
            if (!mutation.target.classList?.contains('view-toggle')) return false;
            const oldClasses = new Set((mutation.oldValue || '').split(/\s+/).filter(Boolean));
            const changed = oldClasses.has('active') !== mutation.target.classList.contains('active');
            if (changed) {
                mutation.target.setAttribute(
                    'aria-pressed',
                    mutation.target.classList.contains('active') ? 'true' : 'false',
                );
            }
            return changed;
        }).some(Boolean);
        if (activeChanged && panel._activeSignature !== activeSignature()) {
            scheduleOverlayPanelShellRefresh();
        }
    }).observe(body, {
        subtree: true,
        attributes: true,
        attributeFilter: ['class'],
        attributeOldValue: true,
    });

    // ── 3. Filter ────────────────────────────────────────────────────────────
    if (search) {
        let filterFrame = null;
        search.addEventListener('input', () => {
            if (searchClear) searchClear.hidden = search.value.length === 0;
            if (filterFrame !== null) return;
            filterFrame = requestAnimationFrame(() => {
                filterFrame = null;
                applyFilter(body, search.value);
            });
        });
        search.addEventListener('keydown', (event) => {
            if (event.key !== 'Escape' || !search.value) return;
            event.preventDefault();
            search.value = '';
            search.dispatchEvent(new Event('input', { bubbles: true }));
        });
        searchClear?.addEventListener('click', () => {
            if (!search.value) return;
            search.value = '';
            search.dispatchEvent(new Event('input', { bubbles: true }));
            search.focus({ preventScroll: true });
        });
    }
}

/** Coalesce toggle bursts and MutationObserver delivery to one shell repaint. */
export function scheduleOverlayPanelShellRefresh() {
    if (refreshFrame !== null) return;
    refreshFrame = requestAnimationFrame(() => {
        refreshFrame = null;
        refreshOverlayPanelShell();
    });
}

/**
 * Rebuild all shell state after a scenario changes overlay applicability.
 * Kept public so the scenario loader can update the panel synchronously rather
 * than waiting for the MutationObserver microtask.
 */
export function refreshOverlayPanelShell() {
    if (refreshFrame !== null) {
        cancelAnimationFrame(refreshFrame);
        refreshFrame = null;
    }
    const panel = document.getElementById('viewport-overlay');
    const body = panel?.querySelector('.s0-overlay-body');
    if (!body) return;

    const activeCount = rebuildActiveStrip(document.getElementById('s0-overlay-active'));
    const summary = document.getElementById('s0-overlay-summary');
    if (summary) {
        // The number is its own node: the collapsed pill shows it without "active".
        const count = summary.querySelector('.s0-overlay-summary-count');
        const next = String(activeCount);
        if (count && count.textContent !== next) count.textContent = next;
        summary.classList.toggle('is-empty', activeCount === 0);
    }
    refreshColumnCounts(body);
    refreshRenderRows(panel);
    const search = document.getElementById('s0-overlay-search');
    applyFilter(body, search?.value || '');
    panel._activeSignature = activeSignature();
}

function setHidden(el, hidden) {
    if (el && el.hidden !== hidden) el.hidden = hidden;
}

/** A Rendering row is shown only while a layer it applies to is on. */
function refreshRenderRows(panel) {
    const scalar = SCALAR_RENDER_TOGGLES.some(isOn);
    const vector = VECTOR_STYLE_TOGGLES.some(isOn);
    setHidden(panel.querySelector('[data-render-row="scalar"]'), !scalar);
    setHidden(panel.querySelector('[data-render-row="vector"]'), !vector);
    setHidden(panel.querySelector('.s0-overlay-render-deck'), !scalar && !vector);
}

function refreshColumnCounts(body) {
    for (const [colName, toggles] of Object.entries(COL_TO_TOGGLES)) {
        const badge = body.querySelector(`[data-count-for="${colName}"]`);
        if (!badge) continue;
        const count = toggles.filter(isOn).length;
        const text = String(count);
        if (badge.textContent !== text) {
            const node = badge.firstChild;
            if (badge.childNodes.length === 1 && node?.nodeType === 3) node.nodeValue = text;
            else badge.textContent = text;
        }
        badge.classList.toggle('is-zero', count === 0);
    }
}

function rebuildActiveStrip(strip) {
    if (!strip) return 0;
    const desired = Object.values(COL_TO_TOGGLES).flat().filter(isOn).map((id) => document.getElementById(id));
    const wanted = new Set(desired.map((btn) => btn.id));
    const existing = new Map();
    for (const chip of [...strip.querySelectorAll(':scope > .s0-overlay-chip')]) {
        const id = chip.dataset.overlayId;
        if (!id || !wanted.has(id) || existing.has(id)) chip.remove();
        else existing.set(id, chip);
    }
    let cursor = strip.firstElementChild;
    for (const btn of desired) {
        let chip = existing.get(btn.id);
        if (!chip) chip = makeChip(btn);
        if (chip !== cursor) strip.insertBefore(chip, cursor);
        cursor = chip.nextElementSibling;
    }
    const hidden = desired.length === 0;
    if (strip.hidden !== hidden) strip.hidden = hidden;
    return desired.length;
}

function makeChip(btn) {
    const chip = document.createElement('span');
    chip.className = 's0-overlay-chip';
    chip.dataset.overlayId = btn.id;

    const sw = btn.querySelector('.field-swatch');
    if (sw) {
        const c = document.createElement('span');
        c.className = 's0-overlay-chip-swatch';
        try { c.style.background = getComputedStyle(sw).backgroundColor; } catch { /* ignore */ }
        chip.appendChild(c);
    }

    const text = btn.textContent.trim();
    const label = document.createElement('span');
    label.className = 's0-overlay-chip-label';
    label.textContent = text;
    label.title = text;
    chip.appendChild(label);

    const x = document.createElement('button');
    x.type = 'button';
    // .u-no-baseline opts the × out of the global 44px control-baseline
    // (tokens.css) so it can render as a small em-sized circle.
    x.className = 's0-overlay-chip-x u-no-baseline';
    // The × is drawn in CSS (::before/::after bars) so it stays exactly centered;
    // the accessible name comes from aria-label, so no text glyph is needed.
    x.setAttribute('aria-label', `Turn off ${text}`);
    x.addEventListener('click', (e) => { e.stopPropagation(); btn.click(); });
    chip.appendChild(x);

    return chip;
}

function applyFilter(body, query) {
    const q = query.trim().toLowerCase();
    body.classList.toggle('is-searching', !!q);

    if (!q) {
        body.classList.remove('is-empty');
        for (const btn of body.querySelectorAll('.view-toggle')) btn.classList.remove('is-filtered-out');
        for (const group of body.querySelectorAll('.s0-overlay-group')) group.classList.remove('is-filtered-out');
        for (const col of body.querySelectorAll('.s0-overlay-col')) {
            col.classList.remove('is-filtered-out');
            setColumnCollapsed(col, readCollapsed(col.dataset.col));   // restore persisted
        }
        return;
    }

    let anyMatch = false;
    for (const col of body.querySelectorAll('.s0-overlay-col')) {
        let colMatch = false;
        // Match/hide a layer and its sub-switch row as one unit
        // (a .s0-overlay-group), not independently — otherwise a query that matches
        // only a mini toggle's label (e.g. "glow", "xy") strands that toggle visible
        // with no trigger label above it, and the now-empty group wrapper is left
        // stretched-and-blank in the grid (.s0-overlay-group.is-filtered-out below).
        const units = col.querySelectorAll(':scope > .view-toggle, :scope > .s0-overlay-group');
        for (const unit of units) {
            const isGroup = unit.classList.contains('s0-overlay-group');
            const btns = isGroup ? unit.querySelectorAll('.view-toggle') : [unit];
            let unitMatch = false;
            // Dimmed layers match too: the list is the same in every scenario.
            for (const btn of btns) {
                const searchable = `${btn.textContent} ${btn.dataset.search || ''}`.toLowerCase();
                if (searchable.includes(q)) { unitMatch = true; break; }
            }
            for (const btn of btns) btn.classList.toggle('is-filtered-out', !unitMatch);
            if (isGroup) unit.classList.toggle('is-filtered-out', !unitMatch);
            if (unitMatch) colMatch = true;
        }
        col.classList.toggle('is-filtered-out', !colMatch);
        setColumnCollapsed(col, false);   // auto-expand matching categories
        if (colMatch) anyMatch = true;
    }
    body.classList.toggle('is-empty', !anyMatch);
}

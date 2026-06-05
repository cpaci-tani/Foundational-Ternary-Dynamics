/**
 * Scale-0 Visualization panel shell — accordion + active strip + filter.
 *
 * The panel's toggle buttons and their click handlers are wired elsewhere
 * (scale0/ui/bindings.js) and are NOT touched here. This module only adds the
 * "container" behaviours of the revamped panel:
 *
 *   1. Per-category collapse — each `.s0-overlay-col` header is a clickable
 *      accordion row; all collapsed by default, state persisted per category.
 *      Multiple categories can be open at once.
 *   2. Active-overlays strip — `#s0-overlay-active` shows a removable chip for
 *      every currently-active overlay, DERIVED from the buttons' `.active` state
 *      via a MutationObserver, so it can never drift. A chip's × re-fires that
 *      toggle off.
 *   3. Filter — `#s0-overlay-search` hides non-matching overlay buttons and
 *      auto-expands the categories that contain a match.
 *
 * Idempotent: a second call is a no-op (guarded by panel._shellInit).
 */

import { COL_TO_TOGGLES } from './presets.js';

const lsKey = (col) => `ftd.s0overlay.cat.${col}.collapsed`;

function readCollapsed(col) {
    try {
        const v = localStorage.getItem(lsKey(col));
        return v === null ? true : v === '1';   // default: collapsed
    } catch { return true; }
}
function writeCollapsed(col, collapsed) {
    try { localStorage.setItem(lsKey(col), collapsed ? '1' : '0'); } catch { /* ignore */ }
}

export function initOverlayPanelShell() {
    const panel = document.getElementById('viewport-overlay');
    if (!panel || panel._shellInit) return;
    panel._shellInit = true;

    const body = panel.querySelector('.s0-overlay-body');
    const strip = document.getElementById('s0-overlay-active');
    const search = document.getElementById('s0-overlay-search');
    if (!body) return;

    // ── 1. Accordion collapse (per category, persisted) ──────────────────────
    for (const col of body.querySelectorAll('.s0-overlay-col')) {
        const head = col.querySelector('.s0-overlay-col-head');
        if (!head) continue;
        col.classList.toggle('is-collapsed', readCollapsed(col.dataset.col));
        head.setAttribute('role', 'button');
        head.setAttribute('tabindex', '0');
        const toggle = (ev) => {
            // The clear-× lives inside the head and owns its own handler — ignore it.
            if (ev.target.closest('.s0-overlay-col-clear')) return;
            const next = !col.classList.contains('is-collapsed');
            col.classList.toggle('is-collapsed', next);
            writeCollapsed(col.dataset.col, next);
        };
        head.addEventListener('click', toggle);
        head.addEventListener('keydown', (e) => {
            if (e.key === 'Enter' || e.key === ' ') { e.preventDefault(); toggle(e); }
        });
    }

    // ── 2. Active-overlays strip (derived from button .active state) ─────────
    const refresh = () => rebuildActiveStrip(strip);
    refresh();
    // Toggle buttons live inside `body`; watch their class changes so the strip
    // (a sibling of body, so its own edits don't re-trigger us) stays in sync.
    new MutationObserver(refresh).observe(body, {
        subtree: true, attributes: true, attributeFilter: ['class'],
    });

    // ── 3. Filter ────────────────────────────────────────────────────────────
    if (search) {
        search.addEventListener('input', () => applyFilter(body, search.value));
    }
}

function rebuildActiveStrip(strip) {
    if (!strip) return;
    strip.textContent = '';
    let n = 0;
    for (const toggles of Object.values(COL_TO_TOGGLES)) {
        for (const id of toggles) {
            const btn = document.getElementById(id);
            if (!btn || !btn.classList.contains('active')) continue;
            strip.appendChild(makeChip(btn));
            n++;
        }
    }
    strip.hidden = n === 0;
}

function makeChip(btn) {
    const chip = document.createElement('span');
    chip.className = 's0-overlay-chip';

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
    x.className = 's0-overlay-chip-x';
    x.setAttribute('aria-label', `Turn off ${text}`);
    x.textContent = '×';
    x.addEventListener('click', (e) => { e.stopPropagation(); btn.click(); });
    chip.appendChild(x);

    return chip;
}

function applyFilter(body, query) {
    const q = query.trim().toLowerCase();

    if (!q) {
        body.classList.remove('is-empty');
        for (const btn of body.querySelectorAll('.view-toggle')) btn.classList.remove('is-filtered-out');
        for (const col of body.querySelectorAll('.s0-overlay-col')) {
            col.classList.remove('is-filtered-out');
            col.classList.toggle('is-collapsed', readCollapsed(col.dataset.col));   // restore persisted
        }
        return;
    }

    let anyMatch = false;
    for (const col of body.querySelectorAll('.s0-overlay-col')) {
        let colMatch = false;
        for (const btn of col.querySelectorAll('.view-toggle')) {
            const match = btn.textContent.trim().toLowerCase().includes(q);
            btn.classList.toggle('is-filtered-out', !match);
            if (match) colMatch = true;
        }
        col.classList.toggle('is-filtered-out', !colMatch);
        col.classList.remove('is-collapsed');   // auto-expand matching categories
        if (colMatch) anyMatch = true;
    }
    body.classList.toggle('is-empty', !anyMatch);
}

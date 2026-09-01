/**
 * Shared tick-history viewport control for temporal side-panel charts.
 *
 * The control changes presentation only. Telemetry buffers retain the complete
 * current-run history so switching to "All" never reveals a silently truncated
 * ring. Each panel persists its own window choice.
 */

const STORAGE_PREFIX = 'ftd.chart-history.';
const MIN_TICKS = 10;
const MAX_TICKS = 1000000;
const DEFAULT_TICKS = 160;

function clampTicks(value, fallback = DEFAULT_TICKS) {
    const parsed = Math.round(Number(value));
    if (!Number.isFinite(parsed)) return fallback;
    return Math.max(MIN_TICKS, Math.min(MAX_TICKS, parsed));
}

function loadState(id, defaultTicks) {
    try {
        const value = JSON.parse(localStorage.getItem(`${STORAGE_PREFIX}${id}`) || 'null');
        return {
            mode: value?.mode === 'all' ? 'all' : 'window',
            ticks: clampTicks(value?.ticks, defaultTicks),
        };
    } catch {
        return { mode: 'window', ticks: defaultTicks };
    }
}

function saveState(id, state) {
    try { localStorage.setItem(`${STORAGE_PREFIX}${id}`, JSON.stringify(state)); } catch {}
}

function bufferTick(buffer, index) {
    const tick = buffer?.getTick?.(index);
    return Number.isFinite(tick) ? tick : index;
}

export class TickHistoryControl {
    constructor(container, {
        id,
        defaultTicks = DEFAULT_TICKS,
        insert = 'prepend',
        onChange = null,
    } = {}) {
        this.id = id || container?.id || 'default';
        this.defaultTicks = clampTicks(defaultTicks);
        this.state = loadState(this.id, this.defaultTicks);
        this.listeners = new Set();
        if (typeof onChange === 'function') this.listeners.add(onChange);

        this.el = document.createElement('section');
        this.el.className = 'tick-history-control';
        this.el.dataset.historyControl = this.id;
        this.el.innerHTML = `
            <div class="tick-history-control__label">
                <span>Chart history</span>
                <span class="tick-history-control__summary" aria-live="polite"></span>
            </div>
            <div class="tick-history-control__body">
                <div class="tick-history-control__modes" role="group" aria-label="Chart history mode">
                    <button type="button" data-history-mode="window">Last</button>
                    <button type="button" data-history-mode="all">All</button>
                </div>
                <label class="tick-history-control__ticks">
                    <input type="number" inputmode="numeric" min="${MIN_TICKS}" max="${MAX_TICKS}"
                        step="10" aria-label="Ticks shown in rolling chart window">
                    <span>ticks</span>
                </label>
            </div>
            <div class="tick-history-control__note"></div>
        `;

        this.summaryEl = this.el.querySelector('.tick-history-control__summary');
        this.noteEl = this.el.querySelector('.tick-history-control__note');
        this.input = this.el.querySelector('input');
        this.modeButtons = [...this.el.querySelectorAll('[data-history-mode]')];

        this._onClick = (event) => {
            const button = event.target.closest('[data-history-mode]');
            if (!button) return;
            this.setMode(button.dataset.historyMode);
        };
        this._onInput = () => {
            // Keep partial edits such as "2" while the user is typing "250".
            // Commit only complete in-range values; blur/change clamps anything
            // still outside the supported range.
            const parsed = Math.round(Number(this.input.value));
            if (!Number.isFinite(parsed) || parsed < MIN_TICKS || parsed > MAX_TICKS) return;
            this.setTicks(parsed);
        };
        this._onChange = () => this.setTicks(this.input.value);
        this.el.addEventListener('click', this._onClick);
        this.input.addEventListener('change', this._onChange);
        this.input.addEventListener('input', this._onInput);

        if (container) {
            if (insert === 'append') container.appendChild(this.el);
            else container.prepend(this.el);
        }
        this.render();
    }

    get mode() { return this.state.mode; }
    get ticks() { return this.state.ticks; }
    get isAll() { return this.state.mode === 'all'; }

    setMode(mode) {
        const next = mode === 'all' ? 'all' : 'window';
        if (next === this.state.mode) return;
        this.state.mode = next;
        this._commit();
    }

    setTicks(ticks) {
        const next = clampTicks(ticks, this.state.ticks);
        if (next === this.state.ticks) {
            this.input.value = String(next);
            return;
        }
        this.state.ticks = next;
        this._commit();
    }

    _commit() {
        saveState(this.id, this.state);
        this.render();
        for (const listener of this.listeners) listener(this);
    }

    render() {
        this.input.value = String(this.state.ticks);
        this.input.disabled = this.isAll;
        for (const button of this.modeButtons) {
            const active = button.dataset.historyMode === this.state.mode;
            button.classList.toggle('active', active);
            button.setAttribute('aria-pressed', active ? 'true' : 'false');
        }
        this.summaryEl.textContent = this.isAll ? 'all retained ticks' : `last ${this.state.ticks} ticks`;
        this.noteEl.textContent = this.isAll
            ? 'All current-run samples remain visible; memory use grows until reset.'
            : 'Rolling tick window; no rows are removed from engine computation.';
    }

    subscribe(listener) {
        if (typeof listener !== 'function') return () => {};
        this.listeners.add(listener);
        return () => this.listeners.delete(listener);
    }

    /** Number of trailing samples whose tick stamps fall inside this viewport. */
    visibleCount(buffer) {
        const count = Math.max(0, Number(buffer?.count) || 0);
        if (this.isAll || count < 2) return count;
        const lastTick = bufferTick(buffer, count - 1);
        const minimumTick = lastTick - this.state.ticks;
        let first = count - 1;
        while (first > 0 && bufferTick(buffer, first - 1) >= minimumTick) first--;
        return count - first;
    }

    /** Trailing slice of an array using an explicit tick accessor. */
    slice(entries, getTick = (entry, index) => entry?.tick ?? index) {
        if (!Array.isArray(entries) || this.isAll || entries.length < 2) return entries;
        const lastTick = Number(getTick(entries[entries.length - 1], entries.length - 1));
        if (!Number.isFinite(lastTick)) return entries.slice(-this.state.ticks);
        const minimumTick = lastTick - this.state.ticks;
        let first = entries.length - 1;
        while (first > 0) {
            const tick = Number(getTick(entries[first - 1], first - 1));
            if (!Number.isFinite(tick) || tick < minimumTick) break;
            first--;
        }
        return entries.slice(first);
    }

    destroy() {
        this.el.removeEventListener('click', this._onClick);
        this.input.removeEventListener('change', this._onChange);
        this.input.removeEventListener('input', this._onInput);
        this.listeners.clear();
        this.el.remove();
    }
}

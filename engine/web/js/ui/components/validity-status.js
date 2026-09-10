// A diagnostic readout only: this component never advances or changes physics.
const mounted = new WeakMap();

export function createValidityIndicator(host, { id = 'validity-status' } = {}) {
    if (mounted.has(host)) return mounted.get(host);
    const doc = host.ownerDocument;
    const win = doc.defaultView;
    const button = doc.createElement('button');
    button.type = 'button';
    button.id = id;
    button.className = 'validity-status';
    button.setAttribute('data-ui-tooltip-skip', '');
    button.setAttribute('aria-expanded', 'false');
    const dot = doc.createElement('span');
    dot.className = 'validity-status-dot';
    dot.setAttribute('aria-hidden', 'true');
    const label = doc.createElement('span');
    label.className = 'validity-status-label';
    label.setAttribute('aria-live', 'polite');
    label.setAttribute('aria-atomic', 'true');
    button.append(dot, label);
    const tooltip = doc.createElement('div');
    tooltip.id = `${id}-details`;
    tooltip.className = 'validity-status-details';
    tooltip.setAttribute('role', 'tooltip');
    tooltip.hidden = true;
    button.setAttribute('aria-describedby', tooltip.id);
    host.appendChild(button);
    // A body portal keeps details visible outside the toolbar's overflow area.
    doc.body.appendChild(tooltip);
    let disposed = false;
    let hovered = false;
    let tooltipHovered = false;
    let focused = false;
    let pinned = false;
    let dismissed = false;
    let hideTimer = null;
    const listeners = [];
    function on(target, event, handler, options) {
        target.addEventListener(event, handler, options);
        listeners.push(() => target.removeEventListener(event, handler, options));
    }
    function position() {
        if (tooltip.hidden || disposed) return;
        const anchor = button.getBoundingClientRect();
        const box = tooltip.getBoundingClientRect();
        const margin = 10;
        tooltip.style.left = `${Math.max(margin, Math.min(anchor.left, win.innerWidth - box.width - margin))}px`;
        const below = anchor.bottom + 8;
        tooltip.style.top = `${Math.max(margin, below + box.height <= win.innerHeight - margin
            ? below : anchor.top - box.height - 8)}px`;
    }
    function visibility() {
        tooltip.hidden = dismissed || !(hovered || tooltipHovered || focused || pinned);
        button.setAttribute('aria-expanded', String(!tooltip.hidden));
        position();
    }
    function delayedHide() {
        win.clearTimeout(hideTimer);
        hideTimer = win.setTimeout(visibility, 100);
    }
    on(button, 'pointerenter', () => { hovered = true; dismissed = false; visibility(); });
    on(button, 'pointerleave', () => { hovered = false; delayedHide(); });
    on(tooltip, 'pointerenter', () => { tooltipHovered = true; visibility(); });
    on(tooltip, 'pointerleave', () => { tooltipHovered = false; delayedHide(); });
    on(button, 'focus', () => { focused = true; dismissed = false; visibility(); });
    on(button, 'blur', () => { focused = false; pinned = false; visibility(); });
    on(button, 'click', () => { pinned = !pinned; dismissed = !pinned; visibility(); });
    on(doc, 'keydown', (event) => {
        if (event.key === 'Escape') { dismissed = true; pinned = false; visibility(); }
    });
    on(doc, 'pointerdown', (event) => {
        if (!button.contains(event.target) && !tooltip.contains(event.target)) {
            pinned = false; dismissed = true; visibility();
        }
    });
    on(win, 'resize', position);
    on(win, 'scroll', position, true);
    const api = {
        set({ label: text, severity = 'neutral', details }) {
            if (disposed) return;
            if (typeof text !== 'string' || typeof details !== 'string'
                || !/^[A-Za-z]+$/.test(text) || !['neutral', 'ok', 'warning', 'error'].includes(severity)) {
                throw new TypeError('Validity status requires one word, text details, and a known severity');
            }
            // Identical publications do not generate DOM mutations or layout reads.
            if (label.textContent !== text) {
                label.textContent = text;
                button.setAttribute('aria-label', `Validity: ${text}`);
            }
            if (button.dataset.severity !== severity) button.dataset.severity = severity;
            if (tooltip.textContent !== details) {
                tooltip.textContent = details;
                position();
            }
        },
        reset(details = 'Waiting for a current diagnostic sample.') {
            api.set({ label: 'Unchecked', severity: 'neutral', details });
        },
        destroy() {
            if (disposed) return;
            disposed = true;
            win.clearTimeout(hideTimer);
            listeners.forEach(remove => remove());
            button.remove(); tooltip.remove(); mounted.delete(host);
        },
    };
    api.reset();
    mounted.set(host, api);
    return api;
}

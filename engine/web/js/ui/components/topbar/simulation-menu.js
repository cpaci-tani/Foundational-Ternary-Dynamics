// @ts-check
import { LifetimeScope } from '../../utils/lifetime-scope.js';

/** Enhance the scale control with actual buttons and a workspace action.
 * The original select remains the scale-change gateway for integrations; the
 * Observer action never writes a synthetic physical scale into that gateway.
 * @param {{select:HTMLElement|null,trigger:HTMLElement|null,panel:HTMLElement|null,label:Element|null}} deps
 */
export function wireSimulationMenu({ select, trigger, panel, label }) {
    if (!(select instanceof HTMLSelectElement) || !(trigger instanceof HTMLButtonElement) || !panel) return () => {};
    const choices = panel.querySelector('[data-simulation-scales]');
    const current = trigger.querySelector('[data-simulation-current]');
    if (!choices || !current) return () => {};
    const scope = new LifetimeScope();
    const previous = { selectHidden: select.hidden, triggerHidden: trigger.hidden, labelFor: label?.getAttribute('for') };
    const nativePopover = typeof panel.showPopover === 'function';
    let open = false;
    select.hidden = true; trigger.hidden = false;
    label?.setAttribute('for', trigger.id);
    if (!nativePopover) panel.hidden = true;

    function refresh() {
        if (!choices || !current || !(select instanceof HTMLSelectElement)) return;
        const fragment = document.createDocumentFragment();
        for (const option of select.options) {
            if (option.hidden || option.dataset.testOnly === 'true') continue;
            const item = document.createElement('button');
            item.type = 'button'; item.className = 'simulation-menu-item';
            item.dataset.simulationMode = option.value; item.textContent = option.text;
            item.disabled = option.disabled;
            item.setAttribute('aria-pressed', String(option.value === select.value));
            fragment.append(item);
        }
        choices.replaceChildren(fragment);
        current.textContent = select.selectedOptions[0]?.text || 'Choose simulation';
    }
    function position() {
        if (!panel || !trigger || !open) return;
        const rect = trigger.getBoundingClientRect();
        const width = panel.offsetWidth;
        panel.style.left = `${Math.max(8, Math.min(rect.left, innerWidth - width - 8))}px`;
        panel.style.top = `${Math.max(8, Math.min(rect.bottom + 8, innerHeight - panel.offsetHeight - 8))}px`;
    }
    /** @param {boolean} [returnFocus] */
    function close(returnFocus = false) {
        if (!panel || !trigger) return;
        if (nativePopover && panel.matches(':popover-open')) panel.hidePopover();
        if (!nativePopover) panel.hidden = true;
        open = false; trigger.setAttribute('aria-expanded', 'false');
        if (returnFocus) trigger.focus({ preventScroll: true });
    }
    /** @param {boolean} [last] */
    function show(last = false) {
        if (!panel || !trigger) return;
        refresh();
        if (nativePopover) panel.showPopover(); else panel.hidden = false;
        open = true; trigger.setAttribute('aria-expanded', 'true'); position();
        const buttons = [...panel.querySelectorAll('button:not(:disabled)')];
        const focus = last ? buttons.at(-1) : panel.querySelector('[data-simulation-mode][aria-pressed="true"]') || buttons[0];
        if (focus instanceof HTMLElement) focus.focus({ preventScroll: true });
    }
    refresh();
    scope.on(trigger, 'click', () => { if (open) close(); else show(); });
    scope.on(trigger, 'keydown', (/** @type {KeyboardEvent} */ event) => {
        if (event.key !== 'ArrowDown' && event.key !== 'ArrowUp') return;
        event.preventDefault(); event.stopPropagation(); show(event.key === 'ArrowUp');
    });
    scope.on(panel, 'keydown', (/** @type {KeyboardEvent} */ event) => {
        if (event.key === 'Escape') { event.preventDefault(); event.stopPropagation(); close(true); return; }
        if (!['ArrowDown', 'ArrowUp', 'Home', 'End'].includes(event.key) || !panel) return;
        event.preventDefault(); event.stopPropagation();
        const buttons = [...panel.querySelectorAll('button:not(:disabled)')];
        const index = buttons.indexOf(/** @type {Element} */ (document.activeElement));
        const next = event.key === 'Home' ? 0 : event.key === 'End' ? buttons.length - 1 : (index + (event.key === 'ArrowDown' ? 1 : -1) + buttons.length) % buttons.length;
        if (buttons[next] instanceof HTMLElement) /** @type {HTMLElement} */ (buttons[next]).focus();
    });
    scope.on(panel, 'click', (/** @type {MouseEvent} */ event) => {
        const button = event.target instanceof Element ? event.target.closest('button') : null;
        if (!(button instanceof HTMLButtonElement) || button.disabled || !panel?.contains(button)) return;
        const mode = button.dataset.simulationMode;
        close(true);
        if (mode && mode !== select.value) {
            select.value = mode;
            select.dispatchEvent(new Event('change', { bubbles: true }));
        }
        // Capture closes before the workspace's own target handler disables
        // its launch button and makes the dashboard inert during startup.
    }, true);
    scope.on(panel, 'toggle', () => {
        if (!panel || !nativePopover || !trigger) return;
        open = panel.matches(':popover-open'); trigger.setAttribute('aria-expanded', String(open));
    });
    scope.on(panel, 'focusout', (/** @type {FocusEvent} */ event) => {
        if (event.relatedTarget instanceof Element && !panel?.contains(event.relatedTarget) && event.relatedTarget !== trigger) close();
    });
    scope.on(document, 'pointerdown', (/** @type {PointerEvent} */ event) => {
        if (!nativePopover && open && event.target instanceof Node && !panel?.contains(event.target) && !trigger?.contains(event.target)) close();
    });
    scope.on(select, 'change', refresh);
    scope.on(window, 'resize', position);
    scope.on(window, 'scroll', position, true);
    const options = new MutationObserver(refresh);
    options.observe(select, { childList: true, subtree: true, characterData: true, attributes: true });
    scope.defer(() => options.disconnect());
    scope.defer(() => {
        close(); select.hidden = previous.selectHidden; trigger.hidden = previous.triggerHidden;
        if (previous.labelFor !== null && previous.labelFor !== undefined) label?.setAttribute('for', previous.labelFor);
    });
    return () => scope.dispose();
}

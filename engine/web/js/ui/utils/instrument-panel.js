import { LifetimeScope } from './lifetime-scope.js';

const activePanels = new Map();
const collapseBindings = new WeakMap();

/**
 * Add the common, lifecycle-owned collapse chrome to an instrument panel.
 * Repeated calls for the same live element return the existing binding.
 */
export function attachInstrumentPanelCollapse({
    element,
    lifetime,
    label = 'Instrument panel',
    collapsed = false,
} = {}) {
    if (!element) throw new Error('attachInstrumentPanelCollapse requires an element');
    if (!lifetime?.on || !lifetime?.defer) {
        throw new Error('attachInstrumentPanelCollapse requires a LifetimeScope');
    }
    const existing = collapseBindings.get(element);
    if (existing && !existing.disposed) return existing;

    const chrome = document.createElement('div');
    chrome.className = 'instrument-panel-chrome';

    const title = document.createElement('span');
    title.className = 'instrument-panel-chrome-title';
    title.textContent = label;

    const button = document.createElement('button');
    button.type = 'button';
    button.className = 'instrument-panel-collapse';
    const icon = document.createElement('span');
    icon.className = 'instrument-panel-collapse-icon';
    icon.setAttribute('aria-hidden', 'true');
    button.appendChild(icon);
    chrome.append(title, button);
    element.prepend(chrome);

    let isCollapsed = false;
    let disposed = false;
    const apply = (next) => {
        if (disposed) return;
        isCollapsed = !!next;
        element.classList.toggle('instrument-panel-collapsed', isCollapsed);
        element.dataset.instrumentPanelCollapsed = String(isCollapsed);
        button.setAttribute('aria-expanded', String(!isCollapsed));
        button.setAttribute('aria-label', `${isCollapsed ? 'Expand' : 'Collapse'} ${label}`);
        button.title = `${isCollapsed ? 'Expand' : 'Collapse'} ${label}`;
        icon.textContent = isCollapsed ? '\u25B8' : '\u25BE';
    };

    const binding = {
        chrome,
        button,
        get collapsed() { return isCollapsed; },
        get disposed() { return disposed; },
        setCollapsed: apply,
    };
    collapseBindings.set(element, binding);
    lifetime.on(button, 'click', () => apply(!isCollapsed));
    lifetime.defer(() => {
        if (disposed) return;
        disposed = true;
        chrome.remove();
        element.classList.remove('instrument-panel-collapsed');
        delete element.dataset.instrumentPanelCollapsed;
        if (collapseBindings.get(element) === binding) collapseBindings.delete(element);
    });
    apply(collapsed);
    return binding;
}

/** A small lifecycle owner for instrument panels mounted outside the dock. */
export function mountInstrumentPanel({
    id,
    host = null,
    className = '',
    label = 'Instrument panel',
    build = null,
    collapsible = false,
    collapsed = false,
} = {}) {
    if (!id) throw new Error('mountInstrumentPanel requires an id');
    const target = host || document.getElementById('viewport') || document.body;
    activePanels.get(id)?.dispose();
    document.getElementById(id)?.remove();

    const element = document.createElement('section');
    element.id = id;
    element.className = ['instrument-panel', className].filter(Boolean).join(' ');
    element.dataset.instrumentPanel = 'true';
    element.setAttribute('role', 'region');
    element.setAttribute('aria-label', label);
    const lifetime = new LifetimeScope();
    lifetime.defer(() => element.remove());
    let owner = null;
    owner = {
        element,
        lifetime,
        dispose: () => {
            try {
                lifetime.dispose();
            } finally {
                if (activePanels.get(id) === owner) activePanels.delete(id);
            }
        },
    };
    try {
        target.appendChild(element);
        build?.(element);
        if (collapsible) {
            owner.collapse = attachInstrumentPanelCollapse({
                element,
                lifetime,
                label,
                collapsed,
            });
        }
        activePanels.set(id, owner);
        return owner;
    } catch (error) {
        owner.dispose();
        throw error;
    }
}

import { createMountRegistry } from './mount-registry.js';

const REGION_MAP = Object.freeze({
    toolbar: 'toolbar',
    viewport: 'viewport',
    tabs: 'tab-bar',
    panels: 'panel-area',
    status: 'status-bar',
    settings: 'settings-modal',
    toasts: 'toast-container',
});

const MOUNT_DEFS = Object.freeze([
    { name: 'toolbar', id: 'shell-toolbar-mount' },
    { name: 'viewport-overlays', id: 'shell-viewport-overlay-mount' },
    { name: 'panels', id: 'shell-panel-mount' },
    { name: 'modals', id: 'shell-modal-mount' },
    { name: 'toasts', id: 'shell-toast-mount' },
]);

function _ensureMountHost(app) {
    let host = app.querySelector('#app-shell-mounts');
    if (!host) {
        host = document.createElement('div');
        host.id = 'app-shell-mounts';
        host.setAttribute('aria-hidden', 'true');
        app.appendChild(host);
    }
    return host;
}

function _ensureMount(host, def) {
    let mount = host.querySelector(`#${def.id}`);
    if (!mount) {
        mount = document.createElement('div');
        mount.id = def.id;
        host.appendChild(mount);
    }
    mount.dataset.shellMount = def.name;
    return mount;
}

/**
 * Phase 0 template pass: annotate the current DOM with shell regions and
 * create future mount roots without reparenting the existing markup yet.
 */
export function ensureShellTemplate(app) {
    if (!app) throw new Error('AppShell requires #app root');

    const registry = createMountRegistry();
    app.dataset.shellRoot = 'app';

    Object.entries(REGION_MAP).forEach(([name, id]) => {
        const element = document.getElementById(id);
        if (!element) return;
        element.dataset.shellRegion = name;
        registry.registerRegion(name, element);
    });

    const host = _ensureMountHost(app);
    host.dataset.shellRegion = 'mount-host';
    registry.registerRegion('mount-host', host);

    MOUNT_DEFS.forEach((def) => {
        registry.registerMount(def.name, _ensureMount(host, def));
    });

    return registry;
}

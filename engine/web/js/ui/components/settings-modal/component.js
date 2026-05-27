import { getSettingsModalTemplate } from './template.js';

/**
 * Settings Modal Component
 * Mounts the settings modal into a container. Event wiring is owned by
 * the settings block in app.js (keeps scale/theme side-effects close
 * to the viewport singleton it needs access to).
 */
export function mountSettingsModal(container) {
    if (!container) return null;
    if (document.getElementById('settings-modal')) return document.getElementById('settings-modal');
    const host = document.createElement('div');
    host.dataset.component = 'settings-modal';
    host.innerHTML = getSettingsModalTemplate();
    // Append the actual modal (the host's single child) rather than the wrapper
    const modal = host.firstElementChild;
    container.appendChild(modal);
    return modal;
}

export function initSettingsModal() {
    const container = document.getElementById('app') || document.body;
    return mountSettingsModal(container);
}

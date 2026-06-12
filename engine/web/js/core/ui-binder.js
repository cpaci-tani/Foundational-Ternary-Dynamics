/**
 * @file ui-binder.js
 * @brief DOM Event Binder and Mediator for FTD web dashboard.
 *
 * Provides a structured way to bind and track event listeners, helping
 * to decouple DOM wiring from app.js and prevent memory leaks.
 */

import { appRegistry } from './registry.js';

export class UIBinder {
    constructor() {
        this._listeners = [];
    }

    /**
     * Helper to add an event listener and track it for cleanup.
     * @param {Element|Document} target
     * @param {string} type
     * @param {EventListener} listener
     */
    listen(target, type, listener) {
        if (!target) return;
        target.addEventListener(type, listener);
        this._listeners.push({ target, type, listener });
    }

    /**
     * Bind global playback buttons.
     * @param {object} handlers
     * @param {function} [handlers.onTogglePlay]
     * @param {function} [handlers.onStep]
     * @param {function} [handlers.onReset]
     */
    bindPlayback(handlers) {
        const btnPlay = document.getElementById('btn-play');
        const btnStep = document.getElementById('btn-step');
        const btnReset = document.getElementById('btn-reset');

        if (btnPlay) {
            this.listen(btnPlay, 'click', () => {
                handlers.onTogglePlay?.();
            });
        }
        if (btnStep) {
            this.listen(btnStep, 'click', () => {
                handlers.onStep?.();
            });
        }
        if (btnReset) {
            this.listen(btnReset, 'click', () => {
                handlers.onReset?.();
            });
        }
    }

    /**
     * Bind settings and UI scale/theme controls.
     * @param {function} applyTheme
     * @param {function} applyScale
     */
    bindSettings(applyTheme, applyScale) {
        const modal = document.getElementById('settings-ui-modal');
        const btnOpen = document.getElementById('settings-ui-toggle');
        const btnClose = document.getElementById('settings-ui-close');
        const slider = document.getElementById('settings-ui-scale');
        
        if (btnOpen && modal) {
            this.listen(btnOpen, 'click', () => modal.classList.add('visible'));
        }
        if (btnClose && modal) {
            this.listen(btnClose, 'click', () => modal.classList.remove('visible'));
        }
        if (slider) {
            this.listen(slider, 'input', () => applyScale(parseFloat(slider.value)));
        }

        const themeSwitches = document.querySelectorAll('.theme-switch');
        for (const sw of themeSwitches) {
            this.listen(sw, 'click', () => applyTheme(sw.dataset.theme));
        }
    }

    /**
     * Unbind all event listeners to avoid memory leaks.
     */
    unbind() {
        for (const { target, type, listener } of this._listeners) {
            if (target) {
                target.removeEventListener(type, listener);
            }
        }
        this._listeners = [];
    }
}

export const uiBinder = new UIBinder();
appRegistry.register('uiBinder', uiBinder);

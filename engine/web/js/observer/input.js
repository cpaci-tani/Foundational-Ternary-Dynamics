// @ts-check
import { LifetimeScope } from '../ui/utils/lifetime-scope.js';
import { isEditableTarget, registerWorkspaceShortcuts } from '../ui/shortcuts.js';
import { cameraBasis } from './math.js';

/** @typedef {{forward:string,backward:string,left:string,right:string,up:string,down:string,boost:string,inspect:string,world:string,reticle:string,overlay:string}} Bindings */
/** @typedef {{sensitivity:number,invertY:boolean,speed:number,acceleration:number,grounded:boolean,worldUp:boolean,roll:number,axisLocks:boolean[],bindings:Bindings,scrollZoomSpeed?:number}} InputSettings */

/** First-person input has one lifetime and never drives a simulation itself. */
export class ObserverInput {
    /** @param {{canvas:HTMLCanvasElement,getSettings:()=>InputSettings,onAction:(action:string)=>void,onLook?:()=>void,onStatus?:(message:string)=>void,onForceStart?:(mode:'pull'|'push')=>void,onForceEnd?:()=>void,onForceWheel?:(pixels:number)=>boolean}} options */
    constructor({ canvas, getSettings, onAction, onLook = () => {}, onStatus = () => {}, onForceStart = () => {}, onForceEnd = () => {}, onForceWheel = () => false }) {
        this.canvas = canvas;
        this.getSettings = getSettings;
        this.onAction = onAction;
        this.onLook = onLook;
        this.onStatus = onStatus;
        this.onForceEnd = onForceEnd;
        this.onForceWheel = onForceWheel;
        /** @type {number|null} */ this.forceButton = null;
        this.scope = new LifetimeScope();
        this.scope.defer(registerWorkspaceShortcuts('observer', () => Object.entries(getSettings().bindings).map(([action, code]) => ({ action, code }))));
        /** @type {Set<string>} */ this.keys = new Set();
        this.enabled = false;
        this.panelOpen = false;
        this.yaw = 0;
        this.pitch = 0;
        this.roll = 0;
        this.dragLook = false;
        this.pendingZoom = [0, 0, 0];
        this.suppressGestureClick = false;
        this.scope.on(canvas, 'click', (/** @type {MouseEvent} */ event) => {
            if (this.suppressGestureClick) { this.suppressGestureClick = false; return; }
            if (!this.enabled || this.panelOpen || event.button !== 0) return;
            // The entry click only captures; held buttons operate after capture.
            if (document.pointerLockElement !== canvas) void this.capture();
        });
        this.scope.on(canvas, 'mousedown', (/** @type {MouseEvent} */ event) => {
            // A fresh unlocked click is a new capture intent even if the prior
            // gesture ended outside the canvas and never generated its click.
            if (event.button === 0) this.suppressGestureClick = document.pointerLockElement === canvas;
            if (!this.enabled || this.panelOpen || document.pointerLockElement !== canvas || this.forceButton !== null || ![0, 2].includes(event.button)) return;
            event.preventDefault();
            this.forceButton = event.button;
            this.clearZoom();
            onForceStart(event.button === 0 ? 'pull' : 'push');
        });
        this.scope.on(document, 'mouseup', (/** @type {MouseEvent} */ event) => {
            if (event.button === this.forceButton) this.releaseForce();
        });
        this.scope.on(canvas, 'contextmenu', (/** @type {MouseEvent} */ event) => {
            if (this.enabled && document.pointerLockElement === canvas) event.preventDefault();
        });
        this.scope.on(canvas, 'wheel', (/** @type {WheelEvent} */ event) => this.wheel(event), { passive: false });
        this.scope.on(document, 'keydown', (/** @type {KeyboardEvent} */ event) => this.keyDown(event), true);
        this.scope.on(document, 'keyup', (/** @type {KeyboardEvent} */ event) => this.keys.delete(event.code), true);
        this.scope.on(document, 'mousemove', (/** @type {MouseEvent} */ event) => {
            if (!this.enabled || this.panelOpen || document.pointerLockElement !== canvas) return;
            const settings = this.getSettings();
            this.yaw -= event.movementX * settings.sensitivity;
            this.pitch = Math.max(-Math.PI / 2 + 0.01, Math.min(Math.PI / 2 - 0.01,
                this.pitch - event.movementY * settings.sensitivity * (settings.invertY ? -1 : 1)));
            this.onLook();
        });
        this.scope.on(document, 'pointerlockchange', () => {
            if (document.pointerLockElement !== canvas) this.clear();
            this.canvas.dataset.captured = String(document.pointerLockElement === canvas);
        });
        this.scope.on(document, 'pointerlockerror', () => this.onStatus('Mouse capture was denied. Click the scene to try again, or use the camera controls.'));
        this.scope.on(window, 'blur', () => this.release());
        this.scope.on(document, 'visibilitychange', () => { if (document.hidden) this.release(); });
        canvas.tabIndex = 0;
        canvas.setAttribute('aria-label', 'Mind’s Eye first-person scene. Click to capture the mouse. Hold left mouse to pull or right mouse to push. Press E to inspect. Escape releases it.');
    }

    /** Dolly along the view, independently of grounded walking and FoV. Trackpad
     * deltas stay fractional; there is no accumulated distance or target clamp.
     * @param {WheelEvent} event
     */
    wheel(event) {
        if (!this.enabled || this.panelOpen || document.hidden || event.ctrlKey || event.metaKey || !Number.isFinite(event.deltaY)) return;
        const unit = event.deltaMode === 1 ? 16 : event.deltaMode === 2 ? (this.canvas.clientHeight || 800) : 1;
        if (this.onForceWheel(event.deltaY * unit)) {
            event.preventDefault(); event.stopPropagation(); return;
        }
        const distance = -event.deltaY * unit * (this.getSettings().scrollZoomSpeed ?? 1) / 100;
        if (!Number.isFinite(distance) || distance === 0) return;
        const forward = cameraBasis(this.yaw, this.pitch).forward;
        const next = this.pendingZoom.map((v, i) => v + forward[i] * distance);
        if (!next.every(Number.isFinite)) return;
        event.preventDefault();
        event.stopPropagation();
        this.pendingZoom = next;
    }

    consumeZoom() {
        const displacement = this.pendingZoom;
        this.pendingZoom = [0, 0, 0];
        return displacement;
    }
    clearZoom() { this.pendingZoom = [0, 0, 0]; }

    /** @param {KeyboardEvent} event */
    keyDown(event) {
        if (!this.enabled || isEditableTarget(event.target) || event.metaKey || event.altKey) return;
        // Ctrl is the flight-descend binding, so Ctrl+W must still be movement
        // while the scene owns keyboard focus. Editable controls returned above.
        if (this.panelOpen && event.ctrlKey && event.code !== 'ControlLeft' && event.code !== 'ControlRight') return;
        if (event.code === 'Escape') {
            this.release();
            this.onAction('escape');
            event.stopImmediatePropagation();
            return;
        }
        const settings = this.getSettings();
        const bindings = settings.bindings;
        const action = Object.keys(bindings).find(key => bindings[/** @type {keyof Bindings} */ (key)] === event.code);
        if (!action) {
            // No dashboard shortcut may reach the preserved, hidden world.
            if (!this.panelOpen) event.stopImmediatePropagation();
            return;
        }
        if (this.panelOpen && !['inspect', 'world'].includes(action)) return;
        if (event.target instanceof Element && event.target.closest('button,select,a,input,textarea')) return;
        event.preventDefault();
        event.stopImmediatePropagation();
        if (['inspect', 'world', 'reticle', 'overlay'].includes(action)) {
            if (!event.repeat) this.onAction(action);
        } else if (!this.panelOpen) this.keys.add(event.code);
    }

    async capture() {
        if (!this.enabled || this.panelOpen || document.pointerLockElement === this.canvas) return;
        try {
            await this.canvas.requestPointerLock({ unadjustedMovement: true });
        } catch (error) {
            try { await this.canvas.requestPointerLock(); }
            catch { this.onStatus(error instanceof Error ? error.message : 'Mouse capture unavailable.'); }
        }
        this.canvas.focus({ preventScroll: true });
    }

    /** @param {{yaw?:number,pitch?:number,roll?:number}} pose */
    setPose(pose) {
        this.yaw = pose.yaw ?? 0;
        this.pitch = pose.pitch ?? 0;
        this.roll = pose.roll ?? 0;
    }

    sample() {
        const s = this.getSettings();
        const b = s.bindings;
        const pressed = (/** @type {string} */ code) => this.enabled && !this.panelOpen && this.keys.has(code) ? 1 : 0;
        const move = [pressed(b.right) - pressed(b.left), pressed(b.up) - pressed(b.down), pressed(b.forward) - pressed(b.backward)];
        const length = Math.hypot(...move);
        if (length > 1) for (let i = 0; i < 3; i++) move[i] /= length;
        return { move, yaw: this.yaw, pitch: this.pitch, roll: s.worldUp ? 0 : s.roll,
            speed: Math.min(0.99, s.speed * (pressed(b.boost) ? 3 : 1)), acceleration: s.acceleration,
            grounded: s.grounded, worldUp: s.worldUp, axisLocks: [...s.axisLocks] };
    }

    releaseForce() { this.forceButton = null; this.onForceEnd(); }
    clear() { this.keys.clear(); this.clearZoom(); this.releaseForce(); }
    release() {
        this.clear();
        if (document.pointerLockElement === this.canvas) document.exitPointerLock();
    }
    dispose() { this.enabled = false; this.release(); this.scope.dispose(); }
}

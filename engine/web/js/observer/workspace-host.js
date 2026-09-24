// @ts-check
import { LifetimeScope } from '../ui/utils/lifetime-scope.js';
import { appRegistry } from '../core/registry.js';
import { setShortcutWorkspace } from '../ui/shortcuts.js';

/**
 * Presentation ownership is independent from engineMode. The native owner is
 * retained, never cloned, reseeded or disposed on entering the Observer.
 */
export class ObserverWorkspaceHost {
    /** @param {{app:HTMLElement,button:HTMLButtonElement,viewport:object,getRunning:()=>boolean,setRunning:(value:boolean)=>void,readLattice:()=>any,getMode:()=>string,suspendDashboard:()=>Promise<()=>void>,getReturnFocus?:()=>HTMLElement|null}} deps */
    constructor(deps) {
        this.deps = deps;
        this.scope = new LifetimeScope();
        /** @type {import('./workspace.js').ObserverWorkspace|null} */ this.workspace = null;
        /** @type {Promise<void>|null} */ this.pending = null;
        this.active = false;
        this.suspended = false;
        this.generation = 0;
        /** @type {(()=>void)|null} */ this.resumeDashboard = null;
        /** @type {Promise<void>|null} */ this.exiting = null;
        this.disposed = false;
        this.previousRunning = false;
        this.previousControls = true;
        /** @type {Element|null} */ this.previousFocus = null;
        /** @type {Map<HTMLElement,{inert:boolean,visibility:string}>} */ this.obscured = new Map();
        this.scope.on(deps.button, 'click', () => { void this.enter().catch(error => {
            deps.button.title = error instanceof Error ? error.message : 'Observer failed to initialize';
            console.error('[Observer]', error);
        }); });
        const portals = new MutationObserver(() => { if (this.active) this.isolatePresentation(); });
        portals.observe(document.body, { childList: true });
        this.scope.defer(() => portals.disconnect());
    }
    get liveLink() { return false; }
    async enter() {
        if (this.disposed || this.active) return;
        if (this.exiting) await this.exiting;
        if (this.disposed || this.active) return;
        if (this.pending) return this.pending;
        this.pending = this.start();
        try { await this.pending; } finally { this.pending = null; }
    }
    async start() {
        const deps = this.deps;
        const generation = ++this.generation;
        deps.button.disabled = true;
        this.previousRunning = deps.getRunning();
        this.previousFocus = deps.getReturnFocus?.() ?? document.activeElement;
        this.suspended = true;
        deps.app.inert = true;
        deps.setRunning(false);
        const viewport = /** @type {{controls?:{enabled:boolean},setPresentationSuspended?:(value:boolean)=>void}} */ (deps.viewport);
        this.previousControls = viewport.controls?.enabled ?? true;
        if (viewport.controls) viewport.controls.enabled = false;
        viewport.setPresentationSuspended?.(true);
        try {
            const resume = await deps.suspendDashboard();
            if (this.disposed || generation !== this.generation) { resume(); return; }
            this.resumeDashboard = resume;
            if (!this.workspace) {
                const { ObserverWorkspace } = await import('./workspace.js');
                if (this.disposed || generation !== this.generation) return;
                this.workspace = new ObserverWorkspace({
                    onExit: () => { void this.exit(); }, readLattice: deps.readLattice,
                });
                await this.workspace.initialize();
                appRegistry.register('observerWorkspace', this.workspace);
            }
            if (this.disposed || generation !== this.generation) {
                this.workspace?.dispose();
                this.workspace = null;
                appRegistry.unregister('observerWorkspace');
                return;
            }
            this.active = true;
            setShortcutWorkspace('observer');
            deps.app.dataset.workspace = 'observer';
            deps.app.inert = true;
            this.isolatePresentation();
            deps.button.setAttribute('aria-pressed', 'true');
            await this.workspace.enter();
            document.dispatchEvent(new CustomEvent('ftd:workspace-change', { bubbles: true, detail: { workspace: 'observer' } }));
        } catch (error) {
            this.restoreDashboard();
            this.workspace?.dispose();
            this.workspace = null;
            appRegistry.unregister('observerWorkspace');
            throw error;
        } finally {
            if ((generation !== this.generation || this.disposed) && !this.exiting) this.restoreDashboard();
            deps.button.disabled = false;
        }
    }
    exit() {
        if (this.exiting) return this.exiting;
        ++this.generation;
        if (!this.suspended) return Promise.resolve();
        if (!this.active && this.pending) {
            this.exiting = this.pending.catch(() => {}).then(() => this.restoreDashboard())
                .finally(() => { this.exiting = null; });
            return this.exiting;
        }
        this.active = false;
        document.dispatchEvent(new CustomEvent('ftd:workspace-change', { bubbles: true, detail: { workspace: 'dashboard' } }));
        this.exiting = Promise.resolve(this.workspace?.suspend())
            .then(() => {})
            .finally(() => { this.restoreDashboard(); this.exiting = null; });
        return this.exiting;
    }
    restoreDashboard() {
        if (!this.suspended) return;
        this.active = false;
        this.suspended = false;
        document.dispatchEvent(new CustomEvent('ftd:workspace-change', { bubbles: true, detail: { workspace: 'dashboard' } }));
        this.resumeDashboard?.();
        this.resumeDashboard = null;
        setShortcutWorkspace('dashboard');
        const { app, button, viewport, setRunning } = this.deps;
        app.inert = false;
        for (const [element, previous] of this.obscured) {
            element.inert = previous.inert;
            element.style.visibility = previous.visibility;
        }
        this.obscured.clear();
        delete app.dataset.workspace;
        button.setAttribute('aria-pressed', 'false');
        const view = /** @type {{controls?:{enabled:boolean},setPresentationSuspended?:(value:boolean)=>void,resize?:()=>void}} */ (viewport);
        view.setPresentationSuspended?.(false);
        if (view.controls) view.controls.enabled = this.previousControls;
        view.resize?.();
        setRunning(this.previousRunning);
        if (this.previousFocus instanceof HTMLElement && this.previousFocus.isConnected) this.previousFocus.focus({ preventScroll: true });
        else button.focus({ preventScroll: true });
    }
    /** @param {number} now */
    frame(now) { if (this.active) this.workspace?.render(now); }
    isolatePresentation() {
        // Some native-service panels are body portals outside AppShell.
        for (const element of document.body.children) {
            if (!(element instanceof HTMLElement) || element === this.workspace?.element || element === this.deps.app
                || ['SCRIPT', 'STYLE', 'LINK'].includes(element.tagName) || this.obscured.has(element)) continue;
            this.obscured.set(element, { inert: element.inert, visibility: element.style.visibility });
            element.inert = true;
            element.style.visibility = 'hidden';
        }
    }
    dispose() {
        if (this.disposed) return;
        this.disposed = true;
        ++this.generation;
        this.workspace?.dispose();
        // An in-flight suspension must return its resume handle before the
        // previous playback request can be restored to that same owner.
        if (this.pending) void this.pending.catch(() => {}).then(() => this.restoreDashboard());
        else this.restoreDashboard();
        this.scope.dispose();
        appRegistry.unregister('observerWorkspace');
    }
}

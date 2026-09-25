// @ts-check
import { LifetimeScope } from '../ui/utils/lifetime-scope.js';
import { ObserverWorkerClient } from './worker-client.js';
import { ObserverRenderer } from './renderer.js';
import { ObserverInput } from './input.js';
import { ObserverGizmo } from './gizmo.js';
import { ObserverSpatialLabels } from './spatial-labels.js';
import { LatticeObservationAdapter } from './lattice-adapter.js';
import { createObserverUI } from './ui.js';
import { DEFAULT_SETTINGS, DEFAULT_BINDINGS } from './catalog.js';
import { boundedFractalDetail } from './fractal-presets.js';
import { ObserverStorage } from './storage.js';
import { reflectAuthorPatch, reflectPolar } from './mirror-frame.js';
import { ObserverForceGun, ObserverTetherOverlay } from './force-gun.js';

/** @typedef {import('./types.js').WorldSnapshot} WorldSnapshot */
/** @typedef {import('./types.js').WorldCommand} WorldCommand */

/** One isolated presentation/session, driven exclusively by the app frame loop. */
export class ObserverWorkspace {
    /** @param {{onExit:()=>void,readLattice:()=>any}} deps */
    constructor(deps) {
        this.deps = deps;
        this.scope = new LifetimeScope();
        this.settings = structuredClone(DEFAULT_SETTINGS);
        /** @type {WorldSnapshot|null} */ this.snapshot = null;
        /** @type {string|null} */ this.selectedId = null;
        /** @type {any} */ this.hit = null;
        /** @type {WorldSnapshot|null} */ this.hitSnapshot = null;
        this.hitViewKey = '';
        /** @type {import('./optics.js').OpticalHit|null} */ this.selectedHit = null;
        /** @type {any} */ this.preview = null;
        this.active = false;
        this.disposed = false;
        this.panelOpen = false;
        this.assistantOpen = false;
        this.assistantViewVersion = 0;
        this.assistantSelectionKey = '';
        /** @type {Set<AbortController>} */ this.assistantCommands = new Set();
        this.lastFrame = 0;
        this.lastUI = 0;
        this.lastAutosave = 0;
        this.status = '';
        /** @type {number[]} */ this.frameIntervals = [];
        /** @type {any} */ this.storageState = { entries: [], usage: { bytes: 0, capBytes: 100 * 1024 * 1024 }, autosave: false };
        this.busy = false;
        this.contextLost = false;
        this.visibilityPlaying = false;
        this.panelPlaying = false;
        this.resumePlaying = true;
        this.lastEpoch = -1;
        this.lastEnvironmentMessage = '';
        this.zoomPending = false;
        this.element = document.createElement('section');
        this.element.className = 'observer-workspace';
        this.element.id = 'observer-workspace';
        this.element.hidden = true;
        this.element.setAttribute('aria-label', 'Mind’s Eye Observer workspace');
        this.stage = document.createElement('div');
        this.stage.className = 'observer-stage';
        this.element.append(this.stage);
        this.reticle = document.createElement('div');
        this.reticle.className = 'observer-reticle';
        this.reticle.setAttribute('aria-hidden', 'true');
        this.element.append(this.reticle);
        document.body.append(this.element);
        this.storage = new ObserverStorage();
        this.lattice = new LatticeObservationAdapter(deps.readLattice);
        this.client = new ObserverWorkerClient({
            onSnapshot: snapshot => this.receive(snapshot),
            onError: error => this.setStatus(error instanceof Error ? error.message : String(error)),
        });
        this.renderer = new ObserverRenderer({ container: this.stage });
        this.labels = new ObserverSpatialLabels(this.stage);
        /** @type {ObserverForceGun} */ this.forceGun = new ObserverForceGun({
            getSnapshot: () => this.snapshot, getSettings: () => this.renderSettings,
            getTelemetry: () => this.client.latestForceGun,
            command: command => this.command(command), onStatus: message => this.setStatus(message),
        });
        this.tether = new ObserverTetherOverlay(this.stage);
        this.ui = createObserverUI({ host: this.element,
            onCommand: command => this.authorCommand(command),
            onSetting: (key, value) => this.setSetting(key, value),
            onAction: (action, payload) => { void this.action(action, payload).catch(error => this.setStatus(String(error))); },
        });
        /** @type {ObserverInput} */ this.input = new ObserverInput({ canvas: this.renderer.canvas,
            getSettings: () => /** @type {any} */ (this.settings),
            onAction: action => { void this.shortcut(action); },
            onStatus: message => this.setStatus(message),
            onForceStart: mode => { void this.forceGun.begin(this.hit, mode); },
            onForceEnd: () => { this.forceGun.end(); this.tether.hide(); },
            onForceWheel: pixels => this.forceGun.wheel(pixels),
        });
        this.gizmo = new ObserverGizmo({ host: this.element,
            getEntity: () => this.authorEntity,
            onPreview: entity => { this.preview = entity ? reflectAuthorPatch(entity, this.authorMirrored) : null; },
            onCommit: (id, patch) => { void this.authorCommand({ type: 'update', id, patch }); },
            getSnap: () => Number(this.settings.gridSnap ?? 0),
        });
        const resize = new ResizeObserver(() => this.resize());
        resize.observe(this.stage);
        this.scope.defer(() => resize.disconnect());
        this.scope.on(document, 'visibilitychange', () => {
            if (!this.active) return;
            this.lastFrame = 0;
            if (document.hidden) {
                this.visibilityPlaying = !!this.snapshot?.playing;
                void this.command({ type: 'pause' });
            } else if (this.visibilityPlaying && !this.panelOpen && !this.contextLost) {
                this.visibilityPlaying = false;
                void this.command({ type: 'play' });
            }
        });
        this.scope.on(this.renderer.canvas, 'webglcontextlost', (/** @type {Event} */ event) => {
            event.preventDefault();
            this.contextLost = true;
            this.input.release();
            void this.command({ type: 'pause' });
            this.setStatus('Graphics context lost. Session paused; state retained.');
        });
        this.scope.on(this.renderer.canvas, 'webglcontextrestored', () => {
            this.contextLost = false;
            this.lastFrame = 0;
            this.setStatus('Graphics restored. Resume when ready.');
        });
    }

    async initialize() { this.receive(await this.client.ready); }
    getAssistantMount() { return this.element; }
    cancelAssistantCommands() { for (const request of this.assistantCommands) request.abort(); }
    /** Release gestures without changing playback or acquiring a new frame loop.
     * @param {boolean} [open]
     */
    releaseAssistantInput(open = true) {
        this.assistantOpen = open;
        this.input.panelOpen = this.panelOpen || open;
        if (open) { this.input.release(); this.forceGun.end(); this.tether.hide(); }
    }
    getAssistantPreparationVersion() {
        const selection = JSON.stringify([this.selectedId, this.authorMirrored]);
        if (selection !== this.assistantSelectionKey) { this.assistantSelectionKey = selection; this.assistantViewVersion++; }
        return `${this.snapshot?.epoch ?? 0}:${this.snapshot?.preparationVersion ?? 0}:${this.assistantViewVersion}`;
    }
    /** @param {WorldSnapshot} snapshot */
    receive(snapshot) {
        if (this.disposed) return;
        this.snapshot = snapshot;
        if (snapshot.epoch !== this.lastEpoch) {
            this.input?.releaseForce();
            this.lastEpoch = snapshot.epoch;
            this.input?.setPose(snapshot.observer);
            this.input?.clearZoom();
            this.preview = null;
            this.hit = null;
            this.selectedHit = null;
        }
        if (this.selectedId && !snapshot.entities.some(entity => entity.id === this.selectedId)) { this.selectedId = null; this.selectedHit = null; }
    }
    async enter() {
        this.active = true;
        this.element.hidden = false;
        this.input.enabled = true;
        this.lastFrame = 0;
        this.resize();
        this.settings = { ...this.settings, liveLink: false };
        if (this.resumePlaying && !this.panelOpen && !document.hidden) await this.command({ type: 'play' });
        this.renderer.canvas.focus({ preventScroll: true });
        this.updateUI();
    }
    suspend() {
        this.cancelAssistantCommands();
        this.resumePlaying = !!this.snapshot?.playing;
        this.active = false;
        this.input.enabled = false;
        this.input.release();
        this.element.hidden = true;
        this.lastFrame = 0;
        return this.command({ type: 'pause' });
    }
    resize() {
        const box = this.stage.getBoundingClientRect();
        if (box.width && box.height) this.renderer.resize(box.width, box.height);
    }
    /** @param {number} now */
    render(now) {
        if (!this.active || !this.snapshot || document.hidden || this.contextLost) { this.lastFrame = 0; return; }
        const dt = this.lastFrame ? (now - this.lastFrame) / 1000 : 0;
        this.lastFrame = now;
        if (dt > 0) {
            this.frameIntervals.push(dt * 1000);
            if (this.frameIntervals.length > 2400) this.frameIntervals.shift();
        }
        if (!this.zoomPending) {
            const displacement = this.input.consumeZoom();
            if (displacement.some(value => value !== 0)) {
                this.zoomPending = true;
                void this.command({ type: 'dolly', displacement }).finally(() => { this.zoomPending = false; });
            }
        }
        const forceGun = this.forceGun.sample();
        this.client.advance(dt, { ...this.input.sample(), ...(forceGun ? { forceGun } : {}) });
        const settings = this.renderSettings;
        this.renderer.render(this.snapshot, settings, dt);
        const environmentMessage = this.renderer.diagnostics.environmentMessage;
        if (environmentMessage && environmentMessage !== this.lastEnvironmentMessage) this.setStatus(environmentMessage);
        this.lastEnvironmentMessage = environmentMessage;
        this.hit = this.renderer.hit;
        this.hitSnapshot = this.snapshot; this.hitViewKey = this.observationViewKey;
        this.tether.update(this.forceGun, this.snapshot, settings, this.renderer);
        this.labels.update(this.snapshot, this.renderer.getOverlayAnchors(this.snapshot, settings), this.settings.overlayFields);
        this.reticle.hidden = !this.settings.reticle || this.panelOpen;
        this.reticle.dataset.target = this.hit?.entityId || this.hit?.id || '';
        this.reticle.dataset.selected = String(!!this.selectedId && this.reticle.dataset.target === this.selectedId);
        this.gizmo.show(this.panelOpen && !!this.selectedId && !!this.snapshot.entities.find(entity => entity.id === this.selectedId)?.alive);
        if (now - this.lastUI >= 100) { this.lastUI = now; this.updateUI(); }
        if (now - this.lastAutosave >= 1000) {
            this.lastAutosave = now;
            void this.storage.maybeAutosave(this.snapshot, this.settings).then(saved => {
                if (saved) return this.refreshStorage();
            }).catch(error => {
                this.storageState.autosave = this.storage.autosave;
                this.setStatus(`Autosave stopped. ${String(error)}`);
                this.updateUI();
            });
        }
    }
    updateUI() {
        if (!this.snapshot) return;
        this.ui.update(this.snapshot, { settings: this.settings, cameraOverride: this.renderSettings.cameraOverride, selectedId: this.selectedId,
            authorEntity: this.authorEntity, authorMirrored: this.authorMirrored,
            forceGun: this.forceGun.state,
            hit: this.hitSnapshot === this.snapshot && this.hitViewKey === this.observationViewKey ? this.hit : null,
            selectedHit: this.selectedHit, lattice: this.lattice.snapshot(), rendering: this.renderer.diagnostics, storage: this.storageState, status: this.status,
        });
    }
    /** @param {string} message */
    setStatus(message) { this.status = message; this.ui?.setStatus(message); }

    get renderSettings() {
        return { ...this.settings, selectedId: this.selectedId, authorPreview: this.preview,
            cameraOverride: { yaw: this.input?.yaw ?? 0, pitch: this.input?.pitch ?? 0, roll: this.settings.worldUp ? 0 : this.settings.roll } };
    }
    get observationViewKey() {
        return JSON.stringify([this.renderSettings.cameraOverride, this.settings.fov, this.settings.optical,
            this.settings.mirrorWorld, this.renderer.width, this.renderer.height]);
    }

    get authorMirrored() {
        return this.settings.mirrorWorld === true && this.selectedHit?.mirrored === true
            && (this.selectedHit.entityId || this.selectedHit.id) === this.selectedId;
    }
    get authorEntity() {
        const entity = this.snapshot?.entities.find(item => item.id === this.selectedId);
        return entity ? reflectAuthorPatch(entity, this.authorMirrored) : undefined;
    }
    /** UI actions use the selected image frame; command() remains source-native. */
    /** @param {WorldCommand} command */
    async authorCommand(command, options = {}) {
        const mirrored = this.authorMirrored && command.id === this.selectedId;
        const translated = { ...command };
        if (mirrored && command.type === 'update' && command.patch) translated.patch = reflectAuthorPatch(command.patch);
        if (mirrored && command.type === 'impulse' && Array.isArray(command.impulse)) translated.impulse = reflectPolar(command.impulse);
        const result = await this.command(translated, options);
        if (mirrored && command.type === 'impulse' && result?.ok) {
            const changed = result.snapshot.entities.find(item => item.id === command.id);
            if (changed) this.setStatus(`Impulse applied · mirrored image velocity [${reflectPolar(changed.velocity).map(value => value.toFixed(2)).join(', ')}]. ${this.snapshot?.playing ? 'Simulation running.' : 'Paused: press Play or close controls to resume prior playback.'}`);
        }
        return result;
    }

    /** @param {WorldCommand} command @param {{signal?:AbortSignal,assertActive?:()=>unknown}} [options] */
    async command(command, options = {}) {
        if (this.disposed) return;
        if (!options.assertActive && !command.type.startsWith('gun-')) this.cancelAssistantCommands();
        // Stop the gesture before commands that rebuild, branch or pause its
        // owner. Tokened release also covers a begin still awaiting its reply.
        if (['pause', 'delete', 'update', 'restore', 'reset', 'preset', 'profile', 'undo', 'load', 'scrub', 'physics', 'gravity', 'world-physics'].includes(command.type)) this.input?.releaseForce();
        const request = options.assertActive ? new AbortController() : null;
        const cancel = () => request?.abort();
        if (request) {
            this.assistantCommands.add(request);
            options.signal?.addEventListener('abort', cancel, { once: true });
            if (options.signal?.aborted) request.abort();
        }
        try {
            const world = this.snapshot;
            const entity = command.id ? world?.entities.find(item => item.id === command.id) : null;
            const result = await this.client.request({ ...command,
                expectedEpoch: command.expectedEpoch ?? world?.epoch,
                ...(entity && command.expectedPreparationVersion === undefined && ['update', 'delete', 'duplicate', 'restore', 'impulse', 'gun-begin'].includes(command.type) ? { expectedTargetRevision: command.expectedTargetRevision ?? entity.revision } : {}),
            }, request ? { ...options, signal: request.signal } : options);
            // The client already applied its response-order and epoch checks.
            // A delayed gesture acknowledgement must not restore an older pose.
            if (result?.snapshot && this.client.snapshot === result.snapshot) this.receive(result.snapshot);
            if (result && !result.ok) this.setStatus(result.error || 'Command was rejected.');
            else if (command.type === 'dolly') this.setStatus('Scroll zoom · viewpoint relocated; observer clock restarted.');
            else if (command.type === 'impulse') {
                const changed = result?.snapshot.entities.find(item => item.id === command.id);
                this.setStatus(`Impulse applied · velocity [${changed?.velocity.map(value => value.toFixed(2)).join(', ')}]. ${this.snapshot?.playing ? 'Simulation running.' : 'Paused: press Play or close controls to resume prior playback.'}`);
            }
            else if (!['play', 'pause', 'observer'].includes(command.type) && !command.type.startsWith('gun-')) {
                this.preview = null;
                this.setStatus(result?.snapshot.warnings?.length ? result.snapshot.warnings.join(' ')
                    : command.type === 'update' && this.snapshot?.profile === 'sr'
                    ? 'Applied at the current event. Earlier emitted light is preserved.' : 'Applied.');
            }
            this.updateUI();
            return result;
        } catch (error) {
            this.setStatus(error instanceof Error ? error.message : String(error));
            if (options.signal || options.assertActive) throw error;
            return undefined;
        } finally {
            if (request) this.assistantCommands.delete(request);
            options.signal?.removeEventListener('abort', cancel);
        }
    }
    /** @param {string} key @param {any} value */
    setSetting(key, value) {
        this.cancelAssistantCommands();
        if (key === 'fractalDetail') value = boundedFractalDetail(value);
        if (key === 'mirrorWorld' || key === 'forceGunEnabled' && !value) this.input.releaseForce();
        // Numeric input is bounded in the UI; imported settings are validated.
        this.settings = { ...this.settings, [key]: value, liveLink: false };
        this.assistantViewVersion++;
        if (key === 'roll') this.input.roll = Number(value);
        this.updateUI();
    }
    /** @param {string} action */
    async shortcut(action) {
        if (['inspect', 'overlay'].includes(action)) this.cancelAssistantCommands();
        if (action === 'inspect') {
            if (this.panelOpen) this.ui.closePanel();
            else {
                const id = this.hit?.entityId || this.hit?.id;
                const entity = this.snapshot?.entities.find(item => item.id === id);
                if (!entity) {
                    this.selectedId = null; this.selectedHit = null;
                    this.setStatus('No object under the crosshair. Aim at a shape, then press E.');
                    this.updateUI();
                    return;
                }
                this.selectedId = entity.id;
                this.selectedHit = structuredClone(this.hit);
                this.updateUI();
                this.ui.openPanel('objects');
            }
        } else if (action === 'world') {
            if (this.panelOpen) this.ui.closePanel(); else this.ui.openPanel('world');
        } else if (action === 'reticle') this.setSetting('reticle', !this.settings.reticle);
        else if (action === 'overlay') {
            const id = this.hit?.entityId || this.hit?.id || this.selectedId;
            const entity = this.snapshot?.entities.find(item => item.id === id);
            if (entity) { this.selectedId = id; await this.command({ type: 'update', id, patch: { overlay: !entity.overlay } }); }
        } else if (action === 'escape' && this.panelOpen) this.ui.closePanel();
    }
    /** @param {string} action @param {any} [payload] */
    async action(action, payload) {
        if (action === 'exit') { this.deps.onExit(); return; }
        if (action === 'assistant') {
            document.dispatchEvent(new CustomEvent('ftd:assistant-toggle', { bubbles: true, detail: { workspace: 'observer' } }));
            return;
        }
        if (action === 'panel-open') {
            const wasOpen = this.panelOpen;
            this.panelOpen = true;
            this.input.panelOpen = true;
            this.input.release();
            if (!wasOpen) {
                this.panelPlaying = !!this.snapshot?.playing;
                if (this.settings.pauseOnInspect) await this.command({ type: 'pause' });
            }
        } else if (action === 'panel-close') {
            this.panelOpen = false;
            this.input.panelOpen = this.assistantOpen;
            this.preview = null;
            if (this.panelPlaying && this.settings.pauseOnInspect && this.active) await this.command({ type: 'play' });
            this.renderer.canvas.focus({ preventScroll: true });
        } else if (action === 'select') {
            this.cancelAssistantCommands();
            this.selectedId = payload.id;
            this.selectedHit = null;
        }
        else if (action === 'preview') {
            const entity = this.snapshot?.entities.find(item => item.id === payload?.id);
            this.preview = entity && payload.patch !== null ? { ...entity,
                ...reflectAuthorPatch(payload.patch, this.authorMirrored && payload.id === this.selectedId) } : null;
        } else if (action === 'experiment') {
            await this.command({ type: 'preset', preset: payload.id });
        } else if (action === 'reset-observer') {
            await this.command({ type: 'observer', patch: { position: [0, 1.6, 8], velocity: [0, 0, 0], yaw: 0, pitch: 0, roll: 0 } });
            this.input.setPose({ yaw: 0, pitch: 0, roll: 0 });
        } else if (action === 'reset-bindings') this.setSetting('bindings', structuredClone(DEFAULT_BINDINGS));
        else if (action === 'autosave') {
            await this.storage.setAutosave(!!payload);
            this.storageState.autosave = !!payload;
            if (payload) await this.refreshStorage();
        } else if (action === 'storage-cap') {
            this.storage.setStorageCap(Number(payload) * 1048576);
            this.settings = { ...this.settings, storageCapMiB: Number(payload) };
            await this.refreshStorage();
        } else if (action === 'save' && this.snapshot) {
            await this.storage.save(payload.name || 'Mind’s Eye world', this.snapshot, this.settings);
            await this.refreshStorage();
            this.setStatus('Snapshot saved on this device.');
        } else if (action === 'load') {
            const document = await this.storage.load(payload.id);
            await this.restore(document);
        } else if (action === 'remove-save') { await this.storage.remove(payload.id); await this.refreshStorage(); }
        else if (action === 'clear-saves') { await this.storage.clear(); await this.refreshStorage(); }
        else if (action === 'refresh-saves') await this.refreshStorage();
        else if (action === 'export' && this.snapshot) {
            const text = await this.storage.exportWorld(this.snapshot, this.settings);
            const url = URL.createObjectURL(new Blob([text], { type: 'application/json' }));
            const link = document.createElement('a');
            link.href = url; link.download = 'minds-eye-world.json'; link.click();
            this.scope.timeout(() => URL.revokeObjectURL(url), 1000);
        } else if (action === 'import') await this.restore(await this.storage.importWorld(payload.text));
        this.updateUI();
    }
    /** @param {any} saved */
    async restore(saved) {
        if (!saved?.snapshot) throw new Error('Snapshot data is missing.');
        const result = await this.command({ type: 'load', snapshot: saved.snapshot });
        if (result?.ok) {
            this.settings = { ...structuredClone(DEFAULT_SETTINGS), ...saved.settings, liveLink: false };
            this.storage.setStorageCap((this.settings.storageCapMiB ?? 100) * 1048576);
            this.input.setPose(result.snapshot.observer);
            this.setStatus('Snapshot restored in a new session epoch.');
        }
    }
    async refreshStorage() {
        const [entries, usage] = await Promise.all([this.storage.list(), this.storage.usage()]);
        this.storageState = { ...this.storageState, entries, usage };
        this.updateUI();
    }
    dispose() {
        if (this.disposed) return;
        this.cancelAssistantCommands();
        this.forceGun.dispose(); this.tether.dispose();
        this.disposed = true;
        this.active = false;
        this.input.dispose(); this.gizmo.dispose(); this.labels.dispose(); this.ui.dispose();
        this.client.dispose(); this.renderer.dispose(); this.storage.dispose();
        this.scope.dispose(); this.element.remove();
    }
}

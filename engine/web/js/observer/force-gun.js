// @ts-check
import { observerRay } from './optics.js';
import { rotate, rotationMatrix } from './geometry.js';
import { reflectPolar } from './mirror-frame.js';
import { forceGunEffort } from './force-gun-effort.js';

/** @typedef {import('./types.js').WorldSnapshot} WorldSnapshot */
/** @typedef {import('./types.js').ForceGunTelemetry} ForceGunTelemetry */
/** @typedef {import('./types.js').ForceGunInput} ForceGunInput */
/** @typedef {import('./types.js').WorldCommand} WorldCommand */
/** @typedef {import('./types.js').CommandResult} CommandResult */
/** @typedef {import('./optics.js').OpticalSettings & {forceGunEnabled?:boolean,forceGunSensitivity?:string,forceGunMultiplier?:number}} GunSettings */
/** @typedef {{token:string,id:string,epoch:number,mode:'pull'|'push',mirrored:boolean,localAnchor:number[],depth:number,sequence:number,starting:boolean}} HeldBody */
let tokenSequence = 0;

/** Transient input state only. The worker owns the force, anchor and rigid body. */
export class ObserverForceGun {
    /** @param {{getSnapshot:()=>WorldSnapshot|null,getSettings:()=>GunSettings,getTelemetry:()=>ForceGunTelemetry|null,command:(command:WorldCommand)=>Promise<CommandResult|undefined>,onStatus:(message:string)=>void}} deps */
    constructor(deps) {
        this.deps = deps;
        /** @type {HeldBody|null} */ this.held = null;
        this.disposed = false;
    }

    /** @param {import('./optics.js').OpticalHit|null} hit @param {'pull'|'push'} mode */
    async begin(hit, mode) {
        if (this.disposed || this.held) return;
        const snapshot = this.deps.getSnapshot(), settings = this.deps.getSettings();
        if (settings.forceGunEnabled === false) { this.deps.onStatus('Force gun is disabled. Enable it in Force gun controls.'); return; }
        if (!snapshot || snapshot.profile !== 'playground') {
            this.deps.onStatus('Force gun requires Playground physics. Press E to inspect, or use World controls to change profile.'); return;
        }
        const entity = snapshot.entities.find(item => item.id === hit?.entityId);
        if (!hit || !entity) { this.deps.onStatus('Aim at a dynamic body, then hold left mouse to pull or right mouse to push.'); return; }
        if (hit.historical || !entity.alive || entity.revision !== hit.revision) { this.deps.onStatus('Historical images cannot receive a force. Press E to restore or duplicate this object.'); return; }
        if (entity.bodyType !== 'dynamic') { this.deps.onStatus('Force gun requires a Dynamic body. Press E to change the body behavior.'); return; }
        const held = { token: `observer-gun-${Date.now()}-${++tokenSequence}`, id: entity.id, epoch: snapshot.epoch,
            mode, mirrored: hit.mirrored, localAnchor: [...hit.restPosition], depth: Math.max(0.1, Math.min(1e6, hit.distance)), sequence: 0, starting: true };
        this.held = held;
        const input = this.targetInput(snapshot, settings, held);
        try {
            const result = await this.deps.command({ type: 'gun-begin', ...input, id: held.id, mode,
                localAnchor: held.localAnchor, historical: false, expectedEpoch: held.epoch, expectedTargetRevision: hit.revision });
            if (this.held !== held || this.disposed) {
                // A queued begin may acknowledge after release, blur, or a new grab.
                // Its token can never cancel a newer grab in the worker.
                if (result?.ok) await this.deps.command({ type: 'gun-end', token: held.token });
                return;
            }
            if (!result?.ok) { this.held = null; return; }
            held.starting = false;
            this.deps.onStatus(`${mode === 'pull' ? 'Pulling' : 'Pushing'} ${entity.name} · move your aim to steer; scroll changes hold distance. Release to let go.`);
        } catch (error) {
            if (this.held === held) this.held = null;
            this.deps.onStatus(error instanceof Error ? error.message : String(error));
        }
    }

    /** @param {WorldSnapshot} snapshot @param {GunSettings} settings @param {HeldBody} held @returns {ForceGunInput} */
    targetInput(snapshot, settings, held) {
        const ray = observerRay(snapshot, settings);
        const target = ray.origin.map((value, axis) => value + ray.direction[axis] * held.depth);
        const sensitivity = settings.forceGunSensitivity === 'delicate' || settings.forceGunSensitivity === 'strong' ? settings.forceGunSensitivity : 'normal';
        return { token: held.token, epoch: held.epoch, sequence: ++held.sequence,
            target: held.mirrored ? reflectPolar(target) : target,
            direction: held.mirrored ? reflectPolar(ray.direction) : ray.direction,
            sensitivity, multiplier: Math.min(10, Math.max(0.1, Number(settings.forceGunMultiplier) || 1)) };
    }

    /** Called by the existing presentation loop; no timers or extra RAF. @returns {ForceGunInput|undefined} */
    sample() {
        const held = this.held, snapshot = this.deps.getSnapshot();
        if (!held || !snapshot) return undefined;
        const entity = snapshot.entities.find(item => item.id === held.id);
        if (snapshot.epoch !== held.epoch || snapshot.profile !== 'playground' || !entity?.alive || (!held.starting && (!snapshot.playing || this.deps.getTelemetry()?.token !== held.token))) {
            this.end(); return undefined;
        }
        return this.targetInput(snapshot, this.deps.getSettings(), held);
    }

    /** @param {number} pixels */
    wheel(pixels) {
        if (!this.held || !Number.isFinite(pixels)) return false;
        // Bound the manipulation lever arm independently of unlimited idle dolly.
        this.held.depth = Math.max(0.1, Math.min(1e6, this.held.depth * Math.exp(Math.max(-10, Math.min(10, pixels * 0.0012)))));
        return true;
    }

    get state() {
        const held = this.held, snapshot = this.deps.getSnapshot();
        if (!held || !snapshot) return null;
        const entity = snapshot.entities.find(item => item.id === held.id), telemetry = this.deps.getTelemetry();
        const current = telemetry?.token === held.token ? telemetry : null;
        const settings = this.deps.getSettings();
        return { active: true, mode: held.mode, name: entity?.name || 'Body', mass: current?.mass ?? entity?.mass ?? 0,
            force: current?.forceMagnitude ?? 0, cap: current?.maxForce ?? 0, depth: held.depth,
            effort: forceGunEffort(current),
            sensitivity: settings.forceGunSensitivity || 'normal', multiplier: Number(settings.forceGunMultiplier) || 1 };
    }

    /** Current source pose moves the same local optical surface anchor. */
    get anchor() {
        const held = this.held, snapshot = this.deps.getSnapshot();
        const entity = snapshot?.entities.find(item => item.id === held?.id);
        if (!held || !entity) return null;
        const offset = rotate(rotationMatrix(entity.rotation), held.localAnchor);
        const source = entity.position.map((value, axis) => value + offset[axis]);
        return held.mirrored ? reflectPolar(source) : source;
    }

    end() {
        const held = this.held;
        if (!held) return;
        this.held = null;
        void this.deps.command({ type: 'gun-end', token: held.token }).catch(error => this.deps.onStatus(String(error)));
    }
    dispose() { this.end(); this.disposed = true; }
}

/** Bounded screen-space tether, owned by one workspace and its existing loop. */
export class ObserverTetherOverlay {
    /** @param {HTMLElement} host */
    constructor(host) {
        this.element = document.createElementNS('http://www.w3.org/2000/svg', 'svg');
        this.element.classList.add('observer-force-tether');
        this.element.setAttribute('aria-hidden', 'true');
        this.element.style.cssText = 'position:absolute;inset:0;width:100%;height:100%;pointer-events:none;overflow:hidden;z-index:3;display:none';
        this.line = document.createElementNS(this.element.namespaceURI, 'path');
        this.line.setAttribute('fill', 'none'); this.line.setAttribute('stroke-width', '2');
        this.marker = document.createElementNS(this.element.namespaceURI, 'circle');
        this.marker.setAttribute('r', '6'); this.marker.setAttribute('fill', 'none'); this.marker.setAttribute('stroke-width', '2');
        this.element.append(this.line, this.marker); host.append(this.element);
    }

    /** @param {ObserverForceGun} gun @param {WorldSnapshot} snapshot @param {GunSettings} settings @param {import('./renderer.js').ObserverRenderer} renderer */
    update(gun, snapshot, settings, renderer) {
        const anchor = gun.anchor, held = gun.held;
        if (!anchor || !held) { this.element.style.display = 'none'; return; }
        const point = renderer.projectPoint(snapshot, settings, anchor);
        if (!point.visible || !Number.isFinite(point.x + point.y)) { this.element.style.display = 'none'; return; }
        const obstruction = renderer.pick(snapshot, settings, point.x * 2 - 1, 1 - point.y * 2);
        // Do not draw the attachment marker through foreground objects or the
        // far side of the held object. The tether remains a faint aiming aid.
        const occluded = !!obstruction && obstruction.distance < point.distance - Math.max(0.03, point.distance * 0.003);
        const width = renderer.canvas.clientWidth, height = renderer.canvas.clientHeight;
        const x = Math.max(2, Math.min(width - 2, point.x * width)), y = Math.max(2, Math.min(height - 2, point.y * height));
        const sx = width * 0.52, sy = height * 0.55;
        this.element.style.display = '';
        this.element.dataset.mode = held.mode;
        this.element.dataset.occluded = String(occluded);
        this.element.setAttribute('viewBox', `0 0 ${width} ${height}`);
        const effort = gun.state?.effort;
        const color = effort?.color ?? '#6de4ff';
        this.element.dataset.effort = effort ? String(effort.percent) : 'pending';
        this.element.dataset.overloaded = String(effort?.overloaded ?? false);
        this.line.setAttribute('stroke', color); this.marker.setAttribute('stroke', color);
        this.line.setAttribute('stroke-width', String(2 + 1.5 * (effort?.fraction ?? 0)));
        this.line.setAttribute('opacity', occluded ? '0.25' : '0.85');
        this.line.setAttribute('stroke-dasharray', occluded ? '4 6' : effort?.overloaded ? '3 4' : 'none');
        this.line.setAttribute('d', `M ${sx} ${sy} Q ${(sx + x) / 2} ${Math.min(height, (sy + y) / 2 + 22)} ${x} ${y}`);
        this.marker.setAttribute('cx', String(x)); this.marker.setAttribute('cy', String(y));
        this.marker.setAttribute('visibility', occluded ? 'hidden' : 'visible');
    }
    hide() { this.element.style.display = 'none'; }
    dispose() { this.element.remove(); }
}

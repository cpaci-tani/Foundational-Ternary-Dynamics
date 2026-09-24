// @ts-check
/** Bounded assistant operations over the existing Observer owner. No physics
 * implementation, transport credentials, model calls, or animation loop live here. */
import { SHAPES, ENVIRONMENT_PRESETS, EXPERIMENTS, LAYERS } from '../observer/catalog.js';
import { ENTITY_LIMIT, MAX_GRAVITY_STRENGTH } from '../observer/types.js';
import { reflectAuthorPatch } from '../observer/mirror-frame.js';
import { validateValue as validate } from './contracts.js';
/** @typedef {import('./contracts.js').ObservationEnvelope} ObservationEnvelope */
/** @typedef {import('./contracts.js').AssistantAction} AssistantAction */
/** @typedef {import('./contracts.js').CommandReceipt} CommandReceipt */
/** @typedef {import('../observer/types.js').WorldSnapshot} WorldSnapshot */
/** @typedef {import('../observer/types.js').WorldEntity} WorldEntity */

/** @param {number} minimum @param {number} maximum */
const numeric = (minimum, maximum) => ({ type: 'number', minimum, maximum });
/** @param {number} minimum @param {number} maximum */
const integer = (minimum, maximum) => ({ type: 'integer', minimum, maximum });
const boolean = { type: 'boolean' };
const text = { type: 'string', minLength: 1, maxLength: 120 };
/** @param {string[]} values */
const choice = values => ({ type: 'string', enum: values });
/** @param {number} [minimum] @param {number} [maximum] */
const vector = (minimum = -1e6, maximum = 1e6) => ({ type: 'array', minItems: 3, maxItems: 3, items: numeric(minimum, maximum) });
/** @param {Record<string,any>} [properties] @param {string[]} [required] */
const object = (properties = {}, required = []) => ({ type: 'object', additionalProperties: false, properties, required });
/** @type {Record<string,any>} */
const entityFields = {
    name: text, shape: choice(SHAPES.map(item => item.id)), position: vector(), size: vector(1e-6, 1e5),
    rotation: vector(-Math.PI * 100, Math.PI * 100), velocity: vector(), color: vector(0, 1),
    emission: numeric(0, 1e5), mass: numeric(1e-6, 1e5), bodyType: choice(['fixed', 'dynamic', 'kinematic']),
    restitution: numeric(0, 1), friction: numeric(0, 1e5), damping: numeric(0, 1e5),
    gravity: boolean, collisions: boolean, overlay: boolean, angularVelocity: vector(), properAcceleration: vector(),
};
const fieldNames = ['name', 'position', 'velocity', 'properTime', 'distance', 'emissionTime', 'dimensions', 'axes', 'bounds', 'trajectory', 'frameVelocity'];
/** @param {string} type @param {string} description @param {any} args */
const descriptor = (type, description, args) => ({ type: `observer.${type}`, description, args });
/** @template T @param {T} value @returns {T} */
function freeze(value) { if (value && typeof value === 'object') { Object.values(value).forEach(freeze); Object.freeze(value); } return value; }

/** Closed JSON schemas sent to the model and checked again before execution.
 * All dimensional values use the observed world's declared units. */
export const OBSERVER_ACTION_DESCRIPTORS = freeze([
    descriptor('pause', 'Pause the current Observer session.', object()),
    descriptor('resume', 'Resume the current session at the present.', object()),
    descriptor('step', 'Advance 1–120 fixed 1/120 second ticks while paused; remain paused.', object({ count: integer(1, 120) })),
    descriptor('create', 'Create one bounded geometric object; do not fabricate a lattice identity.', object(entityFields)),
    descriptor('select', 'Select an existing stable object id, or clear the selection with null.', object({ id: { anyOf: [text, { type: 'null' }] } }, ['id'])),
    descriptor('update', 'Edit the current selected/captured object, or explicit id. massFactor scales its current mass; cannot combine mass and massFactor. Spatial values use the selected mirrored author frame.', object({ id: text, ...entityFields, massFactor: numeric(0.001, 1000) })),
    descriptor('delete', 'Delete a current live object; retained arriving-light history remains visible.', object({ id: text })),
    descriptor('restore', 'Restore an existing deleted object identity.', object({ id: text })),
    descriptor('impulse', 'Apply a finite nonzero central impulse to a dynamic Playground object; uses the selected mirrored author frame.', object({ id: text, impulse: vector() }, ['impulse'])),
    descriptor('world', 'Either change profile alone (fresh preparation), or edit Playground gravity/collisions. Profile cannot be combined with other fields.', object({ profile: choice(['sr', 'playground']), gravityMode: choice(['plane', 'uniform']), gravityStrength: numeric(0, MAX_GRAVITY_STRENGTH), gravity: vector(-1e12, 1e12), objectCollisions: boolean, planeCollision: boolean })),
    descriptor('forceGun', 'Configure the user-operated Playground force gun or release it. This does not autonomously grab or inject forces.', object({ enabled: boolean, sensitivity: choice(['delicate', 'normal', 'strong']), multiplier: numeric(0.1, 10), release: boolean })),
    descriptor('environment', 'Edit the declared decorative environment using the catalog.', object({ preset: choice(ENVIRONMENT_PRESETS.map(item => item.id)), radius: numeric(0.001, 1e6), density: numeric(0, 1e6), spacing: numeric(0.001, 1e6), opacity: numeric(0, 1), seed: integer(0, 1e6), orientation: numeric(-1e6, 1e6), animationRate: numeric(-1e6, 1e6), color: vector(0, 1), anchor: choice(['world', 'camera']) })),
    descriptor('overlay', 'Change exactly one layer, field, filter, or current entity label. Layer/field/entity require enabled. Entity mode requires id or a captured selection.', object({ layer: choice(LAYERS.map(item => item.id)), field: choice(fieldNames), filter: choice(['selected', 'all', 'none']), id: text, enabled: boolean })),
    descriptor('camera', 'Choose mode pose (position/yaw/pitch, relocation resets observer clock), view (presentation/navigation settings; roll is visible when worldUp is false), or lookAt (existing target id). Modes cannot mix fields.', object({ mode: choice(['pose', 'view', 'lookAt']), id: text, position: vector(), yaw: numeric(-Math.PI * 100, Math.PI * 100), pitch: numeric(-Math.PI / 2 + 0.01, Math.PI / 2 - 0.01), roll: numeric(-Math.PI * 100, Math.PI * 100), fov: numeric(20, 120), speed: numeric(0.001, 0.99), acceleration: numeric(0.001, 100), grounded: boolean, worldUp: boolean, optical: boolean, doppler: boolean, beaming: boolean, artisticShading: boolean, renderScale: numeric(0.25, 1.5), autoQuality: boolean }, ['mode'])),
    descriptor('preset', 'Load one declared experiment; replaces the preparation and restarts its history.', object({ preset: choice(EXPERIMENTS.map(item => item.id)) }, ['preset'])),
    descriptor('undo', 'Undo the last authoring transaction and pause in a fresh epoch.', object()),
]);
const byType = new Map(OBSERVER_ACTION_DESCRIPTORS.map(item => [item.type, item]));

/** @param {any} value @returns {any} */
function clone(value) { return value === undefined ? null : structuredClone(value); }
/** @param {WorldEntity|undefined} entity */
function compactEntity(entity) {
    if (!entity) return null;
    /** @type {Record<string,any>} */
    const result = { id: entity.id, alive: entity.alive, editRevision: entity.editRevision ?? 0 };
    for (const key of Object.keys(entityFields)) result[key] = clone(/** @type {any} */(entity)[key]);
    return result;
}
/** @param {any} hit */
function compactHit(hit) {
    if (!hit) return null;
    return { entityId: hit.entityId || hit.id, historical: !!hit.historical, mirrored: !!hit.mirrored,
        emissionTime: hit.emissionTime ?? null, observedRevision: hit.revision ?? null,
        observedPosition: clone(hit.position), sourcePosition: clone(hit.sourcePosition), distance: hit.distance ?? null };
}
/** @param {any} workspace */
function viewOf(workspace) {
    // Reading the version records selection changes even when existing UI code
    // selects directly. Moving source bodies do not change this version.
    const version = workspace.getAssistantPreparationVersion?.();
    return { settings: clone(workspace.settings), selectedId: workspace.selectedId, selectedHit: clone(workspace.selectedHit),
        hit: clone(workspace.hit), mirrored: !!workspace.authorMirrored, forceGun: clone(workspace.forceGun?.state),
        rendering: clone(workspace.renderer?.diagnostics), version: version?.split(':').at(-1) ?? String(workspace.assistantViewVersion ?? 0) };
}
/** @param {WorldSnapshot} snapshot @param {ReturnType<typeof viewOf>} view @returns {ObservationEnvelope} */
function envelope(snapshot, view) {
    const actions = OBSERVER_ACTION_DESCRIPTORS.filter(item => {
        if (['observer.impulse', 'observer.forceGun'].includes(item.type)) return snapshot.profile === 'playground';
        if (item.type === 'observer.step') return !snapshot.playing && snapshot.scrubTime === null;
        return true;
    });
    const current = snapshot.entities.find(entity => entity.id === view.selectedId);
    return {
        workspace: 'observer', ownerId: snapshot.sessionId,
        preparationVersion: `${snapshot.epoch}:${snapshot.preparationVersion ?? 0}:${view.version}`, tick: snapshot.tick,
        capabilities: actions.map(item => item.type), actions: clone(actions),
        facts: { running: snapshot.playing, profile: snapshot.profile, physicsEngine: snapshot.physicsEngine,
            time: snapshot.time, epoch: snapshot.epoch, editVersion: snapshot.preparationVersion ?? 0,
            units: snapshot.units, experiment: snapshot.experiment, historyStart: snapshot.historyStart, scrubTime: snapshot.scrubTime,
            observer: clone(snapshot.observer), entities: snapshot.entities.map(compactEntity),
            gravity: clone(snapshot.gravity), gravityMode: snapshot.gravityMode, gravityStrength: snapshot.gravityStrength,
            objectCollisions: snapshot.objectCollisions, planeCollision: snapshot.planeCollision,
            environment: clone(snapshot.environment), warnings: clone(snapshot.warnings), backlogSeconds: snapshot.backlogSeconds,
            crosshair: compactHit(view.hit), forceGun: view.forceGun, settings: view.settings,
            rendering: view.rendering ? { internalScale: view.rendering.internalScale, gpuTimeMs: view.rendering.gpuTimeMs, adaptiveQuality: view.rendering.adaptiveQuality } : null,
            interpretation: 'Observer reference world. Arriving-light observations and current source entities are distinct; no lattice-to-matter or clock identification.' },
        selected: current ? { id: current.id, mirrored: view.mirrored, current: compactEntity(current),
            author: reflectAuthorPatch(compactEntity(current) ?? {}, view.mirrored), observed: compactHit(view.selectedHit) } : null,
        catalogs: { shapes: clone(SHAPES), experiments: clone(EXPERIMENTS), environments: clone(ENVIRONMENT_PRESETS), layers: clone(LAYERS) },
    };
}
/** @param {Record<string,any>} args @param {string[]} [ignored] */
function requireFields(args, ignored = []) {
    if (!Object.keys(args).some(key => !ignored.includes(key))) throw new Error('Specify at least one supported change.');
}
/** @param {WorldSnapshot} snapshot @param {Record<string,any>} patch @param {Record<string,any>} [original] */
function assertSR(snapshot, patch, original = {}) {
    if (snapshot.profile !== 'sr') return;
    if (patch.velocity && Math.hypot(...patch.velocity) > 0.99) throw new Error('SR velocity must not exceed 0.99c.');
    if (patch.angularVelocity?.some((/** @type {number} */ value) => value !== 0)) throw new Error('Continuous rigid rotation requires Playground.');
    if (patch.properAcceleration?.some((/** @type {number} */ value) => value !== 0) && !['clock', 'beacon'].includes(patch.shape ?? original.shape ?? 'sphere')) throw new Error('SR acceleration is restricted to clock or beacon markers.');
}

/** @param {{getWorkspace:()=>any,isActive:()=>boolean}} deps */
export function createObserverControl({ getWorkspace, isActive }) {
    let disposed = false, sequence = 0;
    /** @type {Set<AbortController>} */ const pending = new Set();
    const observe = () => {
        const workspace = getWorkspace();
        return !disposed && isActive() && workspace && !workspace.disposed && workspace.snapshot
            ? envelope(workspace.snapshot, viewOf(workspace)) : null;
    };
    /** @param {AssistantAction} action @param {{signal?:AbortSignal,assertActive?:()=>unknown,expected?:ObservationEnvelope}} [options] @returns {Promise<CommandReceipt>} */
    async function execute(action, { signal, assertActive, expected } = {}) {
        const commandId = `observer-assistant-${Date.now()}-${++sequence}`;
        /** @type {ObservationEnvelope|null} */ let before = null;
        let sent = false, settled = false;
        const abort = new AbortController();
        const cancel = () => abort.abort();
        signal?.addEventListener('abort', cancel, { once: true });
        if (signal?.aborted) cancel();
        pending.add(abort);
        /** @param {CommandReceipt['status']} status @param {ObservationEnvelope|null} after @param {string} [error] @returns {CommandReceipt} */
        const receipt = (status, after, error) => ({ commandId, status, action: clone(action), before, after, ...(error ? { error } : {}) });
        try {
            if (!action || typeof action !== 'object' || Array.isArray(action) || Object.keys(action).some(key => !['type', 'args'].includes(key))) throw new Error('Action must contain only type and args.');
            const schema = byType.get(action.type);
            if (!schema) throw new Error('Unsupported Observer action.');
            const args = action.args ?? {};
            if (!args || ![Object.prototype, null].includes(Object.getPrototypeOf(args))) throw new Error('Arguments must be a plain data object.');
            validate(args, schema.args);
            before = observe();
            if (!before) throw new Error('Observer is no longer active.');
            if (!expected || expected.ownerId !== before.ownerId || expected.preparationVersion !== before.preparationVersion) throw new Error('Stale Observer owner or preparation. Observe again before applying a change.');
            if (!before.capabilities.includes(action.type)) throw new Error('This action is unavailable in the current Observer profile or playback state.');
            const workspace = getWorkspace(), originalView = viewOf(workspace), captured = before;
            /** @type {WorldSnapshot} */ const snapshot = workspace.snapshot;
            const guard = () => {
                if (disposed || abort.signal.aborted) throw new DOMException('Assistant command cancelled.', 'AbortError');
                if (assertActive?.() === false) throw new Error('Observer is no longer active.');
                const current = observe();
                if (getWorkspace() !== workspace || !current || current.ownerId !== captured.ownerId || current.preparationVersion !== captured.preparationVersion) throw new Error('Stale Observer owner or preparation.');
            };
            guard();
            const target = (allowDeleted = false) => {
                const id = args.id ?? expected.selected?.id ?? expected.facts?.crosshair?.entityId;
                if (!id) throw new Error('Select an object or supply its stable id.');
                const entity = snapshot.entities.find(item => item.id === id);
                if (!entity || (!allowDeleted && !entity.alive)) throw new Error('The captured object is absent or deleted; historical light is not a live object.');
                return entity;
            };
            /** @type {import('../observer/types.js').WorldCommand|null} */ let command = null;
            /** @type {(()=>void)|null} */ let local = null;
            const kind = action.type.slice('observer.'.length);
            if (kind === 'pause' || kind === 'resume') command = { type: kind === 'pause' ? 'pause' : 'play' };
            else if (kind === 'step') command = { type: 'step', count: args.count ?? 1 };
            else if (kind === 'create') {
                if (snapshot.entities.length >= ENTITY_LIMIT) throw new Error('Observer entity budget is full.');
                assertSR(snapshot, args); command = { type: 'create', entity: clone(args) };
            } else if (kind === 'select') {
                if (args.id !== null && !snapshot.entities.some(item => item.id === args.id)) throw new Error('Unknown object identity.');
                local = () => { workspace.selectedId = args.id; workspace.selectedHit = null; workspace.assistantViewVersion++; workspace.updateUI(); };
            } else if (['update', 'delete', 'restore', 'impulse'].includes(kind)) {
                const entity = target(kind === 'restore');
                command = { type: kind, id: entity.id, expectedTargetEditRevision: entity.editRevision ?? 0 };
                if (kind === 'restore' && entity.alive) throw new Error('The selected object is already alive.');
                if (kind === 'update') {
                    requireFields(args, ['id']);
                    if ('mass' in args && 'massFactor' in args) throw new Error('Choose mass or massFactor, not both.');
                    const { id: _id, massFactor, ...patch } = args;
                    if (massFactor !== undefined) { patch.mass = entity.mass * massFactor; validate(patch.mass, entityFields.mass, 'resulting mass'); }
                    assertSR(snapshot, patch, entity); command.patch = clone(patch);
                } else if (kind === 'impulse') {
                    if (entity.bodyType !== 'dynamic') throw new Error('Impulse requires a live dynamic body.');
                    if (!args.impulse.some((/** @type {number} */ value) => value !== 0)) throw new Error('Impulse must be nonzero.');
                    command.impulse = clone(args.impulse);
                }
            } else if (kind === 'world') {
                requireFields(args);
                if ('profile' in args) {
                    if (Object.keys(args).length !== 1) throw new Error('Change profile in its own action, then observe the new preparation.');
                    command = { type: 'profile', profile: args.profile };
                } else {
                    if (snapshot.profile !== 'playground') throw new Error('World gravity and collisions require Playground.');
                    command = { type: 'world-physics', patch: clone(args) };
                }
            } else if (kind === 'forceGun') {
                requireFields(args);
                local = () => {
                    if (args.release) { workspace.input.releaseForce(); workspace.forceGun.end(); workspace.tether.hide(); }
                    for (const [key, setting] of [['enabled', 'forceGunEnabled'], ['sensitivity', 'forceGunSensitivity'], ['multiplier', 'forceGunMultiplier']]) if (key in args) workspace.setSetting(setting, args[key]);
                };
            } else if (kind === 'environment') { requireFields(args); command = { type: 'environment', patch: clone(args) }; }
            else if (kind === 'overlay') {
                const categories = ['layer', 'field', 'filter', 'id'].filter(key => key in args);
                if (categories.length > 1 || !('filter' in args) && !('enabled' in args) || 'filter' in args && 'enabled' in args) throw new Error('Choose one overlay layer, field, filter, or entity.');
                if ('layer' in args) local = () => workspace.setSetting('layers', { ...workspace.settings.layers, [args.layer]: args.enabled });
                else if ('field' in args) local = () => workspace.setSetting('overlayFields', { ...workspace.settings.overlayFields, [args.field]: args.enabled });
                else if ('filter' in args) local = () => workspace.setSetting('overlayFilter', args.filter);
                else { const entity = target(); command = { type: 'update', id: entity.id, expectedTargetEditRevision: entity.editRevision ?? 0, patch: { overlay: args.enabled } }; }
            } else if (kind === 'camera') {
                const { mode, ...fields } = args;
                const poseKeys = ['position', 'yaw', 'pitch'];
                const viewKeys = ['fov', 'speed', 'acceleration', 'grounded', 'worldUp', 'roll', 'optical', 'doppler', 'beaming', 'artisticShading', 'renderScale', 'autoQuality'];
                const allowed = mode === 'view' ? viewKeys : mode === 'pose' ? poseKeys : ['id'];
                if (Object.keys(fields).some(key => !allowed.includes(key))) throw new Error('Camera modes cannot mix pose, target, and view fields.');
                requireFields(fields);
                if (mode === 'view') local = () => { for (const [key, value] of Object.entries(fields)) workspace.setSetting(key, value); };
                else {
                    let patch = fields;
                    if (mode === 'lookAt') {
                        const entity = target(), delta = entity.position.map((value, index) => value - snapshot.observer.position[index]);
                        if (Math.hypot(...delta) < 1e-9) throw new Error('Target is at the current viewpoint.');
                        patch = { yaw: Math.atan2(-delta[0], -delta[2]), pitch: Math.atan2(delta[1], Math.hypot(delta[0], delta[2])) };
                    }
                    command = { type: 'observer', patch };
                }
            } else if (kind === 'preset') command = { type: 'preset', preset: args.preset };
            else if (kind === 'undo') command = { type: 'undo' };
            guard();
            if (local) {
                local(); settled = true;
                return receipt('applied', observe());
            }
            if (!command) throw new Error('No supported operation was prepared.');
            command = { ...command, sessionId: snapshot.sessionId, expectedEpoch: snapshot.epoch, expectedPreparationVersion: snapshot.preparationVersion ?? 0 };
            sent = true;
            const result = await workspace.authorCommand(command, { signal: abort.signal, assertActive: guard });
            settled = true;
            if (!result) return receipt('unknown', null, 'Observer did not return an authoritative command receipt.');
            if (!result.ok) return receipt(/stale|cancelled/i.test(result.error || '') ? 'superseded' : 'rejected', envelope(result.snapshot, originalView), result.error || 'Observer rejected the command.');
            // The receipt is the state returned by this transaction, not a fresh
            // observation that could silently incorporate intervening manual work.
            const committedView = { ...originalView };
            if (result.snapshot.epoch !== snapshot.epoch) {
                committedView.selectedHit = null; committedView.hit = null; committedView.mirrored = false;
            }
            if (!result.snapshot.entities.some((/** @type {WorldEntity} */ entity) => entity.id === committedView.selectedId)) {
                committedView.selectedId = null; committedView.selectedHit = null; committedView.mirrored = false;
            }
            if (committedView.selectedId !== originalView.selectedId || committedView.mirrored !== originalView.mirrored) committedView.version = String(Number(originalView.version) + 1);
            if (kind === 'camera' && !workspace.disposed && workspace.snapshot === result.snapshot) workspace.input.setPose(result.snapshot.observer);
            return receipt('applied', envelope(result.snapshot, committedView));
        } catch (error) {
            const message = error instanceof Error ? error.message : String(error);
            return receipt(sent && !settled ? 'unknown' : /stale|cancelled|no longer active/i.test(message) ? 'superseded' : 'rejected', before, message);
        } finally {
            pending.delete(abort); signal?.removeEventListener('abort', cancel);
        }
    }
    return { observe, execute, dispose() { disposed = true; for (const request of pending) request.abort(); pending.clear(); } };
}

// @ts-check
/** Local, opt-in world storage. Constructing this class never opens IndexedDB. */
import { SHAPES, ENVIRONMENT_PRESETS } from './catalog.js';
import { MAX_GRAVITY_STRENGTH } from './types.js';
/** @typedef {import('./types.js').WorldSnapshot} WorldSnapshot */
/** @typedef {import('./catalog.js').ObserverSettings} ObserverSettings */
/** @typedef {{id:string,name:string,kind:string,updatedAt:number,bytes:number,text:string}} StorageRecord */
/** @typedef {{getAll:()=>Promise<StorageRecord[]>,get:(id:string)=>Promise<StorageRecord|undefined>,put:(record:StorageRecord)=>Promise<unknown>,delete:(id:string)=>Promise<unknown>,clear:()=>Promise<unknown>,close?:()=>void}} StorageBackend */
/** @typedef {{format:string,schemaVersion:number,snapshot:WorldSnapshot,settings:ObserverSettings}} PortableDocument */

const FORMAT = 'ftd-observer-world';
const DB_NAME = 'ftd-observer';
const STORE = 'worlds';
const MAX_DOCUMENT_BYTES = 32 * 1024 * 1024;
const MAX_ENTITIES = 256;
const MAX_SEGMENTS = 65536;
const INFINITY_KEY = '$observerNumber';
const SHAPE_IDS = new Set([...SHAPES.map(shape => shape.id), 'light-pulse']);
const ENV_IDS = new Set(ENVIRONMENT_PRESETS.map(env => env.id));
/** @param {string} value */
const bytesOf = value => new TextEncoder().encode(value).byteLength;

/** @param {string} message @returns {never} */
function fail(message) { throw new TypeError(`Invalid Observer world: ${message}`); }
/** @param {unknown} value @param {string} label @param {number} [minimum] @param {number} [maximum] */
function finite(value, label, minimum = -Infinity, maximum = Infinity) {
    if (typeof value !== 'number' || !Number.isFinite(value) || value < minimum || value > maximum) fail(label);
}
/** @param {unknown} value @param {string} label @param {number} [minimum] @param {number} [maximum] */
function vec(value, label, minimum = -Infinity, maximum = Infinity) {
    if (!Array.isArray(value) || value.length !== 3) fail(`${label} must have three coordinates`);
    value.forEach(number => finite(number, label, minimum, maximum));
}
/** @param {unknown} value @param {string} label @param {number} [limit] */
function string(value, label, limit = 256) {
    if (typeof value !== 'string' || value.length > limit) fail(label);
}

/** Reject executable/prototype-bearing input and non-finite ordinary values.
 * @param {any} value @param {string[]} [path] @param {number} [depth] @param {{count:number}} [budget]
 */
function inspect(value, path = [], depth = 0, budget = { count: 0 }) {
    if (++budget.count > 3000000 || depth > 48) fail('document nesting or entry limit exceeded');
    if (typeof value === 'number') {
        const last = path.at(-1);
        const inSegments = path[0] === 'snapshot' && path[1] === 'segments';
        if (!Number.isFinite(value) && !(value === Infinity && inSegments && ['t1', 'end', 'endTime', 'endTick'].includes(last || ''))) fail(`non-finite value at ${path.join('.')}`);
        return;
    }
    if (value === null || typeof value === 'boolean') return;
    if (typeof value === 'string') { if (value.length > 1000000) fail('oversized string'); return; }
    if (typeof value !== 'object') fail(`unsupported value at ${path.join('.')}`);
    const proto = Object.getPrototypeOf(value);
    if (!Array.isArray(value) && proto !== Object.prototype && proto !== null) fail('non-plain object');
    for (const key of Object.keys(value)) {
        if (['__proto__', 'constructor', 'prototype'].includes(key)) fail('unsafe property name');
        inspect(value[key], [...path, key], depth + 1, budget);
    }
}

/** Validate a portable document before it can replace the authoritative session.
 * @param {any} document @returns {{snapshot:WorldSnapshot,settings:ObserverSettings}}
 */
export function validateObserverDocument(document) {
    inspect(document);
    if (!document || document.format !== FORMAT || document.schemaVersion !== 1) fail('unsupported document format or version');
    const snapshot = document.snapshot;
    if (!snapshot || snapshot.schemaVersion !== 1) fail('unsupported snapshot version');
    string(snapshot.sessionId, 'session identity');
    for (const key of ['epoch', 'revision', 'tick']) {
        finite(snapshot[key], key, 0, Number.MAX_SAFE_INTEGER);
        if (!Number.isInteger(snapshot[key])) fail(`${key} must be an integer`);
    }
    finite(snapshot.time, 'time', 0);
    if (!['sr', 'playground'].includes(snapshot.profile)) fail('unknown physics profile');
    if ('gravityMode' in snapshot && !['plane', 'uniform'].includes(snapshot.gravityMode)) fail('unknown gravity model');
    if ('gravityStrength' in snapshot) finite(snapshot.gravityStrength, 'gravity strength', 0, MAX_GRAVITY_STRENGTH);
    if ('gravity' in snapshot) vec(snapshot.gravity, 'world gravity');
    for (const key of ['objectCollisions', 'planeCollision']) if (key in snapshot && typeof snapshot[key] !== 'boolean') fail(`world ${key} must be boolean`);
    if (typeof snapshot.playing !== 'boolean') fail('playing flag');
    if (!snapshot.observer) fail('missing observer');
    vec(snapshot.observer.position, 'observer position');
    vec(snapshot.observer.velocity, 'observer velocity');
    if (Math.hypot(...snapshot.observer.velocity) >= 1 && snapshot.profile === 'sr') fail('observer velocity must remain below c');
    finite(snapshot.observer.properTime, 'observer proper time', 0);
    for (const key of ['yaw', 'pitch', 'roll']) finite(snapshot.observer[key], `observer ${key}`);
    if (!Array.isArray(snapshot.entities) || snapshot.entities.length > MAX_ENTITIES) fail('entity limit exceeded');
    const ids = new Set();
    for (const entity of snapshot.entities) {
        string(entity.id, 'entity identity');
        if (!entity.id || ids.has(entity.id)) fail('duplicate or empty entity identity');
        ids.add(entity.id);
        string(entity.name, 'entity name');
        if (!SHAPE_IDS.has(entity.shape)) fail('unknown object shape');
        vec(entity.position, 'entity position'); vec(entity.velocity, 'entity velocity');
        vec(entity.size, 'entity size', Number.MIN_VALUE, 1000000);
        vec(entity.rotation, 'entity rotation'); vec(entity.color, 'entity color', 0, 1);
        finite(entity.mass, 'entity mass', 0); finite(entity.emission, 'entity emission', 0);
        if (typeof entity.alive !== 'boolean' || typeof entity.overlay !== 'boolean') fail('entity flags');
        if ('collisions' in entity && typeof entity.collisions !== 'boolean') fail('entity collisions must be boolean');
        finite(entity.revision, 'entity revision', 0, Number.MAX_SAFE_INTEGER);
        finite(entity.createdAt, 'entity creation time');
        finite(entity.clockOffset, 'entity clock offset');
        if (snapshot.profile === 'sr' && entity.shape !== 'pulse' && Math.hypot(...entity.velocity) >= 1) fail('massive entity velocity must remain below c');
    }
    if (!Array.isArray(snapshot.segments) || snapshot.segments.length > MAX_SEGMENTS) fail('history limit exceeded');
    for (const segment of snapshot.segments) {
        if (!segment || typeof segment !== 'object' || Array.isArray(segment)) fail('invalid history segment');
        string(segment.entityId, 'segment entity identity');
        if (!ids.has(segment.entityId)) fail('history references a missing entity');
        finite(segment.start, 'segment start');
        if (segment.end !== null && segment.end !== Infinity) finite(segment.end, 'segment end', segment.start);
        finite(segment.originTime, 'segment origin time');
        finite(segment.clockOffset, 'segment clock offset');
        finite(segment.revision, 'segment revision', 0, Number.MAX_SAFE_INTEGER);
        if (!SHAPE_IDS.has(segment.shape)) fail('unknown history shape');
        for (const key of ['position', 'velocity', 'rotation']) vec(segment[key], `segment ${key}`);
        vec(segment.size, 'segment size', Number.MIN_VALUE, 1000000);
        vec(segment.color, 'segment color', 0, 1);
    }
    finite(snapshot.historyStart, 'history start');
    if (snapshot.historyStart > snapshot.time) fail('history starts in the future');
    const environment = snapshot.environment;
    if (!environment || !ENV_IDS.has(environment.preset)) fail('unknown environment preset');
    for (const key of ['seed', 'radius', 'density', 'spacing', 'orientation', 'opacity', 'animationRate']) finite(environment[key], `environment ${key}`);
    if (environment.radius <= 0 || environment.spacing <= 0 || environment.density < 0 || environment.opacity < 0 || environment.opacity > 1) fail('environment range');
    vec(environment.color, 'environment color', 0, 1);
    if (!['world', 'camera'].includes(environment.anchor)) fail('environment anchor');
    if (!document.settings || typeof document.settings !== 'object' || Array.isArray(document.settings)) fail('missing camera settings');
    const cameraSettings = document.settings;
    for (const [key, min, max] of /** @type {Array<[string,number,number]>} */ ([['fov', 30, 120], ['speed', 0, 0.99], ['acceleration', 0, 100], ['sensitivity', 0.00001, 1], ['renderScale', 0.1, 2], ['roll', -Math.PI, Math.PI], ['gridSnap', 0, 100000]])) {
        if (key in cameraSettings) finite(cameraSettings[key], `camera ${key}`, min, max);
    }
    if ('storageCapMiB' in cameraSettings) finite(cameraSettings.storageCapMiB, 'storage cap', 1, 1024);
    if ('scrollZoomSpeed' in cameraSettings) finite(cameraSettings.scrollZoomSpeed, 'scroll zoom speed', 0.001, 1000000);
    if ('fractalDetail' in cameraSettings) finite(cameraSettings.fractalDetail, 'fractal detail', 0, 1);
    for (const [key, min, max] of /** @type {Array<[string,number,number]>} */ ([['waveWavelength', 0.5, 12], ['waveSeparation', 0, 12], ['waveAmplitude', 0, 2], ['wavePhase', -Math.PI, Math.PI]])) {
        if (key in cameraSettings) finite(cameraSettings[key], key, min, max);
    }
    if ('polarizationMode' in cameraSettings && !['linear', 'circular', 'elliptical'].includes(cameraSettings.polarizationMode)) fail('polarization mode');
    if ('forceGunMultiplier' in cameraSettings) finite(cameraSettings.forceGunMultiplier, 'force gun multiplier', 0.1, 10);
    if ('forceGunSensitivity' in cameraSettings && !['delicate', 'normal', 'strong'].includes(cameraSettings.forceGunSensitivity)) fail('force gun sensitivity');
    for (const [key, min, max] of /** @type {Array<[string,number,number]>} */ ([['feedbackLayers', 1, 3], ['feedbackDepth', 3, 5], ['feedbackStrength', 0, 1], ['feedbackScale', 0.25, 1]])) {
        if (key in cameraSettings) finite(cameraSettings[key], `camera ${key}`, min, max);
    }
    for (const key of ['feedbackLayers', 'feedbackDepth']) if (key in cameraSettings && !Number.isInteger(cameraSettings[key])) fail(`camera ${key} must be an integer`);
    for (const key of ['grounded', 'worldUp', 'invertY', 'reticle', 'doppler', 'beaming', 'optical', 'artisticShading', 'pauseOnInspect', 'liveLink', 'autoQuality', 'mirrorWorld', 'feedbackEnabled', 'forceGunEnabled']) {
        if (key in cameraSettings && typeof cameraSettings[key] !== 'boolean') fail(`camera ${key} must be boolean`);
    }
    if (cameraSettings.axisLocks && (!Array.isArray(cameraSettings.axisLocks) || cameraSettings.axisLocks.length !== 3 || cameraSettings.axisLocks.some((/** @type {unknown} */ value) => typeof value !== 'boolean'))) fail('camera axis locks');
    for (const name of ['layers', 'overlayFields']) if (cameraSettings[name]) {
        if (typeof cameraSettings[name] !== 'object' || Array.isArray(cameraSettings[name]) || Object.values(cameraSettings[name]).some(value => typeof value !== 'boolean')) fail(`camera ${name}`);
    }
    if (cameraSettings.bindings) {
        if (typeof cameraSettings.bindings !== 'object' || Array.isArray(cameraSettings.bindings)) fail('keybindings');
        for (const value of Object.values(cameraSettings.bindings)) string(value, 'keybinding', 40);
    }
    if (snapshot.scrubTime !== null && snapshot.scrubTime !== undefined) finite(snapshot.scrubTime, 'history cursor', snapshot.historyStart, snapshot.time);
    return { snapshot, settings: document.settings };
}

/** @param {PortableDocument} document */
function encode(document) {
    validateObserverDocument(document);
    const text = JSON.stringify(document, (_key, value) => value === Infinity ? { [INFINITY_KEY]: 'Infinity' } : value);
    if (bytesOf(text) > MAX_DOCUMENT_BYTES) fail('document exceeds 32 MiB limit');
    return text;
}

/** @param {string} text */
function decode(text) {
    if (typeof text !== 'string' || bytesOf(text) > MAX_DOCUMENT_BYTES) fail('document exceeds 32 MiB limit');
    const parsed = JSON.parse(text);
    // Inspect the JSON first, before decoding reserved markers.
    inspect(parsed);
    /** @param {any} value @returns {any} */
    function revive(value) {
        if (!value || typeof value !== 'object') return value;
        if (Object.prototype.hasOwnProperty.call(value, INFINITY_KEY)) {
            if (Object.keys(value).length !== 1 || value[INFINITY_KEY] !== 'Infinity') fail('invalid infinity marker');
            return Infinity;
        }
        for (const key of Object.keys(value)) value[key] = revive(value[key]);
        return value;
    }
    const revived = revive(parsed);
    const validated = validateObserverDocument(revived);
    // Legacy infinity encodings mean an open interval. The finite runtime uses null.
    for (const segment of validated.snapshot.segments) if (segment.end === Infinity) segment.end = null;
    return validated;
}

/** @template T @param {IDBRequest<T>} request @returns {Promise<T>} */
function requestPromise(request) {
    return new Promise((resolve, reject) => {
        request.onsuccess = () => resolve(request.result);
        request.onerror = () => reject(request.error || new Error('Local storage request failed.'));
    });
}

/** Adapter contract: async getAll(), get(id), put(record), delete(id), clear().
 * @param {IDBDatabase} db @returns {StorageBackend}
 */
function idbBackend(db) {
    const read = () => requestPromise(db.transaction(STORE, 'readonly').objectStore(STORE).getAll());
    /** @param {'put'|'delete'|'clear'} method @param {StorageRecord|string} [value] @returns {Promise<void>} */
    const write = (method, value) => new Promise((resolve, reject) => {
        const transaction = db.transaction(STORE, 'readwrite');
        const store = transaction.objectStore(STORE);
        if (method === 'clear') store.clear();
        else if (method === 'delete') store.delete(/** @type {string} */ (value));
        else store.put(value);
        transaction.oncomplete = () => resolve();
        transaction.onerror = () => reject(transaction.error || new Error('Local save failed.'));
        transaction.onabort = () => reject(transaction.error || new Error('Local save was cancelled.'));
    });
    return {
        getAll: () => read(),
        get: id => requestPromise(db.transaction(STORE, 'readonly').objectStore(STORE).get(id)),
        put: record => write('put', record), delete: id => write('delete', id), clear: () => write('clear'),
        close: () => db.close(),
    };
}

export class ObserverStorage {
    /** @param {{storageCapBytes?:number,clock?:()=>number,backend?:StorageBackend|null}} [options] */
    constructor({ storageCapBytes = 100 * 1024 * 1024, clock = () => Date.now(), backend = null } = {}) {
        this.storageCapBytes = storageCapBytes;
        this.clock = clock;
        this.backend = backend;
        this.autosave = false;
        this.disposed = false;
        this.lastAutosaveAt = 0;
        this.lastAutosaveText = '';
        this.autosaveSlot = 0;
        /** @type {Promise<StorageBackend>|null} */
        this._openPromise = null;
        this._queue = Promise.resolve();
        this._nextId = 0;
    }

    /** @param {boolean} [create] @returns {Promise<StorageBackend|null>} */
    async _open(create = false) {
        if (this.disposed) throw new Error('Observer storage is disposed.');
        if (this.backend) return this.backend;
        if (this._openPromise) return this._openPromise;
        if (!globalThis.indexedDB) {
            if (!create) return null;
            throw new Error('This browser does not provide IndexedDB. Export a JSON world instead.');
        }
        // Reads may reopen an existing store, but never create an empty database.
        if (!create) {
            if (!indexedDB.databases) return null;
            const databases = await indexedDB.databases();
            if (!databases.some(db => db.name === DB_NAME)) return null;
        }
        this._openPromise = new Promise((resolve, reject) => {
            const request = indexedDB.open(DB_NAME, 1);
            request.onupgradeneeded = () => {
                if (!create) { request.transaction?.abort(); return; }
                request.result.createObjectStore(STORE, { keyPath: 'id' });
            };
            request.onerror = () => reject(request.error || new Error('Could not open local world storage.'));
            request.onblocked = () => reject(new Error('Local world storage is blocked by another open tab.'));
            request.onsuccess = () => {
                if (this.disposed) { request.result.close(); reject(new Error('Observer storage is disposed.')); return; }
                request.result.onversionchange = () => { request.result.close(); this.backend = null; };
                this.backend = idbBackend(request.result);
                resolve(this.backend);
            };
        });
        try { return await this._openPromise; } finally { this._openPromise = null; }
    }

    /** @template T @param {()=>Promise<T>} operation @returns {Promise<T>} */
    _serialize(operation) {
        const result = this._queue.then(operation);
        this._queue = result.then(() => {}, () => {});
        return result;
    }

    /** @param {WorldSnapshot} snapshot @param {ObserverSettings} settings */
    exportWorld(snapshot, settings) { return encode({ format: FORMAT, schemaVersion: 1, snapshot, settings }); }
    /** @param {string} text */
    importWorld(text) { return decode(text); }

    /** @param {string} id @param {string} name @param {string} kind @param {string} text */
    async _save(id, name, kind, text) {
        const backend = await this._open(true);
        if (!backend) throw new Error('Local world storage is unavailable.');
        const records = await backend.getAll();
        const bytes = bytesOf(text);
        const used = records.reduce((sum, record) => sum + (record.id === id ? 0 : record.bytes), 0);
        if (used + bytes > this.storageCapBytes) throw new Error('Observer storage limit reached. Export or remove a saved world; named saves are never removed automatically.');
        const record = { id, name, kind, updatedAt: this.clock(), bytes, text };
        try { await backend.put(record); } catch (error) {
            if (error instanceof Error && error.name === 'QuotaExceededError') throw new Error('Browser storage quota exceeded. Export your world, then remove a saved world to make space.');
            throw error;
        }
        return { id, name, kind, updatedAt: record.updatedAt, bytes };
    }

    /** @param {string} name @param {WorldSnapshot} snapshot @param {ObserverSettings} settings */
    save(name, snapshot, settings) {
        const text = this.exportWorld(snapshot, settings);
        const label = String(name || 'Untitled world').trim().slice(0, 120) || 'Untitled world';
        return this._serialize(() => this._save(`world-${this.clock()}-${++this._nextId}-${globalThis.crypto?.randomUUID?.() || Math.random().toString(36).slice(2)}`, label, 'named', text));
    }

    /** @param {string} id */
    async load(id) {
        const backend = await this._open(false);
        const record = backend ? await backend.get(id) : null;
        if (!record) throw new Error('This saved world is no longer available.');
        return this.importWorld(record.text);
    }

    async list() {
        const backend = await this._open(false);
        return backend ? (await backend.getAll()).map(({ text: _text, ...record }) => record).sort((a, b) => b.updatedAt - a.updatedAt) : [];
    }

    /** @param {string} id */
    remove(id) { return this._serialize(async () => { const backend = await this._open(false); if (backend) await backend.delete(id); }); }
    clear() { return this._serialize(async () => { const backend = await this._open(false); if (backend) await backend.clear(); this.lastAutosaveText = ''; }); }
    async usage() {
        const records = await this.list();
        return { bytes: records.reduce((sum, record) => sum + record.bytes, 0), capBytes: this.storageCapBytes, count: records.length };
    }

    /** @param {boolean} enabled */
    async setAutosave(enabled) {
        if (enabled) await this._open(true);
        this.autosave = !!enabled;
        this.lastAutosaveAt = this.clock();
        return this.autosave;
    }

    /** Lowering the cap never removes existing records. @param {number} bytes */
    setStorageCap(bytes) {
        if (!Number.isFinite(bytes) || bytes < 1048576 || bytes > 1073741824) throw new RangeError('Storage limit must be between 1 and 1024 MiB.');
        this.storageCapBytes = Math.floor(bytes);
    }

    /** @param {WorldSnapshot} snapshot @param {ObserverSettings} settings */
    maybeAutosave(snapshot, settings) {
        if (!this.autosave || this.disposed || this.clock() - this.lastAutosaveAt < 30000) return Promise.resolve(null);
        this.lastAutosaveAt = this.clock();
        const text = this.exportWorld(snapshot, settings);
        if (text === this.lastAutosaveText) return Promise.resolve(null);
        return this._serialize(async () => {
            if (!this.autosave || this.disposed) return null;
            const slot = this.autosaveSlot % 2;
            let result;
            try { result = await this._save(`autosave-${slot}`, `Autosave ${slot + 1}`, 'autosave', text); }
            catch (error) { this.autosave = false; throw error; }
            this.lastAutosaveText = text;
            this.autosaveSlot++;
            return result;
        });
    }

    dispose() { this.autosave = false; this.disposed = true; this.backend?.close?.(); this.backend = null; }
}

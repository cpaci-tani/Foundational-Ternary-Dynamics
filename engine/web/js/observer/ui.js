// @ts-check
/** Observer controls. This module owns DOM only; commands go to the session owner. */
import { LifetimeScope } from '../ui/utils/lifetime-scope.js';
import { DEFAULT_SETTINGS, DEFAULT_BINDINGS, SHAPES, ENVIRONMENT_PRESETS, EXPERIMENTS, LAYERS } from './catalog.js';
import { MAX_GRAVITY_STRENGTH } from './types.js';
import { createPhenomenaInstrument } from './phenomena-instrument.js';
/** @typedef {import('./types.js').WorldSnapshot} WorldSnapshot */
/** @typedef {import('./types.js').WorldCommand} WorldCommand */
/** @typedef {import('./catalog.js').ObserverSettings & Record<string,any>} ObserverSettings */
/** @typedef {{active:boolean,mode?:string,name?:string,mass?:number,force?:number,cap?:number,depth?:number,sensitivity?:string,multiplier?:number,effort?:import('./force-gun-effort.js').TetherEffort|null}} ForceGunView */
/** @typedef {{settings?:Partial<ObserverSettings>,cameraOverride?:Partial<import('./optics.js').CameraState>,selectedId?:string|null,authorEntity?:import('./types.js').WorldEntity|null,authorMirrored?:boolean,hit?:import('./optics.js').OpticalHit|null,selectedHit?:import('./optics.js').OpticalHit|null,forceGun?:ForceGunView|null,lattice?:Record<string,any>,rendering?:{internalResolution:number[],internalScale:number,requestedScale:number,feedbackEnabled?:boolean,feedbackResolution?:number[],feedbackPasses?:number},status?:string,storage?:{entries?:Array<{id:string,name:string,kind:string,updatedAt:number,bytes:number}>,usage?:{bytes:number,capBytes:number},autosave?:boolean}}} ViewState */
/** @typedef {'objects'|'forcegun'|'world'|'camera'|'layers'|'phenomena'|'experiments'|'storage'|'lattice'|'help'} PanelName */
/** @typedef {Record<string,any>} FieldOptions */

const PANELS = [['objects', 'Objects'], ['forcegun', 'Force gun'], ['world', 'World'], ['camera', 'Camera'], ['layers', 'Layers'], ['phenomena', 'Phenomena'], ['experiments', 'Experiments'], ['storage', 'Saves'], ['lattice', 'Lattice'], ['help', 'Controls']];
const FIELD_NAMES = { name: 'Name', position: 'Position', velocity: 'Velocity', properTime: 'Proper time', distance: 'Distance', emissionTime: 'Emission time', dimensions: 'Rest dimensions', axes: 'Rest axes', bounds: 'Rest bounds', trajectory: 'Trajectory interval', frameVelocity: 'Velocity in your frame' };
/** @param {number|undefined} value @param {number} [digits] */
const finiteText = (value, digits = 2) => typeof value === 'number' && Number.isFinite(value) ? value.toFixed(digits) : '—';
/** @param {number[]} color */
const colorHex = color => `#${(color || [0.2, 0.7, 1]).map(n => Math.round(Math.max(0, Math.min(1, n)) * 255).toString(16).padStart(2, '0')).join('')}`;
/** @param {string} hex */
const colorArray = hex => [1, 3, 5].map(start => Number.parseInt(hex.slice(start, start + 2), 16) / 255);

/** @template {keyof HTMLElementTagNameMap} T @param {T} tag @param {string} [className] @param {string} [text] @returns {HTMLElementTagNameMap[T]} */
function node(tag, className = '', text) {
    const element = document.createElement(tag);
    if (className) element.className = className;
    if (text !== undefined) element.textContent = text;
    return element;
}
/** @param {string} text @param {string} action @param {{primary?:boolean,value?:string,title?:string}} [options] */
function button(text, action, options = {}) {
    const element = node('button', options.primary ? 'observer-button observer-button-primary' : 'observer-button', text);
    element.type = 'button';
    element.dataset.observerAction = action;
    if (options.value !== undefined) element.dataset.value = options.value;
    if (options.title) element.title = options.title;
    return element;
}
/** @param {HTMLElement} host @param {string} title @param {string} [description] */
function section(host, title, description) {
    const block = node('section', 'observer-section');
    block.append(node('h3', 'observer-section-title', title));
    if (description) block.append(node('p', 'observer-description', description));
    host.append(block);
    return block;
}
/** @param {HTMLElement} host @param {(Node|null)[]} children */
function row(host, ...children) { const element = node('div', 'observer-actions'); element.append(...children.filter(child => child !== null)); host.append(element); return element; }
/** @param {HTMLElement} host @param {string} label @param {any} value @param {FieldOptions} [options] @returns {HTMLInputElement|HTMLSelectElement} */
function field(host, label, value, { type = 'number', min, max, step = 'any', options, ...data } = {}) {
    const wrap = node('label', type === 'checkbox' ? 'observer-field observer-field-toggle' : 'observer-field');
    const labelNode = node('span', 'observer-field-label', label);
    /** @type {HTMLInputElement|HTMLSelectElement} */
    let input;
    if (options) {
        input = node('select', 'observer-input');
        for (const option of options) {
            const item = node('option', '', option.label ?? option);
            item.value = option.id ?? option;
            input.append(item);
        }
    } else {
        input = node('input', 'observer-input'); input.type = type;
        if (min !== undefined) input.min = String(min);
        if (max !== undefined) input.max = String(max);
        if (type === 'number' || type === 'range') input.step = String(step);
    }
    if (type === 'checkbox' && input instanceof HTMLInputElement) input.checked = !!value;
    else input.value = String(value ?? '');
    input.setAttribute('aria-label', label);
    for (const [key, datum] of Object.entries(data)) input.dataset[key] = String(datum);
    if (type === 'checkbox') wrap.append(input, labelNode); else wrap.append(labelNode, input);
    host.append(wrap);
    return input;
}
/** @param {HTMLElement} host @param {string} label @param {number[]} values @param {string} dataKey @param {string} path @param {{degrees?:boolean,min?:number,max?:number}} [options] */
function vector(host, label, values, dataKey, path, { degrees = false, min, max } = {}) {
    const group = node('fieldset', 'observer-vector');
    group.append(node('legend', 'observer-field-label', label));
    ['X', 'Y', 'Z'].forEach((axis, index) => field(group, axis, degrees ? values[index] * 180 / Math.PI : values[index], {
        [dataKey]: `${path}.${index}`, degrees: degrees ? 'true' : 'false', min, max,
    }));
    host.append(group);
    return group;
}
/** @param {Record<string,any>} object @param {string} path @returns {any} */
function getPath(object, path) { return path.split('.').reduce((value, key) => value?.[key], object); }
/** @param {Record<string,any>} object @param {string} path @param {any} value */
function setPath(object, path, value) {
    const keys = path.split('.');
    const last = keys.pop();
    let current = object;
    for (const key of keys) current = current[key];
    if (last !== undefined) current[last] = value;
}
/** @template T @param {T} value @returns {T} */
function clone(value) { return structuredClone(value); }
/** @param {unknown} error */
function errorText(error) { return error instanceof Error ? error.message : String(error); }
/** @param {HTMLElement} host @param {string} selector @returns {HTMLInputElement|null} */
function inputAt(host, selector) { return /** @type {HTMLInputElement|null} */ (host.querySelector(selector)); }
/** @param {HTMLElement} host @param {string} selector @returns {HTMLInputElement[]} */
function inputsAt(host, selector) { return Array.from(/** @type {NodeListOf<HTMLInputElement>} */ (host.querySelectorAll(selector))); }

/**
 * @param {{host:HTMLElement,onCommand:(command:any)=>any,onSetting:(key:string,value:any)=>any,onAction:(action:string,payload?:any)=>any}} options
 */
export function createObserverUI({ host, onCommand, onSetting, onAction }) {
    const scope = new LifetimeScope();
    const element = node('div', 'observer-ui');
    element.dataset.observerUi = '';
    const head = node('header', 'observer-head');
    const identity = node('div', 'observer-identity');
    identity.append(node('span', 'observer-eyebrow', 'LATTICE / OBSERVER'), node('h1', 'observer-title', 'Mind’s Eye'));
    const back = button('← Lattice Sim', 'exit'); back.dataset.observerExit = '';
    const assistant = button('JEV', 'assistant', { title: 'Open the JEV console' });
    assistant.setAttribute('aria-label', 'Open the JEV console');
    assistant.dataset.observerAssistant = '';
    head.append(identity, assistant, back);
    const badges = node('div', 'observer-badges');
    const profileBadge = node('span', 'observer-badge', 'SPECIAL RELATIVITY');
    const opticsBadge = node('span', 'observer-badge', 'ARRIVING LIGHT');
    const qualityBadge = node('span', 'observer-badge'); qualityBadge.hidden = true;
    qualityBadge.dataset.observerQuality = '';
    badges.append(profileBadge, opticsBadge, qualityBadge);
    identity.append(badges);
    const telemetry = node('div', 'observer-hud');
    telemetry.setAttribute('aria-label', 'Observer clocks and motion');
    /** @type {Record<string,HTMLElement>} */
    const hudFields = {};
    for (const [id, label] of [['coordinate', 'WORLD TIME'], ['proper', 'YOUR CLOCK'], ['beta', 'SPEED / c'], ['gamma', 'LORENTZ γ']]) {
        const entry = node('div', 'observer-hud-metric');
        hudFields[id] = node('strong', 'observer-hud-value', '0.00');
        hudFields[id].dataset.observerMetric = id;
        entry.append(node('span', 'observer-hud-label', label), hudFields[id]); telemetry.append(entry);
    }
    const target = node('div', 'observer-target');
    target.dataset.observerTarget = '';
    target.append(node('span', 'observer-eyebrow', 'IN YOUR SIGHT'));
    const targetName = node('strong', '', 'Infinite potential');
    const targetInfo = node('span', 'observer-description', 'Click to explore · aim and press E to inspect');
    const forceReadout = node('span', 'observer-description');
    forceReadout.dataset.observerForceGun = ''; forceReadout.hidden = true;
    const selectionInfo = node('span', 'observer-description'); selectionInfo.dataset.observerSelection = '';
    target.append(targetName, targetInfo, forceReadout, selectionInfo);
    const toolbar = node('nav', 'observer-toolbar');
    toolbar.setAttribute('aria-label', 'Observer tools');
    for (const [id, label] of PANELS) {
        const item = button(label, 'panel', { value: id });
        item.dataset.observerPanelTab = id; item.setAttribute('aria-expanded', 'false');
        toolbar.append(item);
    }
    const transport = node('div', 'observer-transport');
    const play = button('Pause', 'playback'); play.dataset.observerPlayback = '';
    transport.append(play, button('Undo edit', 'undo'), button('Reset view', 'reset-observer'));
    const status = node('div', 'observer-status');
    status.setAttribute('role', 'status'); status.setAttribute('aria-live', 'polite');
    status.dataset.observerStatus = '';
    const panel = node('aside', 'observer-panel');
    panel.hidden = true; panel.dataset.observerPanel = '';
    panel.setAttribute('aria-label', 'Observer controls');
    const panelHead = node('div', 'observer-panel-head');
    const panelTitle = node('h2', '', 'Objects');
    const panelClose = button('×', 'close-panel', { title: 'Close controls' }); panelClose.setAttribute('aria-label', 'Close controls');
    panelHead.append(panelTitle, panelClose);
    const panelBody = node('div', 'observer-panel-body'); panel.append(panelHead, panelBody);
    element.append(head, telemetry, target, toolbar, transport, status, panel); host.append(element);
    const phenomena = createPhenomenaInstrument();
    element.append(phenomena.element);

    /** @type {WorldSnapshot|null} */ let snapshot = null;
    /** @type {ViewState} */ let viewState = {};
    /** @type {ObserverSettings} */ let settings = clone(DEFAULT_SETTINGS);
    /** @type {PanelName|null} */ let activePanel = null;
    /** @type {string|null} */ let selectedId = null;
    /** @type {Record<string,any>|null} */ let draft = null;
    let dirty = false;
    let storageStamp = '';
    let lastStatus = '';
    let lastExternalStatus = '';
    let disposed = false;
    const entity = () => viewState.authorEntity?.id === selectedId ? viewState.authorEntity : snapshot?.entities?.find(item => item.id === selectedId);
    /** @param {string} name @param {any} [payload] */
    const action = (name, payload) => {
        try { Promise.resolve(onAction(name, payload)).catch(error => setStatus(error.message)); }
        catch (error) { setStatus(errorText(error)); }
    };
    /** @param {WorldCommand} commandValue @returns {Promise<any>} */
    const command = commandValue => {
        try { return Promise.resolve(onCommand(commandValue)).then(result => {
            if (result?.accepted === false || result?.ok === false) setStatus(result.reason || result.error || 'The world rejected this edit.');
            return result;
        }).catch(error => { setStatus(errorText(error)); return { ok: false }; }); }
        catch (error) { setStatus(errorText(error)); return Promise.resolve({ ok: false }); }
    };
    /** @param {string} message */
    function setStatus(message) {
        const next = String(message || '');
        if (next !== lastStatus) { status.textContent = next; lastStatus = next; }
    }
    /** @param {HTMLElement} block @param {string} label @param {string} key @param {FieldOptions} [options] */
    function settingField(block, label, key, options = {}) { return field(block, label, getPath(settings, key), { observerSetting: key, ...options }); }
    function renderObjects() {
        const create = section(panelBody, 'Place something here', 'Objects belong to this sandbox. Select a shape, then place it in front of you.');
        field(create, 'Geometry', 'box', { options: SHAPES, observerCreateShape: '' });
        row(create, button('＋ Place object', 'create-object', { primary: true }));
        const current = entity();
        const catalog = section(panelBody, 'World objects');
        field(catalog, 'Selected object', selectedId || '', { observerEntitySelect: '', options: [{ id: '', label: 'Choose an object…' }, ...(snapshot?.entities || []).map(item => ({ id: item.id, label: `${item.name}${item.alive ? '' : ' · deleted'}` }))] });
        if (!current) { catalog.append(node('p', 'observer-description', 'While exploring, aim the crosshair at a shape and press E. You can also choose an object above.')); return; }
        const received = viewState.selectedHit;
        if (received?.entityId === current.id) {
            const image = section(panelBody, received.historical ? 'Historical image' : 'Received observation', snapshot?.profile === 'sr' ? 'These readings describe the light reaching your eye. Author controls below expose the current preparation.' : 'The Playground shows the current scene without light delay.');
            if (received.mirrored) image.append(node('p', 'observer-description', 'Mirrored image across the plane. Editing changes the original object and its reflected image.'));
            image.append(node('p', 'observer-description', `Revision ${received.revision} · emission ${finiteText(received.emissionTime)} · surface clock ${finiteText(received.properTime)} · rest surface [${received.restPosition.map(v => finiteText(v)).join(', ')}]`));
        }
        const block = section(panelBody, current.alive ? 'Author controls · current state' : 'Deleted object · restore or duplicate', 'Edit a preview, then apply it as one author intervention. Sizes are full extents; rotations are degrees.');
        if (viewState.authorMirrored) block.append(node('p', 'observer-description', 'Mirrored image · world coordinates. Position, motion, rotation and impulses follow this selected image; its source updates with the opposite reflection.'));
        draft = {};
        for (const key of ['name', 'position', 'size', 'rotation', 'velocity', 'color', 'emission', 'mass', 'overlay']) draft[key] = clone(/** @type {Record<string,any>} */ (current)[key]);
        dirty = false;
        field(block, 'Name', current.name, { type: 'text', observerEntityField: 'name' });
        vector(block, 'Position · world units', current.position, 'observerEntityField', 'position');
        vector(block, 'Full size · world units', current.size, 'observerEntityField', 'size', { min: 0.001 });
        vector(block, 'Rotation · degrees', current.rotation, 'observerEntityField', 'rotation', { degrees: true });
        vector(block, snapshot?.profile === 'sr' ? 'Velocity · fraction of c' : 'Velocity · world units per time', current.velocity, 'observerEntityField', 'velocity', snapshot?.profile === 'sr' ? { min: -0.99, max: 0.99 } : {});
        row(block,
            field(node('div'), 'Surface color', colorHex(current.color), { type: 'color', observerEntityField: 'color' }).parentElement,
            field(node('div'), 'Emission', current.emission, { min: 0, max: 100, observerEntityField: 'emission' }).parentElement);
        field(block, 'Rest mass', current.mass, { min: 0, observerEntityField: 'mass' });
        draft.spectral = current.spectral;
        field(block, 'Emission spectrum', current.spectral, { observerEntityField: 'spectral', options: [{ id: 'white', label: 'Tinted three-line spectrum' }, { id: 'red-line', label: 'Red line · 610 nm' }, { id: 'green-line', label: 'Green line · 545 nm' }, { id: 'blue-line', label: 'Blue line · 455 nm' }] });
        if (snapshot?.profile === 'sr' && ['clock', 'beacon'].includes(current.shape)) {
            draft.properAcceleration = clone(current.properAcceleration);
            vector(block, 'Force per rest mass · point marker', current.properAcceleration, 'observerEntityField', 'properAcceleration');
        }
        if (snapshot?.profile === 'playground') {
            field(block, 'Body behavior', current.bodyType, { observerEntityField: 'bodyType', options: [{ id: 'dynamic', label: 'Dynamic body' }, { id: 'kinematic', label: 'Prescribed motion' }, { id: 'fixed', label: 'Fixed landmark' }] });
            draft.bodyType = current.bodyType;
        }
        field(block, 'Spatial overlay', current.overlay, { type: 'checkbox', observerEntityField: 'overlay' });
        row(block, button('Apply changes', 'apply-object', { primary: true }), button('Discard preview', 'discard-preview'));
        row(block, button('Duplicate', 'duplicate'), button(current.alive ? 'Delete' : 'Restore', current.alive ? 'delete' : 'restore'));
        const dynamics = section(panelBody, 'Interactions', snapshot?.profile === 'sr'
            ? 'Impulses are available in Playground physics. In SR, edit Velocity to prepare new inertial motion; the changed image arrives after its light delay.'
            : 'An impulse changes velocity immediately. Position advances when playback resumes. The default gives an upward speed change of 3 world units per time.');
        const impulseInputs = vector(dynamics, 'Impulse · world momentum', [0, current.mass * 3, 0], 'observerImpulse', 'impulse');
        const applyImpulse = button('Apply impulse', 'impulse');
        const impulseAndPlay = button('Apply impulse & play', 'impulse-play');
        const impulseUnavailable = snapshot?.profile !== 'playground' || !current.alive || current.bodyType !== 'dynamic';
        applyImpulse.disabled = impulseUnavailable; impulseAndPlay.disabled = impulseUnavailable;
        inputsAt(impulseInputs, 'input').forEach(input => { input.disabled = impulseUnavailable; });
        row(dynamics, applyImpulse, impulseAndPlay, button('Emit light pulse', 'emit-pulse'));
        if (snapshot?.profile === 'sr') row(dynamics, button('Use Playground physics', 'use-playground'));
        else if (impulseUnavailable) dynamics.append(node('p', 'observer-description', current.alive ? 'Choose Dynamic body and apply changes to enable impulses.' : 'Restore this object to enable impulses.'));
        if (snapshot?.profile === 'playground') {
            draft.angularVelocity = clone(current.angularVelocity);
            vector(dynamics, 'Angular velocity · radians per time', current.angularVelocity, 'observerEntityField', 'angularVelocity');
            const materialFields = /** @type {Array<[string,string,number,number]>} */ ([['restitution', 'Restitution', 0.6, 1], ['friction', 'Friction', 0.2, 2], ['damping', 'Drag', 0, 10]]);
            for (const [key, label, defaultValue, max] of materialFields) {
                draft[key] = /** @type {Record<string,any>} */ (current)[key] ?? defaultValue;
                field(dynamics, label, draft[key], { observerEntityField: key, min: 0, max });
            }
            field(dynamics, 'Gravity on object', current.gravity !== false, { type: 'checkbox', observerEntityField: 'gravity' });
            draft.gravity = current.gravity !== false;
            draft.collisions = current.collisions !== false;
            field(dynamics, 'Collisions on object', current.collisions !== false, { type: 'checkbox', observerEntityField: 'collisions' });
            dynamics.append(node('p', 'observer-description', 'Disable object collisions to pass through shapes and the central plane. World collision switches also apply.'));
            field(dynamics, 'Spring endpoint', '', { observerJointTarget: '', options: [{ id: '', label: 'Choose another object…' }, ...(snapshot?.entities || []).filter(item => item.alive && item.id !== selectedId).map(item => ({ id: item.id, label: item.name }))] });
            for (const [key, label, value] of /** @type {Array<[string,string,number]>} */ ([['restLength', 'Rest length', 3], ['stiffness', 'Stiffness', 1], ['damping', 'Damping', 0.1]])) field(dynamics, label, value, { observerJointField: key, min: 0 });
            row(dynamics, button('Connect spring', 'joint'));
        }
    }
    function updateWorldPhysicsControls() {
        const mode = inputAt(panelBody, '[data-observer-world-physics="gravityMode"]')?.value;
        const strength = inputAt(panelBody, '[data-observer-world-physics="gravityStrength"]');
        if (strength) strength.disabled = mode !== 'plane';
        const uniform = panelBody.querySelector('[data-observer-uniform-gravity]');
        if (uniform instanceof HTMLFieldSetElement) { uniform.hidden = mode !== 'uniform'; uniform.disabled = mode !== 'uniform'; }
    }
    function renderWorldPhysics() {
        const physics = section(panelBody, 'World gravity and collisions', 'The central plane is y = 0. Plane gravity pulls objects toward it from either side. Linked reflections share collisions and motion; your flying camera stays free.');
        field(physics, 'Physics profile', snapshot?.profile || 'sr', { observerProfile: '', options: [{ id: 'sr', label: 'Special Relativity' }, { id: 'playground', label: 'Classical Playground' }] });
        if (snapshot?.profile === 'playground') {
            field(physics, 'Gravity model', snapshot.gravityMode, { observerWorldPhysics: 'gravityMode', options: [{ id: 'plane', label: 'Toward the central plane · linked mirror world' }, { id: 'uniform', label: 'Uniform direction · ordinary world' }] });
            field(physics, 'Gravity strength', snapshot.gravityStrength, { min: 0, max: MAX_GRAVITY_STRENGTH, observerWorldPhysics: 'gravityStrength' });
            const uniform = vector(physics, 'Global gravity', snapshot.gravity, 'observerGlobalGravity', 'gravity');
            uniform.dataset.observerUniformGravity = '';
            field(physics, 'Object-to-object collisions', snapshot.objectCollisions, { type: 'checkbox', observerWorldPhysics: 'objectCollisions' });
            field(physics, 'Collisions with the central plane', snapshot.planeCollision, { type: 'checkbox', observerWorldPhysics: 'planeCollision' });
            physics.append(node('p', 'observer-description', 'Set strength to zero for weightlessness. Disable plane collisions to let objects cross the plane. Plane gravity links interactions on both sides through the same bodies. Uniform gravity uses ordinary source-world collisions.'));
            row(physics, button('Apply world physics', 'world-physics', { primary: true }));
            updateWorldPhysicsControls();
        } else {
            physics.append(node('p', 'observer-description', 'World gravity and rigid-body collisions are available in Playground. Special Relativity retains its reference model.'));
            row(physics, button('Use Playground physics', 'use-playground'));
        }
        field(physics, 'Playback speed', snapshot?.playbackSpeed ?? 1, { min: 0.05, max: 8, observerPlaybackSpeed: '' });
        field(physics, 'Unit label · experiment stays normalized', snapshot?.units ?? 'normalized (c = 1)', { type: 'text', observerUnits: '' });
        row(physics, button('Reset world', 'reset'));
    }
    function renderWorld() {
        renderWorldPhysics();
        const environment = snapshot?.environment || { preset: 'void', seed: 1, radius: 40, density: 1, spacing: 4, orientation: 0, opacity: 1, color: [0.2, 0.7, 1], animationRate: 0, anchor: 'world' };
        const block = section(panelBody, 'Surround yourself', 'A geometric setting wraps the full view. These visual shells do not change the world’s physics.');
        field(block, 'Environment', environment.preset, { observerEnvironment: 'preset', options: ENVIRONMENT_PRESETS });
        const description = node('p', 'observer-description', ENVIRONMENT_PRESETS.find(preset => preset.id === environment.preset)?.description || '');
        description.dataset.observerEnvironmentDescription = ''; block.append(description);
        const fractals = section(block, 'Fractal appearance', 'A shaded, distant background with no collision surface. Density changes its structure; seed chooses a variation. Animation follows simulation time. Set animation rate to 0 to freeze it.');
        fractals.dataset.observerFractalControls = ''; fractals.hidden = !environment.preset.startsWith('fractal-');
        settingField(fractals, 'Fractal detail · 0 fast / 1 intricate', 'fractalDetail', { min: 0, max: 1, step: 0.05 });
        fractals.append(node('p', 'observer-description', 'Detail controls the six volumetric fractals. Camera echoes repeat the image up to five times; lower detail or turn echoes off for a faster view. Shell radius and spacing affect geometric shells and the reference grid.'));
        for (const [key, label, min, max] of /** @type {Array<[string,string,number,number]>} */ ([['seed', 'Seed', 0, 2147483647], ['radius', 'Shell radius', 1, 10000], ['density', 'Density', 0, 4], ['spacing', 'Spacing', 0.1, 100], ['orientation', 'Orientation · degrees', -360, 360], ['opacity', 'Opacity', 0, 1], ['animationRate', 'Animation rate', -5, 5]])) {
            const value = /** @type {Record<string,any>} */ (environment)[key];
            field(block, label, key === 'orientation' ? value * 180 / Math.PI : value, { observerEnvironment: key, min, max, degrees: key === 'orientation' ? 'true' : 'false', step: key === 'seed' ? 1 : 'any' });
        }
        field(block, 'Shell color', colorHex(environment.color), { observerEnvironment: 'color', type: 'color' });
        field(block, 'Shell anchor', environment.anchor, { observerEnvironment: 'anchor', options: [{ id: 'world', label: 'World origin' }, { id: 'camera', label: 'Follow observer' }] });
        const mirror = section(panelBody, 'Both sides of the plane', 'Every object is reflected across the central grid, above to below and below to above. Inspect either image to control the shared object. Hiding reflections changes only the view.');
        settingField(mirror, 'Mirror objects across the plane', 'mirrorWorld', { type: 'checkbox' });
        const echoes = section(panelBody, 'Camera echoes', 'Overlapping camera views form a background billboard. Each nested view fades; recursion ends after the selected depth.');
        settingField(echoes, 'Camera echoes', 'feedbackEnabled', { type: 'checkbox' });
        settingField(echoes, 'Overlapping camera layers', 'feedbackLayers', { min: 1, max: 3, step: 1 });
        settingField(echoes, 'Camera fractal depth', 'feedbackDepth', { min: 3, max: 5, step: 1 });
        settingField(echoes, 'Echo opacity', 'feedbackStrength', { min: 0, max: 1, step: 0.05 });
        settingField(echoes, 'Echo resolution scale', 'feedbackScale', { min: 0.25, max: 1, step: 0.05 });
        const echoQuality = node('p', 'observer-description', echoQualityText());
        echoQuality.dataset.observerEchoQuality = ''; echoes.append(echoQuality);
        if (snapshot?.profile === 'sr') {
            const history = section(panelBody, 'Look through recorded time', 'Scrubbing pauses the world. Editing a historical state creates a new branch. Return to the present to resume without branching.');
            field(history, 'History cursor · world time', snapshot.scrubTime ?? snapshot.time, { type: 'range', min: snapshot.historyStart, max: snapshot.time, step: 0.01, observerScrub: '' });
            row(history, button('Return to present', 'present'));
        }
    }
    function renderForceGun() {
        const block = section(panelBody, 'Hold, swing and release', 'Click the scene to capture the mouse, then hold the left button on a dynamic object to pull and drag it. Hold the right button to push it away while steering. Move your aim to guide the tether; scroll while holding to change its reach. Release to let the object continue with its momentum.');
        settingField(block, 'Enable force gun', 'forceGunEnabled', { type: 'checkbox' });
        settingField(block, 'Force sensitivity', 'forceGunSensitivity', { options: [{ id: 'delicate', label: 'Delicate · soft tether' }, { id: 'normal', label: 'Balanced · everyday objects' }, { id: 'strong', label: 'Strong · heavy objects' }] });
        settingField(block, 'Force multiplier', 'forceGunMultiplier', { min: 0.1, max: 10, step: 0.1 });
        block.append(node('p', 'observer-description', 'The same force accelerates a light object more than a heavy one. These controls adjust spring strength, damping and the force limit. The live readout uses your world’s mass and force units.'));
        block.append(node('p', 'observer-description', 'Tether effort: cyan at 0%, yellow at 50%, orange at 75%, red at 100%. A dashed magenta line and LIMIT EXCEEDED mean the requested force exceeds the available strength. Ease the pull or increase strength. The tether stays attached and recovers as the demand falls.'));
        if (snapshot?.profile !== 'playground') {
            const model = section(panelBody, 'Available in Playground', 'This tool uses classical rigid-body physics. SR observations retain their relativistic reference model; arbitrary forced rigid bodies are not supported there.');
            row(model, button('Use Playground physics', 'use-playground', { primary: true }));
        }
        const behavior = section(panelBody, 'While holding');
        behavior.append(node('p', 'observer-description', 'A successful grab starts playback, even if the world was paused. Collisions, gravity and off-center rotation remain active. Either mirrored image controls the same body. Escape, inspecting, pausing or leaving the workspace releases the tether. Press E to edit mass and other properties.'));
    }
    function renderCamera() {
        const move = section(panelBody, 'Your viewpoint', 'Look up and hold forward to fly up; look down to dive. Free flight crosses the reference plane. Enable ground-plane movement below for level travel.');
        settingField(move, 'Scroll zoom speed · units per notch', 'scrollZoomSpeed', { min: 0.001, max: 1000000, step: 'any' });
        for (const [key, label, min, max, step] of /** @type {Array<[string,string,number,number,number]>} */ ([['fov', 'Field of view · degrees', 30, 120, 1], ['speed', 'Travel speed · c', 0.001, 0.99, 0.001], ['acceleration', 'Acceleration · c per time unit', 0.01, 10, 0.01], ['sensitivity', 'Look sensitivity', 0.0001, 0.02, 0.0001], ['roll', 'Camera roll · radians', -Math.PI, Math.PI, 0.01], ['renderScale', 'Resolution scale', 0.25, 1.5, 0.05], ['gridSnap', 'Author grid snap · 0 disables', 0, 10, 0.1]])) settingField(move, label, key, { min, max, step });
        for (const [key, label] of [['invertY', 'Invert vertical look'], ['grounded', 'Lock movement to ground plane'], ['worldUp', 'Keep world up'], ['reticle', 'Center reticle'], ['pauseOnInspect', 'Pause when inspecting'], ['autoQuality', 'Adapt image resolution to frame time']]) settingField(move, label, key, { type: 'checkbox' });
        const locks = section(panelBody, 'Axis constraints');
        ['X', 'Y', 'Z'].forEach((axis, index) => settingField(locks, `Lock ${axis} movement`, `axisLocks.${index}`, { type: 'checkbox' }));
        const optics = section(panelBody, 'What reaches your eye', 'Optical mode traces arriving light through world history. Simultaneous geometry is an explanatory view.');
        for (const [key, label] of [['optical', 'Retarded-time optical view'], ['doppler', 'Doppler color shift'], ['beaming', 'Relativistic intensity'], ['artisticShading', 'Artistic surface shading']]) settingField(optics, label, key, { type: 'checkbox' });
    }
    function echoQualityText() {
        const quality = viewState.rendering;
        return quality?.feedbackEnabled && quality.feedbackResolution?.[0]
            ? `Echo image: ${quality.feedbackResolution.join(' × ')} · ${quality.feedbackPasses} camera passes`
            : 'Camera echoes are off.';
    }
    function renderLayers() {
        const overlays = section(panelBody, 'Spatial readouts', 'Readouts hover near objects. Filter their visibility independently of the center reticle.');
        settingField(overlays, 'Visible overlays', 'overlayFilter', { options: [{ id: 'selected', label: 'Selected object' }, { id: 'all', label: 'All objects' }, { id: 'none', label: 'Hidden' }] });
        for (const [id, label] of Object.entries(FIELD_NAMES)) settingField(overlays, label, `overlayFields.${id}`, { type: 'checkbox' });
        const layers = section(panelBody, 'Geometry layers');
        for (const { id, label } of LAYERS) settingField(layers, label, `layers.${id}`, { type: 'checkbox' });
    }
    function renderPhenomena() {
        const frame = section(panelBody, 'Space, time and arriving light', 'These instruments use the adopted Minkowski reference model (c = 1). They illustrate the sandbox; they are not measurements of recovered lattice physics.');
        for (const id of ['lightCones', 'simultaneity', 'lightPaths', 'aberration', 'ghosts', 'pulses']) {
            const entry = LAYERS.find(layer => layer.id === id);
            const input = settingField(frame, entry?.label || id, `layers.${id}`, { type: 'checkbox' });
            if (id !== 'pulses') input.disabled = snapshot?.profile !== 'sr';
        }
        frame.append(node('p', 'observer-description', snapshot?.profile === 'sr'
            ? 'Aim at an object for its received event, Doppler factor and clock. Purple boxes compare observer-simultaneous bounds, including length contraction. Pulse fronts mark recorded emissions, not visible photon shells. The compass compares directions of distant sources.'
            : 'Relativistic instruments are unavailable in Playground. Switch the physics profile in World to Special Relativity.'));
        const waves = section(panelBody, 'A wave laboratory in space', 'Coordinate-frame analytic reference illustrations: a two-source scalar interference surface, a standing wave with fixed nodes, and transverse E/B fields. These do not interact with objects and are not a Lorentz-transformed field simulation.');
        for (const id of ['interference', 'standingWaves', 'polarization']) settingField(waves, LAYERS.find(layer => layer.id === id)?.label || id, `layers.${id}`, { type: 'checkbox' });
        for (const [key, label, min, max, step] of /** @type {Array<[string,string,number,number,number]>} */ ([
            ['waveWavelength', 'Wavelength · world units', 0.5, 12, 0.1], ['waveSeparation', 'Source separation · world units', 0, 12, 0.1],
            ['waveAmplitude', 'Wave amplitude', 0, 2, 0.05], ['wavePhase', 'Relative source phase · radians', -Math.PI, Math.PI, 0.05],
        ])) settingField(waves, label, key, { min, max, step });
        settingField(waves, 'Polarization mode', 'polarizationMode', { options: ['linear', 'circular', 'elliptical'] });
        waves.append(node('p', 'observer-description', 'The cyan board is near the origin (z = −4.5); violet standing waves are at z = −12; orange E and blue B ribbons are at x = 6. Reset view to find them. Their phase follows world time, so pausing freezes them. Opening controls pauses by default; close them to continue, or turn off “Pause when inspecting” in Camera.'));
        row(waves, button('Reset view', 'reset-observer'));
        const appearance = section(panelBody, 'Relativistic appearance', 'The optical renderer already includes light-travel delay and aberration. Color and intensity can be displayed separately. Projection FoV never changes physical velocity.');
        for (const [key, label] of [['optical', 'Retarded-time optical view'], ['doppler', 'Doppler color shift'], ['beaming', 'Relativistic intensity']]) settingField(appearance, label, key, { type: 'checkbox' });
    }
    function renderExperiments() {
        const block = section(panelBody, 'Small worlds, clear questions', 'Each preparation replaces the sandbox world. Its physics remains a declared standard SR or classical model.');
        for (const experiment of EXPERIMENTS) {
            const card = node('article', 'observer-experiment');
            card.append(node('h4', '', experiment.label), node('p', 'observer-description', experiment.description), button('Enter preparation', 'experiment', { value: experiment.id }));
            block.append(card);
        }
    }
    function renderStorage() {
        const block = section(panelBody, 'Keep this world', 'Nothing is saved automatically until you enable autosave. Named worlds are never removed to make room.');
        field(block, 'World name', 'My observer world', { type: 'text', observerSaveName: '' });
        row(block, button('Save world', 'save', { primary: true }), button('Export JSON', 'export'));
        field(block, 'Autosave every 30 seconds', !!viewState.storage?.autosave, { type: 'checkbox', observerAutosave: '' });
        const usage = viewState.storage?.usage;
        field(block, 'Storage limit · MiB', (usage?.capBytes ?? 104857600) / 1048576, { min: 1, max: 1024, step: 1, observerStorageCap: '' });
        block.append(node('p', 'observer-description', usage ? `${finiteText(usage.bytes / 1048576, 1)} / ${finiteText(usage.capBytes / 1048576, 0)} MiB local storage` : 'Local storage is opened only when needed.'));
        const file = field(block, 'Import world JSON', '', { type: 'file', observerImport: '' }); if (file instanceof HTMLInputElement) file.accept = '.json,application/json';
        const saved = section(panelBody, 'Saved worlds');
        const entries = viewState.storage?.entries || [];
        if (!entries.length) saved.append(node('p', 'observer-description', 'No local worlds saved yet.'));
        for (const entry of entries) {
            const item = node('article', 'observer-save');
            item.append(node('strong', '', entry.name), node('span', 'observer-description', `${entry.kind === 'autosave' ? 'Autosave · ' : ''}${new Date(entry.updatedAt).toLocaleString()} · ${finiteText(entry.bytes / 1024, 0)} KiB`));
            row(item, button('Load', 'load', { value: entry.id }), button('Remove', 'remove-save', { value: entry.id })); saved.append(item);
        }
        row(saved, button('Refresh', 'refresh-saves'), button('Clear saved worlds', 'clear-saves'));
    }
    function renderLattice() {
        const block = section(panelBody, 'A window into the substrate', 'This is a passive connection to the retained lattice owner. Sandbox edits never write back to the microscopic state.');
        block.append(node('p', 'observer-description', 'The lattice is paused while Mind’s Eye is open. These cached observations retain their source and tick; return to Lattice Sim to continue.'));
        const info = viewState.lattice || {};
        for (const [key, label] of [['ownerIdentity', 'Owner'], ['backend', 'Backend'], ['sourceId', 'Source'], ['lawId', 'Law'], ['sourceEpoch', 'Source epoch'], ['epoch', 'Epoch'], ['sampleTick', 'Completed sample tick'], ['status', 'Status'], ['fieldTokens', 'Field tokens'], ['relationTokens', 'Relation tokens'], ['incidence', 'Signed incidence'], ['negative', 'Negative sites'], ['zero', 'Zero sites'], ['positive', 'Positive sites']]) {
            const value = info[key] ?? (key === 'status' ? info.reason : 'Unavailable');
            const metric = node('div', 'observer-readout');
            const output = node('strong', '', String(value)); output.dataset.observerLatticeField = String(key);
            metric.append(node('span', '', String(label)), output); block.append(metric);
        }
        block.append(node('p', 'observer-description', 'Lattice data carries its own source, epoch and tick. The SR sandbox is an effective model; this view does not establish recovered relativity from the substrate.'));
    }
    function renderHelp() {
        const block = section(panelBody, 'Move through your frame', 'Click the scene to capture the pointer. Escape releases it. Hold forward to fly wherever you look, above or below the plane. Space and Ctrl rise and descend. Scroll up to zoom forward along your view; scroll down to pull back. Travel has no scene-distance limit. Scroll zoom relocates the camera and restarts its clock without changing FoV. World controls offer evolving fractals, a mirrored world and bounded camera echoes. Click a binding and press a key to change it.');
        block.append(node('p', 'observer-description', 'In Playground, hold left mouse to pull a dynamic object or right mouse to push and steer it. While held, scrolling adjusts tether reach. Release either button to let go. E inspects the shape under the crosshair; Force gun controls its sensitivity.'));
        for (const [key, value] of Object.entries(settings.bindings || DEFAULT_BINDINGS)) field(block, key.replace(/[A-Z]/g, letter => ` ${letter.toLowerCase()}`), value, { type: 'text', observerBinding: key });
        row(block, button('Restore keybindings', 'reset-bindings'));
        const meaning = section(panelBody, 'Author controls');
        meaning.append(node('p', 'observer-description', 'Select an object to preview its geometry and motion, then apply one edit. Undo restores a checkpoint in a new timeline branch. Earlier optical history belongs to the previous branch.'));
    }
    function renderPanel() {
        panelBody.replaceChildren();
        if (!activePanel) return;
        ({ objects: renderObjects, forcegun: renderForceGun, world: renderWorld, camera: renderCamera, layers: renderLayers, phenomena: renderPhenomena, experiments: renderExperiments, storage: renderStorage, lattice: renderLattice, help: renderHelp })[activePanel]();
    }
    function discardPreview() { if (dirty) action('preview', { id: selectedId, patch: null }); dirty = false; draft = null; }
    /** @param {string} name */
    function openPanel(name) {
        if (!PANELS.some(([id]) => id === name) || disposed) return;
        discardPreview(); activePanel = /** @type {PanelName} */ (name); panel.hidden = false;
        panel.dataset.observerPanel = name;
        panelTitle.textContent = PANELS.find(([id]) => id === name)?.[1] || name;
        element.classList.add('observer-has-panel');
        toolbar.querySelectorAll('[data-observer-panel-tab]').forEach(tab => { tab.setAttribute('aria-expanded', String(tab.getAttribute('data-observer-panel-tab') === name)); });
        renderPanel(); action('panel-open', name);
        if (name === 'storage') action('refresh-saves');
        panelClose.focus({ preventScroll: true });
    }
    function closePanel() {
        if (!activePanel) return;
        discardPreview(); const previous = activePanel; activePanel = null; panel.hidden = true;
        element.classList.remove('observer-has-panel');
        toolbar.querySelectorAll('[data-observer-panel-tab]').forEach(tab => tab.setAttribute('aria-expanded', 'false'));
        action('panel-close');
        /** @type {HTMLElement|null} */ (toolbar.querySelector(`[data-observer-panel-tab="${previous}"]`))?.focus({ preventScroll: true });
    }
    /** @param {HTMLInputElement|HTMLSelectElement} input @returns {any} */
    function readValue(input) {
        if (input.type === 'checkbox' && input instanceof HTMLInputElement) return input.checked;
        if (input.type === 'number' || input.type === 'range') {
            if (!input.checkValidity()) { input.reportValidity(); return undefined; }
            const value = Number(input.value);
            return input.value !== '' && Number.isFinite(value) ? (input.dataset.degrees === 'true' ? value * Math.PI / 180 : value) : undefined;
        }
        if (input.type === 'color') return colorArray(input.value);
        return input.value;
    }
    /** @param {HTMLInputElement} input */
    function editPreview(input) {
        if (!draft || !input.dataset.observerEntityField) return;
        const value = readValue(input); if (value === undefined) return;
        setPath(draft, input.dataset.observerEntityField, value); dirty = true;
        action('preview', { id: selectedId, patch: clone(draft) });
    }
    scope.on(element, 'input', event => {
        const input = event.target;
        if (input instanceof HTMLInputElement) editPreview(input);
    });
    scope.on(element, 'change', async event => {
        const input = event.target;
        if (!(input instanceof HTMLInputElement || input instanceof HTMLSelectElement)) return;
        const value = readValue(input);
        if (value === undefined && !input.dataset.observerImport) return;
        if (input.dataset.observerSetting) {
            const path = input.dataset.observerSetting;
            const key = path.split('.')[0];
            if (path.includes('.')) { const updated = clone(settings[key]); setPath({ [key]: updated }, path, value); settings[key] = updated; onSetting(key, updated); }
            else { settings[key] = value; onSetting(key, value); }
        } else if (input.dataset.observerEntityField && draft) { setPath(draft, input.dataset.observerEntityField, value); dirty = true; action('preview', { id: selectedId, patch: clone(draft) }); }
        else if (input.hasAttribute('data-observer-environment')) {
            const key = input.dataset.observerEnvironment || 'preset';
            const patch = { [key]: value };
            if (key === 'preset' && String(value).startsWith('fractal-') && snapshot?.environment.animationRate === 0 && !window.matchMedia('(prefers-reduced-motion: reduce)').matches) patch.animationRate = 0.35;
            const result = await command({ type: 'environment', patch });
            if (result?.ok && activePanel === 'world' && 'animationRate' in patch) {
                const rateInput = inputAt(panelBody, '[data-observer-environment="animationRate"]');
                if (rateInput) rateInput.value = String(patch.animationRate);
            }
        }
        else if (input.hasAttribute('data-observer-world-physics')) updateWorldPhysicsControls();
        else if (input.hasAttribute('data-observer-profile')) { command({ type: 'profile', profile: value }); }
        else if (input.hasAttribute('data-observer-entity-select')) { discardPreview(); selectedId = value || null; action('select', { id: selectedId }); renderPanel(); }
        else if (input.hasAttribute('data-observer-autosave')) action('autosave', value);
        else if (input.hasAttribute('data-observer-storage-cap')) action('storage-cap', value);
        else if (input.hasAttribute('data-observer-units')) command({ type: 'settings', patch: { units: value } });
        else if (input.hasAttribute('data-observer-playback-speed')) command({ type: 'settings', patch: { playbackSpeed: value } });
        else if (input.hasAttribute('data-observer-scrub')) command({ type: 'scrub', time: value });
        else if (input.hasAttribute('data-observer-binding')) {
            const binding = input.dataset.observerBinding || '';
            if (Object.entries(settings.bindings).some(([key, code]) => key !== binding && code === value)) { setStatus('That key is already assigned. Choose a different key.'); input.value = settings.bindings[binding]; return; }
            settings.bindings = { ...settings.bindings, [binding]: value }; onSetting('bindings', settings.bindings);
        } else if (input instanceof HTMLInputElement && input.hasAttribute('data-observer-import') && input.files?.[0]) {
            const file = input.files[0];
            if (file.size > 32 * 1024 * 1024) { setStatus('World imports must be smaller than 32 MiB.'); return; }
            try { action('import', { text: await file.text() }); } catch (error) { setStatus(errorText(error)); }
        }
    });
    scope.on(element, 'keydown', event => {
        if (event.target instanceof HTMLInputElement && event.target.hasAttribute('data-observer-binding')) {
            if (event.code === 'Tab') return;
            event.preventDefault(); event.stopPropagation(); event.target.value = event.code;
            event.target.dispatchEvent(new Event('change', { bubbles: true }));
        } else if (event.key === 'Escape' && activePanel) { event.preventDefault(); event.stopPropagation(); closePanel(); }
    });
    scope.on(element, 'click', event => {
        const control = event.target.closest('[data-observer-action]');
        if (!control || !element.contains(control)) return;
        const type = control.dataset.observerAction;
        if (type === 'panel') { if (activePanel === control.dataset.value) closePanel(); else openPanel(control.dataset.value); }
        else if (type === 'close-panel') closePanel();
        else if (type === 'playback') command({ type: snapshot?.playing ? 'pause' : 'play' });
        else if (type === 'create-object') {
            const shape = inputAt(panelBody, '[data-observer-create-shape]')?.value || 'box';
            const observer = snapshot?.observer || { position: [0, 1.6, 0], yaw: 0, pitch: 0 };
            const direction = [-Math.sin(observer.yaw) * Math.cos(observer.pitch), Math.sin(observer.pitch), -Math.cos(observer.yaw) * Math.cos(observer.pitch)];
            command({ type: 'create', entity: { name: SHAPES.find(item => item.id === shape)?.label || shape, shape, position: observer.position.map((v, i) => v + direction[i] * 4), size: [1, 1, 1], rotation: [0, 0, 0], velocity: [0, 0, 0], color: [0.22, 0.78, 1], emission: shape === 'beacon' || shape === 'pulse' ? 2 : 0.1, mass: 1, overlay: true } });
        } else if (type === 'apply-object' && selectedId && draft) {
            const invalid = inputsAt(panelBody, '[data-observer-entity-field]').find(input => !input.checkValidity());
            if (invalid) { invalid.reportValidity(); return; }
            const editingId = selectedId;
            command({ type: 'update', id: selectedId, patch: clone(draft) }).then(result => {
                if (result && result.ok !== false && result.accepted !== false && selectedId === editingId) { discardPreview(); renderPanel(); }
            });
        } else if (type === 'discard-preview') { discardPreview(); renderPanel(); }
        else if (['delete', 'restore', 'duplicate'].includes(type) && selectedId) { discardPreview(); command({ type, id: selectedId }); }
        else if (type === 'use-playground') { discardPreview(); command({ type: 'profile', profile: 'playground' }); }
        else if ((type === 'impulse' || type === 'impulse-play') && selectedId) {
            if (dirty) { setStatus('Apply or discard the object preview before applying an impulse.'); return; }
            const impulse = inputsAt(panelBody, '[data-observer-impulse]').map(input => Number(input.value));
            if (impulse.length !== 3 || !impulse.every(Number.isFinite) || !impulse.some(value => value !== 0)) { setStatus('Set a nonzero impulse to change momentum.'); return; }
            command({ type: 'impulse', id: selectedId, impulse }).then(async result => {
                if (!result?.ok || type !== 'impulse-play') return;
                const resumed = await command({ type: 'play' });
                if (resumed?.ok) { closePanel(); setStatus('Impulse applied · simulation running.'); }
            });
        } else if (type === 'joint' && selectedId) {
            const b = inputAt(panelBody, '[data-observer-joint-target]')?.value;
            if (!b) { setStatus('Choose a second object for this spring.'); return; }
            /** @type {WorldCommand} */
            const joint = { type: 'joint', a: selectedId, b };
            inputsAt(panelBody, '[data-observer-joint-field]').forEach(input => { joint[input.dataset.observerJointField || 'damping'] = Number(input.value); });
            command(joint);
        } else if (type === 'emit-pulse' && selectedId) command({ type: 'pulse', id: selectedId });
        else if (type === 'world-physics') {
            /** @type {Record<string,unknown>} */ const patch = {};
            for (const input of panelBody.querySelectorAll('[data-observer-world-physics]')) {
                if (!(input instanceof HTMLInputElement || input instanceof HTMLSelectElement)) continue;
                const value = readValue(input);
                if (value === undefined) return;
                patch[input.dataset.observerWorldPhysics || 'gravityMode'] = value;
            }
            if (patch.gravityMode === 'uniform') {
                const gravity = inputsAt(panelBody, '[data-observer-global-gravity]').map(input => readValue(input));
                if (gravity.some(value => value === undefined)) return;
                patch.gravity = gravity;
            }
            command({ type: 'world-physics', patch }).then(result => {
                if (result?.ok) setStatus(`World physics applied.${snapshot?.playing ? '' : ' Paused: press Play to see motion.'}`);
            });
        }
        else if (type === 'present') command({ type: 'scrub', time: null });
        else if (type === 'experiment') action('experiment', { id: control.dataset.value });
        else if (type === 'save') action('save', { name: inputAt(panelBody, '[data-observer-save-name]')?.value });
        else if (type === 'load' || type === 'remove-save') action(type, { id: control.dataset.value });
        else if (type === 'undo' || type === 'reset') { discardPreview(); command({ type }); }
        else if (type === 'clear-saves') {
            if (control.dataset.confirmed === 'true') { action(type); control.dataset.confirmed = 'false'; control.textContent = 'Clear saved worlds'; }
            else { control.dataset.confirmed = 'true'; control.textContent = 'Confirm: remove all local saves'; setStatus('Click again to remove all named saves and autosaves. The active world remains open.'); }
        } else action(type);
    });

    /** @param {WorldSnapshot} nextSnapshot @param {ViewState} [nextViewState] */
    function update(nextSnapshot, nextViewState = {}) {
        if (disposed) return;
        const previousEntity = entity(); const previousProfile = snapshot?.profile;
        const previousWorldPhysics = JSON.stringify([snapshot?.gravityMode, snapshot?.gravityStrength, snapshot?.gravity, snapshot?.objectCollisions, snapshot?.planeCollision]);
        const previousAuthorMirrored = viewState.authorMirrored;
        snapshot = nextSnapshot; viewState = nextViewState;
        settings = { ...clone(DEFAULT_SETTINGS), ...nextViewState.settings,
            layers: { ...DEFAULT_SETTINGS.layers, ...nextViewState.settings?.layers },
            bindings: { ...DEFAULT_BINDINGS, ...nextViewState.settings?.bindings },
            overlayFields: { ...DEFAULT_SETTINGS.overlayFields, ...nextViewState.settings?.overlayFields } };
        const nextSelected = nextViewState.selectedId ?? null;
        const selectionChanged = selectedId !== nextSelected || previousAuthorMirrored !== viewState.authorMirrored;
        if (selectionChanged) { discardPreview(); selectedId = nextSelected; }
        const beta = Math.hypot(...(snapshot?.observer?.velocity || [0, 0, 0]));
        hudFields.coordinate.textContent = finiteText(snapshot?.time);
        hudFields.proper.textContent = finiteText(snapshot?.observer?.properTime);
        hudFields.beta.textContent = finiteText(beta, 3);
        hudFields.gamma.textContent = beta < 1 ? finiteText(1 / Math.sqrt(1 - beta * beta), 3) : '—';
        if (hudFields.gamma.parentElement) hudFields.gamma.parentElement.hidden = snapshot?.profile === 'playground';
        if (hudFields.beta.previousElementSibling) hudFields.beta.previousElementSibling.textContent = snapshot?.profile === 'playground' ? 'SPEED' : 'SPEED / c';
        profileBadge.textContent = snapshot?.profile === 'playground' ? 'CLASSICAL PLAYGROUND' : 'SPECIAL RELATIVITY';
        opticsBadge.textContent = snapshot?.profile === 'playground' ? 'INSTANTANEOUS SCENE' : settings.optical ? 'ARRIVING LIGHT' : 'SIMULTANEOUS GEOMETRY';
        const quality = viewState.rendering;
        const echoesReduced = !!quality?.feedbackEnabled && !!quality.feedbackResolution?.[0] && quality.feedbackResolution[0] < Math.floor(quality.internalResolution[0] * (settings.feedbackScale ?? 0.6)) - 1;
        qualityBadge.hidden = !quality || (quality.internalScale >= 0.999 && !echoesReduced);
        if (quality) qualityBadge.textContent = `${quality.internalResolution.join(' × ')} · ${Math.round(quality.internalScale * 100)}% IMAGE${echoesReduced ? ` · ECHOES ${quality.feedbackResolution?.join(' × ')}` : ''}`;
        const echoQuality = panelBody.querySelector('[data-observer-echo-quality]');
        if (echoQuality) echoQuality.textContent = echoQualityText();
        const environmentDescription = panelBody.querySelector('[data-observer-environment-description]');
        if (environmentDescription) environmentDescription.textContent = ENVIRONMENT_PRESETS.find(preset => preset.id === snapshot?.environment.preset)?.description || '';
        const fractalControls = /** @type {HTMLElement|null} */ (panelBody.querySelector('[data-observer-fractal-controls]'));
        if (fractalControls) fractalControls.hidden = !snapshot?.environment.preset.startsWith('fractal-');
        play.textContent = snapshot?.playing ? 'Pause' : 'Play';
        const hitId = viewState.hit?.id ?? viewState.hit?.entityId;
        const aimed = snapshot?.entities?.find(item => item.id === hitId);
        targetName.textContent = aimed?.name || 'Infinite potential';
        const gunReady = snapshot?.profile === 'playground' && settings.forceGunEnabled !== false;
        targetInfo.textContent = aimed ? `${aimed.shape} · ${viewState.hit?.mirrored ? 'mirrored image · ' : ''}${snapshot?.profile === 'playground' ? 'current scene' : settings.optical ? 'observed light' : 'simultaneous geometry'} · E to inspect${gunReady && aimed.alive && aimed.bodyType === 'dynamic' ? ' · hold LMB to pull / RMB to push' : ''}` : 'Click to explore · aim and press E to inspect';
        const gun = viewState.forceGun;
        forceReadout.hidden = !gun?.active;
        forceReadout.textContent = gun?.active ? `${gun.mode === 'push' ? 'PUSH' : 'PULL'} · ${gun.name || 'Object'} · ${gun.effort ? `effort ${gun.effort.percent}%${gun.effort.overloaded ? ' · LIMIT EXCEEDED' : ''}` : 'measuring effort'} · mass ${finiteText(gun.mass)} · force ${finiteText(gun.force)} / ${finiteText(gun.cap)} · reach ${finiteText(gun.depth)} · ${gun.sensitivity || settings.forceGunSensitivity} ×${finiteText(gun.multiplier ?? settings.forceGunMultiplier, 1)}` : '';
        selectionInfo.textContent = entity() ? `Selected: ${entity()?.name}${entity()?.alive ? '' : ' · historical object'} · Objects to edit` : '';
        if (viewState.status !== undefined && viewState.status !== lastExternalStatus) { lastExternalStatus = viewState.status; setStatus(viewState.status); }
        const nextStamp = JSON.stringify(viewState.storage || {});
        const inspectorChanged = previousEntity?.revision !== entity()?.revision && !dirty && !(document.activeElement instanceof HTMLInputElement && panel.contains(document.activeElement));
        const worldPhysicsChanged = previousWorldPhysics !== JSON.stringify([snapshot?.gravityMode, snapshot?.gravityStrength, snapshot?.gravity, snapshot?.objectCollisions, snapshot?.planeCollision]);
        const refresh = (activePanel === 'objects' && (selectionChanged || previousProfile !== snapshot?.profile || previousEntity?.alive !== entity()?.alive || inspectorChanged)) || (activePanel === 'storage' && nextStamp !== storageStamp) || (activePanel === 'forcegun' && previousProfile !== snapshot?.profile) || (activePanel === 'world' && (previousProfile !== snapshot?.profile || worldPhysicsChanged));
        storageStamp = nextStamp;
        if (refresh) renderPanel();
        if (activePanel === 'phenomena' && previousProfile !== snapshot.profile) renderPanel();
        phenomena.update(snapshot, { ...settings, cameraOverride: viewState.cameraOverride, selectedId }, viewState.hit ?? null);
        if (activePanel === 'lattice') panelBody.querySelectorAll('[data-observer-lattice-field]').forEach(output => {
            const key = output.getAttribute('data-observer-lattice-field') || '';
            const info = viewState.lattice || {};
            output.textContent = String(key === 'status' ? (info.status ?? info.reason ?? (info.available ? 'Observation available' : 'Waiting for observation')) : (info[key] ?? '—'));
        });
    }
    return { element, update, setStatus, openPanel, closePanel, dispose() { if (disposed) return; discardPreview(); disposed = true; scope.dispose(); element.remove(); } };
}

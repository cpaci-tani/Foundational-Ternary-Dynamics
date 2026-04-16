import { createTickAccumulator } from '../../scale-utils.js';

export const FIELD_TOGGLE_KEYS = [
    'showEField',
    'showBField',
    'showPoynting',
    'showDivField',
    'showFluxLines',
    'showForceEM',
    'showForceGravity',
    'showForceStrong',
    'showForceWeak',
    'showDualSubstrate',
    'showChirality',
    'showLight',
    'showDarkMatterHalo',
    'showDampingZones',
    'showGenesisIsosurface',
    'showConfinement',
];

export const FORCE_FIELD_KEYS = new Set([
    'showForceEM',
    'showForceGravity',
    'showForceStrong',
    'showForceWeak',
]);

function createFieldFlags() {
    return {
        showEField: false,
        showBField: false,
        showPoynting: false,
        showDivField: false,
        showFluxLines: false,
        showForceEM: false,
        showForceGravity: false,
        showForceStrong: false,
        showForceWeak: false,
        showDualSubstrate: false,
        showChirality: false,
        showLight: false,
        showDarkMatterHalo: false,
        showDampingZones: false,
        showGenesisIsosurface: false,
        showConfinement: false,
    };
}

const state = {
    currentScenarioId: 'flux-pulse',
    fieldFlags: createFieldFlags(),
    fieldFrame: 0,
    fieldNeedsUpdate: false,
    anyFieldActive: false,
    forceStyle: 'arrows',
    fluxMock: null,
    useFluxMock: false,
    latticeNeedsUpload: true,
    tickAccumulator: createTickAccumulator(),
    fieldParticleBuf: [],
    dualLVecs: null,
    dualRVecs: null,
    chiralValues: null,
    weakValues: null,
    weakVectors: null,
};

export function getScale0State() {
    return state;
}

export function recomputeAnyFieldActive() {
    state.anyFieldActive = FIELD_TOGGLE_KEYS.some((key) => !!state.fieldFlags[key]);
    return state.anyFieldActive;
}

export function resetFieldFlags() {
    state.fieldFlags = createFieldFlags();
    state.fieldNeedsUpdate = false;
    recomputeAnyFieldActive();
}

export function setFieldToggle(key, value) {
    if (!Object.prototype.hasOwnProperty.call(state.fieldFlags, key)) return;
    state.fieldFlags[key] = !!value;
    if (value) state.fieldNeedsUpdate = true;
    recomputeAnyFieldActive();
}

export function getFieldStateSnapshot() {
    return {
        ...state.fieldFlags,
        anyFieldActive: state.anyFieldActive,
        fieldNeedsUpdate: state.fieldNeedsUpdate,
        fluxMock: state.fluxMock,
    };
}

export function setForceStyle(style) {
    state.forceStyle = style;
    state.fieldNeedsUpdate = true;
}

export function setFluxMock(mock, useMock = false) {
    state.fluxMock = mock;
    state.useFluxMock = !!useMock;
}

export function clearFluxMock() {
    state.fluxMock = null;
    state.useFluxMock = false;
}

export function setLatticeNeedsUpload(value = true) {
    state.latticeNeedsUpload = !!value;
}

export function markFieldDirty() {
    state.fieldNeedsUpdate = true;
}

export function resetFrameState() {
    state.fieldFrame = 0;
    state.fieldNeedsUpdate = false;
    state.latticeNeedsUpload = true;
    state.tickAccumulator.reset();
}

export function setCurrentScenarioId(id) {
    state.currentScenarioId = id || 'flux-pulse';
}

// The blocking bootstrap owns both first paint and the runtime preference law.
const state = window.FTDPanelMountState;

export const isValidMount = state.isValidMount;
export const migratePanelMount = state.migratePanelMount;
export const readPanelMount = state.readPanelMount;
export const writePanelMount = state.writePanelMount;
export const getValidMounts = state.getValidMounts;
export const getDefaultMount = state.getDefaultMount;
export const getSideMountMinWidth = state.getSideMountMinWidth;
export const resolveEffectiveMount = state.resolveEffectiveMount;

// The page import map owns module versions so every consumer shares one instance.
export { DiagnosticsPanelComponent, initDiagnosticsPanel } from './diagnostics-panel/component.js';
export { ChartsPanelComponent, initChartsPanel } from './charts-panel/component.js';
export { LagrangianPanelComponent, initLagrangianPanel } from './lagrangian-panel/component.js';
export { ScenePanelComponent, initScenePanel } from './scene-panel/component.js';
export { TelemetryGridPanelComponent, initTelemetryGridPanel } from './telemetry-grid/component.js';
export {
    InteractionHierarchyPanelComponent,
    ParticleLogPanelComponent,
    initInteractionHierarchyPanel,
    initParticleLogPanel,
} from '../../scales/scale1/ui/particle-log/component.js';

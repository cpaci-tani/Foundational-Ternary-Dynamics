/**
 * Telemetry module barrel — hub (single write path) + demand gating + registries.
 * CONTRACTS.md §5: collectors write hub; panels/overlays read hub snapshots only.
 */

export { telemetryHub, RingBuffer } from '../telemetry-hub.js';
export {
    getScale0TelemetryDemand,
    collectScale0OnDemand,
    collectScale0Unconditional,
} from './demand.js';
export { SCALE0_GRID_CHANNELS } from './registry/scale0-grid-channels.js';
export {
    readScale0DiagAudit,
    readScale0TotalEnergy,
    readScale0WaveEnergy,
    readScale0FieldEnergy,
} from './scale0-read.js';

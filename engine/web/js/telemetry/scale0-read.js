/**
 * Scale-0 telemetry read helpers — prefer hub snapshots, fall back to bridge.
 * CONTRACTS.md §5: panels/overlays read hub first; bridge only when hub empty.
 */

import { telemetryHub } from '../telemetry-hub.js';

/**
 * @param {object|null} bridge - active Scale-0 bridge (optional fallback)
 * @returns {{ diag: object|null, audit: object|null }}
 */
export function readScale0DiagAudit(bridge = null) {
    const diag = telemetryHub.s0?.diag
        ?? bridge?.getDiagnostics?.()
        ?? null;
    const audit = telemetryHub.s0?.audit
        ?? bridge?.getEnergyAudit?.()
        ?? null;
    return { diag, audit };
}

/** Canonical physical total energy (field + wave + particle KE). */
export function readScale0TotalEnergy(diag, audit) {
    if (audit != null && Number.isFinite(audit.totalEnergy)) return audit.totalEnergy;
    return diag?.totalEnergy ?? 0;
}

/** Wave (kinetic) energy — ½Σ|wave_vel|². */
export function readScale0WaveEnergy(diag, audit) {
    if (audit != null && Number.isFinite(audit.waveEnergy)) return audit.waveEnergy;
    return diag?.totalWaveEnergy ?? 0;
}

/** Field energy — ½Σ|J|². */
export function readScale0FieldEnergy(audit) {
    return audit?.fieldEnergy ?? 0;
}

#pragma once
/**
 * Energy ledger computation — moved out of render_bridge.cpp in the
 * 2026-04-18 R3 refactor. The `EnergyLedger` struct itself stays in
 * render_bridge.h (it's part of the public API surface); only the
 * per-tick update body moves here.
 */

namespace ftd {

class RenderBridge;

// Updates rb.energy_ledger_ with the current-tick energy snapshot, summing the
// host AoS shadow (rb.voxels_). Correct only when that shadow is fresh: CPU, or
// non-interactive GPU (which syncs device->host every tick). Called at the end
// of every such tick.
void update_energy_ledger_cpu(RenderBridge& rb);

// Interactive-GPU variant. The host AoS shadow is deliberately stale there (the
// 469 B/site device->host mirror is deferred), so update_energy_ledger_cpu
// would sum zeros and report E_curr = 0. This sources the identical energy
// channels (field, wave, kinetic, and strong when enabled) from the compact
// device-side rb.energy_audit() reduction instead — a few scalars D2H, not the
// full field transfer. See energy_ledger_compute.cpp for the verified
// convention mapping (V_cell = 1 makes the audit channels equal the host sums).
void update_energy_ledger_from_audit(RenderBridge& rb);

// Shared tick-over-tick bookkeeping given an already-computed E_total. Both the
// host-shadow and device-audit paths funnel through this so E_curr, dE_dt,
// drift_frac, residual, expected_rate, the injection/dissipation accumulators,
// and the update counter are computed identically regardless of energy source.
void commit_energy_ledger(RenderBridge& rb, double E_total);

}  // namespace ftd

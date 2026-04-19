#pragma once
/**
 * Energy ledger computation — moved out of render_bridge.cpp in the
 * 2026-04-18 R3 refactor. The `EnergyLedger` struct itself stays in
 * render_bridge.h (it's part of the public API surface); only the
 * per-tick update body moves here.
 */

namespace ftd {

class RenderBridge;

// Updates rb.energy_ledger_ with the current-tick energy snapshot.
// Called at the end of every tick (CPU and GPU paths alike).
void update_energy_ledger_cpu(RenderBridge& rb);

}  // namespace ftd

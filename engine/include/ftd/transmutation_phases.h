#pragma once
/**
 * Transmutation phases — optional, toggle-gated physics.
 *
 * Extracted from render_bridge.cpp in the 2026-04-18 R2 refactor.
 * These are free functions taking `RenderBridge&` because they mutate
 * voxels AND use the bridge's RNG / toggles / lattice state. RenderBridge
 * exposes a friend declaration so we can touch rng_, uniform_,
 * next_particle_id_ without further surface-area changes.
 */

namespace ftd {

class RenderBridge;

// Rule 6 (weak): stress-driven polarity flip. Flips v.state on manifested
// sites whose field stress exceeds WEAK_THRESHOLD. In dual-substrate mode
// uses left-stress only (parity-violating).
void weak_transmutation_cpu(RenderBridge& rb);

// Scale 0: edge-based gauge link staple relaxation
void relax_su2_links_cpu(RenderBridge& rb, double dt, double beta);
void relax_su3_links_cpu(RenderBridge& rb, double dt, double beta);

// Rule 8: FTD Schwarzschild-like proper time integration.
//   f = 1 − L²   (Schwarzschild factor from the latency field)
//   dτ/dt = √(f² − |v|²) / √f
void accumulate_proper_time(RenderBridge& rb);

// Rule 2b: correlated ±1 pair production from high-|J| void voxels.
// Probability p = 1 − exp(−(|J|−K_GENESIS)/K_MANIFEST); partner placed at
// the first empty face-neighbour; pair_id shared so the two can be
// tracked as a correlated pair.
void pair_production_cpu(RenderBridge& rb);

// Rule 7: near-equilateral same-sign triad detection → lock all three.
void triad_binding_cpu(RenderBridge& rb);

}  // namespace ftd

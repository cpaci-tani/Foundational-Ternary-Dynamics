#pragma once
// Runtime toggles for the logic-first engine.
// 8 toggles correspond to the 6 axiomatic rules + Gauss constraint + Poisson solver.

namespace ftd {

struct TermToggles {
    bool wave_propagation = true;   // phase_read: Laplacian wave equation
    bool coupling = true;           // phase_read: g_c * grad(s) source term
    bool damping = true;            // phase_write: energy dissipation
    bool genesis = true;            // phase_write: manifestation + evaporation
    bool gauss_projection = true;   // gauss_project: div(J) = s constraint
    bool forces = true;             // phase_forces: field-mediated EM + gravity
    bool gravity = true;            // phase_forces: F = G_N·∇ρ gravitational force
    bool poisson_coulomb = true;    // Poisson-based Coulomb (Phase 3). false = legacy grad(div J)
    bool movement = true;           // phase_movement: velocity integration + collisions
    bool lorentz_force = true;      // phase_forces: F = α·s·(v×B) magnetic force
    bool selective_damping = false; // phase_write: damp only near particles (false = legacy uniform)
    bool larmor_radiation = false; // phase_write: acceleration-dependent damping at particle sites
    bool dual_substrate = false;   // dual-substrate mode: J_L, J_R independent fields (Paper: 2026)
    bool color_forces = false;     // phase_forces: SU(3)-inspired color-dependent pairwise force
    bool weak_transmutation = false; // tick: stress-threshold polarity flip (+1 ↔ -1) [CLAUDE.md §6.5]
    bool strong_force = false;     // phase_forces: Yukawa short-range nuclear force [CLAUDE.md §6.4]
    bool triad_binding = false;    // tick: detect 3-particle triads, set locked=true [CLAUDE.md §8.1]
    bool pair_production = false;  // genesis: correlated +1/-1 pairs from high-flux void [CLAUDE.md §4.1]
    bool exchange_force = false;   // phase_forces: Pauli exclusion repulsion (same-spin) [CLAUDE.md §11]

    void enable_all() {
        wave_propagation = coupling = damping = genesis = true;
        gauss_projection = forces = gravity = poisson_coulomb = movement = true;
        lorentz_force = true;
        selective_damping = false;  // Keep legacy damping by default
        larmor_radiation = false;  // Keep uniform damping by default
        dual_substrate = false;    // Keep single-substrate by default
        color_forces = false;      // Keep color off by default
        weak_transmutation = false; // Keep weak transmutation off by default
        strong_force = false;      // Keep strong force off by default
        triad_binding = false;     // Keep triad binding off by default
        pair_production = false;   // Keep pair production off by default
        exchange_force = false;    // Keep exchange force off by default
    }

    void disable_all() {
        wave_propagation = coupling = damping = genesis = false;
        gauss_projection = forces = gravity = poisson_coulomb = movement = false;
        lorentz_force = false;
        selective_damping = false;
        larmor_radiation = false;
        dual_substrate = false;
        color_forces = false;
        weak_transmutation = false;
        strong_force = false;
        triad_binding = false;
        pair_production = false;
        exchange_force = false;
    }
};

}  // namespace ftd

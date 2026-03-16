#pragma once
// Runtime toggles for the logic-first engine.
// 20 toggles: 10 core (logic-derived, default ON) + 10 extensions (default OFF).

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
    bool selective_damping = true;  // phase_write: damp only near particles (true = vacuum EM lossless)
    bool larmor_radiation = false; // phase_write: acceleration-dependent damping at particle sites
    bool dual_substrate = true;    // dual-substrate mode: J_L, J_R independent fields (Paper: 2026)
    bool color_forces = false;     // phase_forces: SU(3)-inspired color-dependent pairwise force
    bool weak_transmutation = true;  // tick: chirality/stress polarity flip (+1 ↔ -1) [CLAUDE.md §6.5]
    bool strong_force = false;     // phase_forces: Yukawa short-range nuclear force [CLAUDE.md §6.4]
    bool triad_binding = false;    // tick: detect 3-particle triads, set locked=true [CLAUDE.md §8.1]
    bool pair_production = false;  // genesis: correlated +1/-1 pairs from high-flux void [CLAUDE.md §4.1]
    bool exchange_force = false;   // phase_forces: Pauli exclusion repulsion (same-spin) [CLAUDE.md §11]
    bool latency_field = false;    // Poisson-based latency field ∇²L = 4πGρ (gravity potential)

    void enable_all() {
        wave_propagation = coupling = damping = genesis = true;
        gauss_projection = forces = gravity = poisson_coulomb = movement = true;
        lorentz_force = true;
        selective_damping = true;   // Vacuum EM waves propagate losslessly
        larmor_radiation = false;  // Keep uniform damping by default
        dual_substrate = true;     // SU(2) weak gauge requires dual substrate
        color_forces = false;      // Keep color off by default
        weak_transmutation = true;  // Chirality-based weak transmutation
        strong_force = false;      // Keep strong force off by default
        triad_binding = false;     // Keep triad binding off by default
        pair_production = false;   // Keep pair production off by default
        exchange_force = false;    // Keep exchange force off by default
        latency_field = false;     // Keep latency field off by default
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
        latency_field = false;
    }
};

}  // namespace ftd

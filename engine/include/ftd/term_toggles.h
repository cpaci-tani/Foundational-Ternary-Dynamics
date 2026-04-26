#pragma once
// Runtime toggles for the logic-first engine.
// 20 toggles: 10 core (logic-derived, default ON) + 10 extensions (default OFF).

#include <string>

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
    bool exact_dual_gauss = false; // gauss_project: exact dual-cell face-flux projection (Phase 1 Electrodynamics)
    bool emergent_forces = false;  // EFT mode: force from flux gradient (no Poisson), alpha = G_C²
    bool langevin = false;         // Stochastic thermalization: OU process on wave_vel with (gamma, T)

    // Langevin thermalization parameters (used only when langevin == true).
    // Continuous form:  dv = -gamma v dt + sqrt(2 gamma T) dW, per component per voxel.
    // Discrete form (per tick, dt=1):
    //     v <- (1 - gamma) v  +  sqrt(2 gamma T) * eta,   eta ~ N(0, I)
    // Equilibrium:  <|v|^2>_voxel = 3 T  (three components of wave_vel).
    // Autocorrelation time: tau ~ 1/gamma ticks.
    // Keep gamma in [0, 0.5] for discrete stability; T sets the energy scale.
    // See docs/theory/10_eft_program/DERIV_LANGEVIN_THERMALIZATION.md (future).
    double langevin_T     = 0.0;
    double langevin_gamma = 0.01;
    unsigned int langevin_seed = 1;  // RNG seed for reproducibility

    // Phase H (Apr 2026): explicit coupling constant in the Gauss law source.
    // gauss_project_cpu uses source = div(J) - coulomb_charge_coupling * s.
    // Default 1.0 preserves geometric Coulomb (Phase G theorem:
    // alpha_r = 2 r G_L(r) -> 1/(2 pi) at continuum). To test whether FTD
    // emergent dynamics can reproduce alpha_ref = 1/137, set this to
    // sqrt(2 pi alpha_ref) ~ 0.2141 (engine convention) or
    // sqrt(4 pi alpha_ref) ~ 0.3028 (classical convention). See
    // docs/theory/10_eft_program/DERIV_EMERGENT_COULOMB_GEOMETRIC.md Section 7.
    double coulomb_charge_coupling = 1.0;

    // ARCH-3 (2026-04-25): when true, RenderBridge::tick() THROWS on the
    // first validate() failure instead of printing to stderr and continuing.
    // Off by default to preserve legacy behaviour; tests/campaigns that need
    // hard guarantees can enable it. The throw type is std::logic_error.
    bool strict_validation = false;

    // Validates known dependency constraints between toggles.
    // Returns true if the combination is valid.
    // If err != nullptr, appends a human-readable description of each violation.
    bool validate(std::string* err = nullptr) const {
        std::string msg;
        if (weak_transmutation && !dual_substrate)
            msg += "weak_transmutation requires dual_substrate (operates on J_L/J_R)\n";
        if (lorentz_force && !forces)
            msg += "lorentz_force requires forces\n";
        if (triad_binding && !color_forces)
            msg += "triad_binding requires color_forces\n";
        if (exchange_force && !poisson_coulomb)
            msg += "exchange_force requires poisson_coulomb\n";
        if (emergent_forces && poisson_coulomb)
            msg += "emergent_forces and poisson_coulomb are mutually exclusive\n";
        if (larmor_radiation && !damping)
            msg += "larmor_radiation requires damping\n";
        if (latency_field && !gravity)
            msg += "latency_field requires gravity\n";
        if (err) *err = msg;
        return msg.empty();
    }

    // F2 (callstack audit 2026-04-17): warn about toggles whose
    // implementation lives only on the GPU path. Called from
    // RenderBridge::tick() when `use_gpu_` is false — returns a
    // non-empty string when any GPU-only toggle is set, so the caller
    // can std::cerr it.
    //
    // Post-fix: `pair_production` and `triad_binding` are now ported
    // (pair_production_cpu / triad_binding_cpu). The two still
    // GPU-only are `strong_force` and `exchange_force`. If either
    // gets ported, drop it from this check.
    std::string cpu_runtime_warnings() const {
        std::string msg;
        if (strong_force)
            msg += "strong_force has no CPU implementation — toggle is a no-op on CPU builds\n";
        if (exchange_force)
            msg += "exchange_force has no CPU implementation — toggle is a no-op on CPU builds\n";
        return msg;
    }

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

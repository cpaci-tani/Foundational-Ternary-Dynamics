/**
 * EMERGENT PHYSICS BENCHMARK — Reverse-Engineering Alpha
 *
 * Can the fine structure constant be MEASURED from lattice field dynamics
 * rather than read from a hardcoded constant?
 *
 * The wave equation coupling term is: delta_J += G_C * grad(s)
 * where G_C = sqrt(alpha). This embeds alpha in the field dynamics.
 * If the field self-consistently encodes alpha, we should be able to
 * extract it from:
 *   E1: Self-energy of a single charge
 *   E2: Two-charge interaction potential V(r)
 *   E3: Emergent force (no explicit Poisson force)
 *   E4: Emergent bound state formation
 *   E5: Coupling-free null baseline
 *
 * ALL experiments run with forces=false, poisson_coulomb=false.
 * Only the wave equation + Gauss constraint + coupling are active.
 * No explicit force computation. Any force is EMERGENT.
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

// Helper: configure a RenderBridge for pure field dynamics (no explicit forces)
void configure_bare_lattice(ftd::RenderBridge& rb) {
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis = false;        // No spontaneous creation
    rb.toggles.damping = false;        // No energy loss
    rb.toggles.forces = false;         // NO explicit Poisson force
    rb.toggles.poisson_coulomb = false; // NO Coulomb solver
    rb.toggles.gravity = false;
    rb.toggles.movement = false;       // Particles stay put (locked behavior)
    rb.toggles.lorentz_force = false;
    rb.toggles.color_forces = false;
    rb.toggles.selective_damping = false;
    rb.toggles.larmor_radiation = false;
    rb.toggles.dual_substrate = false;
    rb.toggles.weak_transmutation = false;
    rb.toggles.latency_field = false;
}

// ================================================================
// E1: Self-Energy of a Single Charge
//
// Place +1 at center, let field equilibrate, measure E_field.
// The self-energy encodes alpha through the coupling strength.
// ================================================================
void experiment_self_energy(int L, int ticks) {
    std::cerr << "  E1: Self-energy (L=" << L << ", " << ticks << " ticks)\n";
    const int mid = L / 2;

    ftd::RenderBridge rb(L);
    configure_bare_lattice(rb);

    // Inject a single +1 charge with minimal flux dressing
    rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B * 0.1});
    rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

    // Track energy over time to find equilibrium
    double E_prev = 0;
    int stable_ticks = 0;
    double E_equil = 0;

    for (int t = 0; t < ticks; ++t) {
        rb.tick();

        if (t % 10 == 0) {
            auto audit = rb.energy_audit();
            double E = audit.field_energy;

            if (t > 50 && std::abs(E - E_prev) < E * 0.001) {
                stable_ticks++;
                if (stable_ticks >= 3) {
                    E_equil = E;
                }
            } else {
                stable_ticks = 0;
            }
            E_prev = E;
        }
    }

    if (E_equil == 0) {
        auto audit = rb.energy_audit();
        E_equil = audit.field_energy;
    }

    // Also measure field energy of empty lattice as baseline
    ftd::RenderBridge rb_empty(L);
    configure_bare_lattice(rb_empty);
    rb_empty.run(ticks);
    double E_empty = rb_empty.energy_audit().field_energy;

    double E_self = E_equil - E_empty;

    // Measure the flux profile: |J| vs distance from charge
    std::cerr << "    E_field=" << E_equil << " E_empty=" << E_empty
              << " E_self=" << E_self << "\n";

    // Radial flux profile
    for (int r = 1; r <= std::min(L / 3, 12); ++r) {
        int idx = rb.lattice().index(mid + r, mid, mid);
        double rho = rb.voxels()[idx].density();
        double div_j = rb.divergence_flux(idx);
        std::cout << "self_energy_profile," << L << ","
                  << r << "," << std::setprecision(8) << rho << ","
                  << std::setprecision(8) << div_j << ",0,0\n";
    }

    std::cout << "self_energy," << L << ","
              << std::setprecision(10) << E_self << ","
              << std::setprecision(10) << ftd::ALPHA << ","
              << 0 << ",0," << ticks << "\n";
}

// ================================================================
// E2: Two-Charge Interaction Potential V(r)
//
// Place +1 and -1 at separation r. Measure total field energy.
// V(r) = E_pair(r) - 2*E_self
// Theory: V(r) = -alpha/r → alpha = -V(r)*r
// ================================================================
void experiment_interaction_potential(int L, int ticks) {
    std::cerr << "  E2: Interaction potential (L=" << L << ")\n";
    const int mid = L / 2;

    // First measure single-charge self-energy
    double E_self_pos = 0, E_self_neg = 0;
    for (int sign = 0; sign < 2; ++sign) {
        int s = (sign == 0) ? +1 : -1;
        ftd::RenderBridge rb(L);
        configure_bare_lattice(rb);
        rb.inject_particle(mid, mid, mid, s, {0, 0, ftd::K_B * 0.1 * s});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
        rb.run(ticks);
        double E = rb.energy_audit().field_energy;
        if (sign == 0) E_self_pos = E; else E_self_neg = E;
    }

    double E_2self = E_self_pos + E_self_neg;
    std::cerr << "    E_self(+)=" << E_self_pos << " E_self(-)=" << E_self_neg
              << " 2*E_self=" << E_2self << "\n";

    // Now measure pair energy at multiple separations
    std::vector<int> separations;
    for (int r = 4; r <= std::min(L / 3, 14); r += 2) separations.push_back(r);

    std::vector<double> r_vals, V_vals;

    for (int r : separations) {
        ftd::RenderBridge rb(L);
        configure_bare_lattice(rb);

        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B * 0.1});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        rb.inject_particle(mid + r, mid, mid, -1, {0, 0, -ftd::K_B * 0.1});
        rb.voxels()[rb.lattice().index(mid + r, mid, mid)].locked = true;

        rb.run(ticks);
        double E_pair = rb.energy_audit().field_energy;
        double V = E_pair - E_2self;

        r_vals.push_back(static_cast<double>(r));
        V_vals.push_back(V);

        // Extract alpha_r = -V(r) * r (should be constant = alpha)
        double alpha_r = -V * r;

        std::cout << "potential," << L << ","
                  << r << "," << std::setprecision(10) << V << ","
                  << std::setprecision(10) << alpha_r << ","
                  << std::setprecision(10) << ftd::ALPHA << ",0\n";

        std::cerr << "    r=" << r << " E_pair=" << E_pair << " V=" << V
                  << " alpha_r=" << alpha_r << "\n";
    }

    // Fit: V(r) = -alpha_fit / r → alpha_fit = -slope of V vs 1/r
    if (r_vals.size() >= 3) {
        // Linear regression: V = a + b*(1/r), where b = -alpha
        int n = static_cast<int>(r_vals.size());
        double sx = 0, sy = 0, sxx = 0, sxy = 0;
        for (int i = 0; i < n; ++i) {
            double x = 1.0 / r_vals[i];
            double y = V_vals[i];
            sx += x; sy += y; sxx += x * x; sxy += x * y;
        }
        double denom = n * sxx - sx * sx;
        double b = (std::abs(denom) > 1e-30) ? (n * sxy - sx * sy) / denom : 0;
        double alpha_fit = -b;

        double err = 100.0 * std::abs(alpha_fit - ftd::ALPHA) / ftd::ALPHA;

        std::cout << "alpha_from_potential," << L << ","
                  << std::setprecision(10) << alpha_fit << ","
                  << std::setprecision(10) << ftd::ALPHA << ","
                  << std::setprecision(4) << err << ",0,0\n";

        std::cerr << "    *** alpha_fit = " << alpha_fit
                  << " (theory: " << ftd::ALPHA << ", err: " << err << "%) ***\n";
    }
}

// ================================================================
// E3: Emergent Force Test
//
// Two locked charges establish field. Unlock one, measure acceleration.
// Any acceleration is PURELY from field dynamics, not Poisson force.
// ================================================================
void experiment_emergent_force(int L, int ticks) {
    std::cerr << "  E3: Emergent force (L=" << L << ")\n";
    const int mid = L / 2;

    std::vector<int> separations = {5, 7, 9};
    std::vector<double> r_vals, a_vals;

    for (int r : separations) {
        if (mid + r >= L - 1) continue;

        ftd::RenderBridge rb(L);
        configure_bare_lattice(rb);
        rb.toggles.movement = true;  // Allow particle motion

        // Source: locked +1
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B * 0.1});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        // Probe: locked -1 (will unlock after field establishes)
        int px = mid + r;
        rb.inject_particle(px, mid, mid, -1, {0, 0, -ftd::K_B * 0.1});
        rb.voxels()[rb.lattice().index(px, mid, mid)].locked = true;

        // Let field equilibrate
        rb.run(ticks);

        // Record probe state BEFORE unlocking
        double vx_before = rb.voxels()[rb.lattice().index(px, mid, mid)].velocity.x;

        // Unlock probe and run 1 tick
        rb.voxels()[rb.lattice().index(px, mid, mid)].locked = false;
        rb.tick();

        // Measure velocity change = acceleration = emergent force
        double vx_after = rb.voxels()[rb.lattice().index(px, mid, mid)].velocity.x;
        double accel = vx_after - vx_before;

        r_vals.push_back(static_cast<double>(r));
        a_vals.push_back(accel);

        // If accel < 0 (toward source), it's an attractive emergent force
        double alpha_emergent = std::abs(accel) * 4.0 * ftd::PI * r * r;

        std::cout << "emergent_force," << L << ","
                  << r << "," << std::setprecision(10) << accel << ","
                  << std::setprecision(10) << alpha_emergent << ","
                  << std::setprecision(10) << ftd::ALPHA << ",0\n";

        std::cerr << "    r=" << r << " accel=" << accel
                  << " alpha_emergent=" << alpha_emergent << "\n";
    }

    // Is there any force at all?
    bool any_force = false;
    for (double a : a_vals) {
        if (std::abs(a) > 1e-15) any_force = true;
    }
    std::cout << "emergent_force_exists," << L << ","
              << (any_force ? 1 : 0) << ",1,0,0,0\n";
    std::cerr << "    Emergent force detected: " << (any_force ? "YES" : "NO") << "\n";
}

// ================================================================
// E4: Emergent Bound State
//
// Place +1 and -1 with no explicit forces. Does the system evolve
// toward a bound configuration?
// ================================================================
void experiment_bound_state(int L, int ticks) {
    std::cerr << "  E4: Emergent bound state (L=" << L << ")\n";
    const int mid = L / 2;
    const int r0 = std::max(4, L / 6);

    ftd::RenderBridge rb(L);
    configure_bare_lattice(rb);
    rb.toggles.movement = true;  // Let particles move

    // +1 at center (locked to provide reference)
    rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B * 0.1});
    rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

    // -1 free at distance r0
    rb.inject_particle(mid + r0, mid, mid, -1, {0, 0, -ftd::K_B * 0.1});
    // NOT locked — free to move

    // Track separation over time
    std::vector<double> separations;
    double prev_r = r0;

    for (int t = 0; t < ticks; ++t) {
        rb.tick();

        if (t % 5 == 0) {
            // Find the -1 particle
            double min_dist = 1e30;
            for (int i = 0; i < rb.lattice().total_sites(); ++i) {
                if (rb.voxels()[i].state == -1) {
                    auto c = rb.lattice().coord(i);
                    double dx = c.x - mid;
                    double dy = c.y - mid;
                    double dz = c.z - mid;
                    double r = std::sqrt(dx*dx + dy*dy + dz*dz);
                    if (r < min_dist) min_dist = r;
                }
            }
            if (min_dist < 1e20) {
                separations.push_back(min_dist);
                prev_r = min_dist;
            }
        }
    }

    // Did the electron approach? (attracted by field coupling)
    double r_initial = separations.empty() ? r0 : separations.front();
    double r_final = separations.empty() ? r0 : separations.back();
    bool approached = (r_final < r_initial - 0.5);
    bool bound = (r_final < r0 * 0.7);

    std::cout << "bound_state," << L << ","
              << std::setprecision(4) << r_initial << ","
              << std::setprecision(4) << r_final << ","
              << (approached ? 1 : 0) << "," << (bound ? 1 : 0) << ",0\n";

    // Output trajectory
    for (size_t i = 0; i < separations.size() && i < 20; ++i) {
        std::cout << "bound_trajectory," << L << ","
                  << i * 5 << "," << std::setprecision(4) << separations[i]
                  << ",0,0,0\n";
    }

    std::cerr << "    r_initial=" << r_initial << " r_final=" << r_final
              << " approached=" << (approached ? "YES" : "NO")
              << " bound=" << (bound ? "YES" : "NO") << "\n";
}

// ================================================================
// E5: Coupling-Free Null Baseline
//
// Same as E1-E3 but with coupling=false (G_C=0).
// Should produce ZERO self-energy, zero force — the null hypothesis.
// ================================================================
void experiment_null_baseline(int L, int ticks) {
    std::cerr << "  E5: Null baseline (coupling=false, L=" << L << ")\n";
    const int mid = L / 2;

    // Self-energy with coupling OFF
    ftd::RenderBridge rb(L);
    configure_bare_lattice(rb);
    rb.toggles.coupling = false;  // KEY: disable G_C coupling

    rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B * 0.1});
    rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

    rb.run(ticks);
    double E_no_coupling = rb.energy_audit().field_energy;

    // Emergent force with coupling OFF
    ftd::RenderBridge rb2(L);
    configure_bare_lattice(rb2);
    rb2.toggles.coupling = false;
    rb2.toggles.movement = true;

    rb2.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B * 0.1});
    rb2.voxels()[rb2.lattice().index(mid, mid, mid)].locked = true;
    rb2.inject_particle(mid + 7, mid, mid, -1, {0, 0, -ftd::K_B * 0.1});
    rb2.voxels()[rb2.lattice().index(mid + 7, mid, mid)].locked = true;
    rb2.run(ticks);

    double vx_before = rb2.voxels()[rb2.lattice().index(mid + 7, mid, mid)].velocity.x;
    rb2.voxels()[rb2.lattice().index(mid + 7, mid, mid)].locked = false;
    rb2.tick();
    double vx_after = rb2.voxels()[rb2.lattice().index(mid + 7, mid, mid)].velocity.x;
    double accel_null = vx_after - vx_before;

    std::cout << "null_self_energy," << L << ","
              << std::setprecision(10) << E_no_coupling << ",0,0,0,0\n";
    std::cout << "null_force," << L << ","
              << std::setprecision(10) << accel_null << ",0,0,0,0\n";

    std::cerr << "    E(no coupling)=" << E_no_coupling
              << "  F(no coupling)=" << accel_null << "\n";
    std::cerr << "    Null hypothesis: both should be ~0\n";
}

// ================================================================
// E6: EFT Emergent Force via emergent_forces toggle
//
// Uses the NEW engine mode where force comes from flux gradient
// instead of Poisson solver. Alpha should emerge as G_C².
// ================================================================
void experiment_eft_force(int L, int ticks) {
    std::cerr << "  E6: EFT emergent force (L=" << L << ")\n";
    const int mid = L / 2;

    std::vector<int> separations = {5, 7, 9, 11};
    std::vector<double> r_vals, f_vals;

    for (int r : separations) {
        if (mid + r >= L - 2) continue;

        ftd::RenderBridge rb(L);
        rb.toggles.wave_propagation = true;
        rb.toggles.coupling = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis = false;
        rb.toggles.damping = false;
        rb.toggles.forces = true;           // Enable force phase
        rb.toggles.poisson_coulomb = true;   // Will be skipped by emergent_forces
        rb.toggles.emergent_forces = true;   // KEY: use flux gradient force
        rb.toggles.gravity = false;
        rb.toggles.movement = true;
        rb.toggles.lorentz_force = false;

        // Source: locked +1
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        // Probe: locked -1 (unlock after field equilibrates)
        int px = mid + r;
        rb.inject_particle(px, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(px, mid, mid)].locked = true;

        // Equilibrate field
        rb.run(ticks);

        // Unlock probe and measure 1-tick acceleration
        double vx_before = rb.voxels()[rb.lattice().index(px, mid, mid)].velocity.x;
        rb.voxels()[rb.lattice().index(px, mid, mid)].locked = false;
        rb.tick();
        double vx_after = rb.voxels()[rb.lattice().index(px, mid, mid)].velocity.x;

        double accel = vx_after - vx_before;
        double alpha_eft = std::abs(accel) * 4.0 * ftd::PI * r * r;

        r_vals.push_back(static_cast<double>(r));
        f_vals.push_back(accel);

        std::cout << "eft_force," << L << ","
                  << r << "," << std::setprecision(10) << accel << ","
                  << std::setprecision(10) << alpha_eft << ","
                  << std::setprecision(10) << ftd::ALPHA_EFT << ",0\n";

        std::cerr << "    r=" << r << " accel=" << accel
                  << " alpha_eft=" << alpha_eft
                  << " (theory: " << ftd::ALPHA_EFT << ")\n";
    }

    bool any_force = false;
    for (double f : f_vals) {
        if (std::abs(f) > 1e-15) any_force = true;
    }
    std::cout << "eft_force_exists," << L << ","
              << (any_force ? 1 : 0) << ",1,0,0,0\n";
    std::cerr << "    EFT force detected: " << (any_force ? "YES" : "NO") << "\n";
    std::cerr << "    ALPHA_EFT = G_C² = " << ftd::ALPHA_EFT << "\n";
}

// ================================================================
// Main
// ================================================================
int main(int argc, char* argv[]) {
    int L = (argc > 1) ? std::atoi(argv[1]) : 32;
    int ticks = (argc > 2) ? std::atoi(argv[2]) : 200;

    std::cout << "experiment,lattice_size,col1,col2,col3,col4,col5\n";

    auto t0 = std::chrono::high_resolution_clock::now();
    std::cerr << "=== EMERGENT ALPHA EXPERIMENTS: L=" << L << ", ticks=" << ticks << " ===\n";
    std::cerr << "  Alpha (hardcoded):  " << ftd::ALPHA << " = 1/" << 1.0/ftd::ALPHA << "\n";
    std::cerr << "  ALPHA_EFT (G_C²):   " << ftd::ALPHA_EFT << " = G_C*G_C\n";
    std::cerr << "  G_C (coupling):     " << ftd::G_C << " = sqrt(alpha)\n";
    std::cerr << "  Match: " << (std::abs(ftd::ALPHA - ftd::ALPHA_EFT) < 1e-12 ? "EXACT" : "MISMATCH") << "\n\n";

    experiment_self_energy(L, ticks);
    experiment_interaction_potential(L, ticks);
    experiment_emergent_force(L, ticks);
    experiment_bound_state(L, ticks);
    experiment_null_baseline(L, ticks);
    experiment_eft_force(L, ticks);

    auto t1 = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(t1 - t0).count();
    std::cerr << "\nCompleted in " << std::fixed << std::setprecision(1) << elapsed << "s\n";

    return 0;
}

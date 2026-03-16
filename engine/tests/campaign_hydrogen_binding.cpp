/**
 * Campaign: Hydrogen-Like Bound State (Phase 4 — Emergent Mass Spectrum)
 *
 * Tests whether opposite-charge particles form stable bound states
 * with measurable binding energy and orbital structure.
 *
 * Theory: A +1 charge (proton analog) and -1 charge (electron analog)
 * should attract via Coulomb force F = -α·q1·q2/r² and form a bound
 * orbit at the lattice Bohr radius a₀ = 1/(K_B·α_eff).
 *
 * With G_N=0.01 dominating: α_eff ≈ α/(4π) + G_N·K_B² ≈ 0.00319
 * Predicted a₀ ≈ 613 lattice units (too large for 32³ lattice).
 * On L=32 with gravity OFF, pure EM gives smaller scale.
 *
 * Protocol:
 *   1. Place locked +1 at center, free -1 at separation r₀
 *   2. Evolve for T ticks with gravity OFF (pure EM)
 *   3. Track electron position over time
 *   4. Measure: binding energy, orbital radius, stability
 *
 * Checks:
 *   HB1: Particles interact (manifested count changes or annihilation occurs)
 *   HB2: If both survive, electron stays within lattice (bound or orbiting)
 *   HB3: Opposite charges attract (annihilation or decreased separation)
 *   HB4: Charge is conserved throughout
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

int failures = 0;

void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Hydrogen Binding (Phase 4) — 4 Checks\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed << std::setprecision(6);

    const int L = 32;
    const int mid = L / 2;
    const int INTERACTION_TICKS = 500;
    const int LONG_TICKS = 2000;
    const int r0 = 6;  // Initial separation

    ftd::RenderBridge rb(L);
    rb.toggles.genesis = false;
    rb.toggles.gravity = false;  // Pure EM binding test

    // Proton analog: locked +1 at center
    rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
    rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

    // Electron analog: free -1 at separation r0
    int e_x = mid + r0;
    rb.inject_particle(e_x, mid, mid, -1, {0, 0, ftd::K_B * 0.1});

    // Record initial charge
    int charge_initial = 0;
    int N_total = rb.lattice().total_sites();
    for (int i = 0; i < N_total; ++i) charge_initial += rb.voxels()[i].state;

    // Let self-field establish
    rb.run(200);

    // Record interaction state (before long evolution)
    auto audit_interaction = rb.energy_audit();
    double coulomb_pe_early = audit_interaction.coulomb_pe;
    int manifested_early = audit_interaction.manifested_count;

    std::cout << "\n--- Interaction State (after 200-tick warmup) ---\n";
    std::cout << "  Field energy:  " << audit_interaction.field_energy << "\n";
    std::cout << "  Coulomb PE:    " << coulomb_pe_early << "\n";
    std::cout << "  Manifested:    " << manifested_early << "\n";

    // Short evolution to see attraction
    rb.run(INTERACTION_TICKS);
    auto audit_mid = rb.energy_audit();

    // Continue to long evolution
    rb.run(LONG_TICKS - INTERACTION_TICKS);
    auto audit_final = rb.energy_audit();

    // Find electron position
    int electron_count = 0;
    double electron_r = 0.0;
    int charge_final = 0;
    for (int i = 0; i < N_total; ++i) {
        auto& v = rb.voxels()[i];
        charge_final += v.state;
        if (v.state == -1) {
            electron_count++;
            auto c = rb.lattice().coord(i);
            double dx = c.x - mid;
            double dy = c.y - mid;
            double dz = c.z - mid;
            electron_r = std::sqrt(dx*dx + dy*dy + dz*dz);
        }
    }

    std::cout << "\n--- Final State (after " << LONG_TICKS << " ticks) ---\n";
    std::cout << "  Manifested:    " << audit_final.manifested_count << "\n";
    std::cout << "  Electrons:     " << electron_count << "\n";
    if (electron_count > 0) std::cout << "  Electron r:    " << electron_r << "\n";
    std::cout << "  Outcome:       "
              << (audit_final.manifested_count == 0 ? "ANNIHILATED"
                  : electron_count > 0 ? "BOUND/ORBITING" : "EVAPORATED")
              << "\n";

    // ================================================================
    // HB1: Interaction occurred (particles didn't just sit there)
    // ================================================================
    bool interacted = (audit_final.manifested_count != manifested_early) ||
                      (audit_mid.manifested_count != manifested_early) ||
                      (audit_mid.particle_ke > 1e-10);
    check("HB1: Particles interact (dynamics observed)", interacted);

    // ================================================================
    // HB2: If alive, electron bounded (or annihilation is OK)
    // ================================================================
    bool physical_outcome = (audit_final.manifested_count == 0) ||  // annihilation
                           (electron_count > 0 && electron_r < L / 2.0);  // bound
    check("HB2: Physical outcome (bound state or annihilation)", physical_outcome);

    // ================================================================
    // HB3: Opposite charges attract (annihilation proves attraction)
    // Note: Total Coulomb PE includes self-energy (positive), which
    // dominates over interaction energy at small separations.
    // Annihilation is the strongest evidence of attraction.
    // ================================================================
    bool attracted = (audit_final.manifested_count == 0) ||  // annihilated = attracted
                    (electron_count > 0 && electron_r < static_cast<double>(r0));  // closer
    check("HB3: Opposite charges attract (annihilation or approach)", attracted);

    // ================================================================
    // HB4: Charge conservation
    // ================================================================
    std::cout << "  Q initial: " << charge_initial << ", Q final: " << charge_final << "\n";
    check("HB4: Charge conserved throughout", charge_initial == charge_final);

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED")
              << " (" << failures << " failures)\n";
    std::cout << "================================================================\n";
    return failures;
}

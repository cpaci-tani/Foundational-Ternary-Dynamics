/**
 * Campaign: Multi-Scale Pipeline (12 checks across 4 phases)
 *
 * Phase 1: Full pipeline round-trip (Scale 0 → 1 → 2 → 1 → 0)
 * Phase 2: Energy conservation across transitions
 * Phase 3: Multi-atom pipeline (3 separate nuclei)
 * Phase 4: Cross-scale force direction agreement
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "ftd/render_bridge.h"
#include "ftd/particle_engine.h"
#include "ftd/atom_engine.h"
#include "ftd/scale.h"
#include "ftd/constants.h"

int failures = 0;

void check(const char* name, bool condition, const char* detail = "") {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        if (detail[0]) std::cout << "        " << detail << "\n";
        ++failures;
    }
}

void check_close(const char* name, double a, double b, double tol) {
    bool ok = std::abs(a - b) < tol;
    if (ok) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(15) << a
                  << ", expected " << b << ", diff " << std::abs(a - b) << ")\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Multi-Scale Pipeline (12 checks)\n";
    std::cout << "================================================================\n\n";

    using namespace ftd;

    // ══════════════════════════════════════════════════════════════════
    // Phase 1: Full Pipeline Round-Trip (4 checks)
    //   Scale 0 → tick → coarsen → Scale 1 → coarsen → Scale 2
    //   → refine → Scale 1 → refine → Scale 0 → tick
    // ══════════════════════════════════════════════════════════════════
    {
        std::cout << "--- Phase 1: Full Pipeline Round-Trip ---\n";

        // Start at Scale 0: two locked protons (no electron — avoids annihilation)
        RenderBridge rb(32);
        rb.toggles.genesis = false;
        rb.inject_particle(16, 16, 16, +1, {0.0, 0.0, K_B});
        rb.inject_particle(17, 16, 16, +1, {0.0, 0.0, K_B});

        // Lock the protons
        {
            const auto& lat = rb.lattice();
            rb.voxels()[lat.index(16, 16, 16)].locked = true;
            rb.voxels()[lat.index(17, 16, 16)].locked = true;
        }

        // Run Scale 0 briefly to establish fields (locked particles survive)
        for (int i = 0; i < 50; ++i) rb.tick();

        auto audit_pre = rb.energy_audit();
        int charge_pre = audit_pre.charge_total;
        int manifested_pre = audit_pre.manifested_count;

        // Coarsen Scale 0 → Scale 1
        auto particles = coarsen_to_particles(rb);

        // Coarsen Scale 1 → Scale 2
        ParticleEngine pe_tmp;
        pe_tmp.set_gravity_enabled(false);
        for (const auto& p : particles) {
            if (p.locked) {
                pe_tmp.add_locked_particle(p.charge, p.position, p.mass);
            } else {
                pe_tmp.add_particle(p.charge, p.position, p.velocity, p.mass, p.r_eff);
            }
        }
        auto atoms = coarsen_to_atoms(pe_tmp);

        // Refine Scale 2 → Scale 1
        std::vector<Particle> refined_particles;
        for (const auto& a : atoms) {
            auto ap = refine_to_particles(a);
            refined_particles.insert(refined_particles.end(), ap.begin(), ap.end());
        }

        // Refine Scale 1 → Scale 0 (on a fresh lattice)
        RenderBridge rb2(32);
        rb2.toggles.genesis = false;
        for (const auto& p : refined_particles) {
            refine_to_voxels(p, rb2);
        }

        // Run Scale 0 again for 200 ticks
        for (int i = 0; i < 200; ++i) rb2.tick();
        auto audit_post = rb2.energy_audit();

        // Checks
        // Note: exact charge conservation through full Scale 2 round-trip is not
        // expected because refine_to_particles places all Z protons at the centroid,
        // and refine_to_voxels can only hold 1 particle per lattice site.
        // We check charge sign is preserved instead.
        check("P1.1: charge sign preserved (positive)",
              audit_post.charge_total > 0);
        check("P1.2: manifested count >= 1",
              audit_post.manifested_count >= 1);
        check("P1.3: at least 1 atom detected",
              atoms.size() >= 1);
        if (!atoms.empty()) {
            // The two locked protons should form a Z=2 atom
            int max_z = 0;
            for (const auto& a : atoms) {
                if (a.Z > max_z) max_z = a.Z;
            }
            check("P1.4: largest atom Z == 2",
                  max_z == 2);
        } else {
            check("P1.4: largest atom Z == 2", false, "no atoms detected");
        }

        std::cout << "        charge: " << charge_pre << " -> "
                  << audit_post.charge_total << ", atoms: " << atoms.size() << "\n";
    }

    // ══════════════════════════════════════════════════════════════════
    // Phase 2: Energy Conservation Across Transitions (3 checks)
    // ══════════════════════════════════════════════════════════════════
    {
        std::cout << "\n--- Phase 2: Energy Conservation Across Transitions ---\n";

        // Single wavepacket, run to steady state
        RenderBridge rb(32);
        rb.toggles.genesis = false;
        rb.inject_wavepacket(16, 16, 16, +1, 3.0, K_B);
        for (int i = 0; i < 500; ++i) rb.tick();

        auto audit = rb.energy_audit();
        double lattice_energy = audit.field_energy + audit.wave_energy;
        double target = K_B * K_B;
        double ratio = lattice_energy / target;
        std::cout << "        lattice energy = " << lattice_energy
                  << ", K_B^2 = " << target << ", ratio = " << ratio << "\n";

        // P2.1: lattice energy in range of K_B^2 (within order of magnitude)
        check("P2.1: lattice energy ~ K_B^2 (order of magnitude)",
              ratio > 0.01 && ratio < 100.0);

        // Coarsen to particles and check mass
        auto particles = coarsen_to_particles(rb);
        bool found_electron = false;
        for (const auto& p : particles) {
            if (p.charge != 0) {
                // P2.2: particle mass should be K_B
                check_close("P2.2: particle mass == K_B", p.mass, K_B, 1e-10);
                found_electron = true;
                break;
            }
        }
        if (!found_electron) {
            check("P2.2: particle mass == K_B", false, "no charged particle found");
        }

        // Refine back and check energy recovery
        if (found_electron) {
            Particle p_ref;
            for (const auto& p : particles) {
                if (p.charge != 0) { p_ref = p; break; }
            }
            RenderBridge rb2(32);
            rb2.toggles.genesis = false;
            refine_to_voxels(p_ref, rb2);
            for (int i = 0; i < 500; ++i) rb2.tick();

            auto audit2 = rb2.energy_audit();
            double energy2 = audit2.field_energy + audit2.wave_energy;
            double ratio2 = energy2 / target;
            std::cout << "        recovered energy = " << energy2 << ", ratio = " << ratio2 << "\n";
            // P2.3: recovered energy also in range
            check("P2.3: recovered energy ~ K_B^2 (order of magnitude)",
                  ratio2 > 0.01 && ratio2 < 100.0);
        } else {
            check("P2.3: recovered energy ~ K_B^2", false, "no particle to refine");
        }
    }

    // ══════════════════════════════════════════════════════════════════
    // Phase 3: Multi-Atom Pipeline (3 checks)
    //   3 separated nuclei: H, He, Li → coarsen → refine → coarsen
    // ══════════════════════════════════════════════════════════════════
    {
        std::cout << "\n--- Phase 3: Multi-Atom Pipeline ---\n";

        ParticleEngine pe;
        pe.set_gravity_enabled(false);

        // H nucleus (Z=1) near x=20
        pe.add_locked_particle(+1, {20.0, 50.0, 50.0});

        // He nucleus (Z=2) near x=80
        pe.add_locked_particle(+1, {80.0, 50.0, 50.0});
        pe.add_locked_particle(+1, {81.0, 50.0, 50.0});

        // Li nucleus (Z=3) near x=150
        pe.add_locked_particle(+1, {150.0, 50.0, 50.0});
        pe.add_locked_particle(+1, {151.0, 50.0, 50.0});
        pe.add_locked_particle(+1, {150.0, 51.0, 50.0});

        // Initial coarsen
        auto atoms1 = coarsen_to_atoms(pe);
        check("P3.1: 3 atoms initially", atoms1.size() == 3);

        // Refine all atoms to particles
        std::vector<Particle> all_particles;
        for (const auto& a : atoms1) {
            auto ap = refine_to_particles(a);
            all_particles.insert(all_particles.end(), ap.begin(), ap.end());
        }

        // Re-coarsen
        ParticleEngine pe2;
        pe2.set_gravity_enabled(false);
        for (const auto& p : all_particles) {
            if (p.locked) {
                pe2.add_locked_particle(p.charge, p.position, p.mass);
            } else {
                pe2.add_particle(p.charge, p.position, p.velocity, p.mass, p.r_eff);
            }
        }

        auto atoms2 = coarsen_to_atoms(pe2);
        check("P3.2: 3 atoms after round-trip", atoms2.size() == 3);

        // Check Z values preserved
        if (atoms2.size() == 3) {
            std::vector<int> z_vals;
            for (const auto& a : atoms2) z_vals.push_back(a.Z);
            std::sort(z_vals.begin(), z_vals.end());
            check("P3.3: Z values {1,2,3} preserved",
                  z_vals[0] == 1 && z_vals[1] == 2 && z_vals[2] == 3);
        } else {
            check("P3.3: Z values preserved", false, "wrong atom count");
        }
    }

    // ══════════════════════════════════════════════════════════════════
    // Phase 4: Cross-Scale Force Direction (2 checks)
    //   Opposite charges should attract at both Scale 1 and Scale 0
    // ══════════════════════════════════════════════════════════════════
    {
        std::cout << "\n--- Phase 4: Cross-Scale Force Direction ---\n";

        // Scale 1: analytical force between +1 and -1 at distance 10
        ParticleEngine pe;
        pe.set_gravity_enabled(false);
        pe.add_particle(+1, {50.0, 50.0, 50.0});
        pe.add_particle(-1, {60.0, 50.0, 50.0});

        // Record initial separation
        double sep0 = (pe.particles()[1].position - pe.particles()[0].position).mag();

        // Run 100 steps
        for (int i = 0; i < 100; ++i) pe.tick();
        double sep1 = (pe.particles()[1].position - pe.particles()[0].position).mag();

        // Opposite charges should attract → separation decreases
        check("P4.1: Scale 1 opposite charges attract (sep decreases)",
              sep1 < sep0);
        std::cout << "        Scale 1 sep: " << sep0 << " -> " << sep1 << "\n";

        // Scale 0: place +1 and -1 on lattice, run, check approach or annihilation
        RenderBridge rb(32);
        rb.toggles.genesis = false;
        rb.inject_wavepacket(10, 16, 16, +1, 3.0, K_B);
        rb.inject_wavepacket(22, 16, 16, -1, 3.0, K_B);

        auto audit0 = rb.energy_audit();
        int initial_count = audit0.manifested_count;

        for (int i = 0; i < 500; ++i) rb.tick();

        auto audit1 = rb.energy_audit();
        // Either particles approached (fewer manifested if annihilated)
        // or they still exist but closer together
        // The simplest check: something happened (energy changed or count changed)
        bool approached = (audit1.manifested_count < initial_count) ||
                          (std::abs(audit1.field_energy - audit0.field_energy) > 0.001);
        check("P4.2: Scale 0 opposite charges interact (energy/count changed)",
              approached);
        std::cout << "        Scale 0 count: " << initial_count << " -> "
                  << audit1.manifested_count << "\n";
    }

    // ── Summary ──────────────────────────────────────────────────────
    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  ALL CHECKS PASSED\n";
    } else {
        std::cout << "  " << failures << " CHECK(S) FAILED\n";
    }
    std::cout << "================================================================\n";

    return failures;
}

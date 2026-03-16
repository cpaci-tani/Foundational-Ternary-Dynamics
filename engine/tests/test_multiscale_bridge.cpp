/**
 * Test: Multi-Scale Bridge (13 unit checks)
 *
 * Covers quantum number preservation, position round-trips,
 * OnticEntity consistency, multi-nuclei clustering, edge cases,
 * and energy budget across Scale 0 ↔ 1 ↔ 2 transitions.
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
    std::cout << "  TEST: Multi-Scale Bridge (13 checks)\n";
    std::cout << "================================================================\n\n";

    using namespace ftd;

    // ── MSB1: spin preservation (Scale 0 → Scale 1) ──────────────────
    {
        std::cout << "--- MSB1: Spin preservation ---\n";
        RenderBridge rb(32);
        rb.toggles.genesis = false;
        rb.inject_particle(16, 16, 16, +1, {0.0, 0.0, K_B}, /*spin=*/1, /*color=*/0);
        // Run a few ticks to establish field
        for (int i = 0; i < 10; ++i) rb.tick();

        auto particles = coarsen_to_particles(rb);
        bool found = false;
        for (const auto& p : particles) {
            if (p.charge == +1) {
                check("spin == 1", p.spin == 1);
                found = true;
                break;
            }
        }
        if (!found) {
            check("particle found for spin test", false, "no +1 particle after coarsen");
        }
    }

    // ── MSB2: color preservation (Scale 0 → Scale 1) ─────────────────
    {
        std::cout << "\n--- MSB2: Color preservation ---\n";
        RenderBridge rb(32);
        rb.toggles.genesis = false;
        rb.inject_particle(16, 16, 16, +1, {0.0, 0.0, K_B}, /*spin=*/0, /*color=*/2);
        for (int i = 0; i < 10; ++i) rb.tick();

        auto particles = coarsen_to_particles(rb);
        bool found = false;
        for (const auto& p : particles) {
            if (p.charge == +1) {
                check("color == 2", p.color == 2);
                found = true;
                break;
            }
        }
        if (!found) {
            check("particle found for color test", false, "no +1 particle after coarsen");
        }
    }

    // ── MSB3: pair_id preservation (entangled pair) ──────────────────
    {
        std::cout << "\n--- MSB3: pair_id preservation ---\n";
        RenderBridge rb(32);
        rb.toggles.genesis = false;
        rb.create_entangled_pair(16, 16, 16, {0.0, 0.0, K_B});
        for (int i = 0; i < 10; ++i) rb.tick();

        auto particles = coarsen_to_particles(rb);
        int with_pair = 0;
        int32_t first_pair_id = -1;
        for (const auto& p : particles) {
            if (p.pair_id >= 0) {
                if (first_pair_id < 0) first_pair_id = p.pair_id;
                with_pair++;
            }
        }
        check("2 particles with pair_id >= 0", with_pair == 2);
        // Both should share the same pair_id
        if (with_pair == 2) {
            bool matching = true;
            for (const auto& p : particles) {
                if (p.pair_id >= 0 && p.pair_id != first_pair_id) {
                    matching = false;
                }
            }
            check("pair_ids match", matching);
        }
    }

    // ── MSB4: quantum round-trip (Scale 0 → 1 → 0) ──────────────────
    {
        std::cout << "\n--- MSB4: Quantum round-trip ---\n";
        RenderBridge rb(32);
        rb.toggles.genesis = false;
        rb.inject_particle(16, 16, 16, +1, {0.0, 0.0, K_B}, /*spin=*/1, /*color=*/3);
        for (int i = 0; i < 10; ++i) rb.tick();

        // Coarsen to Scale 1
        auto particles = coarsen_to_particles(rb);

        // Find our particle
        Particle target;
        bool found = false;
        for (const auto& p : particles) {
            if (p.charge == +1) {
                target = p;
                found = true;
                break;
            }
        }

        if (found) {
            // Refine back to Scale 0 on a fresh lattice
            RenderBridge rb2(32);
            rb2.toggles.genesis = false;
            refine_to_voxels(target, rb2);

            // Read back the voxel
            const auto& lat2 = rb2.lattice();
            int ix = static_cast<int>(std::floor(target.position.x));
            int iy = static_cast<int>(std::floor(target.position.y));
            int iz = static_cast<int>(std::floor(target.position.z));
            ix = ((ix % 32) + 32) % 32;
            iy = ((iy % 32) + 32) % 32;
            iz = ((iz % 32) + 32) % 32;
            int idx = lat2.index(ix, iy, iz);
            const auto& v = rb2.voxels()[idx];
            check("spin preserved after round-trip", v.spin == 1);
            check("color preserved after round-trip", v.color == 3);
        } else {
            check("particle found for round-trip", false, "no +1 particle");
        }
    }

    // ── MSB5: 3D position round-trip ─────────────────────────────────
    {
        std::cout << "\n--- MSB5: 3D position round-trip ---\n";
        RenderBridge rb(32);
        rb.toggles.genesis = false;
        // Inject at integer position (7, 14, 22)
        rb.inject_particle(7, 14, 22, +1, {0.0, 0.0, K_B});
        for (int i = 0; i < 10; ++i) rb.tick();

        auto particles = coarsen_to_particles(rb);
        bool found = false;
        for (const auto& p : particles) {
            if (p.charge == +1) {
                // Position should be near injection point
                check_close("position.x near 7", p.position.x, 7.0, 2.0);
                check_close("position.y near 14", p.position.y, 14.0, 2.0);
                check_close("position.z near 22", p.position.z, 22.0, 2.0);
                found = true;
                break;
            }
        }
        if (!found) {
            check("particle found for position test", false, "no +1 particle");
        }
    }

    // ── MSB6: boundary wrapping ──────────────────────────────────────
    {
        std::cout << "\n--- MSB6: Boundary wrapping ---\n";
        // Create a particle near the edge and refine to voxels
        Particle p;
        p.charge = +1;
        p.mass = K_B;
        p.r_eff = 2.48;
        p.position = {31.7, 0.2, 15.0};
        p.spin = 0;
        p.color = 0;

        RenderBridge rb(32);
        rb.toggles.genesis = false;
        refine_to_voxels(p, rb);

        // Check: state at integer position (31, 0, 15) should be set
        const auto& lat = rb.lattice();
        int idx = lat.index(31, 0, 15);
        const auto& v = rb.voxels()[idx];
        check("state at (31,0,15) is nonzero", v.state != 0);
        // Remainder should capture fractional part
        check_close("remainder.x near 0.7", v.remainder.x, 0.7, 0.05);
        check_close("remainder.y near 0.2", v.remainder.y, 0.2, 0.05);
    }

    // ── MSB7: OnticEntity consistency ────────────────────────────────
    {
        std::cout << "\n--- MSB7: OnticEntity consistency ---\n";
        // Particle OnticEntity
        Particle p;
        p.charge = +1;
        p.mass = K_B;
        p.r_eff = 2.48;
        OnticEntity pe = p.as_ontic();
        check("particle state == +1", pe.state == +1);
        check_close("particle energy == K_B", pe.energy, K_B, 1e-10);
        check_close("particle boundary == 2.48", pe.boundary, 2.48, 1e-10);

        // Atom OnticEntity
        AtomicProperties props = compute_atomic_properties(2, 2);
        Atom a;
        a.Z = 2;
        a.N = 2;
        a.mass = props.mass;
        a.radius = props.radius;
        OnticEntity ae = a.as_ontic();
        check("atom state == Z == 2", ae.state == 2);
        check_close("atom energy == mass", ae.energy, props.mass, 1e-10);
        check_close("atom boundary == radius", ae.boundary, props.radius, 1e-10);
    }

    // ── MSB8: multi-nuclei clustering ────────────────────────────────
    {
        std::cout << "\n--- MSB8: Multi-nuclei clustering ---\n";
        ParticleEngine pe;
        pe.set_gravity_enabled(false);

        // Cluster 1: 3 protons near (50, 50, 50)
        pe.add_locked_particle(+1, {50.0, 50.0, 50.0});
        pe.add_locked_particle(+1, {51.0, 50.0, 50.0});
        pe.add_locked_particle(+1, {50.0, 51.0, 50.0});

        // Cluster 2: 1 proton far away at (200, 200, 200)
        pe.add_locked_particle(+1, {200.0, 200.0, 200.0});

        auto atoms = coarsen_to_atoms(pe);
        check("2 atoms produced", atoms.size() == 2);
        if (atoms.size() == 2) {
            // Sort by Z to get deterministic order
            int z_small = std::min(atoms[0].Z, atoms[1].Z);
            int z_large = std::max(atoms[0].Z, atoms[1].Z);
            check("Z values are {1, 3}", z_small == 1 && z_large == 3);
        }
    }

    // ── MSB9: lone electrons → 0 atoms ───────────────────────────────
    {
        std::cout << "\n--- MSB9: Lone electrons produce 0 atoms ---\n";
        ParticleEngine pe;
        pe.set_gravity_enabled(false);
        // Only electrons, no protons
        pe.add_particle(-1, {10.0, 10.0, 10.0});
        pe.add_particle(-1, {20.0, 20.0, 20.0});
        pe.add_particle(-1, {30.0, 30.0, 30.0});

        auto atoms = coarsen_to_atoms(pe);
        check("0 atoms from lone electrons", atoms.size() == 0);
    }

    // ── MSB10: ionic charge ──────────────────────────────────────────
    {
        std::cout << "\n--- MSB10: Ionic charge ---\n";
        ParticleEngine pe;
        pe.set_gravity_enabled(false);
        // 2 protons (He nucleus) + only 1 electron → He+ ion
        pe.add_locked_particle(+1, {50.0, 50.0, 50.0});
        pe.add_locked_particle(+1, {51.0, 50.0, 50.0});
        pe.add_particle(-1, {53.0, 50.0, 50.0});

        auto atoms = coarsen_to_atoms(pe);
        check("one atom", atoms.size() == 1);
        if (!atoms.empty()) {
            check("Z == 2", atoms[0].Z == 2);
            check("charge == +1 (He+ ion)", atoms[0].charge == 1);
        }
    }

    // ── MSB11: locked preservation (Scale 0 → Scale 1) ───────────────
    {
        std::cout << "\n--- MSB11: Locked preservation ---\n";
        RenderBridge rb(32);
        rb.toggles.genesis = false;
        // Inject and manually set locked flag on the voxel
        rb.inject_particle(16, 16, 16, +1, {0.0, 0.0, K_B});
        const auto& lat = rb.lattice();
        int idx = lat.index(16, 16, 16);
        rb.voxels()[idx].locked = true;
        for (int i = 0; i < 10; ++i) rb.tick();

        auto particles = coarsen_to_particles(rb);
        bool found = false;
        for (const auto& p : particles) {
            if (p.charge == +1) {
                check("locked flag preserved", p.locked == true);
                found = true;
                break;
            }
        }
        if (!found) {
            check("particle found for locked test", false, "no +1 particle");
        }
    }

    // ── MSB12: electron loss in Scale 2→1→2 (known limitation) ───────
    {
        std::cout << "\n--- MSB12: Electron loss (known limitation) ---\n";
        // H atom: Z=1, neutral → 1 electron placed at a.radius from center
        // a.radius >> CLUSTER_RADIUS*3 = 15, so electron won't be recaptured
        Atom a;
        a.Z = 1;
        a.N = 0;
        a.charge = 0;
        AtomicProperties props = compute_atomic_properties(1, 0);
        a.mass = props.mass;
        a.radius = props.radius;
        a.position = {50.0, 50.0, 50.0};

        // Step 1: refine atom to particles
        auto particles = refine_to_particles(a);
        int n_electrons = 0;
        for (const auto& p : particles) {
            if (p.charge == -1) n_electrons++;
        }
        check("refine produces 1 electron", n_electrons == 1);

        // Step 2: put particles into ParticleEngine and coarsen back
        ParticleEngine pe;
        pe.set_gravity_enabled(false);
        for (const auto& p : particles) {
            if (p.locked) {
                pe.add_locked_particle(p.charge, p.position, p.mass);
            } else {
                pe.add_particle(p.charge, p.position, p.velocity, p.mass, p.r_eff);
            }
        }

        auto atoms = coarsen_to_atoms(pe);
        check("atom found", atoms.size() >= 1);
        if (!atoms.empty()) {
            // Electron at radius >> 15 is outside search range → charge should be +1
            std::cout << "        atom radius = " << a.radius
                      << ", search radius = 15, charge = " << atoms[0].charge << "\n";
            check("charge == +1 (electron lost, radius >> search range)",
                  atoms[0].charge == 1);
        }
    }

    // ── MSB13: refine energy budget ──────────────────────────────────
    {
        std::cout << "\n--- MSB13: Refine energy budget ---\n";
        // refine_to_voxels calls inject_wavepacket which injects ~K_B total flux
        Particle p;
        p.charge = +1;
        p.mass = K_B;
        p.r_eff = 2.48;
        p.position = {16.0, 16.0, 16.0};

        RenderBridge rb(32);
        rb.toggles.genesis = false;
        refine_to_voxels(p, rb);

        // Let field settle briefly
        for (int i = 0; i < 5; ++i) rb.tick();

        auto audit = rb.energy_audit();
        double total = audit.field_energy + audit.wave_energy;
        // Energy should be on the order of K_B^2 (the amplitude parameter squared
        // summed over the Gaussian envelope)
        double target = K_B * K_B;
        double ratio = total / target;
        std::cout << "        total energy = " << total << ", K_B^2 = " << target
                  << ", ratio = " << ratio << "\n";
        // Allow wide tolerance — just verify energy is in the right ballpark
        check("energy within order of magnitude of K_B^2",
              ratio > 0.01 && ratio < 100.0);
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

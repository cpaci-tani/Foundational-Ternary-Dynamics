/**
 * Test: Atom Scale Bridge (Scale 1 ↔ Scale 2)
 *
 * 6 checks covering coarsen_to_atoms and refine_to_particles.
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/atom_engine.h"
#include "ftd/particle_engine.h"
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
    std::cout << "  TEST: Atom Scale Bridge (Scale 1 <-> Scale 2)\n";
    std::cout << "================================================================\n\n";

    using namespace ftd;

    // ── ASB1: coarsen — Z from proton count ─────────────────────────
    {
        std::cout << "--- ASB1: Z from proton count ---\n";
        ParticleEngine pe;
        pe.set_gravity_enabled(false);
        // Add 2 locked protons (He nucleus)
        pe.add_locked_particle(+1, {50.0, 50.0, 50.0});
        pe.add_locked_particle(+1, {51.0, 50.0, 50.0});
        // Add 2 electrons nearby
        pe.add_particle(-1, {53.0, 50.0, 50.0});
        pe.add_particle(-1, {47.0, 50.0, 50.0});

        auto atoms = coarsen_to_atoms(pe);
        check("one atom produced", atoms.size() == 1);
        if (!atoms.empty()) {
            check("Z == 2 (helium)", atoms[0].Z == 2);
        }
    }

    // ── ASB2: coarsen — position from centroid ──────────────────────
    {
        std::cout << "\n--- ASB2: Position from centroid ---\n";
        ParticleEngine pe;
        pe.set_gravity_enabled(false);
        pe.add_locked_particle(+1, {10.0, 20.0, 30.0});
        pe.add_locked_particle(+1, {12.0, 20.0, 30.0});

        auto atoms = coarsen_to_atoms(pe);
        if (!atoms.empty()) {
            check_close("centroid x", atoms[0].position.x, 11.0, 1.0);
            check_close("centroid y", atoms[0].position.y, 20.0, 1.0);
        }
    }

    // ── ASB3: refine — correct particle count ───────────────────────
    {
        std::cout << "\n--- ASB3: Refine particle count ---\n";
        Atom a;
        a.Z = 3;  // Lithium
        a.N = 4;
        a.charge = 0;  // neutral → 3 electrons
        a.mass = compute_atomic_properties(3, 4).mass;
        a.radius = compute_atomic_properties(3, 4).radius;
        a.position = {50.0, 50.0, 50.0};

        auto particles = refine_to_particles(a);
        int protons = 0, electrons = 0;
        for (const auto& p : particles) {
            if (p.charge == +1) protons++;
            if (p.charge == -1) electrons++;
        }
        check("3 protons", protons == 3);
        check("3 electrons (neutral)", electrons == 3);
        check("total = 6 particles", static_cast<int>(particles.size()) == 6);
    }

    // ── ASB4: refine — protons locked ───────────────────────────────
    {
        std::cout << "\n--- ASB4: Protons locked ---\n";
        Atom a;
        a.Z = 1;
        a.N = 0;
        a.charge = 0;
        a.mass = compute_atomic_properties(1, 0).mass;
        a.radius = compute_atomic_properties(1, 0).radius;
        a.position = {0.0, 0.0, 0.0};

        auto particles = refine_to_particles(a);
        bool all_protons_locked = true;
        for (const auto& p : particles) {
            if (p.charge == +1 && !p.locked) {
                all_protons_locked = false;
            }
        }
        check("all protons are locked", all_protons_locked);
    }

    // ── ASB5: Round-trip preserves Z ────────────────────────────────
    {
        std::cout << "\n--- ASB5: Round-trip preserves Z ---\n";
        // Start with atom → refine to particles → coarsen back
        Atom a;
        a.Z = 2;
        a.N = 2;
        a.charge = 0;
        a.mass = compute_atomic_properties(2, 2).mass;
        a.radius = compute_atomic_properties(2, 2).radius;
        a.position = {50.0, 50.0, 50.0};

        auto particles = refine_to_particles(a);

        // Put particles into a ParticleEngine
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
        check("round-trip: one atom", atoms.size() == 1);
        if (!atoms.empty()) {
            check("round-trip: Z preserved", atoms[0].Z == 2);
        }
    }

    // ── ASB6: Mass conserved (proton sector) ────────────────────────
    {
        std::cout << "\n--- ASB6: Mass conserved (proton sector) ---\n";
        // Use hydrogen (N=0) so atom mass ≈ proton mass (no neutrons to lose)
        Atom a;
        a.Z = 1;
        a.N = 0;
        a.charge = 0;
        a.mass = compute_atomic_properties(1, 0).mass;
        a.radius = compute_atomic_properties(1, 0).radius;
        a.position = {50.0, 50.0, 50.0};

        auto particles = refine_to_particles(a);
        double total_mass = 0.0;
        for (const auto& p : particles) {
            total_mass += p.mass;
        }
        // H atom mass = 1*M_PROTON (no neutrons)
        // Particle masses = 1*M_PROTON (proton) + 1*K_B (electron)
        // Electron mass << proton mass, so ratio ≈ 1 + K_B/M_PROTON ≈ 1.0003
        double ratio = total_mass / a.mass;
        check("mass ratio within 1% (H)", std::abs(ratio - 1.0) < 0.01);
        std::cout << "        atom mass = " << a.mass << ", particle sum = " << total_mass
                  << ", ratio = " << ratio << "\n";
    }

    // ── Summary ─────────────────────────────────────────────────────
    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  ALL CHECKS PASSED\n";
    } else {
        std::cout << "  " << failures << " CHECK(S) FAILED\n";
    }
    std::cout << "================================================================\n";

    return failures;
}

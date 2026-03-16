/**
 * Phase 7 — Stage 2: ParticleEngine unit tests (12 checks)
 *
 * PE1:  Particle injection (id assigned, charge correct)
 * PE2:  Free particle (constant velocity when alone, no damping)
 * PE3:  Opposite attract (force points inward)
 * PE4:  Same repel (force points outward)
 * PE5:  Force magnitude (F = alpha/(4*pi*r^2) at r=10, within 1%)
 * PE6:  Gravity attractive (both charges attracted)
 * PE7:  Speed limit (|v| clamped to C_SPEED)
 * PE8:  Annihilation (opposites within r_eff removed)
 * PE9:  Energy conservation (|dE/E| < 0.01% over 1000 ticks, 2-body, no damping)
 * PE10: Momentum conservation (|dp/p| < 0.01% for isolated system, no damping)
 * PE11: Softening (no NaN/Inf at r -> 0)
 * PE12: Constants from ontic (alpha, G_N, K_B, PI match ontic.h)
 */

#include "ftd/particle_engine.h"
#include "ftd/ontic.h"
#include <iostream>
#include <cmath>

static int pass_count = 0;
static int fail_count = 0;

static void check(const char* name, bool ok) {
    if (ok) { ++pass_count; std::cout << "  PASS  " << name << "\n"; }
    else    { ++fail_count; std::cout << "  FAIL  " << name << "\n"; }
}

int main() {
    using namespace ftd;

    std::cout << "============================================================\n";
    std::cout << "  Phase 7 Stage 2: ParticleEngine Unit Tests\n";
    std::cout << "============================================================\n\n";

    // ---- PE1: Particle injection ----
    {
        std::cout << "--- PE1: Particle injection ---\n";
        ParticleEngine pe;
        int id0 = pe.add_particle(+1, {0, 0, 0});
        int id1 = pe.add_particle(-1, {10, 0, 0});
        check("PE1a: first particle id = 0", id0 == 0);
        check("PE1b: second particle id = 1", id1 == 1);
        check("PE1c: charge +1", pe.particles()[0].charge == +1);
        check("PE1d: charge -1", pe.particles()[1].charge == -1);
        check("PE1e: particle count = 2", pe.particles().size() == 2);
    }

    // ---- PE2: Free particle (constant velocity, no damping) ----
    {
        std::cout << "\n--- PE2: Free particle ---\n";
        ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {0.1, 0, 0});
        pe.run(100);
        double x = pe.particles()[0].position.x;
        double expected = 0.1 * 100;  // v*t with dt=1
        double err = std::abs(x - expected) / expected;
        std::cout << "    x after 100 ticks: " << x << " (expected " << expected << ")\n";
        check("PE2: free particle x within 0.1%", err < 0.001);
    }

    // ---- PE3: Opposite attract (force points inward) ----
    {
        std::cout << "\n--- PE3: Opposite charges attract ---\n";
        ParticleEngine pe;
        pe.add_particle(+1, {0, 0, 0});
        pe.add_particle(-1, {20, 0, 0});
        Vec3 f = pe.compute_force(0);
        std::cout << "    Force on +1 from -1 at r=20: (" << f.x << ", " << f.y << ", " << f.z << ")\n";
        check("PE3: force x > 0 (attraction toward +x)", f.x > 0);
    }

    // ---- PE4: Same charges — EM repels (gravity dominates on lattice) ----
    {
        std::cout << "\n--- PE4: EM repulsion component ---\n";
        // On the lattice, G_N = 0.01 >> alpha/(4*pi) ≈ 0.00058, so gravity
        // dominates over EM. The NET force for same charges is attractive.
        // We verify the EM COMPONENT is repulsive by comparing:
        //   F(charged) vs F(neutral) — the difference is the EM contribution.
        ParticleEngine pe_charged;
        pe_charged.add_particle(+1, {0, 0, 0});
        pe_charged.add_particle(+1, {20, 0, 0});
        Vec3 f_charged = pe_charged.compute_force(0);

        ParticleEngine pe_neutral;
        pe_neutral.add_particle(0, {0, 0, 0});
        pe_neutral.add_particle(0, {20, 0, 0});
        Vec3 f_neutral = pe_neutral.compute_force(0);

        // EM contribution = total - gravity-only
        double em_x = f_charged.x - f_neutral.x;
        std::cout << "    Total force (charged): " << f_charged.x << "\n";
        std::cout << "    Gravity-only (neutral): " << f_neutral.x << "\n";
        std::cout << "    EM component: " << em_x << "\n";
        check("PE4a: EM component is repulsive (negative x)", em_x < 0);
        check("PE4b: net force less attractive than pure gravity",
              f_charged.x < f_neutral.x);
    }

    // ---- PE5: Force magnitude ----
    {
        std::cout << "\n--- PE5: Force magnitude at r=10 ---\n";
        ParticleEngine pe;
        pe.set_softening(0.0);  // exact for this test
        pe.add_particle(+1, {0, 0, 0});
        pe.add_particle(-1, {10, 0, 0});
        Vec3 f = pe.compute_force(0);
        double f_mag = f.mag();
        // F = alpha * |q1*q2| / (4*pi*r^2) = alpha / (4*pi*100)
        double expected = ALPHA / (4.0 * PI * 100.0);
        // Gravity component: G_N * m1 * m2 / r^2 (very small compared to EM)
        double grav = G_N * K_B * K_B / 100.0;
        double total_expected = expected + grav;  // both point toward +x
        double err = std::abs(f_mag - total_expected) / total_expected;
        std::cout << "    |F| = " << f_mag << ", expected EM = " << expected
                  << " + grav = " << grav << " = " << total_expected << "\n";
        std::cout << "    relative error = " << err * 100.0 << "%\n";
        check("PE5: force magnitude within 1%", err < 0.01);
    }

    // ---- PE6: Gravity attractive ----
    {
        std::cout << "\n--- PE6: Gravity attractive ---\n";
        // Use neutral (charge=0) particles so only gravity acts
        ParticleEngine pe;
        pe.add_particle(0, {0, 0, 0});    // neutral, mass = K_B
        pe.add_particle(0, {5, 0, 0});    // neutral, mass = K_B
        Vec3 f = pe.compute_force(0);
        // r_hat points from 0 toward 1 = +x direction
        // Gravity is attractive → force on 0 should point toward 1 → f.x > 0
        std::cout << "    Pure gravity force: (" << f.x << ", " << f.y << ", " << f.z << ")\n";
        check("PE6a: gravity force points toward other (f.x > 0)", f.x > 0);

        // Also verify that for same charges, gravity makes repulsion weaker
        ParticleEngine pe_em;
        pe_em.add_particle(+1, {0, 0, 0});
        pe_em.add_particle(+1, {5, 0, 0});
        Vec3 f_with_grav = pe_em.compute_force(0);
        // EM repels (f.x < 0), gravity attracts (f.x > 0)
        // Net f.x should be less negative than pure EM
        double f_em_pure = -ALPHA / (4.0 * PI * (25.0 + 1.0));
        std::cout << "    Net force (EM+grav): " << f_with_grav.x
                  << ", pure EM: " << f_em_pure << "\n";
        check("PE6b: gravity reduces repulsion", f_with_grav.x > f_em_pure);
    }

    // ---- PE7: Speed limit ----
    {
        std::cout << "\n--- PE7: Speed limit ---\n";
        ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {2.0, 0, 0});  // v > C_SPEED
        pe.tick();
        double v = pe.particles()[0].velocity.mag();
        std::cout << "    Speed after tick: " << v << " (limit = " << C_SPEED << ")\n";
        check("PE7: speed <= C_SPEED", v <= C_SPEED + 1e-15);
    }

    // ---- PE8: Annihilation ----
    {
        std::cout << "\n--- PE8: Annihilation ---\n";
        ParticleEngine pe;
        pe.set_damping_enabled(false);
        // Place opposite charges within contact distance (r < r_eff1 + r_eff2 = 4.96)
        pe.add_particle(+1, {0, 0, 0});
        pe.add_particle(-1, {2, 0, 0});  // r=2 < 4.96
        pe.tick();
        std::cout << "    Particles after tick: " << pe.particles().size() << "\n";
        check("PE8: annihilation removes both particles", pe.particles().empty());
    }

    // ---- PE9: Energy conservation (2-body, no damping) ----
    {
        std::cout << "\n--- PE9: Energy conservation ---\n";
        ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.set_softening(1.0);
        // Two same-charge particles in a bound-ish orbit won't work well.
        // Use opposite charges at moderate separation (no annihilation risk).
        // Increase r_eff to 0.01 so they don't annihilate easily at r=50.
        int id0 = pe.add_particle(+1, {0, 0, 0}, {0, 0, 0});
        int id1 = pe.add_particle(-1, {50, 0, 0}, {0, 0.0002, 0});
        pe.particles()[0].r_eff = 0.01;
        pe.particles()[1].r_eff = 0.01;

        auto d0 = pe.diagnostics();
        double e0 = d0.total_energy;
        std::cout << "    Initial energy: " << e0 << "\n";

        pe.run(1000);

        auto d1 = pe.diagnostics();
        double e1 = d1.total_energy;
        std::cout << "    Final energy:   " << e1 << "\n";

        double drift = (e0 != 0.0) ? std::abs(e1 - e0) / std::abs(e0) : std::abs(e1 - e0);
        std::cout << "    Energy drift:   " << drift * 100.0 << "%\n";
        check("PE9: energy drift < 0.01%", drift < 0.0001);
    }

    // ---- PE10: Momentum conservation (no damping) ----
    {
        std::cout << "\n--- PE10: Momentum conservation ---\n";
        ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.add_particle(+1, {0, 0, 0}, {0.01, 0, 0});
        pe.add_particle(+1, {30, 0, 0}, {-0.01, 0, 0});
        pe.particles()[0].r_eff = 0.01;
        pe.particles()[1].r_eff = 0.01;

        auto d0 = pe.diagnostics();
        Vec3 p0 = d0.total_momentum;
        double p0_mag = p0.mag();
        // Total initial momentum should be ~0 (equal and opposite)
        std::cout << "    Initial p = (" << p0.x << ", " << p0.y << ", " << p0.z << ")\n";

        pe.run(1000);

        auto d1 = pe.diagnostics();
        Vec3 p1 = d1.total_momentum;
        double dp = (p1 - p0).mag();
        std::cout << "    Final   p = (" << p1.x << ", " << p1.y << ", " << p1.z << ")\n";
        std::cout << "    |dp|      = " << dp << "\n";
        check("PE10: momentum change < 1e-10", dp < 1e-10);
    }

    // ---- PE11: Softening (no NaN at r=0) ----
    {
        std::cout << "\n--- PE11: Softening ---\n";
        ParticleEngine pe;
        pe.set_softening(1.0);
        pe.add_particle(+1, {5, 5, 5});
        pe.add_particle(-1, {5, 5, 5});  // exact same position
        pe.particles()[0].r_eff = 0.01;
        pe.particles()[1].r_eff = 0.01;

        Vec3 f = pe.compute_force(0);
        bool finite = std::isfinite(f.x) && std::isfinite(f.y) && std::isfinite(f.z);
        std::cout << "    Force at r=0: (" << f.x << ", " << f.y << ", " << f.z << ")\n";
        check("PE11: no NaN/Inf at r=0 with softening", finite);
    }

    // ---- PE12: Constants from ontic ----
    {
        std::cout << "\n--- PE12: Constants from ontic ---\n";
        check("PE12a: ALPHA = 1/137.036...",
              std::abs(ALPHA - 1.0 / 137.0361714582) < 1e-12);
        check("PE12b: G_N = 0.01",
              std::abs(G_N - 0.01) < 1e-15);
        check("PE12c: K_B = 0.511",
              std::abs(K_B - 0.511) < 1e-15);
        check("PE12d: PI ~ 3.14159",
              std::abs(PI - 3.14159265358979) < 1e-10);
        check("PE12e: C_SPEED = C_WAVE = 1/sqrt(3)",
              std::abs(C_SPEED - C_WAVE) < 1e-15);
    }

    // ---- Summary ----
    std::cout << "\n============================================================\n";
    std::cout << "  ParticleEngine: " << pass_count << " passed, " << fail_count << " failed\n";
    std::cout << "============================================================\n";

    return fail_count;
}

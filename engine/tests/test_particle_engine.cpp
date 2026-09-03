/**
 * Phase 7 — Stage 2: ParticleEngine unit tests (17 gates)
 *
 * PE1:  Particle injection (id assigned, charge correct)
 * PE2:  Free particle (constant velocity when alone, no damping)
 * PE3:  Opposite attract (force points inward)
 * PE4:  Same repel (force points outward)
 * PE5:  Force magnitude (F = alpha/(4*pi*r^2) at r=10, within 1%)
 * PE6:  Gravity attractive (both charges attracted)
 * PE7:  Speed limit (|v| clamped to C_SPEED)
 * PE8:  Selected contact removal (opposites within r_eff removed)
 * PE9:  Energy conservation (|dE/E| < 0.01% over 1000 ticks, 2-body, no damping)
 * PE10: Momentum conservation (|dp/p| < 0.01% for isolated system, no damping)
 * PE11: Softening (no NaN/Inf at r -> 0)
 * PE12: Scale-1 constants use the active ontic definitions
 * PE13: Effective-record admissibility rejects invalid native inputs/state
 * PE14: Integrator profile transitions preserve a free effective record
 * PE15: Simultaneous contact events are deterministic and identity-safe
 * PE16: Invalid post-state rolls the complete tick transaction back
 * PE17: Perfect-insulator walls block tunneling and pass declared ports only
 */

#include "ftd/particle_engine.h"
#include "ftd/ontic.h"
#include <iostream>
#include <cmath>
#include <limits>
#include <stdexcept>

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
        // ParticleEngine gravity uses G_PE ≈ 5.3e-46 (FTD-0131 physical
        // alpha_G), so EM dominates and the NET force for same charges is
        // repulsive. We verify the EM COMPONENT specifically by comparing:
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
        // Gravity component: G_PE * m1 * m2 / r^2 (FTD-0131 physical alpha_G,
        // ~1e-48 — utterly negligible next to EM)
        double grav = G_PE * K_B * K_B / 100.0;
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
        pe.toggles.gravity = true;
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

    // ---- PE8: Selected contact removal ----
    {
        std::cout << "\n--- PE8: Selected contact removal ---\n";
        ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.toggles.contact_events = true;
        // Place opposite charges within contact distance (r < r_eff1 + r_eff2 = 4.96)
        pe.add_particle(+1, {0, 0, 0});
        pe.add_particle(-1, {2, 0, 0});  // r=2 < 4.96
        pe.tick();
        std::cout << "    Particles after tick: " << pe.particles().size() << "\n";
        check("PE8: selected contact event removes both particles", pe.particles().empty());
    }

    // ---- PE9: Energy conservation (2-body, no damping) ----
    {
        std::cout << "\n--- PE9: Energy conservation ---\n";
        ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.set_softening(1.0);
        // Two same-charge particles in a bound-ish orbit won't work well.
        // Use opposite charges at moderate separation (no contact-removal risk).
        // Reduce r_eff to 0.01 so the selected event cannot trigger at r=50.
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

    // ---- PE12: Active Scale-1 constants from ontic ----
    {
        std::cout << "\n--- PE12: Constants from ontic ---\n";
        // 2026-04-17: ALPHA upgraded from 1/X_PLUS (tree, 137.0361714582)
        // to 1/X_PLUS_PRECISION (137.035999177, CODATA match).
        check("PE12a: ALPHA = 1/137.035999... (precision)",
              std::abs(ALPHA - 1.0 / 137.035999177) < 1e-12);
        check("PE12b: G_PE = 1/(4*pi*m_P^2)",
              std::abs(G_PE - 1.0 / (4.0 * PI * M_PLANCK_MEV * M_PLANCK_MEV))
                  < G_PE * 1e-15);
        check("PE12c: K_B = 0.511",
              std::abs(K_B - 0.511) < 1e-15);
        check("PE12d: PI ~ 3.14159",
              std::abs(PI - 3.14159265358979) < 1e-10);
        check("PE12e: C_SPEED = C_WAVE = 1/sqrt(3)",
              std::abs(C_SPEED - C_WAVE) < 1e-15);
    }

    // ---- PE13: Effective-record admissibility ----
    {
        std::cout << "\n--- PE13: Effective-record admissibility ---\n";
        ParticleEngine pe;
        std::string error;
        check("PE13a: fresh engine state is admissible", pe.validate_state(&error));

        bool rejected_mass = false;
        try {
            pe.add_particle(+1, {0, 0, 0}, {}, 0.0);
        } catch (const std::invalid_argument&) {
            rejected_mass = true;
        }
        check("PE13b: nonpositive mass rejected", rejected_mass);

        bool rejected_position = false;
        try {
            pe.add_particle(+1,
                {std::numeric_limits<double>::quiet_NaN(), 0, 0});
        } catch (const std::invalid_argument&) {
            rejected_position = true;
        }
        check("PE13c: nonfinite position rejected", rejected_position);

        bool rejected_dt = false;
        try {
            pe.set_dt(-1.0);
        } catch (const std::invalid_argument&) {
            rejected_dt = true;
        }
        check("PE13d: nonpositive dt rejected", rejected_dt);

        bool rejected_softening = false;
        try {
            pe.set_softening(std::numeric_limits<double>::infinity());
        } catch (const std::invalid_argument&) {
            rejected_softening = true;
        }
        check("PE13e: nonfinite softening rejected", rejected_softening);

        pe.add_particle(0, {0, 0, 0});
        pe.particles()[0].velocity.x = std::numeric_limits<double>::quiet_NaN();
        check("PE13f: direct record corruption is detected",
              !pe.validate_state(&error) && !error.empty());

        bool tick_failed_closed = false;
        try {
            pe.tick();
        } catch (const std::logic_error&) {
            tick_failed_closed = true;
        }
        check("PE13g: tick fails closed on an inadmissible record", tick_failed_closed);

        ParticleEngine overflow_guard;
        overflow_guard.toggles.coulomb = false;
        overflow_guard.toggles.relativistic_verlet = true;
        bool rejected_momentum_overflow = false;
        try {
            overflow_guard.add_particle(
                0, {0, 0, 0}, {C_SPEED * 0.99, 0, 0},
                std::numeric_limits<double>::max() * 0.5, 0.1);
        } catch (const std::overflow_error&) {
            rejected_momentum_overflow = true;
        }
        check("PE13h: finite inputs with unrepresentable momentum are rejected",
              rejected_momentum_overflow);
        check("PE13i: rejected injection leaves an admissible empty engine",
              overflow_guard.particles().empty()
              && overflow_guard.validate_state(&error));

        const int massive_id = overflow_guard.add_particle(
            0, {0, 0, 0}, {}, std::numeric_limits<double>::max() * 0.5, 0.1);
        bool rejected_velocity_overflow = false;
        try {
            (void)overflow_guard.set_particle_velocity(
                massive_id, {C_SPEED * 0.99, 0, 0});
        } catch (const std::overflow_error&) {
            rejected_velocity_overflow = true;
        }
        check("PE13j: velocity update rejects unrepresentable momentum",
              rejected_velocity_overflow);
        check("PE13k: rejected velocity update preserves the prior record",
              overflow_guard.particles()[0].velocity.mag2() == 0.0
              && overflow_guard.particles()[0].momentum.mag2() == 0.0
              && overflow_guard.validate_state(&error));
    }

    // ---- PE14: Integrator transition coherence ----
    {
        std::cout << "\n--- PE14: Integrator transition coherence ---\n";
        ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.toggles.coulomb = false;
        pe.set_dt(0.25);
        pe.add_particle(0, {0, 0, 0}, {0.2, 0, 0});

        pe.toggles.relativistic_verlet = false;
        pe.tick();
        const Vec3 after_newtonian = pe.particles()[0].velocity;
        pe.toggles.relativistic_verlet = true;
        pe.tick();
        const auto& p = pe.particles()[0];

        check("PE14a: Newtonian transition preserves free velocity",
              std::abs(after_newtonian.x - 0.2) < 1e-15);
        check("PE14b: relativistic transition preserves free velocity",
              std::abs(p.velocity.x - 0.2) < 1e-15);
        check("PE14c: position remains continuous across both profiles",
              std::abs(p.position.x - 0.1) < 1e-15);
        check("PE14d: synchronized relativistic momentum remains finite",
              std::isfinite(p.momentum.x) && p.momentum.x > p.mass * p.velocity.x);
    }

    // ---- PE15: Deterministic simultaneous contact events ----
    {
        std::cout << "\n--- PE15: Deterministic simultaneous contact events ---\n";
        auto seed = [](ParticleEngine& pe) {
            pe.set_damping_enabled(false);
            pe.toggles.coulomb = false;
            pe.toggles.contact_events = true;
            pe.add_particle(+1, {0, 0, 0});
            pe.add_particle(-1, {1, 0, 0});
            pe.add_particle(+1, {10, 0, 0});
            pe.add_particle(-1, {11, 0, 0});
            const int survivor = pe.add_particle(0, {30, 0, 0});
            pe.particles()[survivor].pair_id = 0;
        };

        ParticleEngine first;
        ParticleEngine second;
        seed(first);
        seed(second);
        first.tick();
        second.tick();

        const auto& a = first.event_history();
        const auto& b = second.event_history();
        const bool same_events = a.size() == 2 && b.size() == 2
            && a[0].participant_a == b[0].participant_a
            && a[0].participant_b == b[0].participant_b
            && a[1].participant_a == b[1].participant_a
            && a[1].participant_b == b[1].participant_b;
        check("PE15a: two disjoint contacts are selected", a.size() == 2);
        check("PE15b: selected event ordering is replay deterministic", same_events);
        check("PE15c: deterministic pairs retain stable source ids",
              a.size() == 2 && a[0].participant_a == 0 && a[0].participant_b == 1
              && a[1].participant_a == 2 && a[1].participant_b == 3);
        check("PE15d: removed pair ids are cleared on survivors",
              first.particles().size() == 1 && first.particles()[0].id == 4
              && first.particles()[0].pair_id == -1);
        check("PE15e: multi-event batch does not overclaim per-event accounting",
              a.size() == 2 && !a[0].accounting_complete && !a[1].accounting_complete);
    }

    // ---- PE16: Complete transaction rollback ----
    {
        std::cout << "\n--- PE16: Complete transaction rollback ---\n";
        ParticleEngine pe;
        pe.set_damping_enabled(false);
        pe.toggles.coulomb = false;
        pe.set_dt(1.0e308);
        pe.add_particle(0, {1.7e308, 0, 0}, {0.5, 0, 0});
        const Particle before = pe.particles()[0];

        bool rolled_back = false;
        try {
            pe.tick();
        } catch (const std::logic_error&) {
            rolled_back = true;
        }
        const auto& after = pe.particles()[0];
        check("PE16a: invalid post-state fails the tick", rolled_back);
        check("PE16b: failed tick does not advance the global tick", pe.current_tick() == 0);
        check("PE16c: failed tick restores the particle record",
              after.position.x == before.position.x
              && after.velocity.x == before.velocity.x
              && after.momentum.x == before.momentum.x);
        check("PE16d: failed tick restores event and sink ledgers",
              pe.event_history().empty()
              && pe.diagnostics().contact_event_count == 0
              && pe.diagnostics().speed_projection_count == 0);
    }

    // ---- PE17: Perfect-insulator volume and explicit ports ----
    {
        std::cout << "\n--- PE17: Perfect-insulator volume and ports ---\n";

        ParticleEngine reflected;
        reflected.toggles.coulomb = false;
        reflected.set_dt(10.0);
        reflected.configure_insulating_box({0, 0, 0}, {2, 2, 2});
        reflected.add_particle(0, {0, 0, 0}, {0.5, 0, 0}, K_B, 0.1);
        const double speed_before = reflected.particles()[0].velocity.mag();
        reflected.tick();
        const auto& reflected_particle = reflected.particles()[0];
        const auto reflected_diag = reflected.diagnostics();
        std::cout << "    reflected x=" << reflected_particle.position.x
                  << " vx=" << reflected_particle.velocity.x
                  << " collisions=" << reflected_diag.insulator_collision_count
                  << " impulse_x=" << reflected_diag.cumulative_insulator_impulse.x << "\n";
        check("PE17a: inside particle reflects rather than tunneling out",
              std::abs(reflected_particle.position.x + 1.0) < 1e-8
              && reflected_particle.velocity.x < 0.0);
        check("PE17b: specular wall preserves particle speed",
              std::abs(reflected_particle.velocity.mag() - speed_before) < 1e-14);
        check("PE17c: wall collision and impulse are accounted",
              reflected_diag.insulator_collision_count == 1
              && reflected_diag.cumulative_insulator_impulse.x < 0.0);

        ParticleEngine exterior;
        exterior.toggles.coulomb = false;
        exterior.set_dt(4.0);
        exterior.configure_insulating_box({0, 0, 0}, {2, 2, 2});
        exterior.add_particle(0, {3, 0, 0}, {-0.5, 0, 0}, K_B, 0.1);
        exterior.tick();
        check("PE17d: exterior particle cannot enter through a solid wall",
              std::abs(exterior.particles()[0].position.x - 3.0) < 1e-8
              && exterior.particles()[0].velocity.x > 0.0);

        ParticleEngine port;
        port.toggles.coulomb = false;
        port.set_dt(10.0);
        port.configure_insulating_box({0, 0, 0}, {2, 2, 2});
        port.add_insulating_port(0, +1, 0.0, 0.0, 0.75, 0.75);
        port.add_particle(0, {0, 0, 0}, {0.5, 0, 0}, K_B, 0.1);
        port.tick();
        const auto port_diag = port.diagnostics();
        check("PE17e: centered particle passes the declared terminal port",
              port.particles()[0].position.x > 4.9
              && port.particles()[0].velocity.x > 0.0);
        check("PE17f: port passage is distinct from wall collision",
              port_diag.insulator_port_crossing_count == 1
              && port_diag.insulator_collision_count == 0);

        ParticleEngine aperture_clearance;
        aperture_clearance.toggles.coulomb = false;
        aperture_clearance.set_dt(10.0);
        aperture_clearance.configure_insulating_box({0, 0, 0}, {2, 2, 2});
        aperture_clearance.add_insulating_port(0, +1, 0.0, 0.0, 0.75, 0.75);
        aperture_clearance.add_particle(0, {0, 0.7, 0}, {0.5, 0, 0}, K_B, 0.1);
        aperture_clearance.tick();
        std::cout << "    edge-clearance x=" << aperture_clearance.particles()[0].position.x
                  << " vx=" << aperture_clearance.particles()[0].velocity.x
                  << " collisions=" << aperture_clearance.diagnostics().insulator_collision_count
                  << " crossings=" << aperture_clearance.diagnostics().insulator_port_crossing_count
                  << "\n";
        check("PE17g: finite-radius particle cannot clip a port edge",
              aperture_clearance.particles()[0].position.x < 0.0
              && aperture_clearance.diagnostics().insulator_collision_count == 1);

        bool rejected_port_without_box = false;
        try {
            ParticleEngine invalid;
            invalid.add_insulating_port(0, 1, 0, 0, 1, 1);
        } catch (const std::logic_error&) {
            rejected_port_without_box = true;
        }
        check("PE17h: a port cannot exist without its insulating volume",
              rejected_port_without_box);

        ParticleEngine outlet;
        outlet.toggles.coulomb = false;
        outlet.set_dt(10.0);
        outlet.configure_insulating_box({0, 0, 0}, {2, 2, 2});
        outlet.add_insulating_port(0, +1, 0, 0, 0.75, 0.75, -1, +1);
        outlet.add_particle(-1, {0, 0, 0}, {0.5, 0, 0}, K_B, 0.1);
        outlet.tick();
        check("PE17i: electron exits an electron-only outlet",
              outlet.particles()[0].position.x > 4.9
              && outlet.diagnostics().insulator_port_crossing_count == 1);

        ParticleEngine wrong_charge;
        wrong_charge.toggles.coulomb = false;
        wrong_charge.set_dt(10.0);
        wrong_charge.configure_insulating_box({0, 0, 0}, {2, 2, 2});
        wrong_charge.add_insulating_port(0, +1, 0, 0, 0.75, 0.75, -1, +1);
        wrong_charge.add_particle(+1, {0, 0, 0}, {0.5, 0, 0}, K_B, 0.1);
        wrong_charge.tick();
        check("PE17j: positive carrier reflects from an electron-only outlet",
              wrong_charge.particles()[0].position.x < 0.0
              && wrong_charge.diagnostics().insulator_collision_count == 1);

        ParticleEngine wrong_direction;
        wrong_direction.toggles.coulomb = false;
        wrong_direction.set_dt(4.0);
        wrong_direction.configure_insulating_box({0, 0, 0}, {2, 2, 2});
        wrong_direction.add_insulating_port(0, +1, 0, 0, 0.75, 0.75, -1, +1);
        wrong_direction.add_particle(-1, {3, 0, 0}, {-0.5, 0, 0}, K_B, 0.1);
        wrong_direction.tick();
        check("PE17k: electron cannot enter through an outlet",
              wrong_direction.particles()[0].position.x > 2.9
              && wrong_direction.diagnostics().insulator_collision_count == 1);

        ParticleEngine inlet;
        inlet.toggles.coulomb = false;
        inlet.set_dt(4.0);
        inlet.configure_insulating_box({0, 0, 0}, {2, 2, 2});
        inlet.add_insulating_port(0, +1, 0, 0, 0.75, 0.75, -1, -1);
        inlet.add_particle(-1, {3, 0, 0}, {-0.5, 0, 0}, K_B, 0.1);
        inlet.tick();
        check("PE17l: electron enters through an electron inlet",
              inlet.particles()[0].position.x < 1.1
              && inlet.diagnostics().insulator_port_crossing_count == 1);
    }

    // ---- PE18: Shared observation cache invalidation ----
    {
        std::cout << "\n--- PE18: Shared observation cache invalidation ---\n";
        ParticleEngine pe;
        pe.add_particle(+1, {-1, 0, 0}, {}, K_B, 0.1);
        pe.add_particle(-1, {+1, 0, 0}, {}, K_B, 0.1);

        const auto first = pe.diagnostics();
        const auto repeated = pe.diagnostics();
        check("PE18a: repeated diagnostics preserve the exact state ledger",
              first.total_energy == repeated.total_energy
              && first.total_momentum.x == repeated.total_momentum.x);

        pe.particles()[1].position.x = 3.0;
        const auto mutated = pe.diagnostics();
        check("PE18b: external particle mutation invalidates cached diagnostics",
              mutated.coulomb_pe != first.coulomb_pe);

        const double enabled_force = pe.observation_force_diag()[0].total().mag();
        pe.toggles.coulomb = false;
        const double disabled_force = pe.observation_force_diag()[0].total().mag();
        check("PE18c: direct toggle changes invalidate observed force rows",
              enabled_force > 0.0 && disabled_force == 0.0);

        pe.toggles.coulomb = true;
        const auto observed_snapshot = pe.snapshot("", "test");
        check("PE18d: shared snapshot remains complete after cache refresh",
              observed_snapshot.objects.size() == 2
              && observed_snapshot.particle_count == 2
              && !observed_snapshot.forces.empty());
    }

    // ---- PE19: State-complete deterministic checkpoint/replay ----
    {
        std::cout << "\n--- PE19: Deterministic checkpoint/replay ---\n";
        ParticleEngine pe;
        pe.set_dt(0.2);
        pe.set_softening(0.15);
        pe.toggles.gravity = true;
        pe.configure_insulating_box({0, 0, 0}, {8, 6, 4});
        pe.add_insulating_port(0, +1, 0, 0, 1, 1, -1, +1);
        pe.add_particle(+1, {-2, 0, 0}, {0, 0.02, 0}, 5 * K_B, 0.2);
        pe.add_particle(-1, {+2, 0, 0}, {0, -0.02, 0}, K_B, 0.2);
        pe.run(5);
        const auto saved = pe.checkpoint();
        pe.run(9);
        const auto expected = pe.checkpoint();
        const auto expected_diag = pe.diagnostics();

        std::string restore_error;
        const bool restored = pe.restore_checkpoint(saved, &restore_error);
        pe.run(9);
        const auto replayed = pe.checkpoint();
        const auto replayed_diag = pe.diagnostics();
        bool particle_match = replayed.particles.size() == expected.particles.size();
        if (particle_match) {
            for (std::size_t i = 0; i < replayed.particles.size(); ++i) {
                const auto& a = replayed.particles[i];
                const auto& b = expected.particles[i];
                particle_match &= a.id == b.id
                    && a.position.x == b.position.x
                    && a.position.y == b.position.y
                    && a.position.z == b.position.z
                    && a.velocity.x == b.velocity.x
                    && a.velocity.y == b.velocity.y
                    && a.velocity.z == b.velocity.z
                    && a.momentum.x == b.momentum.x
                    && a.prev_acceleration.x == b.prev_acceleration.x;
            }
        }
        check("PE19a: a valid checkpoint restores successfully",
              restored && restore_error.empty());
        check("PE19b: replay reproduces the exact particle transaction history",
              replayed.tick == expected.tick && particle_match);
        check("PE19c: replay reproduces exact diagnostics and ledgers",
              replayed_diag.total_energy == expected_diag.total_energy
              && replayed_diag.total_momentum.x == expected_diag.total_momentum.x
              && replayed.insulator_collision_count == expected.insulator_collision_count
              && replayed.events.size() == expected.events.size());

        auto invalid = saved;
        invalid.particles.push_back(invalid.particles.front());
        const int tick_before_rejection = pe.current_tick();
        check("PE19d: duplicate-id checkpoint fails closed",
              !pe.restore_checkpoint(invalid, &restore_error)
              && !restore_error.empty()
              && pe.current_tick() == tick_before_rejection);
    }

    // ---- Summary ----
    std::cout << "\n============================================================\n";
    std::cout << "  ParticleEngine: " << pass_count << " passed, " << fail_count << " failed\n";
    std::cout << "============================================================\n";

    return fail_count;
}

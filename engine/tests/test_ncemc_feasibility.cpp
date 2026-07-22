/**
 * FTD-0405: Native Confinement Energy-Momentum Contract feasibility.
 *
 * Diagnostic-only instrument. Production physics and APIs are unchanged.
 */

#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"

#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>

namespace {

using ftd::C_SPEED;
using ftd::EnergyAudit;
using ftd::M_INERTIAL;
using ftd::RenderBridge;
using ftd::Vec3;

constexpr int L = 33;
constexpr int X_LEFT = 8;
constexpr int X_RIGHT = 24;
constexpr int Y = 16;
constexpr int Z = 16;
constexpr double R0 = 16.0;
constexpr double HARMONIC_DENOM = 128.0;

bool same_bits(double a, double b) {
    return std::memcmp(&a, &b, sizeof(double)) == 0;
}

struct Observation {
    double force_left = 0.0;
    double force_right = 0.0;
    double velocity_left = 0.0;
    double velocity_right = 0.0;
    double remainder_left = 0.0;
    double remainder_right = 0.0;
    double r_effective = 0.0;
    double particle_ke = 0.0;
    double delta_potential = 0.0;
    double work_residual = 0.0;
    double momentum_left = 0.0;
    double momentum_right = 0.0;
    Vec3 total_momentum;
    int manifested = 0;
    int projections = 0;
    bool sites_persist = false;
};

void configure(RenderBridge& rb, bool color_force) {
    rb.force_cpu();
    rb.set_dt(1.0);
    rb.toggles.disable_all();
    rb.toggles.forces = true;
    rb.toggles.movement = true;
    rb.toggles.color_forces = color_force;
    rb.inject_particle(X_LEFT, Y, Z, +1, {}, +1, 1);
    rb.inject_particle(X_RIGHT, Y, Z, +1, {}, -1, 2);
}

Observation run_arm(bool color_force) {
    RenderBridge rb(L);
    configure(rb, color_force);
    rb.tick();

    const auto& left = rb.voxel_at(X_LEFT, Y, Z);
    const auto& right = rb.voxel_at(X_RIGHT, Y, Z);
    const EnergyAudit audit = rb.energy_audit();

    Observation o;
    o.force_left = rb.force_diag_at(X_LEFT, Y, Z).f_strong.x;
    o.force_right = rb.force_diag_at(X_RIGHT, Y, Z).f_strong.x;
    o.velocity_left = left.velocity.x;
    o.velocity_right = right.velocity.x;
    o.remainder_left = left.remainder.x;
    o.remainder_right = right.remainder.x;
    o.r_effective = (X_RIGHT + o.remainder_right)
                  - (X_LEFT + o.remainder_left);
    o.particle_ke = audit.particle_ke;
    o.delta_potential = (o.r_effective * o.r_effective - R0 * R0)
                      / HARMONIC_DENOM;
    o.work_residual = o.particle_ke + o.delta_potential;

    const double gamma_left = ftd::flat_gamma(left.velocity.mag2());
    const double gamma_right = ftd::flat_gamma(right.velocity.mag2());
    o.momentum_left = gamma_left * M_INERTIAL * left.velocity.x;
    o.momentum_right = gamma_right * M_INERTIAL * right.velocity.x;
    o.total_momentum = audit.particle_momentum;
    o.manifested = audit.manifested_count;
    o.projections = rb.causal_projection_events_this_tick();
    o.sites_persist = left.state != 0 && right.state != 0
                   && left.color == 1 && right.color == 2;
    return o;
}

bool same_observation(const Observation& a, const Observation& b) {
    return same_bits(a.force_left, b.force_left)
        && same_bits(a.force_right, b.force_right)
        && same_bits(a.velocity_left, b.velocity_left)
        && same_bits(a.velocity_right, b.velocity_right)
        && same_bits(a.remainder_left, b.remainder_left)
        && same_bits(a.remainder_right, b.remainder_right)
        && same_bits(a.r_effective, b.r_effective)
        && same_bits(a.particle_ke, b.particle_ke)
        && same_bits(a.delta_potential, b.delta_potential)
        && same_bits(a.work_residual, b.work_residual)
        && same_bits(a.momentum_left, b.momentum_left)
        && same_bits(a.momentum_right, b.momentum_right)
        && same_bits(a.total_momentum.x, b.total_momentum.x)
        && same_bits(a.total_momentum.y, b.total_momentum.y)
        && same_bits(a.total_momentum.z, b.total_momentum.z)
        && a.manifested == b.manifested
        && a.projections == b.projections
        && a.sites_persist == b.sites_persist;
}

}  // namespace

int main() {
    ftd::test::init("ncemc_feasibility");
    ftd::test::section("FTD-0405 frozen harmonic two-body anchor");

    const Observation a = run_arm(true);
    const Observation duplicate = run_arm(true);
    const Observation control = run_arm(false);

    ftd::test::check("NC-1 duplicate observation is bit-identical",
                     same_observation(a, duplicate));
    ftd::test::check("NC-2 both colored sites persist", a.sites_persist);
    ftd::test::check("NC-3 manifested count remains two", a.manifested == 2);
    ftd::test::check("NC-4 no causal projection", a.projections == 0);

    ftd::test::check_close("NC-5 left strong force is +1/4",
                           a.force_left, 0.25, 1e-15);
    ftd::test::check_close("NC-6 right strong force is -1/4",
                           a.force_right, -0.25, 1e-15);
    ftd::test::check_close("NC-7 pair force cancels",
                           a.force_left + a.force_right, 0.0, 1e-15);

    ftd::test::check("NC-8 individual velocities are nonzero",
                     a.velocity_left > 0.0 && a.velocity_right < 0.0);
    ftd::test::check_close("NC-9 velocities cancel",
                           a.velocity_left + a.velocity_right, 0.0, 1e-15);
    ftd::test::check_close("NC-10 movement stores the left sub-voxel position",
                           a.remainder_left, a.velocity_left, 1e-15);
    ftd::test::check_close("NC-11 movement stores the right sub-voxel position",
                           a.remainder_right, a.velocity_right, 1e-15);
    ftd::test::check("NC-12 effective separation moved but stayed harmonic",
                     a.r_effective < R0 && a.r_effective >= 8.0);

    ftd::test::check("NC-13 individual momenta are nonzero",
                     a.momentum_left > 0.0 && a.momentum_right < 0.0);
    ftd::test::check_close("NC-14 individual momenta cancel",
                           a.momentum_left + a.momentum_right, 0.0, 1e-15);
    ftd::test::check_close("NC-15 audit total momentum x closes",
                           a.total_momentum.x, 0.0, 1e-15);
    ftd::test::check_close("NC-16 audit total momentum y closes",
                           a.total_momentum.y, 0.0, 1e-15);
    ftd::test::check_close("NC-17 audit total momentum z closes",
                           a.total_momentum.z, 0.0, 1e-15);

    // This is the discriminator, not a desired equality: a nonzero result
    // means the actual force-kick / movement split does not exactly exchange
    // normalized particle KE with the force-derived harmonic potential.
    ftd::test::check("NC-18 work residual is finite",
                     std::isfinite(a.work_residual));
    ftd::test::check("NC-19 exact work exchange fails non-vacuously",
                     std::abs(a.work_residual) > 1e-6);

    ftd::test::check_close("NC-20 force-off left velocity remains zero",
                           control.velocity_left, 0.0, 1e-15);
    ftd::test::check_close("NC-21 force-off right velocity remains zero",
                           control.velocity_right, 0.0, 1e-15);
    ftd::test::check_close("NC-22 force-off left remainder remains zero",
                           control.remainder_left, 0.0, 1e-15);
    ftd::test::check_close("NC-23 force-off right remainder remains zero",
                           control.remainder_right, 0.0, 1e-15);
    ftd::test::check_close("NC-24 force-off kinetic energy remains zero",
                           control.particle_ke, 0.0, 1e-15);

    std::cout << std::setprecision(17)
              << "OBS force_left=" << a.force_left
              << " force_right=" << a.force_right
              << " velocity_left=" << a.velocity_left
              << " velocity_right=" << a.velocity_right
              << " r_effective=" << a.r_effective
              << " particle_ke=" << a.particle_ke
              << " delta_potential=" << a.delta_potential
              << " work_residual=" << a.work_residual
              << " momentum_total_x=" << a.total_momentum.x
              << '\n';

    return ftd::test::finalize();
}

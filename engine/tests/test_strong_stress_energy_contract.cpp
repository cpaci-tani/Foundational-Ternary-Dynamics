/**
 * FTD-0406 frozen owner-authorized strong stress-energy CPU contract.
 */

#include "ftd/render_bridge.h"
#include "ftd/strong_stress_energy.h"
#include "ftd/test_telemetry.h"

#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>

namespace {

using ftd::EnergyAudit;
using ftd::RenderBridge;
using ftd::StrongEnergyStepDiagnostics;
using ftd::StrongStressCell;
using ftd::Vec3;

constexpr int L = 33;
constexpr int Y = 16;
constexpr int Z = 16;

void configure_pair(RenderBridge& rb, bool contract) {
    rb.force_cpu();
    rb.set_dt(1.0);
    rb.toggles.disable_all();
    rb.toggles.forces = true;
    rb.toggles.movement = true;
    rb.toggles.color_forces = true;
    rb.toggles.strong_stress_energy = contract;
    rb.inject_particle(8, Y, Z, +1, {}, +1, 1);
    rb.inject_particle(24, Y, Z, +1, {}, -1, 2);
}

double effective_separation(const RenderBridge& rb) {
    const auto& a = rb.voxels()[rb.lattice().index(8, Y, Z)];
    const auto& b = rb.voxels()[rb.lattice().index(24, Y, Z)];
    return (24.0 + b.remainder.x) - (8.0 + a.remainder.x);
}

struct PairObservation {
    double u0 = 0.0;
    double u1 = 0.0;
    double ke = 0.0;
    double residual = 0.0;
    double separation = 0.0;
    double force_left = 0.0;
    double force_right = 0.0;
    double remainder_left = 0.0;
    double remainder_right = 0.0;
    Vec3 momentum;
    StrongEnergyStepDiagnostics step;
    EnergyAudit audit;
};

PairObservation run_pair(bool contract) {
    RenderBridge rb(L);
    configure_pair(rb, contract);
    PairObservation o;
    o.u0 = ftd::compute_strong_potential_energy(rb);
    rb.tick();
    o.u1 = ftd::compute_strong_potential_energy(rb);
    o.audit = rb.energy_audit();
    o.ke = o.audit.particle_ke;
    o.residual = o.ke + o.u1 - o.u0;
    o.separation = effective_separation(rb);
    o.force_left = rb.force_diag_at(8, Y, Z).f_strong.x;
    o.force_right = rb.force_diag_at(24, Y, Z).f_strong.x;
    o.remainder_left = rb.voxel_at(8, Y, Z).remainder.x;
    o.remainder_right = rb.voxel_at(24, Y, Z).remainder.x;
    o.momentum = o.audit.particle_momentum;
    o.step = rb.strong_energy_step_diagnostics();
    return o;
}

bool same_bits(double a, double b) {
    return std::memcmp(&a, &b, sizeof(double)) == 0;
}

bool same_pair_observation(const PairObservation& a, const PairObservation& b) {
    return same_bits(a.u0, b.u0)
        && same_bits(a.u1, b.u1)
        && same_bits(a.ke, b.ke)
        && same_bits(a.residual, b.residual)
        && same_bits(a.separation, b.separation)
        && same_bits(a.force_left, b.force_left)
        && same_bits(a.force_right, b.force_right)
        && same_bits(a.remainder_left, b.remainder_left)
        && same_bits(a.remainder_right, b.remainder_right)
        && same_bits(a.momentum.x, b.momentum.x)
        && same_bits(a.step.lambda, b.step.lambda)
        && same_bits(a.step.residual, b.step.residual);
}

struct StressTotals {
    double energy = 0.0;
    double xx = 0.0, yy = 0.0, zz = 0.0;
    double xy = 0.0, xz = 0.0, yz = 0.0;
};

StressTotals stress_totals(const std::vector<StrongStressCell>& cells) {
    StressTotals t;
    for (const auto& c : cells) {
        t.energy += c.energy_density;
        t.xx += c.stress_xx; t.yy += c.stress_yy; t.zz += c.stress_zz;
        t.xy += c.stress_xy; t.xz += c.stress_xz; t.yz += c.stress_yz;
    }
    return t;
}

void configure_static_pair(RenderBridge& rb, int shift, bool contract,
                           bool swap_colors = false) {
    rb.force_cpu();
    rb.toggles.disable_all();
    rb.toggles.color_forces = true;
    rb.toggles.strong_stress_energy = contract;
    rb.inject_particle(8 + shift, Y, Z, +1, {}, +1, swap_colors ? 2 : 1);
    rb.inject_particle(24 + shift, Y, Z, +1, {}, -1, swap_colors ? 1 : 2);
}

double phi_difference_l1(const RenderBridge& a, const RenderBridge& b) {
    const auto& pa = a.phi_latency();
    const auto& pb = b.phi_latency();
    double total = 0.0;
    for (std::size_t i = 0; i < pa.size(); ++i) total += std::abs(pa[i] - pb[i]);
    return total;
}

}  // namespace

int main() {
    ftd::test::init("strong_stress_energy_contract");

    ftd::test::section("FTD-0406 exact potential and vacuum anchors");
    ftd::test::check_close("SE-1 C_SPEED squared is one third",
                           ftd::C_SPEED * ftd::C_SPEED, 1.0 / 3.0, 1e-15);
    ftd::test::check_close("SE-2 E_REST equals M_INERTIAL C_SPEED squared",
                           ftd::E_REST,
                           ftd::M_INERTIAL * ftd::C_SPEED * ftd::C_SPEED,
                           1e-15);
    ftd::test::check_close("SE-3 different-colour U(1) is vacuum zero",
                           ftd::strong_pair_potential(1.0, 1, 2), 0.0, 1e-15);
    ftd::test::check_close("SE-4 same-colour U(1) is vacuum zero",
                           ftd::strong_pair_potential(1.0, 1, 1), 0.0, 1e-15);
    ftd::test::check_close("SE-5 capped harmonic different-colour delta U",
                           ftd::strong_pair_potential(16.0, 1, 2)
                         - ftd::strong_pair_potential(8.0, 1, 2),
                           1.5, 1e-14);
    ftd::test::check_close("SE-6 capped harmonic same-colour delta U",
                           ftd::strong_pair_potential(16.0, 1, 1)
                         - ftd::strong_pair_potential(8.0, 1, 1),
                           -0.75, 1e-14);

    RenderBridge empty(L);
    empty.force_cpu();
    empty.toggles.disable_all();
    empty.toggles.strong_stress_energy = true;
    ftd::test::check_close("SE-7 empty coloured-pair set has zero energy",
                           ftd::compute_strong_potential_energy(empty), 0.0, 1e-15);
    ftd::test::check_close("SE-8 empty local T00 integrates to zero",
                           stress_totals(empty.strong_stress_cells()).energy, 0.0, 1e-15);

    ftd::test::section("FTD-0405 legacy witness and projected pair");
    const PairObservation legacy = run_pair(false);
    const PairObservation selected = run_pair(true);
    const PairObservation duplicate = run_pair(true);
    ftd::test::check("SE-9 legacy work residual stays nonzero",
                     std::abs(legacy.residual) > 1e-6);
    ftd::test::check_close("SE-10 legacy left force remains +1/4",
                           legacy.force_left, 0.25, 1e-15);
    ftd::test::check_close("SE-11 selected left force remains +1/4",
                           selected.force_left, 0.25, 1e-15);
    ftd::test::check_close("SE-12 selected right force remains -1/4",
                           selected.force_right, -0.25, 1e-15);
    ftd::test::check("SE-13 proposed positions are retained",
                     same_bits(selected.remainder_left, legacy.remainder_left)
                  && same_bits(selected.remainder_right, legacy.remainder_right));
    ftd::test::check("SE-14 movement is non-vacuous",
                     selected.remainder_left > 0.0 && selected.remainder_right < 0.0);
    ftd::test::check_close("SE-15 projected strong Hamiltonian closes",
                           selected.residual, 0.0, 1e-12);
    ftd::test::check_close("SE-16 step diagnostic Hamiltonian closes",
                           selected.step.residual, 0.0, 1e-12);
    ftd::test::check_close("SE-17 projected total momentum x closes",
                           selected.momentum.x, 0.0, 1e-12);
    ftd::test::check("SE-18 projection is nontrivial",
                     selected.step.projection_events == 1
                  && selected.step.projected_particles == 2
                  && std::abs(selected.step.lambda - 1.0) > 1e-6);
    ftd::test::check("SE-19 projected run reports no failure",
                     selected.step.projection_failures == 0
                  && selected.step.topology_failures == 0);
    ftd::test::check("SE-20 duplicate selected run is bit-identical",
                     same_pair_observation(selected, duplicate));
    ftd::test::check_close("SE-21 audit uses the same strong potential",
                           selected.audit.strong_potential_energy,
                           selected.u1, 1e-14);
    ftd::test::check_close("SE-22 audit converts strong energy with derived c",
                           selected.audit.strong_gravitational_mass,
                           selected.u1 / (ftd::C_SPEED * ftd::C_SPEED), 1e-13);

    ftd::test::section("Local string T00 and central stress");
    RenderBridge static_pair(L), translated(L), swapped(L);
    configure_static_pair(static_pair, 0, true);
    configure_static_pair(translated, 3, true);
    configure_static_pair(swapped, 0, true, true);
    const StressTotals base_stress = stress_totals(static_pair.strong_stress_cells());
    const StressTotals translated_stress = stress_totals(translated.strong_stress_cells());
    const StressTotals swapped_stress = stress_totals(swapped.strong_stress_cells());
    const double static_u = ftd::compute_strong_potential_energy(static_pair);
    ftd::test::check_close("SE-23 local T00 integrates to pair U",
                           base_stress.energy, static_u, 1e-12);
    ftd::test::check_close("SE-24 integer translation preserves integrated T00",
                           translated_stress.energy, base_stress.energy, 1e-12);
    ftd::test::check_close("SE-25 endpoint exchange preserves integrated T00",
                           swapped_stress.energy, base_stress.energy, 1e-12);
    ftd::test::check_close("SE-26 translation preserves xx stress",
                           translated_stress.xx, base_stress.xx, 1e-12);
    ftd::test::check_close("SE-27 endpoint exchange preserves xx stress",
                           swapped_stress.xx, base_stress.xx, 1e-12);
    ftd::test::check_close("SE-28 axial string has zero transverse stress",
                           std::abs(base_stress.yy) + std::abs(base_stress.zz)
                         + std::abs(base_stress.xy) + std::abs(base_stress.xz)
                         + std::abs(base_stress.yz), 0.0, 1e-14);

    ftd::test::section("Static strong T00 sources CPU latency as mass");
    RenderBridge gravity_on(L), gravity_control(L);
    configure_static_pair(gravity_on, 0, true);
    configure_static_pair(gravity_control, 0, false);
    for (RenderBridge* rb : {&gravity_on, &gravity_control}) {
        rb->toggles.gravity = true;
        rb->toggles.latency_field = true;
        rb->set_sor_iterations(30);
        rb->tick();
    }
    const auto gravity_audit = gravity_on.energy_audit();
    ftd::test::check("SE-29 selected strong source changes latency potential",
                     phi_difference_l1(gravity_on, gravity_control) > 1e-8);
    ftd::test::check_close("SE-30 integrated added gravitational mass is U/c squared",
                           gravity_audit.strong_gravitational_mass,
                           gravity_audit.strong_potential_energy
                         / (ftd::C_SPEED * ftd::C_SPEED), 1e-12);

    ftd::test::section("Three-body non-vacuity and failure honesty");
    RenderBridge triad(L);
    triad.force_cpu();
    triad.toggles.disable_all();
    triad.toggles.forces = true;
    triad.toggles.movement = true;
    triad.toggles.color_forces = true;
    triad.toggles.strong_stress_energy = true;
    triad.inject_particle(8, 16, 16, +1, {}, +1, 1);
    triad.inject_particle(24, 16, 16, +1, {}, -1, 2);
    triad.inject_particle(16, 24, 16, +1, {}, +1, 3);
    const double triad_h0 = triad.energy_audit().particle_ke
                          + ftd::compute_strong_potential_energy(triad);
    triad.tick();
    const auto triad_audit = triad.energy_audit();
    const double triad_h1 = triad_audit.particle_ke
                          + triad_audit.strong_potential_energy;
    ftd::test::check_close("SE-31 three-body Hamiltonian closes",
                           triad_h1 - triad_h0, 0.0, 1e-12);
    ftd::test::check_close("SE-32 three-body total momentum closes",
                           triad_audit.particle_momentum.mag(), 0.0, 1e-12);
    ftd::test::check("SE-33 three-body projection is non-vacuous",
                     triad.strong_energy_step_diagnostics().projection_events == 1
                  && triad.strong_energy_step_diagnostics().projected_particles == 3);

    RenderBridge topology(L);
    topology.force_cpu();
    topology.toggles.disable_all();
    topology.toggles.forces = true;
    topology.toggles.movement = true;
    topology.toggles.color_forces = true;
    topology.toggles.strong_stress_energy = true;
    topology.inject_particle(0, Y, Z, +1, {}, +1, 1);
    topology.inject_particle(16, Y, Z, +1, {}, -1, 2);
    auto& escaping = topology.voxel_at(0, Y, Z);
    escaping.remainder.x = -0.8;
    escaping.velocity.x = -0.55;
    topology.tick();
    ftd::test::check("SE-34 topology change is surfaced",
                     topology.strong_energy_step_diagnostics().topology_failures == 1
                  && topology.strong_energy_step_diagnostics().projection_events == 0);

    RenderBridge infeasible(L);
    configure_pair(infeasible, true);
    infeasible.toggles.gravity = true; // explicitly outside the frozen projection domain
    infeasible.tick();
    ftd::test::check("SE-35 ineligible mixed-force projection is surfaced",
                     infeasible.strong_energy_step_diagnostics().projection_failures == 1
                  && infeasible.strong_energy_step_diagnostics().projection_events == 0);

    std::cout << std::setprecision(17)
              << "OBS legacy_residual=" << legacy.residual
              << " projected_residual=" << selected.residual
              << " lambda=" << selected.step.lambda
              << " separation=" << selected.separation
              << " local_energy=" << base_stress.energy
              << " gravity_mass=" << gravity_audit.strong_gravitational_mass
              << " triad_residual=" << (triad_h1 - triad_h0)
              << " topology_failures=" << topology.strong_energy_step_diagnostics().topology_failures
              << " projection_failures=" << infeasible.strong_energy_step_diagnostics().projection_failures
              << '\n';

    return ftd::test::finalize();
}

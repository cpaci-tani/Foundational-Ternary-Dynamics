/**
 * Diagnostics — implementation.
 * Extracted from render_bridge.cpp, 2026-04-18 refactor ticket R4.
 */

#include "ftd/diagnostics_compute.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/volumetric_measure.h"
#include <cmath>

namespace ftd {

double compute_entropy_cpu(const RenderBridge& rb) {
  const auto& voxels = rb.voxels();
  const auto& lattice = rb.lattice();
  const int N = static_cast<int>(lattice.total_sites());
  double total_mag2 = 0.0;
  for (int i = 0; i < N; ++i) total_mag2 += voxels[i].flux.mag2();
  if (total_mag2 < EPSILON_FLUX_SQ) return 0.0;
  double entropy = 0.0;
  for (int i = 0; i < N; ++i) {
    double p = voxels[i].flux.mag2() / total_mag2;
    if (p > EPSILON_FLUX_SQ) entropy -= p * std::log(p);
  }
  return entropy;
}

Diagnostics compute_diagnostics(const RenderBridge& rb) {
  Diagnostics d;
  d.tick = rb.current_tick();
  const auto& voxels = rb.voxels();
  const auto& lattice = rb.lattice();
  const auto& ternary = rb.ternary_field();
  const auto& active = ternary.ordered_active_indices();
  const int N = static_cast<int>(lattice.total_sites());

  for (int i = 0; i < N; ++i) {
    const auto &v = voxels[i];
    d.total_flux += v.density();
    d.total_energy += std::abs(v.born_infeld_core());
    double bw = v.bandwidth_used();
    if (bw > d.max_bandwidth) d.max_bandwidth = bw;
    double budget = v.causal_budget();
    if (budget > d.max_causal_budget) d.max_causal_budget = budget;
  }
  d.manifested_count = ternary.manifested_count();
  d.positive_count = ternary.positive_count();
  d.negative_count = ternary.negative_count();
  for (int i : active) {
    const auto& v = voxels[i];
    if (v.spin > 0) d.spin_up_count++;
    else if (v.spin < 0) d.spin_down_count++;
    if (v.color >= 0 && v.color <= 3) d.color_count[v.color]++;
  }

  d.total_entropy = compute_entropy_cpu(rb);
  d.causal_projection_events = rb.causal_projection_events_this_tick();

  Vec3 r_cm;
  const int n_manifested = static_cast<int>(active.size());
  for (int i : active) {
    Coord c = lattice.coord(i);
    r_cm.x += c.x;
    r_cm.y += c.y;
    r_cm.z += c.z;
  }
  if (n_manifested > 0) {
    r_cm *= (1.0 / n_manifested);
    Vec3 L_total;
    for (int i : active) {
      Coord c = lattice.coord(i);
      double rx = c.x - r_cm.x, ry = c.y - r_cm.y, rz = c.z - r_cm.z;
      const auto& vel = voxels[i].velocity;
      L_total.x += ry * vel.z - rz * vel.y;
      L_total.y += rz * vel.x - rx * vel.z;
      L_total.z += rx * vel.y - ry * vel.x;
    }
    d.total_angular_momentum = L_total;
  }
  return d;
}

EnergyAudit compute_energy_audit(const RenderBridge& rb) {
  EnergyAudit a;
  const auto& voxels = rb.voxels();
  const auto& lattice = rb.lattice();
  const auto& ternary = rb.ternary_field();
  const auto& active = ternary.ordered_active_indices();
  const auto& phi_coulomb = rb.phi_coulomb();
  const int N = static_cast<int>(lattice.total_sites());

  // gauss_violation must mirror the constraint the projection actually
  // enforces (poisson_solvers.cpp gauss_project_cpu): the SOR correction
  // skips manifested sites and targets div(J) = charge_coupling·(s − mean_charge).
  // So the residual is meaningful ONLY at vacuum (state==0) sites, where the
  // target is charge_coupling·(0 − mean_charge). Summing over manifested sites
  // (which the projection never corrects) inflated the metric. Match the source
  // sign convention div(J) = +charge_coupling·(s − mean_charge).
  const double charge_coupling = rb.toggles.coulomb_charge_coupling;
  const double mean_charge =
      static_cast<double>(ternary.charge_sum()) / static_cast<double>(N);

  for (int i = 0; i < N; ++i) {
    const auto &v = voxels[i];
    // Field energies follow the canonical ½·|·|² convention used by
    // engine/src/lagrangian.cpp and engine/web/js/bridge/mock-diagnostics.js.
    // Pre-2026-04-27 this site dropped the ½ on field_energy / wave_energy,
    // making MockBridge report half the WasmBridge value for the SAME
    // scenario — the Energy Budget chart and Lagrangian readout silently
    // jumped 2× when the user switched bridges.
    //
    // NAMING (2026-08-21, boson-sector red-team): field_energy is the flux /
    // vector-potential POTENTIAL energy ½Σ|J|², NOT the electric-field energy.
    // (A≡J here, so the actual E-field energy is the separate E_field_energy
    // channel below.) wave_energy is the ½Σ|wave_vel|² channel. Do NOT read
    // "field_energy vs B_field_energy" as "|E|² vs |B|²".
    const double field_density = quadratic_field_energy_density(v.flux.mag2());
    const double wave_density = quadratic_field_energy_density(v.wave_vel.mag2());
    a.field_energy_density_sum += field_density;
    a.wave_energy_density_sum += wave_density;
    a.field_energy += integrate_voxel_density(field_density);
    a.wave_energy  += integrate_voxel_density(wave_density);

    Vec3 E = v.wave_vel * -1.0;
    Vec3 B = rb.curl_flux(i);
    // P2 (2026-07-26): the magnetic channel carries c^2.
    //
    // With J acting as the vector potential and the engine's own wave equation
    // d^2J/dt^2 = c^2 grad^2 J, the energy consistent with the engine's
    // Hamiltonian is  1/2|E|^2 + (c^2/2)|B|^2,  and the flux is c^2 (E x B).
    // The c^2 was dropped here while lagrangian.h:145 and
    // test_em_energy_conservation.cpp both carry it, so for a pure transverse
    // wave this reported a FIXED B/E ratio of 1/c^2 = 3 (measured 2.997 /
    // 3.050 / 3.079 for n = 1,2,4 at L=32) and rendered it in the browser as
    // two bars in the same unit. Diagnostic-only: nothing in the tick cycle
    // reads these fields.
    constexpr double C2 = C_SPEED * C_SPEED;
    // E = -wave_vel (assigned above), so E.mag2() == wave_vel.mag2() and
    // E_field_energy is BYTE-IDENTICAL to wave_energy by construction. It is a
    // separate channel only so the dashboard can pair it with B_field_energy;
    // it is not an independent measurement of a distinct energy reservoir.
    a.E_field_energy += integrate_voxel_density(
        quadratic_field_energy_density(E.mag2()));
    a.B_field_energy += integrate_voxel_density(
        C2 * quadratic_field_energy_density(B.mag2()));

    a.total_poynting.x += integrate_voxel_density(C2 * (E.y * B.z - E.z * B.y));
    a.total_poynting.y += integrate_voxel_density(C2 * (E.z * B.x - E.x * B.z));
    a.total_poynting.z += integrate_voxel_density(C2 * (E.x * B.y - E.y * B.x));

    if (rb.toggles.dual_substrate) {
      // Split flux-channel and wave-channel energies separately so
      // the dashboard's Dual Substrate panel can render them as
      // distinct columns (E_L / E_R = flux; Wave L / R = wave_vel).
      // Same ½·|·|² convention as field_energy / wave_energy above.
      a.E_L_total += integrate_voxel_density(
          quadratic_field_energy_density(v.flux_L.mag2()));
      a.E_R_total += integrate_voxel_density(
          quadratic_field_energy_density(v.flux_R.mag2()));
      a.wv_L_total += integrate_voxel_density(
          quadratic_field_energy_density(v.wave_vel_L.mag2()));
      a.wv_R_total += integrate_voxel_density(
          quadratic_field_energy_density(v.wave_vel_R.mag2()));
      a.chirality_total += integrate_voxel_density(v.chirality_density());
    }

    // Keep the public substrate-energy channels populated on every backend.
    // GpuEngine has always reported these fields; the extracted CPU audit
    // accidentally omitted them, making a backend switch zero the native
    // strong/weak energy bars for an identical lattice snapshot.
    if (rb.toggles.color_forces || rb.toggles.strong_force) {
      a.strong_energy += integrate_voxel_density(
          quadratic_field_energy_density(v.flux_strong.mag2()));
    }
    a.weak_energy += integrate_voxel_density(
        quadratic_field_energy_density(v.flux_weak.mag2()));

    const int8_t s = ternary.state_at(i);
    if (s != 0) {
      const double speed2 = v.velocity.mag2();
      const double gamma0 = flat_gamma(speed2);
      const double kinetic = flat_particle_kinetic_energy(speed2);
      a.particle_ke += kinetic;
      a.particle_rest_energy += E_REST;
      a.particle_momentum += v.velocity * (gamma0 * M_INERTIAL);
      a.charge_total += s;
      a.manifested_count++;
    }

    // Constrained-site Gauss residual: only vacuum (state==0) sites are
    // projected, with target source charge_coupling·(s − mean_charge).
    if (s == 0) {
      double err = rb.divergence_flux(i)
                 - charge_coupling * (static_cast<double>(s) - mean_charge);
      a.gauss_violation += err * err;
      double abs_err = std::abs(err);
      if (abs_err > a.max_gauss_error) a.max_gauss_error = abs_err;
    }
  }

  a.particle_energy = a.particle_rest_energy + a.particle_ke;
  a.dynamic_energy = a.field_energy + a.wave_energy + a.particle_ke;
  a.total_energy = a.field_energy + a.wave_energy + a.particle_energy;
  if (rb.toggles.strong_stress_energy) {
    a.strong_potential_energy = compute_strong_potential_energy(rb);
    a.strong_gravitational_mass = a.strong_potential_energy
                                / (C_SPEED * C_SPEED);
    const auto& step = rb.strong_energy_step_diagnostics();
    a.strong_projection_residual = step.residual;
    a.strong_projection_lambda = step.lambda;
    a.strong_projection_events = step.projection_events;
    a.strong_projection_failures = step.projection_failures;
    a.strong_topology_failures = step.topology_failures;
    a.dynamic_energy += a.strong_potential_energy;
    a.total_energy += a.strong_potential_energy;
  }
  // self_field_injection_ is a private member; RenderBridge::energy_audit()
  // wrapper exposes it via the friend relationship below.

  if (!phi_coulomb.empty()) {
    // Standard pair-potential convention: U = ½·Σ_i α·q_i·φ_i, equivalent
    // to Σ_{i<j} α·q_i·q_j/r_ij — the ½ avoids double-counting each pair.
    // MockBridge (mock-diagnostics.js) uses the i<j form natively and
    // therefore needs no explicit ½. Pre-2026-04-27 this site reported 2×
    // the physical Coulomb PE because the ½ was missing.
    for (int i : active) {
      const int8_t s = ternary.state_at(i);
      if (s != 0)
        a.coulomb_pe += 0.5 * ALPHA * s * phi_coulomb[i];
    }
  }

  return a;
}

EMFieldDiag compute_em_field_at(const RenderBridge& rb, int idx) {
  EMFieldDiag em;
  em.E = rb.voxels()[idx].wave_vel * -1.0;
  em.B = rb.curl_flux(idx);
  em.E_mag = em.E.mag();
  em.B_mag = em.B.mag();
  return em;
}

Vec3 compute_poynting_vector(const RenderBridge& rb, int idx) {
  Vec3 E = rb.voxels()[idx].wave_vel * -1.0;
  Vec3 B = rb.curl_flux(idx);
  constexpr double C2 = C_SPEED * C_SPEED;
  return Vec3{
    C2 * (E.y * B.z - E.z * B.y),
    C2 * (E.z * B.x - E.x * B.z),
    C2 * (E.x * B.y - E.y * B.x)
  };
}

}  // namespace ftd

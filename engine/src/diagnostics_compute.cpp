/**
 * Diagnostics — implementation.
 * Extracted from render_bridge.cpp, 2026-04-18 refactor ticket R4.
 */

#include "ftd/diagnostics_compute.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
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
  const int N = static_cast<int>(lattice.total_sites());

  for (int i = 0; i < N; ++i) {
    const auto &v = voxels[i];
    d.total_flux += v.density();
    d.total_energy += std::abs(v.born_infeld_core());
    double bw = v.bandwidth_used();
    if (bw > d.max_bandwidth) d.max_bandwidth = bw;

    if (v.state != 0) {
      d.manifested_count++;
      if (v.state > 0) d.positive_count++;
      else d.negative_count++;
      if (v.spin > 0) d.spin_up_count++;
      else if (v.spin < 0) d.spin_down_count++;
      if (v.color >= 0 && v.color <= 3) d.color_count[v.color]++;
    }
  }

  d.total_entropy = compute_entropy_cpu(rb);

  Vec3 r_cm;
  int n_manifested = 0;
  for (int i = 0; i < N; ++i) {
    if (voxels[i].state != 0) {
      Coord c = lattice.coord(i);
      r_cm.x += c.x;
      r_cm.y += c.y;
      r_cm.z += c.z;
      n_manifested++;
    }
  }
  if (n_manifested > 0) {
    r_cm *= (1.0 / n_manifested);
    Vec3 L_total;
    for (int i = 0; i < N; ++i) {
      if (voxels[i].state != 0) {
        Coord c = lattice.coord(i);
        double rx = c.x - r_cm.x, ry = c.y - r_cm.y, rz = c.z - r_cm.z;
        const auto& vel = voxels[i].velocity;
        L_total.x += ry * vel.z - rz * vel.y;
        L_total.y += rz * vel.x - rx * vel.z;
        L_total.z += rx * vel.y - ry * vel.x;
      }
    }
    d.total_angular_momentum = L_total;
  }
  return d;
}

EnergyAudit compute_energy_audit(const RenderBridge& rb) {
  EnergyAudit a;
  const auto& voxels = rb.voxels();
  const auto& lattice = rb.lattice();
  const auto& phi_coulomb = rb.phi_coulomb();
  const int N = static_cast<int>(lattice.total_sites());

  for (int i = 0; i < N; ++i) {
    const auto &v = voxels[i];
    a.field_energy += v.flux.mag2();
    a.wave_energy  += v.wave_vel.mag2();

    Vec3 E = v.wave_vel * -1.0;
    Vec3 B = rb.curl_flux(i);
    a.E_field_energy += 0.5 * E.mag2();
    a.B_field_energy += 0.5 * B.mag2();

    a.total_poynting.x += E.y * B.z - E.z * B.y;
    a.total_poynting.y += E.z * B.x - E.x * B.z;
    a.total_poynting.z += E.x * B.y - E.y * B.x;

    if (rb.toggles.dual_substrate) {
      // Split flux-channel and wave-channel energies separately so
      // the dashboard's Dual Substrate panel can render them as
      // distinct columns (E_L / E_R = flux; Wave L / R = wave_vel).
      a.E_L_total += v.flux_L.mag2();
      a.E_R_total += v.flux_R.mag2();
      a.wv_L_total += v.wave_vel_L.mag2();
      a.wv_R_total += v.wave_vel_R.mag2();
      a.chirality_total += v.chirality_density();
    }

    if (v.state != 0) {
      a.particle_ke += 0.5 * v.velocity.mag2();
      a.charge_total += v.state;
      a.manifested_count++;
    }

    double err = rb.divergence_flux(i) - static_cast<double>(v.state);
    a.gauss_violation += err * err;
    double abs_err = std::abs(err);
    if (abs_err > a.max_gauss_error) a.max_gauss_error = abs_err;
  }

  a.total_energy = a.field_energy + a.wave_energy + a.particle_ke;
  // self_field_injection_ is a private member; RenderBridge::energy_audit()
  // wrapper exposes it via the friend relationship below.

  if (!phi_coulomb.empty()) {
    for (int i = 0; i < N; ++i) {
      if (voxels[i].state != 0)
        a.coulomb_pe += ALPHA * voxels[i].state * phi_coulomb[i];
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
  return Vec3{
    E.y * B.z - E.z * B.y,
    E.z * B.x - E.x * B.z,
    E.x * B.y - E.y * B.x
  };
}

}  // namespace ftd

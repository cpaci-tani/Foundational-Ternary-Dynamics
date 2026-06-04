#include "ftd/engine_state.h"
#include "ftd/field_operators.h"
#include "ftd/render_bridge.h"
#include <cassert>
#include <cmath>

namespace {

void assert_vec_eq(const ftd::Vec3& a, const ftd::Vec3& b) {
  assert(a.x == b.x);
  assert(a.y == b.y);
  assert(a.z == b.z);
}

void seed_flux_pattern(ftd::RenderBridge& rb) {
  auto& voxels = rb.voxels();
  const int total = static_cast<int>(voxels.size());
  for (int i = 0; i < total; ++i) {
    const double a = static_cast<double>((i % 11) - 5);
    const double b = static_cast<double>((i % 7) - 3);
    const double c = static_cast<double>((i % 5) - 2);
    voxels[i].flux = {0.125 * a, -0.25 * b, 0.375 * c};
    voxels[i].wave_vel = {-0.0625 * c, 0.03125 * a, -0.015625 * b};
  }
}

}  // namespace

int main() {
  ftd::FieldSoA fields;
  fields.resize_primary(9);
  assert(fields.size() == 9);
  assert(fields.primary_sized());
  assert(fields.optional_empty());

  std::vector<ftd::Voxel> voxels(3);
  voxels[0].flux = {1.0, 2.0, 2.0};
  voxels[0].wave_vel = {0.5, -0.25, 0.125};
  voxels[2].flux = {-3.0, 4.0, 12.0};
  voxels[2].wave_vel = {7.0, 8.0, 9.0};
  fields.rebuild_primary_from_voxels(voxels);
  assert(fields.size() == 3);
  assert(fields.optional_empty());
  assert_vec_eq(fields.flux_at(0), voxels[0].flux);
  assert_vec_eq(fields.wave_vel_at(2), voxels[2].wave_vel);
  assert(fields.density_at(0) == voxels[0].density());
  assert(fields.density_at(2) == voxels[2].density());

  ftd::EngineState state(17);
  assert(state.fields.size() == 17);
  assert(state.fields.primary_sized());
  assert(state.fields.optional_empty());

  ftd::RenderBridge rb(7);
  rb.force_cpu();
  const int idx = rb.lattice().index(1, 1, 1);
  rb.voxel_at(1, 1, 1).flux = {1.0, 2.0, 2.0};
  rb.voxel_at(1, 1, 1).wave_vel = {0.25, 0.5, 0.75};
  assert_vec_eq(rb.flux_at(idx), ftd::Vec3{1.0, 2.0, 2.0});
  assert_vec_eq(rb.wave_vel_at(idx), ftd::Vec3{0.25, 0.5, 0.75});
  assert(rb.density_at(idx) == 3.0);
  assert(rb.fields().optional_empty());

  const int idx2 = rb.lattice().index(2, 2, 2);
  auto& shadow = rb.voxels();
  shadow[idx2].flux = {-2.0, 0.0, 0.0};
  shadow[idx2].wave_vel = {0.0, -2.0, 0.0};
  assert_vec_eq(rb.flux_at(idx2), shadow[idx2].flux);
  assert_vec_eq(rb.wave_vel_at(idx2), shadow[idx2].wave_vel);

  rb.inject_flux(3, 3, 3, {0.0, -4.0, 3.0});
  const int idx3 = rb.lattice().index(3, 3, 3);
  assert(rb.density_at(idx3) == 5.0);

  ftd::RenderBridge rb_ops(7);
  rb_ops.force_cpu();
  seed_flux_pattern(rb_ops);
  const auto& soa = rb_ops.fields();
  const auto& aos = rb_ops.voxels();
  const auto& lattice = rb_ops.lattice();
  for (int i = 0; i < static_cast<int>(lattice.total_sites()); ++i) {
    assert(::ftd::divergence_flux_op(soa, lattice, i) ==
           ::ftd::divergence_flux_op(aos, lattice, i));
    assert_vec_eq(::ftd::curl_flux_op(soa, lattice, i),
                  ::ftd::curl_flux_op(aos, lattice, i));
    assert_vec_eq(::ftd::gradient_density_op(soa, lattice, i),
                  ::ftd::gradient_density_op(aos, lattice, i));
    assert_vec_eq(::ftd::gradient_divergence_op(soa, lattice, i),
                  ::ftd::gradient_divergence_op(aos, lattice, i));
  }

  return 0;
}

#include "ftd/engine_state.h"
#include "ftd/render_bridge.h"
#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

int main() {
  ftd::TernaryField field(16);
  assert(field.manifested_count() == 0);
  assert(field.charge_sum() == 0);

  field.set_state(3, 1);
  field.set_state(5, -1);
  field.set_state(7, 2);   // normalized to +1
  field.set_state(9, -7);  // normalized to -1
  assert(field.state_at(7) == 1);
  assert(field.state_at(9) == -1);
  assert(field.positive_count() == 2);
  assert(field.negative_count() == 2);
  assert(field.manifested_count() == 4);
  assert(field.charge_sum() == 0);

  field.set_state(5, 0);
  field.set_state(7, -1);
  assert(field.positive_count() == 1);
  assert(field.negative_count() == 2);
  assert(field.manifested_count() == 3);
  assert(field.charge_sum() == -1);

  std::string error;
  assert(field.check_invariants(&error));
  assert(error.empty());
  const auto& ordered = field.ordered_active_indices();
  assert(std::is_sorted(ordered.begin(), ordered.end()));

  std::vector<ftd::Voxel> voxels(8);
  voxels[1].state = 1;
  voxels[2].state = -1;
  voxels[4].state = 1;
  field.rebuild_from_voxels(voxels);
  assert(field.positive_count() == 2);
  assert(field.negative_count() == 1);
  assert(field.manifested_count() == 3);
  assert(field.charge_sum() == 1);
  assert(field.check_invariants(&error));

  ftd::RenderBridge rb(7);
  const int a = rb.lattice().index(1, 1, 1);
  const int b = rb.lattice().index(2, 2, 2);
  rb.set_state(a, 1);
  assert(rb.state_at(a) == 1);
  assert(rb.is_manifested(a));
  assert(rb.charge_sum() == 1);

  rb.voxel_at(3, 3, 3).state = -1;
  assert(rb.state_at(3, 3, 3) == -1);
  assert(rb.charge_sum() == 0);

  auto& shadow = rb.voxels();
  shadow[b].state = 1;
  rb.set_state(a, -1);
  assert(rb.state_at(b) == 1);
  assert(rb.state_at(a) == -1);
  assert(rb.charge_sum() == -1);

  std::vector<int> active(rb.active_indices().begin(), rb.active_indices().end());
  std::sort(active.begin(), active.end());
  assert(std::binary_search(active.begin(), active.end(), a));
  assert(std::binary_search(active.begin(), active.end(), b));

  return 0;
}

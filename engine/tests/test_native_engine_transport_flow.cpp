/**
 * Native engine transport-history flow audit.
 *
 * This connects actual RenderBridge movement ticks to the finite-volume
 * dual-cell continuity ledger. The first adapter is intentionally narrow:
 * it extracts signed one-face movement currents from before/after state
 * snapshots, then verifies
 *
 *   Delta rho + div I = S_reaction
 *
 * before and after b=2 blocking.
 */

#include "ftd/constants.h"
#include "ftd/eft/dual_cell_continuity.h"
#include "ftd/render_bridge.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

static int g_failures = 0;

void check(const std::string& name, bool condition) {
  if (condition) {
    std::cout << "  PASS  " << name << "\n";
  } else {
    std::cout << "  FAIL  " << name << "\n";
    ++g_failures;
  }
}

int idx(const ftd::RenderBridge& rb, int x, int y, int z) {
  return rb.lattice().index(x, y, z);
}

std::vector<int> state_snapshot(const ftd::RenderBridge& rb) {
  std::vector<int> out(static_cast<size_t>(rb.lattice().total_sites()), 0);
  const auto& voxels = rb.voxels();
  for (size_t i = 0; i < out.size(); ++i) {
    out[i] = static_cast<int>(voxels[i].state);
  }
  return out;
}

void configure_movement_only(ftd::RenderBridge& rb) {
  rb.toggles.disable_all();
  rb.toggles.movement = true;
  rb.toggles.dual_substrate = false;
}

bool extract_face_transport_history(int L,
                                    const std::vector<int>& before,
                                    const std::vector<int>& after,
                                    ftd::eft::DualCellContinuity& out) {
  out = ftd::eft::DualCellContinuity(L);
  out.rho_before = before;
  out.rho_after = after;

  std::vector<int> delta(before.size(), 0);
  for (size_t i = 0; i < before.size(); ++i) {
    delta[i] = after[i] - before[i];
  }

  auto index = [L](int x, int y, int z) {
    x = (x % L + L) % L;
    y = (y % L + L) % L;
    z = (z % L + L) % L;
    return x * L * L + y * L + z;
  };

  bool ok = true;
  for (int z = 0; z < L; ++z)
    for (int y = 0; y < L; ++y)
      for (int x = 0; x < L; ++x) {
        const int from = index(x, y, z);
        while (delta[static_cast<size_t>(from)] < 0) {
          const int q = +1;
          const int xp = index(x + 1, y, z);
          const int xm = index(x - 1, y, z);
          if (delta[static_cast<size_t>(xp)] > 0) {
            out.current_x[static_cast<size_t>(from)] += q;
            delta[static_cast<size_t>(from)] += q;
            delta[static_cast<size_t>(xp)] -= q;
          } else if (delta[static_cast<size_t>(xm)] > 0) {
            out.current_x[static_cast<size_t>(xm)] += q;
            delta[static_cast<size_t>(from)] += q;
            delta[static_cast<size_t>(xm)] -= q;
          } else {
            ok = false;
            break;
          }
        }
        while (delta[static_cast<size_t>(from)] > 0) {
          const int q = -1;
          const int xp = index(x + 1, y, z);
          const int xm = index(x - 1, y, z);
          if (delta[static_cast<size_t>(xp)] < 0) {
            out.current_x[static_cast<size_t>(from)] += q;
            delta[static_cast<size_t>(from)] += q;
            delta[static_cast<size_t>(xp)] -= q;
          } else if (delta[static_cast<size_t>(xm)] < 0) {
            out.current_x[static_cast<size_t>(xm)] += q;
            delta[static_cast<size_t>(from)] += q;
            delta[static_cast<size_t>(xm)] -= q;
          } else {
            ok = false;
            break;
          }
        }
      }

  for (int value : delta) {
    if (value != 0) ok = false;
  }
  return ok;
}

void check_transport_history(const std::string& name,
                             ftd::RenderBridge& rb) {
  const auto before = state_snapshot(rb);
  rb.tick();
  const auto after = state_snapshot(rb);

  ftd::eft::DualCellContinuity fine;
  const bool extracted =
      extract_face_transport_history(rb.lattice().size(), before, after, fine);
  const auto coarse = ftd::eft::block_dual_cell_continuity_b2(fine);

  std::cout << "    " << name
            << ": extracted=" << (extracted ? "yes" : "no")
            << " Q0=" << ftd::eft::total_before(fine)
            << " Q1=" << ftd::eft::total_after(fine)
            << " fine_res=" << ftd::eft::max_continuity_residual(fine)
            << " coarse_res=" << ftd::eft::max_continuity_residual(coarse)
            << "\n";

  check(name + ": face transport extracted", extracted);
  check(name + ": fine continuity closes",
        ftd::eft::max_continuity_residual(fine) < 1e-12);
  check(name + ": blocked continuity closes",
        ftd::eft::max_continuity_residual(coarse) < 1e-12);
  check(name + ": total source conserved",
        ftd::eft::total_before(fine) == ftd::eft::total_after(fine));
}

}  // namespace

int main() {
  std::cout << "================================================================\n";
  std::cout << "  TEST: Native Engine Transport b=2 Flow\n";
  std::cout << "================================================================\n";

  {
    std::cout << "\n-- NET-1: + charge crosses coarse boundary --\n";
    ftd::RenderBridge rb(8);
    configure_movement_only(rb);
    rb.inject_particle(3, 1, 1, +1, {0, 0, ftd::K_B});
    rb.voxel_at(3, 1, 1).velocity = {1, 0, 0};
    check_transport_history("NET-1 + transport", rb);
  }

  {
    std::cout << "\n-- NET-2: - charge crosses coarse boundary --\n";
    ftd::RenderBridge rb(8);
    configure_movement_only(rb);
    rb.inject_particle(3, 2, 1, -1, {0, 0, -ftd::K_B});
    rb.voxel_at(3, 2, 1).velocity = {1, 0, 0};
    check_transport_history("NET-2 - transport", rb);
  }

  {
    std::cout << "\n-- NET-3: internal transport cancels under coarse boundary --\n";
    ftd::RenderBridge rb(8);
    configure_movement_only(rb);
    rb.inject_particle(1, 1, 1, +1, {0, 0, ftd::K_B});
    rb.voxel_at(1, 1, 1).velocity = {1, 0, 0};
    check_transport_history("NET-3 internal + transport", rb);
  }

  std::cout << "\n================================================================\n";
  if (g_failures == 0) {
    std::cout << "  Native engine transport-flow audit PASSED.\n";
  } else {
    std::cout << "  " << g_failures << " native transport-flow check(s) FAILED.\n";
  }
  std::cout << "================================================================\n";

  return g_failures;
}

/**
 * @file test_voxel_layout.cpp
 * @brief Voxel memory-layout characterization guard (revision 0.8).
 *
 * Pins sizeof(ftd::Voxel), the location of the strong/weak substrate block,
 * and the per-voxel byte budget, so that:
 *   1. Accidental struct growth is loud (every field added to Voxel costs
 *      sizeof * N bytes on every configuration, hot-loop cache density, and
 *      GPU SoA marshalling work).
 *   2. The planned strong/weak side-buffer extraction (revision plan Phase
 *      4.1) has a characterization baseline: the four Vec3 fields
 *      flux_strong / wave_vel_strong / flux_weak / wave_vel_weak currently
 *      occupy the FINAL 96 bytes of the struct (~28% of every voxel), and
 *      are zero on every default-toggle configuration.
 *
 * If sizeof(Voxel) changes DELIBERATELY, update EXPECTED_SIZEOF_VOXEL below
 * in the same commit and say why in the commit message — this test exists to
 * make that a conscious decision, not to forbid it.
 */

#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <type_traits>

#include "ftd/voxel.h"

using ftd::Vec3;
using ftd::Voxel;

// offsetof requires standard-layout; both types qualify today. If a change
// breaks this (virtuals, mixed access specifiers), the GPU SoA mirroring in
// gpu_buffers.cu and the golden-hash fold both need re-auditing first.
static_assert(std::is_standard_layout<Vec3>::value, "Vec3 must stay standard-layout");
static_assert(std::is_standard_layout<Voxel>::value, "Voxel must stay standard-layout");
static_assert(sizeof(Vec3) == 3 * sizeof(double), "Vec3 must stay 3 packed doubles (GPU mirrors assume this)");

namespace {

// Characterization values (MSVC x64 / gcc x64 / wasm32 all agree: every
// member is <=8-aligned, no pointers, identical padding).
constexpr std::size_t EXPECTED_SIZEOF_VOXEL = 344;
constexpr std::size_t STRONG_WEAK_BLOCK_BYTES = 4 * sizeof(Vec3); // 96

int failures = 0;

void check(bool ok, const char* what, long long actual, long long expected) {
  if (ok) {
    std::printf("  PASS  %-52s actual=%lld\n", what, actual);
  } else {
    std::printf("  FAIL  %-52s actual=%lld expected=%lld\n", what, actual, expected);
    ++failures;
  }
}

} // namespace

int main() {
  std::printf("=== Voxel layout characterization (revision 0.8) ===\n\n");

  check(sizeof(Voxel) == EXPECTED_SIZEOF_VOXEL,
        "sizeof(Voxel)", (long long)sizeof(Voxel), (long long)EXPECTED_SIZEOF_VOXEL);

  // The strong/weak substrate block must be the four trailing Vec3 fields,
  // contiguous, ending exactly at sizeof(Voxel). Phase 4.1 relies on this
  // being a cleanly severable suffix.
  const auto off_fs = offsetof(Voxel, flux_strong);
  const auto off_ws = offsetof(Voxel, wave_vel_strong);
  const auto off_fw = offsetof(Voxel, flux_weak);
  const auto off_ww = offsetof(Voxel, wave_vel_weak);

  check(off_ws == off_fs + sizeof(Vec3), "wave_vel_strong follows flux_strong",
        (long long)off_ws, (long long)(off_fs + sizeof(Vec3)));
  check(off_fw == off_ws + sizeof(Vec3), "flux_weak follows wave_vel_strong",
        (long long)off_fw, (long long)(off_ws + sizeof(Vec3)));
  check(off_ww == off_fw + sizeof(Vec3), "wave_vel_weak follows flux_weak",
        (long long)off_ww, (long long)(off_fw + sizeof(Vec3)));
  check(off_fs + STRONG_WEAK_BLOCK_BYTES == sizeof(Voxel),
        "strong/weak block is the trailing 96 bytes",
        (long long)(off_fs + STRONG_WEAK_BLOCK_BYTES), (long long)sizeof(Voxel));

  // Budget documentation (informational, printed for commit messages):
  const long long n64 = 64LL * 64 * 64;
  std::printf("\n  Budget @ L=64 (%lld voxels):\n", n64);
  std::printf("    total voxel array : %8.2f MiB\n",
              (double)(n64 * sizeof(Voxel)) / (1024.0 * 1024.0));
  std::printf("    strong/weak block : %8.2f MiB (%.1f%%) — zero unless strong/weak fields active\n",
              (double)(n64 * STRONG_WEAK_BLOCK_BYTES) / (1024.0 * 1024.0),
              100.0 * (double)STRONG_WEAK_BLOCK_BYTES / (double)sizeof(Voxel));

  std::printf("\n=== RESULT: %s ===\n", failures == 0 ? "PASS" : "FAIL");
  return failures == 0 ? 0 : 1;
}

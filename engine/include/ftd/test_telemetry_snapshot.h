#pragma once
/**
 * @file engine/include/ftd/test_telemetry_snapshot.h
 * @purpose RenderBridge-aware lattice snapshot encoder for FTD Test Bench NDJSON.
 * @consumers engine/tests/* (include alongside ftd/test_telemetry.h + ftd/render_bridge.h)
 *
 * Downsamples ternary state by `stride`, packs int8 values in {-1,0,+1}, and
 * emits the same NDJSON snapshot event as ftd::test::snapshot(raw buffer).
 */

#include <cstdint>
#include <vector>

#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"

namespace ftd {
namespace test {

inline void snapshot(const RenderBridge& rb, int tick_num, int stride = 1) {
    if (stride < 1) stride = 1;
    const int L = rb.lattice().size();
    const auto& voxels = rb.voxels();
    const int n = static_cast<int>(voxels.size());

    std::vector<std::int8_t> down;
    down.reserve(static_cast<std::size_t>((L / stride + 1) * (L / stride + 1) * (L / stride + 1)));

    for (int z = 0; z < L; z += stride) {
        for (int y = 0; y < L; y += stride) {
            for (int x = 0; x < L; x += stride) {
                const int idx = rb.lattice().index(x, y, z);
                if (idx >= 0 && idx < n) {
                    down.push_back(voxels[static_cast<std::size_t>(idx)].state);
                }
            }
        }
    }

    snapshot(tick_num, L, stride, down.data(), down.size());
}

}  // namespace test
}  // namespace ftd

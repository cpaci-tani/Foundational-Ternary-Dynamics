/**
 * test_cluster_genealogy — correctness gate for the genealogy detector.
 *
 * Synthetic hand-built voxel grids (no engine dynamics): proves the detector
 * classifies fission/fusion/persist/death correctly and is NOT itself
 * manufacturing P2/P3 events. Must PASS before any cluster-thermodynamics
 * campaign run. See .claude/plans/lazy-conjuring-marble.md.
 */
#include "ftd/cluster_genealogy.h"
#include "ftd/render_bridge.h"
#include <iostream>
#include <string>

using namespace ftd;

static int failures = 0;
static void check(const char* name, bool ok, const std::string& detail = "") {
    std::cout << "  [" << (ok ? "PASS" : "FAIL") << "] " << name;
    if (!detail.empty()) std::cout << "  -- " << detail;
    std::cout << "\n";
    if (!ok) ++failures;
}

static void set_box(RenderBridge& rb, int x0, int x1, int y0, int y1, int z0, int z1, int8_t s) {
    auto& vox = rb.voxels();
    for (int x = x0; x <= x1; ++x)
        for (int y = y0; y <= y1; ++y)
            for (int z = z0; z <= z1; ++z) {
                int idx = rb.lattice().index(x, y, z);
                vox[idx].state = s;
                vox[idx].flux = Vec3(static_cast<double>(s), 0.0, 0.0);
            }
}
static void clear_all(RenderBridge& rb) {
    auto& vox = rb.voxels();
    for (auto& v : vox) { v.state = 0; v.flux = Vec3(); v.wave_vel = Vec3(); v.velocity = Vec3(); }
}

int main() {
    std::cout << "==================================================\n";
    std::cout << "  test_cluster_genealogy (correctness gate)\n";
    std::cout << "==================================================\n";
    const int L = 24;

    // --- FISSION: one blob x[10..15] -> two blobs x[10,11] | x[14,15] (gap x[12,13]) ---
    {
        std::cout << "[Fission]\n";
        RenderBridge rb(L);
        ClusterGenealogyTracker g;
        clear_all(rb); set_box(rb, 10, 15, 10, 11, 10, 11, +1); g.record(rb);   // parent size 24
        clear_all(rb);
        set_box(rb, 10, 11, 10, 11, 10, 11, +1);                               // left child 8
        set_box(rb, 14, 15, 10, 11, 10, 11, +1);                               // right child 8
        g.record(rb);
        auto fis = g.fissions();
        check("FISSION detected exactly once", g.count(EventType::Fission) == 1,
              "count=" + std::to_string(g.count(EventType::Fission)));
        check("FISSION is 1 parent -> 2 children",
              fis.size() == 1 && fis[0].parent_ids.size() == 1 && fis[0].child_ids.size() == 2);
        if (!fis.empty())
            check("FISSION size accounting (parent=24, children=16)",
                  fis[0].sum_parent_size == 24 && fis[0].sum_child_size == 16,
                  "parent=" + std::to_string(fis[0].sum_parent_size) +
                  " child=" + std::to_string(fis[0].sum_child_size));
        check("FISSION: no spurious Death/Fusion",
              g.count(EventType::Death) == 0 && g.count(EventType::Fusion) == 0);
    }

    // --- FUSION: two blobs -> one blob ---
    {
        std::cout << "[Fusion]\n";
        RenderBridge rb(L);
        ClusterGenealogyTracker g;
        clear_all(rb);
        set_box(rb, 10, 11, 10, 11, 10, 11, +1);
        set_box(rb, 14, 15, 10, 11, 10, 11, +1);
        g.record(rb);                                                          // 2 births
        clear_all(rb); set_box(rb, 10, 15, 10, 11, 10, 11, +1); g.record(rb);  // merged
        auto fus = g.fusions();
        check("FUSION detected exactly once", g.count(EventType::Fusion) == 1,
              "count=" + std::to_string(g.count(EventType::Fusion)));
        check("FUSION is 2 parents -> 1 child",
              fus.size() == 1 && fus[0].parent_ids.size() == 2 && fus[0].child_ids.size() == 1);
        check("FUSION: no spurious Death", g.count(EventType::Death) == 0);
    }

    // --- PERSIST: translate by 1 (overlapping) -> no fission/fusion/death event ---
    {
        std::cout << "[Persist]\n";
        RenderBridge rb(L);
        ClusterGenealogyTracker g;
        clear_all(rb); set_box(rb, 10, 11, 10, 11, 10, 11, +1); g.record(rb);  // 1 birth
        clear_all(rb); set_box(rb, 11, 12, 10, 11, 10, 11, +1); g.record(rb);  // shift, overlap at x11
        check("PERSIST: no Fission/Fusion/Death on translation",
              g.count(EventType::Fission) == 0 && g.count(EventType::Fusion) == 0 &&
              g.count(EventType::Death) == 0);
        check("PERSIST: only the initial Birth recorded (no spurious events)",
              g.count(EventType::Birth) == 1 && g.events().size() == 1,
              "events=" + std::to_string(g.events().size()));
    }

    // --- DEATH: blob -> empty ---
    {
        std::cout << "[Death]\n";
        RenderBridge rb(L);
        ClusterGenealogyTracker g;
        clear_all(rb); set_box(rb, 10, 11, 10, 11, 10, 11, +1); g.record(rb);  // 1 birth
        clear_all(rb); g.record(rb);                                           // gone
        check("DEATH detected exactly once", g.count(EventType::Death) == 1);
    }

    std::cout << "==================================================\n";
    std::cout << (failures == 0 ? "  ALL PASSED\n" : ("  " + std::to_string(failures) + " FAILURE(S)\n"));
    std::cout << "==================================================\n";
    return failures;
}

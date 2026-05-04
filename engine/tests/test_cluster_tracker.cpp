/**
 * Test: ClusterTracker (Class B Phase B.1)
 *
 * Smoke + invariant tests for the ClusterTracker introduced as the first
 * concrete deliverable of the Discrete-Native Derivation Program (FTD-0136).
 * See docs/theory/01_reference/SPEC_CLASS_B_CLUSTER_PERSISTENCE.md.
 *
 * Tests:
 *   1. Empty lattice  -> zero clusters tracked
 *   2. Below-threshold cluster (size < N_min) ignored
 *   3. Single isolated cluster: born at first record, alive across ticks
 *   4. Two disjoint clusters: tracked independently
 *   5. Opposite-sign adjacent voxels: NOT merged (sign-separation)
 *   6. Cluster persistence under unchanged state
 *   7. Cluster death: when no manifested voxels remain
 *   8. Default 6-face vs Moore-26 connectivity differs as expected
 *
 * Notes:
 *   - These tests use a manually-injected state (no engine ticks) to verify
 *     the cluster-identification + persistence-tracking algorithm itself.
 *     Coupling tests (engine + tracker) come later in Phase B.2-B.4.
 */
#include <iostream>
#include <iomanip>
#include "ftd/render_bridge.h"
#include "ftd/cluster_tracker.h"

static int failures = 0;

static void check(const char* name, bool condition) {
    if (condition) std::cout << "  PASS  " << name << "\n";
    else { std::cout << "  FAIL  " << name << "\n"; ++failures; }
}

static void check_eq(const char* name, long got, long expected) {
    bool ok = (got == expected);
    if (ok) std::cout << "  PASS  " << name << " (= " << got << ")\n";
    else { std::cout << "  FAIL  " << name << " (got " << got
                     << ", expected " << expected << ")\n"; ++failures; }
}

// Helper: stamp a small connected cube of manifested voxels at (x0,y0,z0)
// of size dx*dy*dz, with given sign. Bypasses engine dynamics by writing
// directly to the voxel buffer.
static void stamp_cluster(ftd::RenderBridge& rb,
                          int x0, int y0, int z0,
                          int dx, int dy, int dz,
                          int8_t sign) {
    auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    for (int x = x0; x < x0 + dx; ++x) {
        for (int y = y0; y < y0 + dy; ++y) {
            for (int z = z0; z < z0 + dz; ++z) {
                int idx = lat.index(x, y, z);
                vox[idx].state = sign;
            }
        }
    }
}

static void clear_voxels(ftd::RenderBridge& rb) {
    auto& vox = rb.voxels();
    for (auto& v : vox) v.state = 0;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: ClusterTracker (FTD-0136 Class B Phase B.1)\n";
    std::cout << "================================================================\n\n";

    // -----------------------------------------------------------------
    // Test 1: empty lattice -> no clusters
    // -----------------------------------------------------------------
    std::cout << "--- Test 1: empty lattice ---\n";
    {
        ftd::RenderBridge rb(16);
        ftd::ClusterTracker tracker;
        tracker.record(rb);
        check_eq("alive_count == 0", tracker.alive_count(), 0);
        check_eq("total_tracked == 0", tracker.total_tracked(), 0);
    }

    // -----------------------------------------------------------------
    // Test 2: below-threshold cluster ignored
    // -----------------------------------------------------------------
    std::cout << "\n--- Test 2: below-threshold cluster ignored ---\n";
    {
        ftd::RenderBridge rb(16);
        ftd::ClusterTrackerParams params;
        params.min_cluster_size = 4;
        ftd::ClusterTracker tracker(params);
        // Stamp a 1-voxel cluster (size 1 < N_min = 4).
        stamp_cluster(rb, 8, 8, 8, 1, 1, 1, +1);
        tracker.record(rb);
        check_eq("alive_count == 0 for size-1 cluster", tracker.alive_count(), 0);
    }

    // -----------------------------------------------------------------
    // Test 3: single isolated cluster, alive across ticks
    // -----------------------------------------------------------------
    std::cout << "\n--- Test 3: single isolated cluster persists ---\n";
    {
        ftd::RenderBridge rb(16);
        ftd::ClusterTracker tracker;
        // Stamp a 2x2x2 = 8-voxel cluster.
        stamp_cluster(rb, 6, 6, 6, 2, 2, 2, +1);
        tracker.record(rb);  // tick 0: birth
        tracker.record(rb);  // tick 0 again (same state; persists)
        tracker.record(rb);  // tick 0 again
        check_eq("total_tracked == 1", tracker.total_tracked(), 1);
        check_eq("alive_count == 1", tracker.alive_count(), 1);
        check_eq("max_size_observed == 8", tracker.max_size_observed(), 8);
    }

    // -----------------------------------------------------------------
    // Test 4: two disjoint clusters tracked independently
    // -----------------------------------------------------------------
    std::cout << "\n--- Test 4: two disjoint clusters ---\n";
    {
        ftd::RenderBridge rb(32);
        ftd::ClusterTracker tracker;
        stamp_cluster(rb,  4,  4,  4, 2, 2, 2, +1);
        stamp_cluster(rb, 20, 20, 20, 2, 2, 2, +1);
        tracker.record(rb);
        check_eq("alive_count == 2", tracker.alive_count(), 2);
        check_eq("total_tracked == 2", tracker.total_tracked(), 2);
    }

    // -----------------------------------------------------------------
    // Test 5: opposite-sign clusters NOT merged
    // -----------------------------------------------------------------
    std::cout << "\n--- Test 5: opposite-sign clusters separate ---\n";
    {
        ftd::RenderBridge rb(16);
        ftd::ClusterTracker tracker;
        // Two adjacent 2x2x2 cubes, opposite signs.
        stamp_cluster(rb, 4, 4, 4, 2, 2, 2, +1);
        stamp_cluster(rb, 6, 4, 4, 2, 2, 2, -1);
        tracker.record(rb);
        check_eq("alive_count == 2 (sign-separated)", tracker.alive_count(), 2);
        // Verify signs differ.
        int positive = 0, negative = 0;
        for (const auto& [_, h] : tracker.histories()) {
            if (h.state_sign > 0) ++positive;
            if (h.state_sign < 0) ++negative;
        }
        check("one positive cluster", positive == 1);
        check("one negative cluster", negative == 1);
    }

    // -----------------------------------------------------------------
    // Test 6: cluster persistence under unchanged state
    // -----------------------------------------------------------------
    std::cout << "\n--- Test 6: persistence under unchanged state ---\n";
    {
        ftd::RenderBridge rb(16);
        ftd::ClusterTracker tracker;
        stamp_cluster(rb, 6, 6, 6, 3, 3, 3, +1);   // 27-voxel cluster
        for (int t = 0; t < 10; ++t) tracker.record(rb);
        check_eq("alive_count == 1 after 10 records", tracker.alive_count(), 1);
        check_eq("total_tracked == 1", tracker.total_tracked(), 1);
        // The cluster has no death tick yet (still alive at recording time).
        for (const auto& [_, h] : tracker.histories()) {
            check("cluster still alive", h.alive());
            check_eq("max_size == 27", h.max_size, 27);
        }
    }

    // -----------------------------------------------------------------
    // Test 7: cluster death when manifested voxels removed
    // -----------------------------------------------------------------
    std::cout << "\n--- Test 7: cluster death when voxels removed ---\n";
    {
        ftd::RenderBridge rb(16);
        ftd::ClusterTracker tracker;
        stamp_cluster(rb, 6, 6, 6, 2, 2, 2, +1);
        tracker.record(rb);                       // birth at tick 0
        check_eq("alive at birth", tracker.alive_count(), 1);
        // Now wipe state -> no manifested voxels remain.
        clear_voxels(rb);
        tracker.record(rb);                       // detect death
        check_eq("dead after wipe", tracker.alive_count(), 0);
        // Lifetime distribution should now have one entry of 0.
        auto lt = tracker.lifetime_distribution();
        check_eq("lifetime_distribution.size() == 1", static_cast<long>(lt.size()), 1);
    }

    // -----------------------------------------------------------------
    // Test 8: connectivity choice — diagonal voxels
    // -----------------------------------------------------------------
    std::cout << "\n--- Test 8: 6-face vs Moore-26 connectivity ---\n";
    {
        ftd::RenderBridge rb(16);
        // Two 2x2x2 cubes that share only corner-to-corner contact.
        // Cube A: (4,4,4)-(5,5,5). Cube B: (6,6,6)-(7,7,7).
        // Under 6-face: NOT connected (no face contact).
        // Under Moore-26: Connected (corner contact).
        stamp_cluster(rb, 4, 4, 4, 2, 2, 2, +1);
        stamp_cluster(rb, 6, 6, 6, 2, 2, 2, +1);

        // 6-face tracker: should see 2 clusters.
        ftd::ClusterTracker tracker_face;
        tracker_face.record(rb);
        check_eq("6-face: 2 clusters", tracker_face.alive_count(), 2);

        // Moore-26 tracker: should see 1 cluster.
        ftd::ClusterTrackerParams moore;
        moore.use_moore_neighbors = true;
        ftd::ClusterTracker tracker_moore(moore);
        tracker_moore.record(rb);
        check_eq("Moore-26: 1 cluster", tracker_moore.alive_count(), 1);
    }

    // -----------------------------------------------------------------
    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: ";
    if (failures == 0) std::cout << "ALL PASS\n";
    else                std::cout << failures << " FAILURE(S)\n";
    std::cout << "================================================================\n";
    return failures == 0 ? 0 : 1;
}

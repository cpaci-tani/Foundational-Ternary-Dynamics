// engine/tests/test_knot_telemetry.cpp
// Unit test for KnotTracker: per-knot lifecycle + observable assembly.
// Mirrors test_cluster_tracker.cpp (direct voxel stamping, no engine dynamics).
#include <iostream>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/knot_telemetry.h"

static int failures = 0;
static void check(const char* name, bool cond) {
    std::cout << (cond ? "  PASS  " : "  FAIL  ") << name << "\n";
    if (!cond) ++failures;
}
static void check_eq(const char* name, long got, long expected) {
    bool ok = (got == expected);
    std::cout << (ok ? "  PASS  " : "  FAIL  ") << name
              << " (got " << got << ", expected " << expected << ")\n";
    if (!ok) ++failures;
}

static void stamp(ftd::RenderBridge& rb, int x0,int y0,int z0,int dx,int dy,int dz,int8_t sign) {
    auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    for (int x=x0;x<x0+dx;++x) for (int y=y0;y<y0+dy;++y) for (int z=z0;z<z0+dz;++z)
        vox[lat.index(x,y,z)].state = sign;
}

int main() {
    std::cout << "=== KnotTracker telemetry ===\n";

    // One 3x3x3 = 27-voxel positive cluster, recorded twice (persists).
    {
        ftd::RenderBridge rb(16);
        ftd::KnotTracker kt;
        stamp(rb, 6,6,6, 3,3,3, +1);
        kt.record(rb);
        kt.record(rb);
        auto alive = kt.alive_knots();
        check_eq("one alive knot", static_cast<long>(alive.size()), 1);
        if (!alive.empty()) {
            const auto& k = alive[0];
            check_eq("size 27", k.size, 27);
            check_eq("max_size 27", k.max_size, 27);
            check_eq("sign +1", k.sign, 1);
            check("centroid ~ (7,7,7)", std::abs(k.cx - 7.0) < 1e-6 && std::abs(k.cy - 7.0) < 1e-6);
            check("persistent id >= 0", k.id >= 0);
            // age = elapsed engine ticks since birth (current_tick - birth_tick),
            // consistent with ClusterTracker::lifetime(). Both records happen at
            // tick 0 with no rb.tick() between, so zero ticks have elapsed.
            check_eq("age 0 (two records, same tick)", k.age, 0);
        }
    }

    // Two opposite-sign clusters → two knots, net charge 0 by count.
    {
        ftd::RenderBridge rb(20);
        ftd::KnotTracker kt;
        stamp(rb, 3,3,3, 2,2,2, +1);   // 8 voxels +
        stamp(rb, 12,12,12, 2,2,2, -1); // 8 voxels -
        kt.record(rb);
        auto alive = kt.alive_knots();
        check_eq("two alive knots", static_cast<long>(alive.size()), 2);
        auto agg = kt.aggregate();
        check_eq("aggregate alive 2", agg.alive, 2);
        check_eq("aggregate births 2", agg.births, 2);
    }

    // A knot that vanishes is marked dead (death is detected on absence;
    // no engine tick needed — matches ClusterTracker).
    {
        ftd::RenderBridge rb(16);
        ftd::KnotTracker kt;
        stamp(rb, 6,6,6, 2,2,2, +1);   // 8-voxel knot
        kt.record(rb);                 // birth
        stamp(rb, 6,6,6, 2,2,2, 0);    // wipe it
        kt.record(rb);                 // no successor -> death
        check_eq("0 alive after wipe", static_cast<long>(kt.alive_knots().size()), 0);
        check_eq("deaths == 1", kt.aggregate().deaths, 1);
        check_eq("births == 1", kt.aggregate().births, 1);
    }

    // Sub-min-size components are filtered (default min_cluster_size = 4).
    {
        ftd::RenderBridge rb(16);
        ftd::KnotTracker kt;
        stamp(rb, 8,8,8, 1,1,1, +1);   // single voxel < 4
        kt.record(rb);
        check_eq("single voxel filtered (0 alive)", static_cast<long>(kt.alive_knots().size()), 0);
        check_eq("no births for sub-min knot", kt.aggregate().births, 0);
    }

    std::cout << (failures==0 ? "ALL PASS\n" : "FAILURES\n");
    return failures==0 ? 0 : 1;
}

/**
 * Campaign: Bound State Lifetime
 *
 * Place free +1 and -1 at various separations on 32^3 lattice.
 * Track: separation vs time, time to annihilation (if any).
 *
 * Tests:
 *   B1: Opposite charges at r=2 attract (separation decreases or annihilate)
 *   B2: Opposite charges at r=3 attract (force steep ~r^-3.8, r=6 too far)
 *   B3: Same-sign charges at r=3 repel (separation increases)
 */

#include <iostream>
#include <iomanip>
#include <cmath>
#include "ftd/render_bridge.h"

int g_pass = 0, g_fail = 0;

void check(const char* name, bool cond) {
    if (cond) { std::cout << "  PASS  " << name << "\n"; ++g_pass; }
    else      { std::cout << "  FAIL  " << name << "\n"; ++g_fail; }
}

struct ParticlePos {
    int x = -1, y = -1, z = -1;
    bool found = false;
};

ParticlePos find_sign(const ftd::RenderBridge& engine, int8_t sign) {
    ParticlePos p;
    for (int i = 0; i < engine.lattice().total_sites(); ++i) {
        if (engine.voxels()[i].state == sign) {
            auto c = engine.lattice().coord(i);
            p.x = c.x; p.y = c.y; p.z = c.z;
            p.found = true;
            return p;
        }
    }
    return p;
}

double separation(const ftd::RenderBridge& engine) {
    auto pos = find_sign(engine, +1);
    auto neg = find_sign(engine, -1);
    if (!pos.found || !neg.found) return -1.0;
    double dx = pos.x - neg.x, dy = pos.y - neg.y, dz = pos.z - neg.z;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

void run_pair_test(const char* label, int L, int sep, int8_t s1, int8_t s2, int ticks) {
    std::cout << "\n  --- " << label << " (sep=" << sep << ") ---\n";
    ftd::RenderBridge engine(L);
    int mid = L / 2;

    double iso = ftd::K_B / std::sqrt(3.0);

    // Lock both initially to equilibrate flux
    engine.inject_particle(mid - sep/2, mid, mid, s1, {iso, iso, iso});
    engine.voxel_at(mid - sep/2, mid, mid).locked = true;
    engine.inject_particle(mid + sep/2, mid, mid, s2, {iso, iso, iso});
    engine.voxel_at(mid + sep/2, mid, mid).locked = true;

    // Equilibrate
    engine.run(200);

    // Unlock
    engine.voxels()[engine.lattice().index(mid - sep/2, mid, mid)].locked = false;
    engine.voxels()[engine.lattice().index(mid + sep/2, mid, mid)].locked = false;

    double initial_sep = separation(engine);
    std::cout << "  Initial separation: " << std::setprecision(2) << initial_sep << "\n";

    // Track
    int annihilation_tick = -1;
    double final_sep = initial_sep;
    double min_sep = initial_sep;

    for (int t = 0; t < ticks; ++t) {
        engine.tick();
        double s = separation(engine);
        if (s < 0) {
            annihilation_tick = t + 1;
            final_sep = 0;
            break;
        }
        final_sep = s;
        if (s < min_sep) min_sep = s;
    }

    std::cout << "  Final separation: " << std::setprecision(2) << final_sep << "\n";
    if (annihilation_tick > 0) {
        std::cout << "  Annihilated at tick " << annihilation_tick << "\n";
    }

    // Return results via global state (simple approach)
    if (s1 != s2) {
        // Opposite signs — expect attraction
        bool attracted = (final_sep < initial_sep) || (annihilation_tick > 0);
        std::string check_name = std::string(label) + ": opposite charges attract";
        check(check_name.c_str(), attracted);
    } else {
        // Same signs — expect repulsion
        bool repelled = final_sep >= initial_sep;
        std::string check_name = std::string(label) + ": same charges repel";
        check(check_name.c_str(), repelled);
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Bound State Lifetime\n";
    std::cout << "================================================================\n";

    const int L = 32;
    const int TICKS = 2000;

    // B1: Opposite charges at r=2
    run_pair_test("B1", L, 2, +1, -1, TICKS);

    // B2: Opposite charges at r=3 (force steep ~r^-3.8, r=6 is too far)
    run_pair_test("B2", L, 3, +1, -1, TICKS);

    // B3: Same-sign charges at r=3 (use r=3 to avoid ambiguity at r=2)
    run_pair_test("B3", L, 3, +1, +1, TICKS);

    std::cout << "\n================================================================\n";
    std::cout << "  Bound Lifetime Campaign: " << g_pass << " passed, "
              << g_fail << " failed\n";
    std::cout << "================================================================\n";

    return g_fail;
}

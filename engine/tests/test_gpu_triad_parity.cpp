/**
 * CPU/CUDA parity for TermToggles::triad_binding.
 *
 * triad_binding is declared ToggleBackend::ANY (term_toggles.h TOGGLE_SPECS),
 * i.e. it carries a cross-backend equivalence contract. Until 2026-09-16 the
 * GPU broke that contract twice over:
 *
 *   (2) kernels_forces.cu's triad_detection_kernel took NO locked[] buffer, so
 *       an already-locked voxel could be pulled into a new triad. The CPU
 *       (transmutation_phases.cpp triad_binding_cpu) excludes locked voxels at
 *       all three (a,b,c) loop levels.
 *   (3) The GPU selected triads by a per-particle "my two nearest same-sign
 *       neighbours" scan over minimum-image distances; the CPU does an
 *       exhaustive index-ordered a<b<c search over RAW coordinate distances,
 *       taking the first valid c per (a,b) pair and mutating locked[] in place
 *       as it goes. For any cluster with more than two close same-sign
 *       neighbours the two rules lock different, non-corresponding sets.
 *
 * The prior audit confirmed no parity suite covered this. This one does, with
 * a deliberately ambiguous 7-particle cluster (the old heuristic locks all 7;
 * the rule of record leaves one out) and a pre-locked-partner case that the
 * old kernel could not see at all.
 *
 * The expected sets are not hard-coded and are not taken from either backend:
 * a third, independent transcription of the documented rule
 * (reference_triad_locks below) is asserted equal to both, so agreement
 * between CPU and GPU cannot be agreement on a shared mistake in either
 * implementation.
 *
 * Parity here is EXACT, not tolerance-based: every distance is sqrt() of an
 * exactly representable integer and every comparison is an IEEE-754 double
 * operation, so the two backends must produce byte-identical locked sets.
 */

#include "ftd/constants.h"
#include "ftd/gpu_engine.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace {

using ftd::K_B;
using ftd::RenderBridge;
using ftd::TermToggles;
using ftd::Vec3;
using ftd::Voxel;

int passed = 0;
int failed = 0;

void check(const char* name, bool ok) {
    std::printf("  %s  %s\n", ok ? "PASS" : "FAIL", name);
    ok ? ++passed : ++failed;
}

int index_of(int x, int y, int z, int L) {
    return x * L * L + y * L + z;
}

struct Site {
    int x, y, z;
    int state;
    int particle_id;
    bool pre_locked;
};

std::vector<Voxel> seed_lattice(int L, const std::vector<Site>& sites) {
    std::vector<Voxel> seed(static_cast<std::size_t>(L) * L * L);
    for (const auto& s : sites) {
        const int i = index_of(s.x, s.y, s.z, L);
        seed[static_cast<std::size_t>(i)].state = static_cast<int8_t>(s.state);
        seed[static_cast<std::size_t>(i)].flux = Vec3{K_B, 0.0, 0.0};
        seed[static_cast<std::size_t>(i)].particle_id = s.particle_id;
        seed[static_cast<std::size_t>(i)].locked = s.pre_locked;
    }
    return seed;
}

// Locked particle_ids of every manifested voxel, ascending. This is the set
// the two backends must agree on.
std::vector<int> locked_ids(const std::vector<Voxel>& voxels) {
    std::vector<int> ids;
    for (const auto& v : voxels)
        if (v.state != 0 && v.locked) ids.push_back(v.particle_id);
    std::sort(ids.begin(), ids.end());
    return ids;
}

std::string render(const std::vector<int>& ids) {
    std::string s = "{";
    for (std::size_t k = 0; k < ids.size(); ++k) {
        if (k) s += ",";
        s += std::to_string(ids[k]);
    }
    return s + "}";
}

// ---------------------------------------------------------------------------
// Independent transcription of the rule of record (triad_binding_cpu):
//   candidate set = every manifested site, ASCENDING lattice index;
//   a skipped iff locked at the top of its own iteration (never re-checked);
//   b,c skipped iff locked or of a different state, checked live;
//   RAW coordinate distances (no periodic minimum image);
//   all three pairwise distances <= TRIAD_RADIUS and min/max >=
//     TRIAD_RATIO_THRESHOLD;
//   first valid c wins for an (a,b) pair, then the c-loop breaks;
//   locks are applied in place, so later tests see them.
// Deliberately written from the specification, not copied from either backend.
// ---------------------------------------------------------------------------
std::vector<int> reference_triad_locks(int L, const std::vector<Site>& sites,
                                       int ticks) {
    std::vector<Voxel> voxels = seed_lattice(L, sites);
    std::vector<int> active;
    for (int i = 0; i < static_cast<int>(voxels.size()); ++i)
        if (voxels[static_cast<std::size_t>(i)].state != 0) active.push_back(i);
    std::sort(active.begin(), active.end());

    auto dist = [&](int p, int q) {
        const double dx = static_cast<double>(p / (L * L) - q / (L * L));
        const double dy = static_cast<double>((p / L) % L - (q / L) % L);
        const double dz = static_cast<double>(p % L - q % L);
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    };

    const int M = static_cast<int>(active.size());
    for (int t = 0; t < ticks; ++t) {
        for (int a = 0; a < M; ++a) {
            const int ia = active[static_cast<std::size_t>(a)];
            if (voxels[static_cast<std::size_t>(ia)].locked) continue;
            const int8_t sa = voxels[static_cast<std::size_t>(ia)].state;
            for (int b = a + 1; b < M; ++b) {
                const int ib = active[static_cast<std::size_t>(b)];
                if (voxels[static_cast<std::size_t>(ib)].locked) continue;
                if (voxels[static_cast<std::size_t>(ib)].state != sa) continue;
                const double rAB = dist(ia, ib);
                if (rAB > ftd::TRIAD_RADIUS) continue;
                for (int c = b + 1; c < M; ++c) {
                    const int ic = active[static_cast<std::size_t>(c)];
                    if (voxels[static_cast<std::size_t>(ic)].locked) continue;
                    if (voxels[static_cast<std::size_t>(ic)].state != sa) continue;
                    const double rAC = dist(ia, ic);
                    const double rBC = dist(ib, ic);
                    if (rAC > ftd::TRIAD_RADIUS || rBC > ftd::TRIAD_RADIUS) continue;
                    const double rmin = std::min({rAB, rAC, rBC});
                    const double rmax = std::max({rAB, rAC, rBC});
                    if (rmax < 1e-9) continue;
                    if (rmin / rmax < ftd::TRIAD_RATIO_THRESHOLD) continue;
                    voxels[static_cast<std::size_t>(ia)].locked = true;
                    voxels[static_cast<std::size_t>(ib)].locked = true;
                    voxels[static_cast<std::size_t>(ic)].locked = true;
                    break;
                }
            }
        }
    }
    return locked_ids(voxels);
}

// Rule 7 in isolation: nothing else in the tick may write locked[], move a
// particle, or change a state, so the locked set after N ticks is purely the
// triad rule's output. TOGGLE_SPECS declares triad_binding to depend on
// color_forces and dual_substrate, so RenderBridge prints a non-fatal
// "[TermToggles] Invalid combination" line on the CPU path here; that is the
// same isolation the pre-existing triad case in
// test_gpu_identity_lifecycle_parity uses, and enabling either dependency
// would pull unrelated physics into the comparison.
void configure(TermToggles& t) {
    t.disable_all();
    t.triad_binding = true;
}

std::vector<int> cpu_locks(int L, const std::vector<Site>& sites, int ticks) {
    RenderBridge bridge(L);
    bridge.force_cpu();
    configure(bridge.toggles);
    bridge.voxels() = seed_lattice(L, sites);
    for (int t = 0; t < ticks; ++t) bridge.tick();
    return locked_ids(static_cast<const RenderBridge&>(bridge).voxels());
}

std::vector<int> gpu_locks(int L, const std::vector<Site>& sites, int ticks) {
    ftd::gpu::GpuEngine engine(L);
    configure(engine.toggles);
    engine.upload_from_host(seed_lattice(L, sites));
    for (int t = 0; t < ticks; ++t) engine.tick();
    std::vector<Voxel> out;
    engine.sync_to_host(out);
    return locked_ids(out);
}

void compare(const char* label, int L, const std::vector<Site>& sites,
             int ticks) {
    const auto ref = reference_triad_locks(L, sites, ticks);
    const auto cpu = cpu_locks(L, sites, ticks);
    const auto gpu = gpu_locks(L, sites, ticks);
    std::printf("    %s: ref=%s cpu=%s gpu=%s\n", label, render(ref).c_str(),
                render(cpu).c_str(), render(gpu).c_str());
    check((std::string(label) + ": CPU matches the rule of record").c_str(),
          cpu == ref);
    check((std::string(label) + ": GPU locked-id set is identical to CPU").c_str(),
          gpu == cpu);
}

// ---------------------------------------------------------------------------
// Cluster A — seven same-sign particles, mutually ambiguous.
//
// (7,7,7) and its three FCC partners (7,8,8) (8,7,8) (8,8,7) form one regular
// r=sqrt(2) tetrahedron; (6,6,7) (6,7,6) (7,6,6) form a second one sharing
// (7,7,7). Every particle therefore has three or more same-sign neighbours at
// exactly sqrt(2) — the "two nearest neighbours" heuristic has no well-defined
// answer here, while the index-ordered rule of record has exactly one.
//
// Ascending lattice index (L=16) is
//   (6,6,7)=1639 (6,7,6)=1654 (7,6,6)=1894 (7,7,7)=1911
//   (7,8,8)=1928 (8,7,8)=2168 (8,8,7)=2183
// so the rule of record locks {1,2,3} first (from a=(6,6,7)), then {4,5,6}
// (from a=(7,7,7)), and leaves particle 7 = (8,8,7) UNLOCKED: by the time
// (8,8,7) is reached as an anchor there is no higher-index partner left, and
// every earlier (a,b) pair that could have admitted it was either already
// satisfied by a lower-index c or rejected on the ratio test. The old GPU
// heuristic locked all seven.
// ---------------------------------------------------------------------------
const std::vector<Site> kClusterA = {
    {6, 6, 7, +1, 1, false},
    {6, 7, 6, +1, 2, false},
    {7, 6, 6, +1, 3, false},
    {7, 7, 7, +1, 4, false},
    {7, 8, 8, +1, 5, false},
    {8, 7, 8, +1, 6, false},
    {8, 8, 7, +1, 7, false},
};

// Cluster B — one valid equilateral triple whose HIGHEST-index member is
// pre-locked. The rule of record can then admit no triple at all: the locked
// member is rejected at the b and c levels, and the two survivors cannot make
// a third. The old GPU kernel, which read no locked[] at all, locked all
// three. This is bug (2) in isolation.
const std::vector<Site> kClusterB_locked = {
    {4, 4, 4, +1, 1, false},
    {4, 5, 5, +1, 2, false},
    {5, 4, 5, +1, 3, true},
};

// Cluster B' — the same geometry with nothing pre-locked, so the triple is
// admitted. Pins that cluster B's null result is the locked exclusion and not
// a geometry rejection.
const std::vector<Site> kClusterB_free = {
    {4, 4, 4, +1, 1, false},
    {4, 5, 5, +1, 2, false},
    {5, 4, 5, +1, 3, false},
};

// Cluster C — the same seven positions as A, but alternating states. Only
// same-state triples may bind, which exercises the state filter at the b and
// c levels under the new ordering.
const std::vector<Site> kClusterC = {
    {6, 6, 7, +1, 1, false},
    {6, 7, 6, -1, 2, false},
    {7, 6, 6, +1, 3, false},
    {7, 7, 7, -1, 4, false},
    {7, 8, 8, +1, 5, false},
    {8, 7, 8, -1, 6, false},
    {8, 8, 7, +1, 7, false},
};

}  // namespace

int main() {
    std::printf("GPU triad-binding CPU parity (FTD triad_binding, Rule 7)\n");
    constexpr int L = 16;

    compare("clusterA/1tick", L, kClusterA, 1);
    compare("clusterA/4ticks", L, kClusterA, 4);
    compare("clusterB pre-locked partner", L, kClusterB_locked, 1);
    compare("clusterB' free triple", L, kClusterB_free, 1);
    compare("clusterC mixed states", L, kClusterC, 1);

    // The configurations must actually discriminate: a cluster where every
    // manifested particle ends up locked would be satisfied by the old
    // heuristic too, and would prove nothing.
    {
        const auto a = cpu_locks(L, kClusterA, 4);
        check("clusterA locks a strict, non-empty subset (discriminating)",
              !a.empty() && a.size() < kClusterA.size());
        check("clusterA leaves exactly one particle unlocked",
              a.size() == kClusterA.size() - 1);
        check("clusterA is stable under further ticks",
              a == cpu_locks(L, kClusterA, 16));
    }
    {
        const auto locked_case = cpu_locks(L, kClusterB_locked, 1);
        const auto free_case = cpu_locks(L, kClusterB_free, 1);
        check("pre-locked partner blocks the triple (only the seed stays locked)",
              locked_case == std::vector<int>{3});
        check("same geometry binds when nothing is pre-locked",
              free_case.size() == 3);
    }

    // Repeat the GPU run: the serial kernel has a constant <<<1,1>>> topology
    // and no atomics, so its answer must be identical run to run (the property
    // the atomic-proposal kernel it replaces was introduced to protect).
    {
        bool stable = true;
        const auto first = gpu_locks(L, kClusterA, 4);
        for (int repeat = 0; repeat < 8; ++repeat)
            stable = stable && gpu_locks(L, kClusterA, 4) == first;
        check("GPU triad result is reproducible across repeated runs", stable);
    }

    std::printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}

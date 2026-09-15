// engine/tests/test_genesis_symmetry.cpp
// O_h symmetry of the manifested set for the six-axis seed (2026-09-14 finding).
#include "ftd/render_bridge.h"
#include "ftd/scenarios.h"
#include <array>
#include <cstdio>
#include <set>
#include <string>
#include <vector>

namespace {
int g_fail = 0;
void check(const std::string& name, bool ok) {
    std::printf("  [%s] %s\n", ok ? "PASS" : "FAIL", name.c_str());
    if (!ok) ++g_fail;
}
using I3 = std::array<int, 3>;
// All 48 signed permutation matrices as (perm, sign) pairs.
std::vector<std::pair<I3, I3>> ops() {
    std::vector<std::pair<I3, I3>> out;
    const I3 perms[6] = {{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
    for (const auto& p : perms)
        for (int s0 : {1,-1}) for (int s1 : {1,-1}) for (int s2 : {1,-1})
            out.push_back({p, {s0, s1, s2}});
    return out;
}
std::set<I3> manifested(ftd::RenderBridge& rb, int L) {
    std::set<I3> s; const int c = (L - 1) / 2;
    for (int x = 0; x < L; ++x) for (int y = 0; y < L; ++y) for (int z = 0; z < L; ++z)
        if (rb.voxel_at(x, y, z).state != 0) s.insert({x - c, y - c, z - c});
    return s;
}
int broken(const std::set<I3>& s) {
    static const auto O = ops(); int n = 0;
    for (const auto& o : s) {
        bool full = true;
        for (const auto& op : O) {
            I3 im{op.second[0]*o[op.first[0]], op.second[1]*o[op.first[1]], op.second[2]*o[op.first[2]]};
            if (!s.count(im)) { full = false; break; }
        }
        if (!full) ++n;
    }
    return n;
}
struct Run { int first_break = -1; int n120 = 0; int broken120 = 0; };
Run run(bool deterministic, bool gauss, int L, int ticks) {
    ftd::RenderBridge rb(L); rb.force_cpu();
    if (!ftd::dispatch_scenario(rb, "s0-seed-emergent-ic1-isotropic-viz")) return Run{-2, 0, 0};
    rb.toggles.genesis_deterministic = deterministic;
    rb.toggles.gauss_projection = gauss;
    Run r;
    for (int t = 1; t <= ticks; ++t) {
        rb.tick();
        const auto s = manifested(rb, L); const int b = broken(s);
        if (b > 0 && r.first_break < 0) r.first_break = t;
        if (t == ticks) { r.n120 = (int)s.size(); r.broken120 = b; }
    }
    return r;
}
}  // namespace

int main() {
    std::printf("=== genesis O_h symmetry (six-axis seed, L=33) ===\n");
    const Run det = run(true, false, 33, 120);
    std::printf("  deterministic, gauss off: first_break=%d n=%d broken=%d\n", det.first_break, det.n120, det.broken120);
    check("scenario dispatches", det.first_break != -2);
    check("deterministic genesis without Gauss keeps the manifested set O_h-invariant for 120 ticks", det.first_break == -1 && det.n120 > 0);
    const Run shipped = run(false, true, 33, 120);
    std::printf("  shipped (draw + gauss): first_break=%d n=%d broken=%d\n", shipped.first_break, shipped.n120, shipped.broken120);
    check("shipped stochastic manifestation breaks O_h by tick 1 (mechanism pin)", shipped.first_break == 1);
    const Run a = run(true, true, 33, 60), b = run(true, true, 33, 60);
    std::printf("  deterministic + gauss: first_break=%d (asymmetry from the SOR solve; replay-checked only)\n", a.first_break);
    check("deterministic genesis with Gauss replays exactly", a.first_break == b.first_break && a.n120 == b.n120 && a.broken120 == b.broken120);
    std::printf("%d failure(s)\n", g_fail);
    return g_fail ? 1 : 0;
}

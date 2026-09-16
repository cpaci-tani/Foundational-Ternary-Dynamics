// engine/tests/test_gauss_projection_symmetry.cpp
// Gauss-projection O_h asymmetry versus SOR sweep count (2026-09-14 finding).
#include "ftd/render_bridge.h"
#include "ftd/scenarios.h"
#include <array>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace {
int g_fail = 0;
void check(const std::string& n, bool ok) { std::printf("  [%s] %s\n", ok ? "PASS" : "FAIL", n.c_str()); if (!ok) ++g_fail; }
using I3 = std::array<int, 3>;
std::vector<std::pair<I3, I3>> ops() {
    std::vector<std::pair<I3, I3>> out;
    const I3 perms[6] = {{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
    for (const auto& p : perms) for (int s0 : {1,-1}) for (int s1 : {1,-1}) for (int s2 : {1,-1}) out.push_back({p, {s0,s1,s2}});
    return out;
}
// max over cells and ops of | |J(x)| - |J(op x)| | / max |J|
double asymmetry(ftd::RenderBridge& rb, int L) {
    static const auto O = ops(); const int c = (L - 1) / 2;
    double vmax = 0, worst = 0;
    for (int x = 0; x < L; ++x) for (int y = 0; y < L; ++y) for (int z = 0; z < L; ++z)
        vmax = std::max(vmax, std::sqrt(rb.voxel_at(x, y, z).flux.mag2()));
    if (vmax <= 0) return 0;
    for (int x = 0; x < L; ++x) for (int y = 0; y < L; ++y) for (int z = 0; z < L; ++z) {
        const double a = std::sqrt(rb.voxel_at(x, y, z).flux.mag2());
        const int o[3] = {x - c, y - c, z - c};
        for (const auto& op : O) {
            const int ix = op.second[0]*o[op.first[0]] + c, iy = op.second[1]*o[op.first[1]] + c, iz = op.second[2]*o[op.first[2]] + c;
            worst = std::max(worst, std::fabs(a - std::sqrt(rb.voxel_at(ix, iy, iz).flux.mag2())));
        }
    }
    return worst / vmax;
}
double run(bool gauss, int sweeps, int L, int ticks) {
    ftd::RenderBridge rb(L); rb.force_cpu();
    if (!ftd::dispatch_scenario(rb, "s0-seed-emergent-ic1-isotropic-viz")) return -1;
    rb.toggles.genesis = false;
    rb.toggles.gauss_projection = gauss;
    rb.set_sor_iterations(sweeps);
    for (int t = 0; t < ticks; ++t) rb.tick();
    return asymmetry(rb, L);
}
}  // namespace

int main() {
    std::printf("=== Gauss projection O_h asymmetry vs SOR sweeps (six-axis seed, genesis off, L=33) ===\n");
    const double wave = run(false, 6, 33, 20);
    std::printf("  wave only, 20 ticks: %.3e\n", wave);
    check("pure stencil update is O_h-covariant to 1e-12", wave >= 0 && wave < 1e-12);
    const double a6 = run(true, 6, 33, 1), a30 = run(true, 30, 33, 1), a150 = run(true, 150, 33, 1);
    std::printf("  gauss on, 1 tick: sweeps 6 -> %.3e, 30 -> %.3e, 150 -> %.3e\n", a6, a30, a150);
    check("asymmetry is non-increasing in sweep count", a6 >= a30 && a30 >= a150);
    check("150 sweeps reduce the 6-sweep asymmetry at least tenfold", a150 < a6 / 10.0);
    check("6-sweep asymmetry is at the measured ~1e-2 scale (regression pin)", a6 > 1e-3 && a6 < 1e-1);
    std::printf("%d failure(s)\n", g_fail);
    return g_fail ? 1 : 0;
}

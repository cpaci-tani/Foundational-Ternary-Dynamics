// engine/tests/test_stencil18_dispersion.cpp
// Plane-wave frequencies in axial, face-diagonal and body-diagonal directions
// versus the leapfrog prediction from the 18-point stencil symbol (2026-09-14).
#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace {
int g_fail = 0;
void check(const std::string& n, bool ok) { std::printf("  [%s] %s\n", ok ? "PASS" : "FAIL", n.c_str()); if (!ok) ++g_fail; }
constexpr double PI = 3.14159265358979323846;
double lhat(double qx, double qy, double qz) {
    const double cx = std::cos(qx), cy = std::cos(qy), cz = std::cos(qz);
    return -4.0 + (2.0 / 3.0) * (cx + cy + cz) + (2.0 / 3.0) * (cx * cy + cy * cz + cz * cx);
}
struct Case { const char* label; double kx, ky, kz; double px, py, pz; };
double measure(const Case& c, int L, int ticks) {
    ftd::RenderBridge rb(L); rb.force_cpu();
    rb.toggles.disable_all(); rb.toggles.wave_propagation = true;
    const double amp = 0.01;
    for (int x = 0; x < L; ++x) for (int y = 0; y < L; ++y) for (int z = 0; z < L; ++z) {
        const double a = amp * std::cos(c.kx * x + c.ky * y + c.kz * z);
        auto& v = rb.voxel_at(x, y, z); v.flux = ftd::Vec3(a * c.px, a * c.py, a * c.pz); v.wave_vel = ftd::Vec3();
    }
    std::vector<double> s; s.reserve(ticks + 1);
    auto probe = [&]() { const auto f = rb.voxel_at(0, 0, 0).flux; return c.py != 0 ? f.y : (c.pz != 0 ? f.z : f.x); };
    s.push_back(probe());
    for (int t = 0; t < ticks; ++t) { rb.tick(); s.push_back(probe()); }
    std::vector<double> cross;
    for (size_t t = 1; t < s.size(); ++t) {
        const double a = s[t - 1], b = s[t];
        if ((a < 0 && b >= 0) || (a > 0 && b <= 0)) cross.push_back((t - 1) + a / (a - b));
    }
    if (cross.size() < 4) return -1;
    const double n = (double)cross.size(), mx = (n - 1) / 2; double my = 0; for (double x : cross) my += x; my /= n;
    double sxy = 0, sxx = 0; for (size_t i = 0; i < cross.size(); ++i) { sxy += (i - mx) * (cross[i] - my); sxx += (i - mx) * (i - mx); }
    return 2 * PI / (2 * sxy / sxx);
}
}  // namespace

int main() {
    const int L = 33; const double q = 2 * PI / L;
    std::printf("=== 18-point stencil dispersion vs leapfrog prediction (L=%d) ===\n", L);
    const Case cases[] = {
        {"axial (1,0,0) m=1",     q, 0, 0,      0, 1, 0},
        {"face-diag (1,1,0) m=1", q, q, 0,      0, 0, 1},
        {"body-diag (1,1,1) m=1", q, q, q,      std::sqrt(0.5), -std::sqrt(0.5), 0},
        {"axial (1,0,0) m=2",     2 * q, 0, 0,  0, 1, 0},
    };
    for (const auto& c : cases) {
        const double w = measure(c, L, 400);
        const double w_leap = 2 * std::asin(std::min(1.0, 0.5 * ftd::C_SPEED * std::sqrt(-lhat(c.kx, c.ky, c.kz))));
        std::printf("  %-24s omega=%.7f predicted=%.7f ratio=%.7f\n", c.label, w, w_leap, w / w_leap);
        check(std::string(c.label) + " matches the leapfrog prediction to 1e-5", w > 0 && std::fabs(w / w_leap - 1.0) < 1e-5);
    }
    const double k = q, ax = 2 * std::asin(std::min(1.0, 0.5 * ftd::C_SPEED * std::sqrt(-lhat(k, 0, 0))));
    check("axial full-symbol prediction equals 2 asin(C sin(k/2))", std::fabs(ax - 2 * std::asin(ftd::C_SPEED * std::sin(0.5 * k))) < 1e-14);
    std::printf("%d failure(s)\n", g_fail);
    return g_fail ? 1 : 0;
}

/**
 * @file campaign_topological_charge_transport.cpp
 * @brief FTD-0398 terminal transport test for the existing octahedral
 *        Berg--Luescher charge convention.
 *
 * This is a test instrument only. It changes no production API or engine
 * behavior. The frozen protocol and outcome precedence live in
 * PREREG_TOPOLOGICAL_CHARGE_TRANSPORT_v1.md.
 */

#define _USE_MATH_DEFINES
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

namespace {

constexpr int kL = 17;
constexpr int kMaxTick = 8;
constexpr int kFreezeTick = 2;
constexpr int kMaxRadius = 6;
constexpr double kFieldFloor = 1e-12;
constexpr double kGateTolerance = 1e-9;

struct SeedSpec {
    const char* name;
    double ox, oy, oz;
    double amp, sigma, cut_r;
    double known_freeze_e_half;
};

const SeedSpec kSeeds[] = {
    {"A_baseline", 0.31, 0.17, 0.07, 3.00, 0.45, 4.0, 1.368676308503},
    {"C_hot",      0.31, 0.17, 0.07, 5.00, 0.45, 4.0, 5.828246462835},
    {"E_cold",     0.31, 0.17, 0.07, 2.15, 0.45, 4.0, 0.540720277788},
};

struct ChargeResult {
    double q = 0.0;
    double min_j = 0.0;
    bool valid = false;
};

double solid_angle(const ftd::Vec3& a, const ftd::Vec3& b, const ftd::Vec3& c) {
    const double numerator = a.dot(ftd::Vec3::cross(b, c));
    const double denominator = 1.0 + a.dot(b) + b.dot(c) + c.dot(a);
    return 2.0 * std::atan2(numerator, denominator);
}

ChargeResult charge_from_vertices(const std::array<ftd::Vec3, 6>& raw) {
    std::array<ftd::Vec3, 6> n{};
    double min_j = raw[0].mag();
    for (int i = 0; i < 6; ++i) {
        const double mag = raw[i].mag();
        min_j = std::min(min_j, mag);
        if (!(mag > kFieldFloor)) return {0.0, min_j, false};
        n[i] = raw[i] * (1.0 / mag);
    }

    double total = 0.0;
    for (int sx = 0; sx < 2; ++sx)
    for (int sy = 0; sy < 2; ++sy)
    for (int sz = 0; sz < 2; ++sz) {
        const int ix = sx == 0 ? 0 : 1;
        const int iy = sy == 0 ? 2 : 3;
        const int iz = sz == 0 ? 4 : 5;
        total += ((sx + sy + sz) % 2 == 1)
            ? solid_angle(n[ix], n[iz], n[iy])
            : solid_angle(n[ix], n[iy], n[iz]);
    }
    return {total / (4.0 * M_PI), min_j, true};
}

ChargeResult shell_charge(const ftd::RenderBridge& rb,
                          int cx, int cy, int cz, int radius) {
    const int L = rb.lattice().size();
    if (radius < 1 || cx - radius < 0 || cx + radius >= L ||
        cy - radius < 0 || cy + radius >= L ||
        cz - radius < 0 || cz + radius >= L) {
        return {0.0, 0.0, false};
    }
    const auto& voxels = rb.voxels();
    const auto idx = [L](int x, int y, int z) { return (x * L + y) * L + z; };
    const std::array<ftd::Vec3, 6> raw = {
        voxels[idx(cx + radius, cy, cz)].flux,
        voxels[idx(cx - radius, cy, cz)].flux,
        voxels[idx(cx, cy + radius, cz)].flux,
        voxels[idx(cx, cy - radius, cz)].flux,
        voxels[idx(cx, cy, cz + radius)].flux,
        voxels[idx(cx, cy, cz - radius)].flux,
    };
    return charge_from_vertices(raw);
}

double e_half(const ftd::RenderBridge& rb) {
    double value = 0.0;
    for (const auto& v : rb.voxels()) value += v.flux.mag2();
    return 0.5 * value;
}

int manifested_count(const ftd::RenderBridge& rb) {
    int count = 0;
    for (const auto& v : rb.voxels()) if (v.state != 0) ++count;
    return count;
}

bool first_manifested_site(const ftd::RenderBridge& rb, int& mx, int& my, int& mz) {
    const int L = rb.lattice().size();
    const auto& voxels = rb.voxels();
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        if (voxels[(x * L + y) * L + z].state != 0) {
            mx = x; my = y; mz = z;
            return true;
        }
    }
    mx = my = mz = -1;
    return false;
}

void configure(ftd::RenderBridge& rb) {
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis = true;
    rb.toggles.damping = true;
    rb.toggles.selective_damping = true;
}

void seed_radial_pulse(ftd::RenderBridge& rb, const SeedSpec& s) {
    const double cx = (kL - 1) / 2.0 + s.ox;
    const double cy = (kL - 1) / 2.0 + s.oy;
    const double cz = (kL - 1) / 2.0 + s.oz;
    for (int x = std::max(0, static_cast<int>(cx - s.cut_r));
         x <= std::min(kL - 1, static_cast<int>(cx + s.cut_r) + 1); ++x)
    for (int y = std::max(0, static_cast<int>(cy - s.cut_r));
         y <= std::min(kL - 1, static_cast<int>(cy + s.cut_r) + 1); ++y)
    for (int z = std::max(0, static_cast<int>(cz - s.cut_r));
         z <= std::min(kL - 1, static_cast<int>(cz + s.cut_r) + 1); ++z) {
        const double dx = x - cx, dy = y - cy, dz = z - cz;
        const double r2 = dx * dx + dy * dy + dz * dz;
        if (r2 > s.cut_r * s.cut_r) continue;
        const double r = std::sqrt(r2);
        if (r < 1e-9) continue;
        const double amp = s.amp * std::exp(-r2 / (2.0 * s.sigma * s.sigma));
        if (amp < 1e-9) continue;
        rb.inject_flux_add(x, y, z,
            ftd::Vec3(amp * dx / r, amp * dy / r, amp * dz / r));
    }
}

bool synthetic_gates() {
    constexpr double theta = 0.7;
    const double c = std::cos(theta), s = std::sin(theta);
    for (int radius = 1; radius <= kMaxRadius; ++radius) {
        const double r = static_cast<double>(radius);
        const std::array<ftd::Vec3, 6> radial = {
            ftd::Vec3(r,0,0), ftd::Vec3(-r,0,0), ftd::Vec3(0,r,0),
            ftd::Vec3(0,-r,0), ftd::Vec3(0,0,r), ftd::Vec3(0,0,-r),
        };
        std::array<ftd::Vec3, 6> inverted{}, rotated{}, scaled{};
        const double scales[6] = {5.0, 0.1, 3.0, 2.0, 0.5, 4.0};
        for (int i = 0; i < 6; ++i) {
            inverted[i] = radial[i] * -1.0;
            rotated[i] = ftd::Vec3(c * radial[i].x - s * radial[i].y,
                                   s * radial[i].x + c * radial[i].y,
                                   radial[i].z);
            scaled[i] = radial[i] * scales[i];
        }
        const auto q_plus = charge_from_vertices(radial);
        const auto q_minus = charge_from_vertices(inverted);
        const auto q_rot = charge_from_vertices(rotated);
        const auto q_scale = charge_from_vertices(scaled);
        if (!q_plus.valid || !q_minus.valid || !q_rot.valid || !q_scale.valid ||
            std::fabs(q_plus.q - 1.0) > 1e-12 ||
            std::fabs(q_minus.q + 1.0) > 1e-12 ||
            std::fabs(q_rot.q - q_plus.q) > 1e-12 ||
            std::fabs(q_scale.q - q_plus.q) > 1e-12) return false;
    }
    return true;
}

bool discover_center(const SeedSpec& spec, int& cx, int& cy, int& cz) {
    ftd::RenderBridge rb(kL);
    rb.force_cpu();
    configure(rb);
    seed_radial_pulse(rb, spec);
    for (int tick = 0; tick <= kMaxTick; ++tick) {
        if (first_manifested_site(rb, cx, cy, cz)) return tick == kFreezeTick;
        if (tick < kMaxTick) rb.tick();
    }
    return false;
}

bool run_seed(const SeedSpec& spec) {
    int cx = -1, cy = -1, cz = -1;
    if (!discover_center(spec, cx, cy, cz)) {
        std::fprintf(stderr, "GATE,%s,manifestation_reproduction,FAIL\n", spec.name);
        return false;
    }
    if (cx - kMaxRadius < 0 || cx + kMaxRadius >= kL ||
        cy - kMaxRadius < 0 || cy + kMaxRadius >= kL ||
        cz - kMaxRadius < 0 || cz + kMaxRadius >= kL) {
        std::fprintf(stderr, "GATE,%s,boundary_safety,FAIL\n", spec.name);
        return false;
    }

    ftd::RenderBridge rb(kL);
    rb.force_cpu();
    configure(rb);
    seed_radial_pulse(rb, spec);
    bool freeze_gate = false;
    for (int tick = 0; tick <= kMaxTick; ++tick) {
        int mx = -1, my = -1, mz = -1;
        first_manifested_site(rb, mx, my, mz);
        const double energy = e_half(rb);
        for (int radius = 1; radius <= kMaxRadius; ++radius) {
            const auto result = shell_charge(rb, cx, cy, cz, radius);
            std::printf("%s,%d,%d,%.17g,%.17g,%d,%.17g,%d,%d,%d\n",
                        spec.name, tick, radius, result.q, result.min_j,
                        result.valid ? 1 : 0, energy, mx, my, mz);
            if (tick == kFreezeTick && radius == 1) {
                freeze_gate = result.valid && std::fabs(result.q) <= 5e-9 &&
                    std::fabs(energy - spec.known_freeze_e_half) < kGateTolerance &&
                    mx == cx && my == cy && mz == cz && manifested_count(rb) == 1;
            }
        }
        if (tick < kMaxTick) rb.tick();
    }
    std::fprintf(stderr, "GATE,%s,freeze_R1_FTD0392_and_energy,%s\n",
                 spec.name, freeze_gate ? "PASS" : "FAIL");
    return freeze_gate;
}

}  // namespace

int main() {
    if (std::getenv("FTD_FORCE_GPU") != nullptr) {
        std::fprintf(stderr, "GATE,FTD_FORCE_GPU_unset,FAIL\n");
        return 2;
    }
    if (!synthetic_gates()) {
        std::fprintf(stderr, "GATE,synthetic_charge_convention,FAIL\n");
        return 2;
    }
    std::fprintf(stderr, "GATE,synthetic_charge_convention,PASS\n");
    std::fprintf(stderr, "GATE,cpu_forced_and_effective_toggles,PASS\n");
    std::printf("seed,tick,radius,Q,min_j,valid,e_half,manifest_x,manifest_y,manifest_z\n");
    bool ok = true;
    for (const auto& seed : kSeeds) ok = run_seed(seed) && ok;
    return ok ? 0 : 2;
}

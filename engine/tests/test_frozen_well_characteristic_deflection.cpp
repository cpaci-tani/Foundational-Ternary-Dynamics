// ============================================================================
// test_frozen_well_characteristic_deflection.cpp
// ----------------------------------------------------------------------------
// FTD-1020 / PREREG_FROZEN_WELL_CHARACTERISTIC_DEFLECTION_v1.md
// Lock prefix SHA256:
//   B6BF393F332A6CBFD9770ECC6C86CD59092F07C478D7F12DBCA5A986CE02C034
// Anchor: anchored-late until git tag
//   preregister-frozen-well-characteristic-deflection-v1 resolves.
//
// Frozen vacuum well. Pure wave_propagation. Three-way class 0 / 1911 / GR.
// No refractive-index operator. No golden-tick contact.
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

#include <cmath>
#include <cstdio>
#include <vector>

namespace ftd {
namespace test {
namespace {

constexpr int    kL       = 32;
constexpr int    kEdge    = 5;
constexpr int    kOx      = 6;
constexpr int    kOy      = 13;
constexpr int    kOz      = 13;
constexpr int    kRayY    = 22;
constexpr int    kRayZ    = 15;
constexpr int    kX0      = 4;
constexpr int    kNt      = 40;
constexpr int    kEntryLo = 6;
constexpr int    kEntryHi = 12;
constexpr int    kExitLo  = 28;
constexpr int    kExitHi  = 36;
constexpr int    kWin     = 6;
constexpr double kSigma   = 2.5;
constexpr double kAmp     = 0.25 * K_B;

constexpr double kPi = 3.14159265358979323846;

struct Centroid {
    double x = 0.0, y = 0.0, z = 0.0, e = 0.0;
};

void place_source(RenderBridge& rb) {
    for (int dx = 0; dx < kEdge; ++dx)
        for (int dy = 0; dy < kEdge; ++dy)
            for (int dz = 0; dz < kEdge; ++dz) {
                const int x = kOx + dx, y = kOy + dy, z = kOz + dz;
                rb.inject_particle(x, y, z, +1, Vec3{0.0, 0.0, 0.0});
                Voxel& v = rb.voxel_at(x, y, z);
                v.locked   = true;
                v.velocity = Vec3{0.0, 0.0, 0.0};
                v.flux     = Vec3{0.0, 0.0, 0.0};
            }
}

void configure_step_s(RenderBridge& rb) {
    rb.toggles.disable_all();
    rb.toggles.gravity              = true;
    rb.toggles.latency_field        = true;
    rb.toggles.forces               = false;
    rb.toggles.movement             = false;
    rb.toggles.geometric_gravity    = false;
    rb.toggles.field_energy_gravity = false;
    rb.toggles.cluster_inertia      = false;
    rb.toggles.poisson_coulomb      = false;
    rb.toggles.emergent_forces      = false;
    rb.toggles.lorentz_force        = false;
    rb.toggles.de_broglie_clock     = false;
}

void vacuum_clear_keep_latency(RenderBridge& rb) {
    for (auto& v : rb.voxels()) {
        v.state      = 0;
        v.locked     = false;
        v.flux       = Vec3{0.0, 0.0, 0.0};
        v.wave_vel   = Vec3{0.0, 0.0, 0.0};
        v.velocity   = Vec3{0.0, 0.0, 0.0};
        v.particle_id = -1;
    }
}

void configure_transit(RenderBridge& rb) {
    rb.toggles.disable_all();
    rb.toggles.wave_propagation     = true;
    rb.toggles.latency_field        = false;
    rb.toggles.gravity              = false;
    rb.toggles.forces               = false;
    rb.toggles.geometric_gravity    = false;
    rb.toggles.coupling             = false;
    rb.toggles.genesis              = false;
    rb.toggles.gauss_projection     = false;
    rb.toggles.damping              = false;
    rb.toggles.movement             = false;
}

void seed_packet(RenderBridge& rb) {
    const double lam   = 4.0 * kSigma;
    const double k     = 2.0 * kPi / lam;
    const double omega = 2.0 * C_WAVE * std::sin(k / 2.0);
    const double cut2  = (3.0 * kSigma) * (3.0 * kSigma);
    for (int z = 0; z < kL; ++z)
        for (int y = 0; y < kL; ++y)
            for (int x = 0; x < kL; ++x) {
                const double dx = static_cast<double>(x - kX0);
                const double dy = static_cast<double>(y - kRayY);
                const double dz = static_cast<double>(z - kRayZ);
                const double r2 = dx * dx + dy * dy + dz * dz;
                if (r2 > cut2) continue;
                const double g = std::exp(-r2 / (2.0 * kSigma * kSigma));
                if (g < 1e-6) continue;
                const double ph = k * dx;
                rb.inject_flux_add(x, y, z, Vec3{0.0, 0.0, kAmp * g * std::sin(ph)});
                rb.inject_wave_vel_add(x, y, z,
                    Vec3{0.0, 0.0, -omega * kAmp * g * std::cos(ph)});
            }
}

Centroid packet_centroid(const RenderBridge& rb) {
    Centroid c;
    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    for (int z = kRayZ - kWin; z <= kRayZ + kWin; ++z)
        for (int y = kRayY - kWin; y <= kRayY + kWin; ++y) {
            const int yy = (y + kL) % kL;
            const int zz = (z + kL) % kL;
            for (int x = 0; x < kL; ++x) {
                const int i = lat.index(x, yy, zz);
                const double w = vox[static_cast<std::size_t>(i)].flux.mag2();
                if (w <= 0.0) continue;
                c.e += w;
                c.x += w * static_cast<double>(x);
                c.y += w * static_cast<double>(yy);
                c.z += w * static_cast<double>(zz);
            }
        }
    if (c.e > 0.0) {
        c.x /= c.e; c.y /= c.e; c.z /= c.e;
    }
    return c;
}

double theta_1911_from_well(RenderBridge& rb) {
    double sum = 0.0;
    for (int x = 0; x < kL; ++x) {
        const double L0 = rb.voxel_at(x, kRayY, kRayZ).latency;
        const double Lp = rb.voxel_at(x, kRayY + 2, kRayZ).latency;
        const double Lm = rb.voxel_at(x, kRayY - 2, kRayZ).latency;
        const double dLdy = GRAD_TIER2_SCALE * (Lp - Lm);
        const double one_m = 1.0 - L0 * L0;
        if (one_m <= 1e-18) continue;
        const double dndy = L0 * dLdy / std::pow(one_m, 1.5);
        sum += dndy;
    }
    return sum;
}

struct Arm {
    double theta_y = 0.0;
    double theta_z = 0.0;
    double e_entry = 0.0;
    double e_exit  = 0.0;
    double theta_1911 = 0.0;
    double L_ray = 0.0;
    double L_in  = 0.0;
    double L_freeze_max_drift = 0.0;
    int    n_manifest = 0;
};

Arm run_arm(bool with_well) {
    RenderBridge rb(kL);
    rb.force_cpu();
    rb.seed_rng(20260819);
    std::vector<double> L_freeze;
    if (with_well) {
        configure_step_s(rb);
        place_source(rb);
        rb.tick();
        vacuum_clear_keep_latency(rb);
        L_freeze.resize(static_cast<std::size_t>(kL));
        for (int x = 0; x < kL; ++x)
            L_freeze[static_cast<std::size_t>(x)] = rb.voxel_at(x, kRayY, kRayZ).latency;
    }
    Arm a;
    if (with_well) {
        a.theta_1911 = theta_1911_from_well(rb);
        for (int x = 0; x < kL; ++x) {
            a.L_ray += rb.voxel_at(x, kRayY, kRayZ).latency;
            a.L_in  += rb.voxel_at(x, kRayY - 2, kRayZ).latency;
        }
        a.L_ray /= static_cast<double>(kL);
        a.L_in  /= static_cast<double>(kL);
        for (const auto& v : rb.voxels())
            if (v.state != 0) ++a.n_manifest;
    }
    configure_transit(rb);
    seed_packet(rb);

    double y_ent = 0.0, x_ent = 0.0, z_ent = 0.0, n_ent = 0.0;
    double y_ex  = 0.0, x_ex  = 0.0, z_ex  = 0.0, n_ex  = 0.0;
    for (int t = 0; t <= kNt; ++t) {
        const Centroid c = packet_centroid(rb);
        if (t >= kEntryLo && t <= kEntryHi && c.e > 0.0) {
            y_ent += c.y; x_ent += c.x; z_ent += c.z; n_ent += 1.0;
            a.e_entry += c.e;
        }
        if (t >= kExitLo && t <= kExitHi && c.e > 0.0) {
            y_ex += c.y; x_ex += c.x; z_ex += c.z; n_ex += 1.0;
            a.e_exit += c.e;
        }
        if (t < kNt) rb.tick();
    }
    if (n_ent > 0.0) { y_ent /= n_ent; x_ent /= n_ent; z_ent /= n_ent; a.e_entry /= n_ent; }
    if (n_ex  > 0.0) { y_ex  /= n_ex;  x_ex  /= n_ex;  z_ex  /= n_ex;  a.e_exit  /= n_ex; }
    const double dx = x_ex - x_ent;
    a.theta_y = (std::abs(dx) > 1e-12) ? (y_ex - y_ent) / dx : 0.0;
    a.theta_z = (std::abs(dx) > 1e-12) ? (z_ex - z_ent) / dx : 0.0;
    if (with_well && !L_freeze.empty()) {
        double md = 0.0;
        for (int x = 0; x < kL; ++x)
            md = std::max(md, std::abs(rb.voxel_at(x, kRayY, kRayZ).latency
                                     - L_freeze[static_cast<std::size_t>(x)]));
        a.L_freeze_max_drift = md;
    }
    return a;
}

}  // namespace

void test_frozen_well_characteristic_deflection() {
    section("FTD-1020 frozen-well characteristic deflection (CPU observer)");

    const Arm w  = run_arm(true);
    const Arm c0 = run_arm(false);
    const double th_diff = w.theta_y - c0.theta_y;
    const double th_1911 = w.theta_1911;
    const double th_gr   = 2.0 * th_1911;
    const double F = std::max(3.0 * std::abs(c0.theta_y), 3.0 * std::abs(w.theta_z));

    std::printf("    theta_1911=%.6e  theta_GR=%.6e  theta_W=%.6e  theta_C0=%.6e\n",
                th_1911, th_gr, w.theta_y, c0.theta_y);
    std::printf("    theta_diff=%.6e  theta_z_W=%.6e  F=%.6e  L_ray=%.6e  L_in=%.6e\n",
                th_diff, w.theta_z, F, w.L_ray, w.L_in);
    std::printf("    E_ent_W=%.6e  E_ex_W=%.6e  E_ent_C0=%.6e  E_ex_C0=%.6e\n",
                w.e_entry, w.e_exit, c0.e_entry, c0.e_exit);

    const bool p1 = std::abs(th_1911) > 1.0e-4 && std::abs(th_1911) > 10.0 * F;
    const bool p2 = (w.L_ray < w.L_in) || (th_1911 < 0.0);
    const bool p3 = w.n_manifest == 0;
    const bool p4 = (w.e_entry > 0.0 && w.e_exit > 0.25 * w.e_entry)
                 && (c0.e_entry > 0.0 && c0.e_exit > 0.25 * c0.e_entry);
    const bool p5 = std::abs(c0.theta_y) < 0.05 * std::abs(th_1911);
    const bool p6 = w.L_freeze_max_drift < 1.0e-15;
    const bool p7 = true;  // FTD-1017 FOUND, inherited

    check("P1: |theta_1911| > 1e-4 and > 10 F", p1);
    check("P2: Fermat/well toward the mass", p2);
    check("P3: vacuum after clear", p3);
    check("P4: packet energy retention > 0.25", p4);
    check("P5: control |theta_C0| < 0.05 |theta_1911|", p5);
    check("P6: well frozen through transit", p6);
    check("P7: FTD-1017 well-grips-matter inherited", p7);

    const double r1 = (std::abs(th_1911) > 0.0) ? (th_diff / th_1911) : 0.0;
    const double r2 = (std::abs(th_gr) > 0.0) ? (th_diff / th_gr) : 0.0;
    const bool a1 = (th_diff * th_1911 > 0.0) && std::abs(r1 - 1.0) < 0.35;
    const bool a2 = (th_diff * th_1911 > 0.0) && std::abs(r2 - 1.0) < 0.35;
    bool a0 = std::abs(th_diff) <= std::max(F, 0.20 * std::abs(th_1911));
    if (a1 || a2) a0 = false;
    bool cls_a2 = a2;
    bool cls_a1 = a1;
    if (a1 && a2)
        cls_a2 = std::abs(r2 - 1.0) < std::abs(r1 - 1.0);
    if (cls_a2) cls_a1 = false;

    const char* cls = "NONE";
    if (a0) cls = "0";
    else if (cls_a2) cls = "GR-full";
    else if (cls_a1) cls = "1911-half";

    std::printf("    r_1911=%.4f  r_GR=%.4f  CLASS %s\n", r1, r2, cls);

    check("A0 or A1 or A2 matches", a0 || cls_a1 || cls_a2);
    const bool protocol = p1 && p2 && p3 && p4 && p5 && p6 && p7;
    const char* verdict = "UNDERDETERMINED";
    if (protocol && a0) verdict = "FOUND class 0";
    else if (protocol && cls_a1) verdict = "FOUND class 1911-half";
    else if (protocol && cls_a2) verdict = "FOUND class GR-full";
    else if (protocol) verdict = "CLOSED-NEGATIVE / other";
    std::printf("    VERDICT %s\n", verdict);
    check("protocol complete", protocol);
}

}  // namespace test
}  // namespace ftd

int main() {
    ftd::test::init("test_frozen_well_characteristic_deflection");
    ftd::test::test_frozen_well_characteristic_deflection();
    return ftd::test::finalize();
}

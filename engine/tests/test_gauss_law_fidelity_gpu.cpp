// ============================================================================
// test_gauss_law_fidelity_gpu.cpp  (engine-fidelity investigation, 2026-07-16)
// ----------------------------------------------------------------------------
// GPU/FFT counterpart of test_gauss_law_fidelity.cpp: measures the realized
// fraction of the Gauss constraint div(J) = s at a manifested charge site on
// the CUDA path, where gauss_project is a cuFFT spectral solve against the
// EXACT 18-point Laplacian eigenvalues (gpu_buffers.cu:786-807,
// kernel_precompute_green) — i.e. the infinite-SOR-iteration limit of the CPU
// solver, with the SAME 18-pt-solve / 6-pt-central-divergence stencil
// mismatch. The GPU correction kernel (kernels_poisson.cu:346) skips
// manifested sites unconditionally: there is NO exact_dual_gauss branch on
// the GPU, so that mechanism is CPU-only.
//
// Experiments:
//   G0  Projection in isolation at 64^3 (only gauss_projection on): FFT
//       one-shot enforcement fraction + 200 repeated applications.
//   G1  GP-KCOMP-SHELL replica at 128^3 (enable_all, genesis/movement off,
//       wavepacket IC, 1000 ticks): ties this test to the
//       DERIV_KCOMP_VOLUMETRIC_SHELL.md measurement J(r=1)=9.898e-3,
//       J_peak=2.879e-2, and reads the realized div at the charge site that
//       the campaign never probed.
//   G2  Live defaults at 64^3, bare +1 charge, J=0: f at t=1,10,100,1000.
//   G3  Same but gauss_projection OFF (unconstrained dynamics).
//   G4  Same but damping OFF.
//
// Run via WSL2 (engine/build_wsl) per project policy — Windows-native CUDA
// is compile-check only.
// ============================================================================

#include "ftd/gpu_engine.h"
#include "ftd/constants.h"
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <vector>

using namespace ftd;

static int tests_passed = 0;
static int tests_failed = 0;

#define CHECK(cond, msg) do { \
    if (cond) { tests_passed++; std::printf("  PASS: %s\n", msg); } \
    else { tests_failed++; std::printf("  FAIL: %s\n", msg); } \
} while(0)

// ---------------------------------------------------------------------------
// Host-side probe over a synced voxel array. X-major indexing matches the
// kernels and the CPU Lattice (i = x*L*L + y*L + z).
// ---------------------------------------------------------------------------
static inline int wrapi(int a, int L) { return (a % L + L) % L; }
static inline int idx3(int x, int y, int z, int L) {
    return wrapi(x, L) * L * L + wrapi(y, L) * L + wrapi(z, L);
}

struct SiteProbe {
    double div_c = 0.0;
    double target = 0.0;
    double frac = 0.0;
    double j_rad_r1 = 0.0;
    double j_mag_r1 = 0.0;
    double vac_resid = 0.0;
};

static SiteProbe probe_site(const std::vector<Voxel>& vox, int L, int cx, int cy, int cz) {
    SiteProbe p;
    const int N = L * L * L;
    long long charge = 0;
    for (int i = 0; i < N; ++i) charge += vox[i].state;
    const double mean_charge = static_cast<double>(charge) / static_cast<double>(N);

    const int xp = idx3(cx + 1, cy, cz, L), xm = idx3(cx - 1, cy, cz, L);
    const int yp = idx3(cx, cy + 1, cz, L), ym = idx3(cx, cy - 1, cz, L);
    const int zp = idx3(cx, cy, cz + 1, L), zm = idx3(cx, cy, cz - 1, L);
    const int site = idx3(cx, cy, cz, L);

    p.div_c = (vox[xp].flux.x - vox[xm].flux.x) * 0.5
            + (vox[yp].flux.y - vox[ym].flux.y) * 0.5
            + (vox[zp].flux.z - vox[zm].flux.z) * 0.5;
    p.target = static_cast<double>(vox[site].state) - mean_charge;  // coupling = 1.0
    p.frac = (std::abs(p.target) > 1e-300) ? p.div_c / p.target : 0.0;

    p.j_rad_r1 = (  vox[xp].flux.x - vox[xm].flux.x
                  + vox[yp].flux.y - vox[ym].flux.y
                  + vox[zp].flux.z - vox[zm].flux.z ) / 6.0;
    p.j_mag_r1 = ( vox[xp].flux.mag() + vox[xm].flux.mag()
                 + vox[yp].flux.mag() + vox[ym].flux.mag()
                 + vox[zp].flux.mag() + vox[zm].flux.mag() ) / 6.0;

    // Vacuum residual at the +x face neighbor (target there: -mean_charge).
    {
        const int vx = cx + 1;
        const int nxp = idx3(vx + 1, cy, cz, L), nxm = idx3(vx - 1, cy, cz, L);
        const int nyp = idx3(vx, cy + 1, cz, L), nym = idx3(vx, cy - 1, cz, L);
        const int nzp = idx3(vx, cy, cz + 1, L), nzm = idx3(vx, cy, cz - 1, L);
        const double dv = (vox[nxp].flux.x - vox[nxm].flux.x) * 0.5
                        + (vox[nyp].flux.y - vox[nym].flux.y) * 0.5
                        + (vox[nzp].flux.z - vox[nzm].flux.z) * 0.5;
        p.vac_resid = dv - (static_cast<double>(vox[idx3(vx, cy, cz, L)].state) - mean_charge);
    }
    return p;
}

static void print_row(const char* label, int t, const SiteProbe& p) {
    std::printf("    [%-14s] t=%4d  divC=%+.4e  target=%+.4e  f=%+8.4f  "
                "Jrad(r1)=%+.4e  |J|(r1)=%.4e  vacRes=%+.2e\n",
                label, t, p.div_c, p.target, p.frac, p.j_rad_r1, p.j_mag_r1,
                p.vac_resid);
}

// ---------------------------------------------------------------------------
// G0: FFT projection in isolation at 64^3
// ---------------------------------------------------------------------------
static double g0_oneshot_frac = 0.0;

static void test_g0_projection_isolation() {
    std::printf("\n--- G0: FFT projection in isolation (64^3, bare +1, J=0) ---\n");
    constexpr int L = 64;
    constexpr int C = L / 2;

    gpu::GpuEngine gpu(L);
    gpu.toggles.disable_all();
    gpu.toggles.gauss_projection = true;
    gpu.inject_particle(C, C, C, +1, Vec3{0.0, 0.0, 0.0});

    std::vector<Voxel> vox;
    gpu.run(1);
    gpu.sync_to_host(vox);
    SiteProbe p1 = probe_site(vox, L, C, C, C);
    print_row("G0 fft t=1", 1, p1);
    g0_oneshot_frac = p1.frac;

    gpu.run(199);
    gpu.sync_to_host(vox);
    SiteProbe p200 = probe_site(vox, L, C, C, C);
    print_row("G0 fft t=200", 200, p200);

    CHECK(p1.frac > 0.01 && p1.frac < 0.99,
          "G0: one FFT (exact 18-pt solve) projection realizes a PARTIAL site fraction");
    // Even lattice: the 7 corner modes k in {0,pi}^3 have a zero central-
    // difference symbol and are NEVER corrected, but their weight in a point
    // charge is only 7/N — repeated application should still climb well above
    // the one-shot value if the mismatch analysis is right.
    CHECK(p200.frac > p1.frac - 0.02,
          "G0: repeated FFT projection does not degrade the site fraction");
}

// ---------------------------------------------------------------------------
// G1: GP-KCOMP-SHELL replica at 128^3
// ---------------------------------------------------------------------------
static void test_g1_kcomp_replica() {
    std::printf("\n--- G1: GP-KCOMP-SHELL replica (128^3, wavepacket, 1000 ticks) ---\n");
    constexpr int L = 128;
    constexpr int C = L / 2;

    gpu::GpuEngine gpu(L);
    gpu.toggles.enable_all();
    gpu.toggles.genesis = false;
    gpu.toggles.movement = false;
    gpu.inject_wavepacket(C, C, C, +1, 3.0, K_B);
    gpu.run(1000);

    std::vector<Voxel> vox;
    gpu.sync_to_host(vox);
    SiteProbe p = probe_site(vox, L, C, C, C);
    print_row("G1 kcomp t=1e3", 1000, p);

    // Campaign-binned <|J|> on the round(r)==1 shell (6 face + 12 edge sites)
    // and global J_peak — the two numbers DERIV_KCOMP_VOLUMETRIC_SHELL.md
    // reports as J(r=1)=9.898e-3 and J_peak=2.879e-2.
    double sum = 0.0; int cnt = 0;
    for (int dx = -2; dx <= 2; ++dx)
      for (int dy = -2; dy <= 2; ++dy)
        for (int dz = -2; dz <= 2; ++dz) {
            const double r = std::sqrt(double(dx*dx + dy*dy + dz*dz));
            if (static_cast<int>(std::round(r)) == 1) {
                sum += vox[idx3(C + dx, C + dy, C + dz, L)].flux.mag();
                ++cnt;
            }
        }
    double jpeak = 0.0;
    for (const auto& v : vox) jpeak = std::max(jpeak, v.flux.mag());
    const double j_r1 = sum / cnt;
    std::printf("    [G1 kcomp      ] campaign-binned <|J|>(r=1, %d sites)=%.4e  "
                "J_peak=%.4e  (doc: 9.898e-3 / 2.879e-2)\n", cnt, j_r1, jpeak);
    std::printf("    [G1 kcomp      ] realized div fraction at the charge site: f=%+.4f "
                "(exact enforcement would need Jrad(r1)=1/3)\n", p.frac);

    CHECK(j_r1 > 9.898e-3 / 3.0 && j_r1 < 9.898e-3 * 3.0,
          "G1: J(r=1) reproduces the DERIV_KCOMP measurement within 3x");
}

// ---------------------------------------------------------------------------
// G2/G3/G4: live defaults at 64^3, bare charge — toggle matrix
// ---------------------------------------------------------------------------
static void run_live(const char* label, bool gauss_on, bool damping_on) {
    constexpr int L = 64;
    constexpr int C = L / 2;

    gpu::GpuEngine gpu(L);
    gpu.toggles.enable_all();
    gpu.toggles.genesis = false;
    gpu.toggles.movement = false;
    gpu.toggles.gauss_projection = gauss_on;
    if (!damping_on) {
        gpu.toggles.damping = false;
        gpu.toggles.selective_damping = false;
    }
    gpu.inject_particle(C, C, C, +1, Vec3{0.0, 0.0, 0.0});

    std::vector<Voxel> vox;
    int t = 0;
    for (int mark : {1, 10, 100, 1000}) {
        gpu.run(mark - t);
        t = mark;
        gpu.sync_to_host(vox);
        print_row(label, t, probe_site(vox, L, C, C, C));
    }
}

static void test_g2_g3_g4_live_matrix() {
    std::printf("\n--- G2: live defaults (64^3, bare +1, J=0) ---\n");
    run_live("G2 defaults", /*gauss=*/true, /*damping=*/true);

    std::printf("\n--- G3: live, gauss_projection OFF ---\n");
    run_live("G3 gauss OFF", /*gauss=*/false, /*damping=*/true);

    std::printf("\n--- G4: live, damping OFF ---\n");
    run_live("G4 no damping", /*gauss=*/true, /*damping=*/false);
}

int main() {
    std::printf("=== test_gauss_law_fidelity_gpu: realized div(J)=s at a charge site (FFT path) ===\n");
    std::printf("    DAMPING=%.6e  G_C=%.6e  K_B=%.4f\n", DAMPING, G_C, K_B);

    test_g0_projection_isolation();
    test_g1_kcomp_replica();
    test_g2_g3_g4_live_matrix();

    std::printf("\n=== %d passed, %d failed ===\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}

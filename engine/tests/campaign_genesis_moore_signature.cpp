/**
 * @file campaign_genesis_moore_signature.cpp
 * @brief Do genesis CLUSTERS carry an O_h / Moore quantum-number signature?
 *
 * The framework has TWO decoupled pillars: (1) the dynamical substrate (genesis
 * -> clusters), and (2) the STATIC Moore Layer Theorem, which derives the SM
 * particle content (U(1)/SU(2)/SU(3) from SC/FCC/BCC shells = 1/2/3 excited
 * J-components; 3 generations from the FCC planes) from pure lattice
 * combinatorics. They are joined only by the FTD-0110 mass IDENTIFICATION
 * (cluster size N = mass), NOT by a mechanism. This instrument asks the bridge
 * question directly: does a genesis cluster's INTERNAL STRUCTURE carry any of the
 * Moore quantum numbers, or is it purely geometric?
 *
 * SHARPEST TEST: inject AXIALLY (flux = (A*K_GENESIS, 0, 0) -- a single
 * J-component, "U(1)/SC-like"), let a cluster form, and measure whether at
 * "hadron" amplitudes it SPONTANEOUSLY develops the multi-component (FCC/SU(2),
 * BCC/SU(3)/color) structure the Moore theorem assigns to heavy/colored
 * particles -- a real emergent signature -- or whether its component + shell
 * fingerprint is just a smooth GEOMETRIC function of A (null -> pillars
 * decoupled). Amplitudes span the FTD-0110 ladder (A = 2*sqrt(m/m_e): e~2,
 * mu~29, p~86). Read-only post-settle scan -- golden-neutral.
 *
 * Output (one row per manifested voxel of the settled cluster):
 *   A,seed,dx,dy,dz,shell,state,jx,jy,jz
 *
 * Usage:
 *   campaign_genesis_moore_signature --L=32 --As=2,6,10,14,20,28,40 --seeds=3 \
 *       --settle=200 --cpu --output-dir=PATH --tag=sig
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {
inline int min_image(int x, int c, int L) { return ((x - c + L / 2) % L + L) % L - L / 2; }
const char* shell_of(int dx, int dy, int dz) {
    const int r2 = dx * dx + dy * dy + dz * dz;
    if (r2 == 0) return "center";
    if (r2 == 1) return "SC";    // face   r=1  (U(1) shell)
    if (r2 == 2) return "FCC";   // edge   r=sqrt2 (SU(2)/generation shell)
    if (r2 == 3) return "BCC";   // corner r=sqrt3 (SU(3)/color shell)
    if (r2 == 4) return "SC2";
    return "outer";
}
std::vector<double> parse_list(const std::string& s) {
    std::vector<double> out; std::size_t i = 0;
    while (i < s.size()) { std::size_t j = s.find(',', i); if (j == std::string::npos) j = s.size();
        out.push_back(std::atof(s.substr(i, j - i).c_str())); i = j + 1; }
    return out;
}
} // namespace

int main(int argc, char** argv) {
    int L = 32, seeds = 3, settle = 200;
    std::string As_str = "2,6,10,14,20,28,40";
    bool diag = false, force_cpu = false;
    double gamma = 0.02, T = 0.005;
    std::uint32_t seed_base = 0xE0102000u;
    std::string tag = "sig";
    std::string output_dir = "engine/results/genesis_moore_signature/";

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if      (a.rfind("--L=", 0) == 0)          L = std::atoi(a.c_str() + 4);
        else if (a.rfind("--As=", 0) == 0)         As_str = a.substr(5);
        else if (a.rfind("--seeds=", 0) == 0)      seeds = std::atoi(a.c_str() + 8);
        else if (a.rfind("--settle=", 0) == 0)     settle = std::atoi(a.c_str() + 9);
        else if (a == "--dir=diag")                diag = true;
        else if (a == "--dir=axial")               diag = false;
        else if (a == "--cpu")                     force_cpu = true;
        else if (a.rfind("--T=", 0) == 0)          T = std::atof(a.c_str() + 4);
        else if (a.rfind("--tag=", 0) == 0)        tag = a.substr(6);
        else if (a.rfind("--output-dir=", 0) == 0) output_dir = a.substr(13);
    }

    const std::vector<double> As = parse_list(As_str);
    fs::create_directories(output_dir);
    const fs::path out_csv = fs::path(output_dir) / ("sig_" + tag + ".csv");
    std::FILE* f = std::fopen(out_csv.string().c_str(), "w");
    if (!f) { std::fprintf(stderr, "cannot open %s\n", out_csv.string().c_str()); return 1; }
    std::fprintf(f, "A,seed,dx,dy,dz,shell,state,jx,jy,jz\n");

    std::printf("genesis_moore_signature: L=%d As=%s seeds=%d settle=%d dir=%s backend=%s\n",
                L, As_str.c_str(), seeds, settle, diag ? "diag" : "axial", force_cpu ? "cpu" : "default");
    std::fflush(stdout);

    const int cx = L / 2;
    for (double A : As) {
        for (int s = 0; s < seeds; ++s) {
            ftd::RenderBridge rb(L);
            if (force_cpu) { rb.force_cpu(); rb.set_sor_iterations(150); }
            rb.toggles.disable_all();
            rb.toggles.wave_propagation = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.genesis          = true;
            rb.toggles.coupling         = true;
            rb.toggles.dual_substrate   = false;
            rb.toggles.langevin         = true;
            rb.toggles.langevin_T       = T;
            rb.toggles.langevin_gamma   = gamma;
            // Seed depends ONLY on the seed index, NOT on A -- so the same seed
            // set is used at every amplitude (no parity/amplitude confound).
            rb.seed_rng(seed_base + static_cast<std::uint32_t>(s) * 2654435761u);

            // AXIAL injection: a single J-component (x). Whether the cluster
            // develops y/z (multi-component, FCC/BCC) structure is the test.
            if (diag) { const double c3 = A * ftd::K_GENESIS / std::sqrt(3.0); rb.inject_flux(cx, cx, cx, {c3, c3, c3}); }
            else      { rb.inject_flux(cx, cx, cx, {A * ftd::K_GENESIS, 0, 0}); }

            for (int t = 0; t < settle; ++t) rb.tick();

            const auto& voxels = rb.voxels();
            int man = 0;
            for (int i = 0; i < L * L * L; ++i) {
                const auto& v = voxels[i];
                if (v.state == 0) continue;
                const int x = i / (L * L), y = (i / L) % L, z = i % L;
                const int dx = min_image(x, cx, L), dy = min_image(y, cx, L), dz = min_image(z, cx, L);
                std::fprintf(f, "%.0f,%d,%d,%d,%d,%s,%d,%.6f,%.6f,%.6f\n",
                             A, s, dx, dy, dz, shell_of(dx, dy, dz), static_cast<int>(v.state),
                             v.flux.x, v.flux.y, v.flux.z);
                ++man;
            }
            std::printf("  A=%.0f seed=%d  manifested=%d\n", A, s, man);
            std::fflush(stdout);
        }
    }
    std::fclose(f);
    std::printf("wrote %s\n", out_csv.string().c_str());
    return 0;
}

// FTD-0312 Leg B — flux equation-of-state engine measurement.
//
// QUESTION. Is the master quadratic's smaller root x- = 3.023964 (residual
// delta_c = x- - 3 = 0.024) "the dimensionless pressure of the flux"? x- is
// alpha-locked (x- = 16 G*^3 alpha given x+ = 1/alpha). A radiation field has
// 1/w = rho/p = 3 EXACTLY; the lattice trace anomaly pushes 1/w above 3. The
// continuum Maxwell stress is degenerate (traceless -> 1/w == 3 identically), so
// the real measurable is the KINETIC pressure from the flux MODE SPECTRUM
//   1/w = 3 sum_k rho_k / sum_k rho_k (k.vg/omega),  rho_k = |wave_vel_k|^2,
// computed offline by FFT (analyze_flux_eos.py, reusing flux_eos_analytical.py).
//
// DISCRIMINATOR (runtime-feasible, no alpha knob needed): sweep the Langevin
// temperature T. If 1/w MOVES with T (Leg A: 3.006@T=0.02 -> 3.038@T=0.05 -> 5.1
// @T=0.2), the EoS is thermal/geometric -> NOT a fixed alpha-locked x- -> the
// "flux pressure = x-" reading is CLOSED. If 1/w pinned at 3.024 for all T -> x-.
//
// GOLDEN-NEUTRAL: read-only campaign (no engine-source change), default-OFF
// toggles (disable_all() then wave_propagation + gauss_projection + langevin).
// genesis OFF -> a clean transverse flux-wave bath, no manifestation. force_cpu()
// + OMP_NUM_THREADS=1 for bit-exact reproducibility.

#include "ftd/render_bridge.h"

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    int L = 32, settle = 500, snaps = 5, snap_gap = 25, seeds = 3;
    double gamma = 0.02;
    std::vector<double> Ts = {0.02, 0.05, 0.10, 0.20, 0.40};
    std::string out = "engine/results/flux_eos/";

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a.rfind("--L=", 0) == 0) L = std::atoi(a.c_str() + 4);
        else if (a.rfind("--settle=", 0) == 0) settle = std::atoi(a.c_str() + 9);
        else if (a.rfind("--snaps=", 0) == 0) snaps = std::atoi(a.c_str() + 8);
        else if (a.rfind("--gap=", 0) == 0) snap_gap = std::atoi(a.c_str() + 6);
        else if (a.rfind("--seeds=", 0) == 0) seeds = std::atoi(a.c_str() + 8);
        else if (a.rfind("--gamma=", 0) == 0) gamma = std::atof(a.c_str() + 8);
        else if (a.rfind("--out=", 0) == 0) out = a.substr(6);
        else if (a.rfind("--T=", 0) == 0) {
            Ts.clear();
            std::string s = a.substr(4);
            size_t p = 0;
            while (p < s.size()) {
                size_t c = s.find(',', p);
                Ts.push_back(std::atof(s.substr(p, c - p).c_str()));
                if (c == std::string::npos) break;
                p = c + 1;
            }
        }
    }

    fs::create_directories(out);
    const fs::path idx = fs::path(out) / ("flux_eos_index_L" + std::to_string(L) + ".csv");
    std::FILE* fi = std::fopen(idx.string().c_str(), "w");
    std::fprintf(fi, "T,seed,snap,L,file\n");

    const int N = L * L * L;
    const std::uint32_t seed_base = 0x0F1U;
    std::printf("flux_eos: L=%d settle=%d snaps=%d gap=%d seeds=%d gamma=%.3f Ts=%zu\n",
                L, settle, snaps, snap_gap, seeds, gamma, Ts.size());
    std::fflush(stdout);

    std::vector<double> buf(3 * N);
    for (double T : Ts) {
        for (int s = 0; s < seeds; ++s) {
            ftd::RenderBridge rb(L);
            rb.force_cpu();
            rb.set_sor_iterations(150);
            rb.toggles.disable_all();
            rb.toggles.wave_propagation = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.langevin = true;
            rb.toggles.langevin_T = T;
            rb.toggles.langevin_gamma = gamma;
            rb.seed_rng(seed_base +
                        (std::uint32_t)((int)(T * 1000.0) * 131 + s) * 2654435761u);
            rb.run(settle);
            for (int sn = 0; sn < snaps; ++sn) {
                rb.run(snap_gap);
                char fname[160];
                std::snprintf(fname, sizeof(fname), "wv_T%.4f_s%d_n%d.bin", T, s, sn);
                const fs::path fp = fs::path(out) / fname;
                std::FILE* fb = std::fopen(fp.string().c_str(), "wb");
                for (int i = 0; i < N; ++i) {
                    const ftd::Vec3 w = rb.wave_vel_at(i);
                    buf[3 * i] = w.x;
                    buf[3 * i + 1] = w.y;
                    buf[3 * i + 2] = w.z;
                }
                std::fwrite(buf.data(), sizeof(double), buf.size(), fb);
                std::fclose(fb);
                std::fprintf(fi, "%.4f,%d,%d,%d,%s\n", T, s, sn, L, fname);
                std::fflush(fi);
            }
            const ftd::EnergyAudit ea = rb.energy_audit();
            std::printf("  T=%.4f s=%d  wave_e=%.6e  field_e=%.6e\n",
                        T, s, ea.wave_energy, ea.field_energy);
            std::fflush(stdout);
        }
    }
    std::fclose(fi);
    std::printf("flux_eos: wrote snapshots to %s (index %s)\n",
                out.c_str(), idx.string().c_str());
    return 0;
}

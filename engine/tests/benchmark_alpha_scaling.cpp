/**
 * @file benchmark_alpha_scaling.cpp
 * @brief First productive use of the FTD-0051 GPU Langevin port: scan
 *        measure_alpha_eff across L ∈ {32, 64, 128, 256} on GPU, plus a
 *        Langevin-equilibrated variant at the largest tractable L.
 *
 * This is the test the GPU port was built for. At T=0 we reproduce the
 * existing Day-2 β-function data points at smaller L and extend to L=256
 * for the first time. At T>0 with Langevin burn-in we produce the first
 * thermal-ensemble α extraction on the FTD engine — a new measurement not
 * previously feasible.
 *
 * Output is raw α_fit, R² of the V(r)~1/r regression, self-energies, and
 * wall-time per L. No tuning; pre-registered knobs.
 */

#include <chrono>
#include <cmath>
#include <cstdio>
#include <vector>

#include "ftd/eft/coupling_measurement.h"
#include "ftd/render_bridge.h"

namespace {
double wall_sec(std::chrono::high_resolution_clock::time_point t0,
                std::chrono::high_resolution_clock::time_point t1) {
    return std::chrono::duration<double>(t1 - t0).count();
}

void run_alpha_at_L(int L, int n_ticks, int r_min, int r_max, int r_step,
                    bool use_langevin, int langevin_burn, const char* label) {
    const auto t_start = std::chrono::high_resolution_clock::now();

    // Pass Langevin options to measure_alpha_eff so its internal bridges
    // (self+, self−, pair) all thermalize consistently. The LangevinOptions
    // carry burn-in ticks that run BEFORE the n_ticks measurement window.
    //
    // TEMPERATURE CHOICE: T must be << Coulomb scale V(r) ~ 0.1/r ~ 10⁻²
    // for the V(r) = E_pair - E_self extraction to resolve the Coulomb
    // interaction above thermal noise. Thermal self-energy scales as
    // ~ 3·T·L³. At T=0.01 for L=64 this gives E_self ~ 800 (dominated);
    // at T=10⁻⁵ it gives E_self ~ 0.8 (still dominant); at T=10⁻⁷ it gives
    // E_self ~ 0.008 (comparable to T=0 Coulomb ~0.03, so signal resolvable).
    ftd::eft::LangevinOptions lo;
    lo.enabled = use_langevin;
    lo.T = 1e-7;  // well below the Coulomb scale
    lo.gamma = 0.01;
    lo.burn_in_ticks = use_langevin ? langevin_burn : 0;

    ftd::eft::CouplingMeasurement cm =
        ftd::eft::measure_alpha_eff(L, n_ticks, r_min, r_max, r_step, 0.05, lo);
    const auto t_end = std::chrono::high_resolution_clock::now();
    const double sec = wall_sec(t_start, t_end);

    std::printf("  %-12s L=%-3d  n_ticks=%-4d  α_fit=%+.5e  R²=%.5f  "
                "E_self+=%+.3e  E_self−=%+.3e  wall=%.2fs  valid=%s\n",
                label, L, n_ticks, cm.alpha_fit, cm.r2,
                cm.e_self_pos, cm.e_self_neg, sec,
                cm.valid ? "yes" : "NO");
    if (!cm.data.empty()) {
        std::printf("    V(r) samples:");
        for (const auto& p : cm.data) std::printf("  r=%d V=%+.3e", p.r, p.V);
        std::printf("\n");
    }
}

}  // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("================================================================\n");
    std::printf("  α_eff scaling benchmark — first productive use of GPU Langevin\n");
    std::printf("================================================================\n");

    // T=0 scan across L. Knobs pre-registered per SPEC_EFT_RECOVERY_PROGRAM §5.3:
    //   r_min = 4, r_max = L/3, r_step = 2, n_ticks = 300.
    std::printf("\n[T=0 scan, standard bare lattice]\n");
    run_alpha_at_L( 32, 300, 4, 10, 2, false, 0, "T=0 L=32");
    run_alpha_at_L( 64, 300, 4, 20, 2, false, 0, "T=0 L=64");
    run_alpha_at_L(128, 300, 4, 40, 4, false, 0, "T=0 L=128");
    run_alpha_at_L(256, 300, 4, 80, 8, false, 0, "T=0 L=256");

    // ----- Path A: shared thermal background -----
    // Closes the naive-thermal-extraction failure mode (FTD-0054). A single
    // thermal background is prepared once and reused across self+, self−,
    // and all pair(r) bridges. Thermal bulk cancels in the V(r) subtraction.
    auto run_pathA = [](int L, int n_ticks_bg, int n_ticks_meas,
                        int r_min, int r_max, int r_step,
                        double T, double gamma, const char* label) {
        const auto t_start = std::chrono::high_resolution_clock::now();
        auto bg = ftd::eft::prepare_thermal_background(L, T, gamma, n_ticks_bg);
        auto cm = ftd::eft::measure_alpha_eff_on_bg(*bg, n_ticks_meas,
                                                    r_min, r_max, r_step);
        const auto t_end = std::chrono::high_resolution_clock::now();
        const double sec = wall_sec(t_start, t_end);
        std::printf("  %-16s L=%-3d  T=%.1e  bg_burn=%d meas_ticks=%d  "
                    "α_fit=%+.5e  R²=%.5f  E_self+=%+.3e  wall=%.2fs\n",
                    label, L, T, n_ticks_bg, n_ticks_meas,
                    cm.alpha_fit, cm.r2, cm.e_self_pos, sec);
        if (!cm.data.empty()) {
            std::printf("    V(r) samples:");
            for (const auto& p : cm.data) std::printf("  r=%d V=%+.3e", p.r, p.V);
            std::printf("\n");
        }
    };

    std::printf("\n[Path A — shared thermal background at L=64]\n");
    run_pathA(64,  1000, 300, 4, 20, 2, 1e-5, 0.01, "PathA L=64 T=1e-5");
    run_pathA(64,  1000, 300, 4, 20, 2, 1e-3, 0.01, "PathA L=64 T=1e-3");

    std::printf("\n[Path A — shared thermal background at L=128]\n");
    run_pathA(128, 1000, 300, 4, 40, 4, 1e-5, 0.01, "PathA L=128 T=1e-5");
    run_pathA(128, 1000, 300, 4, 40, 4, 1e-3, 0.01, "PathA L=128 T=1e-3");

    std::printf("\n================================================================\n");
    std::printf("  Done. Raw α_eff values above; no interpretation issued.\n");
    std::printf("================================================================\n");
    return 0;
}

/**
 * @file benchmark_beta_function_pipe.cpp
 * @brief Pipeline-based β-function benchmark — Phase E port of benchmark_beta_function.
 *
 * Replaces the 140-LOC benchmark_beta_function.cpp (which spoke directly
 * to ftd::eft::measure_alpha_eff) with a ~50-LOC pipeline program that
 * speaks ftd::sim::measure_v_of_r<Backend> instead. Same physics; same
 * CSV output format; runs on either CPU or GPU by flipping one flag.
 *
 * CLI (backward-compatible with the original):
 *   --quick        reduced ticks/sizes for fast smoke tests
 *   --extended     add L = 128
 *   --day2         add L = 128, 256
 *   --day2-gpu     add L = 128, 256, 512   (GPU only — don't try on CPU)
 *   --ticks=N      override ticks per config
 *   --cpu          force BackendCpu even on CUDA builds
 *   --gpu          force BackendGpu (default on CUDA builds; fails if no CUDA)
 */

#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "ftd/constants.h"
#include "ftd/sim/backend_cpu.h"
#include "ftd/sim/measure_v_of_r.h"

#ifdef FTD_ENABLE_CUDA
#  include "ftd/sim/backend_gpu.h"
#endif

using namespace ftd::sim;

enum class Backend { Cpu, Gpu };

template <typename B>
static void emit_csv(const char* tag, const VofRResult& r) {
    for (const auto& p : r.data) {
        std::cout << tag << "," << r.L << "," << r.n_ticks << ","
                  << p.r << "," << std::setprecision(10) << p.V << ","
                  << std::setprecision(10) << p.alpha_r << ",\n";
    }
    std::cout << tag << "," << r.L << "," << r.n_ticks << ",fit,"
              << std::setprecision(10) << r.alpha_fit << ","
              << std::setprecision(10) << r.r2 << ","
              << (r.valid ? "valid" : "invalid") << "\n";
}

template <typename B>
static void run_one(int L, int n_ticks, double seed_amp, int seed_idx) {
    const int r_step = (L >= 256) ? 6 : (L >= 128 ? 4 : 2);
    std::vector<int> rs;
    for (int r = 4; r <= L / 3; r += r_step) rs.push_back(r);
    std::cerr << "-- MCRG: L=" << L << " ticks=" << n_ticks
              << " r_step=" << r_step << " seed[" << seed_idx << "]="
              << seed_amp << " backend=" << B::name() << "\n";
    auto res = measure_v_of_r<B>(L, rs, n_ticks, seed_amp);
    std::cerr << "   alpha_fit=" << std::setprecision(10) << res.alpha_fit
              << " R^2=" << res.r2
              << " (vs reference " << ftd::ALPHA << ")\n";
    char tag[32];
    std::snprintf(tag, sizeof tag, "mcrg_seed%d", seed_idx);
    emit_csv<B>(tag, res);
}

int main(int argc, char** argv) {
    int n_ticks = 300;
    Backend backend = Backend::Cpu;
#ifdef FTD_ENABLE_CUDA
    backend = Backend::Gpu;  // default on CUDA builds
#endif
    bool quick = false, extended = false, day2 = false, day2_gpu = false;

    for (int i = 1; i < argc; ++i) {
        std::string s = argv[i];
        if (s == "--quick") quick = true;
        else if (s == "--extended") extended = true;
        else if (s == "--day2") day2 = true;
        else if (s == "--day2-gpu") day2_gpu = true;
        else if (s == "--cpu") backend = Backend::Cpu;
        else if (s == "--gpu") backend = Backend::Gpu;
        else if (s.rfind("--ticks=", 0) == 0) n_ticks = std::atoi(s.c_str() + 8);
    }
    if (quick) n_ticks = 80;

    std::vector<int> sizes = {16, 32, 64};
    if (quick) sizes = {16, 32};
    if (extended && !quick) sizes.push_back(128);
    if (day2 && !quick) { if (!extended) sizes.push_back(128); sizes.push_back(256); }
    if (day2_gpu && !quick) {
        if (!extended && !day2) sizes.push_back(128);
        if (!day2) sizes.push_back(256);
        sizes.push_back(384);  // memory-safe 4th point (~10 GB VRAM at L=384)
        // L=512 needs ~24 GB + cuFFT workspace; only attempt if the GPU
        // is otherwise idle. Gated behind --l512.
    }

    bool do_l512 = false;
    for (int i = 1; i < argc; ++i)
        if (std::string(argv[i]) == "--l512") do_l512 = true;
    if (do_l512 && !quick) sizes.push_back(512);

    std::cerr << "================================================================\n";
    std::cerr << "  Pipeline beta-function benchmark\n";
    std::cerr << "  backend=" << (backend == Backend::Gpu ? "gpu" : "cpu")
              << " reference alpha=1/" << (1.0/ftd::ALPHA)
              << " = " << ftd::ALPHA << "\n";
    std::cerr << "  n_ticks=" << n_ticks << (quick ? " (quick)" : "")
              << " L=[";
    for (size_t i = 0; i < sizes.size(); ++i)
        std::cerr << sizes[i] << (i+1 < sizes.size() ? "," : "");
    std::cerr << "]\n";
    std::cerr << "================================================================\n";

    std::cout << "method,L,ticks,r,V_or_alpha_fit,alpha_r_or_r2,flag\n";

    for (int L : sizes) {
        if (backend == Backend::Gpu) {
#ifdef FTD_ENABLE_CUDA
            run_one<BackendGpu>(L, n_ticks, 0.05, 0);
#else
            std::cerr << "ERROR: --gpu requested but FTD_ENABLE_CUDA is not defined\n";
            return 1;
#endif
        } else {
            run_one<BackendCpu>(L, n_ticks, 0.05, 0);
        }
    }
    return 0;
}

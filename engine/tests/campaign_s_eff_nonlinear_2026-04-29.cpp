/**
 * @file campaign_s_eff_nonlinear_2026-04-29.cpp
 * @brief FTD-0112: Nonlinear S_eff measurement campaign.
 *
 * Pre-registration: docs/theory/10_eft_program/PROTOCOL_S_EFF_NONLINEAR_CAMPAIGN.md
 * Tag: preregister-s-eff-nonlinear-v1
 *
 * Method (PROTOCOL §1, §6):
 *   1. Pick a scenario from {S1 langevin-pure, S2 genesis-rich,
 *      S3 pair-rich, S4 mixed-balanced} via --scenario flag.
 *   2. Run the configured nonlinear ensemble at fixed (L, T, gamma).
 *   3. Per snapshot k of seed s, take SNAPSHOT-PAIR:
 *        before = render_bridge_to_dual_cell_fields(rb)
 *        rb.run(1)                         // single-tick advance
 *        after  = render_bridge_to_dual_cell_fields(rb)
 *      Compute 10-operator means on (before) for ops O1-O6 (flux/state) and
 *      on SnapshotPair{before, after} for ops O7-O10 (reaction sector).
 *   4. Block fine -> coarse_b2 (and -> coarse_b4 with --b4).
 *      For coarse blocks, reaction operators are evaluated on a blocked
 *      SnapshotPair (block both before and after, then pair them).
 *   5. Aggregate, compute M_ab(b) via Wilsonian normal equations.
 *   6. Bootstrap 100 resamples for per-entry stderr.
 *   7. Output CSV per (L, b, scenario).
 *
 * CLI (PROTOCOL §3, §4):
 *   --scenario={langevin-pure, genesis-rich, pair-rich, mixed-balanced}
 *   --L=N             (default 32)
 *   --N-seeds=N       (default 10)
 *   --N-samples=N     (default 200)
 *   --N-burn=N        (default 200)
 *   --b4              (also compute b=4 mixing matrix)
 *   --smoke           (L=8, N_seeds=1, N_samples=8 for fast validation)
 *   --output-dir=PATH (default engine/results/s_eff_nonlinear_2026-04-29/<scenario>/L<L>_b<b>)
 *
 * Outputs:
 *   meta.json
 *   M_ab.csv               (10x10)
 *   M_ab_stderr.csv        (10x10 bootstrap stderr)
 *   per_snapshot_moments.csv
 *   eigenvalues.csv
 *   run.log
 */

#include "ftd/constants.h"
#include "ftd/eft/dual_cell_blocking.h"
#include "ftd/eft/reaction_operators.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ── 10-operator basis (FTD-0098 6 + FTD-0112 4) ──────────────────────────

constexpr int kNumOps = 10;
constexpr const char* kOpNames[kNumOps] = {
    "JJ", "divJ2", "curlJ2", "JdotDivJ", "J4", "stateSq",
    "reactionDensity", "genesisFlux", "evapFlux", "JdotDeltaS"
};
constexpr double kNaiveDim[kNumOps] = {
    2.0, 4.0, 4.0, 5.0, 4.0, 2.0, 2.0, 4.0, 4.0, 4.0
};

using OpVec = std::array<double, kNumOps>;

// ── Spatial operators (O1-O6, identical to FTD-0098 conventions) ─────────

static inline int wrap(int i, int L) { return ((i % L) + L) % L; }

static inline double cell_J(const ftd::eft::DualCellFields& f, int x, int y, int z, int axis) {
    const int L = f.L;
    if (axis == 0) return 0.5 * (f.phi_x[f.index(x, y, z)] + f.phi_x[f.index(wrap(x - 1, L), y, z)]);
    if (axis == 1) return 0.5 * (f.phi_y[f.index(x, y, z)] + f.phi_y[f.index(x, wrap(y - 1, L), z)]);
    return 0.5 * (f.phi_z[f.index(x, y, z)] + f.phi_z[f.index(x, y, wrap(z - 1, L))]);
}

static inline double op_J2(const ftd::eft::DualCellFields& f, int x, int y, int z) {
    const double Jx = cell_J(f, x, y, z, 0);
    const double Jy = cell_J(f, x, y, z, 1);
    const double Jz = cell_J(f, x, y, z, 2);
    return Jx * Jx + Jy * Jy + Jz * Jz;
}

static inline double op_divJ2(const ftd::eft::DualCellFields& f, int x, int y, int z) {
    const double d = ftd::eft::div_face_at(f, x, y, z);
    return d * d;
}

static inline double op_curlJ2(const ftd::eft::DualCellFields& f, int x, int y, int z) {
    const int L = f.L;
    const double dJz_dy = 0.5 * (cell_J(f, x, wrap(y + 1, L), z, 2) - cell_J(f, x, wrap(y - 1, L), z, 2));
    const double dJy_dz = 0.5 * (cell_J(f, x, y, wrap(z + 1, L), 1) - cell_J(f, x, y, wrap(z - 1, L), 1));
    const double cx = dJz_dy - dJy_dz;
    const double dJx_dz = 0.5 * (cell_J(f, x, y, wrap(z + 1, L), 0) - cell_J(f, x, y, wrap(z - 1, L), 0));
    const double dJz_dx = 0.5 * (cell_J(f, wrap(x + 1, L), y, z, 2) - cell_J(f, wrap(x - 1, L), y, z, 2));
    const double cy = dJx_dz - dJz_dx;
    const double dJy_dx = 0.5 * (cell_J(f, wrap(x + 1, L), y, z, 1) - cell_J(f, wrap(x - 1, L), y, z, 1));
    const double dJx_dy = 0.5 * (cell_J(f, x, wrap(y + 1, L), z, 0) - cell_J(f, x, wrap(y - 1, L), z, 0));
    const double cz = dJy_dx - dJx_dy;
    return cx * cx + cy * cy + cz * cz;
}

static inline double op_JdotDivJ(const ftd::eft::DualCellFields& f, int x, int y, int z) {
    const int L = f.L;
    const double Jx = cell_J(f, x, y, z, 0);
    const double Jy = cell_J(f, x, y, z, 1);
    const double Jz = cell_J(f, x, y, z, 2);
    const double gx = 0.5 * (ftd::eft::div_face_at(f, wrap(x + 1, L), y, z) -
                             ftd::eft::div_face_at(f, wrap(x - 1, L), y, z));
    const double gy = 0.5 * (ftd::eft::div_face_at(f, x, wrap(y + 1, L), z) -
                             ftd::eft::div_face_at(f, x, wrap(y - 1, L), z));
    const double gz = 0.5 * (ftd::eft::div_face_at(f, x, y, wrap(z + 1, L)) -
                             ftd::eft::div_face_at(f, x, y, wrap(z - 1, L)));
    return Jx * gx + Jy * gy + Jz * gz;
}

static inline double op_J4(const ftd::eft::DualCellFields& f, int x, int y, int z) {
    const double j2 = op_J2(f, x, y, z);
    return j2 * j2;
}

static inline double op_stateSq(const ftd::eft::DualCellFields& f, int x, int y, int z) {
    const double s = static_cast<double>(f.rho_cell[f.index(x, y, z)]);
    return s * s;
}

// ── 10-op evaluator on a SnapshotPair ────────────────────────────────────

static OpVec mean_operators_pair(const ftd::eft::SnapshotPair& p) {
    OpVec acc{};
    const int L = p.L();
    for (int z = 0; z < L; ++z) {
        for (int y = 0; y < L; ++y) {
            for (int x = 0; x < L; ++x) {
                // Spatial ops on "before" snapshot
                acc[0] += op_J2(p.before, x, y, z);
                acc[1] += op_divJ2(p.before, x, y, z);
                acc[2] += op_curlJ2(p.before, x, y, z);
                acc[3] += op_JdotDivJ(p.before, x, y, z);
                acc[4] += op_J4(p.before, x, y, z);
                acc[5] += op_stateSq(p.before, x, y, z);
                // Reaction ops on the pair
                acc[6] += ftd::eft::op_reactionDensity(p, x, y, z);
                acc[7] += ftd::eft::op_genesisFlux(p, x, y, z);
                acc[8] += ftd::eft::op_evapFlux(p, x, y, z);
                acc[9] += ftd::eft::op_JdotDeltaS(p, x, y, z);
            }
        }
    }
    const double inv_N = 1.0 / static_cast<double>(L * L * L);
    for (int a = 0; a < kNumOps; ++a) acc[a] *= inv_N;
    return acc;
}

// ── Linear algebra (Gauss-Jordan inversion + matmul) ─────────────────────

using Mat = std::array<std::array<double, kNumOps>, kNumOps>;

static Mat zero_mat() {
    Mat M{};
    for (auto& row : M) row.fill(0.0);
    return M;
}

static bool invert_mat(const Mat& A, Mat& Ainv, double& cond_estimate) {
    constexpr int N = kNumOps;
    std::array<std::array<double, 2 * N>, N> aug{};
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) aug[i][j] = A[i][j];
        for (int j = 0; j < N; ++j) aug[i][N + j] = (i == j) ? 1.0 : 0.0;
    }
    double max_pivot = 0.0;
    double min_pivot = 1e300;
    for (int col = 0; col < N; ++col) {
        int pivot_row = col;
        double pivot_abs = std::abs(aug[col][col]);
        for (int r = col + 1; r < N; ++r) {
            if (std::abs(aug[r][col]) > pivot_abs) {
                pivot_abs = std::abs(aug[r][col]);
                pivot_row = r;
            }
        }
        if (pivot_abs < 1e-30) {
            cond_estimate = 1e300;
            return false;
        }
        if (pivot_row != col) std::swap(aug[col], aug[pivot_row]);
        const double pivot = aug[col][col];
        max_pivot = std::max(max_pivot, std::abs(pivot));
        min_pivot = std::min(min_pivot, std::abs(pivot));
        for (int j = 0; j < 2 * N; ++j) aug[col][j] /= pivot;
        for (int r = 0; r < N; ++r) {
            if (r == col) continue;
            const double factor = aug[r][col];
            if (factor == 0.0) continue;
            for (int j = 0; j < 2 * N; ++j) aug[r][j] -= factor * aug[col][j];
        }
    }
    cond_estimate = (min_pivot > 0.0) ? (max_pivot / min_pivot) : 1e300;
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            Ainv[i][j] = aug[i][N + j];
    return true;
}

static Mat matmul(const Mat& A, const Mat& B) {
    Mat C = zero_mat();
    for (int i = 0; i < kNumOps; ++i)
        for (int j = 0; j < kNumOps; ++j)
            for (int k = 0; k < kNumOps; ++k)
                C[i][j] += A[i][k] * B[k][j];
    return C;
}

// ── Covariances + mixing matrix ──────────────────────────────────────────

struct CovResult {
    Mat Sigma{};
    Mat S{};
    OpVec mean_fine{};
    OpVec mean_coarse{};
    int N = 0;
    bool valid = false;
};

static CovResult compute_covariances(const std::vector<OpVec>& fine_samples,
                                     const std::vector<OpVec>& coarse_samples) {
    CovResult r;
    if (fine_samples.size() != coarse_samples.size()) return r;
    r.N = static_cast<int>(fine_samples.size());
    if (r.N < 2) return r;

    r.mean_fine.fill(0.0);
    r.mean_coarse.fill(0.0);
    for (int i = 0; i < r.N; ++i) {
        for (int a = 0; a < kNumOps; ++a) {
            r.mean_fine[a] += fine_samples[i][a];
            r.mean_coarse[a] += coarse_samples[i][a];
        }
    }
    const double inv_N = 1.0 / static_cast<double>(r.N);
    for (int a = 0; a < kNumOps; ++a) {
        r.mean_fine[a] *= inv_N;
        r.mean_coarse[a] *= inv_N;
    }
    r.Sigma = zero_mat();
    r.S = zero_mat();
    for (int i = 0; i < r.N; ++i) {
        OpVec dF, dC;
        for (int a = 0; a < kNumOps; ++a) {
            dF[a] = fine_samples[i][a] - r.mean_fine[a];
            dC[a] = coarse_samples[i][a] - r.mean_coarse[a];
        }
        for (int a = 0; a < kNumOps; ++a)
            for (int b = 0; b < kNumOps; ++b) {
                r.Sigma[a][b] += dC[a] * dF[b];
                r.S[a][b] += dF[a] * dF[b];
            }
    }
    for (int a = 0; a < kNumOps; ++a)
        for (int b = 0; b < kNumOps; ++b) {
            r.Sigma[a][b] *= inv_N;
            r.S[a][b] *= inv_N;
        }
    r.valid = true;
    return r;
}

// ── Scenario configuration ───────────────────────────────────────────────

enum class Scenario {
    LangevinPure,
    GenesisRich,
    PairRich,
    MixedBalanced,
};

static const char* scenario_name(Scenario s) {
    switch (s) {
        case Scenario::LangevinPure:  return "langevin-pure";
        case Scenario::GenesisRich:   return "genesis-rich";
        case Scenario::PairRich:      return "pair-rich";
        case Scenario::MixedBalanced: return "mixed-balanced";
    }
    return "unknown";
}

static bool parse_scenario(const std::string& s, Scenario& out) {
    if (s == "langevin-pure")  { out = Scenario::LangevinPure;  return true; }
    if (s == "genesis-rich")   { out = Scenario::GenesisRich;   return true; }
    if (s == "pair-rich")      { out = Scenario::PairRich;      return true; }
    if (s == "mixed-balanced") { out = Scenario::MixedBalanced; return true; }
    return false;
}

static void configure_scenario(ftd::RenderBridge& rb, Scenario sc, int L) {
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.langevin = true;
    rb.toggles.langevin_T = 0.005;
    rb.toggles.langevin_gamma = 0.02;
    rb.toggles.dual_substrate = false;

    switch (sc) {
        case Scenario::LangevinPure:
            // S1: pure Langevin + wave + gauss. No genesis. State stays 0.
            // No flux injection — pure thermal noise.
            break;

        case Scenario::GenesisRich:
            // S2: matches FTD-0098 reference ensemble.
            rb.toggles.genesis = true;
            rb.inject_flux(L / 2, L / 2, L / 2,
                           {1.0 * ftd::K_GENESIS, 0, 0});
            break;

        case Scenario::PairRich:
            // S3: high reaction-rate scenario.
            // Note: engine architecture: `genesis` toggle includes both
            // manifestation AND evaporation (render_bridge.cpp Rule 2);
            // `movement` toggle includes annihilation as part of collision
            // resolution (render_bridge.cpp Rule 5). The protocol's
            // notional "+annihilation" is realized via movement.
            rb.toggles.genesis = true;
            rb.toggles.pair_production = true;
            rb.toggles.movement = true;
            rb.toggles.langevin_T = 0.010;
            // 5 high-|J| seeds at scattered points
            for (int i = 0; i < 5; ++i) {
                const int x = (L / 6) * (1 + i % 3);
                const int y = (L / 6) * (1 + (i + 1) % 3);
                const int z = (L / 6) * (1 + (i + 2) % 3);
                rb.inject_flux(x % L, y % L, z % L,
                               {2.0 * ftd::K_GENESIS, 0, 0});
            }
            break;

        case Scenario::MixedBalanced:
            // S4: full balanced reaction sector.
            // genesis (manifestation + evaporation), pair_production,
            // movement (annihilation channel), weak_transmutation.
            rb.toggles.genesis = true;
            rb.toggles.pair_production = true;
            rb.toggles.movement = true;
            rb.toggles.weak_transmutation = true;
            // Uniform low-|J| Gaussian initial — let Langevin drive it.
            rb.toggles.langevin_T = 0.005;
            break;
    }
}

// ── CLI ──────────────────────────────────────────────────────────────────

struct Args {
    Scenario scenario = Scenario::GenesisRich;
    int L = 32;
    int N_seeds = 10;
    int N_samples = 200;
    int N_burn = 200;
    int sample_stride = 5;
    bool include_b4 = false;
    bool smoke = false;
    std::string output_dir;
    // Tuning knobs for nonlinear-regime exploration (post-smoke direction A)
    double T_override = -1.0;        // < 0 => use scenario default
    int inject_period = 0;           // 0 => no periodic re-injection
    double inject_amp_mult = 1.0;    // multiplier on K_GENESIS for re-injection
};

static bool parse_args(int argc, char** argv, Args& a, std::string& err) {
    for (int i = 1; i < argc; ++i) {
        std::string s = argv[i];
        auto eq_split = [&](const char* prefix, std::string& val) -> bool {
            const size_t plen = std::strlen(prefix);
            if (s.size() <= plen || s.substr(0, plen) != prefix) return false;
            val = s.substr(plen);
            return true;
        };
        std::string val;
        if (eq_split("--scenario=", val)) {
            if (!parse_scenario(val, a.scenario)) {
                err = "unknown scenario: " + val;
                return false;
            }
        } else if (eq_split("--L=", val)) {
            a.L = std::atoi(val.c_str());
        } else if (eq_split("--N-seeds=", val)) {
            a.N_seeds = std::atoi(val.c_str());
        } else if (eq_split("--N-samples=", val)) {
            a.N_samples = std::atoi(val.c_str());
        } else if (eq_split("--N-burn=", val)) {
            a.N_burn = std::atoi(val.c_str());
        } else if (eq_split("--output-dir=", val)) {
            a.output_dir = val;
        } else if (s == "--b4") {
            a.include_b4 = true;
        } else if (eq_split("--T-langevin=", val)) {
            a.T_override = std::atof(val.c_str());
        } else if (eq_split("--inject-period=", val)) {
            a.inject_period = std::atoi(val.c_str());
        } else if (eq_split("--inject-amp=", val)) {
            a.inject_amp_mult = std::atof(val.c_str());
        } else if (s == "--smoke") {
            a.smoke = true;
            a.L = 8;
            a.N_seeds = 1;
            a.N_samples = 8;
            a.N_burn = 50;
        } else if (s == "--help" || s == "-h") {
            std::cout << "campaign_s_eff_nonlinear --scenario={langevin-pure,genesis-rich,pair-rich,mixed-balanced}\n"
                      << "  --L=N --N-seeds=N --N-samples=N --N-burn=N --b4 --smoke --output-dir=PATH\n";
            std::exit(0);
        } else {
            err = "unknown arg: " + s;
            return false;
        }
    }
    if (a.output_dir.empty()) {
        std::ostringstream oss;
        oss << "engine/results/s_eff_nonlinear_2026-04-29/"
            << scenario_name(a.scenario) << "/L" << a.L
            << "_b" << (a.include_b4 ? "4" : "2");
        a.output_dir = oss.str();
    }
    return true;
}

// ── Main ─────────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    Args args;
    std::string err;
    if (!parse_args(argc, argv, args, err)) {
        std::cerr << "ERR: " << err << "\n";
        return 2;
    }

    fs::create_directories(args.output_dir);
    std::ofstream log_out(fs::path(args.output_dir) / "run.log");
    auto log = [&](const std::string& msg) {
        std::cout << msg;
        log_out << msg;
    };

    std::ostringstream banner;
    banner << "============================================================\n"
           << "  CAMPAIGN: FTD-0112 S_eff Nonlinear Measurement\n"
           << "  Pre-reg: PROTOCOL_S_EFF_NONLINEAR_CAMPAIGN.md\n"
           << "  Tag:     preregister-s-eff-nonlinear-v1\n"
           << "============================================================\n"
           << "  Scenario:    " << scenario_name(args.scenario) << "\n"
           << "  L:           " << args.L << "\n"
           << "  N_seeds:     " << args.N_seeds << "\n"
           << "  N_samples:   " << args.N_samples << "\n"
           << "  N_burn:      " << args.N_burn << "\n"
           << "  include_b4:  " << (args.include_b4 ? "yes" : "no") << "\n"
           << "  T_override:  " << (args.T_override >= 0.0 ? std::to_string(args.T_override) : std::string("(default)")) << "\n"
           << "  inject_period: " << args.inject_period << "\n"
           << "  inject_amp:  " << args.inject_amp_mult << "\n"
           << "  output_dir:  " << args.output_dir << "\n"
           << "  smoke:       " << (args.smoke ? "yes" : "no") << "\n";
    log(banner.str());

    constexpr std::uint32_t BASE_SEED = 0xF11212E5u;

    std::vector<OpVec> fine_samples;
    std::vector<OpVec> coarse2_samples;
    std::vector<OpVec> coarse4_samples;
    fine_samples.reserve(args.N_seeds * args.N_samples);
    coarse2_samples.reserve(args.N_seeds * args.N_samples);
    if (args.include_b4) coarse4_samples.reserve(args.N_seeds * args.N_samples);

    int snapshots_dropped = 0;
    int q_violations = 0;

    for (int s = 0; s < args.N_seeds; ++s) {
        const std::uint32_t seed = BASE_SEED + static_cast<std::uint32_t>(s) * 0x100u;
        std::ostringstream sl;
        sl << "  seed " << s << "/" << args.N_seeds << " (rng=0x"
           << std::hex << seed << std::dec << ")\n";
        log(sl.str());

        ftd::RenderBridge rb(args.L);
        configure_scenario(rb, args.scenario, args.L);
        if (args.T_override >= 0.0) {
            rb.toggles.langevin_T = args.T_override;
        }
        rb.seed_rng(seed);
        rb.run(args.N_burn);

        // Track the scenario's primary injection point for periodic re-injection.
        // For multi-source scenarios (S3 pair-rich), re-inject at all 5 points.
        auto reinject = [&]() {
            if (args.inject_period <= 0) return;
            const double amp = args.inject_amp_mult * ftd::K_GENESIS;
            switch (args.scenario) {
                case Scenario::GenesisRich:
                    rb.inject_flux(args.L / 2, args.L / 2, args.L / 2, {amp, 0, 0});
                    break;
                case Scenario::PairRich:
                    for (int i = 0; i < 5; ++i) {
                        const int x = (args.L / 6) * (1 + i % 3);
                        const int y = (args.L / 6) * (1 + (i + 1) % 3);
                        const int z = (args.L / 6) * (1 + (i + 2) % 3);
                        rb.inject_flux(x % args.L, y % args.L, z % args.L,
                                       {amp, 0, 0});
                    }
                    break;
                case Scenario::MixedBalanced:
                    rb.inject_flux(args.L / 2, args.L / 2, args.L / 2, {amp, 0, 0});
                    break;
                case Scenario::LangevinPure:
                    // No injection in pure-Langevin scenario.
                    break;
            }
        };

        int ticks_since_inject = 0;
        for (int k = 0; k < args.N_samples; ++k) {
            rb.run(args.sample_stride);
            ticks_since_inject += args.sample_stride;
            if (args.inject_period > 0 && ticks_since_inject >= args.inject_period) {
                reinject();
                ticks_since_inject = 0;
            }
            // Snapshot pair: before, advance 1 tick, after
            const auto before = ftd::eft::render_bridge_to_dual_cell_fields(rb);
            rb.run(1);
            const auto after = ftd::eft::render_bridge_to_dual_cell_fields(rb);

            const auto coarse2_before = ftd::eft::block_dual_cell_b2(before);
            const auto coarse2_after  = ftd::eft::block_dual_cell_b2(after);

            ftd::eft::DualCellFields coarse4_before{}, coarse4_after{};
            if (args.include_b4) {
                coarse4_before = ftd::eft::block_dual_cell_b2(coarse2_before);
                coarse4_after  = ftd::eft::block_dual_cell_b2(coarse2_after);
            }

            // Q conservation across blocking (snapshot-by-snapshot)
            const int q_fine = ftd::eft::total_source(before);
            const int q_c2   = ftd::eft::total_source(coarse2_before);
            const int q_c4   = args.include_b4 ? ftd::eft::total_source(coarse4_before) : q_fine;
            if (q_fine != q_c2 || q_fine != q_c4) {
                ++q_violations;
                ++snapshots_dropped;
                continue;
            }

            // Compute means on the snapshot pair (10 ops total)
            ftd::eft::SnapshotPair pair_fine{before, after};
            ftd::eft::SnapshotPair pair_c2{coarse2_before, coarse2_after};
            fine_samples.push_back(mean_operators_pair(pair_fine));
            coarse2_samples.push_back(mean_operators_pair(pair_c2));
            if (args.include_b4) {
                ftd::eft::SnapshotPair pair_c4{coarse4_before, coarse4_after};
                coarse4_samples.push_back(mean_operators_pair(pair_c4));
            }
        }
    }

    const int N_total = static_cast<int>(fine_samples.size());
    std::ostringstream summary;
    summary << "\n  collected " << N_total << " snapshot pairs ("
            << snapshots_dropped << " dropped, "
            << q_violations << " Q-violations)\n";
    log(summary.str());

    if (N_total < 4) {
        log("  ERR: insufficient samples (need >= 4)\n");
        return 3;
    }

    // Always write per-snapshot moments first so they're available for
    // diagnosis even if matrix inversion fails on near-zero-variance ops.
    {
        std::ofstream out(fs::path(args.output_dir) / "per_snapshot_moments.csv");
        out << "snapshot,op,fine_value,coarse2_value";
        if (args.include_b4) out << ",coarse4_value";
        out << "\n";
        for (int i = 0; i < N_total; ++i) {
            for (int a = 0; a < kNumOps; ++a) {
                out << i << "," << kOpNames[a] << "," << fine_samples[i][a]
                    << "," << coarse2_samples[i][a];
                if (args.include_b4 && i < (int)coarse4_samples.size())
                    out << "," << coarse4_samples[i][a];
                out << "\n";
            }
        }
    }

    // Per-op variance diagnostic (helps identify near-zero ops that cause
    // S-matrix singularity).
    OpVec op_var{};
    for (int a = 0; a < kNumOps; ++a) {
        double mean = 0.0;
        for (int i = 0; i < N_total; ++i) mean += fine_samples[i][a];
        mean /= N_total;
        double v = 0.0;
        for (int i = 0; i < N_total; ++i) {
            const double d = fine_samples[i][a] - mean;
            v += d * d;
        }
        op_var[a] = v / N_total;
    }
    {
        std::ostringstream os;
        os << "  per-op fine variance:\n";
        for (int a = 0; a < kNumOps; ++a) {
            os << "    " << kOpNames[a] << " = " << op_var[a] << "\n";
        }
        log(os.str());
    }

    // ── Compute M_ab(b=2) — with graceful degradation per PROTOCOL §5 Gate A ──
    //
    // Drop ops with variance below threshold from the active subspace,
    // invert the reduced S, and report reduced-rank M_ab.  Zero-variance
    // operators are reported in meta.json as "structurally inactive in
    // this ensemble"; the M_ab matrix is reported on the active subspace
    // only with NaN entries for dropped ops.
    constexpr double kVarianceFloor = 1e-30;
    std::vector<int> active_ops;
    std::vector<int> dropped_ops;
    for (int a = 0; a < kNumOps; ++a) {
        if (op_var[a] >= kVarianceFloor) active_ops.push_back(a);
        else dropped_ops.push_back(a);
    }
    {
        std::ostringstream os;
        os << "  active ops (" << active_ops.size() << "/10): ";
        for (int a : active_ops) os << kOpNames[a] << " ";
        os << "\n";
        if (!dropped_ops.empty()) {
            os << "  dropped ops (variance < " << kVarianceFloor << "): ";
            for (int a : dropped_ops) os << kOpNames[a] << " ";
            os << "\n";
        }
        log(os.str());
    }

    auto cov2 = compute_covariances(fine_samples, coarse2_samples);
    if (!cov2.valid) {
        log("  ERR: b=2 covariances invalid\n");
        return 4;
    }

    Mat M_b2 = zero_mat();
    double cond_S = 1e300;
    bool inversion_ok = false;

    if (active_ops.size() == kNumOps) {
        // Full 10x10 inversion path.
        Mat S_inv;
        if (invert_mat(cov2.S, S_inv, cond_S)) {
            M_b2 = matmul(cov2.Sigma, S_inv);
            inversion_ok = true;
        }
    }
    if (!inversion_ok) {
        // Reduced-rank inversion: extract active subspace, invert, embed.
        const int Nact = static_cast<int>(active_ops.size());
        if (Nact >= 2) {
            // Build reduced fine/coarse samples at active subspace.
            std::vector<std::vector<double>> S_red(Nact, std::vector<double>(Nact, 0.0));
            std::vector<std::vector<double>> Sigma_red(Nact, std::vector<double>(Nact, 0.0));
            for (int i = 0; i < Nact; ++i)
                for (int j = 0; j < Nact; ++j) {
                    S_red[i][j] = cov2.S[active_ops[i]][active_ops[j]];
                    Sigma_red[i][j] = cov2.Sigma[active_ops[i]][active_ops[j]];
                }
            // Inline Gauss-Jordan on dynamic-size matrix.
            std::vector<std::vector<double>> aug(Nact, std::vector<double>(2 * Nact, 0.0));
            for (int i = 0; i < Nact; ++i) {
                for (int j = 0; j < Nact; ++j) aug[i][j] = S_red[i][j];
                for (int j = 0; j < Nact; ++j) aug[i][Nact + j] = (i == j) ? 1.0 : 0.0;
            }
            double max_pivot = 0.0;
            double min_pivot = 1e300;
            bool ok = true;
            for (int col = 0; col < Nact; ++col) {
                int pivot_row = col;
                double pivot_abs = std::abs(aug[col][col]);
                for (int r = col + 1; r < Nact; ++r) {
                    if (std::abs(aug[r][col]) > pivot_abs) {
                        pivot_abs = std::abs(aug[r][col]);
                        pivot_row = r;
                    }
                }
                if (pivot_abs < 1e-30) { ok = false; break; }
                if (pivot_row != col) std::swap(aug[col], aug[pivot_row]);
                const double pivot = aug[col][col];
                max_pivot = std::max(max_pivot, std::abs(pivot));
                min_pivot = std::min(min_pivot, std::abs(pivot));
                for (int j = 0; j < 2 * Nact; ++j) aug[col][j] /= pivot;
                for (int r = 0; r < Nact; ++r) {
                    if (r == col) continue;
                    const double factor = aug[r][col];
                    if (factor == 0.0) continue;
                    for (int j = 0; j < 2 * Nact; ++j) aug[r][j] -= factor * aug[col][j];
                }
            }
            if (ok) {
                cond_S = (min_pivot > 0.0) ? (max_pivot / min_pivot) : 1e300;
                // M_red = Sigma_red * S_red^{-1}
                std::vector<std::vector<double>> M_red(Nact, std::vector<double>(Nact, 0.0));
                for (int i = 0; i < Nact; ++i)
                    for (int j = 0; j < Nact; ++j)
                        for (int k = 0; k < Nact; ++k)
                            M_red[i][j] += Sigma_red[i][k] * aug[k][Nact + j];
                // Embed into full 10x10 with NaN for dropped rows/cols.
                const double NaN = std::nan("");
                for (int a = 0; a < kNumOps; ++a)
                    for (int b = 0; b < kNumOps; ++b)
                        M_b2[a][b] = NaN;
                for (int i = 0; i < Nact; ++i)
                    for (int j = 0; j < Nact; ++j)
                        M_b2[active_ops[i]][active_ops[j]] = M_red[i][j];
                inversion_ok = true;
                std::ostringstream os;
                os << "  reduced-rank inversion succeeded on " << Nact << "-op subspace\n";
                log(os.str());
            }
        }
        if (!inversion_ok) {
            log("  ERR: even reduced-subspace inversion failed (linear dependence "
                "among active ops?)\n");
            return 5;
        }
    }

    std::ostringstream cs;
    cs << "  cond(S) = " << cond_S << "\n";
    log(cs.str());

    // ── Bootstrap stderr ─────────────────────────────────────────────────
    constexpr int N_BOOTSTRAP = 100;
    std::array<std::array<double, kNumOps>, kNumOps> M_sum{}, M_sum_sq{};
    for (auto& r : M_sum) r.fill(0.0);
    for (auto& r : M_sum_sq) r.fill(0.0);
    int boot_succeeded = 0;

    std::mt19937_64 rng(BASE_SEED ^ 0xB007);
    std::uniform_int_distribution<int> idx_dist(0, N_total - 1);
    const int Nact = static_cast<int>(active_ops.size());
    for (int boot = 0; boot < N_BOOTSTRAP; ++boot) {
        std::vector<OpVec> resampled_fine, resampled_coarse;
        resampled_fine.reserve(N_total);
        resampled_coarse.reserve(N_total);
        for (int i = 0; i < N_total; ++i) {
            const int idx = idx_dist(rng);
            resampled_fine.push_back(fine_samples[idx]);
            resampled_coarse.push_back(coarse2_samples[idx]);
        }
        auto cov_b = compute_covariances(resampled_fine, resampled_coarse);
        if (!cov_b.valid) continue;

        Mat M_b = zero_mat();
        bool ok_b = false;

        if (Nact == kNumOps) {
            Mat S_inv_b;
            double cond_b;
            if (invert_mat(cov_b.S, S_inv_b, cond_b)) {
                M_b = matmul(cov_b.Sigma, S_inv_b);
                ok_b = true;
            }
        } else {
            // Reduced-rank inversion on the locked active subspace.
            std::vector<std::vector<double>> aug(Nact, std::vector<double>(2 * Nact, 0.0));
            for (int i = 0; i < Nact; ++i) {
                for (int j = 0; j < Nact; ++j)
                    aug[i][j] = cov_b.S[active_ops[i]][active_ops[j]];
                for (int j = 0; j < Nact; ++j)
                    aug[i][Nact + j] = (i == j) ? 1.0 : 0.0;
            }
            bool reg_ok = true;
            for (int col = 0; col < Nact; ++col) {
                int pivot_row = col;
                double pivot_abs = std::abs(aug[col][col]);
                for (int r = col + 1; r < Nact; ++r) {
                    if (std::abs(aug[r][col]) > pivot_abs) {
                        pivot_abs = std::abs(aug[r][col]);
                        pivot_row = r;
                    }
                }
                if (pivot_abs < 1e-30) { reg_ok = false; break; }
                if (pivot_row != col) std::swap(aug[col], aug[pivot_row]);
                const double pivot = aug[col][col];
                for (int j = 0; j < 2 * Nact; ++j) aug[col][j] /= pivot;
                for (int r = 0; r < Nact; ++r) {
                    if (r == col) continue;
                    const double factor = aug[r][col];
                    if (factor == 0.0) continue;
                    for (int j = 0; j < 2 * Nact; ++j) aug[r][j] -= factor * aug[col][j];
                }
            }
            if (reg_ok) {
                std::vector<std::vector<double>> M_red(Nact, std::vector<double>(Nact, 0.0));
                for (int i = 0; i < Nact; ++i)
                    for (int j = 0; j < Nact; ++j)
                        for (int k = 0; k < Nact; ++k)
                            M_red[i][j] += cov_b.Sigma[active_ops[i]][active_ops[k]] *
                                           aug[k][Nact + j];
                const double NaN = std::nan("");
                for (int a = 0; a < kNumOps; ++a)
                    for (int b = 0; b < kNumOps; ++b)
                        M_b[a][b] = NaN;
                for (int i = 0; i < Nact; ++i)
                    for (int j = 0; j < Nact; ++j)
                        M_b[active_ops[i]][active_ops[j]] = M_red[i][j];
                ok_b = true;
            }
        }
        if (!ok_b) continue;

        for (int a = 0; a < kNumOps; ++a)
            for (int c = 0; c < kNumOps; ++c) {
                if (std::isnan(M_b[a][c])) continue;
                M_sum[a][c] += M_b[a][c];
                M_sum_sq[a][c] += M_b[a][c] * M_b[a][c];
            }
        ++boot_succeeded;
    }
    Mat M_stderr = zero_mat();
    if (boot_succeeded >= 2) {
        const double inv_n = 1.0 / boot_succeeded;
        for (int a = 0; a < kNumOps; ++a)
            for (int c = 0; c < kNumOps; ++c) {
                const double m = M_sum[a][c] * inv_n;
                const double v = M_sum_sq[a][c] * inv_n - m * m;
                M_stderr[a][c] = (v > 0.0) ? std::sqrt(v) : 0.0;
            }
    }

    std::ostringstream bs;
    bs << "  bootstrap: " << boot_succeeded << "/" << N_BOOTSTRAP << " succeeded\n";
    log(bs.str());

    // ── Output CSVs ──────────────────────────────────────────────────────
    auto write_matrix = [&](const std::string& fname, const Mat& M) {
        std::ofstream out(fs::path(args.output_dir) / fname);
        out << "op_a,op_b,value,naive_dim_a,naive_dim_b\n";
        for (int a = 0; a < kNumOps; ++a)
            for (int b = 0; b < kNumOps; ++b)
                out << kOpNames[a] << "," << kOpNames[b] << ","
                    << M[a][b] << "," << kNaiveDim[a] << "," << kNaiveDim[b] << "\n";
    };
    write_matrix("M_ab.csv", M_b2);
    write_matrix("M_ab_stderr.csv", M_stderr);
    // (per_snapshot_moments.csv was written earlier, before matrix inversion)

    // Diagonal eigenvalue diagnostic
    {
        std::ofstream out(fs::path(args.output_dir) / "eigenvalues.csv");
        out << "op,M_aa,naive_dim,delta_a\n";
        for (int a = 0; a < kNumOps; ++a) {
            const double maa = M_b2[a][a];
            const double delta_a = (maa > 0.0)
                ? (3.0 - std::log(maa) / std::log(2.0))  // D=3 lattice
                : 0.0;
            out << kOpNames[a] << "," << maa << "," << kNaiveDim[a] << "," << delta_a << "\n";
        }
    }

    // Optional: M(b=4) and RG semigroup test M(b=4) ≈ M(b=2)^2
    if (args.include_b4 && !coarse4_samples.empty()) {
        auto cov4 = compute_covariances(fine_samples, coarse4_samples);
        if (cov4.valid) {
            Mat S_inv4;
            double cond_S4;
            if (invert_mat(cov4.S, S_inv4, cond_S4)) {
                Mat M_b4 = matmul(cov4.Sigma, S_inv4);
                write_matrix("M_ab_b4.csv", M_b4);

                Mat M_b2_sq = matmul(M_b2, M_b2);
                Mat diff = zero_mat();
                double frob_diff = 0.0, frob_b4 = 0.0;
                for (int a = 0; a < kNumOps; ++a)
                    for (int b = 0; b < kNumOps; ++b) {
                        diff[a][b] = M_b4[a][b] - M_b2_sq[a][b];
                        frob_diff += diff[a][b] * diff[a][b];
                        frob_b4 += M_b4[a][b] * M_b4[a][b];
                    }
                frob_diff = std::sqrt(frob_diff);
                frob_b4 = std::sqrt(frob_b4);
                const double rg_ratio = (frob_b4 > 0.0) ? (frob_diff / frob_b4) : 0.0;
                std::ostringstream rg;
                rg << "  RG semigroup test: ||M(b=4) - M(b=2)^2|| / ||M(b=4)|| = "
                   << rg_ratio << "\n";
                log(rg.str());
                std::ofstream(fs::path(args.output_dir) / "rg_semigroup.txt")
                    << "rg_ratio " << rg_ratio << "\n"
                    << "frob_diff " << frob_diff << "\n"
                    << "frob_b4 " << frob_b4 << "\n";
            }
        }
    }

    // meta.json
    {
        std::ofstream out(fs::path(args.output_dir) / "meta.json");
        out << "{\n"
            << "  \"campaign\": \"FTD-0112 S_eff nonlinear\",\n"
            << "  \"protocol_tag\": \"preregister-s-eff-nonlinear-v1\",\n"
            << "  \"scenario\": \"" << scenario_name(args.scenario) << "\",\n"
            << "  \"L\": " << args.L << ",\n"
            << "  \"N_seeds\": " << args.N_seeds << ",\n"
            << "  \"N_samples\": " << args.N_samples << ",\n"
            << "  \"N_burn\": " << args.N_burn << ",\n"
            << "  \"sample_stride\": " << args.sample_stride << ",\n"
            << "  \"snapshots_collected\": " << N_total << ",\n"
            << "  \"snapshots_dropped\": " << snapshots_dropped << ",\n"
            << "  \"q_violations\": " << q_violations << ",\n"
            << "  \"cond_S_b2\": " << cond_S << ",\n"
            << "  \"bootstrap_succeeded\": " << boot_succeeded << ",\n"
            << "  \"include_b4\": " << (args.include_b4 ? "true" : "false") << "\n"
            << "}\n";
    }

    log("  → outputs in " + args.output_dir + "\n");
    log("  CAMPAIGN COMPLETE\n");
    return 0;
}

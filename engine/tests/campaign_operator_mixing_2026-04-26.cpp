/**
 * @file campaign_operator_mixing_2026-04-26.cpp
 * @brief FTD-0098: First measured native operator-mixing matrix M_ab(b=2).
 *
 * Pre-registration: docs/theory/10_eft_program/PROTOCOL_OPERATOR_MIXING_MATRIX.md
 * Plan: ~/.claude/plans/let-s-plan-a-way-ethereal-sonnet.md
 * Companion: docs/theory/10_eft_program/AUDIT_OPERATOR_SPECTRUM.md (closes [PARTIAL])
 *
 * Method (Method A — per-seed regression form, see PROTOCOL §4):
 *   1. Run a Langevin+genesis+gauss_projection ensemble (mirrors
 *      test_nonlinear_flow_multiscale.cpp parameters verbatim).
 *   2. Per snapshot k of seed s:
 *        fine_fields[s,k] = render_bridge_to_dual_cell_fields(rb)
 *        coarse_fields[s,k] = block_dual_cell_b2(fine_fields[s,k])
 *        M_fine[s,k][a]   = mean over fine voxels of operator O_a
 *        M_coarse[s,k][a] = mean over coarse voxels of operator O_a
 *      for a ∈ {O1..O6} = {J², (∇·J)², (∇×J)², J·∇(∇·J), (J·J)², s²}.
 *   3. Aggregate over all N_total snapshots:
 *        Σ_ab = ⟨ΔM_coarse_a · ΔM_fine_b⟩
 *        S_bc = ⟨ΔM_fine_b   · ΔM_fine_c⟩
 *        M_ab(b=2) = Σ · S^{-1}
 *   4. Bootstrap 100 resamples for per-entry stderr.
 *   5. Eigenvalue diagnostic: report diagonal M_aa as approximate
 *      eigenvalue, Δ_a = D − log₂(M_aa).
 *
 * Acceptance gates (from PROTOCOL §5):
 *   - Q conservation per snapshot (exact integer match)
 *   - Gauss residual < 1.0 per snapshot (loose, dual-cell adapter is
 *     face-averaged approximation)
 *   - Bootstrap stderr < 30% per entry, ≥30 of 36 entries → [MEASUREMENT]
 *   - cond(S) < 1e8 → use full 6×6
 *
 * CLI:
 *   campaign_operator_mixing                 # production: L=16, N_seeds=5, N_samples=40
 *   campaign_operator_mixing --smoke         # smoke:      L=8,  N_seeds=1, N_samples=4
 *
 * Outputs to engine/results/operator_mixing_2026-04-26/ (auto-created):
 *   meta.json, mixing_matrix.csv, mixing_matrix_stderr.csv,
 *   per_snapshot_moments.csv, eigenvalues.csv, run.log
 */

#include "ftd/constants.h"
#include "ftd/eft/dual_cell_blocking.h"
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
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

// ── Operator implementations on DualCellFields ────────────────────────
//
// The 6 operators from SPEC_OPERATOR_BASIS.md, evaluated on the dual-cell
// face-flux representation (rho_cell + phi_x/y/z). Cell-centered J is
// recovered as the average of the right- and left-face flux of each cell:
//
//   J_α(i) = ½ (phi_α[i] + phi_α[i − e_α])
//
// This is the consistent face-averaged convention used throughout the
// dual-cell module (see render_bridge_to_dual_cell_fields docstring at
// dual_cell_blocking.h:60). The resulting operator values differ from
// the RenderBridge-evaluated ones by a known O(1) face-averaging
// factor — but consistency is what matters here, since both fine and
// coarse operators use the same convention.

constexpr int kNumOps = 6;
constexpr const char* kOpNames[kNumOps] = {
    "JJ", "divJ2", "curlJ2", "JdotDivJ", "J4", "stateSq"
};
constexpr double kNaiveDim[kNumOps] = { 2.0, 4.0, 4.0, 5.0, 4.0, 2.0 };

inline double cell_J(const ftd::eft::DualCellFields& f, int x, int y, int z, int axis) {
    if (axis == 0) return 0.5 * (f.phi_x[f.index(x, y, z)] + f.phi_x[f.index(x - 1, y, z)]);
    if (axis == 1) return 0.5 * (f.phi_y[f.index(x, y, z)] + f.phi_y[f.index(x, y - 1, z)]);
    return 0.5 * (f.phi_z[f.index(x, y, z)] + f.phi_z[f.index(x, y, z - 1)]);
}

inline double op_J2(const ftd::eft::DualCellFields& f, int x, int y, int z) {
    const double Jx = cell_J(f, x, y, z, 0);
    const double Jy = cell_J(f, x, y, z, 1);
    const double Jz = cell_J(f, x, y, z, 2);
    return Jx * Jx + Jy * Jy + Jz * Jz;
}

inline double op_divJ2(const ftd::eft::DualCellFields& f, int x, int y, int z) {
    const double d = ftd::eft::div_face_at(f, x, y, z);
    return d * d;
}

inline double op_curlJ2(const ftd::eft::DualCellFields& f, int x, int y, int z) {
    // Central-difference curl on cell-centered J.
    // (∇×J)_x = ∂J_z/∂y − ∂J_y/∂z
    const double dJz_dy = 0.5 * (cell_J(f, x, y + 1, z, 2) - cell_J(f, x, y - 1, z, 2));
    const double dJy_dz = 0.5 * (cell_J(f, x, y, z + 1, 1) - cell_J(f, x, y, z - 1, 1));
    const double cx = dJz_dy - dJy_dz;
    const double dJx_dz = 0.5 * (cell_J(f, x, y, z + 1, 0) - cell_J(f, x, y, z - 1, 0));
    const double dJz_dx = 0.5 * (cell_J(f, x + 1, y, z, 2) - cell_J(f, x - 1, y, z, 2));
    const double cy = dJx_dz - dJz_dx;
    const double dJy_dx = 0.5 * (cell_J(f, x + 1, y, z, 1) - cell_J(f, x - 1, y, z, 1));
    const double dJx_dy = 0.5 * (cell_J(f, x, y + 1, z, 0) - cell_J(f, x, y - 1, z, 0));
    const double cz = dJy_dx - dJx_dy;
    return cx * cx + cy * cy + cz * cz;
}

inline double op_JdotDivJ(const ftd::eft::DualCellFields& f, int x, int y, int z) {
    const double Jx = cell_J(f, x, y, z, 0);
    const double Jy = cell_J(f, x, y, z, 1);
    const double Jz = cell_J(f, x, y, z, 2);
    const double gx = 0.5 * (ftd::eft::div_face_at(f, x + 1, y, z) -
                             ftd::eft::div_face_at(f, x - 1, y, z));
    const double gy = 0.5 * (ftd::eft::div_face_at(f, x, y + 1, z) -
                             ftd::eft::div_face_at(f, x, y - 1, z));
    const double gz = 0.5 * (ftd::eft::div_face_at(f, x, y, z + 1) -
                             ftd::eft::div_face_at(f, x, y, z - 1));
    return Jx * gx + Jy * gy + Jz * gz;
}

inline double op_J4(const ftd::eft::DualCellFields& f, int x, int y, int z) {
    const double j2 = op_J2(f, x, y, z);
    return j2 * j2;
}

inline double op_stateSq(const ftd::eft::DualCellFields& f, int x, int y, int z) {
    const double s = static_cast<double>(f.rho_cell[f.index(x, y, z)]);
    return s * s;
}

using OpVec = std::array<double, kNumOps>;

// Per-cell evaluator dispatched by operator id.
inline double evaluate_op(const ftd::eft::DualCellFields& f, int x, int y, int z, int op_id) {
    switch (op_id) {
        case 0: return op_J2(f, x, y, z);
        case 1: return op_divJ2(f, x, y, z);
        case 2: return op_curlJ2(f, x, y, z);
        case 3: return op_JdotDivJ(f, x, y, z);
        case 4: return op_J4(f, x, y, z);
        case 5: return op_stateSq(f, x, y, z);
    }
    return 0.0;
}

// Mean-over-cells operator vector for one DualCellFields snapshot.
OpVec mean_operators(const ftd::eft::DualCellFields& f) {
    OpVec acc{};
    const int L = f.L;
    for (int z = 0; z < L; ++z)
        for (int y = 0; y < L; ++y)
            for (int x = 0; x < L; ++x)
                for (int a = 0; a < kNumOps; ++a)
                    acc[a] += evaluate_op(f, x, y, z, a);
    const double inv_N = 1.0 / static_cast<double>(L * L * L);
    for (int a = 0; a < kNumOps; ++a) acc[a] *= inv_N;
    return acc;
}

// ── 6×6 linear algebra (inline, no external dep) ──────────────────────

using Mat = std::array<std::array<double, kNumOps>, kNumOps>;

Mat zero_mat() {
    Mat M{};
    for (auto& row : M) row.fill(0.0);
    return M;
}

// Gauss–Jordan inversion with partial pivoting. Returns false if singular.
bool invert_6x6(const Mat& A, Mat& Ainv, double& cond_estimate) {
    constexpr int N = kNumOps;
    std::array<std::array<double, 2 * N>, N> aug{};
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) aug[i][j] = A[i][j];
        for (int j = 0; j < N; ++j) aug[i][N + j] = (i == j) ? 1.0 : 0.0;
    }
    double max_pivot = 0.0;
    double min_pivot = 1e300;
    for (int col = 0; col < N; ++col) {
        // Pivot
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

Mat matmul(const Mat& A, const Mat& B) {
    Mat C = zero_mat();
    for (int i = 0; i < kNumOps; ++i)
        for (int j = 0; j < kNumOps; ++j)
            for (int k = 0; k < kNumOps; ++k)
                C[i][j] += A[i][k] * B[k][j];
    return C;
}

// Compute Σ (cross-cov) and S (fine-only cov) from a vector of (fine, coarse) samples.
// Returns false if N < 2.
struct CovResult {
    Mat Sigma{};   // Σ_ab = ⟨Δcoarse_a · Δfine_b⟩
    Mat S{};       // S_bc = ⟨Δfine_b · Δfine_c⟩
    OpVec mean_fine{};
    OpVec mean_coarse{};
    int N = 0;
    bool valid = false;
};

CovResult compute_covariances(const std::vector<OpVec>& fine_samples,
                              const std::vector<OpVec>& coarse_samples,
                              const std::vector<int>& indices) {
    CovResult R;
    R.N = static_cast<int>(indices.size());
    if (R.N < 2 || fine_samples.size() != coarse_samples.size()) return R;
    OpVec mf{}, mc{};
    for (int i : indices) {
        for (int a = 0; a < kNumOps; ++a) {
            mf[a] += fine_samples[i][a];
            mc[a] += coarse_samples[i][a];
        }
    }
    for (int a = 0; a < kNumOps; ++a) {
        mf[a] /= R.N;
        mc[a] /= R.N;
    }
    R.mean_fine = mf;
    R.mean_coarse = mc;
    R.Sigma = zero_mat();
    R.S = zero_mat();
    for (int i : indices) {
        OpVec dF, dC;
        for (int a = 0; a < kNumOps; ++a) {
            dF[a] = fine_samples[i][a] - mf[a];
            dC[a] = coarse_samples[i][a] - mc[a];
        }
        for (int a = 0; a < kNumOps; ++a) {
            for (int b = 0; b < kNumOps; ++b) {
                R.Sigma[a][b] += dC[a] * dF[b];
                R.S[a][b] += dF[a] * dF[b];
            }
        }
    }
    const double inv = 1.0 / static_cast<double>(R.N - 1);
    for (int a = 0; a < kNumOps; ++a)
        for (int b = 0; b < kNumOps; ++b) {
            R.Sigma[a][b] *= inv;
            R.S[a][b] *= inv;
        }
    R.valid = true;
    return R;
}

// Solve M = Σ · S^{-1}; returns false on singular S.
bool solve_mixing_matrix(const Mat& Sigma, const Mat& S, Mat& M_out, double& cond_S) {
    Mat S_inv{};
    if (!invert_6x6(S, S_inv, cond_S)) return false;
    M_out = matmul(Sigma, S_inv);
    return true;
}

// ── Symmetric-matrix Jacobi eigendecomposition (FTD-0099, F3) ────────
//
// Used for Wilson-coefficient / RG-eigendirection extraction. The
// regression-derived M_ab is generally not symmetric; we diagonalize
// its symmetric part (M + M^T)/2, which gives the dominant rotation
// of operators under the blocking flow modulo skew-symmetric contri-
// butions. The eigenvalues serve as scaling-dimension estimates for
// the eigendirections (linear combinations of the original basis ops).
//
// Standard cyclic-Jacobi sweep, restricted to active operators (i.e.
// the ones not dropped by the degradation ladder). NaN entries are
// replaced by zeros in the symmetric input (those cells are not
// active anyway).
struct EigResult {
    std::array<double, kNumOps> eigenvalues{};   // sorted descending
    Mat eigenvectors{};                          // columns are eigenvectors
    bool valid = false;
    int active_K = 0;
};

EigResult eig_symmetric(const Mat& M, const std::vector<int>& active) {
    EigResult R;
    const int K = static_cast<int>(active.size());
    R.active_K = K;
    if (K < 1) return R;

    // Build dense K×K symmetric input (M+M^T)/2 over active ops only.
    std::vector<std::vector<double>> A(K, std::vector<double>(K, 0.0));
    std::vector<std::vector<double>> V(K, std::vector<double>(K, 0.0));
    for (int i = 0; i < K; ++i) V[i][i] = 1.0;
    for (int i = 0; i < K; ++i)
        for (int j = 0; j < K; ++j) {
            const double m1 = M[active[i]][active[j]];
            const double m2 = M[active[j]][active[i]];
            const double s = (std::isfinite(m1) ? m1 : 0.0) +
                             (std::isfinite(m2) ? m2 : 0.0);
            A[i][j] = 0.5 * s;
        }

    // Cyclic Jacobi sweep
    for (int sweep = 0; sweep < 100; ++sweep) {
        double off = 0.0;
        for (int i = 0; i < K; ++i)
            for (int j = i + 1; j < K; ++j)
                off += A[i][j] * A[i][j];
        if (off < 1e-24) break;
        for (int p = 0; p < K - 1; ++p) {
            for (int q = p + 1; q < K; ++q) {
                const double app = A[p][p], aqq = A[q][q], apq = A[p][q];
                if (std::abs(apq) < 1e-30) continue;
                double theta = (aqq - app) / (2.0 * apq);
                double t = (theta >= 0.0)
                         ? 1.0 / (theta + std::sqrt(1.0 + theta * theta))
                         : 1.0 / (theta - std::sqrt(1.0 + theta * theta));
                const double c = 1.0 / std::sqrt(1.0 + t * t);
                const double s_ = t * c;
                A[p][p] = app - t * apq;
                A[q][q] = aqq + t * apq;
                A[p][q] = 0.0;
                A[q][p] = 0.0;
                for (int r = 0; r < K; ++r) {
                    if (r != p && r != q) {
                        const double arp = A[r][p], arq = A[r][q];
                        A[r][p] = c * arp - s_ * arq;
                        A[p][r] = A[r][p];
                        A[r][q] = s_ * arp + c * arq;
                        A[q][r] = A[r][q];
                    }
                    const double vrp = V[r][p], vrq = V[r][q];
                    V[r][p] = c * vrp - s_ * vrq;
                    V[r][q] = s_ * vrp + c * vrq;
                }
            }
        }
    }

    // Extract eigenvalues from diagonal; sort descending.
    std::vector<std::pair<double, int>> evs(K);
    for (int i = 0; i < K; ++i) evs[i] = { A[i][i], i };
    std::sort(evs.begin(), evs.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    R.eigenvalues.fill(std::nan("dropped"));
    R.eigenvectors = zero_mat();
    for (int i = 0; i < kNumOps; ++i)
        for (int j = 0; j < kNumOps; ++j)
            R.eigenvectors[i][j] = std::nan("dropped");
    for (int k = 0; k < K; ++k) {
        R.eigenvalues[k] = evs[k].first;
        const int src_col = evs[k].second;
        for (int i = 0; i < K; ++i) {
            R.eigenvectors[active[i]][active[k]] = V[i][src_col];
        }
    }
    R.valid = true;
    return R;
}

// CSV writers
void write_matrix_csv(const fs::path& path, const Mat& M, const char* header_prefix) {
    std::ofstream f(path);
    if (!f) return;
    f << "#," << header_prefix << "\n";
    f << "row\\col";
    for (int j = 0; j < kNumOps; ++j) f << "," << kOpNames[j];
    f << "\n";
    for (int i = 0; i < kNumOps; ++i) {
        f << kOpNames[i];
        for (int j = 0; j < kNumOps; ++j) f << "," << M[i][j];
        f << "\n";
    }
}

const char* tier_name(double Delta) {
    if (Delta < 3.5) return "relevant";
    if (Delta <= 4.5) return "marginal";
    return "irrelevant";
}

}  // namespace

int main(int argc, char** argv) {
    bool smoke_mode = false;
    bool include_b4 = false;
    int L_override = 0;
    int N_SAMPLES_override = 0;
    int N_SEEDS_override = 0;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--smoke") smoke_mode = true;
        else if (a == "--b4") include_b4 = true;
        else if (a.rfind("--L=", 0) == 0) L_override = std::atoi(a.c_str() + 4);
        else if (a.rfind("--samples=", 0) == 0) N_SAMPLES_override = std::atoi(a.c_str() + 10);
        else if (a.rfind("--seeds=", 0) == 0) N_SEEDS_override = std::atoi(a.c_str() + 8);
    }

    // ── Pre-registered ensemble parameters (PROTOCOL §3 + FTD-0099 extension) ─────
    // Smoke mode: small but enough to make the 6×6 covariance non-singular
    // (need N_total > kNumOps = 6, with margin). Production mode mirrors
    // test_nonlinear_flow_multiscale.cpp at L=16.
    //
    // F1 (FTD-0099): --L=N override for multilatitude run. L must be ≥4 and
    //                ≥8 if --b4 is set (to allow two b=2 iterations). At L=32
    //                expect ~50 s wall on RTX 5090 (vs ~6.3 s at L=16).
    // F5 (FTD-0099): --b4 enables M(b=4) measurement via twice-applied b=2;
    //                tests RG semigroup property M(b=4) ≈ M(b=2)·M(b=2) mod
    //                bootstrap noise.
    int L          = smoke_mode ? 8   : 16;
    int N_BURN     = smoke_mode ? 50  : 200;
    int N_SAMPLES  = smoke_mode ? 8   : 40;
    int N_SEEDS    = smoke_mode ? 2   : 5;
    if (L_override > 0)        L         = L_override;
    if (N_SAMPLES_override > 0) N_SAMPLES = N_SAMPLES_override;
    if (N_SEEDS_override > 0)  N_SEEDS   = N_SEEDS_override;

    if (include_b4 && L < 8) {
        std::cerr << "  --b4 requires L >= 8 (need two b=2 iterations); got L=" << L << "\n";
        return 5;
    }

    const int SAMPLE_STRIDE = 5;
    const std::uint32_t BASE_SEED = 0xF10412E5u;
    const int N_BOOTSTRAP = 100;

    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: First Measured Native Operator-Mixing Matrix M_ab(b=2)\n";
    std::cout << "  LEDGER row:  FTD-0098\n";
    std::cout << "  Pre-reg:     PROTOCOL_OPERATOR_MIXING_MATRIX.md\n";
    std::cout << "================================================================\n";
    std::cout << (smoke_mode ? "  Mode: SMOKE\n" : "  Mode: PRODUCTION\n");
    std::cout << "  L=" << L
              << "  N_BURN=" << N_BURN
              << "  N_SAMPLES=" << N_SAMPLES
              << "  SAMPLE_STRIDE=" << SAMPLE_STRIDE
              << "  N_SEEDS=" << N_SEEDS
              << "  N_BOOTSTRAP=" << N_BOOTSTRAP
              << "\n";

    // ── Per-snapshot accumulators ────────────────────────────────────
    std::vector<OpVec> fine_samples;
    std::vector<OpVec> coarse_samples;     // b=2
    std::vector<OpVec> coarse4_samples;    // b=4 (only if --b4)
    fine_samples.reserve(static_cast<size_t>(N_SEEDS) * N_SAMPLES);
    coarse_samples.reserve(static_cast<size_t>(N_SEEDS) * N_SAMPLES);
    if (include_b4) coarse4_samples.reserve(static_cast<size_t>(N_SEEDS) * N_SAMPLES);

    int snapshots_dropped = 0;
    int snapshots_with_state = 0;
    int q_conservation_violations = 0;

    // ── Per-seed ensemble loop ───────────────────────────────────────
    for (int s = 0; s < N_SEEDS; ++s) {
        const std::uint32_t seed = BASE_SEED + static_cast<std::uint32_t>(s) * 0x100u;
        std::cout << "  seed " << s << "/" << N_SEEDS << " (rng=0x" << std::hex << seed << std::dec << ")\n";

        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis          = true;
        rb.toggles.langevin         = true;
        rb.toggles.langevin_T       = 0.005;
        rb.toggles.langevin_gamma   = 0.02;
        rb.toggles.dual_substrate   = false;
        rb.seed_rng(seed);
        rb.inject_flux(L / 2, L / 2, L / 2,
                       {3.0 * ftd::K_GENESIS, 0, 0});
        rb.run(N_BURN);

        for (int k = 0; k < N_SAMPLES; ++k) {
            rb.run(SAMPLE_STRIDE);

            const auto fine    = ftd::eft::render_bridge_to_dual_cell_fields(rb);
            const auto coarse2 = ftd::eft::block_dual_cell_b2(fine);
            // F5: optional b=4 = block twice
            ftd::eft::DualCellFields coarse4 = include_b4
                ? ftd::eft::block_dual_cell_b2(coarse2)
                : ftd::eft::DualCellFields{};

            // Q conservation gate
            const int q_fine    = ftd::eft::total_source(fine);
            const int q_coarse2 = ftd::eft::total_source(coarse2);
            const int q_coarse4 = include_b4 ? ftd::eft::total_source(coarse4) : q_fine;
            if (q_fine != q_coarse2 || q_fine != q_coarse4) {
                ++q_conservation_violations;
                ++snapshots_dropped;
                continue;
            }

            // Gauss residual gate (loose tolerance per PROTOCOL §5)
            const double res_fine    = ftd::eft::max_gauss_residual(fine);
            const double res_coarse2 = ftd::eft::max_gauss_residual(coarse2);
            const double res_coarse4 = include_b4 ? ftd::eft::max_gauss_residual(coarse4) : 0.0;
            if (!std::isfinite(res_fine)    || res_fine    > 1.0 ||
                !std::isfinite(res_coarse2) || res_coarse2 > 1.0 ||
                (include_b4 && (!std::isfinite(res_coarse4) || res_coarse4 > 1.0))) {
                ++snapshots_dropped;
                continue;
            }

            // Track non-zero state (R4 mitigation)
            int abs_state_sum = 0;
            for (int q : fine.rho_cell) abs_state_sum += std::abs(q);
            if (abs_state_sum > 0) ++snapshots_with_state;

            fine_samples.push_back(mean_operators(fine));
            coarse_samples.push_back(mean_operators(coarse2));
            if (include_b4) coarse4_samples.push_back(mean_operators(coarse4));
        }
    }

    const int N_total = static_cast<int>(fine_samples.size());
    std::cout << "\n  collected " << N_total << " snapshots ("
              << snapshots_dropped << " dropped"
              << (q_conservation_violations > 0 ?
                  ", " + std::to_string(q_conservation_violations) + " Q-conservation violations"
                  : "")
              << ", " << snapshots_with_state << " with non-zero state)\n";

    int g_failures = 0;
    auto check = [&g_failures](const std::string& name, bool ok) {
        std::cout << (ok ? "  PASS  " : "  FAIL  ") << name << "\n";
        if (!ok) ++g_failures;
    };

    // Pre-registered acceptance gates ─────────────────────────────────
    check("at least 4 snapshots collected", N_total >= 4);
    check("Q conservation: zero violations", q_conservation_violations == 0);

    if (N_total < 4) {
        std::cout << "\n  ABORT: insufficient snapshots; cannot build mixing matrix.\n";
        return 1;
    }

    // Default: include all snapshots in the regression
    std::vector<int> all_indices(N_total);
    for (int i = 0; i < N_total; ++i) all_indices[i] = i;

    auto cov = compute_covariances(fine_samples, coarse_samples, all_indices);
    if (!cov.valid) {
        std::cout << "  FAIL  covariance computation\n";
        return 2;
    }

    Mat M_main = zero_mat();
    double cond_S = 0.0;
    bool inv_ok = solve_mixing_matrix(cov.Sigma, cov.S, M_main, cond_S);

    // Diagnostic: print fine-only covariance variances (S diagonal) so we
    // can spot zero-variance operators when S is singular.
    std::cout << "\n--- S diagonal (fine-only auto-variance per operator) ---\n";
    for (int a = 0; a < kNumOps; ++a) {
        std::printf("  %-10s  Var(M_fine_a) = %+.4e\n", kOpNames[a], cov.S[a][a]);
    }

    // Pre-registered degradation ladder (PROTOCOL §5, cond(S) > 1e8):
    // identify operators with smallest variance and drop them until S is
    // well-conditioned. Report which were dropped in meta.json.
    std::vector<int> dropped_ops;
    if (!inv_ok || cond_S > 1e8) {
        std::cout << "\n  S ill-conditioned (cond ≈ " << cond_S
                  << "). Engaging degradation ladder.\n";
        // Iteratively drop the operator with smallest variance until
        // remaining submatrix inverts cleanly. The dropped-op IDs are
        // recorded for the ANALYSIS; full M is reported as 6×6 with
        // dropped rows/cols set to NaN.
        std::vector<int> active(kNumOps);
        for (int a = 0; a < kNumOps; ++a) active[a] = a;
        while (active.size() > 2) {
            // Find operator with smallest variance among active
            int worst_idx = 0;
            double worst_var = cov.S[active[0]][active[0]];
            for (size_t i = 1; i < active.size(); ++i) {
                if (cov.S[active[i]][active[i]] < worst_var) {
                    worst_var = cov.S[active[i]][active[i]];
                    worst_idx = static_cast<int>(i);
                }
            }
            const int drop_id = active[worst_idx];
            dropped_ops.push_back(drop_id);
            active.erase(active.begin() + worst_idx);
            std::cout << "    drop " << kOpNames[drop_id]
                      << " (variance " << worst_var << ")\n";

            // Build reduced submatrix and try inversion
            const int K = static_cast<int>(active.size());
            std::vector<std::vector<double>> S_sub(K, std::vector<double>(K, 0.0));
            std::vector<std::vector<double>> Sigma_sub(K, std::vector<double>(K, 0.0));
            for (int i = 0; i < K; ++i)
                for (int j = 0; j < K; ++j) {
                    S_sub[i][j] = cov.S[active[i]][active[j]];
                    Sigma_sub[i][j] = cov.Sigma[active[i]][active[j]];
                }

            // Reduced Gauss-Jordan
            std::vector<std::vector<double>> aug(K, std::vector<double>(2 * K, 0.0));
            for (int i = 0; i < K; ++i) {
                for (int j = 0; j < K; ++j) aug[i][j] = S_sub[i][j];
                aug[i][K + i] = 1.0;
            }
            double mxp = 0.0, mnp = 1e300;
            bool reduced_ok = true;
            for (int col = 0; col < K; ++col) {
                int pr = col; double pa = std::abs(aug[col][col]);
                for (int r = col + 1; r < K; ++r)
                    if (std::abs(aug[r][col]) > pa) { pa = std::abs(aug[r][col]); pr = r; }
                if (pa < 1e-30) { reduced_ok = false; break; }
                if (pr != col) std::swap(aug[col], aug[pr]);
                const double piv = aug[col][col];
                mxp = std::max(mxp, std::abs(piv));
                mnp = std::min(mnp, std::abs(piv));
                for (int j = 0; j < 2 * K; ++j) aug[col][j] /= piv;
                for (int r = 0; r < K; ++r) {
                    if (r == col) continue;
                    const double fac = aug[r][col];
                    if (fac == 0.0) continue;
                    for (int j = 0; j < 2 * K; ++j) aug[r][j] -= fac * aug[col][j];
                }
            }
            if (!reduced_ok) continue;
            const double cond_reduced = (mnp > 0.0) ? (mxp / mnp) : 1e300;
            if (cond_reduced > 1e8) continue;

            // Solve reduced M_sub = Sigma_sub · S_sub^{-1}
            std::vector<std::vector<double>> M_sub(K, std::vector<double>(K, 0.0));
            for (int i = 0; i < K; ++i)
                for (int j = 0; j < K; ++j)
                    for (int k = 0; k < K; ++k)
                        M_sub[i][j] += Sigma_sub[i][k] * aug[k][K + j];

            // Lift M_sub back to 6×6 with NaN in dropped rows/cols
            M_main = zero_mat();
            for (int i = 0; i < kNumOps; ++i)
                for (int j = 0; j < kNumOps; ++j)
                    M_main[i][j] = std::nan("dropped");
            for (int i = 0; i < K; ++i)
                for (int j = 0; j < K; ++j)
                    M_main[active[i]][active[j]] = M_sub[i][j];

            cond_S = cond_reduced;
            inv_ok = true;
            std::cout << "    reduced (" << K << "×" << K
                      << ") cond(S) = " << cond_S << "\n";
            break;
        }
    }

    check("mixing-matrix solve succeeds after degradation ladder", inv_ok);
    check("cond(S) < 1e8 (after any necessary drops)", cond_S < 1e8);

    if (!inv_ok) {
        std::cout << "\n  ABORT: S singular even after degradation. cond(S) ≈ "
                  << cond_S << "\n";
        return 3;
    }

    // ── Bootstrap (uses same active subspace as main solve) ──────────
    // The active set excludes operators that triggered the degradation
    // ladder above. This keeps every resample on the same basis as the
    // headline matrix, so bootstrap stderr is meaningful per-entry.
    std::vector<int> active(kNumOps);
    for (int a = 0; a < kNumOps; ++a) active[a] = a;
    for (int dropped : dropped_ops) {
        active.erase(std::remove(active.begin(), active.end(), dropped), active.end());
    }
    const int K = static_cast<int>(active.size());

    auto solve_reduced_on_indices = [&](const std::vector<int>& indices, Mat& M_out) -> bool {
        const auto cov_b = compute_covariances(fine_samples, coarse_samples, indices);
        if (!cov_b.valid) return false;
        // Build reduced K×K subsystem
        std::vector<std::vector<double>> aug(K, std::vector<double>(2 * K, 0.0));
        for (int i = 0; i < K; ++i) {
            for (int j = 0; j < K; ++j) aug[i][j] = cov_b.S[active[i]][active[j]];
            aug[i][K + i] = 1.0;
        }
        for (int col = 0; col < K; ++col) {
            int pr = col; double pa = std::abs(aug[col][col]);
            for (int r = col + 1; r < K; ++r)
                if (std::abs(aug[r][col]) > pa) { pa = std::abs(aug[r][col]); pr = r; }
            if (pa < 1e-30) return false;
            if (pr != col) std::swap(aug[col], aug[pr]);
            const double piv = aug[col][col];
            for (int j = 0; j < 2 * K; ++j) aug[col][j] /= piv;
            for (int r = 0; r < K; ++r) {
                if (r == col) continue;
                const double fac = aug[r][col];
                if (fac == 0.0) continue;
                for (int j = 0; j < 2 * K; ++j) aug[r][j] -= fac * aug[col][j];
            }
        }
        // M_sub = Sigma_sub · S_sub^{-1}, lift back to 6×6 with NaN pads
        M_out = zero_mat();
        for (int i = 0; i < kNumOps; ++i)
            for (int j = 0; j < kNumOps; ++j)
                M_out[i][j] = std::nan("dropped");
        for (int i = 0; i < K; ++i)
            for (int j = 0; j < K; ++j) {
                double v = 0.0;
                for (int k = 0; k < K; ++k)
                    v += cov_b.Sigma[active[i]][active[k]] * aug[k][K + j];
                M_out[active[i]][active[j]] = v;
            }
        return true;
    };

    std::mt19937 boot_rng(0xB007BEEFu);
    std::uniform_int_distribution<int> picker(0, N_total - 1);
    std::vector<Mat> M_resamples;
    M_resamples.reserve(N_BOOTSTRAP);
    int boot_failures = 0;
    for (int b = 0; b < N_BOOTSTRAP; ++b) {
        std::vector<int> resample(N_total);
        for (int i = 0; i < N_total; ++i) resample[i] = picker(boot_rng);
        Mat M_b{};
        if (!solve_reduced_on_indices(resample, M_b)) { ++boot_failures; continue; }
        M_resamples.push_back(M_b);
    }
    std::cout << "  bootstrap: " << M_resamples.size() << "/" << N_BOOTSTRAP
              << " resamples succeeded";
    if (boot_failures > 0) std::cout << " (" << boot_failures << " singular)";
    std::cout << "\n";

    // Per-entry bootstrap stderr
    Mat M_stderr = zero_mat();
    if (!M_resamples.empty()) {
        Mat M_sum = zero_mat();
        for (const auto& M : M_resamples)
            for (int i = 0; i < kNumOps; ++i)
                for (int j = 0; j < kNumOps; ++j)
                    M_sum[i][j] += M[i][j];
        Mat M_avg = zero_mat();
        for (int i = 0; i < kNumOps; ++i)
            for (int j = 0; j < kNumOps; ++j)
                M_avg[i][j] = M_sum[i][j] / static_cast<double>(M_resamples.size());

        Mat M_var = zero_mat();
        for (const auto& M : M_resamples)
            for (int i = 0; i < kNumOps; ++i)
                for (int j = 0; j < kNumOps; ++j) {
                    const double d = M[i][j] - M_avg[i][j];
                    M_var[i][j] += d * d;
                }
        const double denom = static_cast<double>(M_resamples.size());
        for (int i = 0; i < kNumOps; ++i)
            for (int j = 0; j < kNumOps; ++j)
                M_stderr[i][j] = std::sqrt(M_var[i][j] / denom);
    }

    // ── Eigenvalue diagnostic (diagonal entries as approximate λ_a) ──
    std::array<double, kNumOps> eig_diag{};
    std::array<double, kNumOps> Delta_a{};
    std::array<const char*, kNumOps> tier{};
    constexpr double D = 4.0;  // spacetime dimensions (3+1) per PROTOCOL §5
    for (int a = 0; a < kNumOps; ++a) {
        eig_diag[a] = M_main[a][a];
        if (eig_diag[a] > 0.0 && std::isfinite(eig_diag[a])) {
            Delta_a[a] = D - std::log2(eig_diag[a]);
        } else {
            Delta_a[a] = std::nan("nonpositive_eigenvalue");
        }
        tier[a] = std::isnan(Delta_a[a]) ? "n/a" : tier_name(Delta_a[a]);
    }

    // ── F3: Wilson-coefficient eigendecomposition (FTD-0099) ──────────
    // Diagonalize the symmetric part of M restricted to the active
    // subspace. Eigenvalues are scaling-dimension estimates for the
    // dominant rotation under blocking; eigenvectors are linear
    // combinations of the original 6 operators that approximate
    // fixed-point eigendirections.
    const auto eig = eig_symmetric(M_main, active);
    std::array<double, kNumOps> Delta_eig{};
    std::array<const char*, kNumOps> tier_eig{};
    for (int k = 0; k < kNumOps; ++k) {
        const double lam = eig.eigenvalues[k];
        if (std::isfinite(lam) && lam > 0.0) {
            Delta_eig[k] = D - std::log2(lam);
            tier_eig[k] = tier_name(Delta_eig[k]);
        } else {
            Delta_eig[k] = std::nan("nonpositive_or_dropped");
            tier_eig[k] = "n/a";
        }
    }

    // ── F5: M(b=4) measurement + RG semigroup test (FTD-0099) ─────────
    Mat M_b4 = zero_mat();
    Mat M_b2_squared = zero_mat();
    bool semigroup_ok = false;
    double semigroup_max_relerr = 0.0;
    if (include_b4) {
        // Build (fine, coarse4) covariances on the same active subspace
        const auto cov4 = compute_covariances(fine_samples, coarse4_samples, all_indices);
        if (cov4.valid) {
            // Reduced-subspace solve mirroring the b=2 path
            std::vector<std::vector<double>> aug(K, std::vector<double>(2 * K, 0.0));
            for (int i = 0; i < K; ++i) {
                for (int j = 0; j < K; ++j) aug[i][j] = cov4.S[active[i]][active[j]];
                aug[i][K + i] = 1.0;
            }
            bool ok = true;
            for (int col = 0; col < K; ++col) {
                int pr = col; double pa = std::abs(aug[col][col]);
                for (int r = col + 1; r < K; ++r)
                    if (std::abs(aug[r][col]) > pa) { pa = std::abs(aug[r][col]); pr = r; }
                if (pa < 1e-30) { ok = false; break; }
                if (pr != col) std::swap(aug[col], aug[pr]);
                const double piv = aug[col][col];
                for (int j = 0; j < 2 * K; ++j) aug[col][j] /= piv;
                for (int r = 0; r < K; ++r) {
                    if (r == col) continue;
                    const double fac = aug[r][col];
                    if (fac == 0.0) continue;
                    for (int j = 0; j < 2 * K; ++j) aug[r][j] -= fac * aug[col][j];
                }
            }
            if (ok) {
                M_b4 = zero_mat();
                for (int i = 0; i < kNumOps; ++i)
                    for (int j = 0; j < kNumOps; ++j) M_b4[i][j] = std::nan("dropped");
                for (int i = 0; i < K; ++i) {
                    for (int j = 0; j < K; ++j) {
                        double v = 0.0;
                        for (int k = 0; k < K; ++k)
                            v += cov4.Sigma[active[i]][active[k]] * aug[k][K + j];
                        M_b4[active[i]][active[j]] = v;
                    }
                }
                // RG semigroup: M(b=4) ?= M(b=2) · M(b=2) on the active block
                M_b2_squared = zero_mat();
                for (int i = 0; i < kNumOps; ++i)
                    for (int j = 0; j < kNumOps; ++j) M_b2_squared[i][j] = std::nan("dropped");
                for (int i = 0; i < K; ++i)
                    for (int j = 0; j < K; ++j) {
                        double v = 0.0;
                        for (int k = 0; k < K; ++k)
                            v += M_main[active[i]][active[k]] * M_main[active[k]][active[j]];
                        M_b2_squared[active[i]][active[j]] = v;
                    }
                // Max relative error across active block
                semigroup_max_relerr = 0.0;
                for (int i = 0; i < K; ++i)
                    for (int j = 0; j < K; ++j) {
                        const double m4 = M_b4[active[i]][active[j]];
                        const double m22 = M_b2_squared[active[i]][active[j]];
                        const double denom = std::max({std::abs(m4), std::abs(m22), 1e-12});
                        const double err = std::abs(m4 - m22) / denom;
                        if (err > semigroup_max_relerr) semigroup_max_relerr = err;
                    }
                semigroup_ok = std::isfinite(semigroup_max_relerr);
            }
        }
    }

    // Diagonal-dominance count (report-only; informs ANALYSIS, NOT a gate)
    int diag_dominant_count = 0;
    for (int i = 0; i < kNumOps; ++i) {
        if (std::isnan(M_main[i][i])) continue;
        double row_abs_sum = 0.0;
        for (int j = 0; j < kNumOps; ++j) {
            if (std::isnan(M_main[i][j])) continue;
            row_abs_sum += std::abs(M_main[i][j]);
        }
        const double frac = (row_abs_sum > 0.0)
                          ? std::abs(M_main[i][i]) / row_abs_sum : 0.0;
        if (frac >= 0.5) ++diag_dominant_count;
    }

    // Bootstrap-stderr count (report-only; determines [MEASUREMENT] vs [PARTIAL] tag)
    int converged_entries = 0;
    int counted_entries = 0;  // entries with finite values (excludes degraded NaNs)
    for (int i = 0; i < kNumOps; ++i)
        for (int j = 0; j < kNumOps; ++j) {
            if (std::isnan(M_main[i][j])) continue;
            ++counted_entries;
            const double m = std::abs(M_main[i][j]);
            const double s = M_stderr[i][j];
            if (m > 0.0 && (s / m) < 0.30) ++converged_entries;
        }
    // Tag rule (PROTOCOL §5): MEASUREMENT requires ≥30/36 entries converged
    // when no degradation. With degradation, scale threshold proportionally
    // to the active subspace size.
    const int required_converged = static_cast<int>(std::ceil(30.0 * counted_entries / 36.0));
    const bool measurement_tag = (converged_entries >= required_converged);

    // ── Print headline ───────────────────────────────────────────────
    std::cout << "\n--- M_ab(b=2): mixing matrix (rows=coarse, cols=fine) ---\n";
    std::cout << "          ";
    for (int j = 0; j < kNumOps; ++j) std::printf("%12s", kOpNames[j]);
    std::cout << "\n";
    for (int i = 0; i < kNumOps; ++i) {
        std::printf("  %-8s", kOpNames[i]);
        for (int j = 0; j < kNumOps; ++j) std::printf(" %+11.4e", M_main[i][j]);
        std::cout << "\n";
    }

    std::cout << "\n--- σ(M_ab) bootstrap stderr ---\n";
    std::cout << "          ";
    for (int j = 0; j < kNumOps; ++j) std::printf("%12s", kOpNames[j]);
    std::cout << "\n";
    for (int i = 0; i < kNumOps; ++i) {
        std::printf("  %-8s", kOpNames[i]);
        for (int j = 0; j < kNumOps; ++j) std::printf("  %10.3e", M_stderr[i][j]);
        std::cout << "\n";
    }

    std::cout << "\n--- Eigenvalue diagnostic (diagonal of M; per-step Δ_a = D − log₂ λ_a; D=4) ---\n";
    std::printf("  %-10s %-12s %-12s %-12s %-12s\n",
                "Op", "λ (M_aa)", "Δ_a meas", "Δ_a naive", "tier (meas)");
    for (int a = 0; a < kNumOps; ++a) {
        std::printf("  %-10s %+11.4e %+11.4e %+11.4e %s\n",
                    kOpNames[a], eig_diag[a], Delta_a[a], kNaiveDim[a], tier[a]);
    }
    std::printf("\n  cond(S) = %.3e\n", cond_S);
    std::printf("  diagonal-dominant operators: %d/6\n", diag_dominant_count);
    std::printf("  converged stderr entries: %d/36\n", converged_entries);

    // F3: Wilson-coefficient eigendecomposition output
    std::cout << "\n--- F3: Wilson eigendecomposition of (M+M^T)/2 ---\n";
    std::printf("  %-4s %-14s %-14s %-12s\n", "k", "λ_k (eig)", "Δ_k = D−log₂λ", "tier");
    for (int k = 0; k < eig.active_K; ++k) {
        std::printf("  %-4d %+13.4e %+13.4e %s\n",
                    k, eig.eigenvalues[k], Delta_eig[k], tier_eig[k]);
    }

    // F5: RG semigroup test output
    if (include_b4) {
        std::cout << "\n--- F5: RG semigroup M(b=4) vs M(b=2)·M(b=2) ---\n";
        if (semigroup_ok) {
            std::printf("  max relative error across active block: %.3e\n",
                        semigroup_max_relerr);
            std::printf("  semigroup verdict: %s (threshold: 0.5 = 50%% relerr at this ensemble size)\n",
                        semigroup_max_relerr < 0.5 ? "PASS (within bootstrap noise)"
                                                  : "FAIL (mixing matrix is not multiplicative on this ensemble)");
        } else {
            std::cout << "  semigroup test: SKIPPED (singular S on b=4 covariance)\n";
        }
    }

    // ── Output artifacts ─────────────────────────────────────────────
    // FTD-0099 (2026-04-26): per-config subdirectory to preserve
    // multiple runs (FTD-0098 baseline at L=16 b=2, plus FTD-0099
    // extensions at L=16 b=4 and L=32 b=4) under one umbrella dir.
    const std::string config_tag = "L" + std::to_string(L) +
                                   (include_b4 ? "_b4" : "_b2");
    fs::path out_dir = fs::path("engine/results/operator_mixing_2026-04-26") / config_tag;
    std::error_code ec;
    fs::create_directories(out_dir, ec);
    if (ec) {
        std::cerr << "  WARN  could not create " << out_dir << ": " << ec.message() << "\n";
    }
    write_matrix_csv(out_dir / "mixing_matrix.csv", M_main, "M_ab(b=2)");
    write_matrix_csv(out_dir / "mixing_matrix_stderr.csv", M_stderr, "stderr (bootstrap, 100 resamples)");
    if (include_b4) {
        write_matrix_csv(out_dir / "mixing_matrix_b4.csv", M_b4, "M_ab(b=4)");
        write_matrix_csv(out_dir / "mixing_matrix_b2_squared.csv", M_b2_squared,
                         "M(b=2)·M(b=2) — RG semigroup prediction");
    }
    // F3: eigenvectors as columns of a 6×6 matrix
    if (eig.valid) {
        write_matrix_csv(out_dir / "wilson_eigenvectors.csv", eig.eigenvectors,
                         "Wilson eigenvectors (cols), sym-part of M, sorted by λ desc");
        std::ofstream ev(out_dir / "wilson_eigenvalues.csv");
        if (ev) {
            ev << "k,lambda,delta_eig,tier_eig\n";
            for (int k = 0; k < kNumOps; ++k) {
                if (k < eig.active_K) {
                    ev << k << "," << eig.eigenvalues[k] << ","
                       << Delta_eig[k] << "," << tier_eig[k] << "\n";
                } else {
                    ev << k << ",null,null,n/a\n";
                }
            }
        }
    }

    {
        std::ofstream f(out_dir / "per_snapshot_moments.csv");
        if (f) {
            f << "snapshot";
            for (int a = 0; a < kNumOps; ++a) f << ",fine_" << kOpNames[a];
            for (int a = 0; a < kNumOps; ++a) f << ",coarse_" << kOpNames[a];
            f << "\n";
            for (size_t i = 0; i < fine_samples.size(); ++i) {
                f << i;
                for (int a = 0; a < kNumOps; ++a) f << "," << fine_samples[i][a];
                for (int a = 0; a < kNumOps; ++a) f << "," << coarse_samples[i][a];
                f << "\n";
            }
        }
    }
    {
        std::ofstream f(out_dir / "eigenvalues.csv");
        if (f) {
            f << "op,eigenvalue,delta_a,naive_delta,tier\n";
            for (int a = 0; a < kNumOps; ++a) {
                f << kOpNames[a] << "," << eig_diag[a] << ","
                  << Delta_a[a] << "," << kNaiveDim[a] << ","
                  << tier[a] << "\n";
            }
        }
    }
    {
        std::ofstream f(out_dir / "meta.json");
        if (f) {
            f << "{\n";
            f << "  \"campaign\": \"operator_mixing_2026-04-26\",\n";
            f << "  \"ledger_row\": \"FTD-0098\",\n";
            f << "  \"protocol\": \"docs/theory/10_eft_program/PROTOCOL_OPERATOR_MIXING_MATRIX.md\",\n";
            f << "  \"mode\": \"" << (smoke_mode ? "smoke" : "production") << "\",\n";
            f << "  \"L\": " << L << ",\n";
            f << "  \"N_BURN\": " << N_BURN << ",\n";
            f << "  \"N_SAMPLES_per_seed\": " << N_SAMPLES << ",\n";
            f << "  \"SAMPLE_STRIDE\": " << SAMPLE_STRIDE << ",\n";
            f << "  \"N_SEEDS\": " << N_SEEDS << ",\n";
            f << "  \"N_total_collected\": " << N_total << ",\n";
            f << "  \"snapshots_dropped\": " << snapshots_dropped << ",\n";
            f << "  \"snapshots_with_nonzero_state\": " << snapshots_with_state << ",\n";
            f << "  \"q_conservation_violations\": " << q_conservation_violations << ",\n";
            f << "  \"cond_S\": " << cond_S << ",\n";
            f << "  \"diagonal_dominant_count\": " << diag_dominant_count << ",\n";
            f << "  \"bootstrap_resamples_succeeded\": " << M_resamples.size() << ",\n";
            f << "  \"bootstrap_resamples_singular\": " << boot_failures << ",\n";
            f << "  \"converged_stderr_entries\": " << converged_entries << ",\n";
            f << "  \"counted_entries\": " << counted_entries << ",\n";
            f << "  \"required_converged\": " << required_converged << ",\n";
            f << "  \"measurement_tag\": \""
              << (measurement_tag ? "MEASUREMENT" : "PARTIAL") << "\",\n";
            f << "  \"diagonal_eigenvalues\": [";
            for (int a = 0; a < kNumOps; ++a) {
                if (std::isfinite(eig_diag[a])) f << eig_diag[a];
                else f << "null";
                f << (a + 1 < kNumOps ? "," : "");
            }
            f << "],\n";
            f << "  \"delta_a_measured\": [";
            for (int a = 0; a < kNumOps; ++a) {
                if (std::isfinite(Delta_a[a])) f << Delta_a[a];
                else f << "null";
                f << (a + 1 < kNumOps ? "," : "");
            }
            f << "],\n";
            f << "  \"tiers\": [";
            for (int a = 0; a < kNumOps; ++a) {
                f << "\"" << tier[a] << "\"" << (a + 1 < kNumOps ? "," : "");
            }
            f << "],\n";
            // F3: Wilson eigendecomposition outputs
            f << "  \"wilson_eigenvalues\": [";
            for (int k = 0; k < kNumOps; ++k) {
                if (std::isfinite(eig.eigenvalues[k])) f << eig.eigenvalues[k];
                else f << "null";
                f << (k + 1 < kNumOps ? "," : "");
            }
            f << "],\n";
            f << "  \"wilson_delta\": [";
            for (int k = 0; k < kNumOps; ++k) {
                if (std::isfinite(Delta_eig[k])) f << Delta_eig[k];
                else f << "null";
                f << (k + 1 < kNumOps ? "," : "");
            }
            f << "],\n";
            f << "  \"wilson_tiers\": [";
            for (int k = 0; k < kNumOps; ++k) {
                f << "\"" << tier_eig[k] << "\"" << (k + 1 < kNumOps ? "," : "");
            }
            f << "],\n";
            // F5: RG semigroup test outputs
            f << "  \"include_b4\": " << (include_b4 ? "true" : "false") << ",\n";
            if (include_b4) {
                f << "  \"semigroup_test_ran\": " << (semigroup_ok ? "true" : "false") << ",\n";
                if (semigroup_ok) {
                    f << "  \"semigroup_max_relerr\": " << semigroup_max_relerr << ",\n";
                    f << "  \"semigroup_verdict\": \""
                      << (semigroup_max_relerr < 0.5 ? "PASS" : "FAIL")
                      << "\"\n";
                } else {
                    f << "  \"semigroup_max_relerr\": null,\n";
                    f << "  \"semigroup_verdict\": \"singular\"\n";
                }
            } else {
                f << "  \"semigroup_test_ran\": false\n";
            }
            f << "}\n";
        }
    }

    std::cout << "\n  artifacts → " << out_dir.string() << "\n";

    if (g_failures > 0) {
        std::cout << "\n  " << g_failures << " gate(s) failed.\n";
        return 4;
    }
    std::cout << "\n  all gates PASS.\n";
    return 0;
}

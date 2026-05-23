#pragma once
/**
 * branch_holonomy.h — signed difference operator and Z_2 torus branch twists.
 *
 * SCOPE
 * ─────────────────────────────────────────────────────────────────────────
 * Theorem-level header-only primitive. Does NOT touch RenderBridge physics
 * or the golden-tick gate. Provides the signed Laplacian for a Z_2 line
 * bundle on a discrete graph and the closed-form spectrum for the simplest
 * non-trivial holonomy class: a single Z_2 twist around a periodic ring.
 *
 * DEFINITIONS
 * ─────────────────────────────────────────────────────────────────────────
 * Given a graph with vertices i and oriented edges e: i → j, attach an edge
 * sign σ_e ∈ {+1, −1}. The signed difference operator D_σ maps vertex
 * functions to edge functions by
 *
 *     (D_σ U)_e  =  U_j  −  σ_e · U_i              (eq. 1)
 *
 * Its associated signed Laplacian is L_σ = D_σ^T D_σ. L_σ is symmetric
 * positive-semidefinite. The spectrum of L_σ depends ONLY on the Z_2
 * holonomy
 *
 *     H(C)  =  ∏_{e ∈ C} σ_e   ∈   {+1, −1}        (eq. 2)
 *
 * around each closed cycle C: any two sign configurations with identical
 * cycle holonomies are gauge-equivalent (related by a Z_2 sign change on
 * vertices), giving the same L_σ-spectrum.
 *
 * THEOREM (1D torus, single Z_2 branch twist)
 * ─────────────────────────────────────────────────────────────────────────
 * On a periodic ring of N ≥ 2 sites, let σ be any sign configuration with
 * H_x ≡ ∏_i σ_i = −1 (an odd number of edges flipped). Then
 *
 *     λ_min(L_σ)  =  4 sin²( π / (2N) )            (eq. 3)
 *
 * achieved by a single eigenvector pair at the lowest "half-integer"
 * momentum k = π/N. This is the standard antiperiodic-boundary-condition
 * result: the sign flip shifts Fourier-mode quantisation from integer to
 * half-integer momenta, so the lowest allowed momentum is k = π/N (not 0)
 * and λ = 4 sin²(k/2) = 4 sin²(π/(2N)).
 *
 * Full closed-form spectrum (twisted sector, H_x = −1):
 *     λ_m  =  4 sin²( π (2m+1) / (2N) ),   m = 0, 1, …, N−1.
 *
 * Trivial sector (H_x = +1) for comparison:
 *     λ_k  =  4 sin²( π k / N ),           k = 0, 1, …, N−1.
 *
 * The 3D torus generalises trivially: putting H_x = −1 on the x-cycle of
 * an N×N×N periodic torus quantises k_x to half-integer momenta while k_y,
 * k_z stay integer-periodic, so the 3D Laplacian eigenvalues are
 * λ(k_x,k_y,k_z) = 4 [sin²(k_x/2) + sin²(k_y/2) + sin²(k_z/2)] and
 * λ_min = 4 sin²(π/(2N)) (saturated at k_x = π/N, k_y = k_z = 0). The 1D
 * primitive in this header captures the gap structure; 3D is a corollary.
 *
 * EPISTEMIC TAGS
 * ─────────────────────────────────────────────────────────────────────────
 * Equation 3 is [THEOREM] — a standard fact about the Z_2-twisted Laplacian
 * on a finite cyclic group, equivalent to the antiperiodic-BC spectrum.
 * The supporting verifications in `test_branch_holonomy_gap.cpp` are the
 * constructive proof: build the explicit signed Laplacian and diagonalise
 * (Jacobi) for small N ∈ {4, 8, 16, 32}, compare to closed-form.
 *
 * NO PHYSICS COUPLING
 * ─────────────────────────────────────────────────────────────────────────
 * This header is pure linear algebra on small graphs. It is independent of
 * the flux/state engine, ontic constants, and RenderBridge. Downstream
 * physics work (Z_3 color centre, generation graph, lattice overlays)
 * builds on this primitive but is in separate modules.
 */

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ftd {
namespace branch {

// ---------------------------------------------------------------------------
// SignedRing1D — signed Laplacian on a periodic ring of N sites.
// ---------------------------------------------------------------------------
//
// Convention. Edges are indexed e_i = (i, (i+1) mod N) for i = 0, …, N−1.
// Each edge carries a sign σ_i ∈ {+1, −1}. The signed-difference operator
// D acts as (D U)_e_i = U_{(i+1) mod N} − σ_i · U_i (eq. 1). The signed
// Laplacian L_σ = D^T D is symmetric N×N with
//
//     L_{ii}                =  2,
//     L_{i, (i+1) mod N}    =  L_{(i+1) mod N, i}  =  −σ_i,
//     L_{ij}                =  0        otherwise.
//
// Each site has degree 2 (its two incident edges contribute +1 each to
// the diagonal); each edge's sign appears on its off-diagonal pair.
//
class SignedRing1D {
public:
    // signs.size() == N; each entry must be +1 or −1.
    SignedRing1D(int N, std::vector<int> signs);

    int size() const { return N_; }
    const std::vector<int>& signs() const { return signs_; }

    // Z_2 holonomy around the ring (eq. 2): ∏_i σ_i, in {+1, −1}.
    int holonomy() const;

    // Apply L_σ to U. out is resized to length N; out[i] = (L_σ U)_i.
    void apply(const std::vector<double>& U, std::vector<double>& out) const;

    // Build the explicit N×N signed-Laplacian matrix.
    std::vector<std::vector<double>> build_matrix() const;

private:
    int N_;
    std::vector<int> signs_;  // length N
};

// ---------------------------------------------------------------------------
// Closed-form spectra for the periodic 1D ring (helpers for cross-checks).
// ---------------------------------------------------------------------------

// [THEOREM] Smallest eigenvalue of the Z_2-twisted ring Laplacian (eq. 3):
//   λ_min(N) = 4 sin²( π / (2N) ).
inline double torus_branch_twist_gap_1d(int N) {
    if (N < 2) {
        throw std::invalid_argument("torus_branch_twist_gap_1d: N must be >= 2");
    }
    static constexpr double kPi = 3.14159265358979323846;
    const double s = std::sin(kPi / (2.0 * static_cast<double>(N)));
    return 4.0 * s * s;
}

// [THEOREM] Full twisted-sector spectrum (H_x = −1):
//   λ_m = 4 sin²( π (2m+1) / (2N) ),   m = 0, …, N−1.
inline std::vector<double> twisted_ring_spectrum_closed_form(int N) {
    if (N < 1) {
        throw std::invalid_argument("twisted_ring_spectrum_closed_form: N >= 1");
    }
    static constexpr double kPi = 3.14159265358979323846;
    std::vector<double> out;
    out.reserve(static_cast<std::size_t>(N));
    for (int m = 0; m < N; ++m) {
        const double k = kPi * (2.0 * m + 1.0) / (2.0 * static_cast<double>(N));
        const double s = std::sin(k);
        out.push_back(4.0 * s * s);
    }
    return out;
}

// [THEOREM] Full trivial-sector spectrum (H_x = +1):
//   λ_k = 4 sin²( π k / N ),   k = 0, …, N−1.
inline std::vector<double> trivial_ring_spectrum_closed_form(int N) {
    if (N < 1) {
        throw std::invalid_argument("trivial_ring_spectrum_closed_form: N >= 1");
    }
    static constexpr double kPi = 3.14159265358979323846;
    std::vector<double> out;
    out.reserve(static_cast<std::size_t>(N));
    for (int k = 0; k < N; ++k) {
        const double theta = kPi * static_cast<double>(k) / static_cast<double>(N);
        const double s = std::sin(theta);
        out.push_back(4.0 * s * s);
    }
    return out;
}

// ---------------------------------------------------------------------------
// Inline implementations.
// ---------------------------------------------------------------------------

inline SignedRing1D::SignedRing1D(int N, std::vector<int> signs)
    : N_(N), signs_(std::move(signs)) {
    if (N_ < 2) {
        throw std::invalid_argument("SignedRing1D: N must be >= 2");
    }
    if (static_cast<int>(signs_.size()) != N_) {
        throw std::invalid_argument("SignedRing1D: signs.size() must equal N");
    }
    for (int s : signs_) {
        if (s != +1 && s != -1) {
            throw std::invalid_argument("SignedRing1D: signs must be +1 or -1");
        }
    }
}

inline int SignedRing1D::holonomy() const {
    int h = 1;
    for (int s : signs_) h *= s;
    return h;
}

inline void SignedRing1D::apply(const std::vector<double>& U,
                                std::vector<double>& out) const {
    if (static_cast<int>(U.size()) != N_) {
        throw std::invalid_argument("SignedRing1D::apply: U.size() must equal N");
    }
    out.assign(static_cast<std::size_t>(N_), 0.0);
    for (int i = 0; i < N_; ++i) {
        const int ip = (i + 1) % N_;
        const int im = (i - 1 + N_) % N_;
        // Edge i_minus (e_{i-1}) connects i-1 ↔ i with sign σ_{i-1}.
        // Edge i (e_i) connects i ↔ i+1 with sign σ_i.
        const double sigma_im = static_cast<double>(signs_[im]);
        const double sigma_i  = static_cast<double>(signs_[i]);
        out[static_cast<std::size_t>(i)] =
            2.0 * U[static_cast<std::size_t>(i)]
            - sigma_im * U[static_cast<std::size_t>(im)]
            - sigma_i  * U[static_cast<std::size_t>(ip)];
    }
}

inline std::vector<std::vector<double>> SignedRing1D::build_matrix() const {
    std::vector<std::vector<double>> M(
        static_cast<std::size_t>(N_),
        std::vector<double>(static_cast<std::size_t>(N_), 0.0));
    for (int i = 0; i < N_; ++i) {
        const int ip = (i + 1) % N_;
        const double sigma_i = static_cast<double>(signs_[i]);
        // Edge i contributes: +1 to diag(i), +1 to diag(ip), −σ_i to (i,ip) pair.
        M[static_cast<std::size_t>(i)][static_cast<std::size_t>(i)]   += 1.0;
        M[static_cast<std::size_t>(ip)][static_cast<std::size_t>(ip)] += 1.0;
        M[static_cast<std::size_t>(i)][static_cast<std::size_t>(ip)]  -= sigma_i;
        M[static_cast<std::size_t>(ip)][static_cast<std::size_t>(i)]  -= sigma_i;
    }
    return M;
}

}  // namespace branch
}  // namespace ftd

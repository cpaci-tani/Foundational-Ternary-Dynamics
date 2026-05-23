#pragma once
/**
 * color_center.h — Z_3 color-center charges and the center projector.
 *
 * SCOPE
 * ─────────────────────────────────────────────────────────────────────────
 * Pure header-only mathematics. No engine state, no `RenderBridge` touch,
 * no physics-toggle wiring. The center-closure theorem is elementary
 * finite group theory on `ℤ_3 = ℤ/3ℤ`; the test is a constructive
 * verification at the integer-arithmetic and matrix-arithmetic levels.
 *
 * THEOREM (Z_3 center closure).
 * ─────────────────────────────────────────────────────────────────────────
 * For a system of N constituents carrying Z_3 color charges
 * `c_1, …, c_N ∈ {0, 1, 2}` (fundamental rep: `0` = singlet, `1` = quark
 * `q`, `2` = anti-quark `q̄`), the state is in the Z_3-trivial
 * (center-closed) sub-sector iff
 *
 *     ∑ c_i  ≡  0   (mod 3).                              (eq. 1)
 *
 * Closed configurations include:
 *   - meson      (q + q̄)                      :  1 + 2  =  3 ≡ 0       ✓
 *   - baryon     (q + q + q)                   :  1+1+1  =  3 ≡ 0       ✓
 *   - anti-baryon (q̄ + q̄ + q̄)                :  2+2+2  =  6 ≡ 0       ✓
 *   - tetraquark (q + q + q̄ + q̄)             :  1+1+2+2 = 6 ≡ 0       ✓
 *   - pentaquark (q+q+q+q + q̄)                :  1·4+2  = 6 ≡ 0       ✓
 *
 * Non-closed (forbidden by the Z_3 center, i.e. confined):
 *   - single quark (q)                         :  1       ≢ 0           ✗
 *   - diquark      (q + q)                     :  1+1 = 2 ≢ 0           ✗
 *   - antiquark    (q̄)                        :  2       ≢ 0           ✗
 *
 * Equivalently, the **center projector**
 *
 *     P_0  =  (1/3) · (I + Z + Z²)                        [THEOREM, eq. 2]
 *
 * with `Z = diag(1, ω, ω²)`, `ω = e^{2πi/3}`, projects onto the c = 0
 * subspace. By the cyclic sum identity 1 + ω + ω² = 0 we get
 * `P_0 = diag(1, 0, 0)`; direct matrix arithmetic then confirms
 *
 *     P_0²  =  P_0    (idempotent),      tr(P_0)  =  1    (rank 1).
 *
 * CANDIDATE PRINCIPLE — NOT a theorem, NOT tested here.
 * ─────────────────────────────────────────────────────────────────────────
 * "Open-flux penalty": non-center-closed configurations (single quark,
 * diquark, etc.) are assumed to carry an energy cost ∝ (∑c_i mod 3 ≠ 0).
 * In FTD this is a `[CANDIDATE PRINCIPLE]` — the closure THEOREM above
 * forbids the state, but the *energetics* of forbidding it (a
 * confinement-style linear or quadratic penalty) is conjectural and is
 * not addressed by this header. The LEDGER lists it as
 * `[CANDIDATE PRINCIPLE]` only.
 *
 * EPISTEMIC TAGS
 * ─────────────────────────────────────────────────────────────────────────
 * Equations 1 (closure characterisation) and 2 (the projector identity)
 * are `[THEOREM]` from finite-group theory + linear algebra. The
 * `test_z3_color_center.cpp` constructive verification realises both at
 * machine precision.
 */

#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <initializer_list>

namespace ftd {
namespace color {

// ---------------------------------------------------------------------------
// Z3Charge — an element of ℤ_3 = {0, 1, 2} under addition mod 3.
// ---------------------------------------------------------------------------
class Z3Charge {
public:
    constexpr Z3Charge() = default;
    constexpr explicit Z3Charge(int c) : c_(canonicalise(c)) {}

    constexpr int value() const { return c_; }

    constexpr Z3Charge operator+(Z3Charge rhs) const {
        return Z3Charge(c_ + rhs.c_);
    }
    constexpr Z3Charge operator-() const { return Z3Charge(-c_); }
    constexpr bool operator==(Z3Charge rhs) const { return c_ == rhs.c_; }
    constexpr bool operator!=(Z3Charge rhs) const { return c_ != rhs.c_; }

    // Common shortcuts.
    static constexpr Z3Charge neutral()   { return Z3Charge(0); }  // singlet
    static constexpr Z3Charge color()     { return Z3Charge(1); }  // q
    static constexpr Z3Charge anticolor() { return Z3Charge(2); }  // q̄  (= −q)

private:
    static constexpr int canonicalise(int c) { return ((c % 3) + 3) % 3; }
    int c_ = 0;
};

// ---------------------------------------------------------------------------
// Center-closure check: ∑ c_i ≡ 0 (mod 3).   (eq. 1)
// ---------------------------------------------------------------------------
template <typename It>
inline bool is_center_closed(It begin, It end) {
    int s = 0;
    for (It it = begin; it != end; ++it) {
        s = (s + it->value()) % 3;
    }
    return s == 0;
}

inline bool is_center_closed(std::initializer_list<Z3Charge> charges) {
    return is_center_closed(charges.begin(), charges.end());
}

// ---------------------------------------------------------------------------
// Z_3 generator ω and fundamental-rep matrix Z = diag(1, ω, ω²).
// ---------------------------------------------------------------------------
namespace detail {
inline constexpr double kPi = 3.14159265358979323846;
}

// ω = e^{2πi/3}.
inline std::complex<double> z3_omega() {
    return std::polar(1.0, 2.0 * detail::kPi / 3.0);
}

using ComplexMatrix3 = std::array<std::array<std::complex<double>, 3>, 3>;

inline ComplexMatrix3 identity3() {
    ComplexMatrix3 m{};
    for (std::size_t i = 0; i < 3; ++i) m[i][i] = 1.0;
    return m;
}

// Z = diag(1, ω, ω²) — fundamental rep of the Z_3 generator.
inline ComplexMatrix3 z3_generator_z() {
    const std::complex<double> w = z3_omega();
    ComplexMatrix3 m{};
    m[0][0] = 1.0;
    m[1][1] = w;
    m[2][2] = w * w;
    return m;
}

inline ComplexMatrix3 matrix_add3(const ComplexMatrix3& A, const ComplexMatrix3& B) {
    ComplexMatrix3 C{};
    for (std::size_t i = 0; i < 3; ++i)
        for (std::size_t j = 0; j < 3; ++j)
            C[i][j] = A[i][j] + B[i][j];
    return C;
}

inline ComplexMatrix3 matrix_scale3(const ComplexMatrix3& A, std::complex<double> alpha) {
    ComplexMatrix3 C{};
    for (std::size_t i = 0; i < 3; ++i)
        for (std::size_t j = 0; j < 3; ++j)
            C[i][j] = alpha * A[i][j];
    return C;
}

inline ComplexMatrix3 matrix_multiply3(const ComplexMatrix3& A, const ComplexMatrix3& B) {
    ComplexMatrix3 C{};
    for (std::size_t i = 0; i < 3; ++i)
        for (std::size_t j = 0; j < 3; ++j) {
            std::complex<double> s{0.0, 0.0};
            for (std::size_t k = 0; k < 3; ++k) s += A[i][k] * B[k][j];
            C[i][j] = s;
        }
    return C;
}

inline bool matrix_close3(const ComplexMatrix3& A, const ComplexMatrix3& B, double tol) {
    for (std::size_t i = 0; i < 3; ++i)
        for (std::size_t j = 0; j < 3; ++j) {
            if (std::abs(A[i][j] - B[i][j]) > tol) return false;
        }
    return true;
}

inline std::complex<double> trace3(const ComplexMatrix3& A) {
    return A[0][0] + A[1][1] + A[2][2];
}

// ---------------------------------------------------------------------------
// Center projector P_0 = (1/3)(I + Z + Z²).   (eq. 2)
// ---------------------------------------------------------------------------
// In the fundamental rep this evaluates to diag(1, 0, 0); a unit projector
// onto the c = 0 (Z_3-trivial) subspace.
inline ComplexMatrix3 center_projector_p0() {
    const ComplexMatrix3 I  = identity3();
    const ComplexMatrix3 Z  = z3_generator_z();
    const ComplexMatrix3 Z2 = matrix_multiply3(Z, Z);
    const ComplexMatrix3 sum = matrix_add3(matrix_add3(I, Z), Z2);
    return matrix_scale3(sum, 1.0 / 3.0);
}

}  // namespace color
}  // namespace ftd

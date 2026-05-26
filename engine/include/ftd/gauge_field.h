#pragma once
/**
 * @file engine/include/ftd/gauge_field.h
 * @purpose Declarations of edge-based SU(2) and SU(3) link variable structures
 * for non-Abelian gauge field simulations (Scale 0 upgrades).
 */

#include <complex>

namespace ftd {

/**
 * @struct SU2Link
 * Represents a link variable in SU(2) on the lattice edges.
 * Standard representation: U = [[a, b], [-b*, a*]] with |a|^2 + |b|^2 = 1.
 */
struct SU2Link {
    std::complex<double> a;
    std::complex<double> b;

    SU2Link() : a(1.0, 0.0), b(0.0, 0.0) {}
    SU2Link(std::complex<double> val_a, std::complex<double> val_b) : a(val_a), b(val_b) {}

    // Enforce SU(2) unitarity constraints
    void normalize() {
        double mag = std::sqrt(std::norm(a) + std::norm(b));
        if (mag > 1e-12) {
            a /= mag;
            b /= mag;
        } else {
            a = {1.0, 0.0};
            b = {0.0, 0.0};
        }
    }
};

/**
 * @struct SU3Link
 * Represents a link variable in SU(3) on the lattice edges.
 * 3x3 complex matrix satisfying U^dagger * U = I and det(U) = 1.
 */
struct SU3Link {
    std::complex<double> m[3][3];

    SU3Link() {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                m[i][j] = (i == j) ? std::complex<double>(1.0, 0.0) : std::complex<double>(0.0, 0.0);
            }
        }
    }
};

} // namespace ftd

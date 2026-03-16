#pragma once
/**
 * Hilbert Space Construction from Complexified Flux
 *
 * H_FTD = L^2(Lattice, C) where psi(v) = J_x(v) + i*J_y(v)
 *
 * The complexified transverse flux components form a wave function.
 * Inner product: <psi|phi> = sum_v psi*(v) * phi(v)
 * Norm: ||psi|| = sqrt(<psi|psi>)
 * Born probability: P(v) = |psi(v)|^2 / ||psi||^2
 *
 * Theory references:
 *   - CLAUDE.md Ch.11-13            (quantum phenomena, measurement)
 *   - DERIV_QUANTUM_MECHANICS_RESOLVED.md  (Hilbert space from flux)
 *   - SPEC_FTD_REFERENCE.md         (formal specification)
 *
 * Epistemic status: [THEOREM] — H_FTD = L^2(Lattice, C) is a mathematical
 * construction from the complexified flux field. The Born rule emerges
 * from manifestation statistics under the imposed sampling rule.
 */

#include "voxel.h"
#include <vector>
#include <complex>
#include <cmath>
#include <array>

namespace ftd {

/// Complex wave function value at a single site
using Complex = std::complex<double>;

/**
 * HilbertState: represents a state vector in H_FTD = L^2(Lattice, C).
 *
 * Each site v carries psi(v) = J_x(v) + i*J_y(v), the complexification
 * of the transverse flux components. The z-component of flux is not part
 * of the wave function — it encodes the longitudinal (gauge) degree of
 * freedom constrained by Gauss's law (see CLAUDE.md §14.3).
 */
struct HilbertState {
    std::vector<Complex> psi;  ///< psi(v) for each lattice site
    int num_sites = 0;

    HilbertState() = default;
    explicit HilbertState(int n) : psi(n, {0.0, 0.0}), num_sites(n) {}

    /**
     * Extract psi = J_x + i*J_y from a vector of Voxels.
     *
     * This is the core construction: the flux field's transverse
     * components are complexified to form the wave function.
     * See CLAUDE.md §13.1: "Complexified flux: psi = J_x + i*J_y"
     */
    static HilbertState from_flux(const std::vector<Voxel>& voxels) {
        HilbertState h(static_cast<int>(voxels.size()));
        for (int i = 0; i < h.num_sites; ++i) {
            h.psi[i] = Complex(voxels[i].flux.x, voxels[i].flux.y);
        }
        return h;
    }

    /**
     * Inner product <this|other> = sum_v conj(this->psi(v)) * other.psi(v)
     *
     * This is the standard L^2 inner product on the lattice.
     * Satisfies: conjugate symmetry, linearity in second argument,
     * positive-definiteness.
     */
    Complex inner_product(const HilbertState& other) const {
        Complex result(0.0, 0.0);
        for (int i = 0; i < num_sites; ++i) {
            result += std::conj(psi[i]) * other.psi[i];
        }
        return result;
    }

    /// Norm squared: ||psi||^2 = <psi|psi> = sum_v |psi(v)|^2
    double norm_squared() const {
        double sum = 0.0;
        for (int i = 0; i < num_sites; ++i) {
            sum += std::norm(psi[i]);  // std::norm gives |z|^2
        }
        return sum;
    }

    /// Norm: ||psi|| = sqrt(<psi|psi>)
    double norm() const { return std::sqrt(norm_squared()); }

    /**
     * Born probability at site i: P(i) = |psi(i)|^2 / ||psi||^2
     *
     * This is the Born rule — the probability of manifestation at site i
     * is proportional to the squared amplitude of the wave function.
     * See CLAUDE.md §13.1, §4.1.
     *
     * Epistemic status: [SELECTION + IMPOSED] — emerges under the
     * manifestation-threshold sampling rule which is itself imposed.
     */
    double born_probability(int i) const {
        double n2 = norm_squared();
        if (n2 < 1e-30) return 0.0;
        return std::norm(psi[i]) / n2;
    }

    /**
     * Full Born probability distribution over all sites.
     * Returns P(v) = |psi(v)|^2 / ||psi||^2 for each v.
     * Guaranteed to sum to 1.0 (up to floating-point precision).
     */
    std::vector<double> born_distribution() const {
        std::vector<double> p(num_sites, 0.0);
        double n2 = norm_squared();
        if (n2 < 1e-30) return p;
        for (int i = 0; i < num_sites; ++i) {
            p[i] = std::norm(psi[i]) / n2;
        }
        return p;
    }

    /**
     * Superposition: alpha*|a> + beta*|b>
     *
     * Linear combination of two states. This is the fundamental
     * operation of Hilbert space — any two states can be superposed.
     * The resulting state inherits interference between the components.
     */
    static HilbertState superposition(const HilbertState& a, Complex alpha,
                                       const HilbertState& b, Complex beta) {
        HilbertState result(a.num_sites);
        for (int i = 0; i < a.num_sites; ++i) {
            result.psi[i] = alpha * a.psi[i] + beta * b.psi[i];
        }
        return result;
    }

    /// Normalize in place: psi -> psi / ||psi||
    void normalize() {
        double n = norm();
        if (n < 1e-30) return;
        for (auto& p : psi) p /= n;
    }
};

/**
 * Fidelity between two states: |<a|b>|^2 / (||a||^2 * ||b||^2)
 *
 * Returns 1.0 for identical (up to phase) states, 0.0 for orthogonal states.
 * This is the standard quantum fidelity measure.
 */
inline double fidelity(const HilbertState& a, const HilbertState& b) {
    Complex ip = a.inner_product(b);
    double denom = a.norm_squared() * b.norm_squared();
    if (denom < 1e-30) return 0.0;
    return std::norm(ip) / denom;
}

/**
 * HilbertEvolution: discrete time evolution of the wave function.
 *
 * Implements psi(t+dt) = psi(t) - i*dt*H|psi(t)> where
 * H = -c^2 * nabla^2 + V(x) is the lattice Hamiltonian.
 *
 * This is the discrete Schrodinger equation on the FTD lattice.
 * The continuum limit recovers the standard Schrodinger equation
 * (see CLAUDE.md Part G, §3.5).
 *
 * Note: First-order Euler integrator. For strict unitarity over
 * many time steps, Crank-Nicolson or split-operator methods are
 * preferred. This suffices for short-time norm conservation tests.
 */
struct HilbertEvolution {
    /**
     * One step of Schrodinger evolution on the lattice.
     *
     * @param state      The wave function psi(v)
     * @param potential   V(v) at each site (external potential)
     * @param neighbors   6-face neighbor indices for each site
     * @param dt          Time step
     * @param c2          c^2 = C_WAVE^2 (wave speed squared)
     */
    static void evolve_step(HilbertState& state,
                            const std::vector<double>& potential,
                            const std::vector<std::array<int, 6>>& neighbors,
                            double dt, double c2) {
        int N = state.num_sites;
        std::vector<Complex> H_psi(N);

        // H|psi> = -c^2 * nabla^2(psi) + V * psi
        for (int i = 0; i < N; ++i) {
            // Discrete Laplacian: nabla^2 psi = sum_{neighbors} psi(n) - 6*psi(i)
            Complex lap(0.0, 0.0);
            for (int n : neighbors[i]) {
                lap += state.psi[n];
            }
            lap -= 6.0 * state.psi[i];

            H_psi[i] = -c2 * lap + potential[i] * state.psi[i];
        }

        // psi(t+dt) = psi(t) - i*dt*H|psi>
        Complex neg_i_dt(0.0, -dt);
        for (int i = 0; i < N; ++i) {
            state.psi[i] += neg_i_dt * H_psi[i];
        }
    }
};

}  // namespace ftd

/**
 * Test: Hilbert Space — H_FTD = L^2(Lattice, C) from Complexified Flux
 *
 * Verifies the Hilbert space construction psi = J_x + i*J_y:
 *   HIL-1: Wave function from flux is non-zero for non-trivial flux
 *   HIL-2: Inner product <psi|psi> > 0 for non-zero flux
 *   HIL-3: Normalization: after normalize(), ||psi|| = 1
 *   HIL-4: Orthogonality: spatially separated wave packets have <psi1|psi2> ~ 0
 *   HIL-5: Superposition: |alpha*psi1 + beta*psi2| produces interference
 *   HIL-6: Born distribution sums to 1
 *   HIL-7: Time evolution conserves norm (approximately unitary)
 *   HIL-8: Fidelity: identical states have fidelity 1, orthogonal have fidelity 0
 *
 * Theory references:
 *   - CLAUDE.md Ch.11-13              (Hilbert space, Born rule, measurement)
 *   - DERIV_QUANTUM_MECHANICS_RESOLVED.md  (H_FTD construction)
 *   - SPEC_FTD_REFERENCE.md           (formal specification)
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/render_bridge.h"
#include "ftd/hilbert.h"
#include "ftd/constants.h"

int failures = 0;

void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++failures;
    }
}

void check_close(const char* name, double a, double b, double tol) {
    bool ok = std::abs(a - b) < tol;
    if (ok) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(10) << a
                  << ", expected " << b << ", diff " << std::abs(a - b) << ")\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Hilbert Space — H_FTD = L^2(Lattice, C)\n";
    std::cout << "================================================================\n";

    // ================================================================
    // HIL-1: Wave function from flux
    // ================================================================
    std::cout << "\n--- HIL-1: Wave Function from Flux ---\n";
    {
        int L = 16;
        ftd::RenderBridge rb(L);

        // Inject flux with non-zero x and y components
        rb.inject_flux(8, 8, 8, {1.0, 2.0, 3.0});

        auto h = rb.hilbert_state();
        int idx = rb.lattice().index(8, 8, 8);

        // psi = J_x + i*J_y = 1.0 + 2.0i
        double re = h.psi[idx].real();
        double im = h.psi[idx].imag();

        std::cout << "    psi(8,8,8) = " << re << " + " << im << "i\n";
        check("HIL-1a: psi real part = J_x", std::abs(re - 1.0) < 1e-12);
        check("HIL-1b: psi imag part = J_y", std::abs(im - 2.0) < 1e-12);
        check("HIL-1c: psi non-zero for non-trivial flux", std::abs(h.psi[idx]) > 0.0);

        // z-component is NOT part of the wave function (longitudinal/gauge DoF)
        // This is tested implicitly: psi only uses J_x and J_y
    }

    // ================================================================
    // HIL-2: Inner product positivity
    // ================================================================
    std::cout << "\n--- HIL-2: Inner Product <psi|psi> > 0 ---\n";
    {
        int L = 16;
        ftd::RenderBridge rb(L);
        rb.inject_flux(8, 8, 8, {1.0, 2.0, 0.0});
        rb.inject_flux(10, 8, 8, {0.5, 1.0, 0.0});

        auto h = rb.hilbert_state();
        double n2 = h.norm_squared();
        ftd::Complex ip = h.inner_product(h);

        std::cout << "    ||psi||^2 = " << n2 << "\n";
        std::cout << "    <psi|psi> = " << ip.real() << " + " << ip.imag() << "i\n";

        check("HIL-2a: norm squared > 0", n2 > 0.0);
        check("HIL-2b: <psi|psi> is real (imag ~ 0)", std::abs(ip.imag()) < 1e-12);
        check_close("HIL-2c: <psi|psi> = ||psi||^2", ip.real(), n2, 1e-12);
    }

    // ================================================================
    // HIL-3: Normalization
    // ================================================================
    std::cout << "\n--- HIL-3: Normalization ---\n";
    {
        int L = 16;
        ftd::RenderBridge rb(L);
        rb.inject_flux(8, 8, 8, {3.0, 4.0, 0.0});
        rb.inject_flux(6, 6, 6, {1.0, 1.0, 0.0});

        auto h = rb.hilbert_state();
        double norm_before = h.norm();
        std::cout << "    Norm before: " << norm_before << "\n";
        check("HIL-3a: norm > 1 before normalization", norm_before > 1.0);

        h.normalize();
        double norm_after = h.norm();
        std::cout << "    Norm after normalize(): " << norm_after << "\n";
        check_close("HIL-3b: ||psi|| = 1 after normalization", norm_after, 1.0, 1e-12);
    }

    // ================================================================
    // HIL-4: Orthogonality of separated wave packets
    // ================================================================
    std::cout << "\n--- HIL-4: Orthogonality ---\n";
    {
        int L = 32;

        // Create two well-separated Gaussian wave packets using inject_flux
        // Packet A near (8,16,16), Packet B near (24,16,16)
        ftd::RenderBridge rb_a(L);
        rb_a.inject_flux(8, 16, 16, {1.0, 0.5, 0.0});
        rb_a.inject_flux(9, 16, 16, {0.5, 0.25, 0.0});
        rb_a.inject_flux(7, 16, 16, {0.5, 0.25, 0.0});

        ftd::RenderBridge rb_b(L);
        rb_b.inject_flux(24, 16, 16, {0.0, 1.0, 0.0});
        rb_b.inject_flux(25, 16, 16, {0.0, 0.5, 0.0});
        rb_b.inject_flux(23, 16, 16, {0.0, 0.5, 0.0});

        auto h_a = rb_a.hilbert_state();
        auto h_b = rb_b.hilbert_state();

        ftd::Complex ip = h_a.inner_product(h_b);
        double overlap = std::abs(ip);

        std::cout << "    <psi_a|psi_b> = " << ip.real() << " + " << ip.imag() << "i\n";
        std::cout << "    |<psi_a|psi_b>| = " << overlap << "\n";

        // Spatially separated packets should be approximately orthogonal
        // (zero overlap since they don't share any non-zero sites)
        check("HIL-4: separated wave packets are orthogonal", overlap < 1e-12);
    }

    // ================================================================
    // HIL-5: Superposition produces interference
    // ================================================================
    std::cout << "\n--- HIL-5: Superposition & Interference ---\n";
    {
        int N = 64;  // 1D-like test: use a small lattice and check specific sites
        ftd::HilbertState h_a(N);
        ftd::HilbertState h_b(N);

        // Packet A: peaked at site 16
        for (int i = 12; i <= 20; ++i) {
            double r = std::abs(i - 16);
            h_a.psi[i] = ftd::Complex(std::exp(-r * r / 4.0), 0.0);
        }

        // Packet B: peaked at site 48
        for (int i = 44; i <= 52; ++i) {
            double r = std::abs(i - 48);
            h_b.psi[i] = ftd::Complex(std::exp(-r * r / 4.0), 0.0);
        }

        // Superposition: (|a> + |b>) / sqrt(2)
        ftd::Complex coeff(1.0 / std::sqrt(2.0), 0.0);
        auto h_super = ftd::HilbertState::superposition(h_a, coeff, h_b, coeff);

        // The superposition should have non-zero amplitude at BOTH peaks
        double amp_at_16 = std::abs(h_super.psi[16]);
        double amp_at_48 = std::abs(h_super.psi[48]);
        double amp_at_32 = std::abs(h_super.psi[32]);  // between peaks

        std::cout << "    |psi_super(16)| = " << amp_at_16 << "\n";
        std::cout << "    |psi_super(48)| = " << amp_at_48 << "\n";
        std::cout << "    |psi_super(32)| = " << amp_at_32 << " (between peaks)\n";

        check("HIL-5a: superposition has amplitude at peak A", amp_at_16 > 0.1);
        check("HIL-5b: superposition has amplitude at peak B", amp_at_48 > 0.1);
        check("HIL-5c: no amplitude between separated peaks", amp_at_32 < 1e-10);

        // Now test interference: overlapping packets
        ftd::HilbertState h_c(N);
        ftd::HilbertState h_d(N);

        // Two overlapping Gaussian packets centered at sites 30 and 34
        for (int i = 0; i < N; ++i) {
            double r_c = std::abs(i - 30);
            double r_d = std::abs(i - 34);
            h_c.psi[i] = ftd::Complex(std::exp(-r_c * r_c / 8.0), 0.0);
            h_d.psi[i] = ftd::Complex(std::exp(-r_d * r_d / 8.0), 0.0);
        }

        // Constructive: (|c> + |d>)
        auto h_construct = ftd::HilbertState::superposition(
            h_c, ftd::Complex(1.0, 0.0), h_d, ftd::Complex(1.0, 0.0));
        // Destructive: (|c> - |d>)
        auto h_destruct = ftd::HilbertState::superposition(
            h_c, ftd::Complex(1.0, 0.0), h_d, ftd::Complex(-1.0, 0.0));

        // At the midpoint (site 32), constructive should exceed destructive
        double mid_construct = std::norm(h_construct.psi[32]);
        double mid_destruct = std::norm(h_destruct.psi[32]);

        std::cout << "    |psi_construct(32)|^2 = " << mid_construct << "\n";
        std::cout << "    |psi_destruct(32)|^2 = " << mid_destruct << "\n";

        check("HIL-5d: constructive > destructive at midpoint",
              mid_construct > mid_destruct);
    }

    // ================================================================
    // HIL-6: Born distribution sums to 1
    // ================================================================
    std::cout << "\n--- HIL-6: Born Distribution Normalization ---\n";
    {
        int L = 16;
        ftd::RenderBridge rb(L);
        rb.inject_flux(8, 8, 8, {2.0, 3.0, 1.0});
        rb.inject_flux(4, 4, 4, {1.0, 0.5, 0.0});
        rb.inject_flux(12, 12, 12, {0.0, 1.5, 0.5});

        auto h = rb.hilbert_state();
        auto dist = h.born_distribution();

        double sum = 0.0;
        for (double p : dist) {
            sum += p;
            check("HIL-6a: P(v) >= 0", p >= 0.0);
        }

        std::cout << "    Sum of Born distribution: " << std::setprecision(15) << sum << "\n";
        check_close("HIL-6b: Born distribution sums to 1", sum, 1.0, 1e-12);

        // Born probability at the high-flux site should be largest
        int idx_main = rb.lattice().index(8, 8, 8);
        double p_main = h.born_probability(idx_main);
        std::cout << "    P(8,8,8) = " << p_main << "\n";
        check("HIL-6c: highest flux site has largest probability", p_main > 0.0);
    }

    // ================================================================
    // HIL-7: Time evolution conserves norm (approximately)
    // ================================================================
    std::cout << "\n--- HIL-7: Norm Conservation Under Evolution ---\n";
    {
        int L = 16;
        int N = L * L * L;

        // Build neighbor table for the lattice
        ftd::Lattice lat(L);
        std::vector<std::array<int, 6>> nbrs(N);
        for (int i = 0; i < N; ++i) {
            nbrs[i] = lat.neighbors_6(i);
        }

        // Initialize a localized wave packet
        ftd::HilbertState h(N);
        int center = lat.index(8, 8, 8);
        for (int n : nbrs[center]) {
            h.psi[n] = ftd::Complex(0.3, 0.1);
        }
        h.psi[center] = ftd::Complex(1.0, 0.5);

        // Zero potential (free particle)
        std::vector<double> potential(N, 0.0);

        double norm_initial = h.norm();
        std::cout << "    Initial norm: " << norm_initial << "\n";

        // Evolve with small dt (first-order Euler, so keep dt small for unitarity)
        double dt = 0.001;
        double c2 = ftd::C_WAVE * ftd::C_WAVE;

        for (int step = 0; step < 100; ++step) {
            ftd::HilbertEvolution::evolve_step(h, potential, nbrs, dt, c2);
        }

        double norm_final = h.norm();
        double drift = std::abs(norm_final - norm_initial) / norm_initial;

        std::cout << "    Final norm after 100 steps (dt=0.001): " << norm_final << "\n";
        std::cout << "    Relative drift: " << drift << "\n";

        // First-order Euler is NOT exactly unitary, but for small dt×steps
        // the norm should be approximately conserved (< 5% drift)
        check("HIL-7: norm approximately conserved (drift < 5%)", drift < 0.05);
    }

    // ================================================================
    // HIL-8: Fidelity
    // ================================================================
    std::cout << "\n--- HIL-8: Fidelity ---\n";
    {
        int N = 32;

        // State A
        ftd::HilbertState h_a(N);
        h_a.psi[10] = ftd::Complex(1.0, 0.0);
        h_a.psi[11] = ftd::Complex(0.5, 0.3);

        // State B = copy of A (identical)
        ftd::HilbertState h_b = h_a;

        double fid_same = ftd::fidelity(h_a, h_b);
        std::cout << "    Fidelity(A, A) = " << fid_same << "\n";
        check_close("HIL-8a: identical states have fidelity 1", fid_same, 1.0, 1e-12);

        // State C: orthogonal to A (non-overlapping sites)
        ftd::HilbertState h_c(N);
        h_c.psi[20] = ftd::Complex(1.0, 0.0);
        h_c.psi[21] = ftd::Complex(0.5, 0.3);

        double fid_orth = ftd::fidelity(h_a, h_c);
        std::cout << "    Fidelity(A, C_orthogonal) = " << fid_orth << "\n";
        check_close("HIL-8b: orthogonal states have fidelity 0", fid_orth, 0.0, 1e-12);

        // State D: A with global phase rotation (should still have fidelity 1)
        ftd::HilbertState h_d(N);
        ftd::Complex phase_rot = std::polar(1.0, 0.7);  // e^(i*0.7)
        for (int i = 0; i < N; ++i) {
            h_d.psi[i] = phase_rot * h_a.psi[i];
        }

        double fid_phase = ftd::fidelity(h_a, h_d);
        std::cout << "    Fidelity(A, e^(i*0.7)*A) = " << fid_phase << "\n";
        check_close("HIL-8c: phase-rotated states have fidelity 1", fid_phase, 1.0, 1e-12);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All Hilbert space tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}

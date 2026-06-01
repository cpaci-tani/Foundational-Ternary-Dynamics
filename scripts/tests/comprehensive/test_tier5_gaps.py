"""
TIER 5: Critical Gap Attack (Weight: 15%)

Explicitly test the weakest points of FTD. These are the
make-or-break tests that address the central open questions.

Failures here mean the critical gaps remain open and the
framework cannot be called complete.
"""

import pytest
import numpy as np
import sys
import os

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..")))

# ternary_matrix is the archived Python engine (replaced by C++ engine).
# Tests that require it are skipped when unavailable.
try:
    import ternary_matrix.config as cfg
    from ternary_matrix.model.grid import Universe
    from ternary_matrix.physics import (
        tick,
        propagate_flux,
        calculate_density,
    )

    HAS_TERNARY_MATRIX = True
except ImportError:
    HAS_TERNARY_MATRIX = False

from .ftd_test_utils import CODATA, percent_error
from .test_tier3_predictions import compute_ftd_values

try:
    import mpmath

    mpmath.mp.dps = 50
    HAS_MPMATH = True
except ImportError:
    HAS_MPMATH = False


# =============================================================================
# Test 5.1: Bell Inequality from Pure Lattice
# =============================================================================


class TestBellInequality:
    """Test CHSH inequality from pure lattice dynamics."""

    def test_classical_bound(self):
        """Pure local deterministic model gives S <= 2."""
        np.random.seed(42)
        n_trials = 10000

        lambdas = np.random.uniform(0, 2 * np.pi, n_trials)

        a1, a2 = 0, np.pi / 4
        b1, b2 = np.pi / 8, 3 * np.pi / 8

        def outcome(angle, setting):
            return 1 if np.cos(angle - setting) > 0 else -1

        E = {}
        for a, a_name in [(a1, "a1"), (a2, "a2")]:
            for b, b_name in [(b1, "b1"), (b2, "b2")]:
                products = np.array([outcome(l, a) * outcome(l, b) for l in lambdas])
                E[(a_name, b_name)] = np.mean(products)

        S = abs(E[("a1", "b1")] - E[("a1", "b2")] + E[("a2", "b1")] + E[("a2", "b2")])

        print(f"\n  CHSH S value (local deterministic): {S:.4f}")
        print("  Classical bound: S <= 2.0")
        print("  Quantum bound:   S <= 2*sqrt(2) = 2.828")

        assert S <= 2.0 + 0.1, f"S = {S:.4f} > 2 (impossible for local model!)"

    def test_quantum_correlator(self):
        """Quantum spin-1/2 CHSH gives S = 2*sqrt(2) via correlator formula."""
        # For singlet state: E(a,b) = -cos(a-b) where a,b are measurement directions
        # Optimal CHSH angles: a1=0, a2=pi/2, b1=pi/4, b2=3pi/4
        a1, a2 = 0, np.pi / 2
        b1, b2 = np.pi / 4, 3 * np.pi / 4

        def E(a, b):
            return -np.cos(a - b)

        S = abs(E(a1, b1) - E(a1, b2) + E(a2, b1) + E(a2, b2))

        print(f"\n  CHSH S value (quantum correlator, optimal angles): {S:.4f}")
        print(f"  Expected: 2*sqrt(2) = {2*np.sqrt(2):.4f}")

        assert abs(S - 2 * np.sqrt(2)) < 0.01, f"S = {S:.4f} != 2*sqrt(2)"
        print("  NOTE: This tests the IMPOSED Hilbert space formula, not lattice emergence.")

    def test_gap_documentation(self):
        """Document the Bell gap: substrate S<=2, quantum S=2sqrt(2)."""
        substrate_S = 2.0
        quantum_S = 2 * np.sqrt(2)
        gap = quantum_S - substrate_S

        print("\n  BELL GAP:")
        print(f"    Substrate maximum:  S = {substrate_S:.4f}")
        print(f"    Quantum observed:   S = {quantum_S:.4f}")
        print(f"    Gap:                delta_S = {gap:.4f}")
        print("    Status: OPEN -- No mechanism demonstrated to bridge this gap")
        print("    The substrate-to-aggregate transition remains the central")
        print("    open question in FTD.")


# =============================================================================
# Test 5.2: Born Rule Without Tuning
# =============================================================================


@pytest.mark.skipif(not HAS_TERNARY_MATRIX, reason="ternary_matrix archived; use C++ engine")
class TestBornRule:
    """Test whether Born rule emerges from manifestation dynamics."""

    def test_manifestation_statistics(self):
        """Run many manifestation events and check probability distribution."""
        cfg.CONSTANTS = cfg.PhysicsConfig(GRID_SIZE=16, DAMPING=0.0, C_WAVE=0.3)

        manifestation_counts = np.zeros(16)
        n_trials = 200

        for trial in range(n_trials):
            uni = Universe(16)
            for x in range(16):
                flux_val = 0.1 + 2.0 * x / 15.0
                uni.flux[x, 8, 8] = [flux_val, 0, 0]

            for _ in range(10):
                tick(uni)

            for x in range(16):
                if uni.states[x, 8, 8] != 0:
                    manifestation_counts[x] += 1

        total_manifest = np.sum(manifestation_counts)
        print(f"\n  Total manifestations: {total_manifest}")
        if total_manifest > 10:
            probs = manifestation_counts / max(total_manifest, 1)
            left_half = np.sum(probs[:8])
            right_half = np.sum(probs[8:])
            print(f"  Left half probability:  {left_half:.3f}")
            print(f"  Right half probability: {right_half:.3f}")
            print(f"  Ratio (right/left):     {right_half/max(left_half,1e-10):.2f}")
            if right_half > left_half:
                print("  CONSISTENT with Born rule (higher flux -> more manifestation)")
            else:
                print("  INCONSISTENT with Born rule")
        else:
            print("  Too few manifestations to test Born rule statistics")


# =============================================================================
# Test 5.3: Force Emergence Without Imposed Forms
# =============================================================================


@pytest.mark.skipif(not HAS_TERNARY_MATRIX, reason="ternary_matrix archived; use C++ engine")
class TestForceEmergence:
    """Test whether forces emerge from flux dynamics alone."""

    def test_flux_mediated_interaction(self):
        """Two charged particles interacting via flux alone (no imposed forces)."""
        cfg.CONSTANTS = cfg.PhysicsConfig(
            GRID_SIZE=32,
            DAMPING=0.01,
            C_WAVE=0.4,
            GRAVITY_BIAS=0.0,
            ALPHA=0.0,
            BETA=0.0,
            G_STRONG=0.0,
        )
        uni = Universe(32)

        pos1, pos2 = (10, 16, 16), (22, 16, 16)
        uni.states[pos1] = 1
        uni.states[pos2] = 1
        uni.flux[pos1] = [3.0, 0.0, 0.0]
        uni.flux[pos2] = [3.0, 0.0, 0.0]
        uni.is_locked[pos1] = True
        uni.is_locked[pos2] = True

        initial_flux_at_mid = np.linalg.norm(uni.flux[16, 16, 16])

        for _ in range(50):
            propagate_flux(uni)
            calculate_density(uni)

        final_flux_at_mid = np.linalg.norm(uni.flux[16, 16, 16])

        print("\n  Force emergence test (all explicit forces disabled):")
        print(f"  Flux at midpoint: {initial_flux_at_mid:.4f} -> {final_flux_at_mid:.4f}")
        if final_flux_at_mid > 1e-6:
            print("  Flux propagated between particles (wave-mediated interaction)")
        else:
            print("  No flux reached midpoint (no wave-mediated interaction)")
        print("  NOTE: Wave propagation creates flux gradients, but these only")
        print("  produce forces through the IMPOSED force terms in forces.py.")
        print("  Without imposed forces, particles are inert.")


# =============================================================================
# Test 5.4: Hilbert Space Emergence (Norm Conservation)
# =============================================================================


@pytest.mark.skipif(not HAS_TERNARY_MATRIX, reason="ternary_matrix archived; use C++ engine")
class TestHilbertSpace:
    """Test whether complexified flux behaves like a wave function."""

    def test_norm_conservation_no_damping(self):
        """||psi||^2 should be approximately conserved without damping."""
        cfg.CONSTANTS = cfg.PhysicsConfig(GRID_SIZE=16, DAMPING=0.0, C_WAVE=0.3)
        uni = Universe(16)

        for x in range(4, 12):
            for y in range(4, 12):
                uni.flux[x, y, 8, 0] = np.exp(-0.5 * ((x - 8) ** 2 + (y - 8) ** 2) / 4)
                uni.flux[x, y, 8, 1] = 0.5 * np.exp(-0.5 * ((x - 8) ** 2 + (y - 8) ** 2) / 4)

        def compute_norm_sq(u):
            jx = u.flux[:, :, :, 0]
            jy = u.flux[:, :, :, 1]
            return np.sum(jx**2 + jy**2)

        initial_norm = compute_norm_sq(uni)
        norms = [initial_norm]

        for _ in range(100):
            propagate_flux(uni)
            norms.append(compute_norm_sq(uni))

        final_norm = norms[-1]
        max_deviation = max(abs(n - initial_norm) / max(initial_norm, 1e-10) for n in norms)

        print("\n  Hilbert space norm test (no damping):")
        print(f"  Initial ||psi||^2 = {initial_norm:.6f}")
        print(f"  Final ||psi||^2   = {final_norm:.6f}")
        print(f"  Max deviation     = {max_deviation*100:.2f}%")

        if max_deviation < 0.01:
            print("  EXCELLENT: Norm conserved to < 1% -- consistent with unitarity")
        elif max_deviation < 0.10:
            print("  GOOD: Norm approximately conserved (< 10%)")
        elif max_deviation < 0.50:
            print("  MARGINAL: Significant norm deviation (< 50%)")
        else:
            print("  POOR: Norm NOT conserved -- Hilbert space interpretation doubtful")


# =============================================================================
# Test 5.5: Integer Uniqueness (Exhaustive)
# =============================================================================


class TestIntegerUniquenessExhaustive:
    """Exhaustive search for alternative integer sets."""

    def test_full_chi2_landscape(self):
        """Map the chi^2 landscape over all (N_c, N_base) pairs."""
        baseline = compute_ftd_values(3, 4)
        if baseline is None:
            pytest.skip("Cannot compute baseline")

        baseline_err = 0
        keys = ["alpha_inv", "sin2_theta_w", "m_mu_over_m_e", "m_tau_over_m_e", "delta_CKM", "sin2_theta12_PMNS"]
        for key in keys:
            if key in baseline and key in CODATA:
                baseline_err += percent_error(baseline[key], CODATA[key].value) ** 2

        results = []
        for nc in range(2, 26):
            for nb in range(2, 26):
                ftd = compute_ftd_values(nc, nb)
                if ftd is None:
                    continue
                err = 0
                valid = True
                for key in keys:
                    if key in ftd and key in CODATA:
                        val = ftd[key]
                        if not np.isfinite(val):
                            valid = False
                            break
                        err += percent_error(val, CODATA[key].value) ** 2
                if valid:
                    results.append((nc, nb, err))

        results.sort(key=lambda x: x[2])

        print("\n  INTEGER UNIQUENESS SEARCH (2 <= N_c, N_base <= 25):")
        print(f"  Baseline {{3,4}} error = {baseline_err:.4f}")
        print("  Top 10 results:")
        for i, (nc, nb, err) in enumerate(results[:10]):
            marker = " <-- BASELINE" if nc == 3 and nb == 4 else ""
            ratio = err / max(baseline_err, 1e-10)
            print(f"    #{i+1}: N_c={nc}, N_base={nb}, error={err:.4f} ({ratio:.1f}x){marker}")

        if results[0][0] == 3 and results[0][1] == 4:
            print("  {3,4} IS the global minimum!")
        else:
            best = results[0]
            print(f"  WARNING: {{N_c={best[0]}, N_base={best[1]}}} beats {{3,4}}!")

        n_competitors = sum(1 for _, _, e in results if e < baseline_err * 2 and not (_ == 3))
        print(f"  Competitors within 2x: {n_competitors}")


# =============================================================================
# Test 5.6: Reference frame context Claims Verification
# =============================================================================


class TestReferenceFrameContextVerification:
    """Verify the mathematical claims about reference frame context parameters."""

    def test_reference_frame_context_quadratic_roots(self):
        """Verify complex roots from k=1/2 quadratic."""
        if not HAS_MPMATH:
            pytest.skip("mpmath required")

        g_quarter = mpmath.gamma(mpmath.mpf(1) / 4)
        g_star = mpmath.sqrt(2) * g_quarter**2 / (2 * mpmath.pi)

        a = 1
        b = -(g_star**2) / 2
        c = g_star**3 / 2

        disc = b**2 - 4 * a * c
        print(f"\n  Reference frame context quadratic discriminant: {float(disc):.6f}")
        assert disc < 0, "Discriminant should be negative (complex roots)"

        real_part = -b / (2 * a)
        imag_part = mpmath.sqrt(-disc) / (2 * a)

        K_C = mpmath.sqrt(real_part**2 + imag_part**2)
        theta = mpmath.atan2(imag_part, real_part)
        theta_deg = float(theta * 180 / mpmath.pi)

        print(f"  y = {float(real_part):.4f} +/- {float(imag_part):.4f}i")
        print(f"  |y| = K_C = {float(K_C):.4f}")
        print(f"  Phase = {theta_deg:.2f} degrees")

        assert abs(float(K_C) - 3.5986) < 0.01, f"K_C = {float(K_C)}"
        assert abs(theta_deg - 52.54) < 1.0, f"Phase = {theta_deg} degrees"

        K_C_formula = mpmath.sqrt(g_star**3 / 2)
        assert abs(float(K_C - K_C_formula)) < 1e-10, f"K_C mismatch: {float(K_C)} vs {float(K_C_formula)}"

        print("  K_C = sqrt(G*^3/2) verified")
        print("  NOTE: These are mathematically verified but the INTERPRETATION")
        print("  as reference frame context parameters is [PROPOSED] and untestable.")

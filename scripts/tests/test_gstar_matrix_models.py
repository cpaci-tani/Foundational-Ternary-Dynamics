"""Tests for proof_gstar_matrix_models (FTD-0366).

Thin pytest wrapper around scripts/proofs/proof_gstar_matrix_models.py — the
G* / strongly-coupled matrix-model verification suite (CHPS, Commun. Math.
Phys. 361 (2018) 1235-1274, arXiv:1611.03142).  Fast symbolic core checks run
inline; the full 138-check suite runs once as a subprocess integration test.
"""

import os
import subprocess
import sys

import sympy as sp

PROOFS_DIR = os.path.join(os.path.dirname(__file__), "..", "proofs")
sys.path.insert(0, PROOFS_DIR)

import proof_gstar_matrix_models as gmm  # noqa: E402


def test_amplitude_ratio_is_gstar_at_N1():
    """Gamma-amplitude ratio (4,1)/(4,3) at N=1 equals G* = Gamma(1/4)/Gamma(3/4)."""
    Gs = sp.gamma(sp.Rational(1, 4)) / sp.gamma(sp.Rational(3, 4))
    r1 = sp.cancel(sp.simplify(gmm.amplitude(4, 1, 1) / gmm.amplitude(4, 3, 1)))
    assert sp.simplify(r1 - Gs) == 0


def test_amplitude_product_is_sqrt2_pi_at_N1():
    """Conjugate-sector amplitude product at N=1 = sqrt(2)*pi (Euler reflection)."""
    prod = sp.simplify(gmm.amplitude(4, 1, 1) * gmm.amplitude(4, 3, 1))
    assert sp.simplify(prod - sp.sqrt(2) * sp.pi) == 0


def test_theorem1_full_small_case():
    """det[moments] = delta * amplitude at (r,a,N) = (4,1,4), sign included."""
    det_val = gmm.Z_det(4, 1, 4)
    thm_val = gmm.delta_sign(4, 1, 4) * gmm.amplitude(4, 1, 4)
    assert sp.simplify(det_val - thm_val) == 0


def test_support_rule_off_support():
    """Pure-phase Z vanishes off-support: delta_{4,1}(2) = 0 and det = 0."""
    assert gmm.delta_sign(4, 1, 2) == 0
    assert sp.simplify(gmm.Z_det(4, 1, 2)) == 0


def test_full_suite_subprocess():
    """The complete 138-check suite passes (exit 0)."""
    script = os.path.join(PROOFS_DIR, "proof_gstar_matrix_models.py")
    result = subprocess.run(
        [sys.executable, script], capture_output=True, text=True, timeout=300
    )
    assert result.returncode == 0, result.stdout[-2000:] + result.stderr[-2000:]

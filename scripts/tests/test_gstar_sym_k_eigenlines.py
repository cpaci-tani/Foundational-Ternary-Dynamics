"""Tests for gstar_sym_k_eigenlines module.

Phase 0 of G* opus follow-up — verifies the symmetric period algebra
infrastructure to >=80 digits via mpmath. See companion spec at
docs/superpowers/specs/2026-05-19-gstar-followup-attacks-design.md.
"""

import pytest
import sys
import os

# Make scripts/exploration importable
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'exploration'))


def test_module_imports():
    """Phase 0 sanity: module can be imported and exposes its public API."""
    import gstar_sym_k_eigenlines as gse
    # Public API expected from spec §2:
    assert hasattr(gse, 'sym_k_basis'), "sym_k_basis missing"
    assert hasattr(gse, 'z_i_eigenvalue'), "z_i_eigenvalue missing"
    assert hasattr(gse, 'phi_specialise'), "phi_specialise missing"
    assert hasattr(gse, 'c_invariant_dim'), "c_invariant_dim missing"


def test_identity_I1():
    """I1: Phi(omega^2) = G*^2 * pi, verified to 80 digits.

    Per Convention C2: Phi(omega) = G* * sqrt(pi), so Phi(omega^2) = G*^2 * pi.
    """
    import gstar_sym_k_eigenlines as gse
    import mpmath as mp

    mp.mp.dps = 80
    G_star = mp.gamma(mp.mpf('0.25')) / mp.gamma(mp.mpf('0.75'))
    sqrt_pi = mp.sqrt(mp.pi)

    # Phi(omega^2) via the module
    lhs = gse.phi_specialise(2, 0, G_star, sqrt_pi)

    # Expected: G*^2 * pi
    rhs = G_star**2 * mp.pi

    # Both should agree to 80 digits (allow tolerance 10^-78 for floating slop)
    diff = abs(lhs - rhs)
    assert diff < mp.mpf('1e-78'), f"I1 fails: |lhs - rhs| = {diff}"

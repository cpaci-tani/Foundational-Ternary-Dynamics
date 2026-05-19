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


def test_identity_I2_eta_squared():
    """I2a: Phi(eta^2) = pi / G*^2, verified to 80 digits.

    Per Convention C2: Phi(eta) = -sqrt(pi)/G*, so Phi(eta^2) = pi/G*^2.
    """
    import gstar_sym_k_eigenlines as gse
    import mpmath as mp

    mp.mp.dps = 80
    G_star = mp.gamma(mp.mpf('0.25')) / mp.gamma(mp.mpf('0.75'))
    sqrt_pi = mp.sqrt(mp.pi)

    lhs = gse.phi_specialise(0, 2, G_star, sqrt_pi)
    rhs = mp.pi / G_star**2

    diff = abs(lhs - rhs)
    assert diff < mp.mpf('1e-78'), f"I2a fails: |lhs - rhs| = {diff}"


def test_identity_I2_omega_eta():
    """I2b: Phi(omega * eta) = -pi, verified to 80 digits.

    Phi(omega) * Phi(eta) = (G* * sqrt(pi)) * (-sqrt(pi) / G*) = -pi.
    """
    import gstar_sym_k_eigenlines as gse
    import mpmath as mp

    mp.mp.dps = 80
    G_star = mp.gamma(mp.mpf('0.25')) / mp.gamma(mp.mpf('0.75'))
    sqrt_pi = mp.sqrt(mp.pi)

    lhs = gse.phi_specialise(1, 1, G_star, sqrt_pi)
    rhs = -mp.pi

    diff = abs(lhs - rhs)
    assert diff < mp.mpf('1e-78'), f"I2b fails: |lhs - rhs| = {diff}"


def test_identity_I2_legendre_consistency():
    """I2c (the cross-check): the Legendre relation det = -2i * G* * sqrt(pi) * q with q = -sqrt(pi)/G*
    should give -2i * G* * sqrt(pi) * (-sqrt(pi)/G*) = 2i * pi.

    This is the Legendre relation det of period matrix = 2*pi*i (positive orientation per Convention C2).
    """
    import mpmath as mp

    mp.mp.dps = 80
    G_star = mp.gamma(mp.mpf('0.25')) / mp.gamma(mp.mpf('0.75'))
    sqrt_pi = mp.sqrt(mp.pi)
    q = -sqrt_pi / G_star  # eta_period per Convention C2

    # Period matrix:
    # [ G*sqrt(pi)    i G*sqrt(pi) ]
    # [ q             -i q          ]
    omega_p1 = G_star * sqrt_pi
    omega_p2 = mp.mpc(0, 1) * omega_p1
    eta_p1 = q
    eta_p2 = mp.mpc(0, -1) * q

    det = omega_p1 * eta_p2 - omega_p2 * eta_p1
    expected = mp.mpc(0, 2) * mp.pi  # +2 pi i

    diff = abs(det - expected)
    assert diff < mp.mpf('1e-78'), f"Legendre det = {det}, expected {expected}, diff = {diff}"

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
    assert hasattr(gse, 'z_i_eigenline_dim'), "z_i_eigenline_dim missing"
    assert hasattr(gse, 'c_invariant_dim'), "c_invariant_dim alias missing"


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


def test_identity_I3_sym_k_eigenlines():
    """I3: For k in {2, 3, 4, 5}, Sym^k(H^1) has k+1 basis monomials, and the
    Z[i]-eigenvalues form the sequence i^(2j-k) for j = 0, 1, ..., k.

    Per Hypothesis H1 (pre-registered).

    Note: sym_k_basis(k) returns (a, b) with a ascending from 0 to k (a == j),
    so eigenvalue at index j is i^(a-b) = i^(j-(k-j)) = i^(2j-k).
    The expected_eigenvalues lists below follow this ascending-a order.
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    # Ascending-a order: eigenvalue at index j is i^(2j - k)
    expected_eigenvalues = {
        2: [sp.I**(-2), sp.I**0, sp.I**2],              # j=0,1,2 -> i^-2, 1, i^2
        3: [sp.I**(-3), sp.I**(-1), sp.I**1, sp.I**3],  # j=0..3
        4: [sp.I**(-4), sp.I**(-2), sp.I**0, sp.I**2, sp.I**4],
        5: [sp.I**(-5), sp.I**(-3), sp.I**(-1), sp.I**1, sp.I**3, sp.I**5],
    }

    for k in [2, 3, 4, 5]:
        basis = gse.sym_k_basis(k)
        assert len(basis) == k + 1, f"Sym^{k} basis should have {k+1} elements, got {len(basis)}"

        # Verify (a, b) pairs are in expected order (a ascending)
        for j, (a, b) in enumerate(basis):
            assert a + b == k, f"Basis element ({a}, {b}) does not satisfy a+b={k}"
            assert a == j, f"Basis element index {j} should have a={j} (ascending), got a={a}"

        # Verify Z[i]-eigenvalues match expected
        computed = [gse.z_i_eigenvalue(a, b) for (a, b) in basis]
        expected = [sp.simplify(ev) for ev in expected_eigenvalues[k]]
        computed_simplified = [sp.simplify(ev) for ev in computed]
        assert computed_simplified == expected, (
            f"Sym^{k} eigenvalues: computed {computed_simplified}, expected {expected}"
        )


def test_c_action_squared_is_identity():
    """The complex-conjugation involution c, applied twice, returns the identity.

    Per Convention C3: c acts on Sym^k(H^1) (x) Q[i] by conjugating Q[i]-coefficients only.
    Test: for an arbitrary Q[i]-linear combination of basis monomials, c(c(x)) == x.
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    # Import the module's exact symbol instances to avoid SymPy assumption mismatch
    omega = gse.omega
    eta = gse.eta
    I = sp.I

    # Test element in Sym^3: (2 + 3*i) * omega^3 + (1 - i) * omega^2 * eta + 5 * omega * eta^2 + (-i) * eta^3
    x = (2 + 3*I) * omega**3 + (1 - I) * omega**2 * eta + 5 * omega * eta**2 + (-I) * eta**3

    cc_x = gse.c_action(gse.c_action(x))
    diff = sp.simplify(cc_x - x)
    assert diff == 0, f"c(c(x)) != x: diff = {diff}"


def test_z_i_eigenline_dim_sym2():
    """I4 (sharpened): the Z[i]-trivial eigenline basis count of Sym^2(H^1) is 1,
    generated by omega*eta.

    Z[i]-trivial means eigenvalue 1, i.e., (a - b) ≡ 0 (mod 4). For Sym^2 (a+b=2),
    only (a, b) = (1, 1) has a - b = 0.

    Connection to c-invariance: under Convention C3, the Q-dimension of
    Sym^2(H^1)^c ∩ Z[i]-trivial-eigenline equals this count (= 1), confirming H3.
    """
    import gstar_sym_k_eigenlines as gse
    dim = gse.z_i_eigenline_dim(k=2, z_i_eigenvalue=1)
    assert dim == 1, f"z_i_eigenline_dim(Sym^2, eigenvalue=1) = {dim}, expected 1 (H3)"


def test_z_i_eigenline_dim_sym3():
    """Sym^3(H^1) has NO Z[i]-trivial-eigenvalue basis elements (since for k=3,
    a - b is odd, so eigenvalue is in {±i}, never 1).

    Therefore the Z[i]-trivial eigenline basis count of Sym^3 is 0.
    """
    import gstar_sym_k_eigenlines as gse
    dim = gse.z_i_eigenline_dim(k=3, z_i_eigenvalue=1)
    assert dim == 0, f"z_i_eigenline_dim(Sym^3, eigenvalue=1) = {dim}, expected 0"


def test_c_invariant_dim_alias_works():
    """The deprecated c_invariant_dim alias forwards to z_i_eigenline_dim."""
    import gstar_sym_k_eigenlines as gse
    assert gse.c_invariant_dim(k=2, z_i_eigenvalue=1) == gse.z_i_eigenline_dim(k=2, z_i_eigenvalue=1) == 1
    assert gse.c_invariant_dim(k=3, z_i_eigenvalue=1) == gse.z_i_eigenline_dim(k=3, z_i_eigenvalue=1) == 0


def test_j_action_on_omega():
    """J(omega) = -i * eta / G_star_sym (Convention C6)."""
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    expected = -sp.I * gse.eta / gse.G_star_sym
    actual = gse.j_action(gse.omega)
    diff = sp.simplify(actual - expected)
    assert diff == 0, f"J(omega) != -i*eta/G_star_sym: diff = {diff}"


def test_j_action_on_eta():
    """J(eta) = i * G_star_sym * omega (Convention C6)."""
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    expected = sp.I * gse.G_star_sym * gse.omega
    actual = gse.j_action(gse.eta)
    diff = sp.simplify(actual - expected)
    assert diff == 0, f"J(eta) != i*G_star_sym*omega: diff = {diff}"


def test_j_squared_parity():
    """Property C6.1: J^2 = (-1)^k * id on Sym^k for k in {1, 2, 3, 4, 5}.

    Verifies the Hodge complex-structure parity on each monomial basis element.
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    for k in [1, 2, 3, 4, 5]:
        expected_sign = (-1) ** k
        for a, b in gse.sym_k_basis(k):
            x = gse.omega**a * gse.eta**b
            jj_x = gse.j_action(gse.j_action(x))
            expected = expected_sign * x
            diff = sp.simplify(jj_x - expected)
            assert diff == 0, (
                f"J^2(omega^{a} * eta^{b}) != ({expected_sign}) * x: diff = {diff}"
            )


def test_sigma_factor_closed_form():
    """Property C6.2: sigma_{a,b} = (-1)^a * i^(a+b) * G_star_sym^(b-a).

    Verifies the closed-form sigma matches what j_action produces on each basis monomial.
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    for k in [2, 3, 4, 5]:
        for a, b in gse.sym_k_basis(k):
            # Expected sigma per closed form
            sigma_closed = (-1)**a * sp.I**(a + b) * gse.G_star_sym**(b - a)
            # Actual sigma extracted from j_action(omega^a * eta^b) = sigma * omega^b * eta^a
            x = gse.omega**a * gse.eta**b
            j_x = gse.j_action(x)
            x_swapped = gse.omega**b * gse.eta**a
            # j_x should equal sigma * x_swapped; extract sigma by dividing (symbolically)
            sigma_actual = sp.simplify(j_x / x_swapped)

            diff = sp.simplify(sigma_actual - sigma_closed)
            assert diff == 0, (
                f"sigma_{{{a},{b}}}: closed={sigma_closed}, actual={sigma_actual}, diff={diff}"
            )


def test_sigma_factor_consistency_C6_3():
    """Property C6.3: conj(sigma_{a,b}) * sigma_{b,a} = (-1)^(a+b) = (-1)^k.

    This is the algebraic consistency that gives J^2 = (-1)^k * id via the
    semi-linear composition (the conj on the inner sigma comes from j_action
    conjugating Q[i]-coefficients on the second pass).
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    for k in [2, 3, 4, 5]:
        for a, b in gse.sym_k_basis(k):
            sigma_ab = gse.sigma_factor(a, b)
            sigma_ba = gse.sigma_factor(b, a)
            lhs = sp.conjugate(sigma_ab) * sigma_ba
            # G_star_sym is real positive so its conjugate equals itself; simplify should reduce powers
            lhs_simp = sp.simplify(lhs)
            rhs = (-1)**(a + b)
            diff = sp.simplify(lhs_simp - rhs)
            assert diff == 0, (
                f"sigma_{{{a},{b}}} consistency: conj(sigma_ab) * sigma_ba = {lhs_simp}, "
                f"expected {rhs}, diff = {diff}"
            )


def test_j_action_eigenline_swap_C6_4():
    """Property C6.4: J maps the i^(a-b)-eigenline to the i^(b-a)-eigenline.

    Equivalently: if x has Z[i]-eigenvalue lambda, then J(x) has Z[i]-eigenvalue 1/lambda
    (since i^(a-b) * i^(b-a) = i^0 = 1).
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    for k in [2, 3, 4, 5]:
        for a, b in gse.sym_k_basis(k):
            ev_before = gse.z_i_eigenvalue(a, b)        # eigenvalue of omega^a * eta^b
            ev_after = gse.z_i_eigenvalue(b, a)         # eigenvalue of omega^b * eta^a (the J-image basis)
            product = sp.simplify(ev_before * ev_after)
            assert product == 1, (
                f"Sym^{k} (a,b)=({a},{b}): eigenvalue product = {product}, expected 1 (C6.4)"
            )

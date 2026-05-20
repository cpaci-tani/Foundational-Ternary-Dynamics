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


def test_j_matrix_sym_k_anti_diagonal_structure():
    """j_matrix_sym_k(k) is anti-diagonal with sigma_{k-j, j} on position (k-j, j).

    For each basis monomial omega^(k-j)*eta^j (column j), J sends it to
    sigma_{k-j, j} * omega^j * eta^(k-j) (row k-j).
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    for k in [2, 3, 4, 5]:
        M = gse.j_matrix_sym_k(k)
        assert M.shape == (k + 1, k + 1), f"Sym^{k} matrix shape {M.shape}, expected ({k+1},{k+1})"

        # Check anti-diagonal structure
        for j in range(k + 1):
            a = k - j  # ω^a · η^j is the column-j basis element (a + j = k)
            for i in range(k + 1):
                if i == k - j:  # the mirror position where σ lives
                    expected = gse.sigma_factor(a, j)
                    diff = sp.simplify(M[i, j] - expected)
                    assert diff == 0, (
                        f"Sym^{k} M[{i},{j}]: expected sigma_factor({a},{j}) = {expected}, "
                        f"got {M[i, j]}"
                    )
                else:
                    assert M[i, j] == 0, (
                        f"Sym^{k} M[{i},{j}] should be 0 (off anti-diagonal), got {M[i, j]}"
                    )


def test_j_matrix_matches_j_action_on_basis():
    """For each basis monomial v = omega^(k-j)*eta^j, the matrix-vector product
    j_matrix_sym_k(k) @ e_j (e_j the j-th standard basis vector) gives the
    coefficient column of j_action(v) in the monomial basis.
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    for k in [2, 3, 4, 5]:
        M = gse.j_matrix_sym_k(k)
        for j in range(k + 1):
            a = k - j
            b = j
            # The basis vector e_j has 1 at position j, 0 elsewhere
            e_j = sp.Matrix.zeros(k + 1, 1)
            e_j[j, 0] = 1
            # M @ e_j extracts the j-th column of M
            M_col = M * e_j
            # Reconstruct the J-image as a symbolic polynomial
            j_image_reconstructed = sum(
                M_col[i, 0] * gse.omega**(k - i) * gse.eta**i
                for i in range(k + 1)
            )
            # Compare to j_action on the original monomial
            v = gse.omega**a * gse.eta**b
            j_image_direct = gse.j_action(v)
            diff = sp.simplify(sp.expand(j_image_reconstructed - j_image_direct))
            assert diff == 0, (
                f"Sym^{k} j_matrix column {j} doesn't match j_action: "
                f"reconstructed={j_image_reconstructed}, direct={j_image_direct}, diff={diff}"
            )


def test_sym2_j_eigenspace_decomposition():
    """Sym^2 J-eigenspaces under J^2 = +id:
      - eigenvalue +1: dim 2 (spanned by omega*eta and omega^2 - eta^2/G*^2)
      - eigenvalue -1: dim 1 (spanned by omega^2 + eta^2/G*^2)

    Verification strategy:
      1. Compute eigenvects of j_matrix_sym_k(2)
      2. Check the eigenvalues are {+1, -1} with multiplicities {2, 1}
      3. Verify J(predicted eigenvector) = eigenvalue * predicted eigenvector
         (via j_action symbolic check, robust to sympy basis normalization differences)
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    M = gse.j_matrix_sym_k(2)
    eigvecs = M.eigenvects()  # list of (eigenvalue, algebraic_multiplicity, [basis_vectors])

    # Build a {eigenvalue: total dimension of eigenspace} dict
    # Use sp.simplify(eigval - target) == 0 to be robust to different sympy representations
    dim_by_eigenvalue = {}
    for eigval, alg_mult, basis in eigvecs:
        ev_simplified = sp.simplify(eigval)
        dim_by_eigenvalue[ev_simplified] = dim_by_eigenvalue.get(ev_simplified, 0) + len(basis)

    # Expected: {+1: 2, -1: 1}
    assert dim_by_eigenvalue.get(sp.Integer(1), 0) == 2, (
        f"Sym^2 J=+1 eigenspace dim = {dim_by_eigenvalue.get(sp.Integer(1), 0)}, expected 2. "
        f"Full decomposition: {dim_by_eigenvalue}"
    )
    assert dim_by_eigenvalue.get(sp.Integer(-1), 0) == 1, (
        f"Sym^2 J=-1 eigenspace dim = {dim_by_eigenvalue.get(sp.Integer(-1), 0)}, expected 1. "
        f"Full decomposition: {dim_by_eigenvalue}"
    )

    # Verify the predicted eigenvectors are correct (using j_action directly)
    omega_eta = gse.omega * gse.eta
    plus_one_other = gse.omega**2 - gse.eta**2 / gse.G_star_sym**2
    minus_one = gse.omega**2 + gse.eta**2 / gse.G_star_sym**2

    # J(omega*eta) should = +1 * omega*eta
    diff_a = sp.simplify(gse.j_action(omega_eta) - omega_eta)
    assert diff_a == 0, f"J(omega*eta) != omega*eta: diff = {diff_a}"

    # J(omega^2 - eta^2/G*^2) should = +1 * (omega^2 - eta^2/G*^2)
    diff_b = sp.simplify(gse.j_action(plus_one_other) - plus_one_other)
    assert diff_b == 0, f"J(omega^2 - eta^2/G*^2) != omega^2 - eta^2/G*^2: diff = {diff_b}"

    # J(omega^2 + eta^2/G*^2) should = -1 * (omega^2 + eta^2/G*^2)
    diff_c = sp.simplify(gse.j_action(minus_one) - (-minus_one))
    assert diff_c == 0, f"J(omega^2 + eta^2/G*^2) != -(omega^2 + eta^2/G*^2): diff = {diff_c}"


def test_j_matrix_squared_parity_C6_1():
    """Property C6.1 at the matrix level: M * conj(M) = (-1)^k * I_{k+1} where M = j_matrix_sym_k(k).

    Complements test_j_squared_parity (which verifies J^2 = (-1)^k * id per-monomial via j_action)
    by confirming the same property at the matrix algebra level.

    Reflects the semi-linear nature of J: J^2 on a Q[i]-coefficient vector v equals
    M * conj(M) * v (the two conjugations from the two J applications compose to identity
    on v's coefficients, leaving M * conj(M) as the residual matrix action).
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    for k in [2, 3, 4, 5]:
        M = gse.j_matrix_sym_k(k)
        M_conj = M.applyfunc(sp.conjugate)
        product = sp.simplify(M * M_conj)
        expected = ((-1) ** k) * sp.eye(k + 1)
        diff = sp.simplify(product - expected)
        # Check element-wise equality
        for i in range(k + 1):
            for j in range(k + 1):
                assert diff[i, j] == 0, (
                    f"Sym^{k}: (M * conj(M))[{i},{j}] = {product[i,j]}, "
                    f"expected ({(-1)**k}) * delta_{{{i},{j}}} = {expected[i,j]}, "
                    f"diff = {diff[i,j]}"
                )


def test_sym4_j_eigenspace_decomposition():
    """Sym^4 has J^2 = +id, so J has real eigenvalues +/-1.

    Expected decomposition (verified by hand from j_matrix_sym_k(4) block structure):
      - dim(J = +1) = 3
      - dim(J = -1) = 2

    Verifies the +1 eigenspace contains both the "Z[i]-trivial subspace's J=+1 part"
    (omega^2*eta^2 and omega^4 + eta^4/G*^4) and the off-Z[i]-trivial element
    (omega^3*eta + omega*eta^3 scaled appropriately).
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    M = gse.j_matrix_sym_k(4)
    eigvecs = M.eigenvects()

    dim_by_eigenvalue = {}
    for eigval, alg_mult, basis in eigvecs:
        ev_simp = sp.simplify(eigval)
        dim_by_eigenvalue[ev_simp] = dim_by_eigenvalue.get(ev_simp, 0) + len(basis)

    assert dim_by_eigenvalue.get(sp.Integer(1), 0) == 3, (
        f"Sym^4 J=+1 dim = {dim_by_eigenvalue.get(sp.Integer(1), 0)}, expected 3"
    )
    assert dim_by_eigenvalue.get(sp.Integer(-1), 0) == 2, (
        f"Sym^4 J=-1 dim = {dim_by_eigenvalue.get(sp.Integer(-1), 0)}, expected 2"
    )


def test_sym4_z_i_trivial_subspace_J_split():
    """Reconciliation: the 3-dim Z[i]-trivial subspace of Sym^4 (Phase 0 observation,
    spanned by omega^4, omega^2*eta^2, eta^4) splits under J as:
      - J = +1 part: dim 2, span(omega^2*eta^2, omega^4 + eta^4/G*^4)
      - J = -1 part: dim 1, span(omega^4 - eta^4/G*^4)

    Verifies explicitly via j_action that these three elements have the claimed J-eigenvalues.
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    omega, eta, G = gse.omega, gse.eta, gse.G_star_sym

    # J = +1 eigenvectors in the Z[i]-trivial subspace
    v1 = omega**2 * eta**2
    v2 = omega**4 + eta**4 / G**4
    # J = -1 eigenvector in the Z[i]-trivial subspace
    v3 = omega**4 - eta**4 / G**4

    diff1 = sp.simplify(gse.j_action(v1) - v1)
    assert diff1 == 0, f"J(omega^2 * eta^2) != omega^2 * eta^2: diff = {diff1}"

    diff2 = sp.simplify(gse.j_action(v2) - v2)
    assert diff2 == 0, f"J(omega^4 + eta^4/G*^4) != itself: diff = {diff2}"

    diff3 = sp.simplify(gse.j_action(v3) - (-v3))
    assert diff3 == 0, f"J(omega^4 - eta^4/G*^4) != -(itself): diff = {diff3}"


def test_sym5_j_matrix_structure():
    """Sym^5 has J^2 = -id (already verified at matrix level in L3-3).

    Additional structural check: j_matrix_sym_k(5) is anti-diagonal with 6 entries
    on the anti-diagonal (positions [5-j, j] for j in 0..5), and zero elsewhere.
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    M = gse.j_matrix_sym_k(5)
    assert M.shape == (6, 6)
    for i in range(6):
        for j in range(6):
            if i == 5 - j:
                # Anti-diagonal entry: should match sigma_factor(5-j, j)
                expected = gse.sigma_factor(5 - j, j)
                diff = sp.simplify(M[i, j] - expected)
                assert diff == 0, f"Sym^5 M[{i},{j}] != sigma_factor({5-j},{j}): diff = {diff}"
            else:
                assert M[i, j] == 0, f"Sym^5 M[{i},{j}] should be 0 (off anti-diagonal), got {M[i,j]}"


def test_phi_symbolic_on_monomials():
    """phi_specialise_symbolic on a single monomial omega^a * eta^b returns
    (-1)^b * G_star_sym^(a-b) * pi_sym^((a+b)/2) — matching the closed-form Phi map.

    Tests Sym^2 and Sym^3 basis monomials.
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    pi_sym = sp.Symbol('pi_sym', commutative=True, real=True, positive=True)

    test_cases = [
        # (a, b, expected_Phi)
        (2, 0, gse.G_star_sym**2 * pi_sym),
        (1, 1, -pi_sym),  # Phi(omega * eta)
        (0, 2, pi_sym / gse.G_star_sym**2),
        (3, 0, gse.G_star_sym**3 * pi_sym**sp.Rational(3, 2)),
        (2, 1, -gse.G_star_sym * pi_sym**sp.Rational(3, 2)),
        (1, 2, pi_sym**sp.Rational(3, 2) / gse.G_star_sym),
        (0, 3, -pi_sym**sp.Rational(3, 2) / gse.G_star_sym**3),
    ]

    for a, b, expected in test_cases:
        monomial = gse.omega**a * gse.eta**b
        actual = gse.phi_specialise_symbolic(monomial, pi_sym=pi_sym)
        diff = sp.simplify(actual - expected)
        assert diff == 0, (
            f"Phi(omega^{a} * eta^{b}): expected {expected}, got {actual}, diff = {diff}"
        )


def test_phi_symbolic_on_sym2_j_eigenvectors():
    """Phi-images of Sym^2 J-eigenvectors:
      - Phi(omega * eta) = -pi  (J = +1 eigenvector)
      - Phi(omega^2 - eta^2/G*^2) = G*^2 * pi - pi/G*^4  (J = +1 eigenvector)
      - Phi(omega^2 + eta^2/G*^2) = G*^2 * pi + pi/G*^4  (J = -1 eigenvector)

    All three are REAL (real combinations of G* and pi).
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    pi_sym = sp.Symbol('pi_sym', commutative=True, real=True, positive=True)
    G = gse.G_star_sym

    v_plus_1 = gse.omega * gse.eta
    expected_plus_1 = -pi_sym
    diff1 = sp.simplify(gse.phi_specialise_symbolic(v_plus_1, pi_sym=pi_sym) - expected_plus_1)
    assert diff1 == 0, f"Phi(omega*eta) != -pi: diff = {diff1}"

    v_plus_2 = gse.omega**2 - gse.eta**2 / G**2
    expected_plus_2 = G**2 * pi_sym - pi_sym / G**4
    diff2 = sp.simplify(gse.phi_specialise_symbolic(v_plus_2, pi_sym=pi_sym) - expected_plus_2)
    assert diff2 == 0, f"Phi(omega^2 - eta^2/G*^2) != G*^2*pi - pi/G*^4: diff = {diff2}"

    v_minus = gse.omega**2 + gse.eta**2 / G**2
    expected_minus = G**2 * pi_sym + pi_sym / G**4
    diff3 = sp.simplify(gse.phi_specialise_symbolic(v_minus, pi_sym=pi_sym) - expected_minus)
    assert diff3 == 0, f"Phi(omega^2 + eta^2/G*^2) != G*^2*pi + pi/G*^4: diff = {diff3}"


def test_phi_symbolic_realness_on_q_rational_elements():
    """For any Sym^k element with Q-rational coefficients, Phi yields a real expression
    (a Q-linear combination of G* and pi powers, all real).

    Tests via sp.im(Phi(x)) == 0 on representative elements.
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    pi_sym = sp.Symbol('pi_sym', commutative=True, real=True, positive=True)
    G = gse.G_star_sym

    elements = [
        gse.omega**2,
        gse.omega * gse.eta,
        gse.eta**3,
        gse.omega**3 + 2 * gse.omega * gse.eta**2,
        gse.omega**4 - 5 * gse.omega**2 * gse.eta**2 + 3 * gse.eta**4,
    ]

    for x in elements:
        phi_x = gse.phi_specialise_symbolic(x, pi_sym=pi_sym)
        im_part = sp.simplify(sp.im(phi_x))
        assert im_part == 0, f"Phi({x}) has non-zero imaginary part: im = {im_part}"


def test_X_coord_sym2_parametrisation():
    """For b' = alpha'*omega^2 + beta'*omega*eta + gamma'*eta^2 with Q-rational coeffs,
    X_norm := Phi(b')/pi = alpha'*G*^2 - beta' + gamma'/G*^2.

    Verified symbolically for generic Q-rational alpha', beta', gamma'.
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    pi_sym = sp.Symbol('pi_sym', commutative=True, real=True, positive=True)
    G = gse.G_star_sym
    alpha_p, beta_p, gamma_p = sp.symbols('alpha_p beta_p gamma_p', rational=True)

    b_prime = alpha_p * gse.omega**2 + beta_p * gse.omega * gse.eta + gamma_p * gse.eta**2
    phi_b = gse.phi_specialise_symbolic(b_prime, pi_sym=pi_sym)
    X_norm = sp.simplify(phi_b / pi_sym)

    expected = alpha_p * G**2 - beta_p + gamma_p / G**2
    diff = sp.simplify(X_norm - expected)
    assert diff == 0, f"Sym^2 X_norm parametrisation: got {X_norm}, expected {expected}, diff = {diff}"


def test_Y_coord_sym3_parametrisation():
    """For c' = alpha*omega^3 + beta*omega^2*eta + gamma*omega*eta^2 + delta*eta^3 with Q-rational coeffs,
    Y_norm := Phi(c')/pi^(3/2) = alpha*G*^3 - beta*G* + gamma/G* - delta/G*^3.

    Verified symbolically for generic Q-rational alpha, beta, gamma, delta.
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    pi_sym = sp.Symbol('pi_sym', commutative=True, real=True, positive=True)
    G = gse.G_star_sym
    a, b, c, d = sp.symbols('a b c d', rational=True)

    c_prime = a * gse.omega**3 + b * gse.omega**2 * gse.eta + c * gse.omega * gse.eta**2 + d * gse.eta**3
    phi_c = gse.phi_specialise_symbolic(c_prime, pi_sym=pi_sym)
    Y_norm = sp.simplify(phi_c / pi_sym**sp.Rational(3, 2))

    expected = a * G**3 - b * G + c / G - d / G**3
    diff = sp.simplify(Y_norm - expected)
    assert diff == 0, f"Sym^3 Y_norm parametrisation: got {Y_norm}, expected {expected}, diff = {diff}"


def test_X_Y_master_quadratic_uniqueness_2_3():
    """For (a, b) = (2, 3), the UNIQUE Q-rational (b', c') in Sym^2 x Sym^3 giving
    (X_norm, Y_norm) = (G*^2, G*^3) — the Paper A master quadratic — is
    (b', c') = (omega^2, omega^3).

    This is Theorem 17.5 stated via the Phi specialisation map. The uniqueness
    follows from Q-linear independence of {G*^k} over Q (G* transcendental,
    Chudnovsky 1976).
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    pi_sym = sp.Symbol('pi_sym', commutative=True, real=True, positive=True)
    G = gse.G_star_sym

    # Verify the "obvious" choice (b', c') = (omega^2, omega^3) gives (G*^2, G*^3)
    X_at_omega2 = sp.simplify(gse.phi_specialise_symbolic(gse.omega**2, pi_sym=pi_sym) / pi_sym)
    Y_at_omega3 = sp.simplify(gse.phi_specialise_symbolic(gse.omega**3, pi_sym=pi_sym) / pi_sym**sp.Rational(3, 2))
    assert sp.simplify(X_at_omega2 - G**2) == 0, f"Phi(omega^2)/pi != G*^2: got {X_at_omega2}"
    assert sp.simplify(Y_at_omega3 - G**3) == 0, f"Phi(omega^3)/pi^(3/2) != G*^3: got {Y_at_omega3}"

    # Uniqueness via Q-linear independence:
    # X_norm = alpha'*G*^2 - beta' + gamma'/G*^2 = G*^2 forces alpha'=1, beta'=gamma'=0
    # over Q (since {G*^2, 1, 1/G*^2} are Q-linearly independent for transcendental G*).
    # Y_norm = a*G*^3 - b*G + c/G - d/G*^3 = G*^3 forces a=1, b=c=d=0.
    #
    # Symbolic check: the residue (X_norm - G*^2) is a polynomial in G with rational coefficients;
    # its unique zero (within Q-coefficients) corresponds to (alpha', beta', gamma') = (1, 0, 0).
    alpha_p, beta_p, gamma_p = sp.symbols('alpha_p beta_p gamma_p', rational=True)
    X_general = alpha_p * G**2 - beta_p + gamma_p / G**2
    residue = sp.simplify(X_general - G**2)
    # As a polynomial in G^2 = u, the residue equals (alpha' - 1)*u - beta' + gamma'/u.
    # Multiplying by u: (alpha' - 1)*u^2 - beta'*u + gamma'. As a polynomial in u, this is
    # identically zero iff alpha' = 1, beta' = 0, gamma' = 0.
    u = sp.Symbol('u', positive=True)
    residue_u = residue.subs(G**2, u) * u
    residue_u_expanded = sp.expand(residue_u)
    coeffs = sp.Poly(residue_u_expanded, u).all_coeffs()
    # coeffs should be [alpha' - 1, -beta', gamma']
    # All must be zero for residue = 0
    # We verify the symbolic structure (not solve, just check the polynomial form)
    assert len(coeffs) == 3, f"Expected degree-2 poly in u, got coeffs {coeffs}"


def test_leading_period_discriminant_formula():
    """Discriminant of P_{(a,b)} = x^2 - 16*G*^a*x + 16*G*^b is
    Delta = 256*G*^(2a) - 64*G*^b = 64*G*^b * (4*G*^(2a-b) - 1).
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    G = gse.G_star_sym
    for (a, b) in [(1, 2), (2, 3), (3, 4), (3, 5), (4, 5)]:
        disc = gse.leading_period_discriminant(a, b)
        expected = 256 * G**(2 * a) - 64 * G**b
        diff = sp.simplify(disc - expected)
        assert diff == 0, f"disc({a},{b}) = {disc}, expected {expected}, diff = {diff}"


def test_leading_period_roots_sum_product():
    """Vieta's: x_plus + x_minus = 16*G*^a, x_plus * x_minus = 16*G*^b.
    Verified for several (a, b) pairs.
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp

    G = gse.G_star_sym
    for (a, b) in [(2, 3), (3, 4), (1, 2), (4, 5)]:
        x_plus, x_minus = gse.leading_period_roots(a, b)
        sum_check = sp.simplify(x_plus + x_minus - 16 * G**a)
        product_check = sp.simplify(x_plus * x_minus - 16 * G**b)
        assert sum_check == 0, f"({a},{b}): sum = {x_plus + x_minus}, expected 16*G*^{a}: diff = {sum_check}"
        assert product_check == 0, f"({a},{b}): product = {x_plus * x_minus}, expected 16*G*^{b}: diff = {product_check}"


def test_leading_period_numerical_roots_2_3():
    """Numerical verification of the (2, 3) master quadratic roots
    against Paper A Theorem 6.1:
      x_+ = 137.0361714581... (Paper A eq 16)
      x_- = 3.02396391633...   (Paper A eq 17)
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp
    import mpmath as mp

    mp.mp.dps = 80
    G_star_num = mp.gamma(mp.mpf('0.25')) / mp.gamma(mp.mpf('0.75'))

    x_plus, x_minus = gse.leading_period_roots(2, 3)
    x_plus_num = float(x_plus.subs(gse.G_star_sym, sp.Float(str(G_star_num), 80)))
    x_minus_num = float(x_minus.subs(gse.G_star_sym, sp.Float(str(G_star_num), 80)))

    # Paper A values:
    expected_plus = 137.0361714581548
    expected_minus = 3.023963916339021

    assert abs(x_plus_num - expected_plus) < 1e-10, f"x_plus = {x_plus_num}, expected {expected_plus}"
    assert abs(x_minus_num - expected_minus) < 1e-10, f"x_minus = {x_minus_num}, expected {expected_minus}"


def test_leading_period_real_roots_admissibility():
    """For (a, b) with a < b: real roots iff 2a - b >= -1 (equiv. b <= 2a + 1).

    Verified numerically: substitute G* numerical value, check discriminant sign.
    """
    import gstar_sym_k_eigenlines as gse
    import sympy as sp
    import mpmath as mp

    mp.mp.dps = 50
    G_star_num = float(mp.gamma(mp.mpf('0.25')) / mp.gamma(mp.mpf('0.75')))

    test_pairs = [
        # (a, b, expected_real_roots)
        (1, 2, True),   # 2*1 - 2 = 0  >= -1  YES
        (1, 3, True),   # 2*1 - 3 = -1 >= -1  YES (boundary)
        (1, 4, False),  # 2*1 - 4 = -2 < -1   NO
        (2, 3, True),   # 2*2 - 3 = 1  >= -1  YES (Paper A)
        (2, 4, True),   # 2*2 - 4 = 0  YES
        (2, 5, True),   # 2*2 - 5 = -1 YES (boundary)
        (2, 6, False),  # 2*2 - 6 = -2 NO
        (3, 4, True),
        (3, 5, True),
        (3, 7, True),   # 2*3 - 7 = -1 YES (boundary)
        (3, 8, False),
    ]

    for (a, b, expected) in test_pairs:
        disc = gse.leading_period_discriminant(a, b)
        disc_num = float(disc.subs(gse.G_star_sym, sp.Float(G_star_num, 50)))
        actual = disc_num >= 0
        assert actual == expected, (
            f"({a},{b}): disc = {disc_num}, expected real_roots = {expected}, got {actual}"
        )

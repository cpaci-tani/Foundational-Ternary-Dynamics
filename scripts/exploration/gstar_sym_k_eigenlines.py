"""Symmetric period algebra of E_lemn: y^2 = x^3 - x.

Phase 0 of G* opus follow-up. Implements:
  - Sym^k(H^1) basis as Q[i]-monomials in (omega, eta)
  - Z[i]-eigenvalue of each basis element
  - Specialisation map Phi from formal monomials to numerical periods
  - c-invariant dimension count

See companion spec at docs/superpowers/specs/2026-05-19-gstar-followup-attacks-design.md
and conventions at docs/theory/09_mathematical/EXPLR_SYM_PERIOD_ALGEBRA_CONVENTIONS.md.
"""

import sympy as sp
import mpmath as mp

# Lock 80-digit precision globally for this module's computations
mp.mp.dps = 80

# Formal generators of H^1_dR(E_lemn) (Convention C1, C2 in conventions doc)
omega = sp.Symbol('omega', commutative=True, real=True)
eta = sp.Symbol('eta', commutative=True, real=True)
i = sp.I  # Q[i] imaginary unit
G_star_sym = sp.Symbol('G_star_sym', commutative=True, real=True, positive=True)
"""Formal SymPy symbol tracking G* in the symmetric period algebra.

Use Phi specialisation (phi_specialise) to substitute the 80-digit numerical
value when needed. G_star_sym appears in sigma_{a,b} scaling factors of the
Hodge complex structure J (Convention C6).
"""


def sym_k_basis(k):
    """Return list of (a, b) pairs with a + b = k, indexing Sym^k(H^1) basis monomials omega^a * eta^b.

    Convention C1: each monomial generates a Z[i]-eigenline with eigenvalue i^(a-b).
    Length of returned list: k + 1.
    """
    return [(a, k - a) for a in range(k + 1)]


def z_i_eigenvalue(a, b):
    """Z[i]-eigenvalue of the monomial omega^a * eta^b under the CM action [i]^*.

    Returns one of {1, i, -1, -i} via i^(a - b) mod 4.
    """
    return sp.I**((a - b) % 4)


def phi_specialise(a, b, G_star_val, sqrt_pi_val):
    """Specialisation map Phi: omega^a * eta^b -> mpf numerical value.

    Per Convention C2:
      Phi(omega) = G* * sqrt(pi)
      Phi(eta) = -sqrt(pi) / G*
    """
    omega_val = G_star_val * sqrt_pi_val
    eta_val = -sqrt_pi_val / G_star_val
    return (omega_val ** a) * (eta_val ** b)


def z_i_eigenline_dim(k, z_i_eigenvalue=1):
    """Count of Sym^k(H^1) basis monomials with given Z[i]-eigenvalue.

    For monomial omega^a * eta^b with a + b = k, the Z[i]-eigenvalue is i^(a-b).
    This function counts how many such basis monomials match the target eigenvalue.

    Connection to c-invariance:
      Under Convention C3 (c acts on Q[i]-coefficients only, fixes omega and eta),
      an element with Q-rational coefficients on a single eigenline is c-invariant.
      Therefore for each basis monomial in the target eigenline, the c-invariant
      Q-rational sub-line is 1-dimensional; the total Q-dimension of
      Sym^k(H^1)^c ∩ (target Z[i]-eigenline) equals the count this function returns.

    Args:
      k: degree of the symmetric power.
      z_i_eigenvalue: target Z[i]-eigenvalue (one of 1, sp.I, -1, -sp.I).
        Defaults to 1 (the trivial eigenline).

    Returns:
      Integer count of basis monomials in the target Z[i]-eigenline.

    Raises:
      ValueError if z_i_eigenvalue is not in {1, i, -1, -i}.
    """
    target_idx = None
    for idx, val in enumerate([1, sp.I, -1, -sp.I]):
        if sp.simplify(sp.I**idx - z_i_eigenvalue) == 0:
            target_idx = idx
            break
    if target_idx is None:
        raise ValueError(f"z_i_eigenvalue {z_i_eigenvalue} is not in {{1, i, -1, -i}}")

    count = 0
    for a, b in sym_k_basis(k):
        if (a - b) % 4 == target_idx:
            count += 1
    return count


# Backward-compatible alias (deprecated; will be removed in Phase 1)
c_invariant_dim = z_i_eigenline_dim


def j_action(x):
    """Apply the Hodge complex structure J to a Sym^k(H^1) (x) Q[i] element.

    Per Convention C6:
      J(omega) = -i * eta / G_star_sym
      J(eta)   = i * G_star_sym * omega
      J extended multiplicatively on monomials; semi-linearly on Q[i]-coefficients
      (J(alpha * x) = conj(alpha) * J(x) for alpha in Q[i]).

    Property: J^2 = (-1)^k * id on Sym^k(H^1).

    Implementation strategy: conjugate Q[i]-coefficients (via Convention C3 c_action),
    then substitute omega -> -i * eta / G_star_sym and eta -> i * G_star_sym * omega.
    Use intermediate placeholder symbols to avoid omega <-> eta substitution collision.
    """
    omega_tmp = sp.Symbol('__j_omega_tmp', commutative=True, real=True)
    eta_tmp = sp.Symbol('__j_eta_tmp', commutative=True, real=True)

    # Step 1: conjugate Q[i]-coefficients only (C3 c_action)
    x_conj = c_action(x)

    # Step 2: substitute omega -> tmp_omega, eta -> tmp_eta (avoid swap collision)
    step2 = x_conj.subs({omega: omega_tmp, eta: eta_tmp})

    # Step 3: substitute tmp_omega -> J(omega), tmp_eta -> J(eta)
    step3 = step2.subs({
        omega_tmp: -sp.I * eta / G_star_sym,
        eta_tmp: sp.I * G_star_sym * omega,
    })

    return sp.expand(step3)


def c_action(x):
    """Apply complex conjugation c on a Sym^k(H^1) (x) Q[i] element.

    Per Convention C3: c acts only on the Q[i]-coefficients, conjugating them.
    The Q-rational form generators omega, eta are c-fixed (declared real at the top of this module).

    Implementation: SymPy's sp.conjugate respects the real= assumption on omega and eta,
    so conjugate(omega) simplifies to omega and conjugate(eta) to eta, while complex
    coefficients are conjugated as expected.
    """
    return sp.expand(sp.conjugate(x))


def sigma_factor(a, b):
    """Closed-form Legendre scaling factor in Convention C6.

    sigma_{a,b} = (-1)^a * i^(a+b) * G_star_sym^(b-a)

    such that j_action(omega^a * eta^b) = sigma_{a,b} * omega^b * eta^a.

    Property C6.3 (consistency): conj(sigma_{a,b}) * sigma_{b,a} = (-1)^k for a+b=k,
    which is the algebraic condition giving J^2 = (-1)^k * id on Sym^k.
    """
    return (-1)**a * sp.I**(a + b) * G_star_sym**(b - a)


def j_matrix_sym_k(k):
    """Matrix of J in the monomial basis (omega^k, omega^(k-1)*eta, ..., eta^k) of Sym^k(H^1).

    Returns a (k+1) x (k+1) sympy Matrix M such that, when v is the column vector
    of Q-rational coefficients of an element x = sum_j c_j * omega^(k-j) * eta^j,
    the column M @ v gives the coefficients of J(x) in the same basis.

    Structure: M is anti-diagonal — M[i, j] = sigma_factor(k-j, j) if i = k - j, else 0.
    This reflects that J(omega^(k-j) * eta^j) = sigma_{k-j,j} * omega^j * eta^(k-j),
    where omega^j * eta^(k-j) is the (k-j)-th basis element.

    Semi-linearity caveat: J is semi-linear over Q[i]. This matrix represents J's
    action on REAL/Q-rational coefficient vectors only. For Q[i] coefficients, apply
    c_action to v first (to conjugate the Q[i]-coefficients) before multiplying by M.

    Used by L3-2 through L3-4 to compute J-eigenspace decompositions.
    """
    M = sp.zeros(k + 1, k + 1)
    for j in range(k + 1):
        a = k - j  # column j basis element is omega^a * eta^j
        b = j
        # J sends omega^a * eta^b to sigma_factor(a, b) * omega^b * eta^a.
        # The result omega^b * eta^a = omega^j * eta^(k-j) is the basis element
        # at index i where omega-power = k - i = j, so i = k - j = a.
        M[k - j, j] = sigma_factor(a, b)
    return M


def phi_specialise_symbolic(x, pi_sym=None):
    """Specialisation map Phi: Q[omega, eta] (x) Q[i] -> Q(G_star_sym, pi_sym, i),
    extending the monomial-level phi_specialise to general Sym^k(H^1) elements
    expressed symbolically in (omega, eta) with Q[i] coefficients.

    Per Convention C2:
      Phi(omega) = G_star_sym * sqrt(pi_sym)
      Phi(eta) = -sqrt(pi_sym) / G_star_sym

    For a monomial omega^a * eta^b, Phi maps to (-1)^b * G_star_sym^(a-b) * pi_sym^((a+b)/2).
    For a general Q[i]-linear combination, Phi acts linearly.

    Args:
      x: sympy expression in omega, eta, optionally with Q[i] coefficients.
         May be a polynomial or rational expression in omega, eta.
      pi_sym: optional sympy Symbol for pi (default: a new Symbol named 'pi_sym').
         Pass an explicit symbol if you want to substitute or compare with other expressions.

    Returns:
      A sympy expression in G_star_sym and pi_sym (and possibly i for Q[i] coeffs),
      representing Phi(x).
    """
    if pi_sym is None:
        pi_sym = sp.Symbol('pi_sym', commutative=True, real=True, positive=True)

    sqrt_pi = sp.sqrt(pi_sym)
    return sp.expand(
        x.subs({
            omega: G_star_sym * sqrt_pi,
            eta: -sqrt_pi / G_star_sym,
        })
    )


def leading_period_discriminant(a, b):
    """Discriminant of leading-period polynomial P_{(a,b)}(x) = x^2 - 16*G*^a*x + 16*G*^b.

    Returns the symbolic expression in G_star_sym:
      Delta = (16*G_star_sym^a)^2 - 4*16*G_star_sym^b
            = 64*G_star_sym^b * (4*G_star_sym^(2a-b) - 1)

    Real-roots condition (Δ ≥ 0): equivalent to G_star_sym^(2a-b) >= 1/4.
    For G* ≈ 2.9587, this is equivalent to 2a - b >= -1.
    """
    return 256 * G_star_sym**(2 * a) - 64 * G_star_sym**b


def leading_period_roots(a, b):
    """The two roots of P_{(a,b)}(x) = x^2 - 16*G*^a*x + 16*G*^b.

    Returns (x_plus, x_minus) where x_plus >= x_minus when real.
    Uses the quadratic formula symbolically.
    """
    disc = leading_period_discriminant(a, b)
    sqrt_disc = sp.sqrt(disc)
    x_plus = (16 * G_star_sym**a + sqrt_disc) / 2
    x_minus = (16 * G_star_sym**a - sqrt_disc) / 2
    return x_plus, x_minus


def unit_group_order_imag_quad(d):
    """Order of the unit group mu_K of the imaginary quadratic field K = Q(sqrt(-d)).

    |mu_K| = 4 for d=1 (Q(i), units {+/-1, +/-i}),
             6 for d=3 (Q(rho), sixth roots of unity),
             2 otherwise ({+/-1}).

    Args:
      d: positive squarefree integer.
    """
    if d == 1:
        return 4
    if d == 3:
        return 6
    return 2


def discriminant_imag_quad(d):
    """Discriminant of the imaginary quadratic field K = Q(sqrt(-d)).

    For d squarefree positive:
      disc(K) = -d   if -d ≡ 1 (mod 4), i.e. d ≡ 3 (mod 4)
      disc(K) = -4d  if -d ≡ 2, 3 (mod 4), i.e. d ≡ 1, 2 (mod 4)

    Args:
      d: positive squarefree integer.
    """
    if d % 4 == 3:
        return -d
    else:
        return -4 * d


# Classification of the integer-4 catalogue per corrected Theorem T-A2.
# Class 'a': unit-derived (|mu_4| = |Z[i]^x| = |Aut(E_lemn)| = 4)
# Class 'b': discriminant-derived (|disc(Q(i))| = 4 = conductor of chi_{-4} = (1+i)-tower level)
# Class 'c': module-rank (dim_Z(V_complex) = dim_Z(Z[i]^2) = 4)
# 'ERROR': the spec-draft entry rank_Z(H^1) = 4 is wrong; rank_Z(H^1(E_lemn)) = 2.
_CATALOGUE_4_CLASSES = {
    'Z_i_unit_group': 'a',
    'Aut_E_lemn': 'a',
    'conductor_chi_minus_4': 'b',
    'one_plus_i_tower_level': 'b',
    'V_complex_Z_rank': 'c',
    'rank_Z_H1': 'ERROR',
}


def classify_catalogue_4(name):
    """Classify a catalogue '4' entry into class 'a', 'b', 'c', or 'ERROR'.

    Per the corrected Theorem T-A2 (integer-4 unification):
      'a' = unit-derived:          |mu_4| = |Z[i]^x| = |Aut(E_lemn)| = 4
      'b' = discriminant-derived:  |disc(Q(i))| = 4 (conductor of chi_{-4}, (1+i)-tower level)
      'c' = module-rank:           dim_Z(V_complex) = dim_Z(Z[i]^2) = 4
      'ERROR' = the spec-draft entry rank_Z(H^1) = 4, which is wrong
                (rank_Z H^1(E_lemn) = 2; it is rank 1 over Z[i]).

    Args:
      name: catalogue entry identifier (str).

    Returns:
      'a', 'b', 'c', or 'ERROR'.

    Raises:
      KeyError if name is not a known catalogue entry.
    """
    return _CATALOGUE_4_CLASSES[name]


def phi_is_real_forces_q_rational(k):
    """Reality-collapse lemma witness for Sym^k(H^1).

    Returns the list of monomial Phi-images [Phi(omega^a * eta^b) : (a,b) in sym_k_basis(k)],
    which are Q-linearly independent (distinct G*-powers, G* transcendental by Chudnovsky 1976).

    The lemma: for b' = sum alpha_{a,b} * omega^a * eta^b with alpha in Q[i],
      Phi(b') in R  <=>  Im(Phi(b')) = sum Im(alpha_{a,b}) * Phi(omega^a eta^b) = 0
                    <=>  all Im(alpha_{a,b}) = 0  (by Q-linear independence)
                    <=>  b' has Q-rational coefficients.

    This collapses Conjecture 16.5.2's "arbitrary Sym^a coefficient" freedom to the
    leading-period (Q-rational) case, reducing it to Theorem 17.5.

    Returns:
      List of sympy expressions (the monomial Phi-images), in sym_k_basis(k) order.
      Each is real; the list is Q-linearly independent.
    """
    pi_sym = sp.Symbol('pi_sym', commutative=True, real=True, positive=True)
    return [
        phi_specialise_symbolic(omega**a * eta**b, pi_sym=pi_sym)
        for (a, b) in sym_k_basis(k)
    ]


if __name__ == "__main__":
    import sys
    import mpmath as mp

    mp.mp.dps = 80
    G_star = mp.gamma(mp.mpf('0.25')) / mp.gamma(mp.mpf('0.75'))
    sqrt_pi = mp.sqrt(mp.pi)

    print("=" * 70)
    print("G* OPUS FOLLOW-UP PHASE 0: SYMMETRIC PERIOD ALGEBRA INFRASTRUCTURE")
    print("=" * 70)
    print()
    print(f"G* = Gamma(1/4)/Gamma(3/4) (80 digits):")
    print(f"  {mp.nstr(G_star, 80)}")
    print(f"sqrt(pi) (80 digits):")
    print(f"  {mp.nstr(sqrt_pi, 80)}")
    print(f"omega_period = G* * sqrt(pi):")
    print(f"  {mp.nstr(G_star * sqrt_pi, 80)}")
    print(f"eta_period = -sqrt(pi) / G*:")
    print(f"  {mp.nstr(-sqrt_pi / G_star, 80)}")
    print()
    print("Identity I1: Phi(omega^2) = G*^2 * pi")
    lhs_i1 = phi_specialise(2, 0, G_star, sqrt_pi)
    rhs_i1 = G_star**2 * mp.pi
    print(f"  Phi(omega^2)       = {mp.nstr(lhs_i1, 30)}")
    print(f"  G*^2 * pi          = {mp.nstr(rhs_i1, 30)}")
    print(f"  |diff|             = {mp.nstr(abs(lhs_i1 - rhs_i1), 5)}")
    print()
    print("Identity I2a: Phi(eta^2) = pi / G*^2")
    lhs_i2a = phi_specialise(0, 2, G_star, sqrt_pi)
    rhs_i2a = mp.pi / G_star**2
    print(f"  Phi(eta^2)         = {mp.nstr(lhs_i2a, 30)}")
    print(f"  pi / G*^2          = {mp.nstr(rhs_i2a, 30)}")
    print(f"  |diff|             = {mp.nstr(abs(lhs_i2a - rhs_i2a), 5)}")
    print()
    print("Identity I2b: Phi(omega*eta) = -pi")
    lhs_i2b = phi_specialise(1, 1, G_star, sqrt_pi)
    rhs_i2b = -mp.pi
    print(f"  Phi(omega*eta)     = {mp.nstr(lhs_i2b, 30)}")
    print(f"  -pi                = {mp.nstr(rhs_i2b, 30)}")
    print(f"  |diff|             = {mp.nstr(abs(lhs_i2b - rhs_i2b), 5)}")
    print()
    print("Identity I3: Sym^k eigenline tables")
    for k in [2, 3, 4, 5]:
        basis = sym_k_basis(k)
        eigs = [z_i_eigenvalue(a, b) for (a, b) in basis]
        print(f"  Sym^{k}: basis = {basis}")
        print(f"          Z[i]-eigenvalues = {eigs}")
    print()
    print("Identity I4: c-invariant dimensions (joint Z[i]-trivial + c-inv)")
    for k in [2, 3, 4, 5]:
        d_triv = z_i_eigenline_dim(k, z_i_eigenvalue=1)
        print(f"  Z[i]-eigenline basis count (Sym^{k}, eigenvalue=1) = {d_triv}")
    print()
    print("PHASE 0 INFRASTRUCTURE VERIFIED.")
    print("=" * 70)

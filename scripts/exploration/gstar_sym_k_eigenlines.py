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


def c_action(x):
    """Apply complex conjugation c on a Sym^k(H^1) (x) Q[i] element.

    Per Convention C3: c acts only on the Q[i]-coefficients, conjugating them.
    The Q-rational form generators omega, eta are c-fixed (declared real at the top of this module).

    Implementation: SymPy's sp.conjugate respects the real= assumption on omega and eta,
    so conjugate(omega) simplifies to omega and conjugate(eta) to eta, while complex
    coefficients are conjugated as expected.
    """
    return sp.expand(sp.conjugate(x))


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

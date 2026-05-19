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


def c_invariant_dim(k, z_i_eigenvalue=1):
    """Dimension of (Sym^k(H^1)^c ∩ Z[i]-eigenline) over Q[i].

    Args:
      k: degree of the symmetric power.
      z_i_eigenvalue: target Z[i]-eigenvalue (one of 1, sp.I, -1, -sp.I).
        Defaults to 1 (the trivial eigenline).

    Returns:
      The Q[i]-dimension of the joint subspace.

    Method:
      For each basis monomial omega^a * eta^b in Sym^k, the Z[i]-eigenvalue is i^(a-b).
      Count basis elements whose eigenvalue matches z_i_eigenvalue.
      Per Convention C4, the c-invariance restricts coefficients to Q (not Q[i]),
      but as a Q[i]-module the count of c-eligible monomials is the answer.

      Implementation: a basis monomial is in the target eigenline iff
        i^((a - b) mod 4) == z_i_eigenvalue.
      Then the c-invariant Q[i]-dimension equals the number of such monomials whose
      Q-rational presence is consistent (always 1 per monomial; the c-restriction
      reduces the Q-rational structure but preserves Q[i]-rank-1 per eligible monomial).
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


def c_action(x):
    """Apply complex conjugation c on a Sym^k(H^1) (x) Q[i] element.

    Per Convention C3: c acts only on the Q[i]-coefficients, conjugating them.
    The Q-rational form generators omega, eta are c-fixed (declared real at the top of this module).

    Implementation: SymPy's sp.conjugate respects the real= assumption on omega and eta,
    so conjugate(omega) simplifies to omega and conjugate(eta) to eta, while complex
    coefficients are conjugated as expected.
    """
    return sp.expand(sp.conjugate(x))

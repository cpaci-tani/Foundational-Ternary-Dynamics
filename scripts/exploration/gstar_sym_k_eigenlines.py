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


def c_invariant_dim(k):
    """Dimension of Sym^k(H^1)^c over Q[i].

    Placeholder: actual implementation in Task 8.
    """
    raise NotImplementedError("c_invariant_dim implemented in Task 8")


def c_action(x):
    """Apply complex conjugation c on a Sym^k(H^1) (x) Q[i] element.

    Per Convention C3: c acts only on the Q[i]-coefficients, conjugating them.
    The Q-rational form generators omega, eta are c-fixed (declared real at the top of this module).

    Implementation: SymPy's sp.conjugate respects the real= assumption on omega and eta,
    so conjugate(omega) simplifies to omega and conjugate(eta) to eta, while complex
    coefficients are conjugated as expected.
    """
    return sp.expand(sp.conjugate(x))

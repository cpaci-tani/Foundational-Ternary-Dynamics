"""probe_bell_commutator_price.py — the Landau identity, and what it does
and does not buy a local deterministic substrate.

THE IDENTITY (Landau 1987).  For dichotomic A_i, B_j with A_i^2 = B_j^2 = I,

    B  = A0 (x) (B0+B1) + A1 (x) (B0-B1)
    B^2 = 4 I - [A0,A1] (x) [B0,B1].

Consequences, all checked below:
  * either local pair commuting  =>  |S| <= 2;
  * ||[A0,A1]|| <= 2 each side   =>  ||B|| <= sqrt(8) = 2 sqrt2  (Tsirelson);
  * PARTIAL noncommutativity gives a PARTIAL bound,
        S_max = sqrt(4 + ||[A0,A1]|| ||[B0,B1]||),
    which for qubit projective pairs at angles th_A, th_B is
        S_max = sqrt(4 + 4 sin th_A sin th_B).
    This prices the import: it says exactly how much correlation a given
    amount of local incompatibility can purchase.

THE INFERENCE THAT DOES NOT FOLLOW.  The identity constrains the ALGEBRA
of a quantum model.  It says nothing about whether a local deterministic
substrate can reach S > 2, because Bell's argument never touches the
algebra -- it is a statement about the conditional distributions
P(a,b|x,y).  Section (4) enumerates the local deterministic polytope and
shows max|S| = 2 exactly over ALL 16 vertices, hence over every convex
mixture.  Layering a noncommutative effect algebra on top of a local
deterministic substrate does not move that number, because the recorded
outcomes still come from local response functions.
"""
from __future__ import annotations

import itertools
import numpy as np

I2 = np.eye(2, dtype=complex)
sx = np.array([[0, 1], [1, 0]], dtype=complex)
sy = np.array([[0, -1j], [1j, 0]], dtype=complex)
sz = np.array([[1, 0], [0, -1]], dtype=complex)


def comm(X, Y):
    return X @ Y - Y @ X


def obs(theta):
    """Projective dichotomic qubit observable at angle theta in the z-x plane."""
    return np.cos(theta) * sz + np.sin(theta) * sx


def chsh_op(A0, A1, B0, B1):
    return (np.kron(A0, B0 + B1) + np.kron(A1, B0 - B1))


print("=" * 72)
print("(1) THE LANDAU IDENTITY,  B^2 = 4I - [A0,A1] (x) [B0,B1]")
print("=" * 72)
rng = np.random.default_rng(11)
worst = 0.0
for _ in range(2000):
    ta, ta2, tb, tb2 = rng.uniform(0, 2 * np.pi, 4)
    A0, A1, B0, B1 = obs(ta), obs(ta2), obs(tb), obs(tb2)
    B = chsh_op(A0, A1, B0, B1)
    lhs = B @ B
    rhs = 4 * np.kron(I2, I2) - np.kron(comm(A0, A1), comm(B0, B1))
    worst = max(worst, np.abs(lhs - rhs).max())
print(f"  max |LHS - RHS| over 2000 random angle quadruples: {worst:.3e}")
print(f"  identity holds: {worst < 1e-12}")

print()
print("=" * 72)
print("(2) COMMUTING LOCAL PAIR  =>  |S| <= 2")
print("=" * 72)
print(f"{'case':>34} {'||[A0,A1]||':>12} {'||[B0,B1]||':>12} {'||B||':>9}")
for nm, (ta, ta2, tb, tb2) in [
        ("A pair commuting (th_A = 0)", (0.0, 0.0, 0.0, np.pi / 2)),
        ("B pair commuting (th_B = 0)", (0.0, np.pi / 2, 0.3, 0.3)),
        ("both commuting", (0.0, 0.0, 0.7, 0.7)),
        ("optimal CHSH", (0.0, np.pi / 2, np.pi / 4, -np.pi / 4))]:
    A0, A1, B0, B1 = obs(ta), obs(ta2), obs(tb), obs(tb2)
    nA = np.linalg.norm(comm(A0, A1), 2)
    nB = np.linalg.norm(comm(B0, B1), 2)
    nrm = np.linalg.norm(chsh_op(A0, A1, B0, B1), 2)
    print(f"{nm:>34} {nA:12.6f} {nB:12.6f} {nrm:9.6f}")
print(f"\n  Tsirelson  2 sqrt2 = {2*np.sqrt(2):.6f}")

print()
print("=" * 72)
print("(3) THE PRICE CURVE: how much S does a given incompatibility buy?")
print("=" * 72)
print("    S_max = sqrt(4 + ||[A0,A1]|| ||[B0,B1]||),  both pairs at angle th")
print(f"\n{'th (deg)':>10} {'||[A,A]||':>11} {'S_max (identity)':>18} "
      f"{'||B|| (direct)':>16}")
for deg in (0, 15, 30, 45, 60, 75, 90):
    th = np.deg2rad(deg)
    A0, A1 = obs(0.0), obs(th)
    # place B's symmetrically for the best achievable at this incompatibility
    best = 0.0
    for phi in np.linspace(0, np.pi, 400):
        B0, B1 = obs(phi), obs(phi - th)
        best = max(best, np.linalg.norm(chsh_op(A0, A1, B0, B1), 2))
    nA = np.linalg.norm(comm(A0, A1), 2)
    print(f"{deg:10d} {nA:11.6f} {np.sqrt(4 + nA*nA):18.6f} {best:16.6f}")
print("""
    Zero incompatibility buys exactly 2.  Full incompatibility buys
    2 sqrt2.  Everything between is priced by  sqrt(4 + c^2).""")

print()
print("=" * 72)
print("(4) WHAT THIS DOES NOT BUY: the local deterministic polytope")
print("=" * 72)
print("    Enumerate every deterministic local strategy: Alice's outcome is")
print("    a function of her setting alone (4 functions), likewise Bob.")
print("    A general local hidden-variable model is a convex MIXTURE of")
print("    these 16 vertices, so the maximum over vertices bounds it.\n")
vals = []
for a0, a1, b0, b1 in itertools.product([-1, 1], repeat=4):
    E = {(0, 0): a0 * b0, (0, 1): a0 * b1, (1, 0): a1 * b0, (1, 1): a1 * b1}
    S = E[(0, 0)] + E[(0, 1)] + E[(1, 0)] - E[(1, 1)]
    vals.append(S)
vals = np.array(vals)
print(f"    vertices enumerated : {len(vals)}")
print(f"    distinct |S| values : {sorted(set(np.abs(vals)))}")
print(f"    max |S| over vertices: {np.abs(vals).max()}")
print(f"    => every convex mixture also obeys |S| <= "
      f"{np.abs(vals).max()}, exactly.")
print("""
    This bound is insensitive to ANY algebra layered on top.  Bell's
    argument is about the conditional distributions P(a,b|x,y), not about
    the operators used to describe the alternatives.  A substrate whose
    postulates are (discrete, local, deterministic) supplies exactly a
    mixture over these vertices, with lambda the configuration on a
    spacelike slice -- so it returns |S| <= 2 whatever effect algebra is
    written above it.

    The commutator identity therefore explains what QUANTUM MECHANICS
    needs in order to exceed 2.  It does not supply a route by which a
    Bell-local deterministic substrate could.  Those are different
    questions, and only the second is the substrate's problem.""")

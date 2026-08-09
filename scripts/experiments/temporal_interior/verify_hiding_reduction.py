"""verify_hiding_reduction.py — numerical check of the two claims in the
operational-hiding reduction theorem.

Claim 1 (positive direction).  If a selector on a preferred foliation
reproduces the quantum history weights, then its recorded statistics are
foliation-independent, because the quantum weights themselves are.  We
verify the substantive half numerically: for spacelike-separated local
instruments the composed weight is order-independent.

Claim 2 (the obstruction).  A substrate whose four CHSH observables are
functions on ONE probability space with a setting-independent measure has
a joint distribution, so Fine's theorem caps |S| <= 2.  We verify by brute
force that no such assignment reaches the quantum 2*sqrt(2), i.e. that the
barrier to reproducing the quantum statistics is NOT relativistic.
"""
from __future__ import annotations

import numpy as np
import itertools

rng = np.random.default_rng(20260807)

# ------------------------------------------------------------------
print("=" * 68)
print("CLAIM 1: spacelike instruments compose order-independently")
print("=" * 68)

# Two qubits; A acts on factor 0, B on factor 1 -> the maps commute.
I2 = np.eye(2)


def kron(a, b):
    return np.kron(a, b)


def rand_state():
    v = rng.normal(size=4) + 1j * rng.normal(size=4)
    v /= np.linalg.norm(v)
    return np.outer(v, v.conj())


def proj(theta):
    c, s = np.cos(theta / 2), np.sin(theta / 2)
    v = np.array([c, s])
    return np.outer(v, v.conj())


def instrument_local(P, side):
    """Kraus map rho -> (P ox I) rho (P ox I) acting on one side only."""
    K = kron(P, I2) if side == 0 else kron(I2, P)

    def act(rho):
        return K @ rho @ K.conj().T
    return act


worst = 0.0
for trial in range(400):
    rho = rand_state()
    tA, tB = rng.uniform(0, np.pi, 2)
    PA, PB = proj(tA), proj(tB)
    IA = instrument_local(PA, 0)
    IB = instrument_local(PB, 1)
    pAB = np.trace(IB(IA(rho))).real          # A then B
    pBA = np.trace(IA(IB(rho))).real          # B then A
    worst = max(worst, abs(pAB - pBA))
print(f"  400 random states/settings, max |p(A<B) - p(B<A)| = {worst:.3e}")
print("  => composed weights are order-independent for spacelike local")
print("     instruments (this is the content of (R3)).")

# ------------------------------------------------------------------
print()
print("=" * 68)
print("CLAIM 2: one setting-independent measure caps CHSH at 2")
print("=" * 68)

# Brute force over all deterministic assignments on a single sample space:
# each hidden state lambda fixes A0,A1,B0,B1 in {-1,+1}.  Any measure over
# those 16 deterministic points is a joint distribution; CHSH is linear in
# the measure, so the max over measures is the max over the 16 vertices.
best = -np.inf
for A0, A1, B0, B1 in itertools.product([-1, 1], repeat=4):
    S = A0 * B0 + A0 * B1 + A1 * B0 - A1 * B1
    best = max(best, abs(S))
print(f"  max |S| over all deterministic joint assignments = {best}")
print(f"  quantum (Tsirelson) value                        = {2*np.sqrt(2):.6f}")
print(f"  gap                                              = "
      f"{2*np.sqrt(2) - best:.6f}")

# Confirm the quantum value is attainable, so the gap is real physics.
def chsh_quantum():
    s = np.array([1, 0, 0, -1]) / np.sqrt(2)          # singlet-like
    psi = np.array([0, 1, -1, 0]) / np.sqrt(2)
    rho = np.outer(psi, psi.conj())
    sx = np.array([[0, 1], [1, 0]])
    sz = np.array([[1, 0], [0, -1]])

    def obs(t):
        return np.cos(t) * sz + np.sin(t) * sx
    A0, A1 = obs(0), obs(np.pi / 2)
    B0, B1 = obs(np.pi / 4), obs(-np.pi / 4)
    E = lambda X, Y: np.trace(rho @ kron(X, Y)).real
    return abs(E(A0, B0) + E(A0, B1) + E(A1, B0) - E(A1, B1))


print(f"  attained by the quantum state/settings           = "
      f"{chsh_quantum():.6f}")
print()
print("  => a substrate with ONE setting-independent measure over a")
print("     commutative observable algebra cannot reach the quantum value.")
print("     The barrier is Bell/Fine, not relativistic covariance.")

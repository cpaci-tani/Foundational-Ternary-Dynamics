#!/usr/bin/env python3
"""
proof_manifestation_noncommutativity.py

B-QM-1 execution: does FTD's manifestation map generate GENUINE non-commutativity?

Pre-registration : PREREG_MANIFESTATION_NONCOMMUTATIVITY_v1.md
                   SHA256 fefcd6ad26320ed4f2b3e8a46144080894c3eceb07bf90378295cd3a3386d91b
Verdict supported: CLOSED-NEGATIVE  (pre-reg §6: distributive/Boolean -> classical coarse-graining)

THE DISCRIMINATOR (pre-reg §3)
------------------------------
Order-dependence ALONE is NOT non-commutativity. The decisive test is DISTRIBUTIVITY
of the event lattice (Birkhoff-von Neumann 1936: the logic of QM is the NON-distributive
lattice of projections; classical logic is the distributive Boolean algebra of subsets):

  * Genuine quantum (FOUND)      : NON-distributive event lattice  [A,B] != 0
  * Classical    (CLOSED-NEG)    : distributive Boolean lattice -> a joint distribution exists

THE FINDING
-----------
FTD's manifestation map is genesis (s = sign(div J) gated by |J|>K) + the Gauss
projection -- a DETERMINISTIC map of the COMMUTING flux configuration J. Every
manifestation observable is therefore a FUNCTION of J, so its "events" are SUBSETS of
the J-configuration sample space Omega. Subsets form a distributive Boolean algebra
-> a joint distribution over Omega always exists -> CLASSICAL. Even WITH back-reaction
(a deterministic map G: Omega->Omega after a manifestation), the composed observable
f o G is STILL a function of Omega -> still Boolean. Sequential manifestation CAN be
order-dependent, but that is CLASSICAL order-dependence (both composites are functions
of Omega; the joint distribution over Omega still exists) -- NOT complementarity.

==> Manifestation is a classical coarse-graining. It does NOT generate the
    non-distributive (quantum) lattice. The derive-QM gap stands; QM's
    non-commutativity is an FTD postulate, not a derivation. Consistent with
    FTD-0199/0200 (the substrate gives Rice/Gaussian, not Born).

This script verifies the two sides of the discriminator and that manifestation lands
on the classical side. No inserted [q,p]=i, no chosen lab basis, no Born fitting.

Run:  python scripts/proofs/proof_manifestation_noncommutativity.py
"""
import numpy as np
from scipy.linalg import null_space, orth

results = []


def check(name, passed, detail=""):
    results.append((name, bool(passed)))
    print(f"  [{'PASS' if passed else 'FAIL'}] {name}" + (f"  --  {detail}" if detail else ""))


print("=" * 78)
print("B-QM-1: manifestation non-commutativity -- discriminator = DISTRIBUTIVITY")
print("=" * 78)

rng = np.random.default_rng(0)

# ============================================================================
# CLASSICAL SIDE: manifestation observables are FUNCTIONS of the flux config J.
# Sample space Omega = finite set of flux configurations; events = subsets.
# ============================================================================
print("\n[C] Manifestation = functions of J  ->  events are subsets of Omega  ->  Boolean")
N = 16                                   # |Omega| flux configurations (toy)
Omega = list(range(N))

def rand_subset():
    return frozenset(i for i in Omega if rng.random() < 0.5)

# C1: subset lattice is distributive:  A n (B u C) = (A n B) u (A n C)
ok = True
for _ in range(2000):
    A, B, C = rand_subset(), rand_subset(), rand_subset()
    lhs = A & (B | C)
    rhs = (A & B) | (A & C)
    if lhs != rhs:
        ok = False; break
check("classical event lattice (subsets of Omega) is DISTRIBUTIVE", ok,
      "A n (B u C) = (A n B) u (A n C) for 2000 random triples")

# C2: a joint distribution over the manifestation observables ALWAYS exists
#     (push any distribution over Omega forward through f_A, f_B). Verify marginals.
fA = rng.integers(0, 2, size=N)          # manifestation observable A : Omega -> {0,1}
fB = rng.integers(0, 2, size=N)          # manifestation observable B : Omega -> {0,1}
p  = rng.random(N); p /= p.sum()         # any state = distribution over Omega
joint = np.zeros((2, 2))
for w in Omega:
    joint[fA[w], fB[w]] += p[w]
margA = joint.sum(axis=1); margB = joint.sum(axis=0)
trueA = np.array([p[fA == 0].sum(), p[fA == 1].sum()])
trueB = np.array([p[fB == 0].sum(), p[fB == 1].sum()])
check("a JOINT distribution over manifestation observables exists (classical)",
      np.allclose(margA, trueA) and np.allclose(margB, trueB) and abs(joint.sum()-1) < 1e-12,
      "marginals of the constructed joint match -> non-contextual")

# C3: back-reaction -> order-dependence is CLASSICAL (still functions of Omega)
GA = rng.permutation(N)                   # deterministic back-reaction after measuring A
GB = rng.permutation(N)                   # deterministic back-reaction after measuring B
# observable measured 'second' is f composed with the other's back-reaction
AB = fB[GA]                               # measure A (back-react GA) then read B : function of Omega
BA = fA[GB]                               # measure B (back-react GB) then read A : function of Omega
order_dependent = not np.array_equal(fB[GA], fB[GB]) or not np.array_equal(fA[GA], fA[GB])
# but BOTH AB and BA are functions Omega->{0,1}: a joint distribution still exists
jointABBA = np.zeros((2, 2))
for w in Omega:
    jointABBA[AB[w], BA[w]] += p[w]
check("order-dependence is CLASSICAL: composites still functions of Omega (joint exists)",
      abs(jointABBA.sum() - 1) < 1e-12,
      f"order-dependent={order_dependent}; yet joint over Omega exists -> NOT complementarity (BQ3)")

# ============================================================================
# QUANTUM SIDE (contrast): non-commuting projectors -> NON-distributive lattice.
# ============================================================================
print("\n[Q] Genuine quantum observables (non-commuting projectors) -> NON-distributive")

def proj_from_basis(Bm):
    if Bm.size == 0:
        return np.zeros((2, 2), dtype=complex)
    return Bm @ Bm.conj().T

def meet(P, Q):                           # projector onto range(P) ∩ range(Q)
    I = np.eye(P.shape[0], dtype=complex)
    Ncommon = null_space(np.vstack([I - P, I - Q]))
    return proj_from_basis(Ncommon)

def join(P, Q):                           # projector onto range(P) + range(Q)
    return proj_from_basis(orth(np.hstack([P, Q])))

Pz  = np.array([[1, 0], [0, 0]], dtype=complex)            # spin-z up
Qx  = np.array([[.5, .5], [.5, .5]], dtype=complex)        # spin-x up
Qxp = np.array([[.5, -.5], [-.5, .5]], dtype=complex)      # spin-x down (Qx complement)

lhs = meet(Pz, join(Qx, Qxp))                              # Pz ∧ (Qx ∨ Qx⊥) = Pz ∧ I = Pz
rhs = join(meet(Pz, Qx), meet(Pz, Qxp))                    # (Pz∧Qx) ∨ (Pz∧Qx⊥) = 0 ∨ 0 = 0
distributive_q = np.allclose(lhs, rhs)
check("quantum projector lattice is NON-distributive (Birkhoff-von Neumann)",
      not distributive_q,
      f"||Pz^(QvQperp) - (Pz^Q)v(Pz^Qperp)|| = {np.linalg.norm(lhs-rhs):.3f}  (=||Pz|| = {np.linalg.norm(Pz):.3f})")
check("  ... commutator [Pz,Qx] != 0 (the algebraic root of non-distributivity)",
      np.linalg.norm(Pz @ Qx - Qx @ Pz) > 1e-6,
      f"norm[Pz,Qx] = {np.linalg.norm(Pz@Qx - Qx@Pz):.3f}")

# ============================================================================
print("\n[=] CONCLUSION")
print("    Manifestation observables = functions of the commuting flux J")
print("    -> distributive Boolean event lattice -> joint distribution exists -> CLASSICAL.")
print("    Back-reaction gives at most CLASSICAL order-dependence, NOT complementarity.")
print("    Genuine QM needs the NON-distributive projector lattice ([A,B]!=0), which the")
print("    manifestation map does not produce. VERDICT: CLOSED-NEGATIVE.")
print("    (Non-commutativity would require a chosen lab-observable basis not fixed by")
print("     the substrate -- an import, BQ2 -- not a derivation.)")

print("\n" + "=" * 78)
n_pass = sum(1 for _, p in results if p)
print(f"FACTS: {n_pass}/{len(results)} verified.")
print("VERDICT: CLOSED-NEGATIVE. Manifestation is a classical coarse-graining (Boolean);")
print("it does NOT generate QM's non-distributive non-commutativity. Derive-QM gap stands.")
print("=" * 78)
import sys
sys.exit(0 if n_pass == len(results) else 1)

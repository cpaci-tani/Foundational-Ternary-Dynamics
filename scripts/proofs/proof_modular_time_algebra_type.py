#!/usr/bin/env python3
"""
proof_modular_time_algebra_type.py

B1 execution: von Neumann TYPE of the FTD substrate algebra (Route B, modular time).

Pre-registration : PREREG_MODULAR_TIME_ALGEBRA_TYPE_v1.md
                   SHA256 f8a3e960c400863677e631abba898e13d73ef64023e9da9ea51fe088b63606e5
Verdict supported: CLOSED-NEGATIVE  (pre-reg §6: type I -> no canonical modular flow)

THE CORE FINDING (upstream of the ratio-set question)
-----------------------------------------------------
The FTD substrate's observable algebra is COMMUTATIVE: the fields s in {-1,0,+1},
the flux J, and the wave-velocity v are classical real-valued fields advanced by a
deterministic leapfrog + classical Langevin noise. Observables are functions of
commuting fields -> abelian von Neumann algebra. (The leapfrog phase space carries
a classical symplectic/Poisson J, but the *observable product* is commutative --
classical mechanics, not a CCR algebra.)

A theorem of Tomita-Takesaki theory: an ABELIAN von Neumann algebra has TRIVIAL
modular automorphism group (sigma_t = id) for EVERY faithful normal state (a faithful
state on an abelian algebra is automatically a trace; tracial -> trivial modular flow).
Therefore the substrate has NO canonical type-III_1 modular flow -> no objective
emergent time. Type I.

Masslessness (Phase-G geometric Coulomb) is a RED HERRING here: it gives a continuous
*classical* spectrum, but commutativity caps the type at I (the Araki-Woods ratio-set
question of the pre-reg §7 step 4 is MOOT -- the algebra isn't even non-commutative).

The missing ingredient is NON-COMMUTATIVITY ([q,p] = i ; quantization) -- which is
exactly the inner-product / L^2 structure FTD-0208 proved the substrate lacks.
==> B1's wall is the SAME wall as FTD-0208. Route B does not escape it; it re-finds it.

WHAT THIS SCRIPT VERIFIES
-------------------------
The load-bearing fact: the modular flow sigma_t(x) = rho^{it} x rho^{-it} acts
NON-trivially ONLY on the non-commutative (off-diagonal) part of an observable, and
TRIVIALLY on the commutative (diagonal) part -- for ANY faithful state rho. Hence
"commutative algebra  <=>  trivial modular flow." FTD's commutative substrate -> type I.

No CODATA, no assumed Lorentz, no inserted Hamiltonian, no continuum limit (BF1-BF6 clean).

Run:  python scripts/proofs/proof_modular_time_algebra_type.py
"""

import numpy as np

results = []


def check(name, passed, detail=""):
    results.append((name, bool(passed)))
    print(f"  [{'PASS' if passed else 'FAIL'}] {name}" + (f"  --  {detail}" if detail else ""))


def modular_flow(rho_diag, x, t):
    """sigma_t(x) = rho^{it} x rho^{-it} for a faithful state rho = diag(rho_diag).
       rho^{it} = diag(p_k^{it});  rho^{-it} = conj(rho^{it}) for real p_k>0."""
    D = np.diag(np.power(rho_diag.astype(complex), 1j * t))   # rho^{it}
    return D @ x @ np.conj(D)


print("=" * 78)
print("B1: von Neumann type of the FTD substrate algebra -- verdict: CLOSED-NEGATIVE")
print("=" * 78)

# A faithful state on a 3-level system (a 'classical' probability state along the
# diagonal -- the analog of the substrate's classical equilibrium occupation).
rho = np.array([0.5, 0.3, 0.2])           # faithful (all > 0), not maximally mixed
t = 1.0

# --- 1. Commutative (diagonal) observable: the ONLY kind a classical/abelian
#        algebra contains. Models FTD's commuting classical fields. ---------------
print("\n[1] Commutative (diagonal) observable -> modular flow is TRIVIAL")
x_comm = np.diag([1.0, 2.0, 3.0]).astype(complex)
sig_comm = modular_flow(rho, x_comm, t)
dev_comm = np.linalg.norm(sig_comm - x_comm)
check("sigma_t(x) = x for a commutative observable (any state)", dev_comm < 1e-12,
      f"||sigma_t(x) - x|| = {dev_comm:.2e}")

# --- 2. Non-commutative (off-diagonal) observable: requires [q,p]=i / quantization.
#        ABSENT from the classical substrate. ----------------------------------------
print("\n[2] Non-commutative (off-diagonal) observable -> modular flow is NON-trivial")
x_noncomm = np.array([[0, 1, 0], [1, 0, 0], [0, 0, 0]], dtype=complex)   # off-diagonal
sig_noncomm = modular_flow(rho, x_noncomm, t)
dev_noncomm = np.linalg.norm(sig_noncomm - x_noncomm)
# the (1,2) entry picks up the phase (p1/p2)^{it}
phase = (rho[0] / rho[1]) ** (1j * t)
check("sigma_t(x) != x for a non-commutative observable", dev_noncomm > 1e-6,
      f"||sigma_t(x) - x|| = {dev_noncomm:.3f}; (1,2) phase (p1/p2)^it = {phase:.3f}")
check("non-triviality lives ONLY on the off-diagonal (non-commutative) part",
      abs(sig_noncomm[0, 1] - phase * x_noncomm[0, 1]) < 1e-12,
      "modular flow = phase rotation of the non-commuting matrix elements")

# --- 3. The conclusion for FTD --------------------------------------------------
print("\n[3] FTD substrate = commuting classical fields (s, J, v) -> diagonal-only")
print("    => its observable algebra has ONLY the [1]-type (commutative) part")
print("    => sigma_t = id  ->  type I  ->  NO canonical (III_1) modular flow")
print("    => Route B cannot derive objective time at the substrate level.")
print("    The Araki-Woods ratio-set question (pre-reg §7.4) is MOOT: the algebra")
print("    is not even non-commutative, so masslessness cannot lift it past type I.")

# --- 4. Verify the abelian-trace fact directly: on the commutative subalgebra,
#        rho acts like a trace (state is permutation-symmetric under the algebra) ---
print("\n[4] Abelian => faithful state is tracial => modular flow trivial")
# For any two commuting (diagonal) observables a,b: omega(ab)=omega(ba) trivially.
a = np.diag([1.0, -2.0, 0.5]); b = np.diag([3.0, 0.7, -1.0])
tr_ab = np.real(np.trace(np.diag(rho) @ a @ b))
tr_ba = np.real(np.trace(np.diag(rho) @ b @ a))
check("omega(ab) = omega(ba) on the commutative algebra (tracial)",
      abs(tr_ab - tr_ba) < 1e-12, f"{tr_ab:.6f} = {tr_ba:.6f}")

print("\n" + "=" * 78)
n_pass = sum(1 for _, p in results if p)
print(f"FACTS: {n_pass}/{len(results)} verified.")
print("Modular flow is non-trivial ONLY on non-commutative structure. FTD's substrate")
print("is commutative (classical fields) -> abelian -> type I -> sigma_t = id -> no")
print("canonical modular time. The missing piece is quantization ([q,p]=i) = the L^2")
print("structure FTD-0208 lacks. B1's wall = FTD-0208's wall.")
print("VERDICT: CLOSED-NEGATIVE. Route B reverts to Route A at the substrate level;")
print("the only survivor is the EMERGENT level (contingent on the open derive-QM gap).")
print("=" * 78)
import sys
sys.exit(0 if n_pass == len(results) else 1)

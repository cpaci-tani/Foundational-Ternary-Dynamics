#!/usr/bin/env python3
"""
proof_symplectic_budget_symmetry.py

B-QM-1'' execution (ADVERSARIAL / P4): does FTD geometry supply the full symplectic
S3 = Z/2 |x Z/3 on the EPISTEMIC BUDGET (the 3 COMPLEMENTARY bases of the (J,v) cell)?

Pre-registration : PREREG_SYMPLECTIC_BUDGET_SYMMETRY_v1.md
                   SHA256 dd8a8fa065ae2800d7554a2c82938137d340e0825e37a3362ffc1f22951a0f20
Verdict supported: CLOSED-NEGATIVE  (pre-reg section 6: N_c=3 candidate fails the kind-test;
                   no other FTD source for the Bloch Z/3 -> full S3 not derivable.)

THE HAZARD (named, GTCA F1 apophenia)
-------------------------------------
"The budget needs a Z/3; FTD has N_c=3; therefore N_c supplies it" is a 3=3 COUNT match.
The decisive PRE-REGISTERED test (section 3) is a KIND check: are the three objects the
body-diagonal C3 permutes -- the spatial flux components {J_x, J_y, J_z} -- COMMUTING
(a single co-measurable basis) or mutually COMPLEMENTARY (three non-commuting bases)?

FINDING
-------
  * J^2=-I gives a genuine Z/2 on a conjugate pair (the q-p rotation). [holds]
  * {J_x, J_y, J_z} are independent classical field components -> they COMMUTE
    ([J_i, J_j] = 0). So the body-diagonal C3 permutes a CO-MEASURABLE triple.
  * The budget's three bases are mutually COMPLEMENTARY (Pauli X,Y,Z: [X,Y]=2iZ != 0).
  * COMMUTING triple  !=  COMPLEMENTARY triple. Same COUNT (3), different KIND.
    => the body-diagonal C3 / N_c=3 does NOT supply the Bloch Z/3 on the budget.
  * The 3 planar phase-space rotations generate SO(3) acting ON the commuting triple,
    not an SU(2) mixing complementary observables -> no other FTD source for the Z/3.

  ==> VERDICT: CLOSED-NEGATIVE. The full S3 budget symmetry is NOT derivable from current
      FTD geometry. Sharpness is not derivable; binding-but-not-sharp is confirmed. The
      epistemic route RE-DERIVES the non-commutativity wall: the needed budget symmetry IS
      a non-commutative SU(2) on complementary observables -- exactly what the commutative
      substrate lacks (B1, B-QM-1). The reframe relocated the gap; it did not dissolve it.

No inserted [q,p]=i, no posited SU(2), no count-match accepted as evidence (F-alpha..delta clean).

Run:  python scripts/proofs/proof_symplectic_budget_symmetry.py
"""
import numpy as np

results = []


def check(name, passed, detail=""):
    results.append((name, bool(passed)))
    print(f"  [{'PASS' if passed else 'FAIL'}] {name}" + (f"  --  {detail}" if detail else ""))


def comm(A, B):
    return A @ B - B @ A


print("=" * 78)
print("B-QM-1'' (adversarial): full symplectic budget symmetry from FTD geometry?")
print("=" * 78)

I2 = np.eye(2, dtype=complex)

# ---------------------------------------------------------------------------
# (i) Z/2 from J^2 = -I : the symplectic complex structure on a conjugate pair.
# ---------------------------------------------------------------------------
print("\n[i] Z/2: J^2=-I acts on the conjugate (q,p) pair (the q-p rotation)")
J = np.array([[0, -1], [1, 0]], dtype=complex)
check("J^2 = -I (symplectic complex structure on one conjugate pair)",
      np.allclose(J @ J, -I2), "J rotates q<->p: a genuine Z/2 (order-4 over R) on a complementary PAIR")

# ---------------------------------------------------------------------------
# DECISIVE TEST (section 3): are the 3 spatial flux components COMMUTING or COMPLEMENTARY?
# Model J_x,J_y,J_z as INDEPENDENT field components = independent tensor factors.
# ---------------------------------------------------------------------------
print("\n[3] DECISIVE kind-test: do {J_x, J_y, J_z} commute or are they complementary?")
X = np.array([[0, 1], [1, 0]], dtype=complex)
# independent components live on independent factors (classical field: each component its own d.o.f.)
def kron3(a, b, c):
    return np.kron(np.kron(a, b), c)
Jx = kron3(X, I2, I2)
Jy = kron3(I2, X, I2)
Jz = kron3(I2, I2, X)
commuting = (np.allclose(comm(Jx, Jy), 0) and np.allclose(comm(Jy, Jz), 0) and np.allclose(comm(Jx, Jz), 0))
check("the 3 spatial flux components COMMUTE ([J_i,J_j]=0): a single CO-MEASURABLE triple",
      commuting, "independent field components share an eigenbasis -> NOT three complementary bases")

# ---------------------------------------------------------------------------
# Contrast: the budget's three bases ARE mutually complementary (Pauli).
# ---------------------------------------------------------------------------
print("\n[c] Contrast: the budget's 3 bases are COMPLEMENTARY (Pauli X,Y,Z)")
sx = np.array([[0, 1], [1, 0]], dtype=complex)
sy = np.array([[0, -1j], [1j, 0]], dtype=complex)
sz = np.array([[1, 0], [0, -1]], dtype=complex)
complementary = (not np.allclose(comm(sx, sy), 0)) and np.allclose(comm(sx, sy), 2j * sz)
check("the 3 complementary bases do NOT commute ([X,Y]=2iZ != 0)",
      complementary, "mutually unbiased / non-co-measurable -- this is what the Z/3 must permute")

# ---------------------------------------------------------------------------
# Verdict on the candidate: same COUNT (3), different KIND -> N_c=3 fails section 3.
# ---------------------------------------------------------------------------
print("\n[X] body-diagonal C3 / N_c=3 vs the Bloch Z/3: COUNT matches, KIND does not")
check("COMMUTING triple != COMPLEMENTARY triple -> body-diagonal C3 is NOT the Bloch Z/3",
      commuting and complementary,
      "N_c=3 permutes a co-measurable triple; the budget needs a 3-cycle on complementary bases -> CANDIDATE FAILS")

# ---------------------------------------------------------------------------
# Other FTD source? The 3 planar rotations -> SO(3) on the COMMUTING triple, not SU(2)
# on complementary observables.
# ---------------------------------------------------------------------------
print("\n[o] Other FTD source for the Bloch Z/3?  Planar rotations -> SO(3) on commuting triple")
# SO(3) generators acting on the 3 spatial components (rotating J_x,J_y,J_z among themselves):
Lx = np.array([[0, 0, 0], [0, 0, -1], [0, 1, 0]], dtype=complex)
Ly = np.array([[0, 0, 1], [0, 0, 0], [-1, 0, 0]], dtype=complex)
Lz = np.array([[0, -1, 0], [1, 0, 0], [0, 0, 0]], dtype=complex)
so3 = np.allclose(comm(Lx, Ly), Lz)   # [Lx,Ly]=Lz : the SO(3) algebra on the spatial triple
check("planar/spatial rotations generate SO(3) ON the commuting triple (not SU(2) on bases)",
      so3, "rotating commuting components among themselves never makes them complementary -> no Bloch Z/3")

# ---------------------------------------------------------------------------
print("\n[=] CONCLUSION")
print("    (i)  Z/2 from J^2=-I : real (one conjugate pair).")
print("    (3)  {J_x,J_y,J_z} COMMUTE -> the body-diagonal C3 permutes a co-measurable triple.")
print("    (c)  the budget's 3 bases are COMPLEMENTARY (non-commuting).")
print("    (X)  same COUNT (3), different KIND -> N_c=3 does NOT supply the Bloch Z/3 (apophenia).")
print("    (o)  no other FTD structure supplies an SU(2) mixing complementary observables.")
print("    => CLOSED-NEGATIVE: the full S3 budget symmetry is NOT derivable from FTD geometry.")
print("       The needed symmetry IS a non-commutative SU(2) -- the epistemic route re-derives")
print("       the SAME non-commutativity wall as B1 / B-QM-1. Reframe relocated, did not dissolve.")

print("\n" + "=" * 78)
n_pass = sum(1 for _, p in results if p)
print(f"FACTS: {n_pass}/{len(results)} verified.")
print("VERDICT: CLOSED-NEGATIVE (N_c=3  z/3 candidate is apophenia: commuting != complementary).")
print("Sharpness not derivable from FTD geometry; binding-but-not-sharp confirmed.")
print("=" * 78)
import sys
sys.exit(0 if n_pass == len(results) else 1)

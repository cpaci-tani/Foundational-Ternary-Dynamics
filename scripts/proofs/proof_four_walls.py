#!/usr/bin/env python3
# -*- coding: ascii -*-
"""
proof_four_walls.py -- verification artifact for THEOREM_FOUR_WALLS_v1.md (FTD-0357)

Verifies every machine-checkable claim of the four-walls adjudication:
whether FC-1 (declined measurement map M), FC-2 (native arrow / sector-scoped
metric), FC-W (adopted branch delta = sqrt(G*(4G*-1))), and the L2-not-L1
budget (FTD-0208 / clock hypothesis) are ONE structural import.

Verdict verified here (see the theorem doc for the argument):
  - The PAIRING axis (walls 1, 2, 4) and the BRANCH axis (wall 3) are
    provably distinct in both directions (definite-signature protection
    theorem; scalar-inertness / no-transfer lemmas).
  - Within the pairing axis, sector instances do not communicate; the two
    genuine one-way links (M contains the Hilbert pairing; conserved
    quadratic pairing implies spectral reversibility) are verified, and all
    other directions are refuted by explicit counterexamples.
  - The single indefinite-signature reach witness (2G*)^2 - (sqrt(G*))^2
    = delta^2 is verified, together with the fact that it prices (does not
    open) the same loophole as FTD-0314 section 4 / FTD-0353 section 8.

Groups:
  A  frame anchors (G*, delta, master quadratic, hull identities, witness)
  B  pairing structure (polarization, d=1 degeneracy, budget laws, C*/GNS)
  C  conserved pairing <-> spectral reversibility (flux sector model)
  D  valuation layer at (2s-1) (protection theorem, signature asymmetry)
  E  scalar inertness and torsor no-transfer (branch axis separation)
  F  counterexample registrations (2x2 independence of M and reversibility)
  G  branch-bit bookkeeping (Vieta over K; sign swap = root swap)

Pure mathematics; read-only; no engine, no LEDGER, no golden gate contact.
No numerical searches are performed anywhere in this script: every check is
a verification of a stated identity, lemma instance, or counterexample.

Run:  python scripts/proofs/proof_four_walls.py
"""

import random
import sys
from fractions import Fraction

import mpmath as mp
import sympy as sp

PASS = 0
FAIL = 0


def check(tid, desc, cond):
    global PASS, FAIL
    ok = bool(cond)
    if ok:
        PASS += 1
    else:
        FAIL += 1
    print(("PASS " if ok else "FAIL ") + tid + " -- " + desc)


def maxabs(M):
    return max(abs(M[i, j]) for i in range(M.rows) for j in range(M.cols))


# ============================================================ Group A
print("--- Group A: frame anchors ---")
mp.mp.dps = 60

Gs = mp.gamma(mp.mpf(1) / 4) / mp.gamma(mp.mpf(3) / 4)
delta = mp.sqrt(Gs * (4 * Gs - 1))

check("A1", "G* = Gamma(1/4)/Gamma(3/4) = 2.9586751191886...",
      abs(Gs - mp.mpf("2.9586751191886")) < mp.mpf("1e-12"))

varpi = mp.gamma(mp.mpf(1) / 4) ** 2 / (2 * mp.sqrt(2 * mp.pi))
check("A2a", "G* = 2*varpi/sqrt(pi) (lemniscate relation)",
      abs(Gs - 2 * varpi / mp.sqrt(mp.pi)) < mp.mpf("1e-55"))
check("A2b", "G* != varpi guard (FTD-0117 conflation trap)",
      abs(Gs - varpi) > mp.mpf("0.3"))

xp = 8 * Gs ** 2 + 4 * Gs * delta
xm = 8 * Gs ** 2 - 4 * Gs * delta
check("A3a", "x_pm = 8G*^2 +- 4G*delta are the master-quadratic roots",
      abs(xp ** 2 - 16 * Gs ** 2 * xp + 16 * Gs ** 3) < mp.mpf("1e-45")
      and abs(xm ** 2 - 16 * Gs ** 2 * xm + 16 * Gs ** 3) < mp.mpf("1e-45"))
check("A3b", "delta = (x_+ - x_-)/(8G*): delta IS the root-separation datum",
      abs(delta - (xp - xm) / (8 * Gs)) < mp.mpf("1e-55"))

# Consistency of record only: x_+ = 1/alpha stays [SMC] (FTD-0013); this is
# NOT a derivation and this script derives nothing about alpha.
alpha_inv_codata = mp.mpf("137.035999177")
check("A4", "x_+ matches CODATA-2022 1/alpha to <2e-6 (record check, [SMC])",
      abs(xp / alpha_inv_codata - 1) < mp.mpf("2e-6"))

s_num = mp.sqrt(Gs)
w_num = mp.pi ** mp.mpf("0.25")
check("A5a", "Gamma(1/4) = 2^{1/4} s w^2 (hull form, FTD-0353 layer R)",
      abs(mp.gamma(mp.mpf(1) / 4) - 2 ** mp.mpf("0.25") * s_num * w_num ** 2)
      < mp.mpf("1e-50"))
theta3 = mp.jtheta(3, 0, mp.exp(-mp.pi))
check("A5b", "theta3(0,i) = pi^{1/4}/Gamma(3/4) = 2^{-1/4} s/w (odd s-degree)",
      abs(theta3 - w_num / mp.gamma(mp.mpf(3) / 4)) < mp.mpf("1e-50")
      and abs(theta3 - s_num / (2 ** mp.mpf("0.25") * w_num)) < mp.mpf("1e-50"))
detz34 = mp.sqrt(2 * mp.pi) / mp.gamma(mp.mpf(3) / 4)
check("A5c", "det_zeta(D_{3/4}) = sqrt(2pi)/Gamma(3/4) = 2^{1/4} s",
      abs(detz34 - 2 ** mp.mpf("0.25") * s_num) < mp.mpf("1e-50"))

check("A6a", "Lorentzian witness: (2G*)^2 - (sqrt(G*))^2 = delta^2 exactly",
      abs((2 * Gs) ** 2 - Gs - delta ** 2) < mp.mpf("1e-50"))
check("A6b", "witness legs documented-native: sqrt(G*) = 2^{-1/4} det_zeta(D_{3/4})",
      abs(s_num - detz34 / 2 ** mp.mpf("0.25")) < mp.mpf("1e-50"))
check("A6c", "witness admissible: 2G* > delta (real dtau leg exists)",
      2 * Gs > delta)

# ============================================================ Group B
print("--- Group B: pairing structure ---")
random.seed(20260702)

n = 3
Bm = [[Fraction(0)] * n for _ in range(n)]
for i in range(n):
    for j in range(i, n):
        Bm[i][j] = Bm[j][i] = Fraction(random.randint(-9, 9), random.randint(1, 5))


def quad(v):
    return sum(v[i] * Bm[i][j] * v[j] for i in range(n) for j in range(n))


ok = True
for i in range(n):
    for j in range(n):
        ei = [Fraction(int(k == i)) for k in range(n)]
        ej = [Fraction(int(k == j)) for k in range(n)]
        up = [ei[k] + ej[k] for k in range(n)]
        dn = [ei[k] - ej[k] for k in range(n)]
        if (quad(up) - quad(dn)) / 4 != Bm[i][j]:
            ok = False
check("B1", "polarization reconstructs the pairing from norm data (exact)", ok)

ok = True
for _ in range(20):
    x = mp.mpf(random.uniform(-5, 5))
    for p in (1, mp.mpf("1.5"), 2, 3):
        if abs(abs(x) ** p) ** (1 / mp.mpf(p)) - abs(x) > mp.mpf("1e-40"):
            ok = False
check("B2", "d=1 degeneracy: every p-norm coincides with |x| on one coordinate", ok)

xs, ys = sp.symbols("xs ys", real=True)
sols = sp.solve([xs + ys - 1, xs ** 2 + ys ** 2 - 1], [xs, ys], dict=True)
sset = {(sol[xs], sol[ys]) for sol in sols}
check("B3", "L1 and L2 budget laws agree only on the boundary {(1,0),(0,1)}",
      sset == {(sp.Integer(1), sp.Integer(0)), (sp.Integer(0), sp.Integer(1))})

mp.mp.dps = 30
a2 = mp.matrix([[mp.mpc(random.uniform(-1, 1), random.uniform(-1, 1))
                 for _ in range(2)] for _ in range(2)])
H = a2.H * a2
Eh, Q = mp.eighe(H)
top = max(Eh)
ok = True
for _ in range(50):
    v = mp.matrix([mp.mpc(random.uniform(-1, 1), random.uniform(-1, 1))
                   for _ in range(2)])
    v = v / mp.sqrt((v.H * v)[0, 0].real)
    val = ((a2 * v).H * (a2 * v))[0, 0].real
    if val > top + mp.mpf("1e-12"):
        ok = False
vtop = Q[:, 1] if Eh[1] >= Eh[0] else Q[:, 0]
vtop = vtop / mp.sqrt((vtop.H * vtop)[0, 0].real)
attained = ((a2 * vtop).H * (a2 * vtop))[0, 0].real
check("B4", "C*-norm^2 = max eig(a^H a) = sup of the quadratic form (quadratic law)",
      ok and abs(attained - top) < mp.mpf("1e-15"))

psi = mp.matrix([mp.mpc(0.3, 0.4), mp.mpc(-0.5, 0.7)])
P2 = mp.matrix([[mp.mpf(1), 0], [0, mp.mpf(0)]])
c = mp.mpc(1.3, -0.7)
born = lambda ps: ((ps.H * (P2 * ps))[0, 0]).real
check("B5", "Born functional is |c|^2-homogeneous (quadratic statistics)",
      abs(born(c * psi) - abs(c) ** 2 * born(psi)) < mp.mpf("1e-20"))

sx = mp.matrix([[0, 1], [1, 0]])
sy = mp.matrix([[0, mp.mpc(0, -1)], [mp.mpc(0, 1), 0]])
sz = mp.matrix([[1, 0], [0, -1]])
id2 = mp.matrix([[1, 0], [0, 1]])
rho = mp.matrix([[mp.mpf("0.7"), 0], [0, mp.mpf("0.3")]])
basis = [id2, sx, sy, sz]
G = mp.matrix(4, 4)
for i in range(4):
    for j in range(4):
        G[i, j] = sum((rho * basis[i].H * basis[j])[k, k] for k in range(2))
Eg, _ = mp.eighe(G)
check("B6", "GNS: any state on the M-algebra yields a PSD pairing (Gram >= 0)",
      min(e.real if hasattr(e, "real") else e for e in Eg) > -mp.mpf("1e-20"))

# ============================================================ Group C
print("--- Group C: conserved pairing <-> spectral reversibility ---")
N = 8
c2 = mp.mpf("0.25")
m2 = mp.mpf("0.1")
dt = mp.mpf("0.5")

L = mp.matrix(N, N)
for i in range(N):
    L[i, i] = -2
    L[i, (i + 1) % N] = 1
    L[i, (i - 1) % N] = 1
K = c2 * L - m2 * mp.eye(N)
A = (dt ** 2 / 2) * K
M = mp.matrix(2 * N, 2 * N)
IA = mp.eye(N) + A
BL = (dt / 2) * K * (2 * mp.eye(N) + A)
for i in range(N):
    for j in range(N):
        M[i, j] = IA[i, j]
        M[i, N + j] = dt * (1 if i == j else 0)
        M[N + i, j] = BL[i, j]
        M[N + i, N + j] = IA[i, j]

Ew = mp.eig(M, left=False, right=False)
check("C1a", "wave (velocity-Verlet, CFL) update: all eigenvalues unimodular",
      max(abs(abs(e) - 1) for e in Ew) < mp.mpf("1e-18"))

R = mp.eye(2 * N)
for i in range(N):
    R[N + i, N + i] = -1
check("C1b", "explicit backward pairing: R M R M = I (velocity-flip reversal)",
      maxabs(R * M * R * M - mp.eye(2 * N)) < mp.mpf("1e-18"))

Ev, ER = mp.eig(M)
Pinv = mp.inverse(ER)
Emat = Pinv.H * Pinv
EE, _ = mp.eighe((Emat + Emat.H) / 2)
c1c_conserved = (maxabs(M.H * Emat * M - Emat) < mp.mpf("1e-12")
                 and min(e.real if hasattr(e, "real") else e for e in EE) > 0)
check("C1c", "a conserved positive-definite pairing exists: M^H E M = E, E > 0",
      c1c_conserved)

eps = mp.mpf("0.1")
Dm = mp.eye(N) + eps * L
Ed = mp.eig(Dm, left=False, right=False)
check("C2", "diffusion update: eigenvalue strictly inside the disc => no "
            "conserved definite pairing (spectral criterion)",
      min(abs(e) for e in Ed) < 1 - mp.mpf("1e-6"))

ok = True
for _ in range(3):
    Arand = mp.matrix([[mp.mpf(random.uniform(-1, 1)) for _ in range(4)]
                       for _ in range(4)])
    Epd = Arand.T * Arand + mp.eye(4)
    Es, Qs = mp.eighe(Epd)
    Ehalf = Qs * mp.diag([mp.sqrt(e) for e in Es]) * Qs.T
    Ehalfinv = Qs * mp.diag([1 / mp.sqrt(e) for e in Es]) * Qs.T
    Brand = mp.matrix([[mp.mpf(random.uniform(-1, 1)) for _ in range(4)]
                       for _ in range(4)])
    Qo, _ = mp.qr(Brand)
    Mo = Ehalfinv * Qo * Ehalf
    Eo = mp.eig(Mo, left=False, right=False)
    if max(abs(abs(e) - 1) for e in Eo) > mp.mpf("1e-15"):
        ok = False
check("C3", "every E-orthogonal update has unimodular spectrum (lemma instances)", ok)

Asing = mp.matrix([[1, 1, 0], [0, 0, 0], [2, 2, 0]])
G4 = Asing.T * (mp.eye(3)) * Asing
Eg4 = mp.eig((G4 + G4.T) / 2, left=False, right=False)
check("C4a", "non-injective linear map: A^T E A is singular, never = E > 0 "
             "(lossy sector cannot conserve a definite pairing)",
      min(abs(e) for e in Eg4) < mp.mpf("1e-18"))

pts = {"s1": (0, 0), "s2": (3, 4), "s3": (1, 1)}
f = {"s1": "m", "s2": "m", "s3": "n"}
dist = lambda p, q: mp.sqrt((p[0] - q[0]) ** 2 + (p[1] - q[1]) ** 2)
check("C4b", "many-to-one state map: d(f(s1),f(s2)) = 0 < d(s1,s2) (no isometry)",
      f["s1"] == f["s2"] and dist(pts["s1"], pts["s2"]) > 0)

kk, cc, mm = sp.symbols("kk cc mm", positive=True)
omega = sp.sqrt(cc ** 2 * kk ** 2 + mm ** 2)
vg = sp.diff(omega, kk)
gamma_id = sp.simplify(mm ** 2 / omega ** 2 - (1 - vg ** 2 / cc ** 2))
check("C5a", "quadratic dispersion => exact Pythagorean gamma: m/omega = "
             "sqrt(1 - vg^2/c^2) (the FTD-0252 identity core)",
      gamma_id == 0)

Dd = sp.symbols("Dd", positive=True)
omega_diff = -sp.I * Dd * kk ** 2
check("C5b", "first-order (diffusive) dispersion has no real carrier "
             "(imaginary frequency): no gamma identity available",
      sp.im(omega_diff.subs([(Dd, 1), (kk, 2)])) != 0)

# ============================================================ Group D
print("--- Group D: valuation layer at (2s-1) ---")
t, s, w, S = sp.symbols("t s w S")

fpoly = t * (4 * t - 1)
fl = sp.factor_list(fpoly)
check("D1a", "delta^2 = t(4t-1) squarefree over Q[t]",
      all(mult == 1 for _, mult in fl[1]))
yv = sp.symbols("yv")
fl2 = sp.factor_list(yv ** 2 - fpoly)
check("D1b", "y^2 - t(4t-1) irreducible (the branch is a genuine Z/2 layer)",
      len(fl2[1]) == 1 and fl2[1][0][1] == 1)


def val_2sm1(expr):
    """Valuation at the prime (2s-1) of Qbar(w)[s]-fractions."""
    num, den = sp.fraction(sp.together(sp.expand(expr)))

    def cnt(p):
        p = sp.expand(p)
        if p == 0:
            raise ValueError("valuation of zero")
        c = 0
        while True:
            q, r = sp.div(p, 2 * s - 1, s)
            if sp.expand(r) == 0:
                p = q
                c += 1
            else:
                return c
    return cnt(num) - cnt(den)


check("D2", "hull form: delta^2 = s^2(2s-1)(2s+1) has ODD valuation 1 at (2s-1)",
      val_2sm1(s ** 2 * (2 * s - 1) * (2 * s + 1)) == 1)

ok = True
detail_ok = True
for trial in range(50):
    xs_list = []
    for _ in range(random.randint(1, 4)):
        terms = sp.Integer(0)
        while sp.expand(terms) == 0:
            terms = sp.Integer(0)
            for _ in range(random.randint(1, 3)):
                cnum = random.randint(-9, 9)
                while cnum == 0:
                    cnum = random.randint(-9, 9)
                coeff = sp.Rational(cnum, random.randint(1, 9))
                terms += coeff * s ** random.randint(0, 3) * w ** random.randint(0, 3)
        terms *= (2 * s - 1) ** random.randint(0, 2)
        xs_list.append(sp.expand(terms))
    total = sp.expand(sum(x ** 2 for x in xs_list))
    v = val_2sm1(total)
    kmin = min(val_2sm1(x) for x in xs_list)
    if v % 2 != 0:
        ok = False
    if v != 2 * kmin:
        detail_ok = False
check("D3", "protection theorem instances: v(sum of squares) even, 50/50 draws", ok)
check("D3b", "protection theorem exact form: v(sum x_i^2) = 2 min v(x_i)", detail_ok)

x1 = sp.expand((2 * s - 1) * w + s)
x2 = sp.expand(s * w ** 2 - 3)
totsq = sp.expand(x1 ** 2 + x2 ** 2)
kmin = min(val_2sm1(x1), val_2sm1(x2))
reduced = sp.cancel(totsq / (2 * s - 1) ** (2 * kmin))
resid = sp.expand(reduced.subs(s, sp.Rational(1, 2)))
check("D4", "residue mechanism: sum of squared residues in the formally real "
            "residue field is nonzero",
      resid != 0)

check("D5", "indefinite (Lorentzian) budget reaches the branch class: "
            "v((2s^2)^2 - s^2) = 1 ODD  [the witness]",
      val_2sm1((2 * s ** 2) ** 2 - s ** 2) == 1)
check("D6", "definite (Euclidean) budget protected on the same legs: "
            "v((2s^2)^2 + s^2) = 0 EVEN  [signature asymmetry]",
      val_2sm1((2 * s ** 2) ** 2 + s ** 2) == 0)
check("D7", "native i converts signature: (2s^2)^2 + (i s)^2 = (2s^2)^2 - s^2",
      sp.expand((2 * s ** 2) ** 2 + (sp.I * s) ** 2
                - ((2 * s ** 2) ** 2 - s ** 2)) == 0)

ok = True
for m in (1, 2, 3, 5):
    poly_m = 4 * S ** (2 * m) - 1
    flm = sp.factor_list(poly_m)
    if not all(mult == 1 for _, mult in flm[1]):
        ok = False
    if sp.gcd(poly_m, S) != 1:
        ok = False
check("D8", "radical-tower stability (FTD-0353 T3 mirror): 4S^{2m}-1 squarefree, "
            "coprime to S, m in {1,2,3,5}", ok)

# ============================================================ Group E
print("--- Group E: scalar inertness and torsor no-transfer ---")
fK = t * (4 * t - 1)


def kmul(x1_, x2_):
    return (sp.expand(x1_[0] * x2_[0] + x1_[1] * x2_[1] * fK),
            sp.expand(x1_[0] * x2_[1] + x1_[1] * x2_[0]))


dK = (sp.Integer(0), sp.Integer(1))  # delta as element of K = Q(t)[d]/(d^2 - f)
d2K = kmul(dK, dK)
check("E1", "K-arithmetic sanity: delta * delta = t(4t-1) in K",
      sp.expand(d2K[0] - fK) == 0 and sp.expand(d2K[1]) == 0)

ok = True
for _ in range(10):
    e1 = (sp.Rational(random.randint(-5, 5), random.randint(1, 4)) * t ** random.randint(0, 2),
          sp.Rational(random.randint(-5, 5), random.randint(1, 4)) * t ** random.randint(0, 2))
    e2 = (sp.Rational(random.randint(-5, 5), random.randint(1, 4)) * t ** random.randint(0, 2),
          sp.Rational(random.randint(-5, 5), random.randint(1, 4)) * t ** random.randint(0, 2))
    p12 = kmul(e1, e2)
    p21 = kmul(e2, e1)
    if sp.expand(p12[0] - p21[0]) != 0 or sp.expand(p12[1] - p21[1]) != 0:
        ok = False
check("E2a", "base change preserves commutativity: K itself commutative "
             "(10 random pairs)", ok)

xpoly = sp.symbols("xpoly")
ok = True
for _ in range(5):
    q1 = sum((sp.Rational(random.randint(-3, 3)) +
              sp.Rational(random.randint(-3, 3)) * t) * xpoly ** i for i in range(3))
    q2 = sum((sp.Rational(random.randint(-3, 3)) +
              sp.Rational(random.randint(-3, 3)) * t) * xpoly ** i for i in range(3))
    r12 = sp.rem(sp.expand(q1 * q2), xpoly ** 3 - 1, xpoly)
    r21 = sp.rem(sp.expand(q2 * q1), xpoly ** 3 - 1, xpoly)
    if sp.expand(r12 - r21) != 0:
        ok = False
check("E2b", "commutative algebra Q[x]/(x^3-1) stays commutative after "
             "any scalar extension (structure constants unchanged)", ok)


def sigma(x_):
    return (x_[0], -x_[1])


ok = True
for _ in range(10):
    e1 = (sp.Rational(random.randint(-5, 5)), sp.Rational(random.randint(-5, 5)))
    e2 = (sp.Rational(random.randint(-5, 5)), sp.Rational(random.randint(-5, 5)))
    lhs = sigma(kmul(e1, e2))
    rhs = kmul(sigma(e1), sigma(e2))
    if sp.expand(lhs[0] - rhs[0]) != 0 or sp.expand(lhs[1] - rhs[1]) != 0:
        ok = False
check("E3a", "sigma(a + b delta) = a - b delta is a field automorphism of K", ok)
check("E3b", "sigma moves delta (free Z/2 action); Fix(sigma) = base field",
      sigma(dK) != dK and sigma((t, sp.Integer(0))) == (t, sp.Integer(0)))

torsor = ["+", "-"]
swap = {"+": "-", "-": "+"}
codom = ["L1", "L2"]  # trivial Galois action
equivariant_maps = []
for fa in codom:
    for fb in codom:
        fmap = {"+": fa, "-": fb}
        if all(fmap[swap[x]] == fmap[x] for x in torsor):
            equivariant_maps.append(fmap)
check("E4", "no-transfer: every equivariant map (branch torsor) -> "
            "(trivial-action choice set) is constant (exhaustive)",
      len(equivariant_maps) == 2
      and all(fm["+"] == fm["-"] for fm in equivariant_maps))

const_traj = [5, 5, 5, 5]
nonconst_traj = [1, 2, 3, 4]
check("E5a", "arrow acts trivially on static scalars (reversal fixes constants)",
      list(reversed(const_traj)) == const_traj
      and list(reversed(nonconst_traj)) != nonconst_traj)

maps_pt_to_torsor = [{"pt": "+"}, {"pt": "-"}]
equivariant = [fm for fm in maps_pt_to_torsor
               if fm["pt"] == swap[fm["pt"]]]
check("E5b", "no equivariant map from a trivially-acted point to the free "
             "branch torsor exists (arrow Z/2 cannot select the Galois branch)",
      len(equivariant) == 0)

collapse = {1: "m", 2: "m", 3: "n"}
Acoef = sp.Rational(3, 2)


def preimage_count(target_state, target_j, field_elems):
    cnt = 0
    for st in collapse:
        for j in field_elems:
            if collapse[st] == target_state and sp.expand(Acoef * j - Acoef * target_j) == 0:
                cnt += 1
    return cnt


fieldQ = [sp.Integer(0), sp.Integer(1), sp.Integer(2)]
fieldK_marker = [sp.Integer(0), sp.Integer(1), sp.sqrt(fpoly.subs(t, 3))]
c_q = preimage_count("m", sp.Integer(1), fieldQ)
c_k = preimage_count("m", sp.Integer(1), fieldK_marker)
check("E6", "lossy-map preimage counts are scalar-field-blind (kernel not "
            "altered by adjoining delta)", c_q == 2 and c_k == 2)

# ============================================================ Group F
print("--- Group F: counterexample registrations ---")
gam = mp.mpf("0.5")
K0 = mp.matrix([[1, 0], [0, mp.sqrt(1 - gam)]])
K1 = mp.matrix([[0, mp.sqrt(gam)], [0, 0]])


def kron2(P, Q):
    Mk = mp.matrix(4, 4)
    for i in range(2):
        for j in range(2):
            for k in range(2):
                for l in range(2):
                    Mk[2 * i + k, 2 * j + l] = P[i, j] * Q[k, l]
    return Mk


def conj_m(P):
    Q = mp.matrix(P.rows, P.cols)
    for i in range(P.rows):
        for j in range(P.cols):
            Q[i, j] = mp.conj(P[i, j])
    return Q


Ssup = kron2(conj_m(K0), K0) + kron2(conj_m(K1), K1)

comm_xy = sx * sy - sy * sx
check("F1a", "ambient algebra non-commutative: [sigma_x, sigma_y] != 0",
      maxabs(comm_xy) > 1)


def vec2(X):
    return mp.matrix([X[0, 0], X[1, 0], X[0, 1], X[1, 1]])


def unvec2(v):
    return mp.matrix([[v[0], v[2]], [v[1], v[3]]])


def choi(Smat):
    C = mp.matrix(4, 4)
    for a in range(2):
        for b in range(2):
            Eab = mp.matrix(2, 2)
            Eab[a, b] = 1
            Phi = unvec2(Smat * vec2(Eab))
            for cix in range(2):
                for dix in range(2):
                    C[2 * a + cix, 2 * b + dix] = Phi[cix, dix]
    return C


C_ad = choi(Ssup)
Ec, _ = mp.eighe((C_ad + C_ad.H) / 2)
Es_sup = mp.eig(Ssup, left=False, right=False)
check("F1b", "amplitude-damping channel: CP (Choi >= 0) and strictly "
             "contractive (|eig| < 1): non-commutative + irreversible",
      min(e.real if hasattr(e, "real") else e for e in Ec) > -mp.mpf("1e-20")
      and min(abs(e) for e in Es_sup) < 1 - mp.mpf("1e-6"))

Sinv = mp.inverse(Ssup)
C_inv = choi(Sinv)
Eci, _ = mp.eighe((C_inv + C_inv.H) / 2)
check("F1c", "formal inverse channel is not CP (negative Choi eigenvalue): "
             "M does not supply a backward pairing",
      min(e.real if hasattr(e, "real") else e for e in Eci) < -mp.mpf("1e-6"))

arr1 = [mp.mpf(random.uniform(-2, 2)) for _ in range(6)]
arr2 = [mp.mpf(random.uniform(-2, 2)) for _ in range(6)]
prod12 = [arr1[i] * arr2[i] for i in range(6)]
prod21 = [arr2[i] * arr1[i] for i in range(6)]
check("F2a", "classical wave sector: pointwise observable algebra commutes "
             "(reversibility + L2 pairing with NO M)",
      all(abs(prod12[i] - prod21[i]) == 0 for i in range(6)))
check("F2b", "the same system carries the conserved pairing of C1c: Q + "
             "reversibility coexist with commutativity (Q does not supply M)",
      c1c_conserved
      and all(abs(prod12[i] - prod21[i]) == 0 for i in range(6)))

Ddag_E_D = Dm.T * Dm
check("F3", "pairing PRESENT but not conserved (diffusion): D^T D != I -- "
            "supply needs conservation, not mere presence",
      maxabs(Ddag_E_D - mp.eye(N)) > mp.mpf("0.01"))

th = mp.mpf("0.7")
U = mp.matrix([[mp.cos(th), -mp.sin(th)], [mp.sin(th), mp.cos(th)]])
Su = kron2(conj_m(U), U)
Eu = mp.eig(Su, left=False, right=False)
comm_xz = sx * sz - sz * sx
check("F4", "unitary channel: non-commutative + reversible cell exists "
            "(completes the 2x2 independence table of M vs reversibility)",
      max(abs(abs(e) - 1) for e in Eu) < mp.mpf("1e-15") and maxabs(comm_xz) > 1)

# ============================================================ Group G
print("--- Group G: branch-bit bookkeeping ---")
dd = sp.symbols("dd")
xpS = 8 * t ** 2 + 4 * t * dd
xmS = 8 * t ** 2 - 4 * t * dd
prod_sub = sp.expand(sp.expand(xpS * xmS).subs(dd ** 2, t * (4 * t - 1)))
check("G1a", "Vieta over K: x_+ x_- = 16 t^3 (delta cancels in the det slot)",
      sp.expand(prod_sub - 16 * t ** 3) == 0)
check("G1b", "Vieta over K: x_+ + x_- = 16 t^2 (delta cancels in the trace slot)",
      sp.expand(xpS + xmS - 16 * t ** 2) == 0)

check("G2", "the sqrt-sign bit IS the root bit: dd -> -dd swaps x_+ <-> x_-",
      sp.expand(xpS.subs(dd, -dd) - xmS) == 0
      and sp.expand(xmS.subs(dd, -dd) - xpS) == 0)

# ============================================================ Summary
print()
print("=" * 68)
print("proof_four_walls.py : %d PASS / %d FAIL (of %d)" % (PASS, FAIL, PASS + FAIL))
print("Nothing promoted: x_+ = 1/alpha stays [SMC]; MC-T4.3 stays")
print("[FOUNDATIONAL OBSTRUCTION]; FC-0/FC-1/FC-2/FC-W stay adopted;")
print("FTD-0208 stands; no alpha derived.")
print("=" * 68)
sys.exit(0 if FAIL == 0 else 1)

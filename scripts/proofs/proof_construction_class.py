"""
proof_construction_class.py -- checkable layer for FTD-0358, the closure of the
FTD-0244/FTD-0347 generator-representativeness flag
(ANALYSIS_CONSTRUCTION_CLASS_CLOSURE.md).

Verdict being verified (route (b) NO-GO discharging into route (c) declaration,
with a sector-locked salvage of route (a)):

  A sector-neutral admissible construction class -- one drawn from the five
  postulates alone, treating O_h-conjugate / postulate-equal symmetry data
  uniformly -- provably contains analytic outputs in THREE distinct CM sectors:

     BCC Moore layer  /  C4 (order-4, <100>) characters  /  theta at tau=i
         -> d = -4 sector:  the Gamma(1/4)-line  =  the FTD-0353 hull
            N~ = Qbar(pi^(1/4), sqrt(G*))
     FCC Moore layer  /  C3 (order-3, <111>) characters  /  theta at Q(sqrt-3) CM
         -> d = -3 sector:  the Gamma(1/3)-line
     SC  Moore layer
         -> disc -24 (h=2) sector:  the Gamma(1/24)-family line

  Hence hull-boundedness of the sector-neutral class ("every output lands in
  N~") is EQUIVALENT to Gamma(1/3)-containments that would force
  trdeg_Q Q(pi, Gamma(1/4), Gamma(1/3)) <= 2 -- contradicting the standard
  expectation and OPEN either way beyond Chudnovsky 1976 / Nesterenko 1996.
  Route (a) (prove representativeness + hull-boundedness sector-neutrally) is
  therefore CLOSED.  Every hull-bounded class containing the documented
  generator set breaks sector-neutrality by exactly one bit: the selection of
  the d = -4 datum -- which is FC-0 (the Z[i] reading of the order-4 planar
  symmetry), an already-declared [AXIOM]-class commitment (FTD-0254 s1.2).
  Sector-locked at FC-0, the five constructor families close into the
  radical-closed hull N~_rad = U_m Qbar(pi^(1/4m), G*^(1/2m)), where delta =
  sqrt(G*(4G*-1)) is already excluded by FTD-0353 Theorem 3 -- upgrading the
  FTD-0353 inventory-completeness premise from a 14-row list [SELECTION] to a
  constructor-closure characterization [THEOREM given FC-0 + the declared
  constructor basis], still conditional on Chudnovsky 1976.

Machine-checkable layer in this script:
  0. setup regressions (G*, reflection, master-quadratic roots, delta)
  A. Moore trichotomy: 26 = 6+12+8 as O_h-orbits; structure-function
     identities; BCC is the UNIQUE multiplicative (rank-1 separable) layer;
     O_h contains both order-4 and order-3 rotations (both readings available)
  B. Watson sector trichotomy: all three layer self-energies computed by
     independent 2D-reduced quadrature and matched to their classical closed
     forms (Watson 1939; Glasser--Zucker 1977):
       G_BCC(0) = Gamma(1/4)^4/(4 pi^3) = G*^2/(2 pi)      [d=-4, IN hull]
       G_FCC(0) = 9 Gamma(1/3)^6/(2^(14/3) pi^4)           [d=-3 line]
       G_SC(0)  = sqrt6 GGGG(1,5,7,11/24)/(32 pi^3)        [disc -24 line]
  C. character-twisted zeta-determinants: Lerch det_zeta(D_a) = sqrt(2pi)/
     Gamma(a) verified by Hurwitz-zeta differentiation for a = 1/4 (C4
     character, hull) AND a = 1/3 (C3 character, Gamma(1/3)-line); the
     scaling anomaly zeta_H(0,a) = 1/2 - a is RATIONAL (closure lands in the
     radical tower N~_rad, not bare N~); C6 closes into the d=-3 line via
     Gauss duplication
  D. PSLQ evidence layer (dps 400, heights per the FTD-0351/0353 spurious-
     PSLQ rule): positive controls (reflection; duplication) fire; theta3 at
     tau=i is the hull monomial; theta3 at tau=i*sqrt3 lands on the
     Gamma(1/3)-line (classical singular value recovered, height <= 36) and
     has NO d=-4-hull monomial form; Gamma(1/3) itself has NO monomial
     relation over {pi, Gamma(1/4), 2, 3} at maxcoeff 1e6  [EVIDENCE, not
     proof -- the proof either way is exactly the open question]
  E. dichotomy bookkeeping + conditional delta-stability: W_FCC's prefactor
     is a hull unit (so W_FCC in N~ <=> Gamma(1/3)^6 in N~); in the
     THREE-variable model Q(w, y)(s) (y = a hypothetical independent d=-3
     coordinate) delta^2 = s^2(2s-1)(2s+1) still has odd multiplicity at
     (2s+-1), delta is still not a square, and radical towers still miss it
     -- i.e. the d=-3 enlargement leaves K-BIND intact CONDITIONAL on joint
     independence of {pi, Gamma(1/4), Gamma(1/3)} (OPEN); regression of the
     FTD-0353 branch-locus identity 4 Gamma(1/4)^2 - sqrt2 pi = sqrt2 pi (4G*-1)
  F. FC-0-locked constructor-closure regressions: the documented outputs are
     Qbar-monomials in (s, w) (det_zeta line, Watson BCC, theta-nulls at i,
     eta-invariant rationals, AGM(1,sqrt2), lemniscate, CM period Omega) and
     an explicit composite of four constructor outputs is again a monomial
  G. locating the import: |mu_K| = |disc K| holds uniquely at Q(i) among
     imaginary quadratic fields (the arithmetic motivation for FC-0's d=-4
     selection -- motivation, NOT forcing); INFO: the engine stencil's own
     ((SC+FCC)/2) self-energy has no documented closed form (printed, no
     claim)

NOT machine-checkable and NOT claimed: Chudnovsky 1976 itself; joint
algebraic independence of {pi, Gamma(1/4), Gamma(1/3)} (OPEN -- the script
treats it as the named conditionality, never as a fact); the adequacy of the
declared constructor basis to "everything the substrate can construct"
(that adequacy is the [SELECTION -- declared] this closure permanently
classifies, with the single falsifier: a forced native output with odd
valuation at a prime over (4G*-1)).

Read-only pure mathematics; golden gate untouched (0xb604d81a3d79366e).
No LEDGER / META_INDEX / tracker edits are performed by this script or its doc.
"""

import itertools
import time

import sympy as sp
from mpmath import (mp, mpf, gamma, sqrt, pi, quad, cos, sin, exp, log,
                    jtheta, zeta, diff, pslq, agm)

mp.dps = 60

G = gamma(mpf(1)/4) / gamma(mpf(3)/4)          # G* = 2.9586751191...
S = sqrt(G)                                     # s = sqrt(G*)
W = pi ** (mpf(1)/4)                            # w = pi^(1/4)
R4 = mpf(2) ** (mpf(1)/4)
DELTA = sqrt(G * (4*G - 1))
TOL = mpf(10) ** -48
QUAD_TOL = mpf(10) ** -18                       # quads run at dps 25

checks = []


def check(name, ok, detail=""):
    checks.append((name, bool(ok)))
    print(("  [PASS] " if ok else "  [FAIL] ") + name +
          ((" -- " + detail) if detail else ""))


def close(a, b, tol=TOL):
    return abs(a - b) < tol


print("=" * 92)
print("FTD-0358: construction-class closure of the generator-representativeness flag")
print("G*    = " + mp.nstr(G, 30))
print("delta = " + mp.nstr(DELTA, 30))
print("=" * 92)

# ------------------------------------------------------------------ 0. setup
print("\n0. Setup regressions")
check("0.1 reflection: G* == Gamma(1/4)^2/(pi sqrt2)",
      close(G, gamma(mpf(1)/4)**2 / (pi*sqrt(mpf(2)))))
x_p = 8*G**2 + 4*G*DELTA
x_m = 8*G**2 - 4*G*DELTA
check("0.2 x_pm = 8G*^2 +- 4G* delta solve x^2 - 16G*^2 x + 16G*^3 = 0",
      close(x_p**2 - 16*G**2*x_p + 16*G**3, 0) and
      close(x_m**2 - 16*G**2*x_m + 16*G**3, 0))
check("0.3 hull coordinates: Gamma(1/4) == 2^(1/4) s w^2 ;  pi == w^4",
      close(gamma(mpf(1)/4), R4*S*W**2) and close(pi, W**4))

# ------------------------------------------- A. Moore trichotomy (P4 + O_h)
print("\nA. The Moore trichotomy is postulate-forced and BCC is the unique "
      "multiplicative layer")

moore = [v for v in itertools.product((-1, 0, 1), repeat=3) if v != (0, 0, 0)]
orbits = {1: [], 2: [], 3: []}
for v in moore:
    orbits[sum(c*c for c in v)].append(v)
check("A1 |Moore| = 26 partitions by |v|^2 into 6 (SC) + 12 (FCC) + 8 (BCC)",
      len(moore) == 26 and len(orbits[1]) == 6 and len(orbits[2]) == 12
      and len(orbits[3]) == 8)

# O_h as the 48 signed permutation matrices
perms = list(itertools.permutations((0, 1, 2)))
signs = list(itertools.product((-1, 1), repeat=3))
o_h = [(p, sg) for p in perms for sg in signs]


def act(g, v):
    p, sg = g
    return tuple(sg[i] * v[p[i]] for i in range(3))


ok_orbits = True
for n, orb in orbits.items():
    base = orb[0]
    reached = {act(g, base) for g in o_h}
    ok_orbits &= (reached == set(orb))
check("A2 each layer is a single O_h-orbit (closure + transitivity, |O_h| = 48)",
      ok_orbits and len(o_h) == 48)

# structure functions: (1/|orbit|) sum e^{i k.v}
k0 = (mpf('0.71'), mpf('1.13'), mpf('2.09'))


def struct(orb, k):
    tot = mpf(0)
    for v in orb:
        tot += cos(sum(vv*kk for vv, kk in zip(v, k)))  # imaginary parts cancel pairwise
    return tot / len(orb)


cx, cy, cz = cos(k0[0]), cos(k0[1]), cos(k0[2])
check("A3a lambda_SC  == (cx+cy+cz)/3", close(struct(orbits[1], k0), (cx+cy+cz)/3))
check("A3b lambda_FCC == (cx cy + cy cz + cz cx)/3",
      close(struct(orbits[2], k0), (cx*cy + cy*cz + cz*cx)/3))
check("A3c lambda_BCC == cx cy cz", close(struct(orbits[3], k0), cx*cy*cz))

# BCC is the unique separable (rank-1) structure function
xs = [mpf('0.3'), mpf('0.9'), mpf('1.7')]
z0 = mpf('0.7')


def max_minor(lam):
    m = [[lam(x, y, z0) for y in xs] for x in xs]
    best = mpf(0)
    for i1, i2 in itertools.combinations(range(3), 2):
        for j1, j2 in itertools.combinations(range(3), 2):
            best = max(best, abs(m[i1][j1]*m[i2][j2] - m[i1][j2]*m[i2][j1]))
    return best


lam_sc = lambda x, y, z: (cos(x)+cos(y)+cos(z))/3
lam_fcc = lambda x, y, z: (cos(x)*cos(y)+cos(y)*cos(z)+cos(z)*cos(x))/3
lam_bcc = lambda x, y, z: cos(x)*cos(y)*cos(z)
check("A4a lambda_BCC is rank-1 separable (all 2x2 minors vanish)",
      max_minor(lam_bcc) < TOL)
check("A4b lambda_SC is NOT separable (a 2x2 minor is nonzero)",
      max_minor(lam_sc) > mpf('1e-3'))
check("A4c lambda_FCC is NOT separable (a 2x2 minor is nonzero)",
      max_minor(lam_fcc) > mpf('1e-3'))

# both symmetry readings live in O_h: order-4 (<100>) and order-3 (<111>) axes
r4 = sp.Matrix([[0, -1, 0], [1, 0, 0], [0, 0, 1]])   # C4 about z
r3 = sp.Matrix([[0, 0, 1], [1, 0, 0], [0, 1, 0]])    # C3 about (1,1,1)
check("A5 O_h contains an order-4 axis (C4, Z[i] reading) AND an order-3 axis "
      "(C3, Z[omega] reading)",
      r4**4 == sp.eye(3) and r4**2 != sp.eye(3)
      and r3**3 == sp.eye(3) and r3 != sp.eye(3))

# --------------------------- B. Watson sector trichotomy (classical, verified)
print("\nB. The three layer self-energies land in three distinct CM sectors")
print("   (2D-reduced quadratures at dps 25 vs classical closed forms)")

mp.dps = 25
G13 = gamma(mpf(1)/3)
G14 = gamma(mpf(1)/4)
W_BCC_cf = G14**4 / (4*pi**3)
W_FCC_cf = 9*G13**6 / (2**(mpf(14)/3) * pi**4)
GGGG = (gamma(mpf(1)/24)*gamma(mpf(5)/24)*gamma(mpf(7)/24)*gamma(mpf(11)/24))
W_SC_cf = sqrt(6)*GGGG / (32*pi**3)


def omcc(y, z):
    """1 - cos y cos z, cancellation-stable near (0,0)."""
    return 2*sin(y/2)**2 + cos(y)*2*sin(z/2)**2


# x-reduction identity (1/pi) int_0^pi dx/(A - B cos x) = 1/sqrt(A^2 - B^2)
A0, B0 = mpf(2), mpf('0.7')
red = quad(lambda x: 1/(A0 - B0*cos(x)), [0, pi]) / pi
check("B1 x-reduction identity (1/pi)int dx/(A - B cos x) == 1/sqrt(A^2-B^2)",
      close(red, 1/sqrt(A0**2 - B0**2), mpf(10)**-20))

t0 = time.time()
I_bcc = 4/pi**2 * quad(lambda y, z: 1/sqrt(omcc(y, z)*(1 + cos(y)*cos(z))),
                       [0, pi/2], [0, pi/2])
check("B2 G_BCC(0) quadrature == Gamma(1/4)^4/(4 pi^3)   [d=-4: IN the hull]",
      close(I_bcc, W_BCC_cf, QUAD_TOL), "%.1fs" % (time.time()-t0))
check("B3 G_BCC(0) == G*^2/(2 pi) == s^4/(2 w^4)  (hull monomial, OT-2.1)",
      close(W_BCC_cf, gamma(mpf(1)/4)**4/(4*pi**3))
      and close(I_bcc, (gamma(mpf(1)/4)/gamma(mpf(3)/4))**2/(2*pi), QUAD_TOL))


def f_fcc(y, z):
    cy, czv = cos(y), cos(z)
    amb = 2*sin(y/2)**2 + 2*sin(z/2)**2 + omcc(y, z)
    apb = 3 - cy*czv + cy + czv
    pp = 1/sqrt(amb*apb)
    pm = 1/sqrt((3 + cy*czv)**2 - (cy - czv)**2)
    return pp + pm


t0 = time.time()
I_fcc = 3 * (2/pi**2) * quad(f_fcc, [0, pi/2], [0, pi/2])   # G_FCC(0) = 3 I_F
check("B4 G_FCC(0) quadrature == 9 Gamma(1/3)^6/(2^(14/3) pi^4)   [d=-3 line]",
      close(I_fcc, W_FCC_cf, QUAD_TOL), "%.1fs" % (time.time()-t0))


def f_sc(y, z):
    cy, czv = cos(y), cos(z)
    am1 = 2*sin(y/2)**2 + 2*sin(z/2)**2
    tot = 1/sqrt(am1*(4 - cy - czv))
    for a in (3 - cy + czv, 3 + cy - czv, 3 + cy + czv):
        tot += 1/sqrt((a - 1)*(a + 1))
    return tot


t0 = time.time()
I_sc = 3 * (1/pi**2) * quad(f_sc, [0, pi/2], [0, pi/2])     # G_SC(0) = 3 I_S
check("B5 G_SC(0) quadrature == sqrt6 G(1/24)G(5/24)G(7/24)G(11/24)/(32 pi^3) "
      "[disc -24, h=2 line]",
      close(I_sc, W_SC_cf, QUAD_TOL), "%.1fs" % (time.time()-t0))

mp.dps = 60
G13 = gamma(mpf(1)/3)
G14 = gamma(mpf(1)/4)

# ------------------- C. character-twisted zeta-determinants (both axis classes)
print("\nC. Character-twisted det_zeta: the C4 line is hull, the C3 line is "
      "Gamma(1/3)")

for tag, a in (("C1 (C4 character a=1/4)", mpf(1)/4),
               ("C2 (C3 character a=1/3)", mpf(1)/3)):
    zp0 = diff(lambda s_, a_=a: zeta(s_, a_), 0)
    check(tag + " Lerch: exp(-zeta_H'(0,a)) == sqrt(2pi)/Gamma(a)",
          close(exp(-zp0), sqrt(2*pi)/gamma(a), mpf(10)**-8),
          "finite-difference derivative, tol 1e-8")
check("C3 C4-line values are hull monomials: sqrt(2pi)/Gamma(1/4) = 2^(1/4)/s, "
      "/Gamma(1/2) = sqrt2, /Gamma(3/4) = 2^(1/4) s",
      close(sqrt(2*pi)/gamma(mpf(1)/4), R4/S)
      and close(sqrt(2*pi)/gamma(mpf(1)/2), sqrt(mpf(2)))
      and close(sqrt(2*pi)/gamma(mpf(3)/4), R4*S))
check("C4 scaling anomaly is rational: zeta_H(0,a) == 1/2 - a  (a = 1/4, 1/3)",
      close(zeta(0, mpf(1)/4), mpf(1)/4) and close(zeta(0, mpf(1)/3), mpf(1)/6))
# rescaling by a hull element lands in the RADICAL tower, not bare N~:
# det_zeta(c D_a) = c^(zeta(0,a)) det_zeta(D_a);  c = G*, a = 1/4  ->  G*^(1/4)
det_scaled = G**(zeta(0, mpf(1)/4)) * sqrt(2*pi)/gamma(mpf(1)/4)
check("C5 det_zeta(G* D_{1/4}) == G*^(1/4) 2^(1/4)/s  (element of N~_rad: "
      "radical closure is REQUIRED, FTD-0353 T3 covers it)",
      close(det_scaled, G**(mpf(1)/4) * R4 / S))
check("C6 C6 closes into the d=-3 line: Gamma(1/6) == 2^(-1/3) sqrt(3/pi) "
      "Gamma(1/3)^2  (Gauss duplication)",
      close(gamma(mpf(1)/6), 2**(-mpf(1)/3)*sqrt(3/pi)*G13**2))

# ------------------------------------------------- D. PSLQ evidence (dps 400)
print("\nD. PSLQ layer at dps 400 (heights obey the FTD-0351/0353 spurious-"
      "PSLQ rule)")

mp.dps = 400
G13 = gamma(mpf(1)/3)
G14 = gamma(mpf(1)/4)
G16 = gamma(mpf(1)/6)
l2, l3, lpi = log(mpf(2)), log(mpf(3)), log(pi)

rel = pslq([log(G14), log(gamma(mpf(3)/4)), lpi, l2],
           maxcoeff=10**4, maxsteps=10**6)
check("D1 positive control: reflection relation found on "
      "{lnG(1/4), lnG(3/4), ln pi, ln 2}",
      rel is not None and [abs(c) for c in rel] == [2, 2, 2, 1],
      str(rel))
rel = pslq([log(G16), log(G13), lpi, l2, l3], maxcoeff=10**4, maxsteps=10**6)
check("D2 positive control: duplication relation found on "
      "{lnG(1/6), lnG(1/3), ln pi, ln 2, ln 3}",
      rel is not None and max(abs(c) for c in rel) <= 12, str(rel))

th3_i = jtheta(3, 0, exp(-pi))
check("D3 theta3(0, tau=i) == pi^(1/4)/Gamma(3/4)  (hull monomial; FTD-0341 C2)",
      close(th3_i, pi**mpf('0.25')/gamma(mpf(3)/4), mpf(10)**-390))

th3_s3 = jtheta(3, 0, exp(-pi*sqrt(3)))
vec = [log(th3_s3), lpi, log(G13), l2, l3]
rel = pslq(vec, maxcoeff=10**4, maxsteps=10**6)
found = rel is not None and abs(sum(r*v for r, v in zip(rel, vec))) < mpf(10)**-380
check("D4a theta3(0, tau=i sqrt3) is a Qbar-monomial over {pi, Gamma(1/3)} "
      "(classical d=-3 singular value; PSLQ height <= 36)",
      found and max(abs(c) for c in rel) <= 36, str(rel))
check("D4b implied closed form verified: theta3(0, i sqrt3)^2 == "
      "3^(1/4) Gamma(1/3)^3 / (2^(4/3) pi^2)",
      close(th3_s3**2, 3**mpf('0.25')*G13**3/(2**(mpf(4)/3)*pi**2),
            mpf(10)**-390))
rel = pslq([log(th3_s3), lpi, log(G14), l2, l3], maxcoeff=10**4, maxsteps=10**6)
check("D5 same theta-null has NO monomial form over the d=-4 hull basket "
      "{pi, Gamma(1/4), 2, 3}  [EVIDENCE]",
      rel is None, "PSLQ maxcoeff 1e4 -> None")
rel = pslq([log(G13), lpi, log(G14), l2, l3], maxcoeff=10**6, maxsteps=10**7)
check("D6 Gamma(1/3) has NO monomial relation over {pi, Gamma(1/4), 2, 3} at "
      "maxcoeff 1e6  [EVIDENCE for sector independence; proof is OPEN]",
      rel is None, "dps 400 >> 5 x 6 = 30 (dps rule)")

mp.dps = 60
G13 = gamma(mpf(1)/3)

# ---------------- E. dichotomy bookkeeping + conditional delta-stability
print("\nE. Hull-boundedness dichotomy bookkeeping; the d=-3 enlargement "
      "leaves delta out (conditional)")

check("E1 W_FCC prefactor is a hull unit: 2^(14/3) in Qbar and pi^4 == w^16 "
      "(so W_FCC in N~ <=> Gamma(1/3)^6 in N~)",
      close(pi**4, W**16) and close(W_FCC_cf * 2**(mpf(14)/3) * pi**4 / 9,
                                    G13**6, mpf(10)**-20))

s_, w_, y_, z_ = sp.symbols('s w y z')
fl = sp.factor_list(4*s_**4 - s_**2)
mults = sorted((str(f), m) for f, m in fl[1])
check("E2 delta^2 factors s^2(2s-1)(2s+1): multiplicity 1 (ODD) at (2s-1) and "
      "(2s+1), 2 at (s) -- unchanged with a third coordinate present",
      dict((str(f), m) for f, m in fl[1]).get('2*s - 1') == 1
      and dict((str(f), m) for f, m in fl[1]).get('2*s + 1') == 1
      and dict((str(f), m) for f, m in fl[1]).get('s') == 2, str(mults))
sqf = sp.factor_list(sp.sqf_part(4*s_**4 - s_**2))
check("E3 z^2 - s^2(2s-1)(2s+1) has no root in Q(w,y)(s): squarefree part "
      "(2s-1)(2s+1) has positive degree (odd valuation survives enlargement)",
      sp.degree(sp.sqf_part(4*s_**4 - s_**2), s_) >= 2)
ok_m = True
for m in (2, 3):
    S_ = sp.symbols('S')
    poly = 4*S_**(2*m) - 1
    ok_m &= (sp.gcd(poly, sp.diff(poly, S_)) == 1) and (sp.gcd(poly, S_) == 1)
check("E4 radical towers with a third coordinate: 4S^(2m)-1 squarefree, "
      "coprime to S (m = 2, 3) -- FTD-0353 T3 argument survives verbatim", ok_m)
check("E5 regression: 4 Gamma(1/4)^2 - sqrt2 pi == sqrt2 pi (4G*-1)  "
      "(the one branch locus, FTD-0353 G1)",
      close(4*gamma(mpf(1)/4)**2 - sqrt(mpf(2))*pi, sqrt(mpf(2))*pi*(4*G-1)))

# ------------------- F. FC-0-locked constructor closure (hull regressions)
print("\nF. FC-0-locked closure: documented outputs are Qbar-monomials in "
      "(s, w); composites stay monomial")

check("F1 det_zeta line: sqrt(2pi)/Gamma(1/4) = 2^(1/4) s^(-1); ratio = G* = s^2",
      close(sqrt(2*pi)/gamma(mpf(1)/4), R4/S)
      and close((sqrt(2*pi)/gamma(mpf(3)/4))/(sqrt(2*pi)/gamma(mpf(1)/4)), S**2))
check("F2 Watson BCC = s^4/(2 w^4)", close(G**2/(2*pi), S**4/(2*W**4)))
th2 = jtheta(2, 0, exp(-pi)); th3 = jtheta(3, 0, exp(-pi)); th4 = jtheta(4, 0, exp(-pi))
check("F3 theta-nulls at tau=i: th3 = 2^(-1/4) s w^(-1); th2 = th4 = 2^(-1/2) s w^(-1)",
      close(th3, S/(R4*W)) and close(th2, S/(sqrt(mpf(2))*W))
      and close(th4, S/(sqrt(mpf(2))*W)))
eta_a = zeta(0, mpf(1)/4) - zeta(0, mpf(3)/4)
check("F4 eta-invariant eta(D_{1/4}) = 1 - 2/4 = 1/2 in Q  (FTD-0341 C1)",
      close(eta_a, mpf(1)/2))
check("F5 AGM(1, sqrt2) = 2 sqrt(pi)/G* = 2 w^2 s^(-2)",
      close(agm(1, sqrt(mpf(2))), 2*W**2/S**2))
check("F6 lemniscate pi_lem = G* sqrt(pi)/2 = s^2 w^2/2 and CM period "
      "Omega = Gamma(1/4)^2/sqrt(2pi) = s^2 w^2",
      close(G*sqrt(pi)/2, S**2*W**2/2)
      and close(gamma(mpf(1)/4)**2/sqrt(2*pi), S**2*W**2))
comp = (S**2) * (G**2/(2*pi)) * th3**-2 * agm(1, sqrt(mpf(2)))
check("F7 composite of four constructor outputs (det-ratio x Watson x "
      "theta3^-2 x AGM) == sqrt2 G* = sqrt2 s^2  (composition stays monomial)",
      close(comp, sqrt(mpf(2))*S**2))

# --------------------------------- G. locating the import (FC-0 = d=-4 bit)
print("\nG. The selected bit: FC-0's d = -4 is arithmetically distinguished "
      "(motivation, not forcing)")

# K = Q(sqrt(-n)), n squarefree: disc = -n if n = 3 mod 4 else -4n;
# |mu_K| = 4 for n = 1, 6 for n = 3, else 2.  (Classical.)
ok_mu = True
witness = []
for n in range(1, 200):
    if any(e > 1 for e in sp.factorint(n).values()):
        continue  # not squarefree
    disc = -n if n % 4 == 3 else -4*n
    mu = 4 if n == 1 else (6 if n == 3 else 2)
    if mu == -disc:
        witness.append(disc)
    ok_mu &= (mu == -disc) == (disc == -4)
check("G1 |mu_K| == |disc K| holds for d = -4 ONLY (all imaginary quadratic "
      "fields Q(sqrt(-n)), squarefree n < 200; mu = 4, 6, 2 for n = 1, 3, else)",
      ok_mu and witness == [-4], "witnesses: " + str(witness))

mp.dps = 25
t0 = time.time()


def f_eng(y, z):
    cy, czv = cos(y), cos(z)
    out = mpf(0)
    for sy in (1, -1):
        for szv in (1, -1):
            a = 6 - sy*cy - szv*czv - sy*szv*cy*czv
            b = 1 + sy*cy + szv*czv
            if sy == 1 and szv == 1:
                amb = 4*sin(y/2)**2 + 4*sin(z/2)**2 + omcc(y, z)   # A-B stable
                out += 1/sqrt(amb*(a + b))
            else:
                out += 1/sqrt(a**2 - b**2)
    return out


# G_eng(0) = (1/pi^3) iiint 1/(1 - (lam_SC + lam_FCC)/2)
#          = (6/pi^3) iiint 1/(6 - Sum c_i - Sum c_i c_j)
# after the x-reduction, folded to [0,pi/2]^2 with four sign branches:
I_eng = 6 * quad(f_eng, [0, pi/2], [0, pi/2]) / pi**2
print("  [INFO] engine-stencil ((SC+FCC)/2) self-energy G_eng(0) = "
      + mp.nstr(I_eng, 18) + "  (%.1fs; no documented closed form --"
      " outside the declared basis; no claim made)" % (time.time()-t0))
mp.dps = 60

# ------------------------------------------------------------------ summary
print("\n" + "=" * 92)
n_pass = sum(1 for _, ok in checks if ok)
print("RESULT: %d/%d checks pass" % (n_pass, len(checks)))
if n_pass != len(checks):
    print("FAILURES:")
    for name, ok in checks:
        if not ok:
            print("  - " + name)
    raise SystemExit(1)
print("Conditionality ledger (NOT proven here, named honestly):")
print("  * Chudnovsky 1976 (pi, Gamma(1/4) alg. independent) -- all hull models")
print("  * joint alg. independence of {pi, Gamma(1/4), Gamma(1/3)}: OPEN --")
print("    required only by the sector-neutral K-BIND extension (Corollary 4)")
print("  * adequacy of the declared constructor basis: [SELECTION -- declared],")
print("    falsifier = a forced native output with odd (4G*-1)-valuation")

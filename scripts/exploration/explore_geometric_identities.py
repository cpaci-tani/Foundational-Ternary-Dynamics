"""
Geometric Identities of Z^3: A Systematic Exploration

Explores all geometric structures computable from the cubic lattice
and the CM curve E: y^2 = x^3 - x, with no physics assumptions.

Eight identities, each proved or falsified computationally.
"""

import numpy as np
from scipy.special import gamma
from fractions import Fraction

# =====================================================
# CANONICAL CONSTANTS (all from Gamma(1/4) alone)
# =====================================================

G14 = gamma(0.25)  # Gamma(1/4) = 3.62560990272...
Gstar = np.sqrt(2) * G14**2 / (2 * np.pi)
varpi = G14**2 / (2 * np.sqrt(2 * np.pi))
W3 = G14**4 / (4 * np.pi**3)
PF = np.pi / 4

# Master quadratic
K = 16 * Gstar**2
Delta = K**2 - 4 * K * Gstar
xp = (K + np.sqrt(Delta)) / 2
xm = (K - np.sqrt(Delta)) / 2

print("=" * 70)
print("GEOMETRIC IDENTITIES OF Z^3 AND THE CM CURVE E: y^2 = x^3 - x")
print("=" * 70)
print()
print(f"Gamma(1/4)   = {G14:.15f}")
print(f"varpi        = {varpi:.15f}")
print(f"G*           = {Gstar:.15f}")
print(f"W_3          = {W3:.15f}")
print(f"x+           = {xp:.15f}")
print(f"x-           = {xm:.15f}")

# =====================================================
# IDENTITY 1: The Period-Bridge Ratio
# =====================================================

print()
print("=" * 70)
print("IDENTITY 1: G*/varpi = 2/sqrt(pi)")
print("=" * 70)
print()

ratio1 = Gstar / varpi
target1 = 2 / np.sqrt(np.pi)
print(f"  G*/varpi       = {ratio1:.15f}")
print(f"  2/sqrt(pi)     = {target1:.15f}")
print(f"  Difference     = {abs(ratio1 - target1):.2e}")
print(f"  EXACT IDENTITY: YES")
print()

# PROOF: G* = sqrt(2)*G14^2/(2*pi), varpi = G14^2/(2*sqrt(2*pi))
# G*/varpi = [sqrt(2)*G14^2/(2*pi)] / [G14^2/(2*sqrt(2*pi))]
#          = [sqrt(2)/(2*pi)] * [2*sqrt(2*pi)/1]
#          = [sqrt(2) * 2 * sqrt(2*pi)] / (2*pi)
#          = [2 * sqrt(2) * sqrt(2) * sqrt(pi)] / (2*pi)
#          = [2 * 2 * sqrt(pi)] / (2*pi)
#          = [4*sqrt(pi)] / (2*pi)
#          = 2/sqrt(pi)

print("  PROOF:")
print("    G* = sqrt(2)*Gamma(1/4)^2 / (2*pi)")
print("    varpi = Gamma(1/4)^2 / (2*sqrt(2*pi))")
print("    G*/varpi = sqrt(2)/(2*pi) * 2*sqrt(2*pi)")
print("             = 2*sqrt(2)*sqrt(2*pi) / (2*pi)")
print("             = 2*sqrt(2)*sqrt(2)*sqrt(pi) / (2*pi)")
print("             = 4*sqrt(pi) / (2*pi)")
print("             = 2/sqrt(pi)   QED")
print()
print("  MEANING: The bridge constant exceeds the lemniscate constant")
print("  by the factor 2/sqrt(pi) -- the ratio of the diameter of a")
print("  unit circle to its quarter-arc length. This is purely geometric:")
print("  it converts between linear measure (diameter = 2) and curved")
print("  measure (quarter-circumference = sqrt(pi) in appropriate units).")

# =====================================================
# IDENTITY 2: Packing Fraction = Squared Period Ratio
# =====================================================

print()
print("=" * 70)
print("IDENTITY 2: PF = varpi^2/G*^2 = pi/4")
print("=" * 70)
print()

ratio2 = varpi**2 / Gstar**2
target2 = np.pi / 4
print(f"  varpi^2/G*^2   = {ratio2:.15f}")
print(f"  pi/4           = {target2:.15f}")
print(f"  Difference     = {abs(ratio2 - target2):.2e}")
print(f"  EXACT IDENTITY: YES")
print()

# From Identity 1: varpi/G* = sqrt(pi)/2
# So varpi^2/G*^2 = pi/4

print("  PROOF: Immediate from Identity 1.")
print("    varpi/G* = sqrt(pi)/2  (reciprocal of Identity 1)")
print("    (varpi/G*)^2 = pi/4   QED")
print()
print("  MEANING: The packing fraction pi/4 -- the ratio of a circle's")
print("  area to its bounding square -- equals the squared ratio of")
print("  the lemniscate period to the bridge constant.")
print()
print("  Equivalently: varpi^2 = (pi/4) * G*^2 = PF * G*^2")
print("  The lemniscate period squared is the bridge constant squared")
print("  times the packing fraction. This connects:")
print("    - Circle geometry (pi/4)")
print("    - Lemniscate geometry (varpi)")
print("    - Lattice geometry (G* via Watson)")

# =====================================================
# IDENTITY 3: Watson = Bridge Squared / (2*pi)
# =====================================================

print()
print("=" * 70)
print("IDENTITY 3: W_3 = G*^2/(2*pi)")
print("=" * 70)
print()

ratio3 = Gstar**2 / (2 * np.pi)
print(f"  G*^2/(2*pi)    = {ratio3:.15f}")
print(f"  W_3            = {W3:.15f}")
print(f"  Difference     = {abs(ratio3 - W3):.2e}")
print(f"  EXACT IDENTITY: YES (Paper 0a, Theorem 4.1)")
print()

# Combining with Identity 2:
# W_3 = G*^2/(2*pi) = varpi^2/(2*pi * PF) ... no
# W_3 = G*^2/(2*pi) and varpi^2 = PF * G*^2
# So W_3 = varpi^2 / (2*pi*PF) = varpi^2 / (2*pi*(pi/4)) = varpi^2 / (pi^2/2)
# = 2*varpi^2/pi^2

check = 2 * varpi**2 / np.pi**2
print(f"  Cross-check: 2*varpi^2/pi^2 = {check:.15f}")
print(f"  Matches W_3: {abs(check - W3) < 1e-14}")
print()
print("  COROLLARY: W_3 = 2*varpi^2/pi^2")
print("  The Watson integral is twice the lemniscate constant squared")
print("  divided by pi squared. Three different mathematical objects")
print("  (lattice Green's function, lemniscate arc length, circle constant)")
print("  related by a factor of 2.")

# =====================================================
# IDENTITY 4: Ehrhart Polynomial and Lattice Counts
# =====================================================

print()
print("=" * 70)
print("IDENTITY 4: EHRHART POLYNOMIAL E(n) = (n+1)^3")
print("=" * 70)
print()

print("  The number of integer points in the cube [0,n]^3:")
print()
print(f"  {'n':>3s} {'E(n)':>6s} {'Interior':>8s} {'Boundary':>8s}  Notes")
print(f"  {'---':>3s} {'------':>6s} {'--------':>8s} {'--------':>8s}  -----")

notes = {
    1: "BCC=8 (boundary=total, no interior)",
    2: "Moore=26 (boundary), 3^3=27 (total)",
    3: "BCC=8 (interior!), N_base^3=64 (total)",
    4: "N_c^3=27 (interior), 5^3=125 (total)",
    5: "N_base^3=64 (interior), 6^3=216",
}

for n in range(1, 8):
    total = (n + 1)**3
    interior = max(0, (n - 1)**3)
    boundary = total - interior
    note = notes.get(n, "")
    print(f"  {n:3d} {total:6d} {interior:8d} {boundary:8d}  {note}")

print()
print("  KEY OBSERVATIONS:")
print()
print("  a) n=1: E(1) = 8 = BCC neighbor count = z_BCC")
print("     The BCC neighbors ARE the vertices of the unit cube.")
print("     This is TRIVIALLY true -- not a coincidence, a definition.")
print()
print("  b) n=2: Boundary = 26 = Moore neighborhood")
print("     The Moore neighborhood IS the boundary of the 2-dilated cube")
print("     minus the center point. Again definitional.")
print()
print("  c) n=3: Interior = 8 = z_BCC, Total = 64 = N_base^3 = 4^3")
print("     The BCC count reappears as the INTERIOR of the 3-dilated cube.")
print("     This is NOT trivial: interior(n) = (n-1)^3, so interior(3) = 2^3 = 8.")
print("     The equation 2^3 = 8 = z_BCC is a numerical fact, but it follows")
print("     from z_BCC = 2^D for D=3 (each corner has coordinates in {-1,+1}).")
print()
print("  d) n=4: Interior = 27 = 3^3 = N_c^3, Total = 125 = 5^3")
print("     Interior(4) = 3^3 = N_c^3. This is (4-1)^3 = (N_base-1)^3 = N_c^3.")
print("     Total = 5^3. And 5 = N_base + 1.")
print("     So: E(N_base) = (N_base+1)^3, Interior(N_base) = N_c^3.")
print()

# The deep pattern
print("  THEOREM: For the D-dimensional cube,")
print("    Interior(n) = (n-1)^D")
print("    In D=3:")
print("      Interior(N_base) = (N_base - 1)^3 = N_c^3  because N_c = N_base - 1 = 3")
print("      Interior(N_c + 1) = N_c^3")
print()
print("    This is a TAUTOLOGY given N_base = N_c + 1, not a deep identity.")
print("    The Ehrhart counts are DEFINITIONS, not discoveries.")
print("    They show that the framework integers are consistent with")
print("    lattice point counting, but they don't derive the integers.")

# =====================================================
# IDENTITY 5: Cuboctahedral Group Theory
# =====================================================

print()
print("=" * 70)
print("IDENTITY 5: CUBOCTAHEDRAL INVARIANTS")
print("=" * 70)
print()

print("  The cuboctahedron = convex hull of 12 FCC neighbors of Z^3.")
print("  Its symmetry group is O_h (full octahedral group).")
print()

# Group-theoretic facts
print("  GROUP-THEORETIC FACTS (all [THEOREM]):")
print(f"    |O_h| = 48")
print(f"    Rotation subgroup |O| = 24")
print(f"    Conjugacy classes of O: 5")
print(f"    Irreducible representations of O: 5 (dims 1,1,2,3,3)")
print()

# Axis counts
C4, C3, C2 = 3, 4, 6
total_axes = C4 + C3 + C2
print(f"  ROTATION AXES of the cuboctahedron:")
print(f"    C4 (4-fold, through opposite square faces): {C4}")
print(f"    C3 (3-fold, through opposite triangular faces): {C3}")
print(f"    C2 (2-fold, through opposite edges): {C2}")
print(f"    Total: {C4} + {C3} + {C2} = {total_axes}")
print()

print(f"  FRAMEWORK INTEGER CORRESPONDENCES:")
print(f"    C4 count = {C4} = N_c")
print(f"    C3 count = {C3} = N_base")
print(f"    C2 count = {C2} = N_f = 2*N_gen")
print(f"    Total    = {total_axes} = N_eff")
print(f"    Axis types = 3 = N_gen = N_c")
print()

# Deeper group theory
print(f"  QUOTIENT INVARIANTS:")
print(f"    |O_h| / |Z_3| = 48/3 = 16 = k_phys")
print(f"    |O_h| / |O|   = 48/24 = 2  (parity)")
print(f"    |O| / |T|     = 24/12 = 2  (T = tetrahedral subgroup)")
print(f"    |O_h| / |D_4| = 48/8 = 6   (D_4 = dihedral of square face)")
print()

# Face/vertex/edge counts
V, E, F = 12, 24, 14
print(f"  COMBINATORIAL INVARIANTS:")
print(f"    Vertices V = {V}")
print(f"    Edges    E = {E}")
print(f"    Faces    F = {F} = 8 triangles + 6 squares")
print(f"    Euler: V - E + F = {V} - {E} + {F} = {V - E + F} = 2  (sphere)")
print(f"    E/V = {E}/{V} = {E//V}")
print(f"    F/V = {F}/{V} = {Fraction(F, V)}")

# The 7/6 ratio
print(f"\n    F/V = 7/6: numerator 7 = b_3 (QCD beta coefficient)")
print(f"    Is this a coincidence or structure?")
print(f"    F = 14 = 2*b_3. The face count is twice the beta coefficient.")
print(f"    E = 24 = |O|. The edge count equals the rotation group order.")
print(f"    V = 12 = FCC neighbor count. Definitional.")

# =====================================================
# IDENTITY 6: Fisher Metric Curvature Ratio
# =====================================================

print()
print("=" * 70)
print("IDENTITY 6: FISHER METRIC G(x-)/G(x+) = (x+/x-)^2")
print("=" * 70)
print()

# Fisher metric on 1-parameter coupling space
# G_xx(x) = W_3/x^2 (self-energy gives the metric)
G_xp = W3 / xp**2
G_xm = W3 / xm**2
ratio6 = G_xm / G_xp
ratio6_pred = (xp / xm)**2

print(f"  Fisher metric G_xx(x) = W_3 / x^2")
print(f"  At x+ = {xp:.4f}: G = {G_xp:.6e}")
print(f"  At x- = {xm:.4f}:   G = {G_xm:.6f}")
print(f"  Ratio G(x-)/G(x+) = {ratio6:.4f}")
print(f"  = (x+/x-)^2       = {ratio6_pred:.4f}")
print()

# What IS this ratio in framework terms?
# (x+/x-)^2 = (x+ * x-)^2 / x-^4 = (16*G*^3)^2 / x-^4
# Or: from Vieta, x+ = K - x-, so x+/x- = K/x- - 1 = 16*G*^2/x- - 1

print(f"  The ratio (x+/x-)^2 in framework terms:")
print(f"    x+/x- = {xp/xm:.10f}")
print(f"    1/(alpha * N_c) = {1/(xp * xm / xp**2 * xp):.4f}... no")
print(f"    x+/x- = (K - x-)/x- = K/x- - 1 = {K/xm - 1:.6f}")
print(f"    K/x- = 16*G*^2/x- = {K/xm:.6f}")
print()
print(f"  PHYSICAL INTERPRETATION:")
print(f"  The Fisher information G_xx measures how fast the partition")
print(f"  function changes per unit change in coupling. At strong coupling")
print(f"  (x-), the theory is ~{ratio6:.0f}x more sensitive than at weak coupling (x+).")
print(f"  This is not surprising -- strongly coupled theories are more")
print(f"  rigid (less room to vary couplings without changing physics).")
print(f"  But the RATIO being exactly (x+/x-)^2 is a theorem of the")
print(f"  1/x^2 metric structure, not a numerical coincidence.")

# =====================================================
# IDENTITY 7: The Discriminant Parabola
# =====================================================

print()
print("=" * 70)
print("IDENTITY 7: DISCRIMINANT LANDSCAPE")
print("=" * 70)
print()

# The family x^2 - k*G*^2*x + k*G*^3 = 0
# Discriminant: Delta(k) = k^2*G*^4 - 4*k*G*^3 = k*G*^3*(k*G* - 4)

k_crit = 4 / Gstar
k_phys = 16
k_cons = 0.5

print(f"  Master quadratic family: x^2 - k*G*^2*x + k*G*^3 = 0")
print(f"  Discriminant: Delta(k) = k*G*^3*(k*G* - 4)")
print()
print(f"  THREE REGIMES:")
print(f"    k > {k_crit:.6f} (= 4/G*): Delta > 0, two real roots (BOSONIC)")
print(f"    k = {k_crit:.6f}:          Delta = 0, degenerate root (MEASUREMENT)")
print(f"    k < {k_crit:.6f}:          Delta < 0, complex roots (FERMIONIC)")
print()

# Key k-values
print(f"  KEY PARAMETER VALUES:")
print(f"    k_crit = 4/G*           = {k_crit:.10f}")
print(f"    k_cons = 1/2            = {k_cons:.10f}")
print(f"    k_phys = 16             = {k_phys:.10f}")
print()

# At criticality
x_degen = 2 * Gstar  # degenerate root
print(f"  AT CRITICALITY (k = 4/G*):")
print(f"    Degenerate root = 2*G* = {x_degen:.10f}")
print(f"    This is the Born rule threshold.")
print()

# At k_cons = 1/2
Delta_cons = k_cons * Gstar**3 * (k_cons * Gstar - 4)
x_cons_re = k_cons * Gstar**2 / 2
x_cons_im = np.sqrt(abs(Delta_cons)) / 2
print(f"  AT k_cons = 1/2 (reference frame context):")
print(f"    Delta = {Delta_cons:.10f} < 0 (complex roots)")
print(f"    x = {x_cons_re:.6f} +/- {x_cons_im:.6f}*i")
phase = np.degrees(np.arctan2(x_cons_im, x_cons_re))
print(f"    Phase angle = {phase:.4f} degrees")
print()

# Distance from criticality
dist_phys = k_phys - k_crit
dist_cons = k_crit - k_cons
print(f"  DISTANCE FROM CRITICALITY:")
print(f"    k_phys - k_crit = {dist_phys:.6f}  (bosonic side)")
print(f"    k_crit - k_cons = {dist_cons:.6f}  (fermionic side)")
print(f"    Ratio: {dist_phys / dist_cons:.6f}")
print(f"    = (16 - 4/G*) / (4/G* - 1/2)")
print(f"    = (16*G* - 4) / (4 - G*/2)")
val = (16 * Gstar - 4) / (4 - Gstar / 2)
print(f"    = {val:.10f}")
print()

# The parabola in (k, x+) space
print(f"  THE ROOT CURVES x_+(k) and x_-(k):")
print(f"  As k varies from 4/G* to infinity:")
ks = [k_crit, 2, 4, 8, 12, 16, 20, 32, 64]
print(f"    {'k':>8s} {'x+':>12s} {'x-':>12s} {'x+ + x-':>12s} {'x+ * x-':>12s}")
for k in ks:
    d = k * Gstar**3 * (k * Gstar - 4)
    if d >= 0:
        xp_k = (k * Gstar**2 + np.sqrt(d)) / 2
        xm_k = (k * Gstar**2 - np.sqrt(d)) / 2
        print(f"    {k:8.4f} {xp_k:12.4f} {xm_k:12.4f} {xp_k+xm_k:12.4f} {xp_k*xm_k:12.4f}")
    else:
        re = k * Gstar**2 / 2
        im = np.sqrt(abs(d)) / 2
        print(f"    {k:8.4f}   {re:.4f}+{im:.4f}i   (complex conjugates)")

# =====================================================
# IDENTITY 8: Ehrhart n=4 and delta/3
# =====================================================

print()
print("=" * 70)
print("IDENTITY 8: EHRHART n=4 AND THE COLOR EXCESS")
print("=" * 70)
print()

delta = xm - 3
print(f"  x- = {xm:.15f}")
print(f"  delta = x- - 3 = {delta:.15f}")
print(f"  delta/3 = {delta/3:.15f}")
print(f"  1/125 = {1/125:.15f}")
print(f"  delta/3 - 1/125 = {delta/3 - 1/125:.6e}")
print(f"  Relative error: {abs(delta/3 - 1/125)/(1/125)*100:.4f}%")
print()
print(f"  Ehrhart at n=4: E(4) = 125 = 5^3")
print(f"  Interior(4) = 27 = 3^3 = N_c^3")
print(f"  Interior/Total = 27/125 = (3/5)^3 = {(3/5)**3:.10f}")
print()

# Is there a connection? Let's check rigorously.
# delta/3 = (16*G*^3 - 48*G*^2 + 9) / (48*G*^2 - 18)  [exact from quadratic]
num = 16 * Gstar**3 - 48 * Gstar**2 + 9
den = 48 * Gstar**2 - 18
exact_d3 = num / den
print(f"  Exact: delta/3 = (16*G*^3 - 48*G*^2 + 9) / (48*G*^2 - 18)")
print(f"                 = {num:.10f} / {den:.10f}")
print(f"                 = {exact_d3:.15f}")
print()

# For delta/3 = 1/125 exactly, need:
# (16*G*^3 - 48*G*^2 + 9) / (48*G*^2 - 18) = 1/125
# 125*(16*G*^3 - 48*G*^2 + 9) = 48*G*^2 - 18
# 2000*G*^3 - 6000*G*^2 + 1125 = 48*G*^2 - 18
# 2000*G*^3 - 6048*G*^2 + 1143 = 0

# Check if G* satisfies this:
residual = 2000 * Gstar**3 - 6048 * Gstar**2 + 1143
print(f"  If delta/3 = 1/125 exactly, G* would satisfy:")
print(f"    2000*G*^3 - 6048*G*^2 + 1143 = 0")
print(f"    Residual at actual G*: {residual:.6f}")
print(f"    NOT zero. The identity delta/3 = 1/125 is APPROXIMATE, not exact.")
print()

# What IS special about the near-miss?
print(f"  HONEST ASSESSMENT:")
print(f"    delta/3 = 0.007988, which is 0.15% below 1/125 = 0.008.")
print(f"    This is NOT an identity. It is a near-miss.")
print(f"    The Ehrhart connection (125 = E(4)) provides no mechanism")
print(f"    linking lattice point counts to the gap equation roots.")
print(f"    STATUS: [NUMEROLOGY] -- interesting but not derived.")

# =====================================================
# SUMMARY
# =====================================================

print()
print("=" * 70)
print("SUMMARY: WHAT IS PROVEN vs WHAT IS NUMEROLOGY")
print("=" * 70)
print()

summary = [
    ("1", "G*/varpi = 2/sqrt(pi)", "THEOREM", "Algebraic identity from definitions"),
    ("2", "PF = varpi^2/G*^2 = pi/4", "THEOREM", "Corollary of Identity 1"),
    ("3", "W_3 = G*^2/(2*pi)", "THEOREM", "Watson 1939 + algebraic identity"),
    ("4", "Ehrhart counts match framework", "TAUTOLOGY", "Definitional lattice point counting"),
    ("5a", "Cuboctahedral axes = {3,4,6}, total 13", "THEOREM", "O_h group theory"),
    ("5b", "|O_h|/|Z_3| = 16", "THEOREM", "Group quotient"),
    ("5c", "F/V = 7/6, numerator = b_3", "OBSERVATION", "Combinatorial fact; b_3 connection unclear"),
    ("6", "G(x-)/G(x+) = (x+/x-)^2", "THEOREM", "From G_xx = W_3/x^2 metric"),
    ("7", "Three regimes from discriminant", "THEOREM", "Quadratic discriminant sign"),
    ("8", "delta/3 ~ 1/125 ~ 1/E(4)", "NUMEROLOGY", "0.15% near-miss, no mechanism"),
]

print(f"  {'#':>3s}  {'Identity':40s}  {'Status':12s}  Notes")
print(f"  {'---':>3s}  {'----------':40s}  {'------':12s}  -----")
for num, identity, status, note in summary:
    print(f"  {num:>3s}  {identity:40s}  {status:12s}  {note}")

"""
The Projective Geometry of the Master Quadratic Roots

Starting from the proven identities, we explore what the master quadratic's
root structure implies geometrically. No physics. Pure algebraic geometry
of the family x^2 - kG*^2 x + kG*^3 = 0.

The goal: find every theorem hiding in this equation.
"""

import numpy as np
from scipy.special import gamma

G14 = gamma(0.25)
Gstar = np.sqrt(2) * G14**2 / (2 * np.pi)
varpi = G14**2 / (2 * np.sqrt(2 * np.pi))
K = 16 * Gstar**2
xp = 137.036171458155422
xm = 3.023963916339028

# Dimensionless roots
yp = xp / Gstar
ym = xm / Gstar

print("=" * 70)
print("THE PROJECTIVE GEOMETRY OF THE MASTER QUADRATIC")
print("=" * 70)
print()
print(f"G* = {Gstar:.15f}")
print(f"x+ = {xp:.15f},  y+ = x+/G* = {yp:.10f}")
print(f"x- = {xm:.15f},  y- = x-/G* = {ym:.10f}")

# =====================================================
print()
print("=" * 70)
print("THEOREM 1: THE RECIPROCAL SUM IDENTITY")
print("=" * 70)
print()

recip_sum = 1/yp + 1/ym
print(f"  1/y+ + 1/y- = {recip_sum:.15f}")
print(f"  Expected: 1")
print(f"  EXACT: {abs(recip_sum - 1) < 1e-12}")
print()
print("  PROOF:")
print("    In dimensionless form y = x/G*, the master quadratic becomes:")
print("      y^2 - 16G* y + 16G* = 0")
print()
print("    Vieta relations: y+ + y- = 16G* = y+ * y-")
print("    Therefore: 1/y+ + 1/y- = (y+ + y-) / (y+ * y-) = 1.  QED")
print()
print("  MEANING: The two couplings are projectively complementary.")
print("  In reciprocal space (u = 1/y = G*/x), the roots u+ and u-")
print("  lie on the hyperplane u+ + u- = 1.")
print()

# Reciprocal roots
up = 1/yp
um = 1/ym
print(f"  u+ = G*/x+ = {up:.10f}  (EM sector: ~2% of unit)")
print(f"  u- = G*/x- = {um:.10f}  (Color sector: ~98% of unit)")
print(f"  u+ + u- = {up + um:.15f}")
print()
print("  The electromagnetic coupling consumes {:.2f}% of the reciprocal budget.".format(up*100))
print("  The color coupling consumes {:.2f}%.".format(um*100))
print("  They exhaust the budget exactly.")

# =====================================================
print()
print("=" * 70)
print("THEOREM 2: HARMONIC MEAN = CRITICAL ROOT")
print("=" * 70)
print()

H = 2 * xp * xm / (xp + xm)
x_crit = 2 * Gstar  # degenerate root at k = 4/G*
print(f"  Harmonic mean H(x+, x-) = 2*x+*x- / (x+ + x-)")
print(f"                          = 2 * {xp*xm:.6f} / {xp+xm:.6f}")
print(f"                          = {H:.15f}")
print(f"  Critical root   2*G*    = {x_crit:.15f}")
print(f"  EXACT: {abs(H - x_crit) < 1e-12}")
print()
print("  PROOF:")
print("    H = 2*x+*x- / (x+ + x-) = 2*KG* / K = 2G*.  QED")
print()
print("  MEANING: The Born rule threshold (where discriminant = 0)")
print("  is the HARMONIC MEAN of the two coupling constants.")
print("  The measurement boundary is not arbitrary -- it is the")
print("  unique point equidistant from both roots in reciprocal space.")

# =====================================================
print()
print("=" * 70)
print("THEOREM 3: THE FOUR MEANS OF THE ROOTS")
print("=" * 70)
print()

AM = (xp + xm) / 2  # arithmetic mean
GM = np.sqrt(xp * xm)  # geometric mean
HM = 2 * xp * xm / (xp + xm)  # harmonic mean
QM = np.sqrt((xp**2 + xm**2) / 2)  # quadratic mean (RMS)

print(f"  Quadratic mean  QM = sqrt((x+^2 + x-^2)/2) = {QM:.10f}")
print(f"  Arithmetic mean AM = (x+ + x-)/2            = {AM:.10f}")
print(f"  Geometric mean  GM = sqrt(x+ * x-)           = {GM:.10f}")
print(f"  Harmonic mean   HM = 2*x+*x-/(x+ + x-)      = {HM:.10f}")
print()

# Express each in terms of G*
print(f"  In terms of G*:")
print(f"    QM = {QM/Gstar:.10f} * G*")
print(f"    AM = {AM/Gstar:.10f} * G* = 8G* = {8*Gstar:.10f}")
print(f"    GM = {GM/Gstar:.10f} * G* = 4G*^(3/2)/sqrt(G*) ...")

# AM = K/2 = 8G*^2. Wait: AM = (x+ + x-)/2 = K/2 = 8G*^2
# GM = sqrt(KG*) = sqrt(16G*^3) = 4G*^(3/2)
# HM = 2G*

print(f"    AM = K/2 = 8G*^2 = {8*Gstar**2:.10f}  check: {AM:.10f}")
print(f"    GM = sqrt(KG*) = 4*G*^(3/2) = {4*Gstar**1.5:.10f}  check: {GM:.10f}")
print(f"    HM = 2G*       = {2*Gstar:.10f}  check: {HM:.10f}")
print()

# The AM-GM-HM inequality
print(f"  The classical inequality QM >= AM >= GM >= HM holds:")
print(f"    {QM:.4f} >= {AM:.4f} >= {GM:.4f} >= {HM:.4f}")
print()

# Each mean has a clean expression
print(f"  CLEAN EXPRESSIONS:")
print(f"    HM = 2G*           (linear in G*)")
print(f"    GM = 4G*^(3/2)     (sesquilinear)")
print(f"    AM = 8G*^2         (quadratic)")
print(f"    QM = ?")
print()

# Check QM
QM_check = np.sqrt((xp**2 + xm**2) / 2)
# xp^2 + xm^2 = (xp + xm)^2 - 2*xp*xm = K^2 - 2KG* = K(K - 2G*)
# = 16G*^2 * (16G*^2 - 2G*) = 16G*^2 * 2G*(8G* - 1) = 32G*^3(8G* - 1)
# QM = sqrt(K(K-2G*)/2) = sqrt(16G*^2(16G*^2 - 2G*)/2) = G*sqrt(8(16G*^2 - 2G*))
# = G* * sqrt(128G*^2 - 16G*) = G* * 4 * sqrt(8G*^2 - G*)
# Hmm, not as clean.
print(f"    QM^2 = (K^2 - 2KG*)/2 = K(K - 2G*)/2")
print(f"         = 8G*^2(16G*^2 - 2G*) = 16G*^3(8G* - 1)")
print(f"         = {16*Gstar**3*(8*Gstar - 1):.10f}")
print(f"    QM   = 4G*^(3/2) * sqrt(8G* - 1)^(1/2)")
print(f"    Not as clean. The quadratic mean breaks the pattern.")

# =====================================================
print()
print("=" * 70)
print("THEOREM 4: THE ROOT LOCUS AS A RATIONAL CURVE")
print("=" * 70)
print()

print("  For the family x^2 - kG*^2 x + kG*^3 = 0,")
print("  given a root x, the unique k that admits it is:")
print()
print("    k = x^2 / (G*^2(x - G*))")
print()
print("  In dimensionless form u = x/G*:")
print()
print("    k = u^2 / (u - 1)")
print()
print("  This is a RATIONAL FUNCTION of degree 2 with a pole at u = 1 (x = G*).")
print()

# Verify at x+ and x-
k_from_xp = xp**2 / (Gstar**2 * (xp - Gstar))
k_from_xm = xm**2 / (Gstar**2 * (xm - Gstar))
print(f"  Verification:")
print(f"    k(x+) = {k_from_xp:.10f}  (should be 16)")
print(f"    k(x-) = {k_from_xm:.10f}  (should be 16)")
print()

# Properties of k(u) = u^2/(u-1)
print(f"  PROPERTIES of k(u) = u^2/(u-1):")
print(f"    Pole at u = 1 (i.e., x = G*)")
print(f"    Minimum: dk/du = u(u-2)/(u-1)^2 = 0 at u = 0 or u = 2")
print(f"    k(0) = 0, k(2) = 4")
print(f"    k(2) = 4 is the MINIMUM for u > 1")
print()

# Check: k(2) = 4, and k_crit = 4/G* corresponds to u_crit such that
# the two roots coincide at u = 2G* ... wait.
# At criticality: degenerate root is x = 2G*, so u = 2.
# k_crit(u=2) = 4/(2-1) = 4. But k_crit = 4/G* from the discriminant.
# These are different! k(u) = u^2/(u-1) gives k(2) = 4.
# But we said k_crit = 4/G*. Let me recheck.

# The discriminant of x^2 - kG*^2 x + kG*^3 = 0 is:
# Delta = k^2 G*^4 - 4kG*^3 = kG*^3(kG* - 4)
# Delta = 0 when kG* = 4, i.e., k = 4/G*.
# At k = 4/G*, the degenerate root is x = kG*^2/2 = (4/G*)*G*^2/2 = 2G*.
# So u = x/G* = 2 at the degenerate root.
# And k(u=2) = 4/(2-1) = 4 in the formula k = u^2/(u-1).
# But k_crit = 4/G* ≈ 1.352.
#
# Wait, there's a confusion. Let me recheck k = u^2/(u-1).
# From x^2 - kG*^2 x + kG*^3 = 0:
# x^2 = kG*^2(x - G*) = kG*^2 x - kG*^3
# k = x^2 / (G*^2 x - G*^3) = x^2 / (G*^2(x - G*))
# At x = 2G*: k = (2G*)^2 / (G*^2(2G* - G*)) = 4G*^2 / (G*^2 * G*) = 4/G*
# So k(x=2G*) = 4/G*, which matches k_crit = 4/G*. Good.
# And u = x/G* = 2, k = u^2/(u-1) = 4/1 = 4... but k should be 4/G*!
#
# The issue: k = x^2/(G*^2(x-G*)) = (x/G*)^2 / (x/G* - 1) ... wait no:
# k = x^2/(G*^2(x-G*))
# Let u = x/G*. Then x = uG*, x^2 = u^2 G*^2, x - G* = (u-1)G*
# k = u^2 G*^2 / (G*^2 * (u-1) * G*) = u^2 / ((u-1)*G*)
#
# AH! I had a G* factor wrong. k = u^2/((u-1)*G*), NOT u^2/(u-1).

print(f"  CORRECTION: k = u^2 / ((u-1)*G*)")
print()
k_corrected_xp = yp**2 / ((yp - 1) * Gstar)
k_corrected_xm = ym**2 / ((ym - 1) * Gstar)
print(f"  k(u+) = {yp:.6f}^2 / (({yp:.6f}-1)*G*) = {k_corrected_xp:.10f}")
print(f"  k(u-) = {ym:.6f}^2 / (({ym:.6f}-1)*G*) = {k_corrected_xm:.10f}")
print()

# At u = 2: k = 4/(1*G*) = 4/G* = k_crit. Now consistent.
k_at_2 = 4 / (1 * Gstar)
print(f"  k(u=2) = 4/G* = {k_at_2:.10f} = k_crit. Consistent.")
print()

# So the root locus is k*G* = u^2/(u-1), or defining K' = k*G*:
# K' = u^2/(u-1)
# This is cleaner! K' is dimensionless.
Kp = K * Gstar  # K' = kG* = 16G*^2 * G* = 16G*^3... wait no.
# K' = k*G*. At k=16: K' = 16G* ≈ 47.34

print(f"  Define K' = k*G* (dimensionless product). Then:")
print(f"    K' = u^2 / (u - 1)")
print(f"    This is the clean rational curve.")
print(f"    At k=16: K' = 16G* = {16*Gstar:.10f}")
print(f"    At k=4/G*: K' = 4 (criticality)")
print(f"    K'(u+) = {yp**2/(yp-1):.10f}, should = 16G* = {16*Gstar:.10f}")
print(f"    K'(u-) = {ym**2/(ym-1):.10f}, should = 16G* = {16*Gstar:.10f}")
print()

# K' = u^2/(u-1) is a proper rational curve. Its properties:
print(f"  PROPERTIES of K'(u) = u^2/(u-1):")
print(f"    Pole at u = 1")
print(f"    K'(0) = 0")
print(f"    K'(2) = 4 (minimum for u > 1)")
print(f"    K' -> u as u -> infinity (asymptote)")
print(f"    Inflection: d^2K'/du^2 = 2/(u-1)^3, zero at infinity")
print()

# The inverse: given K', what are the roots?
# K'(u-1) = u^2 => u^2 - K'u + K' = 0
# u = (K' +/- sqrt(K'^2 - 4K'))/2 = (K' +/- sqrt(K'(K'-4)))/2
print(f"  INVERSE: Given K', the two roots are:")
print(f"    u = (K' +/- sqrt(K'(K'-4))) / 2")
print(f"    Real roots exist iff K' >= 4 (i.e., k >= 4/G*)")
print(f"    At K' = 4: u = 2 (degenerate)")
print(f"    At K' = 16G* = {16*Gstar:.6f}: u = {yp:.6f}, {ym:.6f}")

# =====================================================
print()
print("=" * 70)
print("THEOREM 5: THE MOBIUS INVOLUTION")
print("=" * 70)
print()

# Define shifted roots: v = u - 1 = (x - G*)/G*
vp = yp - 1
vm = ym - 1

print(f"  Shifted dimensionless roots v = (x - G*)/G*:")
print(f"    v+ = {vp:.10f}")
print(f"    v- = {vm:.10f}")
print(f"    v+ * v- = {vp*vm:.10f}")
print()

# From Vieta: u+ * u- = 16G* (product), u+ + u- = 16G* (sum)
# (v+ + 1)(v- + 1) = 16G*
# v+*v- + v+ + v- + 1 = 16G*
# v+*v- + (u+ + u- - 2) + 1 = 16G*
# v+*v- + 16G* - 2 + 1 = 16G*
# v+*v- = 1

print(f"  THEOREM: v+ * v- = 1")
print(f"  Verification: {vp*vm:.15f}")
print(f"  EXACT: {abs(vp*vm - 1) < 1e-10}")
print()
print("  PROOF: v+ * v- = (u+ - 1)(u- - 1) = u+*u- - (u+ + u-) + 1")
print("                 = 16G* - 16G* + 1 = 1.  QED")
print()
print("  MEANING: The shifted roots are MULTIPLICATIVE INVERSES.")
print("  v- = 1/v+.  The map v -> 1/v is a Mobius involution that")
print("  exchanges the two roots. G* is the fixed point of this involution")
print("  (since v = 0 corresponds to x = G*).")
print()

print(f"  Numerically:")
print(f"    v+ = {vp:.10f}")
print(f"    1/v+ = {1/vp:.10f}")
print(f"    v- = {vm:.10f}")
print(f"    Match: {abs(1/vp - vm) < 1e-10}")
print()
print(f"    ln(v+) = {np.log(vp):.10f}")
print(f"    ln(v-) = {np.log(vm):.10f}")
print(f"    ln(v+) + ln(v-) = {np.log(vp) + np.log(vm):.2e}  (= 0)")
print()
print(f"  The two roots are SYMMETRIC about G* in logarithmic space.")
print(f"  |ln(v+)| = |ln(v-)| = {abs(np.log(vp)):.10f}")

# =====================================================
print()
print("=" * 70)
print("THEOREM 6: THE CROSS-RATIO")
print("=" * 70)
print()

# The four special points on the u-line: 0, 1, u-, u+
# Cross-ratio (u+, u-; 0, 1) = ?
# CR = (u+ - 0)(u- - 1) / ((u+ - 1)(u- - 0))
#    = u+ * v- / (v+ * u-)

CR = (yp * vm) / (vp * ym)
print(f"  Four points on the u-line: 0, 1, u- = {ym:.6f}, u+ = {yp:.6f}")
print(f"  Cross-ratio (u+, u-; 0, 1) = {CR:.15f}")
print(f"  = u+ * (u- - 1) / ((u+ - 1) * u-)")
print(f"  = {yp:.6f} * {vm:.6f} / ({vp:.6f} * {ym:.6f})")
print()

# Since v+*v- = 1: CR = u+*v- / (v+*u-) = (1+v+)*(1/v+) / (v+*(1+1/v+))
# = (1+v+)/(v+^2) * v+/(1+v+)... let me just compute.
# CR = u+*(u--1) / ((u+-1)*u-) = u+*v- / (v+*u-)
# = (1+v+)*(1/v+) / (v+ * (1+1/v+))
# = (v+ + 1)/(v+^2) * 1/((v+^2 + v+)/v+^2)... this is getting circular.
# Just report the value.

# Check if CR = -1 (harmonic conjugates)
print(f"  Is CR = -1 (harmonic range)? CR = {CR:.6f}. NO.")
print()

# What IS the cross-ratio?
# CR = u+ * v- / (v+ * u-)
# = u+/(v+) * v-/u-
# = (1 + 1/v+) * (1 - 1/u-)
# Hmm. Let me try: u+/v+ = u+/(u+-1), u-/v- = u-/(u--1)
# CR = (u+/v+) * (v-/u-) = [u+/(u+-1)] * [(u--1)/u-]
# = [u+/(u+-1)] / [u-/(u--1)]
# = [u+(u--1)] / [u-(u+-1)]

# From the root locus: K' = u^2/(u-1), so u/(u-1) = K'/u
# CR = [u+(u--1)] / [u-(u+-1)] = (u+/u-) * (u--1)/(u+-1) = (u+/u-) * (v-/v+)
# = (u+/u-) * (1/v+^2) since v- = 1/v+
# = (u+/u-) / v+^2

# Not simplifying nicely. The cross-ratio is not -1, so the four points
# 0, 1, u-, u+ are NOT in harmonic range. Let me just report it.

print(f"  The cross-ratio is {CR:.10f}, which is not a simple rational.")
print(f"  The four points 0, 1, u-, u+ are not harmonically related.")

# =====================================================
print()
print("=" * 70)
print("THEOREM 7: THE GENERATING FUNCTION")
print("=" * 70)
print()

# K'(u) = u^2/(u-1) = u + 1 + 1/(u-1)  (partial fractions)
# So K' - u - 1 = 1/(u-1)
# Or: (K' - u)(u - 1) = u  ... let me verify
# K'(u-1) = u^2, K'u - K' = u^2, K' = u^2/(u-1) = u + 1 + 1/(u-1)

print(f"  K'(u) = u^2/(u-1) decomposes as:")
print(f"    K' = u + 1 + 1/(u-1)")
print()
print(f"  Verification at u = u+:")
print(f"    u+ + 1 + 1/(u+-1) = {yp} + 1 + {1/(yp-1)}")
print(f"                      = {yp + 1 + 1/(yp-1):.10f}")
print(f"    16G* = {16*Gstar:.10f}")
print()

# This decomposition is illuminating:
# K' = u + 1 + 1/(u-1)
# The first term (u) is the "classical" part (K' ~ u for large u)
# The "+1" is a constant offset
# The 1/(u-1) is the "quantum correction" (diverges at u=1, i.e., x=G*)
print(f"  INTERPRETATION:")
print(f"    The root locus K' = u + 1 + 1/(u-1) has three parts:")
print(f"    - u:       the identity (classical limit)")
print(f"    - 1:       the constant offset (the '+1' from self-consistency)")
print(f"    - 1/(u-1): the resonance term (pole at x = G*)")
print()
print(f"  At u = u+ = {yp:.4f}:")
print(f"    Classical: {yp:.4f}")
print(f"    Offset:    1")
print(f"    Resonance: {1/(yp-1):.6f}  (negligible -- far from pole)")
print()
print(f"  At u = u- = {ym:.4f}:")
print(f"    Classical: {ym:.4f}")
print(f"    Offset:    1")
print(f"    Resonance: {1/(ym-1):.6f}  (LARGE -- near pole)")
print()
print(f"  The EM root is deep in the classical regime.")
print(f"  The color root is dominated by the resonance term.")
print(f"  G* (u=1) is the pole -- the singularity that separates them.")

# =====================================================
print()
print("=" * 70)
print("SUMMARY: FIVE THEOREMS FROM ONE QUADRATIC")
print("=" * 70)
print()
print("  1. RECIPROCAL SUM:    1/y+ + 1/y- = 1")
print("     The couplings exhaust a unit reciprocal budget.")
print()
print("  2. HARMONIC MEAN:     H(x+, x-) = 2G* = critical root")
print("     The measurement threshold IS the harmonic mean of the couplings.")
print()
print("  3. FOUR MEANS:        HM = 2G*, GM = 4G*^(3/2), AM = 8G*^2")
print("     Each mean is a clean power of G* times a power of 2.")
print()
print("  4. ROOT LOCUS:        K' = u^2/(u-1) = u + 1 + 1/(u-1)")
print("     The parameter-root relationship is a rational curve with")
print("     pole at G* and partial fractions revealing classical/quantum split.")
print()
print("  5. MOBIUS INVOLUTION:  v+ * v- = 1 where v = (x-G*)/G*")
print("     The roots are multiplicative inverses about G*.")
print("     Symmetric in log space: |ln v+| = |ln v-|.")

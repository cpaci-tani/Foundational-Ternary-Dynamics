"""
Fourcier Curve Geometry — Mathematical Anatomy

Explores the geometry of parametric curves restricted to
Cayley-Dickson frequencies {1, 2, 4, 8, 16}, with coefficients
derived from algebraic property loss at each division algebra level.

No physics assumed. Pure geometry and computation.
"""

import numpy as np
from scipy.special import gamma
from scipy.integrate import quad

# =====================================================
# PART I: COEFFICIENT CASCADE
# =====================================================

print("=" * 70)
print("PART I: THE CAYLEY-DICKSON COEFFICIENT CASCADE")
print("=" * 70)
print()

freqs = [1, 2, 4, 8, 16]
algebras = ["R", "C", "H", "O", "S"]

# FTD Fourcier coefficients (from the canonical blue curve)
a_coeff = [1.0, 0.5, 0.5, 0.4, 0.0625]
b_coeff = [1.0, -0.5, 0.5, -0.35, 0.0625]

print("Frequencies (Cayley-Dickson):  ", freqs)
print("a_n (cosine amplitudes):       ", a_coeff)
print("b_n (sine amplitudes):         ", b_coeff)
print()

print("Amplitude ratios at each doubling:")
for i in range(1, 5):
    r = a_coeff[i] / a_coeff[i - 1]
    loss = algebras[i - 1] + " -> " + algebras[i]
    print(f"  {loss:8s}:  a[{i}]/a[{i-1}] = {r:.6f}")

print()
print("Key identifications:")
print(f"  a[1]/a[0] = 1/2     (conjugation: C has z -> z*)")
print(f"  a[2]/a[1] = 1       (norm preserved: |ab|=|a||b| in H)")
print(f"  a[3]/a[2] = 4/5     (7/35 = 1/5 Fano triples; survive = 4/5)")
print(f"  a[4]/a[3] = 5/32    (sedenion collapse)")
print(f"  a[4]      = 1/16    = 1/|Aut(E)|^2 = reciprocal of coefficient 16")

# =====================================================
# PART II: CURVE DEFINITION AND ARC LENGTH
# =====================================================

print()
print("=" * 70)
print("PART II: GEOMETRIC PROPERTIES")
print("=" * 70)
print()

def x(t):
    return sum(a_coeff[i] * np.cos(freqs[i] * t) for i in range(5))

def y(t):
    return sum(b_coeff[i] * np.sin(freqs[i] * t) for i in range(5))

def dx(t):
    return sum(-a_coeff[i] * freqs[i] * np.sin(freqs[i] * t) for i in range(5))

def dy(t):
    return sum(b_coeff[i] * freqs[i] * np.cos(freqs[i] * t) for i in range(5))

def ddx(t):
    return sum(-a_coeff[i] * freqs[i]**2 * np.cos(freqs[i] * t) for i in range(5))

def ddy(t):
    return sum(-b_coeff[i] * freqs[i]**2 * np.sin(freqs[i] * t) for i in range(5))

def speed(t):
    return np.sqrt(dx(t)**2 + dy(t)**2)

def curvature(t):
    dxt, dyt = dx(t), dy(t)
    ddxt, ddyt = ddx(t), ddy(t)
    num = abs(dxt * ddyt - dyt * ddxt)
    den = (dxt**2 + dyt**2)**1.5
    return num / den if den > 1e-15 else 0.0

# Arc length
L, _ = quad(speed, 0, 2 * np.pi, limit=500)
print(f"Arc length L = {L:.10f}")

# Total curvature
total_curv, _ = quad(lambda t: curvature(t) * speed(t), 0, 2 * np.pi, limit=500)
print(f"Total curvature = {total_curv:.10f}")
print(f"Total curvature / (2*pi) = {total_curv / (2 * np.pi):.6f}  (winding if integer)")

# Fisher information (integral of kappa^2 ds)
fisher, _ = quad(lambda t: curvature(t)**2 * speed(t), 0, 2 * np.pi, limit=500)
print(f"Fisher information I_F = {fisher:.10f}")

# Signed area (Green's theorem)
area, _ = quad(lambda t: 0.5 * (x(t) * dy(t) - y(t) * dx(t)), 0, 2 * np.pi, limit=500)
print(f"Signed area A = {area:.10f}")

# Bounding box
N = 100000
ts = np.linspace(0, 2 * np.pi, N)
xs = np.array([x(t) for t in ts])
ys = np.array([y(t) for t in ts])
print(f"Bounding box: x in [{min(xs):.4f}, {max(xs):.4f}], y in [{min(ys):.4f}, {max(ys):.4f}]")

# Self-crossings near origin
origin_dist = np.sqrt(xs**2 + ys**2)
crossings = 0
tol = 0.02
for i in range(1, N - 1):
    if (origin_dist[i] < tol and
        origin_dist[i] < origin_dist[i - 1] and
        origin_dist[i] < origin_dist[i + 1]):
        crossings += 1
print(f"Self-crossings near origin (tol={tol}): {crossings}")

# =====================================================
# PART III: RELATING TO G* AND FTD CONSTANTS
# =====================================================

print()
print("=" * 70)
print("PART III: RELATIONSHIP TO G*, varpi, W_3")
print("=" * 70)
print()

Gstar = np.sqrt(2) * gamma(0.25)**2 / (2 * np.pi)
varpi = gamma(0.25)**2 / (2 * np.sqrt(2 * np.pi))
W3 = gamma(0.25)**4 / (4 * np.pi**3)
PF = np.pi / 4

print(f"G*    = {Gstar:.10f}")
print(f"varpi = {varpi:.10f}")
print(f"W_3   = {W3:.10f}")
print(f"PF    = pi/4 = {PF:.10f}")
print()

# Systematic search for clean ratios
print("Arc length ratios:")
for name, val in [("G*", Gstar), ("varpi", varpi), ("pi", np.pi),
                   ("2*pi", 2*np.pi), ("4*pi", 4*np.pi), ("8*pi", 8*np.pi),
                   ("sqrt(2*pi)", np.sqrt(2*np.pi))]:
    r = L / val
    print(f"  L / {name:12s} = {r:.8f}")

print()
print("Fisher information ratios:")
for name, val in [("G*", Gstar), ("G*^2", Gstar**2), ("16*G*", 16*Gstar),
                   ("16*G*^2", 16*Gstar**2), ("4*pi^2", 4*np.pi**2),
                   ("pi^2", np.pi**2), ("16*pi", 16*np.pi)]:
    r = fisher / val
    print(f"  I_F / {name:12s} = {r:.8f}")

print()
print("Area ratios:")
for name, val in [("pi", np.pi), ("G*", Gstar), ("varpi", varpi),
                   ("G*^2", Gstar**2), ("pi*G*", np.pi*Gstar)]:
    r = abs(area) / val
    print(f"  |A| / {name:12s} = {r:.8f}")

# =====================================================
# PART IV: STRIPPED CURVES — WHAT HAPPENS AT EACH LEVEL
# =====================================================

print()
print("=" * 70)
print("PART IV: TRUNCATED CURVES (adding one frequency at a time)")
print("=" * 70)
print()

for n_terms in range(1, 6):
    def x_n(t, nt=n_terms):
        return sum(a_coeff[i] * np.cos(freqs[i] * t) for i in range(nt))
    def y_n(t, nt=n_terms):
        return sum(b_coeff[i] * np.sin(freqs[i] * t) for i in range(nt))
    def dx_n(t, nt=n_terms):
        return sum(-a_coeff[i] * freqs[i] * np.sin(freqs[i] * t) for i in range(nt))
    def dy_n(t, nt=n_terms):
        return sum(b_coeff[i] * freqs[i] * np.cos(freqs[i] * t) for i in range(nt))

    speed_n = lambda t, nt=n_terms: np.sqrt(
        sum(-a_coeff[i]*freqs[i]*np.sin(freqs[i]*t) for i in range(nt))**2 +
        sum(b_coeff[i]*freqs[i]*np.cos(freqs[i]*t) for i in range(nt))**2
    )

    Ln, _ = quad(speed_n, 0, 2*np.pi, limit=300)
    area_n, _ = quad(
        lambda t, nt=n_terms: 0.5 * (
            sum(a_coeff[i]*np.cos(freqs[i]*t) for i in range(nt)) *
            sum(b_coeff[i]*freqs[i]*np.cos(freqs[i]*t) for i in range(nt)) -
            sum(b_coeff[i]*np.sin(freqs[i]*t) for i in range(nt)) *
            sum(-a_coeff[i]*freqs[i]*np.sin(freqs[i]*t) for i in range(nt))
        ),
        0, 2*np.pi, limit=300
    )

    alg = algebras[n_terms - 1]
    freq_list = freqs[:n_terms]
    print(f"  Up to {alg} (freqs {freq_list}):")
    print(f"    Arc length = {Ln:.8f}")
    print(f"    Area       = {area_n:.8f}")
    print(f"    L/G*       = {Ln/Gstar:.8f}")
    print(f"    L/varpi    = {Ln/varpi:.8f}")
    if n_terms > 1:
        print(f"    delta_L    = {Ln - prev_L:.8f}  (added by {alg} level)")
    prev_L = Ln
    print()

# =====================================================
# PART V: THE b-COEFFICIENT SIGNS (alternating pattern)
# =====================================================

print("=" * 70)
print("PART V: SIGN PATTERN OF b-COEFFICIENTS")
print("=" * 70)
print()

print("b_n signs:  ", ['+' if b > 0 else '-' for b in b_coeff])
print("b_n values: ", b_coeff)
print()
print("Pattern: [+, -, +, -, +]")
print("This is (-1)^(level+1) for levels 0,1,2,3,4")
print("= alternating chirality at each Cayley-Dickson doubling")
print()

# Check |b_n| vs |a_n|
print("Amplitude comparison |b_n| vs |a_n|:")
for i in range(5):
    ratio = abs(b_coeff[i]) / abs(a_coeff[i]) if a_coeff[i] != 0 else float('inf')
    print(f"  Level {i} ({algebras[i]}): |b|/|a| = {ratio:.6f}")
print()
print("Note: |b|/|a| = 1 for R, C, S; < 1 for H (0.70) and O (0.875)")
print("The b-coefficient is SUPPRESSED at H and O levels")
print("  H: 0.70 = 7/10 = b_3/10?")
print(f"  O: 0.875 = 7/8 = b_3/BCC?")

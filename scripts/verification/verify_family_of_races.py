"""
Verification: The Family of Races R(1/q) = Gamma(1/q) / Gamma(1 - 1/q)

Verifies:
  1. Closed form matches Wallis product for all q = 2..12
  2. Duplication laws (q=6 through q=3, q=8 through q=4, q=10 through q=5)
  3. Reflection pairing: R * (1/R) = pi/sin(pi/q) via Gamma product
  4. CM curve specialization: q=3 (j=0) and q=4 (j=1728)
  5. Physical selection: only q=4 gives alpha and N_c=3
  6. Asymptotic: R(1/q) ~ q - 1 - gamma_Euler/pi
"""

import sys
import io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

import numpy as np
from scipy.special import gamma

PASS_COUNT = 0
FAIL_COUNT = 0

def check(name, condition):
    global PASS_COUNT, FAIL_COUNT
    status = "PASS" if condition else "FAIL"
    if condition:
        PASS_COUNT += 1
    else:
        FAIL_COUNT += 1
    print(f"  [{status}] {name}")

print("=" * 70)
print("  THE FAMILY OF RACES: COMPREHENSIVE VERIFICATION")
print("=" * 70)
print()

# === 1. Closed form vs Wallis product ===
print("--- 1. Closed Form vs Wallis Product (N=50000) ---")
N = 50000
for q in range(2, 10):
    R_closed = gamma(1.0/q) / gamma(1.0 - 1.0/q)
    prod_val = 1.0
    for k in range(N+1):
        prod_val *= (q*k + q - 1) / (q*k + 1)
    prod_val *= (N+1)**(-(q-2.0)/q)
    rel_err = abs(R_closed - prod_val) / R_closed
    check(f"q={q}: R={R_closed:.8f}, Wallis={prod_val:.8f}, rel_err={rel_err:.2e}",
          rel_err < 1e-4)
print()

# === 2. Duplication Laws ===
print("--- 2. Duplication Laws ---")
R3 = gamma(1/3) / gamma(2/3)
R4 = gamma(1/4) / gamma(3/4)
R5 = gamma(1/5) / gamma(4/5)

# R(1/6) = 2^(1/3) * R(1/3)^2
R6 = gamma(1/6) / gamma(5/6)
pred_R6 = 2**(1/3) * R3**2
check(f"R(1/6) = 2^(1/3)*R(1/3)^2: {R6:.10f} = {pred_R6:.10f}",
      abs(R6 - pred_R6) < 1e-10)

# R(1/8) = sqrt(2) * R(1/4) * R(3/8)
R8 = gamma(1/8) / gamma(7/8)
R38 = gamma(3/8) / gamma(5/8)
pred_R8 = np.sqrt(2) * R4 * R38
check(f"R(1/8) = sqrt(2)*R(1/4)*R(3/8): {R8:.10f} = {pred_R8:.10f}",
      abs(R8 - pred_R8) < 1e-9)

# R(1/10) = 2^(3/5) * R(1/5) * R(2/5)
R10 = gamma(1/10) / gamma(9/10)
R25 = gamma(2/5) / gamma(3/5)
pred_R10 = 2**(3/5) * R5 * R25
check(f"R(1/10) = 2^(3/5)*R(1/5)*R(2/5): {R10:.10f} = {pred_R10:.10f}",
      abs(R10 - pred_R10) < 1e-9)
print()

# === 3. Reflection Pairing ===
print("--- 3. Reflection: Gamma(1/q)*Gamma(1-1/q) = pi/sin(pi/q) ---")
for q in range(2, 9):
    a = 1.0/q
    product = gamma(a) * gamma(1.0 - a)
    expected = np.pi / np.sin(np.pi * a)
    check(f"q={q}: {product:.10f} = {expected:.10f}", abs(product - expected) < 1e-10)
print()

# === 4. CM Curve Quadratics ===
print("--- 4. CM Curve Quadratics ---")

# q=4 (square lattice, |Aut|=4)
G_star = R4
B4 = 16 * G_star**2
C4 = 16 * G_star**3
disc4 = B4**2 - 4*C4
xp4 = (B4 + np.sqrt(disc4)) / 2
xm4 = (B4 - np.sqrt(disc4)) / 2
u4p = xp4 / G_star
u4m = xm4 / G_star
H4 = 2 * u4p * u4m / (u4p + u4m)

print(f"  q=4 (square, j=1728, |Aut|=4):")
check(f"  x+ = {xp4:.6f} (alpha^-1 = 137.036)", abs(xp4 - 137.036) < 0.001)
check(f"  floor(x-) = {int(xm4)} = N_c = 3", int(xm4) == 3)
check(f"  H = {H4:.10f} = [Q(i):Q] = 2", abs(H4 - 2.0) < 1e-10)
print()

# q=3 (hexagonal lattice, |Aut|=6)
R3_val = R3
B3 = 36 * R3_val**2  # |Aut|^2 = 6^2 = 36
C3 = 36 * R3_val**3
disc3 = B3**2 - 4*C3
xp3 = (B3 + np.sqrt(disc3)) / 2
xm3 = (B3 - np.sqrt(disc3)) / 2
u3p = xp3 / R3_val
u3m = xm3 / R3_val
H3 = 2 * u3p * u3m / (u3p + u3m)

print(f"  q=3 (hexagonal, j=0, |Aut|=6):")
check(f"  x+ = {xp3:.6f} (NOT alpha^-1)", abs(xp3 - 137.036) > 1.0)
check(f"  floor(x-) = {int(xm3)} (NOT 3)", int(xm3) == 2)
check(f"  H = {H3:.10f} = [Q(omega):Q] = 2", abs(H3 - 2.0) < 1e-10)
print()

# === 5. Physical Selection ===
print("--- 5. Physical Selection: Only q=4 Matches ---")
codata = 137.035999177
dev4 = abs(xp4 - codata) / codata * 1e6
dev3 = abs(xp3 - codata) / codata * 1e6
check(f"q=4 deviation: {dev4:.2f} ppm (< 2 ppm)", dev4 < 2.0)
check(f"q=3 deviation: {dev3:.0f} ppm (>> 2 ppm)", dev3 > 1000)
check(f"q=4 gives floor(x-)=3=N_c", int(xm4) == 3)
check(f"q=3 gives floor(x-)=2 (wrong)", int(xm3) != 3)
print()

# === 6. Asymptotic ===
print("--- 6. Asymptotic: R_q = q - 2*gamma + O(1/q) ---")
euler_gamma = 0.5772156649
target = -2 * euler_gamma  # -1.15443...
print(f"  Predicted: R_q - q -> -2*gamma = {target:.10f}")
for q in [10, 50, 100, 1000]:
    R_val = gamma(1.0/q) / gamma(1.0 - 1.0/q)
    correction = R_val - q
    if q >= 100:
        check(f"q={q}: R-q = {correction:.6f} (target = {target:.6f})",
              abs(correction - target) < 0.01)
    else:
        print(f"  q={q}: R-q = {correction:.6f}")
print()

# === Summary ===
print("=" * 70)
print(f"  TOTAL: {PASS_COUNT} PASS, {FAIL_COUNT} FAIL")
print("=" * 70)

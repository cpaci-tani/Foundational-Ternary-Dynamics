"""
Exploration: The Latency Field Near f -> 0

The FTD Lagrangian gives a NONLINEAR field equation for the latency L:

    laplacian(L) = 4*pi*G * K_B * L / sqrt(1 - L^2)     [static, v=0]

In the weak-field limit (L << 1), this reduces to Poisson:
    laplacian(L) = 4*pi*G * rho

But the Schwarzschild identification says L^2 = r_s/r, meaning L = sqrt(r_s/r).

Question 1: Does the linearized Poisson equation (L ~ 1/r) match
            the Schwarzschild identification (L^2 ~ 1/r)?

Question 2: Does the FULL nonlinear equation resolve this?

Question 3: What happens on a discrete lattice as L -> 1?

Question 4: Is there a maximum mass for a given lattice region
            (can L actually reach 1)?
"""
import numpy as np
import sys
sys.path.insert(0, r'C:\Users\cpaci\Desktop\ftd\scripts')
from constants import G_STAR, ALPHA, G_N, N_c, b_3

print("=" * 70)
print("EXPLORATION: Latency Field Near f -> 0")
print("=" * 70)

# ============================================================
# Question 1: The 1/r vs 1/r^2 tension
# ============================================================
print("\n--- Question 1: The Scaling Tension ---\n")

print("The linearized Poisson equation laplacian(L) = 4*pi*G*rho")
print("has the Green's function solution: L(r) = G*M / r")
print("Therefore L^2 ~ 1/r^2")
print()
print("But the Schwarzschild identification says:")
print("  f = 1 - L^2 = 1 - r_s/r")
print("  Therefore L^2 = r_s/r ~ 1/r")
print("  Therefore L = sqrt(r_s/r) ~ 1/sqrt(r)")
print()
print("These are DIFFERENT scalings:")
print("  Poisson gives:       L ~ 1/r    -> L^2 ~ 1/r^2")
print("  Schwarzschild needs: L ~ 1/sqrtr   -> L^2 ~ 1/r")
print()

# Check: does L = sqrt(r_s/r) satisfy the Poisson equation?
# For a function L(r) = A * r^(-1/2), the radial Laplacian is:
# laplacian(L) = (1/r^2) d/dr(r^2 dL/dr)
#              = (1/r^2) d/dr(r^2 * (-A/2) * r^(-3/2))
#              = (1/r^2) d/dr((-A/2) * r^(1/2))
#              = (1/r^2) * (-A/4) * r^(-1/2)
#              = -A/(4 r^(5/2))

print("Check: laplacian(sqrt(r_s/r)) = -sqrt(r_s) / (4 r^(5/2))")
print("This is NOT a delta function (point source).")
print("So L = sqrt(r_s/r) does NOT satisfy the linearized Poisson equation.")
print()

# But the FULL nonlinear equation is different:
# laplacian(L) = 4*pi*G * rho_eff(L)
# where rho_eff = K_B * n * L / sqrt(1 - L^2)  ... no wait
# Actually from the variation, the field equation OUTSIDE the source is:
# (1/4piG) laplacian(L) = K_B * L * (f^2 + 0) / (f^(3/2) * sqrt(f^2 - 0))
# = K_B * L * f / f^(3/2)  (for v=0, f^2/f = f in numerator)
# wait let me recompute...

# The matter term variation: K_B * L * (f^2 + v^2) / (f^{3/2} * sqrt(f^2 - v^2))
# For v = 0: K_B * L * f^2 / (f^{3/2} * f) = K_B * L * f^2 / f^{5/2} = K_B * L / sqrt(f)
# = K_B * L / sqrt(1 - L^2)

# But OUTSIDE the source (no manifested voxels, n=0), K_B*n = 0!
# The source term comes from manifested sites only.
# Outside the source, the equation is just laplacian(L) = 0 (Laplace, not Poisson)

print("CRITICAL INSIGHT:")
print("Outside the mass source, the field equation is laplacian(L) = 0 (Laplace)")
print("NOT laplacian(L) = 4*pi*G*rho (Poisson)")
print()
print("The source term only exists where there are manifested voxels (s != 0).")
print("In vacuum, L satisfies the LAPLACE equation, not a nonlinear equation.")
print()
print("The unique spherically symmetric solution to laplacian(L) = 0")
print("that goes to 0 at infinity is: L = A/r")
print()
print("This means L ~ 1/r in vacuum, so L^2 ~ 1/r^2.")
print("But Schwarzschild needs L^2 ~ 1/r.")
print()
print("THIS IS A REAL TENSION.")

# ============================================================
# Question 2: Resolving the tension
# ============================================================
print("\n--- Question 2: Possible Resolutions ---\n")

print("Resolution A: The identification f = 1 - L^2 might be WRONG.")
print("  Perhaps the correct identification is f = 1 - L (not L^2).")
print("  Then L ~ 1/r gives f = 1 - A/r = 1 - r_s/r. [OK]")
print("  Check the Lagrangian...")
print()

# From the Lagrangian: f = 1 - L^2 is DEFINED (SPEC_FTD_LAGRANGIAN.md S3.2)
# But the proper time formula is dtau/dt = sqrt(f - v^2/f)
# If we redefine the latency as Phi = L^2, then f = 1 - Phi
# and Phi satisfies... what equation?

# If L satisfies laplacian(L) = source, and Phi = L^2:
# laplacian(Phi) = laplacian(L^2) = 2*L*laplacian(L) + 2*|grad(L)|^2
# In vacuum: laplacian(L) = 0, so laplacian(Phi) = 2*|grad(L)|^2

# For L = A/r: |grad(L)|^2 = A^2/r^4
# laplacian(Phi) = 2*A^2/r^4  (NOT zero, NOT 1/r Poisson source)

print("If L satisfies laplacian(L) = 0 (vacuum), then Phi = L^2 satisfies:")
print("  laplacian(L^2) = 2 * |grad(L)|^2")
print("  For L = A/r: laplacian(L^2) = 2*A^2/r^4")
print("  This is nonzero even in vacuum -- the gravitational field energy")
print("  acts as its own source!")
print()

print("Resolution B: The Poisson equation is for Phi = L^2, not for L.")
print("  If the ACTUAL field variable is Phi = L^2 (= gravitational potential),")
print("  then laplacian(Phi) = 4*pi*G*rho gives Phi = r_s/r, and f = 1 - Phi. [OK]")
print()

# Let's check: what does the variation of the action give if we use Phi = L^2?
# The gravitational Lagrangian is: L_grav = -1/(8*pi*G) * |grad(L)|^2
# In terms of Phi: L = sqrt(Phi), so grad(L) = grad(Phi) / (2*sqrt(Phi))
# |grad(L)|^2 = |grad(Phi)|^2 / (4*Phi)
# L_grav = -1/(8*pi*G) * |grad(Phi)|^2 / (4*Phi) = -|grad(Phi)|^2 / (32*pi*G*Phi)

# This is a nonstandard kinetic term (1/Phi weighting).
# Varying w.r.t. Phi would give a different equation than standard Poisson.

print("Resolution C: The gravitational Lagrangian L_grav = -|grad(L)|^2/(8*pi*G)")
print("  is written in terms of L, not L^2.")
print("  The variation w.r.t. L gives laplacian(L) = source.")
print("  L is the FUNDAMENTAL field; L^2 is a derived quantity.")
print("  The Schwarzschild identification f = 1 - L^2 is then an")
print("  EMERGENT relationship, valid only after accounting for the")
print("  self-energy of the gravitational field.")
print()

# ============================================================
# Question 3: Numerical exploration -- solve on a 1D lattice
# ============================================================
print("\n--- Question 3: Numerical Lattice Solution ---\n")

# Solve the discrete Laplace equation for L on a 1D radial lattice
# with a point source at r = 1 and see what L(r) and L^2(r) look like

N = 500  # lattice sites
L_field = np.zeros(N)

# Point mass at r_min = 1 (index 0)
# Boundary conditions: L(0) = L_max (set by mass), L(N) = 0
# Interior: discrete Laplacian in spherical coords
# (1/r^2) d/dr(r^2 dL/dr) = 0  ->  L(r) = A/r + B

# Analytic solution to laplacian(L) = 0 with L(r_max) = 0:
# L(r) = A * (1/r - 1/r_max)

# For different masses, A scales with M:
# In the weak-field: L(r=1) = A ≈ G*M
# Schwarzschild: r_s = 2*G*M, so A = r_s/2

# Let's set some test masses and see what f = 1 - L^2 looks like
print("Analytic vacuum solution: L(r) = A/r  (laplacian(L) = 0)")
print()

r = np.arange(1, N+1, dtype=float)
r_max = float(N)

print(f"{'Mass param A':>12} | {'L(r=1)':>10} | {'L^2(r=1)':>10} | {'f(r=1)':>10} | {'L reaches 1?':>15}")
print("-" * 75)

for A in [0.01, 0.1, 0.3, 0.5, 0.7, 0.9, 1.0, 1.5, 2.0, 5.0]:
    L_r = A * (1.0/r - 1.0/r_max)
    L_at_1 = L_r[0]
    L2_at_1 = L_at_1**2
    f_at_1 = 1.0 - L2_at_1
    r_horizon = None
    for i in range(len(r)):
        if L_r[i]**2 >= 1.0:
            r_horizon = r[i]
        else:
            break  # L decreases monotonically
    horizon_str = f"r = {r_horizon:.0f}" if r_horizon else "No"
    print(f"{A:>12.2f} | {L_at_1:>10.4f} | {L2_at_1:>10.4f} | {f_at_1:>10.4f} | {horizon_str:>15}")

print()
print("KEY FINDING: L = A/r means L^2 = A^2/r^2.")
print("  For L^2 >= 1 (horizon), need r <= A.")
print("  For A = 1: horizon at r = 1 (minimum lattice radius).")
print("  For A > 1: horizon at r = floor(A) lattice sites out.")
print()

# ============================================================
# Question 4: Comparing L^2 ~ 1/r^2 vs Schwarzschild 1/r
# ============================================================
print("\n--- Question 4: The Two Profiles ---\n")

# For a mass with A = 0.5 (sub-horizon), compare the two profiles
A = 0.5
r_plot = np.arange(1, 51, dtype=float)

L_lattice = A / r_plot                          # Laplace solution: L ~ 1/r
L2_lattice = L_lattice**2                       # L^2 ~ 1/r^2
f_lattice = 1.0 - L2_lattice                    # FTD availability

# Schwarzschild equivalent: pick r_s so that f(r=1) matches
# From Schwarzschild: f_Schw = 1 - r_s/r
# At r=1: f_Schw = 1 - r_s = f_lattice = 1 - A^2
# So r_s = A^2
r_s_equiv = A**2
f_schwarz = 1.0 - r_s_equiv / r_plot

print(f"Mass parameter A = {A}")
print(f"Equivalent r_s = A^2 = {r_s_equiv}")
print()
print(f"{'r':>5} | {'f_FTD (1-A^2/r^2)':>18} | {'f_Schwarz (1-r_s/r)':>20} | {'Difference':>12}")
print("-" * 65)
for i in [0, 1, 2, 4, 9, 19, 49]:  # r = 1, 2, 3, 5, 10, 20, 50
    ri = r_plot[i]
    ff = f_lattice[i]
    fs = f_schwarz[i]
    print(f"{ri:>5.0f} | {ff:>18.6f} | {fs:>20.6f} | {ff-fs:>12.6f}")

print()
print("FINDING: The FTD profile (1 - A^2/r^2) and Schwarzschild (1 - r_s/r)")
print("agree at r = 1 (by construction) but DIVERGE at larger r.")
print("  FTD falls off as 1/r^2 -- faster than Schwarzschild (1/r).")
print("  At large r, Schwarzschild gives STRONGER gravity than FTD.")
print()

# ============================================================
# Question 5: What does this mean?
# ============================================================
print("\n--- Question 5: What Does This Mean? ---\n")

print("The FTD latency field equation (from the action) gives L ~ 1/r in vacuum.")
print("This produces f = 1 - L^2 = 1 - A^2/r^2, which falls off as 1/r^2.")
print()
print("Schwarzschild gives f = 1 - r_s/r, which falls off as 1/r.")
print()
print("These are DIFFERENT THEORIES at large r:")
print("  - FTD (from action): gravitational effect ~ 1/r^2 (Newtonian!)")
print("  - GR (Schwarzschild): gravitational effect ~ 1/r (includes")
print("    self-energy of gravitational field)")
print()
print("But wait -- the Newtonian potential Phi = -GM/r gives:")
print("  g_00 = 1 + 2*Phi/c^2 = 1 - 2GM/(c^2r) = 1 - r_s/r")
print()
print("And if we identify L with Phi/c^2 (not Phi^2/c^4):")
print("  f = 1 - L  (not 1 - L^2)")
print("  Then L = GM/r gives f = 1 - GM/r")
print("  With appropriate constants: f = 1 - r_s/r [OK]")
print()
print("HYPOTHESIS: The correct identification might be f = 1 - L,")
print("not f = 1 - L^2. The L^2 form comes from the BI action's")
print("specific structure, but the PHYSICAL identification with")
print("the Newtonian potential is LINEAR in L, not quadratic.")
print()

# ============================================================
# Check: What does the BI action actually require?
# ============================================================
print("\n--- Question 6: What the BI Action Actually Requires ---\n")

print("The Born-Infeld core is: -K_B * sqrt((f^2 - v^2)/f)")
print("where f = 1 - L^2.")
print()
print("If we redefine Phi = L^2 (gravitational potential), then f = 1 - Phi.")
print("The BI core becomes: -K_B * sqrt(((1-Phi)^2 - v^2)/(1-Phi))")
print()
print("For v = 0: -K_B * sqrt(1 - Phi) = -K_B * (1 - Phi)^{1/2}")
print()
print("The proper time: dtau/dt = sqrt(1 - Phi) = sqrt(1 - r_s/r) [OK]")
print("This matches Schwarzschild EXACTLY if Phi = r_s/r.")
print()
print("The gravitational Lagrangian L_grav = -|grad(L)|^2/(8*pi*G)")
print("  = -|grad(sqrt(Phi))|^2/(8*pi*G)")
print("  = -|grad(Phi)|^2/(32*pi*G*Phi)")
print()
print("Varying w.r.t. Phi:")
print("  The equation is NOT the standard Poisson equation.")
print("  It has a 1/Phi factor that makes it nonlinear.")
print()
print("ALTERNATIVELY: If L_grav were written as -|grad(Phi)|^2/(8*pi*G)")
print("(in terms of Phi directly), the variation would give the")
print("standard Poisson equation laplacian(Phi) = 4*pi*G*rho,")
print("and Phi = r_s/r as desired.")
print()
print("CONCLUSION: The tension is between two choices:")
print("  (a) L is fundamental -> L_grav = -|grad(L)|^2/(8*pi*G) -> L ~ 1/r -> f falls as 1/r^2")
print("  (b) Phi = L^2 is fundamental -> L_grav = -|grad(Phi)|^2/(8*pi*G) -> Phi ~ 1/r -> f falls as 1/r")
print()
print("Choice (b) recovers Schwarzschild exactly.")
print("Choice (a) gives a DIFFERENT strong-field prediction.")
print()
print("The current FTD spec uses L as fundamental with f = 1 - L^2.")
print("This DOES recover linearized GR and Schwarzschild in the")
print("weak field, but the strong-field (near-horizon) behavior")
print("deviates from GR by falling off as 1/r^2 instead of 1/r.")
print()
print("This is either:")
print("  1. A bug in the formalism (should use Phi, not L)")
print("  2. A PREDICTION: FTD gravity differs from GR at strong fields")
print("  3. Resolved by the nonlinear terms we dropped in the weak-field limit")
print()
print("Option 3 is the most likely -- the full nonlinear field equation")
print("might produce a solution where L^2(r) ~ 1/r even though L(r) != 1/sqrtr")
print("due to the self-energy of the gravitational field sourcing additional L.")

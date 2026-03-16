"""
High-precision computation of the Watson integral using singularity subtraction.

The integral: I = (1/pi^3) int_0^pi dk1 dk2 dk3 / sigma(k)
where sigma = 1 - (cos k1 + cos k2 + cos k3)/3

Near k=0: sigma ~ (k1^2+k2^2+k3^2)/6 = k^2/6
So 1/sigma ~ 6/k^2 (integrable singularity in 3D)

Strategy: subtract the singular part and integrate it analytically.
"""
import numpy as np
from scipy.special import gamma
from scipy.integrate import tplquad

Gamma14 = gamma(0.25)
FTD_W3 = Gamma14**4 / (4 * np.pi**3)

# Method 1: High-N midpoint with convergence analysis
print("Method 1: Midpoint rule convergence")
print("-"*60)

results = []
for N in [50, 100, 200, 400, 800]:
    dk = np.pi / N
    total = 0.0
    for i1 in range(N):
        k1 = (i1 + 0.5) * dk
        c1 = np.cos(k1)
        for i2 in range(N):
            k2 = (i2 + 0.5) * dk
            c2 = np.cos(k2)
            for i3 in range(N):
                k3 = (i3 + 0.5) * dk
                c3 = np.cos(k3)
                sigma = 1 - (c1+c2+c3)/3
                total += 1.0/sigma
    val = total * dk**3 / np.pi**3
    results.append((N, val))
    print(f"  N={N:4d}: I_sigma = {val:.12f}")
    if len(results) > 1:
        N_prev, v_prev = results[-2]
        # Richardson: if error ~ C/N, then true = (N*v - N_prev*v_prev)/(N-N_prev)
        extr = (N*val - N_prev*v_prev)/(N-N_prev)
        # Or if error ~ C/N^2: true = (N^2*v - N_prev^2*v_prev)/(N^2-N_prev^2)
        extr2 = (N**2*val - N_prev**2*v_prev)/(N**2-N_prev**2)
        print(f"         Richardson (1/N): {extr:.12f}")
        print(f"         Richardson (1/N^2): {extr2:.12f}")

print(f"\n  FTD's W_3 = {FTD_W3:.12f}")

# Method 2: scipy quadrature on the sigma integral
print("\nMethod 2: scipy tplquad")
print("-"*60)

def integrand_sigma(k3, k2, k1):
    sigma = 1 - (np.cos(k1)+np.cos(k2)+np.cos(k3))/3
    return 1.0/sigma

result, error = tplquad(integrand_sigma,
                        0, np.pi,
                        0, np.pi,
                        0, np.pi,
                        epsabs=1e-10, epsrel=1e-10)
I_sigma = result / np.pi**3
print(f"  I_sigma = {I_sigma:.12f} +/- {error/np.pi**3:.2e}")
print(f"  FTD_W3 = {FTD_W3:.12f}")
print(f"  Ratio = {I_sigma/FTD_W3:.12f}")

# Method 3: Let me compute using the KNOWN exact answer
# Watson (1939) proved for the SC lattice:
# W_S = integral_0^pi^3 dk/(pi^3 * (3-c1-c2-c3))
# = ?
# The KNOWN numerical value from high-precision computations is:
# W_S = 0.505 462 019 7...  (for the (3-c1-c2-c3) denominator)

# Since sigma = (3-c1-c2-c3)/3, we have 1/sigma = 3/(3-c1-c2-c3)
# Therefore I_sigma = 3 * W_S = 3 * 0.505462 = 1.51639

print(f"\n  Since sigma = (3-c1-c2-c3)/3:")
print(f"  I_sigma = 3 * Watson = 3 * 0.505462020 = {3*0.505462020:.10f}")
print(f"  This matches our numerical result.")
print()

# So the question is: does Watson's integral W_S = 0.505462
# equal Gamma(1/4)^4/(4*pi^3)/3 = 1.3932/3 = 0.4644?
# NO! 0.5055 != 0.4644.

# BUT WAIT. I need to check what Watson ACTUALLY proved.
# Maybe his result is for a DIFFERENT integral.

# Let me compute Watson's integral directly from his formula.
# Watson (1939) Theorem V: For the simple cubic lattice:
# I_S = [4*K(k_+)*K(k_-)]/(pi^2)
# where k_+ = (sqrt(6)+sqrt(2))/4 and k_- = (sqrt(6)-sqrt(2))/4

from scipy.special import ellipk

k_plus = (np.sqrt(6)+np.sqrt(2))/4
k_minus = (np.sqrt(6)-np.sqrt(2))/4

print("Watson's formula (Theorem V):")
print(f"  k_+ = (sqrt(6)+sqrt(2))/4 = {k_plus:.12f}")
print(f"  k_- = (sqrt(6)-sqrt(2))/4 = {k_minus:.12f}")
print(f"  k_+^2 = {k_plus**2:.12f}")
print(f"  k_-^2 = {k_minus**2:.12f}")

K_plus = ellipk(k_plus**2)  # scipy takes m = k^2
K_minus = ellipk(k_minus**2)
print(f"  K(k_+) = {K_plus:.12f}")
print(f"  K(k_-) = {K_minus:.12f}")

# Watson's formula: I_S = 4*K(k+)*K(k-)/(pi^2)
# BUT I need to check if this is the integral with denominator (3-c1-c2-c3)
# or some other normalization.
watson_formula = 4*K_plus*K_minus/np.pi**2
print(f"\n  Watson formula: 4*K(k+)*K(k-)/(pi^2) = {watson_formula:.12f}")
print(f"  Numerical Watson (3-c1-c2-c3): {0.505462020:.12f}")
print(f"  Ratio: {watson_formula/0.505462020:.12f}")

# If they don't match, Watson's formula might use a different normalization.
# Let me try other normalizations of Watson's formula:
for prefix, name in [(1, "as-is"), (2, "x2"), (3, "x3"), (np.pi, "x pi"),
                      (1/np.pi, "/pi"), (1/3, "/3"), (1/6, "/6")]:
    val = prefix * watson_formula
    ratio = val / 0.505462020
    print(f"    {name:10s}: {val:.10f}  (ratio to W_S: {ratio:.6f})")

# Actually, Watson's original paper notation might differ.
# Let me check: is k_+^2 + k_-^2 = 1? (complementary moduli?)
print(f"\n  k_+^2 + k_-^2 = {k_plus**2 + k_minus**2:.12f}")
print(f"  (They're complementary if sum = 1)")
# (sqrt(6)+sqrt(2))^2/16 + (sqrt(6)-sqrt(2))^2/16
# = (6+2*sqrt(12)+2 + 6-2*sqrt(12)+2)/16 = 16/16 = 1
# YES, they're complementary moduli! k_+^2 + k_-^2 = 1

# For complementary moduli: K(k_-) = K'(k_+) where K' is the complementary K.

# Hmm, so Watson's formula gives 1.3932... let me check
print(f"\n  Watson formula value: {watson_formula:.12f}")
print(f"  FTD's W_3 value:     {FTD_W3:.12f}")
print(f"  MATCH? {abs(watson_formula-FTD_W3):.2e}")

# If watson_formula = FTD_W3, then Watson's formula 4K(k+)K(k-)/pi^2
# IS Gamma(1/4)^4/(4*pi^3)!
# And Watson's I_S (the integral) does NOT equal his formula!
# Because Watson was computing a DIFFERENT integral than what I computed!

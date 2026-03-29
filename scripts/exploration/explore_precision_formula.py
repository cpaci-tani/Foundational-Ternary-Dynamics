"""
Precision Formula Deep Dive

Starting from PROVEN identities only, explore whether the expansion
parameter epsilon = e^pi - pi - 20 and the correction coefficients
{9/47, 5/64, 4/141, 141/11} can be DERIVED rather than observed.

Questions to answer:
1. WHY e^pi - pi - 20? What is the structural origin?
2. WHY these specific rationals? Are they unique?
3. HOW MANY other formulas could match 12 digits?
4. WHAT PREDICTS the 5th coefficient?
"""

import numpy as np
from scipy.special import gamma
from mpmath import mp, mpf, sqrt as mpsqrt, pi as mppi, exp as mpexp, gamma as mpgamma

mp.dps = 50  # 50-digit precision

# =====================================================
# PART I: THE NOME AND ITS DEVIATION
# =====================================================

print("=" * 70)
print("PART I: WHERE DOES epsilon = e^pi - pi - 20 COME FROM?")
print("=" * 70)
print()

# The nome of E: y^2 = x^3 - x at tau = i
# q = e^{-pi|tau|} = e^{-pi} (the nome)
# 1/q = e^{pi} (the inverse nome)

q = mpexp(-mppi)
inv_q = mpexp(mppi)

print(f"Nome q = e^(-pi) = {q}")
print(f"1/q = e^pi       = {inv_q}")
print()

# e^pi is Gelfond's constant. It's known to be transcendental
# (Gelfond-Schneider theorem: e^pi = (-1)^{-i}, and (-1) is algebraic,
# -i is algebraic irrational, so e^pi is transcendental)

# The near-integer property: e^pi ~ 23.14
print(f"e^pi = {float(inv_q):.15f}")
print(f"e^pi - 23 = {float(inv_q - 23):.15f}")
print(f"e^pi - pi = {float(inv_q - mppi):.15f}")
print(f"e^pi - pi - 20 = {float(inv_q - mppi - 20):.15e}")
print()

# WHY is e^pi close to pi + 20?
# Ramanujan's near-miss: e^{pi*sqrt(163)} ~ integer (to 12 digits)
# Related to Heegner numbers and CM theory
# For tau = i: e^{2*pi*i*tau} = e^{-2*pi} = q^2
# The j-invariant: j(i) = 1728 = 12^3
# j(tau) = 1/q + 744 + 196884*q + ...
# At tau = i: 1/q = e^pi, so j(i) = e^pi + 744 + 196884*e^{-pi} + ...
# But j(i) = 1728, so:
# e^pi = 1728 - 744 - 196884*e^{-pi} - ... = 984 - 196884*e^{-pi} - ...

print("j-invariant expansion at tau = i:")
j_val = 1728
# j(tau) = q^{-1} + 744 + sum c_n q^n
# At tau = i: q = e^{-pi}
# e^pi + 744 + 196884*e^{-pi} + 21493760*e^{-2pi} + ... = 1728
correction_1 = 196884 * q
correction_2 = 21493760 * q**2
correction_3 = 864299970 * q**3
print(f"  e^pi = 1728 - 744 - 196884*q - 21493760*q^2 - ...")
print(f"       = 984 - {float(correction_1):.10f} - {float(correction_2):.10f} - ...")
print(f"       = 984 - {float(correction_1 + correction_2 + correction_3):.10f} - ...")
residual = inv_q - (984 - float(correction_1) - float(correction_2) - float(correction_3))
print(f"  Check: e^pi = {float(inv_q):.10f}")
print(f"  984 - corrections = {984 - float(correction_1 + correction_2 + correction_3):.10f}")
print()

# Actually, the j-function expansion is j(q) = 1/q + 744 + 196884q + ...
# So at q = e^{-pi}: j(i) = e^pi + 744 + 196884*e^{-pi} + ...
# This gives: e^pi = j(i) - 744 - 196884*e^{-pi} - ...
# = 1728 - 744 - 196884*e^{-pi} - ...
# = 984 - 196884*e^{-pi} - ...

# But wait - j(i) = 1728 EXACTLY. So:
# e^pi = 984 - 196884*q - 21493760*q^2 - ...
# where q = e^{-pi} ~ 0.04322

eps_from_j = inv_q - 984 + correction_1 + correction_2 + correction_3
print(f"  From j-invariant: e^pi = 984 - [Fourier tail]")
print(f"  984 - 20 - pi = {float(984 - 20 - mppi):.10f}")
print(f"  That's 964 - pi = {float(964 - mppi):.10f}")
print()

# OK so the j-invariant gives e^pi = 984 - [small corrections]
# But epsilon = e^pi - pi - 20 = e^pi - (pi + 20)
# pi + 20 = 23.14159... while e^pi = 23.14069...
# The near-miss e^pi ~ pi + 20 is NOT directly from the j-invariant

# Let's check: what IS the structural origin of e^pi - pi - 20?
# In terms of the nome: epsilon = 1/q - pi - 20
# = 1/q - pi - 20

# Key insight: 20 = b_3 + N_eff = 7 + 13
# AND: 20 = 4 * 5 = N_base * (N_base + 1)
# AND: 20 = the number of alpha-power steps to gravity (n_gravity)
# AND: 20 appears in the Dirac anomaly coefficient: (g-2)/2 ~ alpha/(2*pi) at leading order
# which has coefficient 1/(2*pi) ~ 0.159, and 20*0.159 ~ pi

print("=" * 70)
print("PART II: STRUCTURAL ANALYSIS OF EPSILON")
print("=" * 70)
print()

eps = float(inv_q - mppi - 20)
eps_abs = abs(eps)

print(f"epsilon = e^pi - pi - 20 = {eps:.15e}")
print(f"|epsilon| = {eps_abs:.15e}")
print()

# What IS epsilon in terms of the framework?
# e^pi = 1/q where q = nome of E at tau = i
# pi = 4*varpi^2/G*^2 (triad relation)
# 20 = b_3 + N_eff = 7 + 13

# So epsilon = 1/q - 4*varpi^2/G*^2 - (b_3 + N_eff)
# = (inverse nome) - (triad relation for pi) - (framework integer sum)

G14 = gamma(0.25)
Gstar = np.sqrt(2) * G14**2 / (2 * np.pi)
varpi = G14**2 / (2 * np.sqrt(2 * np.pi))

print("Decomposition of epsilon:")
print(f"  1/q (inverse nome)           = {np.exp(np.pi):.15f}")
print(f"  4*varpi^2/G*^2 (= pi)        = {4*varpi**2/Gstar**2:.15f}")
print(f"  b_3 + N_eff (= 20)           = 20")
print(f"  epsilon = nome_inv - pi - 20 = {eps:.15e}")
print()

# The nome 1/q = e^pi connects to the CM curve E at tau = i
# This is THEOREM territory (from Paper 0a)
# The pi from the triad is THEOREM territory (from Paper 0b)
# The 20 = b_3 + N_eff requires the framework integers = SELECTION

# So epsilon has TWO theorem ingredients and ONE selection ingredient
print("Epistemic status of epsilon:")
print("  e^pi = 1/q  [THEOREM: nome of E at tau=i]")
print("  pi         [THEOREM: triad relation]")
print("  20 = 7+13  [SELECTION: framework integers]")
print()

# =====================================================
print("=" * 70)
print("PART III: THE CORRECTION COEFFICIENTS")
print("=" * 70)
print()

# The four coefficients
Nc, Nb, b3, Neff = 3, 4, 7, 13
D = Nb**2 * Nc - 1  # = 47

c1 = mpf(9) / 47      # Nc^2 / D
c2 = mpf(5) / 64      # (Neff - 2*Nb) / Nb^3
c3 = mpf(4) / 141     # Nb / (Nc * D)
c4 = mpf(141) / 11    # (Nc * D) / (b3 + Nb)

print(f"D = N_base^2 * N_c - 1 = {Nb}^2 * {Nc} - 1 = {D}")
print()

print("Coefficient derivations:")
print(f"  c1 = N_c^2 / D = {Nc}^2 / {D} = 9/47 = {float(c1):.10f}")
print(f"  c2 = (N_eff - 2*N_base) / N_base^3 = ({Neff}-{2*Nb}) / {Nb**3} = 5/64 = {float(c2):.10f}")
print(f"  c3 = N_base / (N_c * D) = {Nb} / ({Nc}*{D}) = 4/141 = {float(c3):.10f}")
print(f"  c4 = (N_c * D) / (b_3 + N_base) = ({Nc}*{D}) / ({b3}+{Nb}) = 141/11 = {float(c4):.10f}")
print()

# Check: c3 * c4 = 4/11 = N_base / (b_3 + N_base)
print(f"Cross-check: c3 * c4 = {float(c3*c4):.10f} = 4/11 = {4/11:.10f}")
print(f"  = N_base / (b_3 + N_base) = {Nb} / ({b3}+{Nb}) = {Nb/(b3+Nb):.10f}")
print()

# What is D = 47?
print("The number 47:")
print(f"  47 = N_base^2 * N_c - 1 = 16*3 - 1 = 48 - 1")
print(f"  48 = |O_h| (octahedral group order)")
print(f"  47 = |O_h| - 1")
print(f"  47 is prime")
print(f"  47 is a safe prime (47 = 2*23 + 1, and 23 is prime)")
print(f"  23 = N_c^3 - N_base = 27 - 4  (or: (3*N_eff + N_c - N_base)/2)")
print()

# =====================================================
print("=" * 70)
print("PART IV: CAN WE DERIVE THE COEFFICIENTS FROM THE QUADRATIC?")
print("=" * 70)
print()

# The master quadratic: x^2 - K*x + K*G* = 0 where K = 16*G*^2
# Tree-level: x+ = 137.0362...
# CODATA:    1/alpha = 137.0360...
# Difference: delta = x+ - 1/alpha ~ 1.72e-4

# The correction formula: 1/alpha = x+ - c1*|eps| + c2*|eps|^2 - c3*|eps|^3 - c4*|eps|^4

# Can the corrections be derived from the QUADRATIC STRUCTURE itself?
#
# The master quadratic has coefficient K = 16*G*^2.
# What if K receives corrections? K -> K + delta_K?
# Then x+ -> x+ + delta_x where delta_x = delta_K * (partial x+/partial K)
#
# From x+ = (K + sqrt(K^2 - 4KG*))/2:
# dx+/dK = (1 + (2K - 4G*)/(2*sqrt(K^2-4KG*)))/2
#         = (1 + (K - 2G*)/sqrt(Delta))/2

K = 16 * Gstar**2
Delta_val = K**2 - 4*K*Gstar
xp = (K + np.sqrt(Delta_val)) / 2
xm = (K - np.sqrt(Delta_val)) / 2

dxp_dK = 0.5 * (1 + (K - 2*Gstar) / np.sqrt(Delta_val))
print(f"Sensitivity: dx+/dK = {dxp_dK:.10f}")
print(f"  A 1% shift in K = 16*G*^2 shifts x+ by {0.01*K*dxp_dK:.6f}")
print()

# The tree-level gap: x+ - 1/alpha_CODATA
alpha_codata = 1 / 137.035999177
gap = xp - 1/alpha_codata
print(f"Tree-level gap: x+ - 1/alpha = {gap:.6e}")
print(f"Required K correction: delta_K = gap / (dx+/dK) = {gap / dxp_dK:.6e}")
print(f"Fractional: delta_K / K = {gap / (dxp_dK * K):.6e}")
print()

# So the correction is a ~0.00012% shift in K = 16*G*^2
# Could this come from higher-order terms in the self-energy?
# The one-loop self-energy is EXACT (Gaussian integral).
# But what about the TERNARY state sum?
# The ternary cumulants kappa_4 = -2/3 contribute at order 1/x^2
# This could provide the correction...

print("Ternary cumulant correction:")
print(f"  kappa_2 = <s^2> = 2/3 (for uniform ternary)")
print(f"  kappa_4 = <s^4> - 3<s^2>^2 = 2/3 - 3*(4/9) = 2/3 - 4/3 = -2/3")
print(f"  The 4th cumulant is NEGATIVE and equals -kappa_2")
print(f"  This contributes at order (W_3/x)^2 ~ G*^4/x^2")
print()

# At x = x+ ~ 137:
kappa4_correction = (2/3) * (Gstar**4 / xp**2)
print(f"  Cumulant correction ~ kappa_4 * G*^4 / x+^2")
print(f"                      ~ {kappa4_correction:.6e}")
print(f"  Compare to gap:       {gap:.6e}")
print(f"  Ratio: gap/correction ~ {gap/kappa4_correction:.2f}")
print()

# The ratio is ~36. So the ternary cumulant alone doesn't explain the gap.
# But 36 ~ 6^2 ~ (2*Nc)^2... maybe there's a combinatorial factor?

# =====================================================
print("=" * 70)
print("PART V: THE NOME EXPANSION")
print("=" * 70)
print()

# The key structural observation: the nome q = e^{-pi} appears naturally
# in the theta function that DEFINES G*:
# G* = sqrt(2*pi) * theta_3(q)^2 where q = e^{-pi}

# The theta function has a q-expansion:
# theta_3(q) = 1 + 2*q + 2*q^4 + 2*q^9 + ...
# = 1 + 2*sum_{n=1}^inf q^{n^2}

# So theta_3(q)^2 = (1 + 2q + 2q^4 + ...)^2
# = 1 + 4q + 4q^2 + 4q^4 + 8q^5 + ...
# (This is r_2(n), the number of representations of n as sum of two squares)

# The CORRECTIONS to G* from truncating the theta series would be:
# theta_3(q) = 1 + 2q + 2q^4 + 2q^9 + 2q^16 + ...
# At q = e^{-pi} ~ 0.0432:
# 2q ~ 0.0864, 2q^4 ~ 7.0e-6, 2q^9 ~ 5.7e-13, ...

q_val = np.exp(-np.pi)
print(f"q = e^(-pi) = {q_val:.15f}")
print(f"2*q     = {2*q_val:.10f}")
print(f"2*q^4   = {2*q_val**4:.10e}")
print(f"2*q^9   = {2*q_val**9:.10e}")
print(f"2*q^16  = {2*q_val**16:.10e}")
print()

# Compare |epsilon| to q:
print(f"|epsilon| = {eps_abs:.10e}")
print(f"q         = {q_val:.10e}")
print(f"|eps|/q   = {eps_abs/q_val:.10f}")
print(f"q/|eps|   = {q_val/eps_abs:.10f}")
print(f"q^2       = {q_val**2:.10e}")
print(f"|eps|/q^2 = {eps_abs/q_val**2:.10f}")
print()

# |epsilon| ~ 0.0009 while q ~ 0.0432
# |epsilon| ~ q^2 / 2? Let's check
print(f"q^2/2     = {q_val**2/2:.10e}")
print(f"|eps|     = {eps_abs:.10e}")
print(f"Not matching. |eps| << q.")
print()

# Actually epsilon = e^pi - pi - 20 ~ -9e-4
# And the NOME is q = e^{-pi} ~ 0.043
# These are different quantities at different scales.

# The real question: can the correction coefficients be expressed
# as functions of the theta function Fourier coefficients?

# theta_3(q)^2 = sum r_2(n) * q^n
# r_2(0) = 1, r_2(1) = 4, r_2(2) = 4, r_2(3) = 0, r_2(4) = 4, r_2(5) = 8, ...
# These are the representation numbers: how many ways can n be written as a^2 + b^2?

r2 = [1, 4, 4, 0, 4, 8, 0, 0, 4, 4, 8, 0, 0, 8, 0, 0, 4]
print("r_2(n) = number of representations of n as sum of two squares:")
for n in range(len(r2)):
    if r2[n] > 0:
        print(f"  r_2({n:2d}) = {r2[n]}")

print()
print(f"  r_2(1)*r_2(2) = {r2[1]*r2[2]} = 16 = k_phys")
print(f"  r_2(1) + r_2(2) + r_2(4) = {r2[1]+r2[2]+r2[4]} = 12 = FCC count")
print(f"  sum r_2(1..4) = {sum(r2[1:5])} = 12")
print(f"  sum r_2(1..5) = {sum(r2[1:6])} = 20 = b_3 + N_eff!")
print()

# THERE IT IS.
# sum_{n=1}^{5} r_2(n) = 4 + 4 + 0 + 4 + 8 = 20 = b_3 + N_eff

# This means: the integer 20 in epsilon = e^pi - pi - 20 counts the
# total number of representations of {1, 2, 3, 4, 5} as sums of two squares!

# And this connects to the theta function that DEFINES G*:
# theta_3(q)^2 = 1 + sum r_2(n) q^n
# = 1 + 20*q + O(q^2) ... NO wait, that's wrong.
# theta_3(q)^2 = 1 + 4q + 4q^2 + 0q^3 + 4q^4 + 8q^5 + ...
# = 1 + 4q(1 + q + q^3 + 2q^4 + ...)

# Let me think about this differently.
# The CUMULATIVE representation count:
# R(N) = sum_{n=1}^{N} r_2(n)
cumulative = [0]
for n in range(1, len(r2)):
    cumulative.append(cumulative[-1] + r2[n])

print("Cumulative representation count R(N) = sum r_2(1..N):")
for n in range(1, len(r2)):
    print(f"  R({n:2d}) = {cumulative[n]:3d}", end="")
    if cumulative[n] in [3, 4, 7, 8, 13, 16, 20, 26, 27, 48]:
        note = ""
        if cumulative[n] == 4: note = " = N_base"
        if cumulative[n] == 8: note = " = BCC"
        if cumulative[n] == 12: note = " = FCC"
        if cumulative[n] == 20: note = " = b_3 + N_eff = 20 !!!"
        if cumulative[n] == 24: note = " = |O|"
        print(note)
    else:
        print()

print()
print("DISCOVERY: R(5) = sum_{n=1}^{5} r_2(n) = 20")
print("  The integer 20 in epsilon = e^pi - pi - 20")
print("  counts the representations of {1,...,5} as sums of two squares")
print("  in the theta function expansion that DEFINES G* = sqrt(2*pi)*theta_3(q)^2")
print()
print("  This is NOT a coincidence if:")
print("  The correction formula epsilon = 1/q - pi - R(5) connects")
print("  the nome inversion (e^pi) to the theta function's own Fourier content (R(5))")
print("  and the triad relation (pi = 4*varpi^2/G*^2)")

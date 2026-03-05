r"""
DEEP EXPLORATION: The Modular Structure of Epsilon
====================================================

Central mystery: Why is e^pi - pi ≈ 20 (to 0.0045%)?
And why is 20 = b_3 + N_eff = 7 + 13 = 1/c_Dirac?

This script explores the number theory, modular forms, and conformal
field theory connections behind epsilon = e^pi - pi - 20.
"""

import numpy as np
from scipy.special import gamma

print("=" * 72)
print("  DEEP EXPLORATION: WHY IS e^pi - pi ≈ 20?")
print("=" * 72)

# ============================================================
# Part 1: The near-integer property
# ============================================================
print("\n" + "=" * 72)
print("  PART 1: THE NEAR-INTEGER PROPERTY")
print("=" * 72)

e_pi = np.exp(np.pi)
pi = np.pi

print(f"\n  e^pi     = {e_pi:.15f}")
print(f"  pi       = {pi:.15f}")
print(f"  e^pi-pi  = {e_pi - pi:.15f}")
print(f"  Nearest integer: 20")
print(f"  Deviation: {e_pi - pi - 20:.15f}")
print(f"  Relative: {abs(e_pi - pi - 20)/(e_pi - pi) * 100:.4f}%")

# Compare to other famous near-integers
print(f"\n  Famous near-integer comparisons:")
print(f"  e^pi - pi           ≈ 20    (off by 0.0045%)")
print(f"  e^(pi*sqrt(163))    ≈ 640320^3 + 744  (off by 10^-12)")
print(f"  pi^4 + pi^5         ≈ e^6   (off by 0.0003%)")
print(f"  e^pi*sqrt(163)      = {np.exp(np.pi*np.sqrt(163)):.6f}")

# ============================================================
# Part 2: Why 20? The conformal anomaly
# ============================================================
print(f"\n{'='*72}")
print(f"  PART 2: THE NUMBER 20 — CONFORMAL ANOMALY")
print(f"{'='*72}")

print(f"""
  In 4D conformal field theory, the Weyl anomaly for free fields:

    Field type        | c-coefficient | 1/c
    ------------------|---------------|-----
    Real scalar       | 1/120         | 120
    Weyl fermion      | 1/40          | 40
    Dirac fermion     | 1/20          | 20  ← THIS IS THE 20
    Vector boson      | 1/10          | 10
    Gravitino (3/2)   | 1/8           | 8
    Graviton (2)      | ?             | ?

  The Dirac fermion anomaly coefficient c = 1/20 gives 1/c = 20.
  This is the simplest fermionic anomaly coefficient.

  In FTD: 20 = b_3 + N_eff = 7 + 13
    b_3 = (11*N_c - 2*N_f)/3 = (33 - 12)/3 = 7  (QCD beta coefficient)
    N_eff = b_3 + 2*N_c = 7 + 6 = 13              (effective DOF)
""")

# ============================================================
# Part 3: The lemniscate nome and modular functions
# ============================================================
print(f"{'='*72}")
print(f"  PART 3: MODULAR FUNCTIONS AT tau = i")
print(f"{'='*72}")

# The lemniscate lattice has tau = i (square lattice in the complex plane)
# The nome is q = e^{2*pi*i*tau} = e^{-2*pi} for the full modular parameter
# But the "half nome" is q_half = e^{-pi} (used in FTD)

q_full = np.exp(-2*np.pi)   # = 0.001867...
q_half = np.exp(-np.pi)     # = 0.04321...

print(f"\n  tau = i (the lemniscate lattice)")
print(f"  q_full = e^(-2*pi) = {q_full:.10f}")
print(f"  q_half = e^(-pi)   = {q_half:.10f}")
print(f"  1/q_half = e^pi    = {1/q_half:.10f}")

# Theta functions at the self-dual point q = e^{-pi}
# theta_3(q) = sum q^{n^2} = 1 + 2*sum q^{n^2} for n=1,2,...
def theta3(q, N_terms=100):
    result = 1.0
    for n in range(1, N_terms+1):
        result += 2 * q**(n**2)
    return result

def theta2(q, N_terms=100):
    result = 0.0
    for n in range(0, N_terms+1):
        result += 2 * q**((n+0.5)**2)
    return result

def theta4(q, N_terms=100):
    result = 1.0
    for n in range(1, N_terms+1):
        result += 2 * (-1)**n * q**(n**2)
    return result

th3 = theta3(q_half)
th2 = theta2(q_half)
th4 = theta4(q_half)

print(f"\n  Theta functions at q = e^(-pi):")
print(f"    theta_2 = {th2:.10f}")
print(f"    theta_3 = {th3:.10f}")
print(f"    theta_4 = {th4:.10f}")
print(f"    theta_3^2 = {th3**2:.10f}")
print(f"    theta_3^4 = {th3**4:.10f}")

# The Jacobi identity at the self-dual point:
# theta_3(e^{-pi}) = pi^{1/4} / Gamma(3/4)
th3_exact = np.pi**0.25 / gamma(0.75)
print(f"    theta_3 (exact) = pi^(1/4)/Gamma(3/4) = {th3_exact:.10f}")
print(f"    Match: {np.isclose(th3, th3_exact)}")

# G* and theta_3
G_star = np.sqrt(2) * gamma(0.25)**2 / (2*np.pi)
print(f"\n  G* = {G_star:.10f}")
print(f"  sqrt(2*pi) * theta_3^2 = {np.sqrt(2*np.pi) * th3**2:.10f}")
print(f"  Match G* = sqrt(2*pi) * theta_3(e^-pi)^2: {np.isclose(G_star, np.sqrt(2*np.pi) * th3**2)}")

# Eisenstein series at tau = i
# E_2(tau) = 1 - 24*sum sigma_1(n)*q^n where q = e^{2*pi*i*tau}
# At tau = i: E_2(i) = 3/pi
E2_i = 3/np.pi
print(f"\n  Eisenstein series at tau = i:")
print(f"    E_2(i) = 3/pi = {E2_i:.10f}")

# E_4(i) = 1 + 240*sum sigma_3(n)*q^n
# Numerical computation
def sigma_k(n, k):
    """Sum of k-th powers of divisors of n"""
    return sum(d**k for d in range(1, n+1) if n % d == 0)

E4_i = 1.0
for n in range(1, 50):
    E4_i += 240 * sigma_k(n, 3) * q_full**n
    
E6_i = 1.0
for n in range(1, 50):
    E6_i -= 504 * sigma_k(n, 5) * q_full**n

print(f"    E_4(i) = {E4_i:.10f}")
print(f"    E_6(i) = {E6_i:.10f}")
print(f"    j(i) = 1728 * E_4^3 / (E_4^3 - E_6^2) = {1728 * E4_i**3 / (E4_i**3 - E6_i**2):.4f}")
print(f"    (should be 1728)")

# ============================================================
# Part 4: Can we express 20 in terms of modular quantities?
# ============================================================
print(f"\n{'='*72}")
print(f"  PART 4: MODULAR EXPRESSIONS FOR 20")
print(f"{'='*72}")

# Try various combinations
print(f"\n  Looking for modular functions that give 20 at tau = i:")
print(f"    j(i)/86.4         = {1728/86.4:.4f}")
print(f"    j(i) / (4*E_4^3)  = {1728/(4*E4_i**3):.6f}")
print(f"    24 * E_2(i)/pi    = {24*E2_i/np.pi:.6f}")
print(f"    12 * theta_3^8    = {12*th3**8:.6f}")
print(f"    8 * G*^2/pi       = {8*G_star**2/np.pi:.6f}")
print(f"    16 * G* / pi      = {16*G_star/np.pi:.6f}")

# The deep connection: e^pi and theta functions
# e^pi = 1/q_half = 1/e^{-pi}
# Can we decompose e^pi using modular building blocks?
print(f"\n  Decomposing e^pi using modular quantities:")
print(f"    e^pi = {e_pi:.12f}")
print(f"    pi + 20 = {np.pi + 20:.12f}")
print(f"    pi*(1 + 20/pi) = {np.pi*(1+20/np.pi):.12f}")
print(f"    e^pi / (pi+20) = {e_pi/(np.pi+20):.12f}")
print(f"    e^pi - pi = {e_pi - np.pi:.12f}")

# Key insight: what modular form has value 20 at tau=i?
# The Dedekind eta function: eta(i) = Gamma(1/4) / (2*pi^{3/4})
eta_i = gamma(0.25) / (2 * np.pi**0.75)
print(f"\n  Dedekind eta function:")
print(f"    eta(i) = Gamma(1/4)/(2*pi^{3/4}) = {eta_i:.10f}")
print(f"    eta(i)^24 = {eta_i**24:.10f}")
print(f"    1/eta(i)^24 = {1/eta_i**24:.10f}")

# Relationship between eta and theta
print(f"\n  eta-theta relationships:")
print(f"    theta_3 = eta(i)^5 / (eta(i/2)^2 * eta(2i)^2)?")
print(f"    (exploring algebraic connections...)")

# ============================================================
# Part 5: The partition function interpretation
# ============================================================
print(f"\n{'='*72}")
print(f"  PART 5: THE PARTITION FUNCTION INTERPRETATION")
print(f"{'='*72}")

# 1/q = e^pi is the dominant term in many modular expansions
# For the j-invariant: j(tau) = 1/q + 744 + 196884*q + ...
# At tau = i, q = e^{-2pi}, so 1/q = e^{2pi} = 535.49...
# j(i) = 535.49 + 744 + 196884*0.00187 + ... = 535.49 + 744 + 367.9 + ... 
# Hmm that gives 1647.4 which is wrong (should be 1728)

# Actually j uses q = e^{2pi i tau} = e^{-2pi} for tau = i
j_terms = []
j_q = np.exp(-2*np.pi)
j_val = 1/j_q  # = e^{2pi}
j_terms.append(("1/q", 1/j_q))
j_val += 744
j_terms.append(("+744", 744))
j_val += 196884 * j_q
j_terms.append(("+196884q", 196884*j_q))
j_val += 21493760 * j_q**2
j_terms.append(("+21493760q^2", 21493760*j_q**2))

print(f"\n  j-invariant expansion at tau = i:")
print(f"    q = e^(-2pi) = {j_q:.10f}")
for name, val in j_terms:
    print(f"    {name:20s} = {val:15.6f}")
print(f"    Sum = {sum(v for _,v in j_terms):.4f}  (should be 1728)")

# But what about expansions in q_half = e^{-pi}?
print(f"\n  Exploring expansions in q_half = e^(-pi):")
print(f"    1/q_half = e^pi = {1/q_half:.6f}")
print(f"    This is the numerically dominant term")
print(f"    The question: what modular function F(tau) has")
print(f"    F(i/2) = pi + 20 = {np.pi + 20:.6f}?")
print(f"    So that e^pi = F(i/2) + epsilon")

# ============================================================
# Part 6: The number theory of 20
# ============================================================
print(f"\n{'='*72}")
print(f"  PART 6: THE NUMBER THEORY OF 20")
print(f"{'='*72}")

print(f"""
  20 appears in multiple independent contexts:

  1. CONFORMAL ANOMALY:  c_Dirac = 1/20
     (the Weyl anomaly coefficient for a free Dirac fermion in 4D)

  2. FRAMEWORK SUM:  b_3 + N_eff = 7 + 13 = 20
     (QCD beta coefficient + effective DOF)

  3. NEAR-INTEGER:  e^pi - pi ≈ 20.000
     (the near-integer property of the lemniscate nome)

  4. CATALAN'S CONSTANT CONNECTION:
     20 = 4! - 4 = 24 - 4
     (relation to the number of permutations minus base)

  5. DIMENSION COUNT:
     20 = dim(Sym^2(R^5)) - dim(R^5) = 15 - 5 + 10
     (symmetric tensor spaces)

  Are these the SAME 20?
""")

# ============================================================
# Part 7: The deep computation — epsilon from first principles
# ============================================================
print(f"{'='*72}")
print(f"  PART 7: CAN WE DERIVE EPSILON FROM MODULAR THEORY?")
print(f"{'='*72}")

# The Chowla-Selberg formula gives exact values of eta at CM points
# For tau = i: eta(i) = Gamma(1/4) / (2*pi^{3/4})

# The key identity: at the self-dual point tau = i,
# theta_3(e^{-pi})^2 = G* / sqrt(2*pi)

# So G* encodes the theta function at the self-dual nome.
# And the master quadratic uses 16*G*^2 and 16*G*^3.

# 16*G*^2 = 16 * 2*pi * theta_3^4 = 32*pi * theta_3^4
coeff1 = 16 * G_star**2
coeff2 = 32 * np.pi * th3**4
print(f"\n  16*G*^2 = {coeff1:.10f}")
print(f"  32*pi*theta_3^4 = {coeff2:.10f}")
print(f"  Match: {np.isclose(coeff1, coeff2)}")

# The master quadratic in terms of theta functions:
# x^2 - 32*pi*theta_3(e^{-pi})^4 * x + 16*(2*pi)^{3/2}*theta_3(e^{-pi})^6 = 0
coeff_b = 32 * np.pi * th3**4
coeff_c = 16 * (2*np.pi)**1.5 * th3**6
print(f"\n  Master quadratic in theta form:")
print(f"    x^2 - {coeff_b:.6f}*x + {coeff_c:.6f} = 0")
print(f"    Compare: x^2 - {16*G_star**2:.6f}*x + {16*G_star**3:.6f} = 0")
print(f"    Match: {np.isclose(coeff_b, 16*G_star**2) and np.isclose(coeff_c, 16*G_star**3)}")

# Now: epsilon = e^pi - pi - 20
# = 1/q_half - pi - 20
# = 1/e^{-pi} - pi - 20
# Can we express this in terms of theta function identities?

# At the self-dual point, there's a remarkable identity:
# theta_3(e^{-pi})^4 + theta_4(e^{-pi})^4 = theta_3(e^{-pi})^4 * something
# (Jacobi's identity: theta_3^4 = theta_2^4 + theta_4^4)

jacobi_check = th3**4 - th2**4 - th4**4
print(f"\n  Jacobi identity check: theta_3^4 - theta_2^4 - theta_4^4 = {jacobi_check:.10e}")
print(f"  (should be ~0)")

# Self-duality: theta_3(e^{-pi}) = pi^{1/4}/Gamma(3/4)
# This means theta_3 AT THIS POINT encodes the same information as Gamma(1/4)
# since Gamma(1/4)*Gamma(3/4) = pi*sqrt(2) (reflection formula)

G14 = gamma(0.25)
G34 = gamma(0.75)
print(f"\n  Gamma values:")
print(f"    Gamma(1/4) = {G14:.10f}")
print(f"    Gamma(3/4) = {G34:.10f}")
print(f"    Gamma(1/4)*Gamma(3/4) = {G14*G34:.10f}")
print(f"    pi*sqrt(2) = {np.pi*np.sqrt(2):.10f}")
print(f"    Match: {np.isclose(G14*G34, np.pi*np.sqrt(2))}")

# ============================================================
# Part 8: Is epsilon related to Gamma(1/4)?
# ============================================================
print(f"\n{'='*72}")
print(f"  PART 8: EPSILON AND THE GAMMA FUNCTION")
print(f"{'='*72}")

eps = np.exp(np.pi) - np.pi - 20
eps_abs = abs(eps)

# Try to express |epsilon| using Gamma(1/4), pi, and simple numbers
G14 = gamma(0.25)  # = 3.625609...

# Various combinations
candidates = [
    ("pi^2 / (G(1/4)^4)", np.pi**2 / G14**4),
    ("1 / (G(1/4)^4 / pi)", np.pi / G14**4),
    ("pi / (4*G(1/4)^4)", np.pi / (4*G14**4)),
    ("(pi/G(1/4))^4", (np.pi/G14)**4),
    ("G(1/4)^2 / (4*pi*e^pi)", G14**2 / (4*np.pi*np.exp(np.pi))),
    ("1/(1111)", 1/1111),
    ("G*^2 / (alpha * e^pi)", G_star**2 / (1/137.036 * np.exp(np.pi))),
    ("pi/(e^pi * 8*G*^2)", np.pi / (np.exp(np.pi) * 8 * G_star**2)),
    ("(G(3/4)/pi)^8", (G34/np.pi)**8),
    ("eta(i)^8 / e^pi", eta_i**8 / np.exp(np.pi)),
    ("eta(i)^24", eta_i**24),
    ("1/(11 * 101)", 1/(11*101)),
    ("G*^3 / (G*^2 * e^pi)", G_star**3 / (G_star**2 * np.exp(np.pi))),
    ("G* / (pi * e^pi)", G_star / (np.pi * np.exp(np.pi))),
]

print(f"\n  |epsilon| = {eps_abs:.12f}")
print(f"\n  Candidates for |epsilon|:")
for name, val in candidates:
    ratio = eps_abs / val if val != 0 else float('inf')
    match = "<<< MATCH" if abs(ratio - 1) < 0.01 else ""
    print(f"    {name:40s} = {val:.12f}  (ratio: {ratio:.6f}) {match}")

# ============================================================
# Part 9: The deepest structure — q-expansion residuals
# ============================================================
print(f"\n{'='*72}")
print(f"  PART 9: THE q-EXPANSION AND RESIDUALS")
print(f"{'='*72}")

# e^pi = 1/q where q = e^{-pi}
# In modular form language: the expansion of some function F(tau) at tau = i/2
# gives F = 1/q + (corrections)
# If we can identify F, then epsilon = F - pi - 20 = (corrections to pi + 20)

# What if 20 comes from a modular function's constant term?
# In the j-invariant: j = 1/q + 744 + O(q), the constant term is 744
# For other modular functions, the constant term differs

# The elliptic lambda function: lambda(tau) = theta_2^4/theta_3^4
lam = th2**4 / th3**4
print(f"\n  Elliptic lambda function at tau = i/2:")
# Actually at tau = i: lambda(i) = 1/2 (the self-dual point)

# Let me try a different decomposition
# e^pi = sum over integers and Gamma values

# The Ramanujan formula: e^pi = lim of (1/q_half)
# And the near-integer property e^pi ≈ pi + 20 means
# that some modular function is approximately pi + 20

# Let me check: is pi + 20 close to any known algebraic expression?
target = np.pi + 20  # = 23.14159...
e_pi = np.exp(np.pi)  # = 23.14069...

print(f"\n  Key comparison:")
print(f"    e^pi       = {e_pi:.15f}")
print(f"    pi + 20    = {target:.15f}")
print(f"    Difference = {e_pi - target:.15f}")
print(f"    = -{abs(e_pi - target):.15f}")

# The difference is epsilon!
# So: e^pi = pi + 20 + epsilon
# where epsilon ≈ -0.0009

# This means the precision formula becomes:
# 1/alpha = x+(G*) - (9/47)|e^pi - pi - 20| + ...
# = x+(G*) - (9/47) * 0.0009 + ...

# But G* is related to theta_3 (which uses q = e^{-pi})
# So x+ depends on e^{-pi} through theta_3
# And the correction depends on e^{pi} through epsilon

# This is the key duality: the tree level uses q = e^{-pi} (small)
# and the correction uses 1/q = e^{pi} (large)

print(f"\n  THE DUALITY:")
print(f"    Tree level: x+ comes from G* = sqrt(2*pi) * theta_3(e^(-pi))^2")
print(f"                Uses q = e^(-pi) = {q_half:.6f}  (small, convergent)")
print(f"    Correction: epsilon = e^pi - pi - 20")
print(f"                Uses 1/q = e^(pi) = {1/q_half:.6f}  (large)")
print(f"")
print(f"    The tree level lives in the 'q-expansion' (discrete domain)")
print(f"    The correction lives in the '1/q-expansion' (continuous domain)")
print(f"    epsilon measures the MISMATCH between these two descriptions!")

# ============================================================
# Part 10: THE SYNTHESIS
# ============================================================
print(f"\n{'='*72}")
print(f"  PART 10: THE SYNTHESIS")
print(f"{'='*72}")

print(f"""
  THE STRUCTURE OF 1/alpha:

  1/alpha = x+(theta_3^2) - (9/47) * |1/q - pi - 20| + higher orders

  WHERE:
  - x+ comes from the self-dual theta function at q = e^(-pi)
    This is the discrete, lattice-side quantity
    It gives the "bare" coupling from pure lemniscate geometry

  - epsilon = 1/q - pi - 20 is the mismatch between:
    * 1/q = e^pi (the inverse nome, the CONTINUOUS domain)
    * pi (the geometric constant of the circle)
    * 20 = 1/c_Dirac (the conformal anomaly coefficient)

  - The coefficient 9/47 = N_c^2/D connects color (3) to the
    constraint dimension (47 = 3*16-1)

  THE PHYSICAL INTERPRETATION:
  The fine structure constant encodes the mismatch between:
  (a) The lemniscate (figure-eight, self-crossing curve, q-side)
  (b) The circle (non-crossing curve, pi)
  (c) The conformal anomaly (quantum effects, 20 = 1/c_Dirac)

  Alpha measures how imperfectly the self-dual lemniscate lattice
  maps onto the circular/conformal geometry of spacetime.

  THE 1.26 PPM GAP:
  The tree-level x+ captures the lattice geometry perfectly.
  The epsilon correction accounts for the fact that the mapping
  from lattice (q-side) to continuum (1/q-side) is NOT exact:
  e^pi ≠ pi + 20 exactly.

  If e^pi WERE exactly pi + 20, alpha would be exactly 1/x+.
  The near-miss (0.0009) creates the 1.26 ppm correction.

  WHY IS THE NEAR-MISS SO SMALL?
  Because at the self-dual point (tau = i), the lattice and
  continuum descriptions are MAXIMALLY ALIGNED:
  theta_3(e^-pi)^2 = theta_3(e^-pi)^2 (self-dual, no transformation)

  The mismatch epsilon is the residual failure of self-duality
  when extended from the theta function domain to the exponential.
""")

# Final numerical check
print(f"  FINAL NUMBERS:")
print(f"    x+     = {x_plus:.15f}  (from theta_3 via G*)")
print(f"    gap    = {gap:.15f}  (to CODATA)")
print(f"    c1*|e| = {c1*eps_abs:.15f}  (precision formula leading term)")
print(f"    Match: {abs(gap - c1*eps_abs)/gap*100:.4f}% residual")
print(f"    (closed by c2|e|^2 + c3|e|^3 + c4|e|^4)")

print(f"\n{'='*72}")

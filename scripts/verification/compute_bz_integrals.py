r"""
THE BZ COMPUTATION: Lattice Integrals and the 1.26 ppm Gap
===========================================================

Computes the one-loop vacuum polarization on the FTD lattice (3D spatial
lattice, continuous time) and investigates the finite lattice correction
that connects x+ to the physical alpha.

KEY INSIGHT: x+ is NOT the bare coupling at the Planck scale.
Running from M_P to m_e would shift 1/alpha by ~11, not 0.000172.
So x+ = alpha at a scale very close to q=0, and the 1.26 ppm gap
is a FINITE lattice correction, not logarithmic running.
"""

import numpy as np
from scipy.special import gamma
from scipy import integrate

print("=" * 72)
print("  THE BZ COMPUTATION")
print("  Lattice integrals and the 1.26 ppm gap")
print("=" * 72)

# ============================================================
# Part 0: Framework constants
# ============================================================
G_star = np.sqrt(2) * gamma(0.25)**2 / (2 * np.pi)
b = -16 * G_star**2
c = 16 * G_star**3
disc = b**2 - 4*c
x_plus = (-b + np.sqrt(disc))/2
alpha = 1.0/x_plus
alpha_CODATA = 1.0/137.035999177

gap = x_plus - 137.035999177  # = 0.000172...
gap_ppm = gap / 137.036 * 1e6

eps = np.exp(np.pi) - np.pi - 20
eps_abs = abs(eps)

# Precision formula coefficients
c1 = 9.0/47
c2 = 5.0/64
c3 = 4.0/141
c4 = 141.0/11

print(f"\n--- Part 0: The gap ---")
print(f"  x+          = {x_plus:.12f}")
print(f"  1/alpha_exp = 137.035999177000")
print(f"  Gap         = {gap:.10f}")
print(f"  Gap (ppm)   = {gap_ppm:.2f}")
print(f"  epsilon     = {eps:.12f}")
print(f"  |epsilon|   = {eps_abs:.12f}")
print(f"  c1*|eps|    = {c1*eps_abs:.10f}")
print(f"  Match: gap ~ c1*|eps|? {abs(gap - c1*eps_abs) < 1e-6}")

# ============================================================
# Part 1: Verify precision formula
# ============================================================
print(f"\n--- Part 1: Precision formula verification ---")

alpha_inv_tree = x_plus
alpha_inv_1 = x_plus - c1*eps_abs
alpha_inv_2 = alpha_inv_1 + c2*eps_abs**2
alpha_inv_3 = alpha_inv_2 - c3*eps_abs**3
alpha_inv_4 = alpha_inv_3 - c4*eps_abs**4

print(f"  Tree:   {alpha_inv_tree:.15f}  (gap: {abs(alpha_inv_tree-137.035999177)/137.036*1e6:.2f} ppm)")
print(f"  1-term: {alpha_inv_1:.15f}  (gap: {abs(alpha_inv_1-137.035999177)/137.036*1e12:.3f} ppt)")
print(f"  2-term: {alpha_inv_2:.15f}  (gap: {abs(alpha_inv_2-137.035999177)/137.036*1e12:.3f} ppt)")
print(f"  3-term: {alpha_inv_3:.15f}  (gap: {abs(alpha_inv_3-137.035999177)/137.036*1e12:.3f} ppt)")
print(f"  4-term: {alpha_inv_4:.15f}  (gap: {abs(alpha_inv_4-137.035999177)/137.036*1e12:.6f} ppt)")
print(f"  CODATA: 137.035999177000000")

# ============================================================
# Part 2: The matching scale question
# ============================================================
print(f"\n--- Part 2: Where is x+ defined? ---")

# If x+ were at the Planck scale, one-loop running gives:
# Delta(1/alpha) = -(2/3pi) * ln(M_P^2/m_e^2)
m_e_planck = 0.511e-3 / 1.221e19  # m_e in Planck units
Delta_one_loop = -(2/(3*np.pi)) * np.log(np.pi**2 / m_e_planck**2)
print(f"  m_e in Planck units = {m_e_planck:.3e}")
print(f"  If x+ were at Planck scale:")
print(f"    One-loop running Delta(1/alpha) = {Delta_one_loop:.2f}")
print(f"    This would give 1/alpha_phys = {x_plus + Delta_one_loop:.2f}")
print(f"    COMPLETELY WRONG (should be ~137)")
print(f"")
print(f"  => x+ is NOT at the Planck scale.")
print(f"  => x+ is essentially the physical coupling with a tiny correction.")

# At what scale mu0 would the 1-loop running explain the gap?
# gap = (2/3pi) * ln(mu0^2/m_e^2)
# ln(mu0^2/m_e^2) = gap * 3*pi/2
ln_ratio = gap * 3*np.pi/2
mu0_over_me = np.exp(ln_ratio/2)
print(f"\n  If gap came from one-loop running:")
print(f"    ln(mu0^2/m_e^2) = {ln_ratio:.6f}")
print(f"    mu0/m_e = {mu0_over_me:.8f}")
print(f"    mu0 = m_e * {mu0_over_me:.8f} = m_e + {(mu0_over_me-1)*0.511*1e3:.2f} eV")
print(f"    The matching scale is essentially AT the electron mass!")

# ============================================================
# Part 3: One-loop lattice integrals (3D BZ)
# ============================================================
print(f"\n--- Part 3: One-loop lattice integrals on BZ^3 ---")

# The lattice tadpole integral: I = integral over BZ^3 of 1/omega(k)
# where omega(k) = sqrt(sum_i sin^2(k_i) + m^2)
# This is the basic building block of all loop corrections.

N = 100  # Grid points per dimension

def compute_lattice_integrals(N, m_lat):
    """Compute key lattice integrals on BZ^3 = [-pi, pi]^3"""
    k = np.linspace(-np.pi, np.pi, N, endpoint=False)
    dk = k[1] - k[0]

    I_tadpole = 0.0      # int 1/omega
    I_bubble = 0.0       # int 1/omega^2 (vacuum polarization related)
    I_sunset = 0.0       # int 1/omega^3
    I_sin2_omega3 = 0.0  # int sin^2(k_x) / omega^3
    
    for kx in k:
        for ky in k:
            for kz in k:
                sin2 = np.sin(kx)**2 + np.sin(ky)**2 + np.sin(kz)**2
                omega = np.sqrt(sin2 + m_lat**2)
                I_tadpole += 1.0 / omega
                I_bubble += 1.0 / omega**2
                I_sunset += 1.0 / omega**3
                I_sin2_omega3 += np.sin(kx)**2 / omega**3
    
    vol = N**3
    return (I_tadpole/vol, I_bubble/vol, I_sunset/vol, I_sin2_omega3/vol)

# Compute with m=0 (massless limit) and small m
print(f"  Computing lattice integrals (N={N})...")

# Massless limit (m -> 0, use small m for regularization)
m_reg = 0.01  # small regulator
I_tad, I_bub, I_sun, I_s2o3 = compute_lattice_integrals(N, m_reg)

print(f"\n  Lattice integrals (m_reg = {m_reg}):")
print(f"    I_tadpole = int 1/omega        = {I_tad:.8f}")
print(f"    I_bubble  = int 1/omega^2      = {I_bub:.8f}")
print(f"    I_sunset  = int 1/omega^3      = {I_sun:.8f}")
print(f"    I_sin2    = int sin^2(kx)/omega^3 = {I_s2o3:.8f}")

# The one-loop vacuum polarization at q=0 on the 3D lattice
# After k0 integration: Pi(0) = (alpha/3) * int_BZ3 d3k/(2pi)^3 * [stuff/omega^3]
# Specifically: Pi_ii(0) ~ alpha * I_vp where I_vp is a combination of the above

# For the spatial vacuum polarization (transverse part):
# Pi_T(0) = (2*alpha/3) * (3*I_sin2/omega^3 - I_bubble)
# This gives the finite lattice correction to the charge

# The vacuum polarization contribution to the charge renormalization:
# 1/alpha_phys = 1/alpha_bare * (1 + Pi_vp)
# where Pi_vp is the one-loop vacuum polarization

# In lattice QED, the finite one-loop correction is:
# delta(1/alpha) = (1/3pi) * Sigma_1
# where Sigma_1 is a lattice constant
Sigma_1 = 3 * np.pi * (I_bub - 3*I_s2o3)  # lattice correction constant
delta_1loop = Sigma_1 / (3 * np.pi)

print(f"\n  Lattice correction constant Sigma_1 = {Sigma_1:.8f}")
print(f"  One-loop lattice shift delta(1/alpha) = {delta_1loop:.8f}")
print(f"  Gap to close: {gap:.8f}")
print(f"  Ratio: delta/gap = {delta_1loop/gap:.4f}")

# ============================================================
# Part 4: The continuum integral for comparison
# ============================================================
print(f"\n--- Part 4: Continuum vs lattice comparison ---")

# Continuum 3D integral: int d3k/(2pi)^3 * 1/(k^2 + m^2) = 1/(4*pi*m)
I_tad_cont = 1.0/(4*np.pi*m_reg)
print(f"  Continuum I_tadpole = 1/(4*pi*m) = {I_tad_cont:.8f}")
print(f"  Lattice I_tadpole   = {I_tad:.8f}")
print(f"  Difference (lattice artifact) = {I_tad - I_tad_cont:.8f}")

# ============================================================
# Part 5: The deep question — is epsilon a lattice constant?
# ============================================================
print(f"\n--- Part 5: Is epsilon a lattice constant? ---")

# epsilon = e^pi - pi - 20 = 23.14069... - 3.14159... - 20 = 0.000900...
# The question: does any combination of lattice integrals equal epsilon?

# Key lattice constants:
# Watson's integrals for the 3D cubic lattice
# W_s = (1/pi^3) * int_0^pi int_0^pi int_0^pi dk1 dk2 dk3 / 
#        (3 - cos k1 - cos k2 - cos k3) = 0.505462...

def watson_integral(N=200):
    """Compute Watson's integral for the simple cubic lattice"""
    k = np.linspace(0, np.pi, N, endpoint=False)
    dk = k[1] - k[0]
    total = 0.0
    for k1 in k:
        for k2 in k:
            for k3 in k:
                denom = 3 - np.cos(k1) - np.cos(k2) - np.cos(k3)
                if denom > 1e-10:
                    total += 1.0/denom
    return total * (dk/np.pi)**3

print("  Computing Watson's integral...")
W_s = watson_integral(100)
print(f"  Watson W_s = {W_s:.8f}  (known: 0.505462...)")
print(f"  1/(6*W_s) = {1/(6*W_s):.8f}")
print(f"  |epsilon| = {eps_abs:.8f}")
print(f"  Ratio |eps|/W_s = {eps_abs/W_s:.8f}")

# The zero-point energy integral
def bz_zero_point():
    """Compute <omega> on BZ^3"""
    N = 100
    k = np.linspace(-np.pi, np.pi, N, endpoint=False)
    total = 0.0
    for kx in k:
        for ky in k:
            for kz in k:
                omega = 2*np.sqrt(np.sin(kx/2)**2 + np.sin(ky/2)**2 + np.sin(kz/2)**2)
                total += omega
    return total / N**3

E0 = bz_zero_point()
print(f"\n  BZ zero-point energy <omega> = {E0:.8f}")
print(f"  E0 - 2 = {E0 - 2:.8f}")
print(f"  Compare |epsilon| = {eps_abs:.8f}")

# Various lattice constants and their relationship to epsilon
print(f"\n  Exploring connections to epsilon = {eps:.12f}")
print(f"  e^pi = {np.exp(np.pi):.12f}")
print(f"  pi   = {np.pi:.12f}")
print(f"  e^pi - pi = {np.exp(np.pi) - np.pi:.12f}")
print(f"  e^pi - pi - 20 = {eps:.12f}")
print(f"  23 + e^pi - pi - 23 = {eps:.12f}")
print(f"  (e^pi - 23) = {np.exp(np.pi) - 23:.12f}")
print(f"  (e^pi - 23) - pi + 3 = {np.exp(np.pi) - 23 - np.pi + 3:.12f}")

# The lemniscate nome
q_lem = np.exp(-np.pi)
print(f"\n  Lemniscate nome q = e^(-pi) = {q_lem:.12f}")
print(f"  1/q = e^pi = {1/q_lem:.12f}")
print(f"  1/q - pi = {1/q_lem - np.pi:.12f}")
print(f"  1/q - pi - 20 = epsilon = {1/q_lem - np.pi - 20:.12f}")

# Check: is epsilon connected to j-invariant?
# j(i) = 1728 (for the lemniscate lattice tau = i)
# j(i) - 12^3 = 0
# The Ramanujan-type constant e^(pi*sqrt(163)) ≈ integer
print(f"\n  Modular connections:")
print(f"    j(i) = 1728 = 12^3")
print(f"    e^pi = 23.1406926... ≈ 23 + 1/7 + delta")
print(f"    23 = 3 + 4*5 = N_c + N_base * 5")
print(f"    1/|eps| ≈ {1/eps_abs:.4f} ≈ 1111")
print(f"    1111 = 11 * 101 = (b3+Nbase)(8*Neff-Nc)")

# ============================================================
# Part 6: The critical comparison
# ============================================================
print(f"\n{'='*72}")
print(f"  CRITICAL COMPARISON")
print(f"{'='*72}")

# What we know:
# 1. The gap is 0.000172281 = c1 * |eps| = (9/47) * |e^pi - pi - 20|
# 2. This is NOT from QED running (wrong magnitude by 5 orders)
# 3. The lattice integrals give finite corrections of order ~0.01-1

# The gap expressed in terms of the master quadratic:
print(f"""
  THE GAP: x+ - 1/alpha_CODATA = {gap:.10f}

  PRECISION FORMULA DECOMPOSITION:
    c1*|eps|   = {c1*eps_abs:.10f}  (accounts for 99.97% of gap)
    c2*|eps|^2 = {c2*eps_abs**2:.10f}  (refines to sub-ppt)
    
  WHERE epsilon COMES FROM:
    eps = e^pi - pi - 20 = (1/q_lem) - pi - (b3 + Neff)
    
  QFT INTERPRETATION:
    At q=0, the lattice coupling differs from the continuum by
    a FINITE constant (not a log). This finite constant involves
    integrals over the Brillouin zone that depend on the lattice
    geometry — specifically the lemniscatic geometry.
    
  THE CONNECTION (still [CONJECTURE]):
    If the lattice defines alpha at the matching scale via the
    master quadratic, then the finite lattice correction from
    BZ integrals should equal c1*|eps| = {c1*eps_abs:.6e}.
    
    The one-loop lattice correction computed here is: {delta_1loop:.6e}
    Ratio to gap: {delta_1loop/gap:.4f}
    
  STATUS: The one-loop correction is order-of-magnitude comparable
    but not an exact match. The full calculation requires:
    1. Proper treatment of the fermion propagator (Wilson vs naive)
    2. The exact matching condition for x+ 
    3. Possibly the two-loop contribution as well
""")

print("=" * 72)

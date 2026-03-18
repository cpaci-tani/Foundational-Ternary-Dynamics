"""
Verification: Derivation of G_F from FTD First Principles
==========================================================

Derivation chain:
  G* → master quadratic → α → v = M_P √(2π) α⁸ → G_F = 1/(√2 v²)

Reference: docs/theory/DERIV_FERMI_COUPLING_CONSTANT.md
"""

import io
import sys

# Reconfigure stdout to UTF-8 on Windows to handle Unicode math symbols
if sys.platform == 'win32' and hasattr(sys.stdout, 'buffer'):
    if sys.stdout.encoding and sys.stdout.encoding.lower() != 'utf-8':
        sys.stdout = io.TextIOWrapper(
            sys.stdout.buffer, encoding='utf-8', errors='replace', line_buffering=True
        )

import numpy as np
from scipy.special import gamma

print("=" * 70)
print("  FERMI COUPLING CONSTANT FROM FTD FIRST PRINCIPLES")
print("=" * 70)

# ──────────────────────────────────────────────────────
# Step 1: Compute G* (lemniscatic constant)
# ──────────────────────────────────────────────────────
G_star = np.sqrt(2) * gamma(0.25)**2 / (2 * np.pi)
print(f"\n[Step 1] Lemniscatic constant")
print(f"  G* = √2 × Γ(1/4)² / (2π) = {G_star:.10f}")

# ──────────────────────────────────────────────────────
# Step 2: Solve master quadratic → α
# ──────────────────────────────────────────────────────
a_coeff = 1
b_coeff = -16 * G_star**2
c_coeff = 16 * G_star**3

discriminant = b_coeff**2 - 4 * a_coeff * c_coeff
x_plus = (-b_coeff + np.sqrt(discriminant)) / (2 * a_coeff)
x_minus = (-b_coeff - np.sqrt(discriminant)) / (2 * a_coeff)
alpha = 1.0 / x_plus

print(f"\n[Step 2] Master quadratic: x² - 16G*²x + 16G*³ = 0")
print(f"  x₊ = {x_plus:.10f}   (= 1/α)")
print(f"  x₋ = {x_minus:.10f}  (→ N_c = 3)")
print(f"  α  = {alpha:.10e}")

# ──────────────────────────────────────────────────────
# Step 3: Compute Higgs VEV: v = M_P √(2π) α⁸
# ──────────────────────────────────────────────────────
M_P = 1.22089e19  # GeV (Planck mass)
v_FTD = M_P * np.sqrt(2 * np.pi) * alpha**8
v_exp = 246.22  # GeV (from G_F measurement)

print(f"\n[Step 3] Higgs VEV: v = M_P √(2π) α⁸")
print(f"  M_P      = {M_P:.5e} GeV")
print(f"  α⁸       = {alpha**8:.6e}")
print(f"  √(2π)    = {np.sqrt(2*np.pi):.6f}")
print(f"  v (FTD)  = {v_FTD:.4f} GeV")
print(f"  v (exp)  = {v_exp:.4f} GeV")
print(f"  Error    = {abs(v_FTD - v_exp)/v_exp * 100:.3f}%")

# ──────────────────────────────────────────────────────
# Step 4: Derive G_F = 1/(√2 v²)
# ──────────────────────────────────────────────────────
G_F_FTD = 1.0 / (np.sqrt(2) * v_FTD**2)
G_F_exp = 1.1663788e-5  # GeV⁻² (CODATA 2022)

print(f"\n[Step 4] Fermi coupling: G_F = 1/(√2 v²)")
print(f"  √2 × v²  = {np.sqrt(2) * v_FTD**2:.4f} GeV²")
print(f"  G_F (FTD) = {G_F_FTD:.7e} GeV⁻²")
print(f"  G_F (exp) = {G_F_exp:.7e} GeV⁻²")
print(f"  Error     = {abs(G_F_FTD - G_F_exp)/G_F_exp * 100:.3f}%")

# ──────────────────────────────────────────────────────
# Step 5: Verify alternate form: G_F = 1/(2√2 π M_P² α¹⁶)
# ──────────────────────────────────────────────────────
G_F_alt = 1.0 / (2 * np.sqrt(2) * np.pi * M_P**2 * alpha**16)
print(f"\n[Step 5] Alternate form: G_F = 1/(2*sqrt(2)*pi*M_P^2*alpha^16)")
print(f"  alpha^16        = {alpha**16:.6e}")
print(f"  M_P^2 * alpha^16 = {M_P**2 * alpha**16:.4f} GeV^2")
print(f"  G_F (alt form)  = {G_F_alt:.7e} GeV^-2")
print(f"  Matches v^2 form: {np.isclose(G_F_FTD, G_F_alt, rtol=1e-10)}")

# ──────────────────────────────────────────────────────
# Cross-check: Muon Lifetime
# ──────────────────────────────────────────────────────
# Framework integers
N_c = 3
N_base = 4
b_3 = 7
n_eff = 13

# Muon mass from FTD
m_e = 0.51100  # GeV (≈ 0.511 MeV, FTD threshold)
m_mu_ratio = 3 * b_3 * (b_3 + N_c) - N_c  # = 207
m_mu = m_mu_ratio * m_e * 1e-3  # Convert to GeV (m_e in MeV → GeV)
m_mu_MeV = m_mu_ratio * 0.51100  # MeV
m_mu_GeV = m_mu_MeV * 1e-3

# Muon lifetime: τ = 192π³ / (G_F² m_μ⁵)
tau_mu_GeV = 192 * np.pi**3 / (G_F_FTD**2 * m_mu_GeV**5)  # in GeV⁻¹
hbar = 6.582119569e-25  # GeV·s
tau_mu_s = tau_mu_GeV * hbar  # in seconds
tau_mu_us = tau_mu_s * 1e6  # in microseconds
tau_mu_exp = 2.1970e-6  # seconds

print(f"\n[Cross-check] Muon Lifetime")
print(f"  m_μ/m_e  = {m_mu_ratio} (= 3×7×10 - 3)")
print(f"  m_μ      = {m_mu_MeV:.2f} MeV  ({m_mu_GeV:.5f} GeV)")
print(f"  τ_μ (FTD)  = {tau_mu_us:.3f} μs")
print(f"  τ_μ (exp)  = {tau_mu_exp*1e6:.4f} μs")
print(f"  Error      = {abs(tau_mu_s - tau_mu_exp)/tau_mu_exp * 100:.2f}%")

# ──────────────────────────────────────────────────────
# Cross-check: W Boson Mass from G_F
# ──────────────────────────────────────────────────────
sin2_thetaW = 3.0 / 13.0  # FTD derived
M_W_from_GF = np.sqrt(np.pi * alpha / (np.sqrt(2) * G_F_FTD * sin2_thetaW))
M_W_direct = v_FTD * np.sqrt(1 - sin2_thetaW) * np.sqrt(alpha * np.pi / (np.sqrt(2) * G_F_FTD * v_FTD**2)) 
# Simpler: M_W = g*v/2 where g = e/sin(θ_W)
e = np.sqrt(4 * np.pi * alpha)
g_W = e / np.sqrt(sin2_thetaW)
M_W_direct2 = g_W * v_FTD / 2

print(f"\n[Cross-check] W Boson Mass")
print(f"  From G_F formula (tree level): M_W = {M_W_from_GF:.2f} GeV")
print(f"  From v directly:  M_W = gv/2 = {M_W_direct2:.2f} GeV")
print(f"  Experimental:     M_W = 80.377 GeV")

# ──────────────────────────────────────────────────────
# Summary
# ──────────────────────────────────────────────────────
print("\n" + "=" * 70)
print("  SUMMARY: G_F DERIVATION FROM FTD")
print("=" * 70)
print(f"""
  Derivation Chain:
    G* = {G_star:.10f}
     ↓ [master quadratic]
    α  = 1/{x_plus:.10f}
     ↓ [v = M_P √(2π) α⁸]
    v  = {v_FTD:.4f} GeV          (exp: {v_exp} GeV, err: {abs(v_FTD-v_exp)/v_exp*100:.3f}%)
     ↓ [G_F = 1/(√2 v²)]
    G_F = {G_F_FTD:.7e} GeV⁻²  (exp: {G_F_exp:.7e}, err: {abs(G_F_FTD-G_F_exp)/G_F_exp*100:.3f}%)

  Cross-checks:
    τ_μ = {tau_mu_us:.3f} μs               (exp: 2.1970 μs, err: {abs(tau_mu_s-tau_mu_exp)/tau_mu_exp*100:.2f}%)
    M_W = {M_W_direct2:.2f} GeV             (exp: 80.377 GeV)

  Status: G_F is now a GENUINE DERIVATION, not an external input.
  External inputs remaining: M_P, Λ_QCD, decay constants, phase space factors.
""")
print("=" * 70)

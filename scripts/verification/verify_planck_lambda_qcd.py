"""
Verification: M_P Self-Consistency and Lambda_QCD Derivation
=============================================================

Chain 1: M_P as sole axiom - verify all ratios are integer-determined
Chain 2: Lambda_QCD from dimensional transmutation with FTD inputs

Reference: docs/theory/DERIV_PLANCK_MASS_AND_LAMBDA_QCD.md
"""

import numpy as np
from scipy.special import gamma

print("=" * 70)
print("  M_P AND LAMBDA_QCD FROM FTD FIRST PRINCIPLES")
print("=" * 70)

# ============================================================
# Constants from FTD
# ============================================================
G_star = np.sqrt(2) * gamma(0.25)**2 / (2 * np.pi)

# Master quadratic
a_coeff = 1
b_coeff = -16 * G_star**2
c_coeff = 16 * G_star**3
discriminant = b_coeff**2 - 4 * a_coeff * c_coeff
x_plus = (-b_coeff + np.sqrt(discriminant)) / (2 * a_coeff)
alpha = 1.0 / x_plus

# Framework integers
N_c = 3
N_base = 4
b_3 = 7
n_eff = 13

print(f"\nFTD Constants:")
print(f"  G* = {G_star:.10f}")
print(f"  alpha = 1/{x_plus:.10f} = {alpha:.10e}")
print(f"  Integers: N_c={N_c}, N_base={N_base}, b_3={b_3}, n_eff={n_eff}")

# ============================================================
# PART 1: M_P SELF-CONSISTENCY
# ============================================================
print("\n" + "=" * 70)
print("  PART 1: PLANCK MASS SELF-CONSISTENCY")
print("=" * 70)

M_P = 1.22089e19  # GeV (Planck mass - the single axiom)

# Electron mass from M_P
m_e_formula = M_P * np.sqrt(2 * np.pi) * (16.0 / 3.0) * alpha**11
m_e_exp = 0.51100e-3  # GeV

print(f"\n[1] Electron mass: m_e = M_P * sqrt(2pi) * (16/3) * alpha^11")
print(f"  M_P = {M_P:.5e} GeV")
print(f"  sqrt(2pi) * (16/3) * alpha^11 = {np.sqrt(2*np.pi) * (16/3) * alpha**11:.6e}")
print(f"  m_e (FTD)  = {m_e_formula*1e3:.4f} MeV")
print(f"  m_e (exp)  = {m_e_exp*1e3:.4f} MeV")
print(f"  Error      = {abs(m_e_formula - m_e_exp)/m_e_exp * 100:.3f}%")

# Verify inverse: M_P from m_e
M_P_from_me = m_e_exp / (np.sqrt(2 * np.pi) * (16.0 / 3.0) * alpha**11)
print(f"\n[2] Inverse: M_P = m_e / (sqrt(2pi) * (16/3) * alpha^11)")
print(f"  M_P (computed) = {M_P_from_me:.5e} GeV")
print(f"  M_P (axiom)    = {M_P:.5e} GeV")
print(f"  Consistent: {np.isclose(M_P_from_me, M_P, rtol=0.003)}")

# Verify mass ratios (independent of M_P)
m_mu_ratio = 3 * b_3 * (b_3 + N_c) - N_c  # = 207
m_tau_ratio = (n_eff + N_base) * m_mu_ratio - 2 * N_c * b_3  # = 3477
T_10 = 10 * 11 // 2  # = 55
m_p_ratio = n_eff / alpha + T_10  # = 1836.47

print(f"\n[3] Mass ratios (M_P-independent, pure integers + alpha):")
print(f"  m_mu/m_e  = {m_mu_ratio}     (exp: 206.768, err: {abs(m_mu_ratio-206.768)/206.768*100:.2f}%)")
print(f"  m_tau/m_e = {m_tau_ratio}    (exp: 3477.23, err: {abs(m_tau_ratio-3477.23)/3477.23*100:.3f}%)")
print(f"  m_p/m_e   = {m_p_ratio:.2f}  (exp: 1836.15, err: {abs(m_p_ratio-1836.15)/1836.15*100:.3f}%)")

# Verify VEV
v_FTD = M_P * np.sqrt(2 * np.pi) * alpha**8
v_exp = 246.22  # GeV
print(f"\n[4] Higgs VEV: v = M_P * sqrt(2pi) * alpha^8")
print(f"  v (FTD) = {v_FTD:.4f} GeV  (exp: {v_exp}, err: {abs(v_FTD-v_exp)/v_exp*100:.3f}%)")

# Verify G_F
G_F_FTD = 1.0 / (np.sqrt(2) * v_FTD**2)
G_F_exp = 1.1663788e-5
print(f"\n[5] Fermi coupling: G_F = 1/(sqrt(2) v^2)")
print(f"  G_F (FTD) = {G_F_FTD:.7e} GeV^-2  (exp: {G_F_exp:.7e}, err: {abs(G_F_FTD-G_F_exp)/G_F_exp*100:.3f}%)")

# ============================================================
# PART 2: LAMBDA_QCD
# ============================================================
print("\n" + "=" * 70)
print("  PART 2: LAMBDA_QCD FROM DIMENSIONAL TRANSMUTATION")
print("=" * 70)

# Strong coupling at M_Z
alpha_s_MZ = float(b_3) / (b_3 + 4 * n_eff)  # = 7/59
alpha_s_exp = 0.1179
print(f"\n[1] Strong coupling: alpha_s(M_Z) = b_3/(b_3 + 4*N_eff)")
print(f"  alpha_s (FTD) = {alpha_s_MZ:.5f}  (= {b_3}/{b_3 + 4*n_eff})")
print(f"  alpha_s (PDG) = {alpha_s_exp:.4f}")
print(f"  Error = {abs(alpha_s_MZ - alpha_s_exp)/alpha_s_exp * 100:.2f}%")

# Z boson mass from v and sin^2(theta_W)
sin2_thetaW = float(N_c) / n_eff  # = 3/13
e = np.sqrt(4 * np.pi * alpha)
g_W = e / np.sqrt(sin2_thetaW)
M_W_FTD = g_W * v_FTD / 2
cos_thetaW = np.sqrt(1 - sin2_thetaW)
M_Z_FTD = M_W_FTD / cos_thetaW
M_Z_exp = 91.1876

print(f"\n[2] Z boson mass from v and sin^2(theta_W)")
print(f"  sin^2(theta_W) = {sin2_thetaW:.6f}  (= {N_c}/{n_eff})")
print(f"  M_W (FTD) = {M_W_FTD:.2f} GeV")
print(f"  M_Z (FTD) = {M_Z_FTD:.2f} GeV  (exp: {M_Z_exp}, err: {abs(M_Z_FTD-M_Z_exp)/M_Z_exp*100:.2f}%)")

# One-loop Lambda_QCD
n_f_5 = 5
b0_5 = (11 * N_c - 2 * n_f_5) / 3.0  # = 23/3
exponent_1loop = -2 * np.pi / (b0_5 * alpha_s_MZ)
Lambda_1loop = M_Z_FTD * np.exp(exponent_1loop)

print(f"\n[3] One-loop Lambda_QCD^(5)")
print(f"  b_0(n_f=5) = {b0_5:.4f}  (= 23/3)")
print(f"  Exponent = -2pi / (b0 * alpha_s) = {exponent_1loop:.4f}")
print(f"  Lambda^(5) (1-loop) = {Lambda_1loop*1e3:.1f} MeV")

# Two-loop Lambda_QCD (approximate)
b1_5 = (306 - 38 * n_f_5) / 3.0  # = 116/3
# Two-loop correction factor
two_loop_factor = (b0_5 * alpha_s_MZ / (4 * np.pi))**(b1_5 / (2 * b0_5**2))
Lambda_2loop_approx = Lambda_1loop / two_loop_factor

# More precise two-loop: numerical integration of RG equation
# We use the implicit formula: ln(mu^2/Lambda^2) = 4pi/(b0*alpha_s) + (b1/(2*b0^2))*ln(b0*alpha_s/(4*pi))
# Solving for Lambda given alpha_s at M_Z

# Using the standard 2-loop formula:
# alpha_s(mu) = (4*pi)/(b0*L) * [1 - (b1*ln(L))/(b0^2*L)]
# where L = ln(mu^2/Lambda^2)
# We solve for Lambda by iterating

def alpha_s_2loop(mu, Lambda, b0, b1):
    """Two-loop alpha_s at scale mu given Lambda."""
    L = 2 * np.log(mu / Lambda)
    if L <= 0:
        return float('inf')
    return (4 * np.pi) / (b0 * L) * (1 - (b1 * np.log(L)) / (b0**2 * L))

# Binary search for Lambda that gives alpha_s = 7/59 at M_Z
Lambda_low = 0.10  # GeV
Lambda_high = 0.40  # GeV
for _ in range(100):
    Lambda_mid = (Lambda_low + Lambda_high) / 2
    alpha_test = alpha_s_2loop(M_Z_FTD, Lambda_mid, b0_5, b1_5)
    if alpha_test > alpha_s_MZ:
        Lambda_high = Lambda_mid
    else:
        Lambda_low = Lambda_mid
Lambda_2loop = (Lambda_low + Lambda_high) / 2
alpha_check = alpha_s_2loop(M_Z_FTD, Lambda_2loop, b0_5, b1_5)

Lambda_PDG = 0.213  # GeV (PDG)
Lambda_PDG_err = 0.008  # GeV

print(f"\n[4] Two-loop Lambda_QCD^(5) (numerically solved)")
print(f"  b_1(n_f=5) = {b1_5:.4f}  (= 116/3)")
print(f"  Lambda^(5) (2-loop) = {Lambda_2loop*1e3:.1f} MeV")
print(f"  Verification: alpha_s(M_Z, Lambda={Lambda_2loop*1e3:.0f} MeV) = {alpha_check:.5f} (target: {alpha_s_MZ:.5f})")
print(f"  Lambda^(5) (PDG) = {Lambda_PDG*1e3:.0f} +/- {Lambda_PDG_err*1e3:.0f} MeV")
print(f"  Error = {abs(Lambda_2loop - Lambda_PDG)/Lambda_PDG * 100:.1f}%")
print(f"  Within 1-sigma: {abs(Lambda_2loop - Lambda_PDG) < Lambda_PDG_err}")

# Derived f_pi
f_pi_FTD = Lambda_2loop / np.sqrt(N_c)
f_pi_exp = 0.1304  # GeV (PDG: f_pi = 130.4 MeV)
print(f"\n[5] Pion decay constant: f_pi = Lambda_QCD / sqrt(N_c)")
print(f"  f_pi (FTD) = {f_pi_FTD*1e3:.1f} MeV")
print(f"  f_pi (PDG) = {f_pi_exp*1e3:.1f} MeV")
print(f"  Error = {abs(f_pi_FTD - f_pi_exp)/f_pi_exp * 100:.1f}%")

# ============================================================
# SUMMARY
# ============================================================
print("\n" + "=" * 70)
print("  SUMMARY: EXTERNAL INPUT INVENTORY")
print("=" * 70)
print(f"""
  BEFORE this derivation campaign:
    External: M_P, G_F, Lambda_QCD, f_pi, f_K, phase space factors
    Free parameters: ~5

  AFTER:
    AXIOM (1):  M_P = {M_P:.3e} GeV (sets absolute scale)
    DERIVED:    G_F = {G_F_FTD:.4e} GeV^-2 (0.11% accuracy)
    DERIVED:    Lambda_QCD = {Lambda_2loop*1e3:.0f} MeV (2-loop, {abs(Lambda_2loop-Lambda_PDG)/Lambda_PDG*100:.1f}% accuracy)
    DERIVED:    f_pi = {f_pi_FTD*1e3:.0f} MeV (from Lambda_QCD/sqrt(N_c))
    REMAINING:  Phase space factors (standard kinematics, not FTD-specific)

  FTD free parameters: 0 dimensionless + 1 dimensionful (M_P)

  Key verification:
    alpha      = 1/{x_plus:.4f}  (from G*)
    alpha_s    = {alpha_s_MZ:.5f}  (from integers)
    v          = {v_FTD:.2f} GeV  (from M_P + alpha)
    G_F        = {G_F_FTD:.4e}  (from v)
    Lambda_QCD = {Lambda_2loop*1e3:.0f} MeV  (from alpha_s + M_Z)
    m_e        = {m_e_formula*1e3:.3f} MeV  (from M_P + alpha)
    m_mu/m_e   = {m_mu_ratio}  (from integers)
    m_p/m_e    = {m_p_ratio:.2f}  (from integers + alpha)
""")
print("=" * 70)

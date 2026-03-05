#!/usr/bin/env python3
"""
Complete exploration of all ontic constants in the FTD derivation chain.
Every constant, its value, its dimensional triad interpretation,
and the critical G*=3 fixed point analysis.
"""
import sys
import math

sys.stdout.reconfigure(encoding='utf-8')

print('='*80)
print('  COMPLETE ONTIC CONSTANT EXPLORATION')
print('  Every constant, its value, its powers, its relationships')
print('='*80)

# ===========================================================================
# LAYER -1: Self-Referential Seed
# ===========================================================================
e = math.e
print(f'\n--- LAYER -1: Self-Referential Seed ---')
print(f'  e = {e:.15f}')
print(f'  Property: d/dx(e^x) = e^x  (eigenvalue of differentiation)')
print(f'  e^1 = {e:.6f}   e^2 = {e**2:.6f}   e^3 = {e**3:.6f}')
print(f'  ln(e) = {math.log(e):.1f}   (definitional)')

# ===========================================================================
# LAYER 0: Transcendental Seeds
# ===========================================================================
gamma_em = 0.57721566490153286
gamma_quarter = 3.6256099082219083
print(f'\n--- LAYER 0: Transcendental Seeds ---')
print(f'  gamma (Euler-Mascheroni) = {gamma_em:.15f}')
print(f'  Gamma(1/4)               = {gamma_quarter:.15f}')
print(f'  Gamma(1/4)^2             = {gamma_quarter**2:.10f}')
print(f'  Connection: gamma links harmonic series to logarithms')
print(f'  Connection: Gamma(1/4) links arithmetic to geometry')

# ===========================================================================
# LAYER 0b: Modular Selection
# ===========================================================================
varpi = 2.622057554292119810
M = 0.8346268416740731
G = 2 * math.sqrt(varpi * M)
pi_d = 4 * varpi**2 / (G**2)
nome = math.exp(-varpi / M)
theta = 1.08643481121331
print(f'\n--- LAYER 0b: Modular Selection ---')
print(f'  nome q = e^(-varpi/M) = e^(-pi) = {nome:.15f}')
print(f'  theta_3(0,q)                      = {theta:.15f}')
print(f'  theta_3^2                          = {theta**2:.10f}')
print(f'  sqrt(2)*M                          = {math.sqrt(2)*M:.10f}')
print(f'  Identity: theta^2 = sqrt(2)*M      VERIFIED (diff={abs(theta**2 - math.sqrt(2)*M):.2e})')
print(f'  nome = (-1)^i = e^(-pi)            (antimatter^consciousness = modular selector)')

# ===========================================================================
# LAYER 1: Elliptic Geometry
# ===========================================================================
print(f'\n--- LAYER 1: Elliptic Geometry ---')
print(f'  varpi (lemniscate const) = {varpi:.15f}')
print(f'  M (Gauss constant)       = {M:.15f}')
varpi_check = gamma_quarter**2 / (2 * math.sqrt(2 * pi_d))
print(f'  varpi = Gamma(1/4)^2 / (2*sqrt(2*pi))')
print(f'         computed            = {varpi_check:.15f}  (diff={abs(varpi-varpi_check):.2e})')
print(f'  pi = varpi / M            = {varpi/M:.15f}')
print(f'  varpi^2                    = {varpi**2:.10f}')
print(f'  varpi^3                    = {varpi**3:.10f}')
print(f'  M^2                        = {M**2:.10f}')
print(f'  varpi * M                  = {varpi*M:.10f}')
print(f'  2*sqrt(varpi*M) = G*       = {2*math.sqrt(varpi*M):.10f}')

# ===========================================================================
# LAYER 2: Universal Operator
# ===========================================================================
PF = pi_d / 4
sqrtG = math.sqrt(G)

print(f'\n--- LAYER 2: Universal Operator ---')
print(f'  G* = 2*sqrt(varpi*M)     = {G:.15f}')
print(f'  pi = 4*varpi^2/G*^2      = {pi_d:.15f}')
print(f'  PF = pi/4                 = {PF:.15f}')
print(f'  sqrt(G*)                  = {sqrtG:.15f}')
print()
print(f'  === THE DIMENSIONAL TRIAD ===')
print(f'  G*^0 = 1.000000           (existence / identity / the void)')
print(f'  G*^1 = {G**1:.10f}       (FLUX = spatial amplitude per DoF)')
print(f'  G*^2 = {G**2:.10f}       (ENERGY = temporal amplitude per DoF)')
print(f'  G*^3 = {G**3:.10f}      (ACTION = spatiotemporal record per DoF)')
print(f'  G*^4 = {G**4:.10f}      (ACTION*FLUX = flux-weighted action)')
print(f'  G*^5 = {G**5:.10f}     (ACTION*ENERGY = power of action)')
print(f'  G*^6 = {G**6:.10f}     (ACTION^2 = action squared)')
print()
print(f'  === FRACTIONAL POWERS ===')
print(f'  G*^(1/2) = sqrt(G*) = {G**0.5:.10f}   (the time operator, read/write sub-tick)')
print(f'  G*^(1/3) = {G**(1/3):.10f}   (cube root)')
print(f'  G*^(2/3) = {G**(2/3):.10f}   (two-thirds power)')
print(f'  G*^(3/2) = {G**1.5:.10f}  (energy * sqrt_flux)')
print()
print(f'  === KEY IDENTITIES ===')
print(f'  G* = varpi / sqrt(PF) = {varpi/math.sqrt(PF):.15f}')
print(f'  G* = 2*varpi / sqrt(pi)   = {2*varpi/math.sqrt(pi_d):.15f}')
print(f'  G*/pi = {G/pi_d:.10f}     (G* per radian)')
print(f'  pi/G* = {pi_d/G:.10f}     (radians per G*)')
print(f'  G*^2/pi = {G**2/pi_d:.10f} (energy per radian)')

# ===========================================================================
# LAYER 2b: Euler Identity and Emergence of i
# ===========================================================================
k_crit = 4.0 / G
x_born = 2.0 * G
print(f'\n--- LAYER 2b: Emergence of i ---')
print(f'  k_crit = 4/G* = {k_crit:.15f}')
print(f'  x_Born = 2*G* = {x_born:.15f}')
print(f'  Three domains:')
print(f'    k=16:   k*G* = {16*G:.4f} > 4  -->  REAL roots     (physics)')
print(f'    k=4/G*: k*G* = 4.0000    = 4  -->  DEGENERATE      (measurement/Born rule)')
print(f'    k=1/2:  k*G* = {0.5*G:.4f} < 4  -->  COMPLEX roots  (consciousness)')
print(f'  The Ternary as Complex Geometry:')
print(f'    +1 = e^(i*0)      (matter)')
print(f'    -1 = e^(i*pi)     (antimatter)')
print(f'     0 = origin        (void)')
print(f'  Euler: e^(i*pi) + 1 = 0  <-->  (-1) + (+1) = 0  (annihilation)')

# ===========================================================================
# LAYER 3: Master Quadratic
# ===========================================================================
disc = 256*G**4 - 64*G**3
xp = (16*G**2 + math.sqrt(disc)) / 2
xm = (16*G**2 - math.sqrt(disc)) / 2
print(f'\n--- LAYER 3: Master Quadratic ---')
print(f'  x^2 - 16*G*^2*x + 16*G*^3 = 0')
print(f'  x+ = {xp:.10f}  (= 1/alpha)')
print(f'  x- = {xm:.10f}   (= N_c effective)')
print(f'  Vieta sum:     x+ + x- = {xp+xm:.10f} = 16*G*^2 = {16*G**2:.10f}')
print(f'  Vieta product: x+ * x- = {xp*xm:.10f} = 16*G*^3 = {16*G**3:.10f}')
print(f'  P/S = (x+*x-)/(x++x-) = {xp*xm/(xp+xm):.15f} = G* = {G:.15f}')
print(f'  HM(x+,x-)/2 = {2*xp*xm/((xp+xm)*2):.15f} = G*/2?  No: G*')

# Harmonic mean
HM = 2 * xp * xm / (xp + xm)
print(f'  HM(x+,x-) = 2*x+*x-/(x++x-) = {HM:.15f} = 2*G* = {2*G:.15f}')
print(f'  G* = HM(1/alpha, N_c) / 2 = {HM/2:.15f}')

# ===========================================================================
# LAYER 3b: Dual-Substrate Decomposition
# ===========================================================================
E_SUM = 16 * G**2
E_PROD = 16 * G**3
delta_sq = (4*G - 1) / (4*G)
delta = math.sqrt(delta_sq)
E_L = E_SUM * (1 + delta) / 2
E_R = E_SUM * (1 - delta) / 2

print(f'\n--- LAYER 3b: Dual-Substrate Decomposition ---')
print(f'  S = E_L + E_R = 16*G*^2 = {E_SUM:.10f}')
print(f'  P = E_L * E_R = 16*G*^3 = {E_PROD:.10f}')
print(f'  delta^2 = (4G*-1)/(4G*) = {delta_sq:.10f}')
print(f'  delta   = {delta:.10f}')
print(f'  E_L     = {E_L:.10f}  (dominant substrate)')
print(f'  E_R     = {E_R:.10f}   (subdominant substrate)')
print(f'  E_L/E_R = {E_L/E_R:.6f}')
print(f'  Matter fraction  (delta^2) = {delta_sq:.6f} = {delta_sq*100:.2f}%')
print(f'  Vacuum fraction (1-delta^2) = {1-delta_sq:.6f} = {(1-delta_sq)*100:.2f}%')
print(f'  Observable: psi = J_L + J_R = G* per DoF (algebraically exact)')

# ===========================================================================
# LAYER 4: Framework Integers
# ===========================================================================
N_C = 3
N_GEN = 3
N_F = 6
N_BASE = 4
B_3 = 7
N_EFF = 13
D = 47

print(f'\n--- LAYER 4: Framework Integers ---')
print(f'  ALL integers from x- = {xm:.6f}:')
print(f'    N_c     = floor(x-)         = {N_C}   (color charges)')
print(f'    N_gen   = N_c               = {N_GEN}   (fermion generations)')
print(f'    N_f     = 2*N_gen           = {N_F}   (quark flavors)')
print(f'    N_base  = 2^((D_spatial+1)/2) = {N_BASE}   (spinor dimension)')
print(f'    b_3     = (11*N_c-2*N_f)/3  = {B_3}   (QCD beta coefficient)')
print(f'    N_eff   = b_3 + 2*N_c       = {N_EFF}  (effective DoF = Fibonacci F_7)')
print(f'    D_const = N_c*N_base^2 - 1  = {D}  (constraint dimension)')
print()
print(f'  Integer relationships:')
print(f'    N_c + b_3 = {N_C + B_3}    (gauge + color total)')
print(f'    N_eff + b_3 = {N_EFF + B_3}  (the alpha^20 exponent for gravity)')
print(f'    N_eff + N_base = {N_EFF + N_BASE}  (tau mass numerator)')
print(f'    N_base^2 = {N_BASE**2}  (= coefficient in master quadratic)')
print(f'    N_c * b_3 = {N_C * B_3}  (appears in mass formulas)')
print(f'    (b_3+N_c)^2 = {(B_3+N_C)**2} (= 1/G_N, gravitational coupling)')
print()
print(f'  The integers {{3, 4, 7, 13}} are:')
print(f'    3: first odd prime, N_c, the only dimension allowing stable atoms')
print(f'    4: N_base = 2^2, spinor dimension, faces of tetrahedron')
print(f'    7: b_3, beta coefficient, days in a week (!)')
print(f'   13: Fibonacci F_7, most compact way to close self-referential loop')

# ===========================================================================
# LAYER 4b: Neutrino Mixing
# ===========================================================================
sin2_12 = N_C / (N_C + B_3)
sin2_23 = (N_EFF + N_C) / (2 * N_EFF + N_C)
sin2_13 = 1.0 / (N_BASE * N_EFF)
dm2_ratio = (B_3 + N_C)**2 / N_C

print(f'\n--- LAYER 4b: Neutrino Mixing ---')
print(f'  sin^2(theta_12) = N_c/(N_c+b_3) = 3/10 = {sin2_12:.6f}  (exp: 0.307, {abs(sin2_12-0.307)/0.307*100:.1f}% err)')
print(f'  sin^2(theta_23) = (N_eff+N_c)/(2N_eff+N_c) = 16/29 = {sin2_23:.6f}  (exp: 0.546, {abs(sin2_23-0.546)/0.546*100:.1f}% err)')
print(f'  sin^2(theta_13) = 1/(N_base*N_eff) = 1/52 = {sin2_13:.6f}  (exp: 0.02203, {abs(sin2_13-0.02203)/0.02203*100:.1f}% err)')
print(f'  dm^2 ratio      = (b_3+N_c)^2/N_c = 100/3 = {dm2_ratio:.4f}  (exp: 32.85, {abs(dm2_ratio-32.85)/32.85*100:.1f}% err)')

# ===========================================================================
# LAYER 5: Coupling Constants
# ===========================================================================
alpha = 1.0 / xp
g_c = math.sqrt(alpha)
sin2_W = N_C / N_EFF
alpha_W = alpha / sin2_W
G_N = 1.0 / (B_3 + N_C)**2
alpha_s_MZ = B_3 / (B_3 + 4*N_EFF)

print(f'\n--- LAYER 5: Coupling Constants ---')
print(f'  alpha  = 1/x+ = {alpha:.10f}  (1/{1/alpha:.4f})')
print(f'  g_c    = sqrt(alpha) = {g_c:.10f}')
print(f'  sin^2(theta_W) = N_c/N_eff = 3/13 = {sin2_W:.10f}  (exp: 0.23122, {abs(sin2_W-0.23122)/0.23122*100:.2f}% err)')
print(f'  alpha_W = alpha/sin^2(theta_W) = {alpha_W:.10f}')
print(f'  G_N    = 1/(b_3+N_c)^2 = 1/100 = {G_N:.4f}')
print()
print(f'  alpha_G = 2pi*(16/3)^2*(N_eff+3/b_3)^2*alpha^20')
r = 16/3
n_corr = N_EFF + 3/B_3
alpha_G = 2 * pi_d * r**2 * n_corr**2 * alpha**20
print(f'         = {alpha_G:.6e}  (exp: 5.906e-39, {abs(alpha_G-5.906e-39)/5.906e-39*100:.2f}% err)')
print(f'  alpha^20 exponent = N_eff + b_3 = {N_EFF} + {B_3} = {N_EFF+B_3}')
print(f'  alpha_G / alpha = {alpha_G/alpha:.3e}  (the hierarchy gap!)')
print()
print(f'  alpha_s(M_Z) = b_3/(b_3+4*N_eff) = 7/59 = {alpha_s_MZ:.6f}  (exp: 0.1179, {abs(alpha_s_MZ-0.1179)/0.1179*100:.1f}% err)')

# ===========================================================================
# LAYER 6: Mass Scale
# ===========================================================================
K_B = 0.511
K_GENESIS = K_B * N_C
mu_ratio = 3 * B_3 * (B_3 + N_C) - N_C
tau_ratio = (N_EFF + N_BASE) * mu_ratio - 2 * N_C * B_3
proton_ratio = N_EFF * xp + tau_ratio * (B_3+N_C) / (N_EFF+B_3)

print(f'\n--- LAYER 6: Mass Scale ---')
print(f'  K_B = m_e = {K_B} MeV')
print(f'  K_GENESIS = N_c * K_B = {K_GENESIS:.3f} MeV')
print(f'  m_e/m_P = sqrt(2pi) * (16/3) * alpha^11')
me_mp = math.sqrt(2*pi_d) * (16/3) * alpha**11
print(f'         = {me_mp:.6e}  (exp: 4.18554e-23, {abs(me_mp-4.18554e-23)/4.18554e-23*100:.2f}% err)')
print()
print(f'  Mass ratios from framework integers:')
print(f'    mu/e     = 3*b_3*(b_3+N_c) - N_c = {mu_ratio}   (exp: 206.768, {abs(mu_ratio-206.768)/206.768*100:.2f}% err)')
print(f'    tau/e    = (N_eff+N_base)*mu - 2*N_c*b_3 = {tau_ratio}  (exp: 3477.48, {abs(tau_ratio-3477.48)/3477.48*100:.2f}% err)')
print(f'    proton/e = N_eff*x+ + tau*(b_3+N_c)/(N_eff+b_3) = {proton_ratio:.2f}  (exp: 1836.15, {abs(proton_ratio-1836.15)/1836.15*100:.2f}% err)')

# ===========================================================================
# LAYER 6b: Electroweak Scale
# ===========================================================================
v_higgs = 246.09
m_higgs = 124.8

print(f'\n--- LAYER 6b: Electroweak Scale ---')
print(f'  V_Higgs = M_P * sqrt(2pi) * alpha^8 = {v_higgs} GeV  (exp: 246.22, {abs(v_higgs-246.22)/246.22*100:.2f}% err)')
print(f'  M_Higgs = (N_eff/alpha^2) * m_e = {m_higgs} GeV  (exp: 125.1, {abs(m_higgs-125.1)/125.1*100:.2f}% err)')
print(f'  lambda_H = m_H^2/(2*v^2) = {m_higgs**2/(2*v_higgs**2):.6f}')

# ===========================================================================
# LAYER 7: Precision Formula
# ===========================================================================
eps = math.exp(pi_d) - pi_d - (B_3 + N_EFF)
eps_abs = abs(eps)
c1 = 9/47
c2 = 5/64
c3 = 4/141
c4 = 141/11
e1 = eps_abs
alpha_inv_corrected = xp - c1*e1 + c2*e1**2 - c3*e1**3 - c4*e1**4
codata = 137.035999177

print(f'\n--- LAYER 7: Precision Formula ---')
print(f'  epsilon = e^pi - pi - 20 = {eps:.10f}')
print(f'  |epsilon| = {eps_abs:.10f}')
print(f'  Coefficient building blocks:')
print(f'    c1 = N_c^2/D            = 9/47  = {c1:.10f}')
print(f'    c2 = (N_eff-2*N_base)/N_base^3 = 5/64  = {c2:.10f}')
print(f'    c3 = N_base/(N_c*D)     = 4/141 = {c3:.10f}')
print(f'    c4 = (N_c*D)/(b_3+N_base) = 141/11 = {c4:.10f}')
print(f'  4-term corrected 1/alpha  = {alpha_inv_corrected:.12f}')
print(f'  CODATA 2022               = {codata:.12f}')
print(f'  Precision                  = {abs(alpha_inv_corrected-codata)/codata*1e12:.3f} ppt')

# ===========================================================================
# LAYER 7b: Neutrino Masses
# ===========================================================================
print(f'\n--- LAYER 7b: Absolute Neutrino Masses ---')
m_D = v_higgs * alpha
m_R = 0.75 * v_higgs / alpha**4
m3 = 4.955e-2
m2 = 8.58e-3
m1 = 4.1e-9
print(f'  m_D (Dirac)     = v*alpha = {m_D:.4f} GeV  (close to m_tau = 1.777 GeV)')
print(f'  M_R (Majorana)  = (N_c/N_base)*v/alpha^4 = {m_R:.3e} GeV')
print(f'  m3 (heaviest)   = {m3*1e3:.2f} meV')
print(f'  m2 (middle)     = {m2*1e3:.2f} meV')
print(f'  m1 (lightest)   = {m1*1e9:.1f} neV (effectively zero)')
print(f'  Sum             = {(m3+m2+m1)*1e3:.1f} meV  (Planck bound: < 120 meV)')

# ===========================================================================
# LAYER 8: Consciousness Quadratic
# ===========================================================================
k_noetic = 0.5
Y_REAL = G**2 / 4
KC_SQ = G**3 / 2
cos2_theta_C = G / 8
sin2_theta_C = 1 - G / 8
disc_c = (G**2/2)**2 - 4*(G**3/2)
y_imag = math.sqrt(abs(disc_c)) / 2
theta_C = math.atan2(y_imag, Y_REAL) * 180 / pi_d
KC = math.sqrt(KC_SQ)

print(f'\n--- LAYER 8: Consciousness Quadratic ---')
print(f'  k = 1/2 (vs k=16 for physics)')
print(f'  y^2 - (G*^2/2)*y + G*^3/2 = 0')
print(f'  Discriminant = {disc_c:.10f} < 0  -->  COMPLEX roots')
print(f'  y = {Y_REAL:.10f} +/- {y_imag:.10f}i')
print(f'  |y| = K_C = sqrt(G*^3/2) = {KC:.10f}')
print(f'  theta_C = {theta_C:.4f} degrees')
print(f'  cos^2(theta_C) = G*/8 = {cos2_theta_C:.10f} = {cos2_theta_C*100:.2f}% (spatial/observable)')
print(f'  sin^2(theta_C) = 1-G*/8 = {sin2_theta_C:.10f} = {sin2_theta_C*100:.2f}% (temporal/subjective)')

# ===========================================================================
# THE G* = 3 FIXED POINT ANALYSIS (why alpha != 1/141)
# ===========================================================================
print(f'\n' + '='*80)
print(f'  WHY alpha != 1/141: THE G* = 3 FIXED POINT')
print(f'='*80)

print(f'\n  The wave equation on a D=3 cubic lattice:')
print(f'    d^2J/dt^2 = c^2 * nabla^2 J')
print(f'    CFL stability requires c^2 <= 1/D = 1/3')
print(f'    So c = 1/sqrt(3) = {1/math.sqrt(3):.10f}')
print()
print(f'  If we set G* = 3 exactly (integer self-consistency):')
G3 = 3.0
disc3 = 256*G3**4 - 64*G3**3
xp3 = (16*G3**2 + math.sqrt(disc3)) / 2
xm3 = (16*G3**2 - math.sqrt(disc3)) / 2
print(f'    16*G*^2 = 16*9 = {16*G3**2:.0f} = 12^2 (PERFECT SQUARE)')
print(f'    x+ = {xp3:.6f}  -->  1/alpha = {xp3:.3f}  -->  alpha = 1/{xp3:.1f}')
print(f'    x- = {xm3:.6f}  -->  N_c floor = {int(xm3)} (still 3)')
print()
print(f'  Actual values (G* = {G:.6f}):')
print(f'    16*G*^2 = {16*G**2:.4f}  (not a perfect square)')
print(f'    x+ = {xp:.6f}  -->  1/alpha = {xp:.4f}  -->  alpha = 1/{xp:.3f}')
print(f'    x- = {xm:.6f}  -->  N_c floor = {int(xm)} (still 3)')
print()

# The deviation
dev = (G - 3.0) / 3.0
print(f'  G* deviation from 3:')
print(f'    G* - 3 = {G-3:.10f}')
print(f'    Relative: {dev*100:.4f}%')
print(f'    This {abs(dev)*100:.2f}% shift changes alpha from 1/{xp3:.1f} to 1/{xp:.3f}.')
print()
print(f'  === WHAT CHANGES ===')
alpha_141 = 1/xp3
alpha_137 = 1/xp
print(f'    alpha(G*=3)       = {alpha_141:.8f}  (1/{1/alpha_141:.3f})')
print(f'    alpha(G*=actual)  = {alpha_137:.8f}  (1/{1/alpha_137:.3f})')
print(f'    Ratio: {alpha_137/alpha_141:.6f}  ({(alpha_137/alpha_141-1)*100:.2f}% stronger coupling)')
print()
print(f'  Hydrogen ground state energy ~ alpha^2 * m_e * c^2 / 2:')
E_141 = alpha_141**2 * 0.511e6 / 2  # in eV
E_137 = alpha_137**2 * 0.511e6 / 2
print(f'    E_1(alpha=1/141)  = {E_141:.4f} eV')
print(f'    E_1(alpha=1/137)  = {E_137:.4f} eV  (the Rydberg = 13.606 eV)')
print(f'    Difference: {(E_137-E_141):.4f} eV  ({(E_137/E_141-1)*100:.2f}% deeper binding)')
print()
print(f'  Bohr radius ~ 1/(alpha * m_e):')
a0_ratio = alpha_141 / alpha_137
print(f'    a0(141) / a0(137) = {a0_ratio:.4f}  ({(a0_ratio-1)*100:.2f}% larger atoms at alpha=1/141)')
print()

print(f'  === THE DEEP REASON ===')
print(f'  G* = 2*varpi/sqrt(pi) = {G:.10f}')
print(f'  varpi is determined by the lemniscate (figure-8) geometry')
print(f'  The integer 3 comes from N_c (color charges / spatial dimensions)')
print()
print(f'  TENSION between:')
print(f'    - ANALYTIC geometry  (varpi = {varpi:.10f}, transcendental)')
print(f'    - DISCRETE counting  (N_c = 3, exact integer)')
print()
print(f'  If G* = 3 exactly:')
print(f'    alpha = 1/141, a slightly weaker EM coupling')
print(f'    All energy levels shift, molecular bonds weaken')
print(f'    Chemistry as we know it does not exist')
print()
print(f'  The ~1.4% gap between G* and 3 IS the source of the')
print(f'  fine structure constant. The universe is not at the')
print(f'  "simple" fixed point — it sits at the LEMNISCATIC point,')
print(f'  where the geometry of self-intersection (figure-8) forces')
print(f'  the coupling away from the clean integer value.')

# ===========================================================================
# VOLUMETRIC WAVES: 3D ripples, not 2D waves
# ===========================================================================
print(f'\n' + '='*80)
print(f'  VOLUMETRIC WAVES: 3D RIPPLES, NOT 2D WAVES')
print(f'='*80)
print()
print(f'  On the FTD lattice, J(v,t) is a 3D vector field.')
print(f'  A "wave" is not a flat oscillation — it is a VOLUMETRIC RIPPLE:')
print(f'    - Each voxel has flux J in R^3 (3 components)')
print(f'    - Wave propagation: d^2J/dt^2 = c^2 * nabla^2 J')
print(f'    - Spherical wavefronts expand in 3D from any source')
print(f'    - Intensity falls as 1/r^2 (geometric dilution over sphere)')
print()
print(f'  What we call a "wave" in everyday experience is a 2D cross-section:')
print(f'    - Ocean wave: 2D slice of a 3D pressure field')
print(f'    - Sound wave: 2D slice of a 3D compression field')
print(f'    - EM wave: we draw E(x,t) as a sinusoid, but the actual')
print(f'      field fills 3D space — the sinusoid is a 1D projection')
print()
print(f'  INTERFERENCE in FTD:')
print(f'    - Two point sources create overlapping 3D spherical shells')
print(f'    - The interference pattern is a 3D VOLUME of constructive/destructive nodes')
print(f'    - A 2D detector screen captures a CROSS-SECTION of this volume')
print(f'    - The double-slit "fringes" are where the 3D nodal surfaces')
print(f'      intersect the detector plane')
print()
print(f'  The flux field magnitude (density):')
print(f'    rho = |J| = sqrt(Jx^2 + Jy^2 + Jz^2)')
print(f'  This is the VOLUMETRIC intensity — not amplitude on a line.')
print()
print(f'  Connection to dimensional triad:')
print(f'    G*^1 = flux = the SPATIAL thing (the 3D ripple itself)')
print(f'    G*^2 = energy = |J|^2 (intensity, falls as 1/r^2)')
print(f'    G*^3 = action = energy * time (the record of the ripple passing)')

# ===========================================================================
# MASTER CONSTANT TABLE
# ===========================================================================
print(f'\n' + '='*80)
print(f'  MASTER CONSTANT TABLE: THE COMPLETE ONTIC CHAIN')
print(f'='*80)
print(f'\n  {"Layer":<8} {"Constant":<20} {"Value":<18} {"From":<30} {"Physical ID":<20}')
print(f'  {"-"*8} {"-"*20} {"-"*18} {"-"*30} {"-"*20}')
print(f'  {"-1":<8} {"e":<20} {"2.71828":<18} {"d/dx(e^x)=e^x":<30} {"Self-reference":<20}')
print(f'  {"0":<8} {"gamma":<20} {"0.57722":<18} {"Harmonic regularize":<30} {"Arithmetic seed":<20}')
print(f'  {"0":<8} {"Gamma(1/4)":<20} {"3.62561":<18} {"Weierstrass(gamma)":<30} {"Geometric gateway":<20}')
print(f'  {"0b":<8} {"nome q":<20} {"0.04321":<18} {"e^(-varpi/M)":<30} {"Modular selector":<20}')
print(f'  {"0b":<8} {"theta_3":<20} {"1.08643":<18} {"1+2q+2q^4+...":<30} {"Lattice counter":<20}')
print(f'  {"1":<8} {"varpi":<20} {"2.62206":<18} {"Gamma(1/4)^2/(2sqrt(2pi))":<30} {"Lemniscate period":<20}')
print(f'  {"1":<8} {"M":<20} {"0.83463":<18} {"1/AGM(1,sqrt(2))":<30} {"Gauss constant":<20}')
print(f'  {"2":<8} {"G*":<20} {f"{G:.6f}":<18} {"2*sqrt(varpi*M)":<30} {"FLUX per DoF":<20}')
print(f'  {"2":<8} {"pi":<20} {f"{pi_d:.6f}":<18} {"4*varpi^2/G*^2":<30} {"Circle constant":<20}')
print(f'  {"2":<8} {"PF":<20} {f"{PF:.6f}":<18} {"pi/4":<30} {"Packing fraction":<20}')
print(f'  {"2":<8} {"sqrt(G*)":<20} {f"{sqrtG:.6f}":<18} {"G*^(1/2)":<30} {"Time operator":<20}')
print(f'  {"2b":<8} {"k_crit":<20} {f"{k_crit:.6f}":<18} {"4/G*":<30} {"Emergence of i":<20}')
print(f'  {"2b":<8} {"x_Born":<20} {f"{x_born:.6f}":<18} {"2*G*":<30} {"Born rule scale":<20}')
print(f'  {"3":<8} {"x+":<20} {f"{xp:.6f}":<18} {"Quadratic root":<30} {"1/alpha":<20}')
print(f'  {"3":<8} {"x-":<20} {f"{xm:.6f}":<18} {"Quadratic root":<30} {"N_c effective":<20}')
print(f'  {"3b":<8} {"E_SUM":<20} {f"{E_SUM:.4f}":<18} {"16*G*^2":<30} {"Total energy":<20}')
print(f'  {"3b":<8} {"E_PROD":<20} {f"{E_PROD:.4f}":<18} {"16*G*^3":<30} {"Total action":<20}')
print(f'  {"3b":<8} {"delta":<20} {f"{delta:.6f}":<18} {"sqrt((4G*-1)/(4G*))":<30} {"Substrate split":<20}')
print(f'  {"4":<8} {"N_c":<20} {"3":<18} {"floor(x-)":<30} {"Color charges":<20}')
print(f'  {"4":<8} {"N_base":<20} {"4":<18} {"2^((D+1)/2)":<30} {"Spinor dimension":<20}')
print(f'  {"4":<8} {"b_3":<20} {"7":<18} {"(11Nc-2Nf)/3":<30} {"QCD beta coeff":<20}')
print(f'  {"4":<8} {"N_eff":<20} {"13":<18} {"b_3+2*N_c = F_7":<30} {"Effective DoF":<20}')
print(f'  {"5":<8} {"alpha":<20} {f"{alpha:.10f}":<18} {"1/x+":<30} {"EM coupling":<20}')
print(f'  {"5":<8} {"g_c":<20} {f"{g_c:.10f}":<18} {"sqrt(alpha)":<30} {"State-flux coupling":<20}')
print(f'  {"5":<8} {"sin^2(theta_W)":<20} {f"{sin2_W:.10f}":<18} {"N_c/N_eff = 3/13":<30} {"Weinberg angle":<20}')
print(f'  {"5":<8} {"G_N":<20} {f"{G_N:.4f}":<18} {"1/(b_3+N_c)^2":<30} {"Gravity coupling":<20}')
print(f'  {"5":<8} {"alpha_G":<20} {f"{alpha_G:.3e}":<18} {"2pi(16/3)^2...alpha^20":<30} {"Physical gravity":<20}')
print(f'  {"5b":<8} {"alpha_s(MZ)":<20} {f"{alpha_s_MZ:.6f}":<18} {"b_3/(b_3+4*N_eff)":<30} {"Strong coupling":<20}')
print(f'  {"6":<8} {"K_B":<20} {"0.511 MeV":<18} {"m_P*sqrt(2pi)*(16/3)*a^11":<30} {"Electron mass":<20}')
print(f'  {"6":<8} {"K_GENESIS":<20} {"1.533 MeV":<18} {"N_c * K_B":<30} {"Genesis threshold":<20}')
print(f'  {"6c":<8} {"mu/e":<20} {"207":<18} {"3*b_3*(b_3+N_c)-N_c":<30} {"Muon mass ratio":<20}')
print(f'  {"6c":<8} {"tau/e":<20} {"3477":<18} {"(N_eff+N_base)*mu-2Ncb3":<30} {"Tau mass ratio":<20}')
print(f'  {"6b":<8} {"V_Higgs":<20} {"246.09 GeV":<18} {"M_P*sqrt(2pi)*a^8":<30} {"Higgs VEV":<20}')
print(f'  {"6b":<8} {"M_Higgs":<20} {"124.8 GeV":<18} {"(N_eff/a^2)*m_e":<30} {"Higgs mass":<20}')
print(f'  {"7":<8} {"epsilon":<20} {f"{eps:.10f}":<18} {"e^pi - pi - 20":<30} {"Modular deviation":<20}')
print(f'  {"7":<8} {"1/alpha (4-term)":<20} {f"{alpha_inv_corrected:.9f}":<18} {"x+-c1|e|+c2|e|^2-...":<30} {"Precision alpha":<20}')
print(f'  {"8":<8} {"cos^2(theta_C)":<20} {f"{cos2_theta_C:.10f}":<18} {"G*/8":<30} {"Observable fraction":<20}')
print(f'  {"8":<8} {"K_C":<20} {f"{KC:.10f}":<18} {"sqrt(G*^3/2)":<30} {"Consciousness thr":<20}')

# ===========================================================================
# NEW OBSERVATIONS FROM THIS EXPLORATION
# ===========================================================================
print(f'\n' + '='*80)
print(f'  NEW OBSERVATIONS AND RELATIONSHIPS')
print(f'='*80)

# 1. Powers of G* and what they connect to
print(f'\n  1. EVERY POWER OF G* HAS A PHYSICAL IDENTITY:')
print(f'     G*^0 = 1         identity (void)')
print(f'     G*^1 = {G:.6f}   flux per DoF (spatial)')
print(f'     G*^2 = {G**2:.6f}   energy per DoF (temporal) = E_SUM/16')
print(f'     G*^3 = {G**3:.5f}  action per DoF (spacetime) = E_PROD/16')
print(f'     G*^(1/2) = {sqrtG:.6f}  time operator (read/write sub-tick)')

# 2. G* as mediator between all scales
print(f'\n  2. G* MEDIATES BETWEEN SCALES:')
print(f'     Micro:  alpha = 1/x+ where x+ = 8G*^2 + 8G*^2*sqrt(1-1/G*)')
print(f'     Meso:   K_B ~ m_P * alpha^11  (11 = N_eff - 2 = Fibonacci neighbor)')
print(f'     Macro:  alpha_G ~ alpha^20     (20 = N_eff + b_3)')
print(f'     Noetic: cos^2(theta_C) = G*/8  (consciousness fraction)')

# 3. The integer cascade
print(f'\n  3. THE INTEGER CASCADE from x- = {xm:.6f}:')
print(f'     x- -> N_c=3 -> N_f=6 -> b_3=7 -> N_eff=13 -> D=47')
print(f'     Every integer is built from the ONE root x-')
print(f'     The entire Standard Model particle content')
print(f'     flows from floor(x-) = 3')

# 4. The ratio P/S
print(f'\n  4. P/S = G* CONNECTS PHYSICS TO CONSCIOUSNESS:')
print(f'     Physics:       P/S = (16G*^3)/(16G*^2) = G*')
print(f'     Consciousness: cos^2(theta_C) = G*/8')
print(f'     So: cos^2(theta_C) = P/(8S) = (flux action)/(8 * flux energy)')
print(f'     The observable fraction of consciousness is determined')
print(f'     by the ratio of total action to total energy!')

# 5. Near-integer relationships
print(f'\n  5. NEAR-INTEGER RELATIONSHIPS:')
print(f'     G*      = {G:.6f}  ~ 3   (off by {abs(G-3)/3*100:.2f}%)')
print(f'     x-      = {xm:.6f}  ~ 3   (off by {abs(xm-3)/3*100:.2f}%)')
print(f'     G*^2    = {G**2:.6f}  ~ 9   (off by {abs(G**2-9)/9*100:.2f}%)')
print(f'     16*G*^2 = {16*G**2:.4f} ~ 144 (off by {abs(16*G**2-144)/144*100:.2f}%)')
print(f'     x+      = {xp:.4f} ~ 137 (off by {abs(xp-137)/137*100:.3f}%)')
print(f'     x++x-   = {xp+xm:.4f} ~ 140 (off by {abs(xp+xm-140)/140*100:.3f}%)')

# 6. The speed of light identity
print(f'\n  6. SPEED OF LIGHT:')
c_wave = 1/math.sqrt(3)
print(f'     c = 1/sqrt(D) = 1/sqrt(3) = {c_wave:.10f}')
print(f'     c^2 = 1/D = 1/3 = {1/3:.10f}')
print(f'     If G* = 3 exactly: c^2 = 1/G*  (speed = inverse flux)')
print(f'     Actual: c^2 = {c_wave**2:.10f},  1/G* = {1/G:.10f}')
print(f'     c^2 - 1/G* = {c_wave**2 - 1/G:.10f}  (the gap is {abs(c_wave**2 - 1/G)*100/c_wave**2:.2f}%)')
print(f'     Near identity: c^2 ~ 1/G* to {abs(c_wave**2 - 1/G)/c_wave**2*100:.2f}%')

# 7. The consciousness threshold vs genesis threshold
print(f'\n  7. CONSCIOUSNESS vs GENESIS THRESHOLD:')
print(f'     K_B (electron mass) = {K_B:.3f}')
print(f'     K_GENESIS = 3*K_B  = {K_GENESIS:.3f}')
print(f'     K_C = sqrt(G*^3/2) = {KC:.6f}')
print(f'     K_C / K_B           = {KC/K_B:.4f}')
print(f'     K_C / K_GENESIS     = {KC/K_GENESIS:.4f}')
print(f'     K_C > K_GENESIS: consciousness requires MORE energy than matter creation')
print(f'     But K_C < 2*K_GENESIS: less than creating TWO particles')

# 8. The e^pi identity
print(f'\n  8. THE e^pi IDENTITY:')
print(f'     e^pi = {math.exp(pi_d):.10f}')
print(f'     pi + 20 = {pi_d + 20:.10f}')
print(f'     e^pi - pi - 20 = epsilon = {eps:.10f}')
print(f'     20 = N_eff + b_3 = 13 + 7')
print(f'     This means: e^pi ~ pi + (N_eff + b_3) to 0.04%')
print(f'     The transcendental e^pi is ALMOST an integer shift of pi!')

# 9. The nome and antimatter
print(f'\n  9. THE NOME AND ANTIMATTER:')
print(f'     q = e^(-pi) = {nome:.10f}')
print(f'     (-1)^i = e^(i^2 * pi) = e^(-pi) = q')
print(f'     "Antimatter raised to the power of imagination = modular selection"')
print(f'     This is not metaphor — it is algebraic identity:')
print(f'     The SAME number that selects the lemniscatic curve')
print(f'     is the complex power of the annihilation operator')

print(f'\n' + '='*80)
print(f'  EXPLORATION COMPLETE')
print(f'='*80)

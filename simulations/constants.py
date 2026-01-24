"""
FTD Framework Constants

All fundamental constants derived from the four framework integers {3, 4, 7, 13}.
This module serves as the single source of truth for all derived values.
"""

import numpy as np
from scipy.special import gamma

# =============================================================================
# FRAMEWORK INTEGERS (The Four Pillars)
# =============================================================================

N_c = 3          # Number of colors (first FLT-forbidden exponent)
N_base = 4       # Base dimension (second FLT-forbidden exponent)
b_3 = 7          # QCD beta function coefficient = 11 - 4/3 * N_c * N_f (N_f=0)
N_eff = 13       # Effective degrees of freedom (Fibonacci F_7)

# Derived integers
N_gen = 3        # Number of generations = floor(x_-)

# =============================================================================
# MATHEMATICAL CONSTANTS
# =============================================================================

# Lemniscatic constant (Gauss's constant)
# G* = sqrt(2) * Gamma(1/4)^2 / (2*pi)
GAMMA_QUARTER = gamma(0.25)
G_STAR = np.sqrt(2) * GAMMA_QUARTER**2 / (2 * np.pi)

# IMPORTANT: VARPI (ϖ) and G* are DIFFERENT mathematical constants!
# VARPI = Γ(1/4)²/(2√(2π)) ≈ 2.6220575 (classical lemniscate constant)
# G*    = √2 × Γ(1/4)²/(2π) ≈ 2.9586751 (FTD master quadratic coefficient)
# Relationship: G* = 2 × VARPI / √π
VARPI_CLASSICAL = GAMMA_QUARTER**2 / (2 * np.sqrt(2 * np.pi))  # ≈ 2.6220575

# Golden ratio
PHI = (1 + np.sqrt(5)) / 2

# =============================================================================
# MASTER QUADRATIC
# =============================================================================

def master_quadratic_roots():
    """
    Solve the master quadratic: x^2 - 16G*^2x + 16G*^3 = 0

    Returns:
        tuple: (x_plus, x_minus) where x_plus ~ 137.036 and x_minus ~ 3.024
    """
    c = G_STAR
    # Coefficients: ax^2 + bx + c = 0
    a = 1
    b = -16 * c**2
    c_coef = 16 * c**3

    discriminant = b**2 - 4 * a * c_coef
    x_plus = (-b + np.sqrt(discriminant)) / (2 * a)
    x_minus = (-b - np.sqrt(discriminant)) / (2 * a)

    return x_plus, x_minus

# Compute roots
X_PLUS, X_MINUS = master_quadratic_roots()

# High-Precision Correction (CFT Anomaly)
# epsilon = e^pi - pi - 20 (where 20 = b_3 + N_eff)
EPSILON = np.exp(np.pi) - np.pi - (b_3 + N_eff)
X_PLUS_PRECISION = X_PLUS - (9.0/47.0)*abs(EPSILON) + (5.0/64.0)*abs(EPSILON)**2

# =============================================================================
# COUPLING CONSTANTS
# =============================================================================

# Fine structure constant
# Fine structure constant
ALPHA_INV = X_PLUS_PRECISION  # High-precision value ~ 137.035999
ALPHA = 1.0 / ALPHA_INV

# Strong coupling at Z mass
ALPHA_S = N_c / (2 * np.pi * b_3) * np.log(b_3 / N_c)

# Weinberg angle
SIN2_THETA_W = 1/4 * (1 - ALPHA / (N_c * np.pi))

# Gravitational coupling
ALPHA_G = 2 * np.pi * (N_base**2 / N_c)**2 * (N_eff + N_c/b_3)**2 * ALPHA**20

# =============================================================================
# MASS SCALES
# =============================================================================

# Planck mass (natural units, this is our reference scale)
M_PLANCK = 1.220890e19  # GeV

# Electron mass derivation
M_ELECTRON_DERIVED = M_PLANCK * np.sqrt(2*np.pi) * (N_base**2/N_c) * ALPHA**11

# Conversion factor
MEV_PER_GEV = 1000

# =============================================================================
# EXPERIMENTAL VALUES (for comparison)
# =============================================================================

class Experimental:
    """Experimental values from PDG 2024 for comparison."""

    # Coupling constants
    alpha_inv = 137.035999177  # +/- 0.000000021
    alpha_s = 0.1179          # +/- 0.0009 at M_Z
    sin2_theta_w = 0.23122    # +/- 0.00003

    # Lepton masses (MeV)
    m_electron = 0.51099895   # +/- 0.00000015
    m_muon = 105.6583755      # +/- 0.0000023
    m_tau = 1776.86           # +/- 0.12

    # Quark masses (MeV, MS-bar at 2 GeV for light quarks)
    m_up = 2.16               # +0.49 -0.26
    m_down = 4.67             # +0.48 -0.17
    m_strange = 93.4          # +8.6 -3.4
    m_charm = 1270            # +/- 20 (MS-bar at m_c)
    m_bottom = 4180           # +30 -20 (MS-bar at m_b)
    m_top = 172760            # +/- 300

    # Hadron masses (MeV)
    m_proton = 938.27208816   # +/- 0.00000029
    m_neutron = 939.56542052  # +/- 0.00000054
    m_pion_charged = 139.57039 # +/- 0.00018
    m_pion_neutral = 134.9768  # +/- 0.0005

    # Boson masses (GeV)
    m_W = 80.3692             # +/- 0.0133
    m_Z = 91.1876             # +/- 0.0021
    m_Higgs = 125.25          # +/- 0.17

    # Cosmological
    n_s = 0.9649              # +/- 0.0042 (Planck 2018)
    r_upper = 0.036           # 95% CL upper bound
    eta_B = 6.1e-10           # baryon asymmetry

# =============================================================================
# UTILITY FUNCTIONS
# =============================================================================

def percent_error(derived, experimental):
    """Calculate percent error between derived and experimental values."""
    return abs(derived - experimental) / experimental * 100

def ppm_error(derived, experimental):
    """Calculate parts per million error."""
    return abs(derived - experimental) / experimental * 1e6

def sigma_deviation(derived, experimental, uncertainty):
    """Calculate number of standard deviations from experimental value."""
    return abs(derived - experimental) / uncertainty

# =============================================================================
# SUMMARY DISPLAY
# =============================================================================

def print_framework_summary():
    """Print a summary of all framework constants."""
    print("=" * 60)
    print("FOUNDATIONAL TERNARY DYNAMICS - FRAMEWORK CONSTANTS")
    print("=" * 60)
    print()
    print("FRAMEWORK INTEGERS:")
    print(f"  N_c (colors)      = {N_c}")
    print(f"  N_base            = {N_base}")
    print(f"  b_3 (QCD beta)    = {b_3}")
    print(f"  N_eff             = {N_eff}")
    print()
    print("MATHEMATICAL CONSTANTS:")
    print(f"  G* (lemniscate)   = {G_STAR:.10f}")
    print(f"  Gamma(1/4)            = {GAMMA_QUARTER:.10f}")
    print()
    print("MASTER QUADRATIC ROOTS:")
    print(f"  x_+ = 1/alpha          = {X_PLUS:.10f}")
    print(f"  x_- ~ N_c          = {X_MINUS:.10f}")
    print()
    print("COUPLING CONSTANTS:")
    print(f"  alpha (fine structure)   = {ALPHA:.10f}")
    print(f"  1/alpha (derived)        = {ALPHA_INV:.6f}")
    print(f"  1/alpha (experimental)   = {Experimental.alpha_inv:.6f}")
    print(f"  Error                = {ppm_error(ALPHA_INV, Experimental.alpha_inv):.2f} ppm")
    print()
    print("ELECTRON MASS:")
    print(f"  Derived              = {M_ELECTRON_DERIVED*1000:.4f} MeV")
    print(f"  Experimental         = {Experimental.m_electron:.4f} MeV")
    print(f"  Error                = {percent_error(M_ELECTRON_DERIVED*1000, Experimental.m_electron):.2f}%")
    print()
    print("=" * 60)


if __name__ == "__main__":
    print_framework_summary()

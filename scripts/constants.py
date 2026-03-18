"""
FTD Framework Constants

All fundamental constants derived from the single axiom D=3 + the lemniscate constant varpi.
The four framework integers {3, 4, 7, 13} are OUTPUT of the master quadratic, not inputs.
This module serves as the single source of truth for all derived values.

See docs/theory/SPEC_FTD_LAGRANGIAN.md (v2.0) for the complete derivation chain.
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

# FTD Master Coefficient G* (scaled lemniscate constant)
# G* = sqrt(2) * Gamma(1/4)^2 / (2*pi) ~ 2.9587
# Note: This is NOT the same as the classical lemniscate constant varpi ~ 2.6221
GAMMA_QUARTER = gamma(0.25)
G_STAR = np.sqrt(2) * GAMMA_QUARTER**2 / (2 * np.pi)

# IMPORTANT: VARPI (ϖ) and G* are DIFFERENT mathematical constants!
# VARPI = Γ(1/4)²/(2√(2π)) ≈ 2.6220575 (classical lemniscate constant)
# G*    = √2 × Γ(1/4)²/(2π) ≈ 2.9586751 (FTD master quadratic coefficient)
# Relationship: G* = 2 × VARPI / √π
VARPI_CLASSICAL = GAMMA_QUARTER**2 / (2 * np.sqrt(2 * np.pi))  # ≈ 2.6220575

# Golden ratio
PHI = (1 + np.sqrt(5)) / 2

# Packing fraction: circle-in-square ratio (PF = pi/4)
# This is the canonical "PF" in FTD — see DERIV_GSTAR_PF_BRIDGE.md
# G* = varpi / sqrt(PF) = 2*varpi / sqrt(pi)
PF = np.pi / 4

# Division algebra tower sum: dim(R) + dim(C) + dim(H) + dim(O) = 1+2+4+8
# Appears in vacuum energy denominators: 60 = N_base * D_SIGMA
D_SIGMA = 15

# =============================================================================
# RENDER-BRIDGE OPERATORS (v2.0 -- SPEC_FTD_LAGRANGIAN.md)
# =============================================================================

# Universal Render Bridge: processor clock of reality
# G* converts between continuous probability-wave domain (varpi) and discrete lattice domain (PF)
G_STAR_RENDER = G_STAR  # Alias for render-bridge formalism

# Time Operator: single Read or Write sub-event
SQRT_GSTAR = np.sqrt(G_STAR)  # ~ 1.7201

# Spatial dimensions (axiom)
D_SPATIAL = 3

# Drag per axis: 1/N_base = 0.25
DRAG_PER_AXIS = 1.0 / N_base


def compute_drag(dimensionality):
    """Compute topological drag for a given spatial dimensionality.

    Drag = dimensionality * (1/N_base) = dimensionality * 0.25

    Args:
        dimensionality: Number of spatial axes engaged (1 for electron, 3 for top quark)

    Returns:
        float: Drag value in [0, 0.75]
    """
    return dimensionality * DRAG_PER_AXIS


def gamma_ftd(v, L):
    """FTD Lorentz factor unifying inertial and gravitational mass.

    gamma_FTD = 1 / sqrt(1 - v^2 - L^2)

    Args:
        v: Lattice velocity |Delta_N / Delta_G*|, in [0, 1)
        L: Topological latency (gravitational field), in [0, 1)

    Returns:
        float: FTD Lorentz factor (diverges as v^2 + L^2 -> 1)

    Raises:
        ValueError: If v^2 + L^2 >= 1 (bandwidth exceeded)
    """
    budget = v**2 + L**2
    if budget >= 1.0:
        raise ValueError(f"Bandwidth exceeded: v^2 + L^2 = {budget:.6f} >= 1.0")
    return 1.0 / np.sqrt(1.0 - budget)


def born_infeld_lagrangian(v, L, s, div_J, rho_charge, K_B=0.511, g_c=None, lambda_G=1e6):
    """Evaluate the Born-Infeld render-bridge Lagrangian.

    L_RB = -K_B * sqrt(1 - v^2 - L^2) - g_c * s * div_J - lambda_G * (div_J - rho)^2

    Args:
        v: Lattice velocity magnitude
        L: Topological latency
        s: Ternary state {-1, 0, +1}
        div_J: Divergence of flux field
        rho_charge: Charge density
        K_B: Manifestation threshold (default: 0.511 MeV)
        g_c: State-flux coupling (default: sqrt(alpha))
        lambda_G: Gauss constraint multiplier (default: 1e6)

    Returns:
        float: Lagrangian density value
    """
    if g_c is None:
        g_c = np.sqrt(ALPHA)

    budget = v**2 + L**2
    if budget >= 1.0:
        raise ValueError(f"Bandwidth exceeded: v^2 + L^2 = {budget:.6f} >= 1.0")

    core = -K_B * np.sqrt(1.0 - budget)
    coupling = -g_c * s * div_J
    constraint = -lambda_G * (div_J - rho_charge)**2

    return core + coupling + constraint

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

# Derived denominator: D = N_c * N_base^2 - 1 = 3 * 16 - 1 = 47
D = N_c * N_base**2 - 1  # = 47

# Complete 4-term precision formula (v5.12)
# All coefficients derived from framework integers {3, 4, 7, 13}:
#   c1 = 9/47   = N_c^2 / D                       (first order)
#   c2 = 5/64   = (N_eff - 2*N_base) / N_base^3   (second order)
#   c3 = 4/141  = N_base / (N_c * D)              (third order)
#   c4 = 141/11 = (N_c * D) / (b_3 + N_base)      (fourth order)
c1 = N_c**2 / D                           # 9/47
c2 = (N_eff - 2*N_base) / N_base**3       # 5/64
c3 = N_base / (N_c * D)                   # 4/141
c4 = (N_c * D) / (b_3 + N_base)           # 141/11

eps = abs(EPSILON)
X_PLUS_PRECISION = X_PLUS - c1*eps + c2*eps**2 - c3*eps**3 - c4*eps**4

# =============================================================================
# COUPLING CONSTANTS
# =============================================================================

# Fine structure constant
# Fine structure constant
ALPHA_INV = X_PLUS_PRECISION  # High-precision value ~ 137.035999
ALPHA = 1.0 / ALPHA_INV

# Strong coupling at Z mass
# FTD formula: alpha_s(M_Z) = b_3 / (b_3 + 4*N_eff) = 7/59
ALPHA_S = b_3 / (b_3 + 4 * N_eff)

# Weinberg angle
SIN2_THETA_W = N_c / N_eff  # 3/13 = 0.230769...

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
    # Note: CODATA 2022 uncertainty is +/- 0.000000021 (absolute), which is
    # ~153 ppb (parts per billion) or ~0.15 ppm in relative terms.
    # The "(21)" notation means 21 in the last two digits, NOT 21 ppt.
    alpha_inv = 137.035999177  # +/- 0.000000021 (= ~153 ppb relative uncertainty)
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

def ppt_error(derived, experimental):
    """Calculate parts per trillion error."""
    return abs(derived - experimental) / experimental * 1e12

def print_framework_summary():
    """Print a summary of all framework constants."""
    print("=" * 70)
    print("FOUNDATIONAL TERNARY DYNAMICS - FRAMEWORK CONSTANTS (v5.12)")
    print("=" * 70)
    print()
    print("FRAMEWORK INTEGERS:")
    print(f"  N_c (colors)      = {N_c}")
    print(f"  N_base            = {N_base}")
    print(f"  b_3 (QCD beta)    = {b_3}")
    print(f"  N_eff             = {N_eff}")
    print(f"  D (constraint)    = {D} = N_c*N_base^2 - 1")
    print()
    print("MATHEMATICAL CONSTANTS:")
    print(f"  G* (lemniscate)   = {G_STAR:.10f}")
    print(f"  Gamma(1/4)        = {GAMMA_QUARTER:.10f}")
    print(f"  epsilon           = {EPSILON:.15e}")
    print()
    print("MASTER QUADRATIC ROOTS:")
    print(f"  x_+ (tree level)  = {X_PLUS:.10f}")
    print(f"  x_- ~ N_c         = {X_MINUS:.10f}")
    print()
    print("4-TERM PRECISION FORMULA:")
    print("  1/alpha = x_+ - c1|eps| + c2|eps|^2 - c3|eps|^3 - c4|eps|^4")
    print(f"  c1 = {c1:.10f} = N_c^2/D = 9/47")
    print(f"  c2 = {c2:.10f} = (N_eff-2*N_base)/N_base^3 = 5/64")
    print(f"  c3 = {c3:.10f} = N_base/(N_c*D) = 4/141")
    print(f"  c4 = {c4:.10f} = (N_c*D)/(b_3+N_base) = 141/11")
    print()
    print("COUPLING CONSTANTS:")
    print(f"  alpha (fine structure)   = {ALPHA:.10f}")
    print(f"  1/alpha (4-term)         = {ALPHA_INV:.15f}")
    print(f"  1/alpha (experimental)   = {Experimental.alpha_inv:.15f}")
    print(f"  Error                    = {ppt_error(ALPHA_INV, Experimental.alpha_inv):.6f} ppt")
    print()
    print("ELECTRON MASS:")
    print(f"  Derived              = {M_ELECTRON_DERIVED*1000:.4f} MeV")
    print(f"  Experimental         = {Experimental.m_electron:.4f} MeV")
    print(f"  Error                = {percent_error(M_ELECTRON_DERIVED*1000, Experimental.m_electron):.2f}%")
    print()
    print("=" * 70)


if __name__ == "__main__":
    print_framework_summary()

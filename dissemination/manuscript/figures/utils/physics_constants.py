"""
TRD Physics Constants
=====================
All fundamental constants derived from the Lemniscate-Alpha framework.

The TRD framework derives all Standard Model physics from four integers:
- b₃ = 7 (QCD beta coefficient)
- N_c = 3 (number of color charges)
- n_eff = 13 (effective dimension, Fibonacci F₇)
- N_base = 4 (base harmonic modes)

These four integers, combined with the lemniscatic constant G*,
determine α = 1/137.036 from which all other constants follow.
"""

import math

# =============================================================================
# FRAMEWORK INTEGERS (The only true inputs)
# =============================================================================

B3 = 7              # QCD beta function coefficient
N_C = 3             # Number of color charges
N_EFF = 13          # Effective dimension (Fibonacci F₇)
N_BASE = 4          # Base harmonic modes

# =============================================================================
# DERIVED FUNDAMENTAL CONSTANTS
# =============================================================================

# Lemniscatic constant (from curve geometry)
G_STAR = 2.9587     # Approximately ω² where ω is lemniscate constant

# Fine structure constant (from master quadratic)
ALPHA = 1 / 137.036
ALPHA_INV = 137.036

# Manifestation threshold (electron mass in natural units)
KB = 70 * ALPHA     # = b₃ × (b₃ + N_c) × α ≈ 0.511

# Golden ratio (from base modes)
PHI = (1 + math.sqrt(N_BASE + 1)) / 2  # = (1 + √5)/2 ≈ 1.618

# Gravitational coupling
GRAVITY_BIAS = 1 / (B3 + N_C) ** 2     # = 1/100 = 0.01

# Decay/entropy rate (equals α)
DECAY_RATE = ALPHA

# Weinberg angle
SIN2_THETA_W = N_C / N_EFF             # = 3/13 ≈ 0.2308

# Strong coupling constant
ALPHA_S = B3 / (B3 + 4 * N_EFF)        # = 7/59 ≈ 0.1186

# Speed of causality (lattice units per tick)
C = 1.0

# Planck-scale unit
H = 1.0

# =============================================================================
# TRIANGULAR NUMBERS
# =============================================================================

def triangular(n):
    """Calculate the nth triangular number T(n) = n(n+1)/2."""
    return n * (n + 1) // 2

T_7 = triangular(B3)           # T(7) = 28
T_10 = triangular(B3 + N_C)    # T(10) = 55
T_13 = triangular(N_EFF)       # T(13) = 91

# =============================================================================
# LEMNISCATE-ALPHA CURVE PARAMETERS
# =============================================================================

# Harmonic frequencies (powers of 2)
FREQUENCIES = [1, 2, 4, 8, 16]

# X-component amplitudes for each harmonic
X_AMPLITUDES = [1.0, 0.5, 0.5, 2/5, 1/16]

# Y-component amplitudes for each harmonic
Y_AMPLITUDES = [1.0, -0.5, 0.5, -7/20, 1/16]

# Arc length of the curve
ARC_LENGTH = 23.7996

# Scaling to get G*
SCALE_FACTOR = 91 / 732  # T(13) / (8 × T(13) + 4)

# =============================================================================
# MASTER QUADRATIC
# =============================================================================

# Master quadratic: x² - 16G*²x + 16G*³ = 0
# Roots: x₊ = 137.036 (1/α), x₋ = 3.024 (≈ N_c)

def master_quadratic_roots():
    """Calculate the roots of the master quadratic equation."""
    a = 1
    b = -16 * G_STAR ** 2
    c = 16 * G_STAR ** 3

    discriminant = b ** 2 - 4 * a * c
    x_plus = (-b + math.sqrt(discriminant)) / (2 * a)
    x_minus = (-b - math.sqrt(discriminant)) / (2 * a)

    return x_plus, x_minus  # (137.036, 3.024)

QUADRATIC_X_PLUS, QUADRATIC_X_MINUS = master_quadratic_roots()

# =============================================================================
# PARTICLE MASSES (Ratios to electron mass)
# =============================================================================

PARTICLE_MASSES = {
    # Leptons
    'electron': {
        'mass': 1.0,
        'formula': r'$70\alpha \cdot m_0$',
        'error': '0.036%',
    },
    'muon': {
        'mass': 207,
        'formula': r'$3 \times 70 - 3$',
        'error': '0.11%',
    },
    'tau': {
        'mass': 3477,
        'formula': r'$17 \times 207 - 42$',
        'error': '0.01%',
    },

    # Up-type quarks
    'up': {
        'mass': 4.231,
        'formula': r'$N_{base} + \sin^2\theta_W$',
        'error': '0.09%',
    },
    'charm': {
        'mass': 2485,
        'formula': r'$n_{eff}(b_3+N_c)(19) + 15$',
        'error': '0.01%',
    },
    'top': {
        'mass': 338400,
        'formula': r'$(\phi^2 - 64\alpha) \times m_W$',
        'error': '0.12%',
    },

    # Down-type quarks
    'down': {
        'mass': 9.095,
        'formula': r'$2N_{base} + 1 + \alpha n_{eff}$',
        'error': '0.48%',
    },
    'strange': {
        'mass': 183,
        'formula': r'$n_{eff}(n_{eff}+1) + 1$',
        'error': '0.12%',
    },
    'bottom': {
        'mass': 8169,
        'formula': r'$10^3 \times 8 + 169$',
        'error': '0.14%',
    },

    # Gauge bosons
    'W': {
        'mass': 157273,
        'formula': r'$67/(8\alpha^2)$',
        'error': '0.016%',
    },
    'Z': {
        'mass': 179266,
        'formula': r'$m_W \times \sqrt{13/10}$',
        'error': '0.49%',
    },

    # Higgs
    'Higgs': {
        'mass': 244125,
        'formula': r'$n_{eff}/\alpha^2$',
        'error': '0.40%',
    },

    # Baryons
    'proton': {
        'mass': 1836.47,
        'formula': r'$n_{eff}/\alpha + T(10)$',
        'error': '0.017%',
    },
    'neutron_delta': {
        'mass': 2.53,
        'formula': r'$\phi^2 - 12\alpha$',
        'error': '0.53%',
    },
}

# =============================================================================
# FORCE PARAMETERS
# =============================================================================

FORCES = {
    'strong': {
        'relative_strength': 1.0,
        'range_m': 1e-15,
        'carrier': 'gluon',
        'color': '#FF4500',
    },
    'electromagnetic': {
        'relative_strength': ALPHA,  # ~1/137
        'range_m': float('inf'),
        'carrier': 'photon',
        'color': '#FFD700',
    },
    'weak': {
        'relative_strength': 1e-6,
        'range_m': 1e-18,
        'carrier': 'W/Z bosons',
        'color': '#9370DB',
    },
    'gravity': {
        'relative_strength': 1e-40,
        'range_m': float('inf'),
        'carrier': 'graviton',
        'color': '#8B4513',
    },
}

# =============================================================================
# CAUSAL LOOP STEPS
# =============================================================================

CAUSAL_LOOP_STEPS = [
    {'name': 'TIME GATE', 'phase': 'temporal', 'description': 'Phase accumulator check'},
    {'name': 'DECAY', 'phase': 'existence', 'description': 'Entropy to unlocked voxels'},
    {'name': 'EXISTENCE', 'phase': 'existence', 'description': 'Evaporate or Genesis'},
    {'name': 'PROPAGATE', 'phase': 'propagation', 'description': 'Flux waves advance'},
    {'name': 'SUPERPOSE', 'phase': 'propagation', 'description': 'Vector field summation'},
    {'name': 'FIELDS', 'phase': 'propagation', 'description': 'Compute gradients, curl, div'},
    {'name': 'FORCES', 'phase': 'forces', 'description': 'Calculate all force types'},
    {'name': 'INTEGRATE', 'phase': 'forces', 'description': 'Forces to velocity updates'},
    {'name': 'MOVE', 'phase': 'motion', 'description': 'Particle position updates'},
    {'name': 'COLLIDE', 'phase': 'motion', 'description': 'Handle interactions'},
    {'name': 'TRANSMUTE', 'phase': 'motion', 'description': 'Polarity flip if stressed'},
    {'name': 'BIND', 'phase': 'motion', 'description': 'Lock stable structures'},
    {'name': 'INCREMENT', 'phase': 'temporal', 'description': 't ← t + 1'},
]

# =============================================================================
# VOXEL DATA STRUCTURE
# =============================================================================

VOXEL_STRUCTURE = {
    'identity': {
        'color': '#FF6B6B',
        'fields': ['position (x,y,z)', 'uuid', 'partner_uuid'],
    },
    'state': {
        'color': '#4ECDC4',
        'fields': ['state {-1,0,+1}', 'charge', 'shell_n'],
    },
    'energy': {
        'color': '#FFE66D',
        'fields': ['flux (Jx,Jy,Jz)', 'density |J|', 'frequency'],
    },
    'mechanics': {
        'color': '#95E1D3',
        'fields': ['force_accumulator', 'position_remainder', 'wave_velocity'],
    },
    'temporal': {
        'color': '#A8E6CF',
        'fields': ['phase'],
    },
    'stability': {
        'color': '#DCD6F7',
        'fields': ['is_locked'],
    },
}

# =============================================================================
# STANDARD MODEL LAYOUT
# =============================================================================

SM_LAYOUT = {
    # Quarks - 3 generations, 2 types each
    'quarks': [
        [('u', 'up', '+2/3'), ('c', 'charm', '+2/3'), ('t', 'top', '+2/3')],
        [('d', 'down', '-1/3'), ('s', 'strange', '-1/3'), ('b', 'bottom', '-1/3')],
    ],
    # Leptons - 3 generations, 2 types each
    'leptons': [
        [('e', 'electron', '-1'), ('μ', 'muon', '-1'), ('τ', 'tau', '-1')],
        [('νe', 'e-neutrino', '0'), ('νμ', 'μ-neutrino', '0'), ('ντ', 'τ-neutrino', '0')],
    ],
    # Gauge bosons
    'gauge': [
        ('γ', 'photon', '0'),
        ('g', 'gluon', '0'),
        ('W', 'W boson', '±1'),
        ('Z', 'Z boson', '0'),
    ],
    # Higgs
    'higgs': ('H', 'Higgs', '0'),
}

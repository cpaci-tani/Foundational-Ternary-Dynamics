"""
Monodromy and Connection Matrices of the Picard-Fuchs Equation
==============================================================

[CONJECTURE] -- Numerical computation exploring whether the monodromy
representation of the Picard-Fuchs ODE at c=4 connects to the FTD
master quadratic.

The Picard-Fuchs ODE:
    [(c-4)(c+4)c] Pi'' + [3c^2 - 16] Pi' + c Pi = 0

Standard form: Pi'' + p(c) Pi' + q(c) Pi = 0
    p(c) = (3c^2 - 16) / [c(c-4)(c+4)]
    q(c) = 1 / [(c-4)(c+4)]

Singular points: c = 0, 4, -4, infinity.

KEY FINDING: All three finite singular points have indicial exponents
rho = 0, 0 (double roots). This makes ALL monodromies unipotent:
    M_j = [[1, b_j], [0, 1]]  (in an appropriate basis)
The invariant content is in the NILPOTENT PART N_j = M_j - I, specifically
the off-diagonal entry b_j which encodes the logarithmic period.

Author: FTD verification suite
"""

import numpy as np
from scipy.integrate import solve_ivp
from scipy.special import gamma
import sys
import os

# Import FTD constants
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from constants import G_STAR, X_PLUS, X_MINUS, GAMMA_QUARTER, GAMMA_HALF


# =============================================================================
# PART 0: Local Exponents (Indicial Equation) at Each Singular Point
# =============================================================================

def indicial_analysis():
    """
    At each regular singular point c_j, the Frobenius method gives the
    indicial equation from the standard form:

        w^2 Pi'' + w [w p(w)] Pi' + [w^2 q(w)] Pi = 0

    where w = c - c_j, and p(w), q(w) are the coefficients in
    Pi'' + p(w) Pi' + q(w) Pi = 0.

    The indicial equation is: rho(rho-1) + p_0 rho + q_0 = 0
    where p_0 = lim_{w->0} w*p(w) and q_0 = lim_{w->0} w^2*q(w).
    """
    print("=" * 72)
    print("PART 0: INDICIAL ANALYSIS AT SINGULAR POINTS")
    print("=" * 72)

    # ODE: c(c-4)(c+4) Pi'' + (3c^2-16) Pi' + c Pi = 0
    # Standard form: Pi'' + P(c) Pi' + Q(c) Pi = 0
    #   P(c) = (3c^2-16) / [c(c-4)(c+4)]
    #   Q(c) = 1 / [(c-4)(c+4)]

    # --- At c = 0 ---
    # P(c) = (3c^2-16)/[c(c^2-16)]
    # Near c=0: P ~ -16/(-16c) = 1/c, so c*P -> 1 => p_0 = 1
    # Q(c) = 1/(c^2-16), near c=0: Q ~ -1/16, so c^2*Q -> 0 => q_0 = 0
    # Indicial: rho(rho-1) + rho = rho^2 = 0 => rho = 0, 0
    print("\nAt c = 0:")
    print("  P(c) ~ 1/c,  Q(c) ~ -1/16 (regular)")
    print("  p_0 = lim c*P(c) = 1,  q_0 = lim c^2*Q(c) = 0")
    print("  Indicial: rho^2 = 0  =>  rho = 0, 0  (LOGARITHMIC)")

    # --- At c = 4 ---
    # Let w = c - 4. P(c) in terms of w:
    # P = (3(w+4)^2-16) / [(w+4)*w*(w+8)]
    # Near w=0: P ~ 32/(4*w*8) = 32/(32w) = 1/w => w*P -> 1 => p_0 = 1
    # Q = 1/[w*(w+8)], near w=0: Q ~ 1/(8w) => w^2*Q -> 0 => q_0 = 0
    # Indicial: rho^2 = 0 => rho = 0, 0
    print("\nAt c = 4:")
    print("  P(w) ~ 1/w,  Q(w) ~ 1/(8w) (regular, but NOT 1/w^2)")
    print("  p_0 = 1,  q_0 = lim w^2 * Q(w) = lim w/8 = 0")
    print("  Indicial: rho^2 = 0  =>  rho = 0, 0  (LOGARITHMIC)")
    print("  NOTE: The 1/(8w) term in Q is sub-leading for Frobenius.")
    print("  It affects the RECURRENCE RELATIONS, not the indicial equation.")

    # --- At c = -4 ---
    # Let w = c + 4. P = (3(w-4)^2-16)/[(w-4)*w*(w-8)]
    # Near w=0: P ~ 32/((-4)*w*(-8)) = 32/(32w) = 1/w => p_0 = 1
    # Q = 1/[(w-8)*w], near w=0: Q ~ 1/(-8w) = -1/(8w) => q_0 = 0
    # Indicial: rho^2 = 0 => rho = 0, 0
    print("\nAt c = -4:")
    print("  Same structure: p_0 = 1, q_0 = 0")
    print("  Indicial: rho^2 = 0  =>  rho = 0, 0  (LOGARITHMIC)")

    # --- At c = infinity ---
    # Fuchs relation: sum of all exponents = (n-2) where n = number of singular points
    # With 4 singular points (0, 4, -4, inf): sum = 2
    # Finite sum = 0+0+0+0+0+0 = 0, so exponents at infinity sum to 2.
    print("\nAt c = infinity:")
    print("  Fuchs relation: total exponent sum = n-2 = 2")
    print("  Finite sum = 0, so infinity exponents sum to 2")
    print("  Detailed analysis gives rho = 1, 1 (LOGARITHMIC)")

    print("\n  CONCLUSION: All four singular points are LOGARITHMIC (double root).")
    print("  All monodromies are UNIPOTENT: M_j has eigenvalues (1, 1).")
    print("  The invariant content is in the NILPOTENT RESIDUE N_j = M_j - I.")
    print("  In the right basis, M_j = [[1, b_j], [0, 1]] where b_j = 2*pi*i * (log period).")


# =============================================================================
# ODE coefficients and integration system
# =============================================================================

def p_coeff(c):
    """p(c) = (3c^2 - 16) / [c(c-4)(c+4)]"""
    return (3 * c**2 - 16) / (c * (c - 4) * (c + 4))


def q_coeff(c):
    """q(c) = 1 / [(c-4)(c+4)]"""
    return 1.0 / ((c - 4) * (c + 4))


def ode_system_complex(t, Y, c_func, dc_dt_func):
    """
    ODE system for Pi'' + p(c)*Pi' + q(c)*Pi = 0 along a complex path c(t).

    State vector Y has 8 real components encoding two complex solutions:
        Y[0:2] = Re(Pi_1), Im(Pi_1)
        Y[2:4] = Re(dPi_1/dc), Im(dPi_1/dc)
        Y[4:6] = Re(Pi_2), Im(Pi_2)
        Y[6:8] = Re(dPi_2/dc), Im(dPi_2/dc)
    """
    c = c_func(t)
    dcdt = dc_dt_func(t)

    p = p_coeff(c)
    q = q_coeff(c)

    Pi1 = Y[0] + 1j * Y[1]
    dPi1_dc = Y[2] + 1j * Y[3]
    Pi2 = Y[4] + 1j * Y[5]
    dPi2_dc = Y[6] + 1j * Y[7]

    # ODE: d^2Pi/dc^2 = -p(c)*dPi/dc - q(c)*Pi
    d2Pi1_dc2 = -p * dPi1_dc - q * Pi1
    d2Pi2_dc2 = -p * dPi2_dc - q * Pi2

    # Convert to t-derivatives via chain rule
    dY = np.zeros(8)
    dPi1_dt = dPi1_dc * dcdt
    ddPi1dc_dt = d2Pi1_dc2 * dcdt
    dY[0] = dPi1_dt.real
    dY[1] = dPi1_dt.imag
    dY[2] = ddPi1dc_dt.real
    dY[3] = ddPi1dc_dt.imag

    dPi2_dt = dPi2_dc * dcdt
    ddPi2dc_dt = d2Pi2_dc2 * dcdt
    dY[4] = dPi2_dt.real
    dY[5] = dPi2_dt.imag
    dY[6] = ddPi2dc_dt.real
    dY[7] = ddPi2dc_dt.imag

    return dY


def integrate_along_path(c_path, dcdt_path, t_span=(0.0, 1.0), n_steps=10000):
    """
    Integrate the ODE system along a parameterized complex path.
    Returns the 2x2 transport matrix (columns = transported basis).
    """
    Y0 = np.array([1., 0., 0., 0., 0., 0., 1., 0.])

    sol = solve_ivp(
        ode_system_complex, t_span, Y0,
        args=(c_path, dcdt_path),
        method='DOP853', rtol=1e-13, atol=1e-15,
        max_step=(t_span[1] - t_span[0]) / n_steps
    )

    if not sol.success:
        return None

    Yf = sol.y[:, -1]
    M = np.array([
        [Yf[0] + 1j*Yf[1], Yf[4] + 1j*Yf[5]],
        [Yf[2] + 1j*Yf[3], Yf[6] + 1j*Yf[7]]
    ])
    return M


# =============================================================================
# PART 1: Monodromy Matrices at All Three Finite Singular Points
# =============================================================================

def compute_monodromy(center, radius, label, n_steps=10000):
    """
    Compute monodromy matrix by integrating around a circle centered at `center`.
    For unipotent monodromy, extract the nilpotent residue N = M - I.
    """
    print(f"\n{'=' * 72}")
    print(f"MONODROMY AT c = {center} (radius = {radius})")
    print(f"{'=' * 72}")

    r = radius

    def c_path(t):
        return center + r * np.exp(2j * np.pi * t)

    def dcdt_path(t):
        return r * 2j * np.pi * np.exp(2j * np.pi * t)

    M = integrate_along_path(c_path, dcdt_path, n_steps=n_steps)
    if M is None:
        print("  Integration FAILED")
        return None

    det_M = np.linalg.det(M)
    tr_M = np.trace(M)
    eigenvalues = np.linalg.eigvals(M)

    print(f"\n  Monodromy matrix M_{label}:")
    print(f"    M[0,0] = {M[0,0]:.15f}")
    print(f"    M[0,1] = {M[0,1]:.15f}")
    print(f"    M[1,0] = {M[1,0]:.15f}")
    print(f"    M[1,1] = {M[1,1]:.15f}")
    print(f"\n  det(M) = {det_M:.15f},  |det| = {abs(det_M):.15e}")
    print(f"  tr(M)  = {tr_M:.15f}")
    print(f"  Eigenvalues: {eigenvalues[0]:.12f}, {eigenvalues[1]:.12f}")

    # Nilpotent residue
    N = M - np.eye(2)
    print(f"\n  Nilpotent residue N = M - I:")
    print(f"    N[0,0] = {N[0,0]:.6e}")
    print(f"    N[0,1] = {N[0,1]:.15f}")
    print(f"    N[1,0] = {N[1,0]:.6e}")
    print(f"    N[1,1] = {N[1,1]:.6e}")

    # The key invariant: the upper-right entry of N (in the right basis)
    # For unipotent M, the trace of N^2 is basis-independent:
    # tr(N^2) = (M[0,0]-1)^2 + 2*M[0,1]*M[1,0] + (M[1,1]-1)^2
    trN2 = np.trace(N @ N)
    print(f"\n  tr(N^2) = {trN2:.15f}  (basis-independent)")

    # The logarithmic monodromy: log(M) = N - N^2/2 + N^3/3 - ...
    # For nearly unipotent M, log(M) ~ N
    # The (0,1) entry of log(M)/(2*pi*i) gives the logarithmic period ratio
    logM_01 = N[0, 1]  # leading-order approximation
    period_ratio = logM_01 / (2j * np.pi)
    print(f"\n  Logarithmic period (N[0,1] / (2*pi*i)):")
    print(f"    = {period_ratio:.15f}")
    print(f"    |period_ratio| = {abs(period_ratio):.15f}")

    return M, N, eigenvalues, period_ratio


# =============================================================================
# PART 2: Connection Matrix from c=4 to c=0
# =============================================================================

def compute_connection_matrix(n_steps=20000):
    """
    Transport solutions from near c=4 to near c=0 along the real axis
    (with small imaginary displacement to stay off the real axis).
    """
    print(f"\n{'=' * 72}")
    print(f"CONNECTION MATRIX: c = 4.5 -> c = 0.5")
    print(f"{'=' * 72}")

    c_start = 4.5 + 0.01j
    c_end = 0.5 + 0.01j

    def c_path(t):
        return c_start + t * (c_end - c_start)

    def dcdt_path(t):
        return c_end - c_start

    C = integrate_along_path(c_path, dcdt_path, n_steps=n_steps)
    if C is None:
        print("  Integration FAILED")
        return None

    det_C = np.linalg.det(C)

    print(f"  Path: {c_start} -> {c_end} (slightly above real axis)")
    print(f"\n  Connection matrix C:")
    print(f"    C[0,0] = {C[0,0]:.15f}")
    print(f"    C[0,1] = {C[0,1]:.15f}")
    print(f"    C[1,0] = {C[1,0]:.15f}")
    print(f"    C[1,1] = {C[1,1]:.15f}")
    print(f"\n  det(C) = {det_C:.15f}")
    print(f"  |det(C)| = {abs(det_C):.15f}")

    return C, det_C


# =============================================================================
# PART 3: High-Precision Monodromy via mpmath
# =============================================================================

def compute_monodromy_mpmath(center, radius, label, npts=4000):
    """High-precision monodromy using mpmath RK4."""
    try:
        import mpmath
    except ImportError:
        print(f"\n  mpmath not available, skipping high-precision for c={center}")
        return None

    print(f"\n{'=' * 72}")
    print(f"HIGH-PRECISION MONODROMY AT c = {center} (mpmath, 50 digits)")
    print(f"{'=' * 72}")

    mpmath.mp.dps = 50
    r = mpmath.mpf(str(radius))
    ctr = mpmath.mpf(str(center))
    pi = mpmath.pi

    def p_mp(c):
        return (3 * c**2 - 16) / (c * (c - 4) * (c + 4))

    def q_mp(c):
        return mpmath.mpf(1) / ((c - 4) * (c + 4))

    def rk4_step(state, t, dt):
        def f(t_val, s):
            c = ctr + r * mpmath.exp(2j * pi * t_val)
            dcdt = r * 2j * pi * mpmath.exp(2j * pi * t_val)
            pp = p_mp(c)
            qq = q_mp(c)
            return [
                s[1] * dcdt,
                (-pp * s[1] - qq * s[0]) * dcdt,
                s[3] * dcdt,
                (-pp * s[3] - qq * s[2]) * dcdt,
            ]

        k1 = f(t, state)
        s2 = [state[i] + dt/2 * k1[i] for i in range(4)]
        k2 = f(t + dt/2, s2)
        s3 = [state[i] + dt/2 * k2[i] for i in range(4)]
        k3 = f(t + dt/2, s3)
        s4 = [state[i] + dt * k3[i] for i in range(4)]
        k4 = f(t + dt, s4)
        return [state[i] + dt/6 * (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]) for i in range(4)]

    state = [mpmath.mpf(1), mpmath.mpf(0), mpmath.mpf(0), mpmath.mpf(1)]
    dt = mpmath.mpf(1) / npts

    for step in range(npts):
        t = step * dt
        state = rk4_step(state, t, dt)

    Pi1_f, dPi1_f, Pi2_f, dPi2_f = state
    det_M = Pi1_f * dPi2_f - Pi2_f * dPi1_f
    tr_M = Pi1_f + dPi2_f

    # Nilpotent residue entries
    N00 = Pi1_f - 1
    N01 = Pi2_f
    N10 = dPi1_f
    N11 = dPi2_f - 1

    print(f"  {npts} RK4 steps at {mpmath.mp.dps}-digit precision")
    print(f"\n  M[0,0] = {mpmath.nstr(Pi1_f, 20)}")
    print(f"  M[0,1] = {mpmath.nstr(Pi2_f, 20)}")
    print(f"  M[1,0] = {mpmath.nstr(dPi1_f, 20)}")
    print(f"  M[1,1] = {mpmath.nstr(dPi2_f, 20)}")
    print(f"\n  det(M) = {mpmath.nstr(det_M, 20)}")
    print(f"  tr(M)  = {mpmath.nstr(tr_M, 20)}")

    print(f"\n  Nilpotent residue N = M - I:")
    print(f"    N[0,0] = {mpmath.nstr(N00, 10)}")
    print(f"    N[0,1] = {mpmath.nstr(N01, 20)}")
    print(f"    N[1,0] = {mpmath.nstr(N10, 10)}")
    print(f"    N[1,1] = {mpmath.nstr(N11, 10)}")

    # Key invariant: tr(N^2)
    trN2 = N00**2 + 2*N01*N10 + N11**2
    print(f"\n  tr(N^2) = {mpmath.nstr(trN2, 20)}")

    # Period ratio
    period = N01 / (2j * pi)
    print(f"  Period ratio N[0,1]/(2*pi*i) = {mpmath.nstr(period, 20)}")
    print(f"  |period ratio| = {mpmath.nstr(abs(period), 20)}")

    return {
        'M': [[Pi1_f, Pi2_f], [dPi1_f, dPi2_f]],
        'N01': N01, 'N10': N10, 'N00': N00, 'N11': N11,
        'det': det_M, 'tr': tr_M, 'trN2': trN2,
        'period': period,
    }


# =============================================================================
# PART 4: Convergence Study
# =============================================================================

def convergence_check():
    """Verify numerical convergence of monodromy trace and nilpotent entries."""
    print(f"\n{'=' * 72}")
    print(f"CONVERGENCE CHECK")
    print(f"{'=' * 72}")

    print(f"\n  --- Monodromy at c=4, varying radius and resolution ---")
    print(f"  {'r':>6s}  {'steps':>7s}  {'Re(tr)':>20s}  {'N[0,1]':>35s}  {'|N[0,1]|':>15s}  {'|det-1|':>12s}")
    print(f"  {'-'*6}  {'-'*7}  {'-'*20}  {'-'*35}  {'-'*15}  {'-'*12}")

    for r in [0.5, 0.3, 0.1]:
        for ns in [5000, 10000, 20000]:
            def c_path(t, _r=r):
                return 4.0 + _r * np.exp(2j * np.pi * t)
            def dcdt_path(t, _r=r):
                return _r * 2j * np.pi * np.exp(2j * np.pi * t)

            M = integrate_along_path(c_path, dcdt_path, n_steps=ns)
            if M is not None:
                tr_val = np.trace(M)
                det_val = np.linalg.det(M)
                N01 = M[0, 1]
                print(f"  {r:6.2f}  {ns:7d}  {tr_val.real:20.15f}  {N01.real:+17.12f}{N01.imag:+17.12f}j  {abs(N01):15.12f}  {abs(abs(det_val)-1):12.2e}")


# =============================================================================
# PART 5: Deep Analysis
# =============================================================================

def deep_analysis(results_c4, results_c0, results_cm4, conn_result, mp_c4):
    """
    Analyze all computed quantities for connections to G* and
    the master quadratic.
    """
    print(f"\n{'=' * 72}")
    print(f"DEEP ANALYSIS: CONNECTIONS TO G* AND MASTER QUADRATIC")
    print(f"{'=' * 72}")

    print(f"\n  FTD Constants:")
    print(f"    G* = {G_STAR:.15f}")
    print(f"    G*^2 = {G_STAR**2:.15f}")
    print(f"    16*G*^2 = {16*G_STAR**2:.15f} (master quadratic trace)")
    print(f"    16*G*^3 = {16*G_STAR**3:.15f} (master quadratic det)")
    print(f"    x+ = {X_PLUS:.15f}")
    print(f"    x- = {X_MINUS:.15f}")
    print(f"    G*^2/(2*pi) = {G_STAR**2/(2*np.pi):.15f}")
    print(f"    Gamma(1/4) = {GAMMA_QUARTER:.15f}")
    print(f"    Gamma(1/4)^2/(2*pi) = {GAMMA_QUARTER**2/(2*np.pi):.15f}")

    # Extract nilpotent residues
    if results_c4:
        M4, N4, ev4, pr4 = results_c4
        N01_c4 = N4[0, 1]
        N10_c4 = N4[1, 0]
        trN2_c4 = np.trace(N4 @ N4)

        print(f"\n  --- Nilpotent residue at c=4 ---")
        print(f"  N[0,1] = {N01_c4:.15f}")
        print(f"  |N[0,1]| = {abs(N01_c4):.15f}")
        print(f"  N[1,0] = {N10_c4:.15e}")
        print(f"  tr(N^2) = {trN2_c4:.15f}")

    if results_c0:
        M0, N0, ev0, pr0 = results_c0
        N01_c0 = N0[0, 1]
        N10_c0 = N0[1, 0]
        trN2_c0 = np.trace(N0 @ N0)

        print(f"\n  --- Nilpotent residue at c=0 ---")
        print(f"  N[0,1] = {N01_c0:.15f}")
        print(f"  |N[0,1]| = {abs(N01_c0):.15f}")
        print(f"  N[1,0] = {N10_c0:.15e}")
        print(f"  tr(N^2) = {trN2_c0:.15f}")

    if results_cm4:
        Mm4, Nm4, evm4, prm4 = results_cm4
        N01_cm4 = Nm4[0, 1]
        trN2_cm4 = np.trace(Nm4 @ Nm4)

        print(f"\n  --- Nilpotent residue at c=-4 ---")
        print(f"  N[0,1] = {N01_cm4:.15f}")
        print(f"  |N[0,1]| = {abs(N01_cm4):.15f}")
        print(f"  tr(N^2) = {trN2_cm4:.15f}")

    # Key invariant: N[0,1] is basis-dependent, but tr(N^2) is not
    # Also, for unipotent M, the RATIO of N[0,1] values between
    # different singular points is basis-independent
    if results_c4 and results_c0:
        print(f"\n  --- Ratios of nilpotent residues ---")
        if abs(N01_c0) > 1e-10 and abs(N01_c4) > 1e-10:
            ratio_01 = N01_c4 / N01_c0
            print(f"  N[0,1](c=4) / N[0,1](c=0) = {ratio_01:.15f}")
            print(f"  |ratio| = {abs(ratio_01):.15f}")
        if abs(trN2_c0) > 1e-10 and abs(trN2_c4) > 1e-10:
            ratio_trN2 = trN2_c4 / trN2_c0
            print(f"  tr(N^2)(c=4) / tr(N^2)(c=0) = {ratio_trN2:.15f}")
            print(f"  |ratio| = {abs(ratio_trN2):.15f}")

    if results_c4 and results_cm4:
        if abs(N01_cm4) > 1e-10 and abs(N01_c4) > 1e-10:
            ratio_pm = N01_c4 / N01_cm4
            print(f"  N[0,1](c=4) / N[0,1](c=-4) = {ratio_pm:.15f}")
            print(f"  |ratio| = {abs(ratio_pm):.15f}")

    # The monodromy representation has a GLOBAL constraint:
    # M_0 * M_4 * M_{-4} * M_inf = I (up to conjugation and path ordering)
    # This means the nilpotent parts satisfy N_0 + N_4 + N_{-4} + N_inf ~ 0
    # (to leading order in the nilpotent expansion)
    if results_c4 and results_c0 and results_cm4:
        print(f"\n  --- Global constraint: sum of nilpotent residues ---")
        N_sum = N4 + N0 + Nm4
        print(f"  N_4 + N_0 + N_(-4):")
        print(f"    [0,0] = {N_sum[0,0]:.6e}")
        print(f"    [0,1] = {N_sum[0,1]:.15f}")
        print(f"    [1,0] = {N_sum[1,0]:.6e}")
        print(f"    [1,1] = {N_sum[1,1]:.6e}")
        print(f"  (This should approximately equal -N_inf)")

    # Connection matrix analysis
    if conn_result:
        C, det_C = conn_result
        print(f"\n  --- Connection matrix entries vs G* ---")
        print(f"  |det(C)| = {abs(det_C):.15f}")
        print(f"  G*^2/(2*pi) = {G_STAR**2/(2*np.pi):.15f}")
        ratio_det = abs(det_C) / (G_STAR**2 / (2*np.pi))
        print(f"  |det(C)| / (G*^2/(2*pi)) = {ratio_det:.15f}")

        # Check individual entries
        for i in range(2):
            for j in range(2):
                v = C[i, j]
                print(f"  C[{i},{j}] = {v:.10f},  |C|={abs(v):.10f}, "
                      f"|C|/G*={abs(v)/G_STAR:.10f}")

    # Now check: the N[0,1] entries encode the LOG PERIODS
    # In the Picard-Fuchs theory, the periods satisfy:
    #   omega_1(c) and omega_2(c) = omega_1(c) * tau(c) + ...
    # where tau is the period ratio (modular parameter)
    # The monodromy sends tau -> tau + b (for unipotent M with entry b)
    # This b is related to 2*pi*i * (something)
    if results_c4:
        b_c4 = N01_c4
        print(f"\n  --- Period structure at c=4 ---")
        print(f"  Monodromy shift b = N[0,1] = {b_c4:.15f}")
        print(f"  b / (2*pi*i) = {b_c4/(2j*np.pi):.15f}")
        print(f"  |b / (2*pi*i)| = {abs(b_c4/(2j*np.pi)):.15f}")
        print(f"  |b| / (2*pi) = {abs(b_c4)/(2*np.pi):.15f}")
        val = abs(b_c4) / (2*np.pi)
        print(f"\n  Compare |b|/(2*pi) = {val:.15f} with:")
        print(f"    G*^2/(2*pi) = {G_STAR**2/(2*np.pi):.15f}")
        print(f"    G*/pi = {G_STAR/np.pi:.15f}")
        print(f"    1/sqrt(8) = {1/np.sqrt(8):.15f}")
        print(f"    Gamma(1/4)^2/(4*pi^2) = {GAMMA_QUARTER**2/(4*np.pi**2):.15f}")

    # High precision results
    if mp_c4:
        import mpmath
        print(f"\n  --- High-precision confirmation ---")
        print(f"  N[0,1] (mpmath) = {mpmath.nstr(mp_c4['N01'], 25)}")
        print(f"  |N[0,1]| = {mpmath.nstr(abs(mp_c4['N01']), 25)}")
        print(f"  tr(N^2) = {mpmath.nstr(mp_c4['trN2'], 25)}")
        b_hp = mp_c4['N01']
        period_hp = b_hp / (2j * mpmath.pi)
        print(f"  b/(2*pi*i) = {mpmath.nstr(period_hp, 25)}")
        print(f"  |b/(2*pi*i)| = {mpmath.nstr(abs(period_hp), 25)}")
        print(f"  |b|/(2*pi) = {mpmath.nstr(abs(b_hp)/(2*mpmath.pi), 25)}")

        # Check against G*
        G_mp = mpmath.mpf(str(G_STAR))
        val_hp = abs(b_hp) / (2 * mpmath.pi)
        print(f"\n  |b|/(2*pi) / G* = {mpmath.nstr(val_hp / G_mp, 15)}")
        print(f"  |b|/(2*pi) / (G*^2/(2*pi)) = {mpmath.nstr(val_hp / (G_mp**2/(2*mpmath.pi)), 15)}")


# =============================================================================
# MAIN
# =============================================================================

def main():
    print("=" * 72)
    print("PICARD-FUCHS MONODROMY AND CONNECTION MATRICES")
    print("ODE: c(c-4)(c+4) Pi'' + (3c^2-16) Pi' + c Pi = 0")
    print("=" * 72)

    # Part 0: Indicial analysis
    indicial_analysis()

    # Part 1: Monodromy at all three finite singular points
    # Use radii that keep the contour away from other singularities
    # c=4: nearest other singularity is c=0 (distance 4), use r=0.5
    # c=0: nearest other is c=4 or c=-4 (distance 4), use r=0.5
    # c=-4: nearest other is c=0 (distance 4), use r=0.5
    res_c4 = compute_monodromy(center=4.0, radius=0.5, label="4", n_steps=10000)
    res_c0 = compute_monodromy(center=0.0, radius=0.5, label="0", n_steps=10000)
    # For c=-4, the contour c=-4+0.5*exp(2pi*i*t) passes near c=0 at closest
    # approach of |-4+0.5 - 0| = 3.5, so we are safe
    res_cm4 = compute_monodromy(center=-4.0, radius=0.5, label="-4", n_steps=10000)

    # Part 2: Connection matrix
    conn_result = compute_connection_matrix(n_steps=20000)

    # Part 3: High-precision at c=4
    mp_c4 = compute_monodromy_mpmath(center=4, radius=0.5, label="4", npts=4000)

    # Part 4: Convergence
    convergence_check()

    # Part 5: Deep analysis
    deep_analysis(res_c4, res_c0, res_cm4, conn_result, mp_c4)

    # =============================================================================
    # FINAL SUMMARY
    # =============================================================================
    print(f"\n{'=' * 72}")
    print(f"FINAL SUMMARY")
    print(f"{'=' * 72}")

    print(f"""
  Picard-Fuchs ODE: c(c-4)(c+4) Pi'' + (3c^2-16) Pi' + c Pi = 0
  Singular points: c = 0, 4, -4, infinity

  INDICIAL EXPONENTS:
    c = 0:    rho = 0, 0  (logarithmic)
    c = 4:    rho = 0, 0  (logarithmic)
    c = -4:   rho = 0, 0  (logarithmic)
    c = inf:  rho = 1, 1  (logarithmic, from Fuchs relation)

    All singularities are APPARENT or LOGARITHMIC.
    Every monodromy matrix is UNIPOTENT: eigenvalues = (1, 1).

  MONODROMY MATRICES:
    M_j = I + N_j  where N_j is nilpotent (N_j^2 ~ 0 in suitable basis)
    The invariant content is tr(N_j^2) and the period shifts N_j[0,1].

  CHARACTERISTIC POLYNOMIAL:
    At each singular point: lambda^2 - 2*lambda + 1 = (lambda - 1)^2 = 0

  MASTER QUADRATIC COMPARISON:
    x^2 - 16*G*^2 * x + 16*G*^3 = 0

    The monodromy characteristic polynomial (lambda-1)^2 = 0 does NOT
    directly match the master quadratic. The master quadratic lives at
    a DIFFERENT level -- it governs the COUPLING CONSTANTS, not the
    period matrices.

    The monodromy representation is a faithful representation of
    pi_1(P^1 - {{0, 4, -4, inf}}) into SL(2,C), with all generators
    unipotent. The connection to the master quadratic, if any, must
    come through the NILPOTENT RESIDUES or the CONNECTION MATRICES,
    not through the eigenvalues.
""")

    # Print the key numerical values for the nilpotent residues
    if res_c4:
        N01 = res_c4[1][0, 1]
        print(f"  KEY NUMERICAL RESULTS:")
        print(f"    N[0,1] at c=4:  {N01:.15f}")
        print(f"    |N[0,1]|/(2*pi) at c=4:  {abs(N01)/(2*np.pi):.15f}")
    if res_c0:
        N01_0 = res_c0[1][0, 1]
        print(f"    N[0,1] at c=0:  {N01_0:.15f}")
        print(f"    |N[0,1]|/(2*pi) at c=0:  {abs(N01_0)/(2*np.pi):.15f}")
    if res_cm4:
        N01_m4 = res_cm4[1][0, 1]
        print(f"    N[0,1] at c=-4: {N01_m4:.15f}")
        print(f"    |N[0,1]|/(2*pi) at c=-4: {abs(N01_m4)/(2*np.pi):.15f}")

    if conn_result:
        C, det_C = conn_result
        print(f"\n    Connection matrix determinant: {det_C:.15f}")
        print(f"    |det(C)| = {abs(det_C):.15f}")

    print(f"\n  G*^2/(2*pi) = {G_STAR**2/(2*np.pi):.15f}")
    print(f"  G* = {G_STAR:.15f}")
    print()


if __name__ == "__main__":
    main()

"""
DECISIVE COMPUTATION v2: The 18-point stencil mismatch as the Watson mechanism
==============================================================================

KEY FINDING FROM v1:
- With 6-point Laplacian for BOTH M_vec and div: hat_k^2 cancels exactly, giving G=1
- With the Gauss constraint: G -> 0 as lambda_G -> infinity
- Neither produces W_3

NEW INSIGHT:
The FTD engine uses an 18-point isotropic stencil for the VECTOR wave equation
(lagrangian.h lines 79-82) but the DIVERGENCE operator uses 6-point differences
(it's a sum of forward/backward differences along axes only).

The stencil mismatch means hat_k^2 does NOT cancel!

Let me get the 18-point stencil correct.
"""
import numpy as np
from scipy.special import gamma

# Constants
VARPI = 2.622057554292119810
M_GAUSS = 0.8346268416740731
G_STAR = 2 * np.sqrt(VARPI * M_GAUSS)
W3_exact = gamma(0.25)**4 / (4 * np.pi**3)

print(f"G* = {G_STAR:.10f}")
print(f"W_3 = {W3_exact:.10f}")
print(f"G*^2/(2pi) = {G_STAR**2/(2*np.pi):.10f}")
print()

# ============================================================================
# The 18-point isotropic Laplacian
# ============================================================================
# From lagrangian.h:
#   L f(v) = (1/3) sum_{6 face} [f(n)-f(v)] + (1/6) sum_{12 edge} [f(n)-f(v)]
#
# The 6 face neighbors: +/-e_1, +/-e_2, +/-e_3
# The 12 edge neighbors: +/-e_i +/- e_j for i<j (all combinations)
#
# In Fourier space:
# L(k) = (1/3) [sum_i 2cos(k_i) - 6] + (1/6) [sum_{i<j} (2cos(k_i+k_j) + 2cos(k_i-k_j)) - 12]
#
# Using: cos(a+b)+cos(a-b) = 2cos(a)cos(b)
# = (2/3) sum_i [cos(k_i) - 1] + (2/3) sum_{i<j} [cos(k_i)cos(k_j) - 1]
# = -(2/3) sum_i [1-cos(k_i)] - (2/3) sum_{i<j} [1-cos(k_i)cos(k_j)]
#
# So hat_k^2_18 = -L(k) = (2/3) sum_i [1-cos(k_i)] + (2/3) sum_{i<j} [1-cos(k_i)cos(k_j)]

def hat_k2_6pt(k):
    """Standard 6-point: sum_mu 2(1-cos(k_mu))"""
    return sum(2*(1-np.cos(ki)) for ki in k)

def hat_k2_18pt(k):
    """18-point isotropic stencil.
    From the code: (1/3) face + (1/6) edge
    = (2/3) sum_i (1-cos ki) + (2/3) sum_{i<j} (1-cos(ki)*cos(kj))
    """
    c = [np.cos(ki) for ki in k]
    face = sum(1-ci for ci in c)
    edge = sum(1 - c[i]*c[j] for i in range(3) for j in range(i+1, 3))
    return (2/3) * face + (2/3) * edge

# Verify at small k
k_test = np.array([0.01, 0.02, 0.03])
k2 = sum(k_test**2)
print("Small-k verification:")
print(f"  |k|^2      = {k2:.10f}")
print(f"  6-point    = {hat_k2_6pt(k_test):.10f}")
print(f"  18-point   = {hat_k2_18pt(k_test):.10f}")
print(f"  Ratio 18/6 = {hat_k2_18pt(k_test)/hat_k2_6pt(k_test):.10f}")
print()

# At large k (corner of BZ)
k_corner = np.array([np.pi, np.pi, np.pi])
print("BZ corner (pi,pi,pi):")
print(f"  6-point    = {hat_k2_6pt(k_corner):.10f}")
print(f"  18-point   = {hat_k2_18pt(k_corner):.10f}")
print(f"  Ratio 18/6 = {hat_k2_18pt(k_corner)/hat_k2_6pt(k_corner):.10f}")
print()

# ============================================================================
# COMPUTATION: The mixed-stencil propagator
# ============================================================================
# If M_vec uses 18-point stencil but divergence uses 6-point forward differences:
#   G_charge(k) = hat_k^2_6(k) / hat_k^2_18(k)
#
# G_charge(0,0) = (1/(2pi)^3) int dk hat_k^2_6/hat_k^2_18

print("="*80)
print("COMPUTATION A: Mixed-stencil propagator hat_k^2_6 / hat_k^2_18")
print("="*80)

def compute_ratio_integral(L):
    """Compute sum_k hat_k^2_6(k) / hat_k^2_18(k) on L^3 lattice"""
    total = 0.0
    for n1 in range(L):
        for n2 in range(L):
            for n3 in range(L):
                k = np.array([2*np.pi*n1/L, 2*np.pi*n2/L, 2*np.pi*n3/L])
                num = hat_k2_6pt(k)
                den = hat_k2_18pt(k)
                if den < 1e-12:
                    continue
                total += num / den
    return total / L**3

for L in [8, 16, 32, 64]:
    G = compute_ratio_integral(L)
    print(f"  L={L:3d}: hat_k2_6/hat_k2_18 = {G:.10f}")

print(f"\n  W_3 = {W3_exact:.10f}")
print(f"  Ratio to W_3: {compute_ratio_integral(64)/W3_exact:.10f}")

print()
print("="*80)
print("COMPUTATION B: Scalar Green's function with 18-point stencil")
print("="*80)

def compute_G_scalar_18(L):
    total = 0.0
    for n1 in range(L):
        for n2 in range(L):
            for n3 in range(L):
                k = np.array([2*np.pi*n1/L, 2*np.pi*n2/L, 2*np.pi*n3/L])
                den = hat_k2_18pt(k)
                if den < 1e-12:
                    continue
                total += 1.0 / den
    return total / L**3

for L in [8, 16, 32, 64]:
    G = compute_G_scalar_18(L)
    print(f"  L={L:3d}: 1/hat_k2_18 = {G:.10f}")

# Watson integral with 6-point
def compute_W3_lattice(L):
    total = 0.0
    for n1 in range(L):
        for n2 in range(L):
            for n3 in range(L):
                k = np.array([2*np.pi*n1/L, 2*np.pi*n2/L, 2*np.pi*n3/L])
                den = hat_k2_6pt(k)
                if den < 1e-12:
                    continue
                total += 1.0 / den
    return total / L**3

print(f"\n  6-point W_3 (L=64) = {compute_W3_lattice(64):.10f}")
print(f"  Exact W_3 = {W3_exact:.10f}")

print()
print("="*80)
print("COMPUTATION C: Does the engine's Gauss projection change things?")
print("="*80)
print()
print("The engine's gauss_project() phase (render_bridge.h line 199)")
print("implements: solve Poisson equation Delta phi = div(J) - rho,")
print("then J_new = J - grad(phi). This is Helmholtz projection.")
print()
print("After Gauss projection, J is TRANSVERSE: div(J)=rho exactly.")
print("The coupling term g_c*s*div(J) = g_c*s*rho is now trivial.")
print()
print("But the FORCE on particles comes from the Coulomb potential phi_C,")
print("which is solved by a SEPARATE Poisson solve:")
print("  -Delta phi_C = rho")
print("  F = -alpha * grad(phi_C)")
print()
print("The Coulomb potential phi_C is NOT the same as the Gauss projection")
print("potential phi. They solve DIFFERENT Poisson equations.")
print()

print("="*80)
print("COMPUTATION D: The CORRECT path integral with both J and phi_C")
print("="*80)
print()
print("The full FTD dynamics involves TWO separate computations:")
print()
print("1. The wave equation for J (with 18-point stencil):")
print("   Delta_t^2 J = c^2 * L_18 J + g_c * grad(s)")
print()
print("2. The Poisson equation for phi_C:")
print("   -L_6 phi_C = rho    (6-point Laplacian!)")
print("   F = -alpha * s * grad(phi_C)")
print()
print("In the path integral, phi_C appears as an AUXILIARY field.")
print("The J field propagates dynamically. phi_C is solved instantaneously")
print("at each tick (no dynamics). This is EXACTLY the structure of")
print("electrostatics in temporal/Coulomb gauge.")
print()
print("The effective s-s interaction from phi_C is:")
print("  V(x,y) = alpha * G_6pt(x-y)")
print("where G_6pt = (-L_6)^{-1} is the 6-point scalar Green's function.")
print()
print("G_6pt(0) = W_3 (Watson integral of the 3D cubic lattice).")
print()
print("So W_3 enters the physics through the INSTANTANEOUS Coulomb")
print("interaction, not through the retarded J-field propagation.")
print()

# ============================================================================
# COMPUTATION E: The self-consistent gap equation
# ============================================================================
print("="*80)
print("COMPUTATION E: Self-consistent gap equation from lattice mean-field theory")
print("="*80)
print()
print("On the lattice, the self-energy of a charge at site x=0 is:")
print("  Sigma = g_c^2 * G(0,0) = g_c^2 * W_3")
print()
print("In the self-consistent mean-field theory, the renormalized coupling")
print("alpha_R = g_c^2/(4*pi) must satisfy a gap equation.")
print()
print("The bare coupling g_c from the lattice action and the physical")
print("alpha = 1/137.036 are related by the self-energy renormalization.")
print()
print("KEY QUESTION: What is the gap equation that uses n_DOF = 16?")
print()
print("From the master quadratic: x^2 = 16*G*^2*(x - G*)")
print("Rewrite: x = 16*G*^2 - 16*G*^3/x")
print("       = 16*G*^2*(1 - G*/x)")
print()
print("Since W_3 = G*^2/(2*pi):")
print("  x = 32*pi*W_3*(1 - G*/x)")
print()
print("If x = 1/alpha, this is:")
print("  1/alpha = 32*pi*W_3*(1 - alpha*G*)")
print()
print("This is a SELF-CONSISTENCY equation for alpha in terms of the")
print("lattice self-energy W_3 and the coupling G*.")
print()

# Let's verify
alpha = 1/137.0361714582
x_plus = 1/alpha
print(f"  x_+ = 1/alpha = {x_plus:.10f}")
print(f"  32*pi*W_3 = {32*np.pi*W3_exact:.10f}")
print(f"  G* = {G_STAR:.10f}")
print(f"  alpha*G* = {alpha*G_STAR:.10f}")
print(f"  1 - alpha*G* = {1-alpha*G_STAR:.10f}")
print(f"  32*pi*W_3*(1-alpha*G*) = {32*np.pi*W3_exact*(1-alpha*G_STAR):.10f}")
print()

# The other root
x_minus = 3.0239639163
print(f"  x_- = N_c = {x_minus:.10f}")
print(f"  32*pi*W_3*(1-x_minus_inv*G*) with x_minus_inv = 1/x_minus:")
inv_xm = 1/x_minus
print(f"  1/x_minus = {inv_xm:.10f}")
print(f"  (1/x_minus)*G* = {inv_xm*G_STAR:.10f}")
print(f"  1 - G*/x_minus = {1-G_STAR/x_minus:.10f}")
print(f"  32*pi*W_3*(1-G*/x_minus) = {32*np.pi*W3_exact*(1-G_STAR/x_minus):.10f}")
print()

print("="*80)
print("COMPUTATION F: What generates the factor 16 = n_DOF?")
print("="*80)
print()
print("The gap equation x = 32*pi*W_3*(1 - G*/x) uses 32*pi*W_3 = 16*G*^2.")
print("The 32*pi comes from 2 * 16.")
print()
print("In lattice QED, the one-loop self-energy of a charge has the form:")
print("  Sigma = n * g^2 * W_3 / (4*pi)")
print("where n counts the internal degrees of freedom in the loop.")
print()
print("The master quadratic requires n * g^2 / (4*pi) to give the right")
print("coefficient. With g_c = alpha^{1/2} * (4*pi)^{1/2}:")
print("  n * g_c^2 / (4*pi) = n * alpha")
print()
print("For the self-consistent equation:")
print("  1/alpha = n * W_3 * (normalization)")
print()
print("The factor 16 represents the number of independent DoF that")
print("contribute to the vacuum polarization loop.")
print()

# ============================================================================
# COMPUTATION G: The DECISIVE test - Lagrange multiplier with 18-pt stencil
# ============================================================================
print("="*80)
print("COMPUTATION G: Lagrange multiplier propagator with 18-pt stencil")
print("="*80)
print()
print("If M_vec uses the 18-point stencil, the Lagrange multiplier")
print("(Coulomb potential) acquires a NONTRIVIAL propagator.")
print()
print("Redo the calculation from Computation 7 with 18-point M_vec:")
print()
print("  S(k) = (1/2) hat_k2_18 |hat_J|^2 + eta^* d_mu^* hat_J_mu - hat_phi^* hat_rho")
print()
print("Complete the square:")
print("  hat_J_mu^opt = -d_mu * eta / hat_k2_18")
print()
print("  S_eff = -(1/2) |eta|^2 * hat_k2_6 / hat_k2_18 - hat_phi^* hat_rho")
print()
print("Define R(k) = hat_k2_6(k) / hat_k2_18(k). Then:")
print("  S_eff = -(1/2) R(k) |hat_phi - g_c hat_s|^2 - hat_phi^* hat_rho")
print()
print("Now integrate out phi (if we treat it as dynamical):")
print("  delta S/delta phi^* = -R(k) (hat_phi - g_c hat_s) - hat_rho = 0")
print("  hat_phi = g_c hat_s - hat_rho / R(k)")
print()
print("The effective s-s interaction from the phi-mediated exchange is:")
print("  S_eff[s] = -(g_c^2/2) R(k) |hat_s|^2 - (1/2) |hat_rho|^2/R(k)")
print()
print("Wait -- this is still not right because rho depends on s.")
print("Let me set rho = s (single charge):")
print()
print("  S_eff[s] = -(g_c^2/2) R(k) |hat_s|^2 + g_c R(k) hat_phi^* hat_s")
print("             - (1/2) R(k) |hat_phi|^2 - hat_phi^* hat_s")
print()

# Actually, let me just compute R(k) = hat_k2_6/hat_k2_18 averaged over BZ
print("The key ratio R(k) = hat_k2_6(k)/hat_k2_18(k) averaged over BZ:")
print()

# Already computed above. Let me also look at the profile of R(k)
print("Profile of R(k) along high-symmetry directions:")
print()
N = 100
print("  Along (k,0,0):")
for i in [0, 10, 25, 50, 75, 100]:
    kx = np.pi * i / 100
    k = np.array([kx, 0, 0])
    r6 = hat_k2_6pt(k)
    r18 = hat_k2_18pt(k)
    if r18 > 1e-12:
        print(f"    k/pi = {i/100:.2f}:  6pt = {r6:.6f}  18pt = {r18:.6f}  R = {r6/r18:.6f}")
    else:
        print(f"    k/pi = {i/100:.2f}:  zero mode")

print()
print("  Along (k,k,k):")
for i in [0, 10, 25, 50, 75, 100]:
    kx = np.pi * i / 100
    k = np.array([kx, kx, kx])
    r6 = hat_k2_6pt(k)
    r18 = hat_k2_18pt(k)
    if r18 > 1e-12:
        print(f"    k/pi = {i/100:.2f}:  6pt = {r6:.6f}  18pt = {r18:.6f}  R = {r6/r18:.6f}")
    else:
        print(f"    k/pi = {i/100:.2f}:  zero mode")

print()

# ============================================================================
# COMPUTATION H: The phi propagator with 18-point stencil (CORRECT)
# ============================================================================
print("="*80)
print("COMPUTATION H: CORRECT Lagrange multiplier propagator")
print("="*80)
print()
print("The partition function with Lagrange multiplier phi:")
print()
print("  Z = int DJ D(phi) exp[-S[J, phi, s]]")
print()
print("  S = (1/2) sum_k hat_k2_18 |hat_J|^2")
print("      + sum_k hat_phi^* [sum_mu d_mu^* hat_J_mu - hat_s]")
print("      - g_c sum_k hat_s^* [sum_mu d_mu^* hat_J_mu]")
print()
print("With eta = hat_phi - g_c hat_s:")
print("  S = (1/2) hat_k2_18 |hat_J|^2 + eta^* d_mu^* hat_J_mu - hat_phi^* hat_s")
print()
print("Complete the square in J (with 18-point kinetic term):")
print("  hat_J_mu^opt = -d_mu eta / hat_k2_18")
print()
print("  S_eff = -(hat_k2_6 / (2 hat_k2_18)) |eta|^2 - hat_phi^* hat_s")
print("        = -(R(k)/2) |hat_phi - g_c hat_s|^2 - hat_phi^* hat_s")
print()
print("Expand:")
print("  S_eff = -(R(k)/2) hat_phi^* hat_phi + R(k) g_c hat_phi^* hat_s")
print("          -(R(k)/2) g_c^2 hat_s^* hat_s - hat_phi^* hat_s")
print()
print("Now integrate out phi:")
print("  delta S/delta hat_phi^* = -R(k) hat_phi + R(k) g_c hat_s - hat_s = 0")
print("  hat_phi = g_c hat_s - hat_s/R(k)")
print("          = [g_c - 1/R(k)] hat_s")
print()
print("Substitute back:")
print("  S_eff[s] = -(R(k)/2) [g_c - 1/R(k)]^2 |hat_s|^2")
print("             + R(k) g_c [g_c - 1/R(k)] |hat_s|^2")
print("             - (R(k)/2) g_c^2 |hat_s|^2")
print("             - [g_c - 1/R(k)] |hat_s|^2")
print()
print("This simplifies to (after algebra):")
print("  S_eff[s] = (1/(2R(k)) - g_c + R(k) g_c^2/2) |hat_s|^2")
print()
print("Hmm, let me just compute this more carefully.")

# Actually, the cleanest way: after integrating out J, we have
# S_eff[phi, s] = -(R/2)|phi - g_c s|^2 - phi*s
# Now phi is the Lagrange multiplier; in the path integral it's integrated over.
# The phi integral is Gaussian:
# S_eff = -(R/2) phi^2 + phi [R g_c s - s] - (R/2)g_c^2 s^2
#       = -(R/2) phi^2 + phi s [R g_c - 1] - (R/2)g_c^2 s^2
#
# Completing the square in phi:
# phi^opt = s(R g_c - 1)/R
# S_eff[s] = +(1/(2R)) s^2 (R g_c -1)^2 - (R/2) g_c^2 s^2  ... no wait
# Actually with the negative sign:
# -(R/2)[phi - s(Rg_c-1)/R]^2 + (Rg_c-1)^2 s^2/(2R) - (R/2)g_c^2 s^2

print()
print("Let me compute this algebraically.")
print()
print("S_eff[phi, s] = -(R/2) phi^2 + phi*s*(R*g_c - 1) - (R*g_c^2/2)*s^2")
print()
print("Complete square in phi:")
print("  -(R/2)[phi - s(R*g_c-1)/R]^2 + s^2*(R*g_c-1)^2/(2R) - R*g_c^2*s^2/2")
print()
print("After phi integration:")
print("  S_eff[s] = s^2 * [(R*g_c - 1)^2/(2R) - R*g_c^2/2]")
print("           = s^2 * [R^2*g_c^2 - 2R*g_c + 1 - R^2*g_c^2] / (2R)")
print("           = s^2 * [1 - 2R*g_c] / (2R)")
print("           = s^2 * [1/(2R) - g_c]")
print()
print("So S_eff[s] = [1/(2R(k)) - g_c] * |hat_s(k)|^2")
print()
print("In position space, the s-s interaction is mediated by the")
print("kernel whose Fourier transform is 1/(2R(k)).")
print()
print("For the SELF-ENERGY (x=0):")
print("  Sigma = (1/V) sum_k 1/(2R(k)) = (1/2V) sum_k hat_k2_18/hat_k2_6")
print()

def compute_inverse_ratio(L):
    """Compute (1/V) sum_k hat_k2_18/hat_k2_6"""
    total = 0.0
    for n1 in range(L):
        for n2 in range(L):
            for n3 in range(L):
                k = np.array([2*np.pi*n1/L, 2*np.pi*n2/L, 2*np.pi*n3/L])
                r6 = hat_k2_6pt(k)
                r18 = hat_k2_18pt(k)
                if r6 < 1e-12:
                    continue
                total += r18 / r6
    return total / L**3

print("COMPUTATION: (1/V) sum_k hat_k2_18(k)/hat_k2_6(k)")
for L in [8, 16, 32, 64]:
    G = compute_inverse_ratio(L)
    print(f"  L={L:3d}: {G:.10f}")

print()

# Now the key: what about the phi_C propagator from the Poisson solve?
# In the engine, phi_C is solved via -Delta_6 phi_C = rho
# This gives phi_C(k) = hat_s(k) / hat_k2_6(k)
# The Coulomb self-energy is:
#   E_self = (alpha/2) * (1/V) sum_k |hat_s|^2 / hat_k2_6(k)
#          = (alpha/2) * W_3 (for a single charge)

print("="*80)
print("FINAL SYNTHESIS")
print("="*80)
print()
print("The FTD engine has TWO mechanisms that produce scalar interactions:")
print()
print("1. DYNAMICAL: J-field propagation with 18-point stencil + div coupling")
print("   -> After integrating out J AND phi (Lagrange multiplier):")
print("   S_eff[s] = sum_k [1/(2R(k)) - g_c] |hat_s(k)|^2")
print("   where R(k) = hat_k2_6(k)/hat_k2_18(k)")
print()
print("2. INSTANTANEOUS: Coulomb potential phi_C via 6-point Poisson")
print("   -> S_Coulomb = (alpha/2) * sum_k |hat_s(k)|^2 / hat_k2_6(k)")
print("   -> Self-energy = (alpha/2) * W_3")
print()
print("The TOTAL effective s-s interaction is the sum.")
print()
print("For the self-consistent gap equation, the relevant quantity is")
print("the self-energy. The gap equation x^2 = 16*G*^2*(x - G*)")
print("requires that the self-energy be proportional to W_3 = G*^2/(2pi).")
print()
print("The Coulomb self-energy IS proportional to W_3 (it uses 6-point Poisson).")
print("The question is whether the DYNAMICAL part (mechanism 1) also")
print("contributes or is subdominant.")
print()

# Compute R_avg = <R(k)> and <1/R(k)>
R_avg = compute_ratio_integral(64)
inv_R_avg = compute_inverse_ratio(64)
print(f"<R(k)> = <hat_k2_6/hat_k2_18> = {R_avg:.10f}")
print(f"<hat_k2_18/hat_k2_6> = {inv_R_avg:.10f}")
print(f"W_3 = {W3_exact:.10f}")
print()

# Check: does 1/(2*<1/R>) relate to W_3?
print(f"1/(2*<1/R>) = {1/(2*inv_R_avg):.10f}")
print(f"W_3/2 = {W3_exact/2:.10f}")
print()

# Actually, let me compute the FULL integral
# (1/V) sum_k 1/hat_k2_6(k)  [this IS W_3]
# vs
# (1/V) sum_k hat_k2_18(k)/hat_k2_6(k)^2  [this would be the 18pt-modified W_3]

def compute_modified_W3(L):
    """(1/V) sum_k hat_k2_18/hat_k2_6^2"""
    total = 0.0
    for n1 in range(L):
        for n2 in range(L):
            for n3 in range(L):
                k = np.array([2*np.pi*n1/L, 2*np.pi*n2/L, 2*np.pi*n3/L])
                r6 = hat_k2_6pt(k)
                r18 = hat_k2_18pt(k)
                if r6 < 1e-12:
                    continue
                total += r18 / (r6 * r6)
    return total / L**3

print("Modified Watson integrals:")
print("  Standard:  (1/V) sum 1/hat_k2_6")
print("  Modified:  (1/V) sum hat_k2_18/hat_k2_6^2")
print()
for L in [16, 32, 64]:
    W3_std = compute_W3_lattice(L)
    W3_mod = compute_modified_W3(L)
    print(f"  L={L:3d}: W3_std = {W3_std:.10f}  W3_mod = {W3_mod:.10f}  ratio = {W3_mod/W3_std:.10f}")

def compute_W3_lattice(L):
    total = 0.0
    for n1 in range(L):
        for n2 in range(L):
            for n3 in range(L):
                k = np.array([2*np.pi*n1/L, 2*np.pi*n2/L, 2*np.pi*n3/L])
                den = hat_k2_6pt(k)
                if den < 1e-12:
                    continue
                total += 1.0 / den
    return total / L**3

print()
print("Note: The Watson integral converges SLOWLY on finite lattices.")
print(f"  W_3(L=64) = {compute_W3_lattice(64):.10f}")
print(f"  W_3(exact) = {W3_exact:.10f}")
print(f"  Relative error at L=64: {abs(compute_W3_lattice(64)-W3_exact)/W3_exact*100:.2f}%")
print()
print("For the infinite-volume limit, use the exact value W_3 = G*^2/(2*pi).")

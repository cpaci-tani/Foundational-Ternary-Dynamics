"""
THE DECISIVE COMPUTATION: Gauss-Constrained Green's Function on the 3D Cubic Lattice
=====================================================================================

QUESTION: Does the Gauss constraint (div J = rho) change the effective scalar
Green's function from the trivial G_charge = 1/c^2 = 3 to the Watson integral W_3?

SETUP:
------
The FTD Euclidean action (from lagrangian.h) for the flux field J and state field s:

  S_E[J, s] = (1/2) J^T M_vec J  +  g_c * s * div(J)  +  lambda_G * (div J - rho)^2

where:
  - M_vec is the vector Laplacian (acts on 3-component J at each site)
  - div(J) = sum_mu (J_mu(x+e_mu) - J_mu(x)) is the lattice divergence
  - lambda_G -> infinity enforces div(J) = rho (Gauss's law)

In Fourier space (momentum k):
  - M_vec(k) = hat_k^2 * I_3  where hat_k^2 = sum_mu 4*sin^2(k_mu/2)
    (using the 6-point Laplacian; the 18-point version has different weights
     but the same qualitative structure)
  - div -> i * hat_k_mu (lattice momentum)
  - The divergence operator: D_mu(k) = (e^{ik_mu} - 1) for forward difference

The effective wave operator WITH Gauss constraint:
  M_eff(k) = hat_k^2 * I_3  +  2*lambda_G * hat_k hat_k^T

where hat_k = (e^{ik_1}-1, e^{ik_2}-1, e^{ik_3}-1) is the lattice gradient operator.

The effective scalar propagator after integrating out J:
  G_eff(k) = hat_k^T * M_eff^{-1}(k) * hat_k

We need: G_eff(x=0) = (1/V) * sum_k G_eff(k)

ANALYSIS IN FOURIER SPACE:
--------------------------
M_eff(k) = hat_k^2 * I_3 + 2*lambda_G * hat_k_vec hat_k_vec^T

where hat_k_vec = vector of (e^{ik_mu} - 1) and hat_k^2 = sum |e^{ik_mu}-1|^2.

By Sherman-Morrison:
  M_eff^{-1} = (1/hat_k^2) * [I_3 - 2*lambda_G * hat_k_vec hat_k_vec^T / (hat_k^2 + 2*lambda_G * hat_k^2)]
             = (1/hat_k^2) * [I_3 - 2*lambda_G/(hat_k^2(1+2*lambda_G)) * hat_k_vec hat_k_vec^T]

Then:
  hat_k_vec^T * M_eff^{-1} * hat_k_vec
    = hat_k^2/hat_k^2 - 2*lambda_G * hat_k^4 / (hat_k^2 * hat_k^2 * (1+2*lambda_G))
    = 1 - 2*lambda_G/(1+2*lambda_G)
    = 1/(1+2*lambda_G)

WAIT. That gives G_eff(k) = 1/(1+2*lambda_G) for ALL k.

As lambda_G -> infinity: G_eff -> 0. That's wrong in a physical sense but
mathematically expected: the constraint REMOVES the longitudinal mode entirely.

Let me reconsider. The Gauss constraint doesn't just add a penalty.
It constrains div(J) = rho, which means we should compute the CONSTRAINED
partition function differently.

CORRECT APPROACH: The constrained integral
------------------------------------------
With the hard constraint div(J) = rho, we integrate only over J satisfying
div(J) = rho. Decompose J = J_T + J_L where:
  - J_T is transverse: div(J_T) = 0
  - J_L is longitudinal: curl(J_L) = 0, div(J_L) = rho

The J_L part is FIXED by the constraint. The only free integration is over J_T.

But wait: the coupling term is g_c * s * div(J) = g_c * s * div(J_L) = g_c * s * rho.

If the constraint IS div(J) = rho = s (as in the engine), then the coupling
term becomes g_c * s * s = g_c (a constant for s=+/-1). This doesn't generate
a propagator at all!

THIRD APPROACH: The PHYSICAL question
--------------------------------------
Actually, the Gauss constraint doesn't set div(J) = s at every site independently.
It sets div(J)(x) = rho(x) where rho is the charge density. For a SINGLE charge
at the origin, div(J)(x) = delta(x,0) everywhere.

The longitudinal J field is then determined by:
  -Delta J_L = delta(x,0)  (Poisson equation)

So J_L(k) = hat_k_vec / hat_k^2, and the self-energy is:

  E_self = (1/2) * J_L^T * M_vec * J_L
         = (1/2) * sum_k |hat_k_vec|^2 / hat_k^4 * hat_k^2
         = (1/2) * sum_k hat_k^2 / hat_k^2 = trivial

NO WAIT. Let me be more careful. J_L is a VECTOR field. The constraint is:
  div(J)(x) = rho(x)

The MINIMUM energy longitudinal solution is:
  hat_J_L(k) = hat_k_vec^* * hat_rho(k) / hat_k^2

The energy of this configuration:
  E = (1/2) sum_k hat_J_L^dag(k) * hat_k^2 * hat_J_L(k)
    = (1/2) sum_k |hat_rho(k)|^2 * |hat_k_vec|^2 / hat_k^4 * hat_k^2
    = (1/2) sum_k |hat_rho(k)|^2 * hat_k^2 / hat_k^2
    = (1/2) * sum_k |hat_rho(k)|^2

For rho = delta(0): hat_rho = 1 for all k, so E = V/2. That's the trivial result.

But ACTUALLY: the correct Poisson solution has J_L as a GRADIENT:
  J_L = -grad(phi) where Delta phi = -rho
  So phi(k) = hat_rho(k) / hat_k^2
  And J_L(k) = -i * hat_k_vec * hat_rho(k) / hat_k^2

Wait, I need to use the CORRECT lattice operators. Let me be very precise.

PRECISE LATTICE OPERATORS
--------------------------
Forward difference: (D_mu f)(x) = f(x + e_mu) - f(x)
                    In Fourier: D_mu(k) = e^{ik_mu} - 1

Backward difference: (D_mu^* f)(x) = f(x) - f(x - e_mu)
                     In Fourier: D_mu^*(k) = 1 - e^{-ik_mu}

Divergence: div(J) = sum_mu D_mu^* J_mu = sum_mu (J_mu(x) - J_mu(x-e_mu))
            In Fourier: div(J)(k) = sum_mu (1 - e^{-ik_mu}) * hat_J_mu(k)

Gradient: (grad phi)_mu = D_mu phi = phi(x+e_mu) - phi(x)
          In Fourier: (grad phi)_mu(k) = (e^{ik_mu} - 1) * hat_phi(k)

Laplacian: Delta = div . grad = sum_mu D_mu^* D_mu
           In Fourier: hat_k^2 = sum_mu |e^{ik_mu} - 1|^2 = sum_mu 2(1-cos(k_mu))

The SCALAR Green's function: G_scalar(k) = 1/hat_k^2
  G_scalar(0,0) = (1/(2pi)^3) * int dk 1/hat_k^2 = W_3 (Watson integral!)

CORRECT SETUP
-------------
The J integral is Gaussian with source g_c * grad(s):

S_E = (1/2) sum_k hat_J^dag M_vec hat_J + g_c * sum_k hat_s^* * div^*(k) * hat_J(k)
    + lambda_G * sum_k |div^*(k) . hat_J - hat_rho|^2

div^*(k) = (1-e^{-ik_1}, 1-e^{-ik_2}, 1-e^{-ik_3})

COMPLETING THE SQUARE: What propagator does the charge-charge interaction use?

The question is what effective interaction s(x) feels with s(y) after
integrating out J. This depends on how s COUPLES to J.

From the Lagrangian:
  L_coupling = -g_c * s * div(J) = -g_c * s * sum_mu D_mu^* J_mu

The EL equation for J gives (from the coupling term):
  delta S / delta J_mu = 0 => gradient of s as source for J

So the source for J_mu is: j_mu(k) = -g_c * D_mu(k) * hat_s(k)
                                     = -g_c * (e^{ik_mu} - 1) * hat_s(k)

WITHOUT Gauss constraint:
  hat_J_mu(k) = M_vec^{-1} * j_mu = -g_c * (e^{ik_mu} - 1) * hat_s / hat_k^2

  S_eff = -(g_c^2/2) * sum_k |hat_s|^2 * sum_mu |e^{ik_mu}-1|^2 / hat_k^2
        = -(g_c^2/2) * sum_k |hat_s|^2 * hat_k^2/hat_k^2
        = -(g_c^2/2) * sum_k |hat_s|^2

  => G_charge(0) = 1  (trivial! The hat_k^2 cancels.)

WITH Gauss constraint (penalty lambda_G):
  M_eff_munu(k) = hat_k^2 delta_munu + 2*lambda_G * D_mu^*(k) * D_nu(k)

  The source is still j_mu = -g_c * D_mu * hat_s,
  but we need to add the constraint source too.

  Actually for the s-s effective action we just need:
  G_eff(k) = sum_mu,nu D_mu^*(k) * [M_eff^{-1}]_munu(k) * D_nu(k)

Let me compute this properly, including the effect of the Gauss constraint
on the FULL propagator for J, and then contract with the divergence operator
that appears in the coupling.

ALTERNATIVE: What if the coupling is NOT div-coupling but something else?

From lagrangian.h line 8:
  L_coupling = -g_c * s * (div J) - g_c * s * (v . J)

The velocity coupling gives a DIFFERENT vertex. For a static particle,
only the div-coupling matters, and as we showed, it gives the trivial result.

BUT: the force (lagrangian.h line 138) is:
  F = -alpha * s * grad(div J)

And from the engine, the Coulomb potential is solved via Poisson (render_bridge.h line 201):
  solve_coulomb_poisson() -- SOR Poisson solver!

So in the engine, the ACTUAL particle-particle interaction goes through the
SCALAR Poisson equation, not the vector wave equation! The Coulomb potential
phi_C satisfies:
  -Delta phi_C = rho

And the interaction energy is:
  E_Coulomb = (alpha/2) * sum_{x,y} s(x) * G_scalar(x-y) * s(y)

where G_scalar = (-Delta)^{-1} and G_scalar(0) = W_3!

So the Watson integral enters through the SCALAR POISSON SOLVER,
not through the vector J propagator.

This is the key insight. Let me verify numerically.
"""

import numpy as np
from scipy.integrate import dblquad

print("="*80)
print("COMPUTATION 1: Verify G_charge = hat_k^2/hat_k^2 = 1 (trivial)")
print("="*80)

# On an L x L x L lattice with periodic BC
def compute_G_charge(L):
    """Compute the div-coupled Green's function sum_k hat_k^2/hat_k^2"""
    total = 0.0
    count = 0
    for n1 in range(L):
        for n2 in range(L):
            for n3 in range(L):
                k1 = 2*np.pi*n1/L
                k2 = 2*np.pi*n2/L
                k3 = 2*np.pi*n3/L
                hat_k2 = 2*(1-np.cos(k1)) + 2*(1-np.cos(k2)) + 2*(1-np.cos(k3))
                if hat_k2 < 1e-12:
                    continue  # skip zero mode
                # The div-coupled propagator: sum_mu |e^{ik_mu}-1|^2 / hat_k^2
                # = hat_k^2 / hat_k^2 = 1
                G_k = hat_k2 / hat_k2  # = 1 always
                total += G_k
                count += 1
    return total / L**3

for L in [8, 16, 32, 64]:
    G = compute_G_charge(L)
    print(f"L={L:3d}: G_charge = {G:.10f}  (expected: 1 - 1/L^3 = {1-1/L**3:.10f})")

print()
print("="*80)
print("COMPUTATION 2: Verify G_scalar(0) = W_3 (Watson integral)")
print("="*80)

def compute_G_scalar(L):
    """Compute the SCALAR Green's function sum_k 1/hat_k^2"""
    total = 0.0
    for n1 in range(L):
        for n2 in range(L):
            for n3 in range(L):
                k1 = 2*np.pi*n1/L
                k2 = 2*np.pi*n2/L
                k3 = 2*np.pi*n3/L
                hat_k2 = 2*(1-np.cos(k1)) + 2*(1-np.cos(k2)) + 2*(1-np.cos(k3))
                if hat_k2 < 1e-12:
                    continue
                total += 1.0 / hat_k2
    return total / L**3

# Known: W_3 = Gamma(1/4)^4 / (4*pi^3) = G*^2/(2*pi)
from scipy.special import gamma
W3_exact = gamma(0.25)**4 / (4 * np.pi**3)
G_star = 2 * np.sqrt(2.622057554292119810 * 0.8346268416740731)
W3_from_Gstar = G_star**2 / (2 * np.pi)

print(f"W_3 (exact) = {W3_exact:.10f}")
print(f"W_3 (from G*^2/2pi) = {W3_from_Gstar:.10f}")
print()

for L in [8, 16, 32, 64]:
    G = compute_G_scalar(L)
    print(f"L={L:3d}: G_scalar(0) = {G:.10f}  (error: {abs(G-W3_exact)/W3_exact*100:.4f}%)")

print()
print("="*80)
print("COMPUTATION 3: Gauss-constrained Green's function")
print("="*80)
print()
print("The Gauss constraint modifies the vector propagator.")
print("M_eff(k) = hat_k^2 * I_3 + 2*lambda_G * d_vec^* d_vec^T")
print("where d_vec = (e^{ik_1}-1, ..., e^{ik_3}-1)")
print()

def compute_G_gauss_constrained(L, lambda_G):
    """
    Compute the effective scalar propagator with Gauss constraint.

    M_eff = hat_k^2 I_3 + 2*lambda_G * d^* d^T  (3x3 matrix at each k)

    G_eff(k) = d^T * M_eff^{-1} * d^*

    By Sherman-Morrison:
    M_eff^{-1} = (1/hat_k^2)[I - 2*lambda_G * d^* d^T / (hat_k^2 + 2*lambda_G * hat_k^2)]
               = (1/hat_k^2)[I - 2*lambda_G/(hat_k^2(1+2*lambda_G)) * d^* d^T]

    G_eff(k) = d^T * M_eff^{-1} * d^*
             = |d|^2/hat_k^2 - 2*lambda_G * |d^T d^*|^2 / (hat_k^4 * (1+2*lambda_G))

    BUT NOTE: d^T d^* = sum_mu (e^{ik_mu}-1)(e^{-ik_mu}-1) = sum_mu (2-e^{ik_mu}-e^{-ik_mu}) = hat_k^2
    And |d|^2 = sum_mu |e^{ik_mu}-1|^2 = hat_k^2

    So:
    G_eff(k) = hat_k^2/hat_k^2 - 2*lambda_G * hat_k^4/(hat_k^4 * (1+2*lambda_G))
             = 1 - 2*lambda_G/(1+2*lambda_G)
             = 1/(1+2*lambda_G)

    This is INDEPENDENT of k! And goes to 0 as lambda_G -> infinity.
    """
    total = 0.0
    count = 0
    for n1 in range(L):
        for n2 in range(L):
            for n3 in range(L):
                k = np.array([2*np.pi*n1/L, 2*np.pi*n2/L, 2*np.pi*n3/L])
                hat_k2 = sum(2*(1-np.cos(ki)) for ki in k)
                if hat_k2 < 1e-12:
                    continue

                # Lattice gradient operator d_mu = e^{ik_mu} - 1
                d_vec = np.array([np.exp(1j*ki) - 1 for ki in k])
                d_dag = np.conj(d_vec)

                # M_eff = hat_k^2 * I + 2*lambda_G * outer(d_dag, d_vec)
                M_eff = hat_k2 * np.eye(3) + 2*lambda_G * np.outer(d_dag, d_vec)

                # G_eff(k) = d_vec^T . M_eff^{-1} . d_dag
                M_inv = np.linalg.inv(M_eff)
                G_k = np.real(d_vec @ M_inv @ d_dag)

                total += G_k
                count += 1
    return total / L**3

L = 16
print(f"Using L={L} lattice")
print()
print("lambda_G     G_constrained    1/(1+2*lambda_G)   Match?")
print("-" * 65)
for lam in [0.0, 0.1, 1.0, 10.0, 100.0, 1000.0, 1e6]:
    G_num = compute_G_gauss_constrained(L, lam)
    G_analytic = 1.0 / (1.0 + 2.0*lam) if lam < 1e10 else 0.0
    expected = (1 - 1/L**3) * G_analytic  # account for zero-mode removal
    print(f"{lam:10.1f}    {G_num:.10f}     {expected:.10f}       {'YES' if abs(G_num - expected) < 1e-6 else 'NO'}")

print()
print("="*80)
print("RESULT: The Gauss constraint KILLS the div-coupled propagator.")
print("As lambda_G -> infinity, G_eff -> 0.")
print("The div-coupling does NOT produce the Watson integral.")
print("="*80)

print()
print("="*80)
print("COMPUTATION 4: The ACTUAL mechanism in the FTD engine")
print("="*80)
print()
print("From the engine code (lagrangian.h lines 136-139):")
print("  Force = -alpha * s * grad(div J)")
print("")
print("From render_bridge.h line 201:")
print("  solve_coulomb_poisson()  -- SOR Poisson solver for phi_C")
print("")
print("The engine ACTUALLY solves:")
print("  -Delta phi_C = rho  (Poisson equation)")
print("  F = -alpha * s * grad(phi_C)")
print("")
print("This means the particle-particle interaction is:")
print("  V(x,y) = alpha * s(x) * G_scalar(x-y) * s(y)")
print("")
print("where G_scalar = (-Delta)^{-1} has:")
print(f"  G_scalar(0,0) = W_3 = {W3_exact:.10f}")
print()

print("="*80)
print("COMPUTATION 5: WHERE does W_3 enter the self-consistency equation?")
print("="*80)
print()
print("The self-energy of a single charge at the origin:")
print("  Sigma = alpha * G_scalar(0) = alpha * W_3")
print()
print("The gap equation for the effective coupling (from self-consistent")
print("mean-field theory on the lattice):")
print()

alpha_FTD = 1.0 / 137.0361714582
print(f"  alpha = {alpha_FTD:.10f}")
print(f"  W_3 = {W3_exact:.10f}")
print(f"  G*^2 = {G_star**2:.10f}")
print(f"  16*G*^2 = {16*G_star**2:.10f}")
print(f"  1/alpha + 3 = {1/alpha_FTD + 3:.10f}")
print(f"  32*pi*W_3 = {32*np.pi*W3_exact:.10f}")
print()
print("Verification: 16*G*^2 = 32*pi*W_3?")
print(f"  16*G*^2      = {16*G_star**2:.10f}")
print(f"  32*pi*W_3    = {32*np.pi*W3_exact:.10f}")
print(f"  Difference   = {abs(16*G_star**2 - 32*np.pi*W3_exact):.2e}")

print()
print("="*80)
print("COMPUTATION 6: The CORRECT effective action")
print("="*80)
print()
print("The FTD action has TWO independent scalar operators acting on charges:")
print()
print("1. The vector J field with div-coupling:")
print("   After integrating J out, the s-s interaction goes through")
print("   the VECTOR propagator contracted with divergence operators.")
print("   This gives G_charge = 1 (trivial, as shown above).")
print()
print("2. The Coulomb potential phi_C solved via Poisson:")
print("   This is a SEPARATE scalar field computation in the engine.")
print("   It gives G_scalar = W_3 (Watson integral).")
print()
print("The question is: where does the Poisson equation COME FROM")
print("in the Lagrangian formulation?")
print()
print("ANSWER: The Gauss constraint div(J) = rho is enforced by the")
print("Lagrange multiplier lambda_G. The PHYSICAL MEANING of this")
print("constraint is that phi_C (the Coulomb potential) is the")
print("Lagrange multiplier field for the Gauss constraint!")
print()
print("In the path integral, the Gauss constraint introduces:")
print("  delta(div J - rho) = integral d(phi_C) exp(i*phi_C*(div J - rho))")
print()
print("After integrating out J, phi_C acquires a kinetic term from the")
print("J integration, giving an effective action for phi_C with propagator")
print("1/hat_k^2 = the SCALAR Green's function.")
print()
print("Let's verify this: the Lagrange multiplier field's effective propagator")
print("after integrating out J should be 1/hat_k^2.")
print()

print("="*80)
print("COMPUTATION 7: Lagrange multiplier propagator")
print("="*80)
print()
print("Start from the constrained action:")
print("  S = (1/2) J^T M_vec J - g_c s div(J) + phi(div J - rho)")
print()
print("where phi is the Lagrange multiplier (= Coulomb potential).")
print("Rewrite using div(J) = D^T J and grad(phi) = D phi:")
print()
print("  S = (1/2) J^T M_vec J + (D phi - g_c D s)^T J - phi rho + ...")
print()
print("Wait, let me be more careful. In Fourier space:")
print()
print("  S(k) = (1/2) hat_J_mu^* hat_k^2 hat_J_mu")
print("         - g_c hat_s^* d_mu^* hat_J_mu")
print("         + hat_phi^* d_mu^* hat_J_mu  - hat_phi^* hat_rho")
print()
print("The J integral is Gaussian: complete the square in J.")
print("The source for J_mu is: j_mu = g_c d_mu hat_s - d_mu hat_phi")
print("                             = d_mu (g_c hat_s - hat_phi)")
print()
print("After integrating out J:")
print("  S_eff = -(1/2) sum_k |d_vec|^2/hat_k^2 * |g_c hat_s - hat_phi|^2 - hat_phi^* hat_rho")
print("        = -(1/2) sum_k hat_k^2/hat_k^2 * |g_c hat_s - hat_phi|^2 - hat_phi^* hat_rho")
print("        = -(1/2) sum_k |g_c hat_s - hat_phi|^2 - hat_phi^* hat_rho")
print()

# WAIT. Let me redo this even more carefully.

print("ACTUALLY: Let me redo this with the CORRECT lattice operators.")
print()
print("The FTD action in Fourier space (for a single k-mode):")
print()
print("  S(k) = (1/2) hat_k^2 |hat_J(k)|^2  [kinetic]")
print("       - g_c * hat_s^*(-k) * [sum_mu d_mu^*(k) hat_J_mu(k)]  [coupling]")
print("       + hat_phi^*(-k) * [sum_mu d_mu^*(k) hat_J_mu(k)]  [constraint]")
print("       - hat_phi^*(-k) * hat_rho(k)  [constraint source]")
print()
print("Combine the J-linear terms:")
print("  S(k) = (1/2) hat_k^2 |hat_J|^2 + sum_mu (hat_phi - g_c hat_s)^* d_mu^* hat_J_mu - hat_phi^* hat_rho")
print()
print("Let eta = hat_phi - g_c hat_s.  The source for J_mu is eta^* d_mu^*.")
print()
print("Complete the square:")
print("  hat_J_mu^{optimal} = -d_mu * eta / hat_k^2")
print()
print("  S_eff = -(1/(2 hat_k^2)) * |eta|^2 * sum_mu |d_mu|^2 - hat_phi^* hat_rho")
print("        = -(1/(2 hat_k^2)) * |eta|^2 * hat_k^2 - hat_phi^* hat_rho")
print("        = -(1/2) |hat_phi - g_c hat_s|^2 - hat_phi^* hat_rho")
print()
print("This is EXACT! The hat_k^2 cancels!")
print()
print("Now: S_eff = -(1/2)|hat_phi|^2 + g_c hat_phi^* hat_s - (g_c^2/2)|hat_s|^2 - hat_phi^* hat_rho")
print()
print("The phi equation of motion (delta S_eff / delta phi^* = 0):")
print("  -hat_phi + g_c hat_s - hat_rho = 0")
print("  hat_phi = g_c hat_s - hat_rho")
print()
print("This gives phi = g_c * s - rho (a local relation, no propagator!).")
print()
print("BUT this is using the TRIVIAL (no-gradient) effective action.")
print("The hat_k^2 in the J propagator cancelled against the hat_k^2")
print("from |d_vec|^2, giving a k-independent result.")
print()
print("CONCLUSION: With the naive 6-point Laplacian as the J kinetic term,")
print("the Lagrange multiplier phi acquires NO kinetic term.")
print("The scalar Poisson equation phi_C arises from a SEPARATE physical")
print("mechanism, not from integrating out the vector J field.")

print()
print("="*80)
print("COMPUTATION 8: What if M_vec != hat_k^2 * I_3?")
print("="*80)
print()
print("The 18-point ISOTROPIC stencil (from lagrangian.h line 79-82):")
print("  The FTD engine uses an 18-point stencil: 6 face neighbors (weight 1/3)")
print("  plus 12 edge neighbors (weight 1/6).")
print()

def compute_18pt_hat_k2(k):
    """18-point isotropic Laplacian in Fourier space"""
    k1, k2, k3 = k
    # Face neighbors: weight 1/3 each
    face = (1/3) * (2*(1-np.cos(k1)) + 2*(1-np.cos(k2)) + 2*(1-np.cos(k3)))
    # Edge neighbors: weight 1/6 each
    # Pairs: (k1,k2), (k1,-k2), (k2,k3), (k2,-k3), (k1,k3), (k1,-k3)
    edge = (1/6) * (
        2*(1-np.cos(k1)*np.cos(k2)) +  # +x+y, -x-y
        2*(1-np.cos(k1)*np.cos(k2)) +  # +x-y, -x+y -- wait, this is wrong
        0
    )
    # Let me think about this more carefully.
    # The 18-point Laplacian: for a SCALAR field f:
    #   L f(x) = (1/3) sum_{face} [f(n)-f(x)] + (1/6) sum_{edge} [f(n)-f(x)]
    #          = (1/3) sum_{face} f(n) + (1/6) sum_{edge} f(n) - (6/3 + 12/6)*f(x)
    #          = (1/3) sum_{face} f(n) + (1/6) sum_{edge} f(n) - 4*f(x)
    #
    # In Fourier space:
    #   hat_L(k) = (1/3)[e^{ik1}+e^{-ik1}+e^{ik2}+e^{-ik2}+e^{ik3}+e^{-ik3}]
    #            + (1/6)[e^{i(k1+k2)}+e^{-i(k1+k2)}+e^{i(k1-k2)}+e^{-i(k1-k2)}
    #                   +e^{i(k2+k3)}+e^{-i(k2+k3)}+e^{i(k2-k3)}+e^{-i(k2-k3)}
    #                   +e^{i(k1+k3)}+e^{-i(k1+k3)}+e^{i(k1-k3)}+e^{-i(k1-k3)}] - 4
    #   = (2/3)[c1+c2+c3] + (1/3)[c1c2+c1c3+c2c3] - 4     (where ci = cos(ki))

    c1, c2, c3 = np.cos(k1), np.cos(k2), np.cos(k3)
    return 4 - (2/3)*(c1+c2+c3) - (1/3)*(c1*c2 + c1*c3 + c2*c3)

def compute_6pt_hat_k2(k):
    """Standard 6-point Laplacian"""
    return sum(2*(1-np.cos(ki)) for ki in k)

# Verify: at small k, both should give k^2
k_test = np.array([0.01, 0.02, 0.03])
print(f"Small k test: k = {k_test}")
print(f"  6-point:  hat_k^2 = {compute_6pt_hat_k2(k_test):.10f}")
print(f"  18-point: hat_k^2 = {compute_18pt_hat_k2(k_test):.10f}")
print(f"  |k|^2 =            {sum(k_test**2):.10f}")
print()

# The key question: with the 18-point Laplacian as M_vec, does the
# div-coupled propagator give something non-trivial?
#
# The divergence operator still uses the 6-point stencil (forward differences).
# So d_mu(k) = e^{ik_mu} - 1 and |d|^2 = hat_k^2 (6-point).
#
# But M_vec uses the 18-point stencil: M_vec(k) = hat_k2_18(k) * I_3
#
# G_charge(k) = |d|^2 / M_vec(k) = hat_k^2_6 / hat_k^2_18
# This is NOT trivially 1!

print("="*80)
print("COMPUTATION 9: div-coupled propagator with 18-point Laplacian")
print("="*80)
print()
print("If M_vec uses 18-point stencil but div uses 6-point:")
print("  G_charge(k) = hat_k^2_6(k) / hat_k^2_18(k)")
print()

def compute_G_mixed(L):
    """G_charge with 18-point Laplacian but 6-point divergence"""
    total = 0.0
    for n1 in range(L):
        for n2 in range(L):
            for n3 in range(L):
                k = np.array([2*np.pi*n1/L, 2*np.pi*n2/L, 2*np.pi*n3/L])
                hat_k2_6 = compute_6pt_hat_k2(k)
                hat_k2_18 = compute_18pt_hat_k2(k)
                if hat_k2_18 < 1e-12:
                    continue
                total += hat_k2_6 / hat_k2_18
    return total / L**3

print("Computing G_charge with mixed stencils...")
for L in [8, 16, 32, 64]:
    G = compute_G_mixed(L)
    print(f"  L={L:3d}: G_mixed = {G:.10f}")

print()
print(f"  W_3 = {W3_exact:.10f}")
print()

# And the full scalar Green's function with 18-point Laplacian
def compute_G_scalar_18pt(L):
    """Scalar Green's function with 18-point Laplacian"""
    total = 0.0
    for n1 in range(L):
        for n2 in range(L):
            for n3 in range(L):
                k = np.array([2*np.pi*n1/L, 2*np.pi*n2/L, 2*np.pi*n3/L])
                hat_k2_18 = compute_18pt_hat_k2(k)
                if hat_k2_18 < 1e-12:
                    continue
                total += 1.0 / hat_k2_18
    return total / L**3

print("Scalar Green's function with 18-point Laplacian:")
for L in [8, 16, 32]:
    G = compute_G_scalar_18pt(L)
    print(f"  L={L:3d}: G_scalar_18 = {G:.10f}")

print()
print(f"  W_3 (6-point) = {W3_exact:.10f}")
print(f"  G*^2/(2*pi) = {G_star**2/(2*np.pi):.10f}")

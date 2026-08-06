"""Is the SC network's registered 'simple shear costs 12 eps N gamma^4'
a real quartic, or a chord across a flat valley (the FTD-0787 error class)?

The repo records the SC binding network as "cohesive but not yet solid:
dilation costs 144 eps N eta^2, whereas simple shear costs only
12 eps N gamma^4, giving zero harmonic shear modulus."

A quartic on a first-order-flat direction is the n=4 signature, so this is
load-bearing for C3. But the Maxwell C3 screen finds ALL null directions of
the L=2,3,4 checkerboard blocks exactly flat under relaxation. Those two
statements are compatible only if the shear quartic is the AFFINELY
CONSTRAINED cost -- i.e. measured without letting the block relax.

Test: apply affine simple shear, measure E unrelaxed, then relax and measure
again. Same protocol as the C3 screen (PREREG_MAXWELL_C3_SCREEN_v1 sec 3.1).
"""
import itertools
import numpy as np
from scipy.optimize import minimize
from maxwell_c3_screen import EPS, mask, energy, grad, n_bonds, bond_set

def block(L):
    pts, sgn = [], []
    for c in itertools.product(range(L), repeat=3):
        pts.append(c); sgn.append((-1) ** sum(c))
    return np.array(pts, float), mask(np.array(sgn))

def shear(p, g):
    """Simple shear: x -> x + g*y. Volume preserving, no dilation."""
    q = p.copy(); q[:, 0] = q[:, 0] + g * q[:, 1]
    return q

print("Registered claim: simple shear costs 12*eps*N*gamma^4, dilation 144*eps*N*eta^2")
print(f"eps = {EPS}\n")

for L in (3, 4):
    p0, A = block(L); N = len(p0)
    x0 = p0.reshape(-1); E0 = energy(x0, A); B0 = bond_set(x0, A)
    pred = 12 * EPS * N
    print(f"=== L={L}  N={N}  B={n_bonds(x0,A)}  E0={E0:.10f} ===")
    print(f"  predicted affine shear coefficient 12*eps*N = {pred:.4f}")
    print(f"  {'gamma':>8} {'dE affine':>14} {'dE/g^4':>12} {'dE pinned':>13} "
          f"{'shear':>7} {'dE proj':>13} {'shear':>7}")
    # CRITICAL: an unconstrained relax can simply snap back to unsheared,
    # which would give dE = 0 for a trivial reason and prove nothing. Two
    # independent ways of actually holding the shear:
    #   (a) PIN the two extreme y-planes at their sheared positions, relax
    #       the interior. The standard shear-modulus measurement.
    #   (b) PROJECT: relax only over displacements orthogonal to the affine
    #       shear mode (and the rigid-body modes). Matches the C3 screen.
    ylo = np.isclose(p0[:, 1], p0[:, 1].min())
    yhi = np.isclose(p0[:, 1], p0[:, 1].max())
    pin = np.repeat(ylo | yhi, 3)
    u_sh = np.zeros_like(x0); u_sh[0::3] = p0[:, 1]      # affine shear field
    u_sh /= np.linalg.norm(u_sh)
    for g in (0.01, 0.02, 0.05, 0.1, 0.2):
        xs = shear(p0, g).reshape(-1)
        dE_aff = energy(xs, A) - E0
        # (a) pinned-boundary relaxation
        free = ~pin
        def fa(z):
            y = xs.copy(); y[free] = z; return energy(y, A)
        def ga(z):
            y = xs.copy(); y[free] = z; return grad(y, A)[free]
        ra = minimize(fa, xs[free], jac=ga, method="L-BFGS-B",
                      options=dict(maxiter=40000, ftol=1e-18, gtol=1e-15))
        xa = xs.copy(); xa[free] = ra.x
        # (b) shear-preserving projected relaxation
        rb = minimize(lambda z: energy(xs + z - u_sh * (u_sh @ z), A),
                      np.zeros_like(xs), method="L-BFGS-B",
                      options=dict(maxiter=40000, ftol=1e-18, gtol=1e-15))
        xb = xs + rb.x - u_sh * (u_sh @ rb.x)
        # how much shear actually survived each relaxation?
        sh_a = float(u_sh @ (xa - x0)) / float(u_sh @ (xs - x0))
        sh_b = float(u_sh @ (xb - x0)) / float(u_sh @ (xs - x0))
        print(f"  {g:>8.3f} {dE_aff:>14.6e} {dE_aff/g**4:>12.4f} "
              f"{float(ra.fun)-E0:>13.6e} {sh_a:>7.4f} "
              f"{energy(xb,A)-E0:>13.6e} {sh_b:>7.4f}")
    print()

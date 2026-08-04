"""Geometric inference probes — the checks a visual cortex does for free.

Each probe targets a specific failure mode observed on 2026-08-03/04:
  relax()      : FTD-0787 — computed along a held-fixed chord across a flat valley
  zero_modes() : FTD-0787 — 7 Maxwell zero modes were never counted
  multipoles() : protonucleus — a uniform +1 ball is just a charged sphere
  orient_sweep(): frame-dependence vs physical anisotropy (O_h, 48 elements)
  scale_law()  : bound-vs-actual extrapolation errors
  absent()     : de Rham cone — the complex has no time direction at all

Convention: the caller's array axes ARE the preferred frame. Every probe
reports which frame it evaluated in, so orientation is never implicit.
"""
import numpy as np, itertools
from scipy.optimize import minimize

# ---- the 48 elements of O_h as signed axis permutations -------------------
def oh_group():
    els = []
    for perm in itertools.permutations(range(3)):
        for sx, sy, sz in itertools.product((1, -1), repeat=3):
            M = np.zeros((3, 3), int)
            for i, p in enumerate(perm):
                M[i, p] = (sx, sy, sz)[i]
            els.append(M)
    return els                      # 6 * 8 = 48

def relax(energy, x0, fixed=None, tol=1e-12):
    """Minimise over ALL coordinates. `fixed` masks any that are truly pinned.
    Returns (x_relaxed, E0, E_relaxed, drop). A large drop means the original
    configuration was a chord, not a valley floor."""
    x0 = np.asarray(x0, float)
    mask = np.ones_like(x0, bool) if fixed is None else ~np.asarray(fixed, bool)
    def f(z):
        y = x0.copy(); y[mask] = z; return energy(y)
    r = minimize(f, x0[mask], method="Nelder-Mead",
                 options=dict(xatol=tol, fatol=tol, maxiter=20000))
    xr = x0.copy(); xr[mask] = r.x
    return dict(x=xr, E0=float(energy(x0)), E=float(r.fun),
                drop=float(energy(x0) - r.fun), frame="caller axes")

def zero_modes(energy, x0, h=1e-4, tol=1e-7):
    """Hessian spectrum + Maxwell count. Reports zero modes and whether each
    is a FINITE mechanism (energy stays flat along it) or only infinitesimal."""
    x0 = np.asarray(x0, float); n = len(x0)
    H = np.zeros((n, n))
    for i in range(n):
        for j in range(n):
            xpp = x0.copy(); xpp[i] += h; xpp[j] += h
            xpm = x0.copy(); xpm[i] += h; xpm[j] -= h
            xmp = x0.copy(); xmp[i] -= h; xmp[j] += h
            xmm = x0.copy(); xmm[i] -= h; xmm[j] -= h
            H[i, j] = (energy(xpp)-energy(xpm)-energy(xmp)+energy(xmm))/(4*h*h)
    H = (H + H.T)/2
    ev, evec = np.linalg.eigh(H)
    nz = int(np.sum(np.abs(ev) < tol))
    finite = []
    for idx in np.where(np.abs(ev) < tol)[0]:
        v = evec[:, idx]
        # walk a finite distance along the null direction
        dE = max(abs(energy(x0 + a*v) - energy(x0)) for a in (0.1, 0.3, 1.0))
        finite.append(bool(dE < 1e-6))
    return dict(eigenvalues=ev, n_zero=nz, n_stiff=int(n-nz),
                finite_mechanism=finite, maxwell_hint=f"{n} dof",
                frame="caller axes")

def multipoles(rho, order=2):
    """Monopole / dipole / quadrupole of a scalar source on a cubic grid.
    A large monopole means 'this is a charged ball' — its field grows with R
    for reasons that have nothing to do with the physics under test."""
    L = rho.shape[0]; c = (L-1)/2
    ax = np.arange(L) - c
    X, Y, Z = np.meshgrid(ax, ax, ax, indexing="ij")
    q = float(rho.sum())
    d = np.array([float((rho*A).sum()) for A in (X, Y, Z)])
    Q = np.zeros((3, 3))
    r2 = X*X + Y*Y + Z*Z
    for i, A in enumerate((X, Y, Z)):
        for j, B in enumerate((X, Y, Z)):
            Q[i, j] = float((rho*(3*A*B - (i == j)*r2)).sum())
    return dict(monopole=q, dipole=d, quadrupole=Q,
                charged=bool(abs(q) > 1e-9), frame="caller axes")

def orient_sweep(field_fn, tol=1e-9):
    """Evaluate a scalar functional under all 48 O_h frames.
    Spread ~ 0  -> frame-independent. Spread large -> either physical cubic
    anisotropy or an artifact of the axes you happened to choose."""
    vals = []
    for M in oh_group():
        vals.append(float(field_fn(M)))
    v = np.array(vals)
    return dict(mean=float(v.mean()), spread=float(v.max()-v.min()),
                invariant=bool(v.max()-v.min() < tol),
                n_frames=len(v), frame="swept over O_h (48)")

def scale_law(xs, ys):
    """Power-law fit y ~ x^p, with the residual. Guards against extrapolating
    a BOUND as if it were the measured quantity."""
    xs, ys = np.asarray(xs, float), np.asarray(ys, float)
    m = (xs > 0) & (ys > 0)
    p, b = np.polyfit(np.log(xs[m]), np.log(ys[m]), 1)
    pred = np.exp(b)*xs[m]**p
    return dict(exponent=float(p), prefactor=float(np.exp(b)),
                rel_resid=float(np.max(np.abs(pred-ys[m])/ys[m])))

def absent(obj_axes, needed_axes):
    """Structural audit: which required directions does this object simply
    not have? (de Rham complex: spatial only — no time index anywhere.)"""
    miss = [a for a in needed_axes if a not in obj_axes]
    return dict(present=list(obj_axes), missing=miss, complete=(not miss))

"""derive_composite_clock_dilation.py — the substitution, executed.

THE OWED ITEM.  The minimum viable clock is Galilean because its NODES
were given Newtonian dispersion p^2/2m.  The proposed fix was a
substitution: give the nodes the lattice dispersion instead, and (it was
claimed) the carrier would then inherit dilation, "a modelling change, not
a carrier search".  That was asserted and never run.  This runs it.

RESULT, ahead of the detail: the substitution is NECESSARY and NOT
SUFFICIENT, and the claim it was sufficient is withdrawn.

THE MODEL.  Two constituents on the axial section of the M18 lattice,
bound by a finite square well in the relative coordinate.  At fixed total
momentum K the relative-motion Hamiltonian is
    H_K = Toeplitz[ FT^-1( omega(q) + omega(K-q) ) ] + diag(V(r))
Hermitian and small, so it is diagonalised exactly.  Two bound states
below the two-particle continuum give a genuine INTERNAL CLOCK with gap
Omega = E_1 - E_0.

THE TEST.  A moving clock must satisfy Omega(K) = Omega(0)/gamma with
gamma = E_0(K)/E_0(0) -- and crucially this must hold UNIVERSALLY, with
the same exponent whatever the clock is made of.  That universality IS
time dilation; a material-dependent rate is not dilation at all.  So fit
    Omega(K)/Omega(0) = gamma^p
and ask whether p = -1 independently of the constituent mass and the
binding strength.

WHAT IS FOUND.
  Newtonian nodes:  p = 0.  The gap is K-independent to ~1e-4 while gamma
                    runs past 2.  No dilation, as the Galilean theorem
                    requires.  The substitution is therefore necessary.
  Lattice nodes:    p is NON-UNIVERSAL, ranging about -0.9 to -2.7 across
                    constituent mass and binding fraction.  Relativity
                    admits only p = -1, for every clock.  So the
                    substitution alone cannot deliver dilation.

THE DIAGNOSIS.  The relative effective mass obeys mu_K/mu_0 = gamma^3
exactly (free-dispersion kinematics at the equal-velocity point, verified
below), but a STATIC lab-frame well does not Lorentz-contract.  A
covariant binding would supply the missing, material-independent factor.
So the retardation axis -- dismissed earlier on the grounds that hydrogen
has instantaneous Coulomb binding and dilates exactly -- is load-bearing
after all.  Hydrogen escapes because its binding fraction is ~1e-5, not
because non-covariant binding is harmless.

SCOPE.  1-D axial section; two constituents rather than four; square-well
binding.  This is a negative structural result about static potentials; it
does not bound what a covariantly-bound carrier would do.
"""
from __future__ import annotations

import numpy as np
from scipy.linalg import toeplitz, eigh
from scipy.optimize import minimize_scalar

C = 1.0 / np.sqrt(3.0)
C2 = C * C
L = 512


def w_lat(q, M):
    """Exact M18 dispersion along an axis."""
    W = 4.0 * C2 * np.sin(np.asarray(q) / 2.0) ** 2 + M * M
    return 2.0 * np.arcsin(np.sqrt(W) / 2.0)


def w_nr(q, M, h=1e-5):
    """Newtonian node: same rest energy and curvature at q=0, p^2/2m form.

    q MUST be folded into [-pi, pi) first: q^2 on the raw grid [0, 2 pi)
    is not periodic, so omega(K - q) becomes discontinuous at the zone
    edge and the Galilean control silently fails."""
    qf = np.mod(np.asarray(q) + np.pi, 2 * np.pi) - np.pi
    w0 = w_lat(0.0, M)
    w2 = (w_lat(h, M) - 2 * w_lat(0.0, M) + w_lat(-h, M)) / h ** 2
    return w0 + 0.5 * w2 * qf ** 2


def spectrum(K, G, R, M, disp):
    q = 2.0 * np.pi * np.arange(L) / L
    T = disp(q, M) + disp(K - q, M)
    t = np.fft.ifft(T)
    r = np.arange(L)
    d = np.minimum(r, L - r)
    V = np.where(d <= R, -G, 0.0)
    ev = eigh(toeplitz(t, np.conj(t)) + np.diag(V), eigvals_only=True)
    return ev[0], ev[1], float(T.min())


def exponent(G, R, M, disp, Ks=(0.10, 0.15, 0.20)):
    """Fit p in Omega(K)/Omega(0) = gamma^p, at small K."""
    a0, b0, thr = spectrum(0.0, G, R, M, disp)
    gap0 = b0 - a0
    ps = []
    for K in Ks:
        a, b, _ = spectrum(K, G, R, M, disp)
        gam = a / a0
        ps.append(np.log((b - a) / gap0) / np.log(gam))
    return float(np.mean(ps)), (thr - a0) / thr, gap0, int(b0 < thr)


def mu_ratio(K, M, h=1e-4):
    """mu_K/mu_0 from the curvature of T_K at its minimum."""
    def T(q, KK):
        return w_lat(q, M) + w_lat(KK - q, M)

    def curv(KK):
        r = minimize_scalar(lambda q: T(q, KK), bounds=(KK / 2 - 1.0,
                                                        KK / 2 + 1.0),
                            method="bounded").x
        return (T(r + h, KK) - 2 * T(r, KK) + T(r - h, KK)) / h ** 2
    return curv(0.0) / curv(K)


def main():
    print("Composite clock dilation — the dispersion substitution, executed")
    print(f"  C = 1/sqrt3 = {C:.6f},  relative lattice L = {L}")

    # ---------------------------------------------------------------
    print("\n  [1] CONTROL — Newtonian nodes must give NO dilation")
    M, G, R = 0.40, 0.045, 13
    a0, b0, thr = spectrum(0.0, G, R, M, w_nr)
    gap0 = b0 - a0
    worst, gmax = 0.0, 1.0
    for K in np.linspace(0, 1.2, 13):
        a, b, _ = spectrum(K, G, R, M, w_nr)
        worst = max(worst, abs((b - a) / gap0 - 1.0))
        gmax = max(gmax, a / a0)
    print(f"      max |Omega(K)/Omega(0) - 1| = {worst:.3e}   "
          f"while gamma reaches {gmax:.4f}")
    assert worst < 5e-3, "Galilean control drifted"
    print(f"      => p = 0.  The carrier as built could never have tested"
          f" dilation.")

    # ---------------------------------------------------------------
    print("\n  [2] KINEMATICS — the relative effective mass")
    print(f"      {'K':>6} {'mu_K/mu_0':>12} {'gamma^3':>10} {'ratio':>9}")
    a0r, _, _ = spectrum(0.0, G, R, M, w_lat)
    for K in (0.2, 0.4, 0.6, 0.8):
        aK, _, _ = spectrum(K, G, R, M, w_lat)
        gam = aK / a0r
        mr = mu_ratio(K, M)
        print(f"      {K:6.2f} {mr:12.5f} {gam**3:10.5f} {mr/gam**3:9.5f}")
    print("      => mu_K = mu_0 gamma^3, exact free-dispersion kinematics.")

    # ---------------------------------------------------------------
    print("\n  [3] LATTICE NODES — is the dilation exponent universal?")
    print(f"      {'M':>5} {'well (G,R)':>13} {'bind.frac':>10} "
          f"{'Omega(0)':>10} {'p':>9}")
    cases = [(0.40, 0.045, 13), (0.40, 0.150, 6), (0.40, 0.300, 4),
             (0.25, 0.030, 13), (0.60, 0.100, 8), (0.80, 0.200, 6)]
    ps = []
    for Mc, Gc, Rc in cases:
        p, fB, g0, bound = exponent(Gc, Rc, Mc, w_lat)
        assert bound == 1, f"excited state unbound at M={Mc}"
        ps.append(p)
        print(f"      {Mc:5.2f} {f'({Gc:.3f},{Rc:2d})':>13} {fB:10.4f} "
              f"{g0:10.6f} {p:+9.4f}")
    ps = np.array(ps)
    print(f"\n  [verify] p spans [{ps.min():+.3f}, {ps.max():+.3f}]  "
          f"— spread {ps.max()-ps.min():.3f}")
    print(f"  [verify] relativity admits p = -1 for EVERY clock; "
          f"Newtonian gives p = 0")
    assert ps.max() - ps.min() > 0.5, "unexpectedly universal — recheck"

    print(f"""
  VERDICT
    The substitution is NECESSARY: with Newtonian nodes the internal clock
    does not move at all ({worst:.0e}) however fast the composite goes.

    It is NOT SUFFICIENT: with lattice nodes the clock does slow, but the
    exponent depends on what the clock is MADE OF, spanning
    {ps.min():+.2f} to {ps.max():+.2f} across constituent mass and binding
    fraction.  Time dilation is the statement that every clock slows by the
    same factor; a material-dependent rate is not dilation.

    Diagnosis: mu_K = mu_0 gamma^3 is exact kinematics, but a static
    lab-frame well does not Lorentz-contract, and nothing else supplies the
    missing material-independent factor.  The binding must be made
    covariant.  The earlier claim that this was "a modelling change, not a
    carrier search" is withdrawn.""")


if __name__ == "__main__":
    main()

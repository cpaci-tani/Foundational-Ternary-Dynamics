"""derive_composite_clock_dilation.py — finite-gap SR diagnostic.

THE MODEL.  Two constituents on the axial section of the M18 lattice are
bound by a finite square well in the relative coordinate.  At fixed total
momentum K the relative-motion Hamiltonian is

    H_K = Toeplitz[FT^-1(omega(q) + omega(K-q))] + diag(V(r)).

Two eigenvalues below the free continuum define a spectral gap
Omega(K) = E_1(K) - E_0(K).

THE CORRECT COMPARATOR.  The often-used Omega(K)/Omega(0) = 1/gamma law is
only the infinitesimal-gap limit.  A Lorentz-covariant two-level system with
rest energies M_0 and M_1 has, at fixed momentum P,

    R_SR(P) = [sqrt(M_1^2 + c^2 P^2) - sqrt(M_0^2 + c^2 P^2)]
              / (M_1 - M_0).

Writing gamma_0 = E_0(P)/M_0 gives the numerically stable equivalent

    R_SR = (M_0 + M_1)
           / [gamma_0 M_0 + sqrt(M_1^2 + M_0^2(gamma_0^2 - 1))].

Only as (M_1-M_0)/M_0 -> 0 does this reduce to 1/gamma_0.  This script
therefore reports R_observed/R_SR - 1 for the six fixed cases.  It uses the
measured ground-state energy ratio as a conditional gamma proxy; a complete
Lorentz test would separately have to establish the ground-state common
cone and its continuum limit.

RESULT.  The fixed cases miss the conditional finite-gap comparator by
0.04%--1.08% over K in {0.10, 0.15, 0.20}; one case is much closer than the
others.  Without finite-volume, momentum-window, continuum, and interacting
common-cone controls, this is exploratory and inconclusive.  The former fit
of a material-dependent exponent p in Omega(K)/Omega(0)=gamma^p is retained
nowhere as a relativistic verdict because p=-1 was not the exact finite-gap
target and the logarithmic fit was ill-conditioned near gamma=1.

The free-pair curvature relation mu_K/mu_0 ~= gamma_free^3 is also only a
low-momentum continuum approximation for the lattice dispersion.  Its
observed relative residual grows from 0.33% at K=0.2 to 6.54% at K=0.8.

SCOPE.  One-dimensional axial section; two constituents; one finite square
well; one lattice size; six fixed parameter cases.  This script does not
establish a structural no-go for static binding or Lorentz recovery.
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


def finite_gap_sr_ratio(gamma0, ground_rest, gap_rest):
    """Exact fixed-momentum SR gap ratio, conditioned on ground gamma.

    ``ground_rest`` and ``ground_rest + gap_rest`` are the two rest
    energies.  Rationalizing the difference of square roots avoids loss of
    precision for a small clock gap.
    """
    excited_rest = ground_rest + gap_rest
    momentum_sq = ground_rest * ground_rest * (gamma0 * gamma0 - 1.0)
    excited_moving = np.sqrt(excited_rest * excited_rest + momentum_sq)
    ground_moving = gamma0 * ground_rest
    return (ground_rest + excited_rest) / (ground_moving + excited_moving)


def case_residuals(G, R, M, disp, Ks=(0.10, 0.15, 0.20)):
    """Return residuals of the observed gap ratio from finite-gap SR."""
    a0, b0, thr = spectrum(0.0, G, R, M, disp)
    gap0 = b0 - a0
    rows = []
    for K in Ks:
        a, b, _ = spectrum(K, G, R, M, disp)
        gamma0 = a / a0
        observed = (b - a) / gap0
        expected = finite_gap_sr_ratio(gamma0, a0, gap0)
        rows.append((K, gamma0, observed, expected, observed / expected - 1.0))
    return rows, (thr - a0) / thr, gap0, a0, int(b0 < thr)


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
    print("Composite clock dilation — finite-gap SR diagnostic")
    print(f"  C = 1/sqrt3 = {C:.6f},  relative lattice L = {L}")

    # ---------------------------------------------------------------
    print("\n  [1] CONTROL — Newtonian nodes keep the gap nearly fixed")
    M, G, R = 0.40, 0.045, 13
    a0, b0, thr = spectrum(0.0, G, R, M, w_nr)
    gap0 = b0 - a0
    worst_drift, worst_sr_residual, gmax = 0.0, 0.0, 1.0
    for K in np.linspace(0, 1.2, 13):
        a, b, _ = spectrum(K, G, R, M, w_nr)
        gamma0 = a / a0
        observed = (b - a) / gap0
        expected = finite_gap_sr_ratio(gamma0, a0, gap0)
        worst_drift = max(worst_drift, abs(observed - 1.0))
        worst_sr_residual = max(worst_sr_residual,
                                abs(observed / expected - 1.0))
        gmax = max(gmax, gamma0)
    print(f"      max |Omega(K)/Omega(0) - 1| = {worst_drift:.3e}   "
          f"while gamma_0 reaches {gmax:.4f}")
    print(f"      max |R_observed/R_SR - 1| = {worst_sr_residual:.4f}")
    assert worst_drift < 5e-3, "Galilean control drifted"
    assert worst_sr_residual > 0.1, "Newtonian control no longer separates"
    print("      => within this instrument, Newtonian constituent dispersion")
    print("         does not reproduce the finite-gap SR comparator.")

    # ---------------------------------------------------------------
    print("\n  [2] KINEMATICS — free-pair curvature is only approximately gamma^3")
    print(f"      {'K':>6} {'mu_K/mu_0':>12} {'gamma_f^3':>11} "
          f"{'rel.resid':>11}")
    mu_residuals = []
    for K in (0.2, 0.4, 0.6, 0.8):
        gamma_free = w_lat(K / 2.0, M) / w_lat(0.0, M)
        mr = mu_ratio(K, M)
        rel = mr / gamma_free ** 3 - 1.0
        mu_residuals.append(rel)
        print(f"      {K:6.2f} {mr:12.5f} {gamma_free**3:11.5f} "
              f"{100*rel:+10.3f}%")
    print("      => gamma^3 is the continuum/small-K relation, not an exact")
    print("         identity of the lattice dispersion.")

    # ---------------------------------------------------------------
    print("\n  [3] LATTICE NODES — residual from the finite-gap SR comparator")
    print(f"      {'M':>5} {'well (G,R)':>13} {'bind.frac':>10} "
          f"{'gap/E0':>9} {'resid @ K=.10,.15,.20':>29} {'max|r|':>9}")
    cases = [(0.40, 0.045, 13), (0.40, 0.150, 6), (0.40, 0.300, 4),
             (0.25, 0.030, 13), (0.60, 0.100, 8), (0.80, 0.200, 6)]
    all_max = []
    for Mc, Gc, Rc in cases:
        rows, fB, gap0, ground0, bound = case_residuals(Gc, Rc, Mc, w_lat)
        assert bound == 1, f"excited state unbound at M={Mc}"
        residuals = np.array([row[-1] for row in rows])
        max_residual = float(np.max(np.abs(residuals)))
        all_max.append(max_residual)
        rendered = ",".join(f"{100*r:+.3f}%" for r in residuals)
        print(f"      {Mc:5.2f} {f'({Gc:.3f},{Rc:2d})':>13} {fB:10.4f} "
              f"{gap0/ground0:9.4f} {rendered:>29} {100*max_residual:8.3f}%")
    all_max = np.array(all_max)
    print(f"\n  [report] max fixed-case residuals span "
          f"{100*all_max.min():.3f}%--{100*all_max.max():.3f}%")
    # Check the exact formula's infinitesimal-gap limit independently of the
    # measured cases.  This is a mathematical integrity assertion, not an
    # outcome gate.
    for gamma0 in (1.01, 1.2, 2.0):
        limit = finite_gap_sr_ratio(gamma0, 1.0, 1e-9)
        assert abs(limit - 1.0 / gamma0) < 2e-9

    print(f"""
  VERDICT — EXPLORATORY / INCONCLUSIVE
    The old p=-1 target was only the infinitesimal-gap approximation and its
    logarithmic exponent fit amplified small discrepancies near gamma=1.
    Against the exact finite-gap comparator, the six fixed cases have
    maximum residuals of {100*all_max.min():.3f}% to
    {100*all_max.max():.3f}% over K = 0.10, 0.15, 0.20.

    These numbers do not establish relativistic dilation, but neither do
    they establish a structural no-go for static binding.  Finite-volume,
    momentum-window, continuum, and common-cone controls are still absent.
    The former claims of a universal p=-1 test, an exact mu_K/mu_0=gamma^3
    lattice identity, and a diagnosed static-binding obstruction are
    withdrawn.""")


if __name__ == "__main__":
    main()

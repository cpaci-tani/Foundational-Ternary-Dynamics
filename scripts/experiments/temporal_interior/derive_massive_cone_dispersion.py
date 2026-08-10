"""derive_massive_cone_dispersion.py — does the limiting speed depend on
the rest mass?  A free-sector mass-shell diagnostic, computed.

WHY THIS AND NOT A BOOSTED MVC.  The minimum viable clock is a mechanical
framework, m q'' = -grad V(|q_i - q_j|).  That is GALILEAN invariant: boost
it and its period is unchanged, T(v) = T(0) exactly.  It therefore cannot
serve as an SR clock test.  The calculation below interrogates only the
DISPERSION of a free massive scalar mode; it does not construct a moving
physical clock.

THE FREE MASS-SHELL DIAGNOSTIC.  A packet built around wavenumber k has
group velocity v_g = |grad_k Omega|.  Define

    D(k) = Omega(k) * sqrt(1 - v_g(k)^2 / c^2)   =  M   (const)

which is an identity for Omega^2 = c^2 k^2 + M^2 (since v_g = c^2 k/Omega
gives 1 - v_g^2/c^2 = M^2/Omega^2).  Constancy of D therefore diagnoses a
Lorentz-form free dispersion on the stated branch and with the stated
choice of c.  Its k-dependence measures a mass-shell residual; it is not an
operational clock observable and does not demonstrate physical time
dilation.

THE LATTICE KLEIN-GORDON.  Adding a mass to the M18 leapfrog,
    phi(t+1) - 2 phi(t) + phi(t-1) = C^2 L phi - M^2 phi
gives the exact dispersion
    4 sin^2(Omega/2) = C^2 (-L(k)) + M^2  ==  W(k).

SCOPE.  This is the generic quadratic massive mode on this substrate --
what ANY massive excitation obeys at quadratic order in the scalar sector.
It is NOT a claim about FTD's own manifestation/mass mechanism, and it is a
different question from FTD-0412, which compares distinct SECTORS (flux vs
Wilson vs gauge) at zero mass.  Here the sector is fixed and the mass
varies.  No bound carrier or clock readout is constructed.
"""
from __future__ import annotations

import numpy as np
import sympy as sp

C2 = sp.Rational(1, 3)                    # C^2 = 1/3
C_NUM = 1.0 / np.sqrt(3.0)

k, M, t = sp.symbols('k M t', positive=True)


def mL_series(direction, order=8):
    """-L(k) along a unit direction, as a series in k."""
    d = [sp.nsimplify(x) for x in direction]
    n = sp.sqrt(sum(x ** 2 for x in d))
    d = [x / n for x in d]
    ks = [t * d[i] * k for i in range(3)]
    c = [sp.cos(x) for x in ks]
    L = (sp.Rational(2, 3) * (c[0] + c[1] + c[2])
         + sp.Rational(2, 3) * (c[0] * c[1] + c[1] * c[2] + c[2] * c[0]) - 4)
    return sp.series(-L, t, 0, order).removeO().subs(t, 1)


def omega_sq_series(direction, order=8):
    """Omega^2 expanded in k at fixed M, from 4 sin^2(Omega/2) = W."""
    W = C2 * mL_series(direction, order) + M ** 2
    Om = 2 * sp.asin(sp.sqrt(W.subs(k, t * k)) / 2)
    s = sp.series(Om ** 2, t, 0, order).removeO().subs(t, 1)
    return sp.expand(sp.simplify(s))


print("=" * 74)
print("(1) THE MASSIVE DISPERSION, EXPANDED")
print("=" * 74)
dirs = [("[100]", (1, 0, 0)), ("[110]", (1, 1, 0)), ("[111]", (1, 1, 1))]
coeffs = {}
for nm, d in dirs:
    s = omega_sq_series(d, 8)
    p = sp.Poly(s, k)
    c2 = sp.simplify(p.coeff_monomial(k ** 2))
    c4 = sp.simplify(p.coeff_monomial(k ** 4))
    c0 = sp.simplify(p.coeff_monomial(1))
    coeffs[nm] = (c0, c2, c4)
    print(f"  {nm}:")
    print(f"      k^0 : {sp.nsimplify(sp.series(c0, M, 0, 6).removeO())}")
    print(f"      k^2 : {sp.nsimplify(sp.series(c2, M, 0, 6).removeO())}")
    print(f"      k^4 : {sp.nsimplify(sp.series(c4, M, 0, 4).removeO())}")

print()
print("=" * 74)
print("(2) IS THE k^2 COEFFICIENT DIRECTION-INDEPENDENT?")
print("=" * 74)
base = coeffs["[100]"][1]
iso = all(sp.simplify(coeffs[nm][1] - base) == 0 for nm, _ in dirs)
print(f"  k^2 coefficients equal across [100], [110], [111]:  {iso}")
print(f"  => the limiting speed is ISOTROPIC.")
Ceff2 = sp.simplify(sp.series(base, M, 0, 6).removeO())
print(f"\n  C_eff^2(M) = {Ceff2}")
print(f"             = C^2 * (1 + {sp.simplify(Ceff2/C2 - 1)})")
ratio = sp.simplify(sp.sqrt(Ceff2 / C2))
print(f"  C_eff / C  = {sp.series(ratio, M, 0, 5)}")

print()
print("  ANISOTROPY: the k^4 coefficient of Omega^2 is also direction-")
print("  independent, so the first anisotropy is at k^6.  Cross-check it")
print("  against the recorded free-sector figure |dv/v| = (ka)^4/3240:")
vph = {}
for nm, d in dirs:
    W0 = C2 * mL_series(d, 10)               # massless
    Om = 2 * sp.asin(sp.sqrt(W0.subs(k, t * k)) / 2)
    s = sp.series(Om / (t * k), t, 0, 6).removeO().subs(t, 1)
    vph[nm] = sp.simplify(sp.expand(s) / sp.sqrt(C2))
    print(f"      {nm}:  v_phase/C = {sp.nsimplify(vph[nm])}")
d_ax_diag = sp.simplify(vph["[100]"] - vph["[111]"])
print(f"\n      v[100] - v[111], over C = {sp.nsimplify(d_ax_diag)}")
c4 = sp.Poly(sp.expand(d_ax_diag), k).coeff_monomial(k ** 4)
print(f"      k^4 coefficient = {sp.nsimplify(c4)}"
      f"   -> 1/{sp.nsimplify(1/c4)}   (recorded: 1/3240)")

print()
print("=" * 74)
print("(3) THE FREE MASS-SHELL DIAGNOSTIC, NUMERICALLY")
print("=" * 74)


def omega(kv, Mv):
    kv = np.atleast_2d(kv)
    c = np.cos(kv)
    L = (2 / 3) * c.sum(-1) + (2 / 3) * (
        c[..., 0] * c[..., 1] + c[..., 1] * c[..., 2] + c[..., 2] * c[..., 0]) - 4
    W = C_NUM ** 2 * (-L) + Mv ** 2
    return 2 * np.arcsin(np.sqrt(W) / 2)


def vgroup(kv, Mv):
    """|grad Omega|.  W = 4 sin^2(Omega/2) gives dW/dOmega = 2 sin(Omega),
    so dOmega/dk_i = (dW/dk_i) / (2 sin Omega), and
    dW/dk_i = C^2 (2/3) sin k_i (1 + cos k_j + cos k_l).
    Net: C^2 sin k_i (1 + c_j + c_l) / (3 sin Omega).  (The factor 2 in
    dW/dOmega is easy to drop; doing so doubles v_g and turns the O(k^4)
    mass-shell residual below into a spurious O(k^2) one.)"""
    kv = np.atleast_2d(kv)
    c, s = np.cos(kv), np.sin(kv)
    Om = omega(kv, Mv)
    j = [(1, 2), (2, 0), (0, 1)]
    g = np.stack([C_NUM ** 2 * s[..., i] * (1 + c[..., a] + c[..., b])
                  / (3.0 * np.sin(Om))
                  for i, (a, b) in enumerate(j)], -1)
    return np.linalg.norm(g, axis=-1)


def check_vgroup_massless():
    """Cross-check against the independently-derived massless formula."""
    kv = np.array([[0.3, 0.17, 0.05]])
    c, s = np.cos(kv), np.sin(kv)
    L = (2 / 3) * c.sum(-1) + (2 / 3) * (
        c[..., 0] * c[..., 1] + c[..., 1] * c[..., 2] + c[..., 2] * c[..., 0]) - 4
    mL = -L
    S = (C_NUM / 2) * np.sqrt(mL)
    j = [(1, 2), (2, 0), (0, 1)]
    g = np.stack([(C_NUM / 3) * s[..., i] * (1 + c[..., a] + c[..., b])
                  / (np.sqrt(1 - S ** 2) * np.sqrt(mL))
                  for i, (a, b) in enumerate(j)], -1)
    return float(np.linalg.norm(g, axis=-1)[0]), float(vgroup(kv, 0.0)[0])


a, b = check_vgroup_massless()
print(f"  [check] v_g at M=0 via massless formula {a:.12f}")
print(f"          v_g at M=0 via massive  formula {b:.12f}   "
      f"agree: {abs(a-b) < 1e-13}")
print()
print("  D(k) = Omega * sqrt(1 - v_g^2/c^2), along [100].")
print("  For Lorentz-form free dispersion this is k-independent.  Two candidate c:")
print()
def C_eff_exact(Mv):
    """k^2 coefficient of the EXACT dispersion, by fit at small k.
    (Using the M-truncated series instead leaves a ~1% error in C_eff that
    swamps the O(k^4) residual this test is trying to expose.)"""
    ks = np.linspace(1e-3, 6e-3, 12)
    O2 = np.array([float(omega(np.array([[x, 0.0, 0.0]]), Mv)[0]) ** 2
                   for x in ks])
    O0 = float(omega(np.zeros((1, 3)), Mv)[0]) ** 2
    A = np.stack([ks ** 2, ks ** 4], -1)
    a2, _ = np.linalg.lstsq(A, O2 - O0, rcond=None)[0]
    return np.sqrt(a2)


hdr = (f"{'M':>8} {'k':>8} {'v_g/C':>9} "
       f"{'dev, c=C':>13} {'dev, c=C_eff':>15} {'k^4/(36 M^2)':>15}")
for Mv in (0.05, 0.2, 0.5):
    Ce = C_eff_exact(Mv)
    print(f"\n  M = {Mv}   C_eff/C - 1 = {Ce/C_NUM - 1:.8e}"
          f"   (leading prediction M^2/12 = {Mv**2/12:.8e})")
    print(hdr)
    O0 = float(omega(np.zeros((1, 3)), Mv)[0])
    for kk in (0.01, 0.02, 0.05, 0.1, 0.2):
        kv = np.array([[kk, 0.0, 0.0]])
        Om = float(omega(kv, Mv)[0])
        vg = float(vgroup(kv, Mv)[0])
        dC = Om * np.sqrt(max(1 - (vg / C_NUM) ** 2, 0.0)) / O0 - 1
        dE = Om * np.sqrt(max(1 - (vg / Ce) ** 2, 0.0)) / O0 - 1
        print(f"  {Mv:8.3f} {kk:8.3f} {vg/C_NUM:9.5f} "
              f"{dC:13.3e} {dE:15.3e} {kk**4/(36*Mv**2):15.3e}")
print("""
  READING.  With the bare cone speed C the mass-shell diagnostic fails at
  O(k^2), coefficient -1/36 -- but that is the ordinary lattice dispersion,
  not a Lorentz anomaly: it is removed exactly by using the species' own
  limiting speed C_eff(M).  What survives is O(k^4), tracking k^4/(36 M^2),
  which is the k^4 term of the dispersion (-1/54) fed through the diagnostic.
  So each mass has its OWN very nearly Lorentzian free sector; the
  violation lives in the MISMATCH BETWEEN sectors.""")

print()
print("=" * 74)
print("(4) THE TWO-SPECIES FREE-SECTOR STATEMENT, UNDER A SCALE INSERTION")
print("=" * 74)
T_PHYS = 5.391247e-44 / np.sqrt(3.0)         # one tick, electron-primary
W_E = 7.7634e20                              # m_e c^2 / hbar  [rad/s]
MP_ME = 1836.152673
Me = W_E * T_PHYS
Mp = W_E * MP_ME * T_PHYS
print(f"  one tick t_phys           = {T_PHYS:.4e} s")
print(f"  electron  M_e = w_e t     = {Me:.4e}")
print(f"  proton    M_p             = {Mp:.4e}")
print(f"\n  dC/C for the electron     = M_e^2/12 = {Me**2/12:.4e}")
print(f"  dC/C for the proton       = M_p^2/12 = {Mp**2/12:.4e}")
print(f"  DIFFERENTIAL, proton-electron       = {(Mp**2 - Me**2)/12:.4e}")
print(f"""
  So two free scalar modes of different rest mass, in the SAME sector, do
  not share a limiting speed exactly.  Under the displayed Planck-scale
  insertion, the mismatch is (m/M_Planck)^2/12 in natural units -- a
  dimension-six-suppressed effect, isotropic, and of order 1e-40 for the
  proton/electron mass scales.  This is a conditional scale calibration,
  not a proton/electron carrier or clock prediction.""")

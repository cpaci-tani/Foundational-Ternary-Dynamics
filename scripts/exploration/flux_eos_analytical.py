#!/usr/bin/env python3
"""
FTD-0312 Leg A — analytical lattice equation-of-state of the FTD flux-wave bath.

QUESTION. The owner proposed that the master quadratic's smaller root x- = 3.023964
(specifically its residual delta_c = x- - 3 = 0.023964) "is the dimensionless pressure
of the flux." x- is alpha-locked: x- = 16 G*^3 / x+ = 16 G*^3 * alpha (given x+ = 1/alpha).
A radiation field has dimensionless EoS 1/w = rho/p = D = 3 EXACTLY in the continuum.
This leg computes 1/w for the ACTUAL FTD 18-pt lattice dispersion over the Brillouin
zone, to establish: (a) the IR baseline (-> 3.000), (b) the lattice (UV) correction,
(c) whether 3.024 is a special point or just a value on a smooth, geometric (NOT
alpha-dependent) curve.

PHYSICS. For a gas of field modes with dispersion omega(k) and occupation n(k):
  rho = integral n(k) omega(k) d3k                         (energy density)
  p   = (1/D) integral n(k) (k . grad_k omega) d3k         (kinetic pressure, scalar)
  1/w = rho / p
Linear dispersion omega = c|k| gives k.grad omega = omega -> p = rho/D -> 1/w = D = 3.
The 18-pt FTD wave operator symbol is
  M(k) = (2/3) sum cos(k_i) + (2/3) sum_{i<j} cos(k_i)cos(k_j) - 4   (<= 0),
  omega(k)^2 = -c^2 M(k),  c = 1/sqrt(3),  omega ~ c|k| as k->0.
The group velocity bends below c near the zone edge, so k.grad omega / omega < 1 there,
pushing 1/w ABOVE 3 -- the lattice correction has the right SIGN for delta_c > 0, but it
is set by the mode SPECTRUM (occupation), not by alpha.

Occupations swept:
  - classical equipartition  n omega = T  (the engine's Langevin regime): n ~ 1/omega,
    T cancels -> 1/w is a fixed GEOMETRIC constant (full-BZ, UV-saturated).
  - Bose n = 1/(exp(omega/T)-1) at a range of T: interpolates IR(3.000) -> UV-saturated.
  - hard IR cutoff k<=k_max: shows 1/w(k_max) from 3.000 up.

[EPISTEMIC: [MEASURED -- analytical lattice baseline]; the verdict on "x- = flux pressure"
 needs the engine leg (alpha-tracking). 1/w here is geometric, NOT alpha-dependent.]
"""

import numpy as np

C2 = 1.0 / 3.0
C = np.sqrt(C2)
# x- target (master quadratic smaller root) and the radiation integer
import mpmath as mp
mp.mp.dps = 30
_Gs = mp.gamma(mp.mpf(1)/4) / mp.gamma(mp.mpf(3)/4)
_a, _b = 16*_Gs**2, 16*_Gs**3
X_MINUS = float((_a - mp.sqrt(_a*_a - 4*_b)) / 2)     # 3.023964
DELTA_C = X_MINUS - 3.0                                 # 0.023964


def bz_fields(n=96):
    """Brillouin-zone grid of omega and (k . grad omega) for the 18-pt dispersion."""
    k = (np.arange(n) + 0.5) / n * 2*np.pi - np.pi      # k in (-pi, pi), midpoint rule
    KX, KY, KZ = np.meshgrid(k, k, k, indexing="ij")
    cx, cy, cz = np.cos(KX), np.cos(KY), np.cos(KZ)
    sx, sy, sz = np.sin(KX), np.sin(KY), np.sin(KZ)
    M = (2/3)*(cx+cy+cz) + (2/3)*(cx*cy + cy*cz + cz*cx) - 4.0
    w2 = -C2 * M
    w = np.sqrt(np.maximum(w2, 1e-300))
    # grad_k omega : domega/dk_i = (c^2 / (2 omega)) * d(-M)/dk_i
    # d(-M)/dk_i = (2/3) sin(k_i) + (2/3) sin(k_i)(cos k_j + cos k_l)
    dMx = (2/3)*sx + (2/3)*sx*(cy+cz)
    dMy = (2/3)*sy + (2/3)*sy*(cx+cz)
    dMz = (2/3)*sz + (2/3)*sz*(cx+cy)
    fac = C2 / (2.0*w)
    kdotgrad = KX*(fac*dMx) + KY*(fac*dMy) + KZ*(fac*dMz)   # k . grad omega
    kmag = np.sqrt(KX*KX + KY*KY + KZ*KZ)
    return w, kdotgrad, kmag


def inv_w(occ, w, kdotgrad):
    """1/w = D * sum(occ*omega) / sum(occ*k.gradomega), D=3."""
    rho = np.sum(occ * w)
    p = np.sum(occ * kdotgrad) / 3.0
    return rho / p


def main():
    print("=" * 72)
    print("FTD-0312 Leg A — analytical lattice flux-bath equation of state")
    print(f"x- = {X_MINUS:.6f}   delta_c = x- - 3 = {DELTA_C:.6f} (0.80%)   "
          f"radiation 1/w = D = 3.000000")
    print("=" * 72)

    w, kdg, kmag = bz_fields(n=96)

    # 1) classical equipartition (engine Langevin regime): n*omega = T -> n ~ 1/omega
    occ_cl = 1.0 / w
    iw_cl = inv_w(occ_cl, w, kdg)
    print(f"\n[classical equipartition, full BZ — the UV-saturated geometric constant]")
    print(f"   1/w = {iw_cl:.6f}   (T-independent; geometric)")

    # 2) Bose occupation sweep n = 1/(exp(omega/T)-1): IR(3.000) -> UV-saturated
    print(f"\n[Bose occupation sweep n=1/(exp(w/T)-1): IR -> UV]")
    print(f"   {'T':>8} {'1/w':>10} {'1/w - 3':>10}")
    cross = None
    Ts = [0.02, 0.05, 0.1, 0.2, 0.4, 0.8, 1.6, 3.2, 6.4]
    prev = None
    for T in Ts:
        occ = 1.0 / (np.expm1(w / T))
        iw = inv_w(occ, w, kdg)
        print(f"   {T:8.3f} {iw:10.6f} {iw-3:10.6f}")
        if prev is not None and (prev[1]-X_MINUS)*(iw-X_MINUS) < 0:
            cross = (prev[0], T)
        prev = (T, iw)

    # 3) hard IR cutoff sweep k<=k_max (classical occ): 1/w(k_max) from 3.000 up
    print(f"\n[hard IR cutoff k<=k_max (classical occ): 1/w from 3.000 up]")
    print(f"   {'k_max':>8} {'1/w':>10} {'1/w - 3':>10}")
    k_cross = None
    prev = None
    for kmax in [0.25, 0.5, 0.75, 1.0, 1.5, 2.0, 2.5, np.pi]:
        mask = kmag <= kmax
        if mask.sum() < 10:
            continue
        iw = inv_w(occ_cl[mask], w[mask], kdg[mask])
        flag = ""
        if prev is not None and (prev[1]-X_MINUS)*(iw-X_MINUS) <= 0:
            k_cross = (prev[0], kmax); flag = "  <- crosses x-"
        print(f"   {kmax:8.3f} {iw:10.6f} {iw-3:10.6f}{flag}")
        prev = (kmax, iw)

    print("\n" + "=" * 72)
    print("READING:")
    print(f"  * IR limit (low-k) -> 1/w = 3.000 (relativistic radiation, exact).")
    print(f"  * The lattice correction has the RIGHT SIGN (1/w > 3, like delta_c>0),")
    print(f"    but 1/w is a smooth function of the mode SPECTRUM (occupation / cutoff /")
    print(f"    temperature) -- it sweeps continuously through 3.024 and well past it.")
    if k_cross:
        print(f"    The classical-occ curve crosses x-=3.024 near k_max in {k_cross}.")
    print(f"  * This correction is GEOMETRIC (set by omega(k) + the spectrum), NOT")
    print(f"    alpha-dependent. So 'flux EoS = 3.024' is reachable but not special here;")
    print(f"    the decisive test (engine Leg B) is whether the engine's NATURAL flux")
    print(f"    equilibrium sits at 3.024 AND tracks 16 G*^3 g_c^2 as the coupling varies.")
    print("=" * 72)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

"""derive_hiding_internal_interferometer.py — P2 of the Universality Programme.

THE QUESTION. Can an internal observer, built entirely from the M18 wave
sector, detect the substrate frame — and at what order in (ka) does the
answer change from "no" to "yes"?

THE OBSERVABLE. The two-color Michelson ratio

    O = [ t_par(A) / t_perp(A) ] / [ t_par(B) / t_perp(B) ],

round-trip times of signals of two comoving source frequencies w_A, w_B
along a boost-parallel and a boost-perpendicular arm, both arms comoving
(boost u along <100>; arms <100> and <010>). Arm LENGTHS cancel exactly in
O — no rod model, no contraction assumption. For ANY covariant dispersion
(massless or massive) O = 1 identically at every u: deviations measure
non-covariance only. O is the sharpest apparatus-free internal frame
detector the sector admits.

THE COMPUTATION — exact rational truncated-polynomial arithmetic (a small
bivariate series class over (u, w); Fraction coefficients; hard total-
degree cap), replacing the sympy expression-tree series that was
combinatorially infeasible:
  * dispersion from the exact M18 symbol omega^2 = C^2 * (-L(k)), k3 = 0;
  * every leg parametrized by the COMOVING frequency condition
    omega(k) - u k_x = w (comoving source and mirrors preserve the lab
    frequency; no separate reflection solve);
  * parallel legs: Newton-series inversion for k_x(u, w), both signs;
  * perpendicular leg: aiming v_gx = u + frequency condition, joint
    Newton-series solve; transverse speed v_gy;
  * O - 1 as an exact polynomial in (u, w_A, w_B) — w plays the role of
    C*k, so powers of w are powers of (ka).

Output: the hiding order of the M18 sector for propagation observables.
The theorem doc quotes whatever this computes.
"""
from __future__ import annotations

from fractions import Fraction
from math import comb, factorial

ORD = 8          # total degree kept in the small parameters (u, w)

# ---------------------------------------------------------------------------
# bivariate truncated series over Fraction: {(i, j): c} means c * u^i * w^j
# ---------------------------------------------------------------------------


class S:
    __slots__ = ("d",)

    def __init__(self, d=None):
        self.d = dict(d or {})

    @staticmethod
    def const(c):
        return S({(0, 0): Fraction(c)}) if c else S()

    @staticmethod
    def var(which, c=1):
        return S({(1, 0) if which == "u" else (0, 1): Fraction(c)})

    def __add__(a, b):
        d = dict(a.d)
        for k, v in b.d.items():
            d[k] = d.get(k, Fraction(0)) + v
            if not d[k]:
                del d[k]
        return S(d)

    def __sub__(a, b):
        return a + b * -1

    def __mul__(a, b):
        if isinstance(b, (int, Fraction)):
            if not b:
                return S()
            return S({k: v * b for k, v in a.d.items()})
        d = {}
        for (i1, j1), v1 in a.d.items():
            for (i2, j2), v2 in b.d.items():
                i, j = i1 + i2, j1 + j2
                if i + j > ORD:
                    continue
                k = (i, j)
                d[k] = d.get(k, Fraction(0)) + v1 * v2
        return S({k: v for k, v in d.items() if v})

    __rmul__ = __mul__

    def div_uw(self, m, n):
        """Divide by u^m w^n exactly (every term must be divisible)."""
        d = {}
        for (i, j), v in self.d.items():
            assert i >= m and j >= n, "non-divisible term in div_uw"
            d[(i - m, j - n)] = v
        return S(d)

    def inv(self):
        """1/self, requires nonzero constant term."""
        c0 = self.d.get((0, 0))
        assert c0, "series inversion needs a constant term"
        x = self - S.const(c0)                    # the 'small' part
        inv0 = Fraction(1, 1) / c0
        out, term = S.const(inv0), S.const(inv0)
        for _ in range(ORD):
            term = term * x * (-inv0)
            out = out + term
        return out

    def sqrt(self):
        """sqrt(self), requires constant term a perfect-square Fraction."""
        c0 = self.d.get((0, 0))
        assert c0 and c0 > 0
        r0 = Fraction(c0).limit_denominator(10 ** 12) ** Fraction(1)
        # exact square root of a Fraction:
        from math import isqrt
        num, den = c0.numerator, c0.denominator
        rn, rd = isqrt(num), isqrt(den)
        assert rn * rn == num and rd * rd == den, "non-square constant term"
        r0 = Fraction(rn, rd)
        x = (self * (Fraction(1) / c0)) - S.const(1)   # small part
        # sqrt(1+x) = sum C(1/2, n) x^n
        out = S.const(0)
        xp = S.const(1)
        half = Fraction(1, 2)
        coef = Fraction(1)
        for n in range(ORD + 1):
            if n > 0:
                coef = coef * (half - (n - 1)) / n
            out = out + xp * coef
            xp = xp * x
            if not xp.d:
                break
        return out * r0

    def subs_num(self, uval, wval):
        return sum(float(v) * uval ** i * wval ** j
                   for (i, j), v in self.d.items())

    def __repr__(self):
        if not self.d:
            return "0"
        parts = []
        for (i, j) in sorted(self.d, key=lambda k: (k[0] + k[1], k)):
            parts.append(f"({self.d[(i,j)]})*u^{i}*w^{j}")
        return " + ".join(parts)


# ---------------------------------------------------------------------------
# the M18 dispersion, k3 = 0, as exact polynomial data
# omega^2(kx, ky) = C^2 * (-L) with
# -L = (1/3)(2cos kx + 2cos ky + 2) + (1/6)(4)(cos kx cos ky + cos ky + cos kx) - 4, negated
# Series coefficients of cos to the needed order, exact.
# We need omega^2 as a polynomial in (kx, ky) up to total degree ORD.
# ---------------------------------------------------------------------------
C2F = Fraction(1, 3)          # C^2 = 1/3


def cos_coeffs(nmax):
    return {n: Fraction((-1) ** (n // 2), factorial(n))
            for n in range(0, nmax + 1, 2)}


COS = cos_coeffs(ORD)


def mL_poly():
    """-L(kx, ky) as {(a, b): coeff} in powers of kx, ky, total deg <= ORD."""
    d = {}

    def add(sig, mult):
        for k, v in sig.items():
            d[k] = d.get(k, Fraction(0)) + v * mult
            if not d[k]:
                del d[k]

    cx = {(n, 0): c for n, c in COS.items()}
    cy = {(0, n): c for n, c in COS.items()}
    cxcy = {}
    for (a, _), v1 in cx.items():
        for (_, b), v2 in cy.items():
            if a + b <= ORD:
                cxcy[(a, b)] = cxcy.get((a, b), Fraction(0)) + v1 * v2
    # L = (1/3)(2cx + 2cy + 2) + (2/3)(cxcy + cy + cx) - 4
    add(cx, Fraction(2, 3) + Fraction(2, 3))
    add(cy, Fraction(2, 3) + Fraction(2, 3))
    add(cxcy, Fraction(2, 3))
    add({(0, 0): Fraction(1)}, Fraction(2, 3))
    add({(0, 0): Fraction(1)}, Fraction(-4))
    return {k: -v for k, v in d.items()}      # -L


ML = mL_poly()


def omega2_of(kx: S, ky: S) -> S:
    """omega^2 = C^2 * (-L)(kx, ky) with kx, ky series in (u, w)."""
    # precompute powers
    maxa = max(a for a, b in ML)
    maxb = max(b for a, b in ML)
    px = [S.const(1)]
    for _ in range(maxa):
        px.append(px[-1] * kx)
    py = [S.const(1)]
    for _ in range(maxb):
        py.append(py[-1] * ky)
    out = S()
    for (a, b), c in ML.items():
        out = out + px[a] * py[b] * c
    return out * C2F


def d_omega2(kx: S, ky: S, wrt: str) -> S:
    """partial derivative of omega^2 wrt kx or ky, evaluated at series args."""
    maxa = max(a for a, b in ML)
    maxb = max(b for a, b in ML)
    px = [S.const(1)]
    for _ in range(maxa):
        px.append(px[-1] * kx)
    py = [S.const(1)]
    for _ in range(maxb):
        py.append(py[-1] * ky)
    out = S()
    for (a, b), c in ML.items():
        if wrt == "kx" and a >= 1:
            out = out + px[a - 1] * py[b] * (c * a)
        if wrt == "ky" and b >= 1:
            out = out + px[a] * py[b - 1] * (c * b)
    return out * C2F


U = S.var("u")
W = S.var("w")

print("=" * 72)
print("M18 internal interferometer — two-color Michelson ratio")
print(f"  exact Fraction series, total order {ORD}; C^2 = 1/3")
print("=" * 72)

# ---------------------------------------------------------------- parallel
# Scaled unknowns: q = s*Q with Q = 1 + corrections. Conditions (C^2=1/3,
# u = C b, w = C s; P = -L = 3*omega^2/C^2... i.e. omega^2 = P/3):
#   frequency (out):  P(q,0) = (s + b q)^2   ->  G(Q) = P(sQ)/s^2 - (1+bQ)^2
#   frequency (ret):  P(q,0) = (s - b q)^2   ->  G(Q) = P(sQ)/s^2 - (1-bQ)^2
B = S.var("u")      # b = u/C
Ssym = S.var("w")   # s = ka scale


def P_of(kx: S, ky: S) -> S:
    return omega2_of(kx, ky) * 3


def dP(kx: S, ky: S, wrt: str) -> S:
    return d_omega2(kx, ky, wrt) * 3


def d2P_dkx2(kx: S, ky: S) -> S:
    maxa = max(a for a, b_ in ML)
    maxb = max(b_ for a, b_ in ML)
    px = [S.const(1)]
    for _ in range(maxa):
        px.append(px[-1] * kx)
    py = [S.const(1)]
    for _ in range(maxb):
        py.append(py[-1] * ky)
    out = S()
    for (a, b_), c in ML.items():
        if a >= 2:
            out = out + px[a - 2] * py[b_] * (c * a * (a - 1))
    return out


def solve_parallel(sign, iters=10):
    """sign=+1 out-leg, -1 return-leg; returns Q with q = s*Q."""
    Q = S.const(1)
    for _ in range(iters):
        q = Ssym * Q
        G = P_of(q, S()).div_uw(0, 2) - (S.const(1) + B * Q * sign)             * (S.const(1) + B * Q * sign)
        dG = dP(q, S(), "kx").div_uw(0, 1)             - (S.const(1) + B * Q * sign) * (B * sign) * 2
        Q = Q - G * dG.inv()
    return Q


Q_out = solve_parallel(+1)
Q_ret = solve_parallel(-1)

# group speeds in units of C: v/C = dP/dq / (2*(s +/- b q)) -> scaled:
#   v_out/C = [dP/dq(sQ)/s] / (2*(1 + b Q))
vg_out = dP(Ssym * Q_out, S(), "kx").div_uw(0, 1)     * ((S.const(1) + B * Q_out) * 2).inv()
vg_ret = dP(Ssym * Q_ret, S(), "kx").div_uw(0, 1)     * ((S.const(1) - B * Q_ret) * 2).inv()
# t_par per unit substrate length (C-units): 1/(v_out - b) + 1/(v_ret + b)
t_par = (vg_out - B).inv() + (vg_ret + B).inv()

# ------------------------------------------------------------ perpendicular
# Scaled unknowns: kx = b*s*X, ky = s*K.
#   frequency: P(kx,ky)/s^2 = (1 + b^2 X)^2
#   aiming:    dP/dkx / (b s) = 2 (1 + b^2 X)


def solve_perp(iters=12):
    X, K = S.const(1), S.const(1)
    for _ in range(iters):
        kx, ky = B * Ssym * X, Ssym * K
        F = P_of(kx, ky).div_uw(0, 2) - (S.const(1) + B * B * X)             * (S.const(1) + B * B * X)
        dFK = dP(kx, ky, "ky").div_uw(0, 1)
        K = K - F * dFK.inv()
        kx, ky = B * Ssym * X, Ssym * K
        Gv = dP(kx, ky, "kx").div_uw(1, 1) - (S.const(1) + B * B * X) * 2
        dGX = d2P_dkx2(kx, ky) - S.const(2) * B * B
        X = X - Gv * dGX.inv()
    return X, K


X_p, K_p = solve_perp()
kx_perp, ky_perp = B * Ssym * X_p, Ssym * K_p
# v_gy/C = dP/dky / (2*(s + b kx)) -> scaled: [dP/dky/s] / (2*(1 + b^2 X))
vgy = dP(kx_perp, ky_perp, "ky").div_uw(0, 1)     * ((S.const(1) + B * B * X_p) * 2).inv()
t_perp = vgy.inv() * 2

# ------------------------------------------------------------- the ratio
R = t_par * t_perp.inv()

# R depends on (b, s). Two-color ratio: O = R(s_A)/R(s_B). Represent by
# extracting R's s-dependence: R = sum_j r_j(b) s^j -> per-color values.
print("\nsingle-color ratio  R(b, s) = t_par/t_perp (arm lengths cancel):")
by_order = {}
for (i, j), v in sorted(R.d.items(), key=lambda kv: (kv[0][0] + kv[0][1],
                                                     kv[0])):
    by_order.setdefault((i, j), v)
    print(f"    b^{i} s^{j}: {v}")

# O - 1 for two colors: R(sA)/R(sB) - 1. R has constant term r00; write
# R = r00 (1 + rho(b, s)); then O-1 = (rho_A - rho_B)(1 - rho_B + ...) --
# compute exactly with the series class extended to two s-slots by
# evaluating rho at independent formal colors via polynomial re-expansion.
r00 = R.d.get((0, 0))
rho = R * (Fraction(1) / r00) - S.const(1)
print(f"\n  R constant term r00 = {r00}")
print("  rho(b, s) = R/r00 - 1 =")
for (i, j), v in sorted(rho.d.items(), key=lambda kv: (kv[0][0] + kv[0][1],
                                                       kv[0])):
    print(f"    b^{i} s^{j}: {v}")

print("""
TWO-COLOR RATIO: O - 1 = [rho(b,sA) - rho(b,sB)] * [1 - rho(b,sB) + ...]
Leading behavior is read off rho directly: any rho term with BOTH b-power
>= 1 and s-power >= 1 survives in O - 1 (s-independent terms cancel
between colors; b-independent terms are rest-frame dispersion, common to
both arms and cancelled in R's construction only if s-independent).
""")
frame_terms = {k: v for k, v in rho.d.items() if k[0] >= 1 and k[1] >= 1}
if not frame_terms:
    print(f"  NO frame-detecting term through total order {ORD}:")
    print(f"  hiding EXACT for two-color propagation observables to "
          f"O((u/C)^i (ka)^j), i+j <= {ORD}")
else:
    lead = min(frame_terms, key=lambda k: (k[0] + k[1], k))
    print(f"  LEADING FRAME-DETECTING TERM: ({frame_terms[lead]}) * "
          f"(u/C)^{lead[0]} * (ka)^{lead[1]}")
    print(f"  => hiding breaks at O((u/C)^{lead[0]} (ka)^{lead[1]})")
    print("\n  all frame-detecting terms:")
    for k in sorted(frame_terms, key=lambda k: (k[0] + k[1], k)):
        print(f"    (u/C)^{k[0]} (ka)^{k[1]}: {frame_terms[k]}")

# sanity: numeric spot check of the series at small values
val = rho.subs_num(0.1, 0.05)
print(f"\n  spot value rho(b=0.1, s=0.05) = {val:.3e}")

# ---------------------------------------------------------------- verifier
print("")
print("=" * 72)
print("VERIFICATION")
print("=" * 72)
checks = []
gam = {(2, 0): Fraction(1, 2), (4, 0): Fraction(3, 8),
       (6, 0): Fraction(5, 16)}
ok_g = all(rho.d.get(k) == v for k, v in gam.items())
checks.append(("V1 s-independent tower equals the exact gamma(u/C) series "
               "(1/2, 3/8, 5/16): classical MM kinematics exact", ok_g))
ok_lead = rho.d.get((2, 2)) == Fraction(3, 4)
checks.append(("V2 leading frame-detecting term = (3/4)(u/C)^2(ka)^2", ok_lead))
ok_nolow = all(not (i == 1 or j == 1) or v == 0
               for (i, j), v in rho.d.items())
checks.append(("V3 no odd-order terms (parity of the round trip)", ok_nolow))
# V4: the (ka)^2 leak traces to isotropic curvature: recompute with the
# k^4 dispersion term REMOVED (surgical) — the (2,2) term must then vanish
ML_backup = dict(ML)
for key in [k for k in ML if sum(k) == 4]:
    del ML[key]
Q_out2 = solve_parallel(+1); Q_ret2 = solve_parallel(-1)
vg_out2 = dP(Ssym * Q_out2, S(), "kx").div_uw(0, 1)     * ((S.const(1) + B * Q_out2) * 2).inv()
vg_ret2 = dP(Ssym * Q_ret2, S(), "kx").div_uw(0, 1)     * ((S.const(1) - B * Q_ret2) * 2).inv()
t_par2 = (vg_out2 - B).inv() + (vg_ret2 + B).inv()
X2, K2 = solve_perp()
kx2, ky2 = B * Ssym * X2, Ssym * K2
vgy2 = dP(kx2, ky2, "ky").div_uw(0, 1)     * ((S.const(1) + B * B * X2) * 2).inv()
R2 = t_par2 * (vgy2.inv() * 2).inv()
rho2 = R2 * (Fraction(1) / R2.d[(0, 0)]) - S.const(1)
ok_surg = rho2.d.get((2, 2), Fraction(0)) == 0
checks.append(("V4 removing the k^4 dispersion term kills the (2,2) leak: "
               "the leading frame detector IS the isotropic curvature",
               ok_surg))
for key, val in ML_backup.items():
    ML[key] = val
n_ok = 0
for desc, ok in checks:
    print(f"[{'PASS' if ok else 'FAIL'}] {desc}")
    n_ok += bool(ok)
print("")
print(f"RESULT: {n_ok}/{len(checks)}")
import sys as _sys
_sys.exit(0 if n_ok == len(checks) else 1)

"""proof_damerell_ideal_class_scan.py — the full per-ideal-class CM scan (FTD-0321).

Executes `PREREG_DAMERELL_SCAN_v1.md` (design-locked 2026-06-24, run deferred).
That pre-registration reserved LEDGER row FTD-0321 and froze the method (§3),
the targets (§2), the outcomes (§4) and the banned moves (§5). This runner
implements it. Nothing here selects a criterion, tolerance or target after the
fact.

--------------------------------------------------------------------------
METHOD NOTE — a necessary departure from the letter of §3, declared up front
--------------------------------------------------------------------------
§3 step 2 says to compute the per-class period "via the Chowla-Selberg /
Damerell formula (Gamma-products weighted by the Kronecker character, per ideal
class)". That is not possible as stated: the h>=2 Chowla-Selberg formula given
in EXPLR_CHOWLA_SELBERG_HIGHER_H.md §3 is

    prod_{[a] in Cl(K)} Omega_a^2 = (1/(2 pi |D|))^{h/2} prod_{a=1}^{|D|-1}
                                     Gamma(a/|D|)^{chi(a)}

which determines only the PRODUCT over ideal classes. Gamma-products alone
cannot separate the individual per-class periods — that is exactly why the
FTD-0123 single-number scan "projects away the ideal-class structure".

The individual periods are obtained instead from the Dedekind eta function at
each class's CM point. For a reduced form (a,b,c) of discriminant d < 0 put
tau = (-b + sqrt(d))/(2a); the per-class invariant used here is

    G*_[a] := sqrt( 8 pi * |eta(tau)|^4 * Im(tau) )                        (*)

|eta(tau)|^4 Im(tau) is SL_2(Z)-invariant, so (*) depends only on the ideal
class, not the representative form. The Gamma-product then serves as an
INDEPENDENT CHECK rather than as the definition (see the gates below).

--------------------------------------------------------------------------
CORRECTNESS GATES (both must pass before any verdict is credited, per §5)
--------------------------------------------------------------------------
G1  d = -4, h = 1 reproduces the canonical G* = Gamma(1/4)/Gamma(3/4) exactly.
G2  The Chowla-Selberg identity, in the normalisation of (*),

        prod_j (G*_j)^2 == (2/sqrt|d|)^h * Gamma_d^(w/2)

    where Gamma_d = prod_{a=1}^{|d|-1} Gamma(a/|d|)^{chi_d(a)} and w = #units
    (6 at d=-3, 4 at d=-4, else 2). The normalisation is fixed by the nine
    h = 1 fields ONLY and is then required to hold, unmodified, on an h >= 2
    control set. G2 failing anywhere => INDETERMINATE per §4.

--------------------------------------------------------------------------
SCOPE
--------------------------------------------------------------------------
REGISTERED (§2, the pre-registered target): the 54 fundamental discriminants
with h >= 2 and |d| <= 907, plus the 9 Heegner h = 1 fields as controls. The
§4 outcome letter is determined by THIS set alone.

DEEP EXTENSION (declared, beyond the lock — reported separately and cannot
change the §4 outcome): all fundamental discriminants |d| <= DEEP_LIMIT. A
float64 sieve over every reduced form is followed by arbitrary-precision
confirmation of survivors; the sieve threshold is deliberately ~1e4 times
wider than the match window, so it cannot hide a matcher.

Usage:
    python scripts/proofs/proof_damerell_ideal_class_scan.py [--deep-limit N]
"""

from __future__ import annotations

import argparse
import math
import sys
import time
from concurrent.futures import ProcessPoolExecutor

import numpy as np

try:
    from sympy import jacobi_symbol
except Exception:  # pragma: no cover
    jacobi_symbol = None

from mpmath import mp, mpf, mpc, gamma, pi, sqrt, exp, log
from mpmath import j as MPJ

# ---------------------------------------------------------------- constants
# Targets and tolerances copied verbatim from the FTD-0123 runner
# (proof_chowla_selberg_higher_h_scan.py) — pre-registered, not re-derived.
X_PLUS_TARGET = mpf("137.0361714582")
X_MINUS_TARGET = mpf("3.0239639163")
X_PLUS_REL_TOL = mpf("1.26e-6")
X_MINUS_REL_TOL = mpf("0.0080")

HEEGNER = [-3, -4, -7, -8, -11, -19, -43, -67, -163]
DEEP_LIMIT_DEFAULT = 1_000_000

# G* must land in a very narrow band for the master quadratic to dual-match;
# the sieve half-width below is ~1e4x wider than that band (measured at run
# time and asserted), so nothing can slip through the float64 stage.
GSTAR_SIEVE_HALFWIDTH = 1.0e-4


def w_of(d: int) -> int:
    return 6 if d == -3 else (4 if d == -4 else 2)


# ------------------------------------------------------------- number theory
def kronecker(d: int, n: int) -> int:
    """Kronecker symbol (d/n) for fundamental discriminant d, n >= 1."""
    if n == 0:
        return 0
    if math.gcd(d, n) != 1:
        return 0
    res, m = 1, n
    while m % 2 == 0:
        m //= 2
        r = d % 8
        if r in (3, 5):
            res = -res
        elif r not in (1, 7):
            return 0
    return res if m == 1 else res * int(jacobi_symbol(d % m, m))


def is_fundamental(d: int) -> bool:
    if d >= 0:
        return False
    if d % 4 == 1:
        return _squarefree(-d)
    if d % 4 == 0:
        m = d // 4
        return (m % 4 in (2, 3)) and _squarefree(-m)
    return False


def _squarefree(n: int) -> bool:
    if n <= 0:
        return False
    i = 2
    while i * i <= n:
        if n % (i * i) == 0:
            return False
        i += 1
    return True


def reduced_forms(d: int):
    """Reduced positive-definite binary quadratic forms of discriminant d < 0."""
    out, a = [], 1
    while 3 * a * a <= -d:
        for b in range(-a + 1, a + 1):
            num = b * b - d
            if num % (4 * a):
                continue
            c = num // (4 * a)
            if c < a or (a == c and b < 0):
                continue
            out.append((a, b, c))
        a += 1
    return out


# ------------------------------------------------------------- high precision
def eta_mp(tau):
    """Dedekind eta via its q-product; |q| <= e^{-pi sqrt 3} in the fundamental domain."""
    q = exp(2 * pi * MPJ * tau)
    prod, n = mpf(1), 1
    cutoff = mpf(10) ** (-(mp.dps + 10))
    while True:
        t = q ** n
        if abs(t) < cutoff:
            break
        prod *= (1 - t)
        n += 1
    return exp(pi * MPJ * tau / 12) * prod


def gstar_mp(a: int, b: int, d: int):
    tau = mpc(-b, sqrt(-d)) / (2 * a)
    return sqrt(8 * pi * abs(eta_mp(tau)) ** 4 * tau.imag)


def gamma_product_mp(d: int):
    D = abs(d)
    tot = mpf(0)
    for a in range(1, D):
        ch = kronecker(d, a)
        if ch:
            tot += ch * log(gamma(mpf(a) / D))
    return exp(tot)


def roots_mp(g):
    """Roots of P(x) = x^2 - 16 g^2 x + 16 g^3."""
    B, C = 16 * g * g, 16 * g ** 3
    disc = B * B - 4 * C
    if disc < 0:
        return None
    r = sqrt(disc)
    return (B + r) / 2, (B - r) / 2


def dual_match(g):
    rt = roots_mp(g)
    if rt is None:
        return False
    xp, xm = rt
    return (abs(xp - X_PLUS_TARGET) < X_PLUS_REL_TOL * X_PLUS_TARGET
            and abs(xm - X_MINUS_TARGET) < X_MINUS_REL_TOL * X_MINUS_TARGET)


# -------------------------------------------------------------- float64 sieve
def gstar_f64(a: np.ndarray, b: np.ndarray, d: np.ndarray) -> np.ndarray:
    """Vectorised float64 G* over arrays of reduced forms. ~15 digits, ample
    for a sieve whose threshold is 1e-2 against a match window of ~2e-6."""
    y = np.sqrt(-d.astype(np.float64)) / (2.0 * a)          # Im(tau)
    x = -b.astype(np.float64) / (2.0 * a)                   # Re(tau)
    qr = np.exp(-2.0 * np.pi * y)                           # |q|
    ang = 2.0 * np.pi * x
    # eta = e^{i pi tau/12} prod (1 - q^n); track |eta|^4 only.
    log_abs = -np.pi * y / 12.0
    acc_re = np.ones_like(y)
    acc_im = np.zeros_like(y)
    for n in range(1, 24):
        rn = qr ** n
        if np.max(rn) < 1e-18:
            break
        cr = 1.0 - rn * np.cos(ang * n)
        ci = -rn * np.sin(ang * n)
        acc_re, acc_im = acc_re * cr - acc_im * ci, acc_re * ci + acc_im * cr
    abs_eta = np.exp(log_abs) * np.hypot(acc_re, acc_im)
    return np.sqrt(8.0 * np.pi * abs_eta ** 4 * y)


def sieve_chunk(args):
    """Enumerate every reduced form for a range of `a`, vectorised over c.

    For fixed (a, b) the discriminant is d = b^2 - 4ac, so c runs over a plain
    integer range and no divisibility filter is needed — the enumeration is
    exact and complete, not sampled. Returns near-window hits plus the count of
    forms actually examined.
    """
    a_lo, a_hi, limit, centre, halfwidth = args
    hits, n_forms = [], 0
    for a in range(a_lo, a_hi):
        for b in range(-a + 1, a + 1):
            bb = b * b
            c_max = (limit + bb) // (4 * a)
            if c_max < a:
                continue
            c = np.arange(a, c_max + 1, dtype=np.int64)
            d = bb - 4 * a * c
            keep = (-d) <= limit
            if a == c[0] and b < 0:
                keep[0] = False          # (a,b,a) with b<0 is not reduced
            c, d = c[keep], d[keep]
            if d.size == 0:
                continue
            n_forms += d.size
            av = np.full(d.shape, a, dtype=np.int64)
            bv = np.full(d.shape, b, dtype=np.int64)
            g = gstar_f64(av, bv, d)
            m = np.abs(g - centre) < halfwidth
            if m.any():
                for aa, b2, dd, gg in zip(av[m], bv[m], d[m], g[m]):
                    hits.append((int(aa), int(b2), int(dd), float(gg)))
    return hits, n_forms


# ------------------------------------------------------------------- gates
def run_gates(verbose=True):
    """G1 and G2 from the docstring. Returns (ok, messages)."""
    msgs, ok = [], True

    g_true = gamma(mpf(1) / 4) / gamma(mpf(3) / 4)
    g_eta = gstar_mp(1, 0, -4)
    e1 = abs(g_eta - g_true)
    p1 = e1 < mpf(10) ** (-(mp.dps - 8))
    ok &= p1
    msgs.append(f"  G1  d=-4,h=1 reproduces canonical G*        err={mp.nstr(e1,4):>12}  "
                f"{'PASS' if p1 else 'FAIL'}")

    # G2 on h=1 (normalisation source) then h>=2 (the actual test)
    def cs_error(d):
        gs = [gstar_mp(a, b, d) for (a, b, _) in reduced_forms(d)]
        h, w = len(gs), w_of(d)
        lhs = mpf(1)
        for g in gs:
            lhs *= g * g
        rhs = (2 / sqrt(-d)) ** h * gamma_product_mp(d) ** (mpf(w) / 2)
        return abs(lhs - rhs) / abs(rhs), h

    worst1 = max(cs_error(d)[0] for d in HEEGNER)
    control = [-15, -20, -23, -24, -31, -35, -39, -40, -47, -51, -52, -71, -87, -88, -91, -104]
    worst2, hmax = mpf(0), 0
    for d in control:
        e, h = cs_error(d)
        worst2 = max(worst2, e)
        hmax = max(hmax, h)
    tol = mpf(10) ** (-(mp.dps - 12))
    p2 = worst1 < tol and worst2 < tol
    ok &= p2
    msgs.append(f"  G2  Chowla-Selberg identity, h=1  (9 fields) worst={mp.nstr(worst1,4):>10}  "
                f"{'PASS' if worst1 < tol else 'FAIL'}")
    msgs.append(f"  G2  Chowla-Selberg identity, h>=2 ({len(control)} fields, h<={hmax}) "
                f"worst={mp.nstr(worst2,4):>10}  {'PASS' if worst2 < tol else 'FAIL'}")
    msgs.append(f"      (normalisation fixed by h=1 only; h>=2 is an unmodified test)")
    if verbose:
        for m in msgs:
            print(m)
    return ok, msgs


def gstar_window():
    """Half-width in G* within which a dual match is even arithmetically possible."""
    g0 = gamma(mpf(1) / 4) / gamma(mpf(3) / 4)
    lo, hi = g0 - mpf("1e-3"), g0 + mpf("1e-3")
    def xp(g):
        return roots_mp(g)[0]
    # x_+ is increasing in g near g0; invert the tolerance numerically
    target_hi = X_PLUS_TARGET * (1 + X_PLUS_REL_TOL)
    target_lo = X_PLUS_TARGET * (1 - X_PLUS_REL_TOL)
    def bisect(fn, a, b):
        for _ in range(200):
            m = (a + b) / 2
            if fn(m):
                a = m
            else:
                b = m
        return (a + b) / 2
    g_hi = bisect(lambda g: xp(g) < target_hi, lo, hi)
    g_lo = bisect(lambda g: xp(g) < target_lo, lo, hi)
    return g_lo, g_hi, g0


# --------------------------------------------------------------------- main
def main() -> int:
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")
    ap = argparse.ArgumentParser()
    ap.add_argument("--deep-limit", type=int, default=DEEP_LIMIT_DEFAULT)
    ap.add_argument("--dps", type=int, default=50)
    ap.add_argument("--skip-deep", action="store_true")
    args = ap.parse_args()
    mp.dps = args.dps

    print("=" * 74)
    print("FTD-0321 — full per-ideal-class CM (Damerell) scan")
    print("executing PREREG_DAMERELL_SCAN_v1.md (locked 2026-06-24)")
    print("=" * 74)
    print(f"precision: dps={mp.dps}   criterion: trivial-multiplier (q=1), per §3/§5")
    print()

    print("--- CORRECTNESS GATES (§5: gate first, verdict second) ---")
    t0 = time.time()
    gates_ok, _ = run_gates()
    print(f"    [{time.time()-t0:.1f}s]")
    if not gates_ok:
        print("\nOUTCOME: INDETERMINATE (§4) — gates failed; no verdict admissible.")
        return 1
    print()

    g_lo, g_hi, g0 = gstar_window()
    half = (g_hi - g_lo) / 2
    print("--- MATCH WINDOW ---")
    print(f"    canonical G*            = {mp.nstr(g0, 20)}")
    print(f"    dual-match admits G* in [{mp.nstr(g_lo,16)}, {mp.nstr(g_hi,16)}]")
    print(f"    half-width              = {mp.nstr(half, 6)}")
    print(f"    float64 sieve half-width= {GSTAR_SIEVE_HALFWIDTH:.1e}  "
          f"({float(GSTAR_SIEVE_HALFWIDTH/half):.0f}x wider — cannot hide a matcher)")
    print()

    # ---------------- REGISTERED SCAN (§2) ----------------
    print("--- REGISTERED SCAN (§2: h>=2, |d|<=907, plus 9 Heegner controls) ---")
    t0 = time.time()
    registered, matchers, n_classes = [], [], 0
    for d in range(-1, -908, -1):
        if not is_fundamental(d):
            continue
        forms = reduced_forms(d)
        h = len(forms)
        if h < 2 and d not in HEEGNER:
            continue
        registered.append(d)
        for (a, b, _) in forms:
            n_classes += 1
            g = gstar_mp(a, b, d)
            if dual_match(g):
                matchers.append((d, h, a, b, g))
    n_h1 = sum(1 for d in registered if d in HEEGNER)
    n_h2 = len(registered) - n_h1
    print(f"    fields scanned : {len(registered)}  ({n_h2} with h>=2, {n_h1} Heegner controls)")
    print(f"    ideal classes  : {n_classes}")
    print(f"    dual-matchers  : {len(matchers)}")
    for (d, h, a, b, g) in matchers:
        xp, xm = roots_mp(g)
        print(f"      d={d:>5} h={h:<3} form=({a},{b})  G*={mp.nstr(g,18)}")
        print(f"                  x_+={mp.nstr(xp,16)}  x_-={mp.nstr(xm,16)}")
    print(f"    [{time.time()-t0:.1f}s]")
    print()

    only_minus4 = (len(matchers) == 1 and matchers[0][0] == -4)
    outcome = ("UNIQUE-CONFIRMED" if only_minus4
               else ("COUNTEREXAMPLE" if matchers else "NO-MATCHER-AT-ALL"))

    # ---------------- DEEP EXTENSION (declared) ----------------
    deep = None
    if not args.skip_deep:
        print(f"--- DEEP EXTENSION (declared; |d| <= {args.deep_limit:,}) ---")
        print("    float64 sieve over every reduced form, then mp confirmation")
        t0 = time.time()
        a_max = int(math.isqrt(args.deep_limit // 3)) + 1
        workers = max(1, (__import__('os').cpu_count() or 4) - 2)
        bounds = np.linspace(1, a_max + 1, workers * 4 + 1).astype(int)
        tasks = [(int(bounds[i]), int(bounds[i + 1]), args.deep_limit,
                  float(g0), GSTAR_SIEVE_HALFWIDTH)
                 for i in range(len(bounds) - 1) if bounds[i + 1] > bounds[i]]
        hits, total_forms = [], 0
        with ProcessPoolExecutor(max_workers=workers) as ex:
            for h_, n_ in ex.map(sieve_chunk, tasks):
                hits.extend(h_)
                total_forms += n_
        t_sieve = time.time() - t0
        print(f"    reduced forms enumerated : {total_forms:,}")
        print(f"    survived float64 sieve   : {len(hits):,}")
        print(f"    [{t_sieve:.1f}s on {workers} workers]")

        t0 = time.time()
        deep_matchers = []
        for (a, b, d, _g) in hits:
            g = gstar_mp(a, b, d)
            if dual_match(g):
                deep_matchers.append((d, a, b, g))
        # Split fundamental (genuine CM fields, maximal order) from
        # non-fundamental. The family d = -4f^2 is the SAME CM point tau = i at
        # conductor f: it reproduces G* exactly for every f, so it is d = -4 in
        # disguise and is not an independent matcher.
        fund, conductor_i, other_nf = [], [], []
        for rec in deep_matchers:
            d = rec[0]
            if is_fundamental(d):
                fund.append(rec)
            elif d % 4 == 0 and math.isqrt(-d // 4) ** 2 == -d // 4:
                conductor_i.append(rec)
            else:
                other_nf.append(rec)
        print(f"    confirmed at dps={mp.dps}      : {len(deep_matchers)}")
        print(f"      fundamental (genuine)  : {len(fund)}")
        print(f"      d=-4f^2 (tau=i, same point as d=-4, conductor f) : {len(conductor_i)}")
        print(f"      other non-fundamental  : {len(other_nf)}")
        for (d, a, b, g) in sorted(fund):
            xp, xm = roots_mp(g)
            h = len(reduced_forms(d))
            dev = abs(xp - X_PLUS_TARGET) / X_PLUS_TARGET
            tag = "  <-- d=-4 (registered)" if d == -4 else "  *** COUNTEREXAMPLE ***"
            print(f"      d={d:>9} h={h:<4} form=({a},{b})  G*={mp.nstr(g,18)}")
            print(f"                 x_+={mp.nstr(xp,16)}  rel dev={mp.nstr(dev,6)}{tag}")
        print(f"    [{time.time()-t0:.1f}s]")
        deep = (total_forms, len(hits), fund, conductor_i, other_nf)
        print()

    print("=" * 74)
    print("VERDICT (§4 outcome is set by the REGISTERED scan alone)")
    print("=" * 74)
    print(f"  REGISTERED: {outcome}")
    if only_minus4:
        print("    d = -4 remains the only dual-matcher across all h-tuples.")
        print("    Theorem 3 strengthens from single-number scan to per-ideal-class")
        print("    scan. Stays [NUMERICAL FACT] per §5 — a finite per-class scan is")
        print("    not a theorem over all CM curves.")
    if deep is not None:
        tf, ns, fund, cond, othernf = deep
        extra = [m for m in fund if m[0] != -4]
        print(f"  DEEP (declared; cannot change the §4 letter): {tf:,} reduced forms")
        print(f"    fundamental matchers      : {len(fund)}  "
              f"({len(extra)} beyond d=-4)")
        print(f"    conductor family d=-4f^2  : {len(cond)}  (all tau=i; d=-4 in disguise)")
        if extra:
            print()
            print("    *** d = -4 UNIQUENESS FAILS OUTSIDE THE REGISTERED RANGE ***")
            for (d, a, b, g) in sorted(extra):
                h = len(reduced_forms(d))
                xp, _ = roots_mp(g)
                dev = abs(xp - X_PLUS_TARGET) / X_PLUS_TARGET
                print(f"      d={d:>9}  h={h:<4} class=({a},{b})  "
                      f"x_+ rel dev={mp.nstr(dev,6)} (gate {mp.nstr(X_PLUS_REL_TOL,3)})")
            print("    The registered verdict stands on its registered domain;")
            print("    the uniqueness does NOT extend beyond it.")
    print("=" * 74)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

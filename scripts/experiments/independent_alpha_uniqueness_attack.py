"""Independent adversarial recomputation of FTD-0791 / FTD-0802.

Shares NO code with the repo's scan or verify scripts. Rebuilds both
families from their frozen definitions, recomputes every load-bearing
number, and uses DIFFERENT seeds / windows where randomness enters.
Refute-by-default in both directions: a defect in the audits counts as
much as a defect in the original scans.
"""
import numpy as np
from fractions import Fraction
from math import gcd
import mpmath as mp

mp.mp.dps = 60

# ---------- exact arithmetic layer ------------------------------------
GSTAR_MP = mp.gamma(mp.mpf(1) / 4) / mp.gamma(mp.mpf(3) / 4)
B_MP = 16 * GSTAR_MP ** 2
C_MP = 16 * GSTAR_MP ** 3
DISC = B_MP * B_MP - 4 * C_MP
XP_MP = (B_MP + mp.sqrt(DISC)) / 2
XM_MP = (B_MP - mp.sqrt(DISC)) / 2
CODATA = mp.mpf("137.035999177")
print("=== SECTION 0: exact arithmetic (mpmath, 60 dps) ===")
print(f"G*        = {mp.nstr(GSTAR_MP, 20)}")
print(f"x_+       = {mp.nstr(XP_MP, 20)}")
print(f"x_-       = {mp.nstr(XM_MP, 20)}")
print(f"resid vs CODATA 1/a  = {mp.nstr(abs(XP_MP - CODATA) / CODATA, 8)}")
print(f"resid x_- vs 3       = {mp.nstr(abs(XM_MP - 3) / 3, 8)}")
print(f"claim-root const 137.0361714582 dev = {mp.nstr(abs(XP_MP - mp.mpf('137.0361714582')), 5)}")

G = float(GSTAR_MP)
ALPHA_INV = 137.035999177
N_C = 3.0

# ---------- FTD-0319 family (18-constant basket, frozen def) ----------
print()
print("=== SECTION 1: FTD-0319 family rebuilt from frozen definition ===")
mp.mp.dps = 40
g = mp.gamma
consts = {
    "G_star": G,
    "pi": float(mp.pi), "e": float(mp.e), "sqrt2": float(mp.sqrt(2)),
    "sqrt3": float(mp.sqrt(3)), "sqrt5": float(mp.sqrt(5)),
    "golden_phi": float((1 + mp.sqrt(5)) / 2), "euler_gamma": float(mp.euler),
    "ln2": float(mp.log(2)), "apery_zeta3": float(mp.zeta(3)),
    "catalan": float(mp.catalan),
    "varpi_lemn": float(g(mp.mpf(1) / 4) ** 2 / (2 * mp.sqrt(2 * mp.pi))),
    "gauss_G": float(1 / mp.agm(1, mp.sqrt(2))),
    "sqrt_pi": float(mp.sqrt(mp.pi)), "gamma_1_3": float(g(mp.mpf(1) / 3)),
    "R3_equianh": float(g(mp.mpf(1) / 3) / g(mp.mpf(2) / 3)),
    "khinchin": float(mp.khinchin), "glaisher": float(mp.glaisher),
}
cvals = np.arange(1, 65, dtype=np.float64)
evals = np.arange(0, 6, dtype=np.float64)

xp_l, xm_l, who = [], [], []
total = 0
for name, K in consts.items():
    coef = (cvals[:, None] * (K ** evals)[None, :]).ravel()      # 384 values
    b = np.repeat(coef, coef.size)
    cc = np.tile(coef, coef.size)
    total += b.size
    d = b * b - 4 * cc
    ok = d >= 0
    r = np.sqrt(d[ok])
    xp_l.append((b[ok] + r) / 2)
    xm_l.append((b[ok] - r) / 2)
    who.append(np.full(ok.sum(), name == "G_star"))
xp = np.concatenate(xp_l)
xm = np.concatenate(xm_l)
isg = np.concatenate(who)
print(f"total polynomials {total:,} | real-rooted {xp.size:,}")

rp = np.abs(xp - ALPHA_INV) / ALPHA_INV
rm = np.abs(xm - N_C) / N_C

print("leg-1-only counts (vs CODATA):")
for tol in (1e-3, 1e-4, 1e-5, 2e-6, 1.2572e-6):
    print(f"  tol {tol:.4e}: {int((rp < tol).sum())}")

dual = (rp < 2e-6) & (rm < 1e-2)
print(f"dual-matchers at registered gate (2e-6, 1e-2): {int(dual.sum())}")
print(f"  of which G*-family: {int((dual & isg).sum())}, "
      f"non-G*: {int((dual & ~isg).sum())}")

near = (xp > 136) & (xp < 138)
dens = near.sum() / 2.0
win = 2 * 2e-6 * ALPHA_INV
print(f"roots in [136,138]: {int(near.sum()):,} -> density {dens:.1f}/unit")
print(f"analytic null at gate: {dens * win:.3f} expected")

pool = rm < 1e-2
xp_pool = xp[pool]
near_p = (xp_pool > 130) & (xp_pool < 145)
dens_p = near_p.sum() / 15.0
print(f"x_- gated pool: {int(pool.sum()):,} | gated density [130,145]: "
      f"{dens_p:.2f}/unit -> dual null {dens_p * win:.2e}")
near_p2 = (xp_pool > 136) & (xp_pool < 138)
print(f"  (gated density [136,138]: {near_p2.sum()/2.0:.2f}/unit -> "
      f"dual null {near_p2.sum()/2.0 * win:.2e})")

# rank order among gated pool
order = np.argsort(np.abs(xp_pool - ALPHA_INV))
print("top-3 gated by |x_+ - CODATA|:")
for i in order[:3]:
    print(f"  x_+ = {xp_pool[i]:.6f}  resid {abs(xp_pool[i]-ALPHA_INV)/ALPHA_INV:.4e}")

# counterfactual: x_- within 1% of ANY integer 1..10
ints = np.arange(1, 11, dtype=np.float64)
anyint = np.min(np.abs(xm[:, None] - ints[None, :]) / ints[None, :], axis=1) < 1e-2
print(f"counterfactual any-integer leg-2 at gate 2e-6: "
      f"{int(((rp < 2e-6) & anyint).sum())} (vs x_-~3 only: {int(dual.sum())})")

# ---------- my own MC, different seed AND different windows -----------
print()
print("=== SECTION 2: independent Monte Carlo (seed 424242) ===")
rng = np.random.default_rng(424242)
xp_sorted = np.sort(xp)
xp_pool_sorted = np.sort(xp_pool)

for lo, hi, label in ((110.0, 170.0, "U(110,170)  [their window]"),
                      (130.0, 145.0, "U(130,145)  [narrower probe]")):
    t = rng.uniform(lo, hi, 20000)
    # single leg, relative 2e-6 window around each target
    L = np.searchsorted(xp_sorted, t * (1 - 2e-6))
    R = np.searchsorted(xp_sorted, t * (1 + 2e-6))
    hits = R - L
    # dual: same but in the x_- gated pool
    Lp = np.searchsorted(xp_pool_sorted, t * (1 - 2e-6))
    Rp = np.searchsorted(xp_pool_sorted, t * (1 + 2e-6))
    dhits = Rp - Lp
    print(f"{label}: single-leg mean {hits.mean():.3f}  P(>=1) {np.mean(hits>0):.3f}"
          f" | dual mean {dhits.mean():.5f}  P(>=1) {np.mean(dhits>0):.5f}")

# ---------- OT-3.3 EXT-A family ---------------------------------------
print()
print("=== SECTION 3: OT-3.3 EXT-A rebuilt from frozen definition ===")
XPT = 137.0361714582
XMT = 3.0239639163
TOLP = 1.26e-6 * XPT      # absolute
TOLM = 0.0080 * XMT       # absolute

n_arr = np.arange(1, 65)
d_arr = np.arange(1, 5)
frac = (n_arr[:, None] / d_arr[None, :]).ravel()               # 256 rationals
pw = G ** np.arange(0, 6)
coefA = (frac[:, None] * pw[None, :]).ravel()                  # 1536 values
bA = np.repeat(coefA, coefA.size)
cA = np.tile(coefA, coefA.size)
print(f"nominal EXT-A size: {bA.size:,}")

# distinct polynomials after exact fraction reduction
red = {}
for n in range(1, 65):
    for d in range(1, 5):
        f = Fraction(n, d)
        red[(n, d)] = (f.numerator, f.denominator)
uniq = set()
for n in range(1, 65):
    for d in range(1, 5):
        uniq.add(red[(n, d)])
n_uniq_coeff = len(uniq)
print(f"distinct rationals n/d: {n_uniq_coeff} of 256 "
      f"-> distinct polynomials {(n_uniq_coeff*6)**2:,} "
      f"(aliasing x{bA.size / (n_uniq_coeff*6)**2:.4f})")

dd = bA * bA - 4 * cA
okA = dd >= 0
rr = np.sqrt(dd[okA])
xpA = (bA[okA] + rr) / 2
xmA = (bA[okA] - rr) / 2
legp = np.abs(xpA - XPT) < TOLP
both = legp & (np.abs(xmA - XMT) < TOLM)
print(f"registered gate (claim-centred): pass x_+ leg {int(legp.sum())}, "
      f"pass both {int(both.sum())}  -> x_- leg removed {int(legp.sum()-both.sum())}")

# are all passers the master in disguise?
idx = np.where(both)[0]
ok_idx = np.where(okA)[0][idx]
masters = 0
others = []
for k in ok_idx:
    i_b, i_c = divmod(int(k), coefA.size)
    fi, pi_ = divmod(i_b, 6)
    ni, di = divmod(fi, 4)
    fj, qj = divmod(i_c, 6)
    nj, dj = divmod(fj, 4)
    nb, db = red[(ni + 1, di + 1)]
    nc, dc = red[(nj + 1, dj + 1)]
    if (nb, db, pi_, nc, dc, qj) == (16, 1, 2, 16, 1, 3):
        masters += 1
    else:
        others.append((nb, db, pi_, nc, dc, qj))
print(f"passers reducing to the master quadratic (16*G^2, 16*G^3): {masters}"
      f" | reducing to anything else: {len(others)} {others[:5]}")

# ---------- OT-3.3 displaced-pair null, my own seed -------------------
print()
print("=== SECTION 4: EXT-A displaced-pair null (seed 987654321) ===")
srt = np.argsort(xpA)
xpS, xmS = xpA[srt], xmA[srt]
rng2 = np.random.default_rng(987654321)
tp = rng2.uniform(110.0, 170.0, 20000)
tm = rng2.uniform(2.0, 4.5, 20000)
Lx = np.searchsorted(xpS, tp - TOLP)
Rx = np.searchsorted(xpS, tp + TOLP)
counts = np.zeros(20000)
for i in range(20000):
    if Rx[i] > Lx[i]:
        seg = xmS[Lx[i]:Rx[i]]
        counts[i] = np.count_nonzero(np.abs(seg - tm[i]) < TOLM)
print(f"dual matchers under displaced pairs: mean {counts.mean():.5f}  "
      f"P(>=1) {np.mean(counts > 0):.5f}")
single = (Rx - Lx)
print(f"x_+ leg alone under displaced x_+: mean {single.mean():.3f}  "
      f"P(>=1) {np.mean(single > 0):.3f}")

# ---------- EXT-B cubic scan, full, independent method ----------------
print()
print("=== SECTION 5: EXT-B cubic family, full independent rescan ===")
# family: x^3 - n2 G^p2 x^2 + n1 G^p1 x - n0 G^p0, n in [1,16], p in [0,4]
nv = np.arange(1, 17, dtype=np.float64)
pv = G ** np.arange(0, 5)
cf = (nv[:, None] * pv[None, :]).ravel()                       # 80 values
A2 = np.repeat(np.repeat(cf, cf.size), cf.size)
A1 = np.tile(np.repeat(cf, cf.size), cf.size)
A0 = np.tile(cf, cf.size * cf.size)
print(f"cubic family size: {A2.size:,}")

# necessary conditions: |P(XPT)| and |P(XMT)| small
PXp = XPT**3 - A2 * XPT**2 + A1 * XPT - A0
PXm = XMT**3 - A2 * XMT**2 + A1 * XMT - A0
cand = (np.abs(PXp) < 1.0e4) & (np.abs(PXm) < 1.0e3)
print(f"prefilter survivors: {int(cand.sum())}")
found = []
for k in np.where(cand)[0]:
    roots = np.roots([1.0, -A2[k], A1[k], -A0[k]])
    rl = sorted((z.real for z in roots if abs(z.imag) < 1e-9), reverse=True)
    hit = False
    for i in range(len(rl)):
        for j in range(i + 1, len(rl)):
            if abs(rl[i] - XPT) < TOLP and abs(rl[j] - XMT) < TOLM:
                hit = True
    if hit:
        found.append((A2[k], A1[k], A0[k]))
print(f"cubic dual-matchers (exact root check): {len(found)}")
B16, C16 = 16 * G**2, 16 * G**3
for a2, a1, a0 in found:
    is_xPform = abs(a2 - B16) < 1e-9 and abs(a1 - C16) < 1e-9
    # exact remainder of division by master quadratic: q0 = B - a2
    q0 = a2 - B16
    r1 = a1 - C16 - q0 * B16
    r0 = -a0 + q0 * C16
    print(f"  a2={a2:.6f} a1={a1:.6f} a0={a0:.6f} | x*P(x)-a0 form: {is_xPform}"
          f" | remainder ({r1:.3e})x + ({r0:.6f})  nonzero: {abs(r0) > 1e-9 or abs(r1) > 1e-9}")

# would a0 = 4 have passed? (gate boundary check)
for a0t in (1.0, 2.0, 3.0, G, 4.0, 2*G):
    roots = np.roots([1.0, -B16, C16, -a0t])
    rl = sorted((z.real for z in roots if abs(z.imag) < 1e-9), reverse=True)
    dev = min((abs(r - XPT) for r in rl), default=np.inf)
    print(f"  x*P(x)-{a0t:.4f}: nearest-root |dev| from x_+ target = {dev:.4e} "
          f"(gate {TOLP:.4e}) -> {'PASS' if dev < TOLP else 'FAIL'}")
print()
print("done.")

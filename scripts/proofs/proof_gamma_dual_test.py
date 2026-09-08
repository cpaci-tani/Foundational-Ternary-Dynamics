"""proof_gamma_dual_test.py -- executes PREREG_GAMMA_DUAL_TEST_v1.md.

Fix the relation 16 c^2 alpha (1 - c alpha) = 1; let CODATA alpha DEFINE c*;
generate Gamma-structured constants by declared rules; ask at what complexity a
hit as good as G* = Gamma(1/4)/Gamma(3/4) becomes expected.

G* is one catalog member among equals. The target never sees G*. Catalogs are
deduplicated by VALUE (Gamma identities make many expressions for one number).
Every reported number is computed here.
"""
from __future__ import annotations
import hashlib
import itertools
import math
import sys
from fractions import Fraction
from pathlib import Path

import numpy as np
from mpmath import mp, mpf, gamma, findroot

mp.dps = 30
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations/spine_master_quadratic/PREREG_GAMMA_DUAL_TEST_v1.md"


def sha(p: Path) -> str:
    return hashlib.sha256(p.read_bytes()).hexdigest()[:16]


def rule(t):
    print("\n" + "=" * 78 + f"\n{t}\n" + "=" * 78)


rule("0. LOCK")
print(f"  prereg sha256[:16] = {sha(PREREG)}")
print(f"  runner sha256[:16] = {sha(Path(__file__))}")

# ------------------------------------------------------------------ target
rule("1. TARGET  (from CODATA alpha only)")
ALPHA_INV = mpf("137.035999177")
a = 1 / ALPHA_INV
N_INT = 16


def c_star(n: int) -> float:
    """lower root of n c^2 a (1 - c a) = 1, via bracketed bisection on (0, 2/(3a))."""
    f = lambda c: n * c ** 2 * a * (1 - c * a) - 1
    lo, hi = mpf("1e-9"), 2 / (3 * a)
    if f(hi) < 0:
        return float("nan")
    for _ in range(200):
        mid = (lo + hi) / 2
        if f(mid) < 0:
            lo = mid
        else:
            hi = mid
    return float((lo + hi) / 2)


CSTAR = c_star(N_INT)
GSTAR = float(gamma(mpf(1) / 4) / gamma(mpf(3) / 4))
EPS = abs(GSTAR / CSTAR - 1)
print(f"  c*  = {CSTAR:.15f}   (root of 16 c^2 a (1-ca) = 1)")
print(f"  G*  = {GSTAR:.15f}   (Gamma(1/4)/Gamma(3/4), computed, not looked up)")
print(f"  eps* = |G*/c* - 1| = {EPS:.6e}  = {EPS*1e6:.4f} ppm   <- tolerance")

# ------------------------------------------------------------------ catalogs
rule("2. CATALOGS  (rule-generated; deduplicated by value at 1e-10)")


def fracs(B):
    out = []
    for b in range(2, B + 1):
        for aa in range(1, b):
            if math.gcd(aa, b) == 1:
                out.append(Fraction(aa, b))
    return out


GAM = {}
def G(fr: Fraction) -> float:
    if fr not in GAM:
        GAM[fr] = float(gamma(mpf(fr.numerator) / fr.denominator))
    return GAM[fr]


F8, F12 = fracs(8), fracs(12)
LO, HI = 1.0, 10.0

# dressing factors: pi^{k/2} * m/l, deduplicated by value
DRESS = {}
for k in range(-2, 3):
    for m in range(1, 5):
        for l in range(1, 5):
            v = math.pi ** (k / 2) * m / l
            key = round(math.log(v), 10)
            if key not in DRESS:
                DRESS[key] = (v, f"pi^({k}/2)*{Fraction(m, l)}")
DRESS_V = np.array([v for v, _ in DRESS.values()])
DRESS_E = [e for _, e in DRESS.values()]


def dedupe(vals, exprs):
    """keep first expression per distinct value; restrict to [LO,HI]."""
    seen, V, E = {}, [], []
    for v, e in zip(vals, exprs):
        if not (LO <= v <= HI):
            continue
        key = round(math.log(v), 10)
        if key not in seen:
            seen[key] = True
            V.append(v)
            E.append(e)
    return np.array(V), E


def single_ratio(F):
    vals, exprs = [], []
    for p in F:
        for q in F:
            if p != q:
                vals.append(G(p) / G(q))
                exprs.append(f"G({p})/G({q})")
    return dedupe(vals, exprs)


def two_term(F):
    pairs = list(itertools.combinations_with_replacement(F, 2))
    vals, exprs = [], []
    for (p, q) in pairs:
        num = G(p) * G(q)
        for (r, s) in pairs:
            if {p, q} == {r, s} and sorted((p, q)) == sorted((r, s)):
                continue
            vals.append(num / (G(r) * G(s)))
            exprs.append(f"G({p})G({q})/(G({r})G({s}))")
    return dedupe(vals, exprs)


def dressed(V, E):
    # vectorised outer product, then dedupe with expression recovery
    M = np.outer(V, DRESS_V)
    idx_i, idx_j = np.meshgrid(np.arange(len(V)), np.arange(len(DRESS_V)), indexing="ij")
    flat = M.ravel(); ii = idx_i.ravel(); jj = idx_j.ravel()
    inr = (flat >= LO) & (flat <= HI)
    flat, ii, jj = flat[inr], ii[inr], jj[inr]
    keys = np.round(np.log(flat), 10)
    _, first = np.unique(keys, return_index=True)
    first.sort()
    V2 = flat[first]
    E2 = [f"[{E[ii[t]]}]*{DRESS_E[jj[t]]}" for t in first]
    return V2, E2


C = {}
C["C1  Γ(p)/Γ(q), F(8)"] = single_ratio(F8)
C["C2  Γ(p)/Γ(q), F(12)"] = single_ratio(F12)
C["C3  C1 × π^{k/2} × m/l"] = dressed(*C["C1  Γ(p)/Γ(q), F(8)"])
C["C4  Γ(p)Γ(q)/(Γ(r)Γ(s)), F(8)"] = two_term(F8)
C["C5  C4 × π^{k/2} × m/l"] = dressed(*C["C4  Γ(p)Γ(q)/(Γ(r)Γ(s)), F(8)"])

for name, (V, E) in C.items():
    print(f"  {name:32s} N = {len(V):8d} distinct values in [{LO:.0f},{HI:.0f}]")

# G* must be present in C1 as an ordinary member
V1, E1 = C["C1  Γ(p)/Γ(q), F(8)"]
assert np.min(np.abs(V1 / GSTAR - 1)) < 1e-9, "G* not found in C1 -- generator bug"

# ------------------------------------------------------------------ statistics
rule("3. PRIMARY  (n = 16)")
rng = np.random.default_rng(20260904)
NT = 200_000
targets = np.exp(rng.uniform(math.log(LO), math.log(HI), NT))


def nearest_rel(Vs, t):
    t = np.atleast_1d(t)
    i = np.searchsorted(Vs, t)
    lo = np.clip(i - 1, 0, len(Vs) - 1); hi = np.clip(i, 0, len(Vs) - 1)
    return np.minimum(np.abs(Vs[lo] - t), np.abs(Vs[hi] - t)) / t


print(f"  {'catalog':32s} {'N':>7s} {'n(±1%)':>7s} {'E':>9s} {'hits':>5s} {'p_slide':>9s} {'rank G*':>8s}  best member")
summary = {}
for name, (V, E) in C.items():
    order = np.argsort(V); Vs = V[order]; Es = [E[i] for i in order]
    rel = np.abs(Vs / CSTAR - 1)
    hits = int(np.sum(rel <= EPS))
    n_win = int(np.sum(rel <= 1e-2))
    E_exp = n_win * (EPS / 1e-2)
    p = float(np.mean(nearest_rel(Vs, targets) <= EPS))
    gi = int(np.argmin(np.abs(Vs / GSTAR - 1)))
    rank = int(np.sum(rel < rel[gi])) + 1
    bi = int(np.argmin(rel))
    summary[name] = dict(N=len(V), n_win=n_win, E=E_exp, hits=hits, p=p, rank=rank)
    print(f"  {name:32s} {len(V):7d} {n_win:7d} {E_exp:9.4f} {hits:5d} {p:9.5f} {rank:8d}  "
          f"{Es[bi]}  ({rel[bi]*1e6:.3f} ppm)")

# ------------------------------------------------------------------ secondary
rule("4. SECONDARY  (n in [1,64] — the integer is a choice too)")
print(f"  {'catalog':32s} {'Σ hits':>7s} {'Σ E':>9s} {'n with a hit':>40s}")
for name, (V, E) in C.items():
    order = np.argsort(V); Vs = V[order]; Es = [E[i] for i in order]
    tot_hits, tot_E, which = 0, 0.0, []
    for n in range(1, 65):
        cs = c_star(n)
        if not (LO <= cs <= HI):
            continue
        rel = np.abs(Vs / cs - 1)
        h = int(np.sum(rel <= EPS))
        tot_hits += h
        tot_E += int(np.sum(rel <= 1e-2)) * (EPS / 1e-2)
        if h:
            bi = int(np.argmin(rel))
            which.append(f"n={n}:{Es[bi]}({rel[bi]*1e6:.2f}ppm)")
    print(f"  {name:32s} {tot_hits:7d} {tot_E:9.4f}   {'; '.join(which) if which else '—'}")

# ------------------------------------------------------------------ threshold
rule("5. THRESHOLD  (the deliverable)")
struct = [k for k, s in summary.items() if s["hits"] == 1 and s["rank"] == 1 and s["E"] < 0.1]
chance = [k for k, s in summary.items() if s["E"] >= 1.0]
print("  structure-at-level (hits=1, G* is the hit, E<0.1):")
for k in struct: print("    ", k)
print("  chance-at-level (E >= 1):")
for k in chance: print("    ", k)
if not chance:
    print("     (none — even the largest declared catalog does not reach E = 1)")

"""ALL non-isomorphic N=6 frameworks with self-stress AND flex, vs the FTD law.

Supersedes n6_full_sweep.py, which enumerated 27,824 LABELED edge sets and so
re-embedded the same graph hundreds of times under relabelings -- 222,592
least-squares solves, ~1h. There are only ~O(100) non-isomorphic graphs on 6
vertices with min degree >= 2. Dedup by canonical form under S_6 first
(vectorised over 15-bit edge masks), then embed each class once.

Non-degenerate embeddings only (dmin = 0.5): the first attempt at this test
returned COINCIDENT vertices, which the polarity check waved through because
same-polarity pairs carry mask = 0 at any distance, including zero.
"""
import itertools, json
import numpy as np
from scipy.optimize import least_squares
from maxwell_c3_screen import (CUT, mask, energy, grad, hessian,
                               null_beyond_trivial, relaxed_profile)

N, DMIN = 6, 0.5
RNG = np.random.default_rng(20260804)
PAIRS = [(i, j) for i in range(N) for j in range(i+1, N)]
PIDX = {p: k for k, p in enumerate(PAIRS)}

# ---- enumerate labeled masks with min degree >= 2 -------------------------
masks = []
for k in range(N, 16):
    for es in itertools.combinations(range(15), k):
        deg = np.zeros(N, int)
        for e in es:
            i, j = PAIRS[e]; deg[i] += 1; deg[j] += 1
        if deg.min() >= 2:
            masks.append(sum(1 << e for e in es))
masks = np.array(masks, dtype=np.int64)
print(f"labeled edge sets (min degree >= 2): {len(masks):,}")

# ---- canonical form under S_6, vectorised --------------------------------
canon = masks.copy()
for perm in itertools.permutations(range(N)):
    tgt = np.array([PIDX[tuple(sorted((perm[i], perm[j])))] for i, j in PAIRS])
    out = np.zeros_like(masks)
    for b in range(15):
        out |= (((masks >> b) & 1) << tgt[b])
    np.minimum(canon, out, out=canon)
uniq = np.unique(canon)
print(f"non-isomorphic classes: {len(uniq):,}  ({len(masks)/len(uniq):.0f}x reduction)")

def edges_of(m):
    return [PAIRS[b] for b in range(15) if (m >> b) & 1]

def embed(edges, tries=40):
    best = None
    for _ in range(tries):
        p0 = RNG.normal(scale=0.8, size=(N, 3))
        def f(z):
            p = z.reshape(N, 3)
            r = [np.linalg.norm(p[i]-p[j]) - 1.0 for i, j in edges]
            r += [10.0*max(0.0, DMIN-np.linalg.norm(p[i]-p[j])) for i, j in PAIRS]
            return np.array(r)
        r = least_squares(f, p0.reshape(-1), xtol=1e-15, ftol=1e-15, gtol=1e-15)
        p = r.x.reshape(N, 3)
        ee = max(abs(np.linalg.norm(p[i]-p[j])-1.0) for i, j in edges)
        sep = min(np.linalg.norm(p[i]-p[j]) for i, j in PAIRS)
        if ee < 1e-10 and sep >= DMIN-1e-9 and (best is None or ee < best[1]):
            best = (p, ee)
    return best

def realizable(p, edges):
    E = set(edges); out = []
    for s in itertools.product((-1,0,1), repeat=N):
        if s[0] < 0: continue
        A = mask(np.array(s)); ok = True
        for i, j in PAIRS:
            q = float(np.sum((p[i]-p[j])**2)); b = (i,j) in E
            if b and (A[i,j] == 0.0 or q >= CUT): ok = False; break
            if (not b) and A[i,j] != 0.0 and q < CUT: ok = False; break
        if ok: out.append(s)
    return out

emb = both = real = probed = 0
hits = []
for m in uniq:
    edges = edges_of(int(m))
    got = embed(edges)
    if got is None: continue
    emb += 1
    p, _ = got
    R = np.zeros((len(edges), 3*N))
    for e, (i, j) in enumerate(edges):
        d = p[i]-p[j]; R[e,3*i:3*i+3] = d; R[e,3*j:3*j+3] = -d
    rk = np.linalg.matrix_rank(R, tol=1e-8)
    st, fl = len(edges)-rk, 3*N-6-rk
    if st <= 0 or fl <= 0: continue
    both += 1
    sols = realizable(p, edges)
    if not sols: continue
    real += 1
    s = sols[0]; A = mask(np.array(s)); x0 = p.reshape(-1)
    if np.linalg.norm(grad(x0, A)) > 1e-7: continue
    H = hessian(x0, A); N0, _ = null_beyond_trivial(H, x0, 1e-7)
    if N0.shape[1] == 0: continue
    probed += 1
    flat, exps = 0, []
    for k in range(N0.shape[1]):
        pr = relaxed_profile(x0, A, N0[:, k]); keep = [q for q in pr if q["valid"]]
        t = np.array([q["t"] for q in keep]); dE = np.array([q["dE"] for q in keep])
        if len(keep) < 2 or np.max(np.abs(dE)) < 1e-12: flat += 1; continue
        mm = np.abs(dE) > 1e-14
        if mm.sum() >= 2:
            exps.append(float(np.polyfit(np.log(t[mm]), np.log(np.abs(dE[mm])), 1)[0]))
    tag = ("n=4" if (flat == 0 and exps and all(3.5 < e < 4.5 for e in exps))
           else ("n=inf" if flat == N0.shape[1] else "semidef"))
    print(f"  B={len(edges):>2} rank={rk:>2} stress={st} flex={fl} s={s} "
          f"dimN0={N0.shape[1]} flat={flat} exps={np.round(exps,2).tolist()} -> {tag}")
    if tag == "n=4":
        hits.append(dict(edges=edges, s=list(s), stress=int(st), flex=int(fl), exps=exps))

print(f"\nclasses={len(uniq)}  embeddable={emb}  stress&flex={both}  "
      f"FTD-realizable={real}  probed={probed}  n=4 HITS={len(hits)}")
json.dump(hits, open("n6_iso_sweep_hits.json","w"), indent=1)

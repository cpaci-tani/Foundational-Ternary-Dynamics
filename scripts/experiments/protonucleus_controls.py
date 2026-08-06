"""Control battery for PREREG_PROTONUCLEUS_GROWTH_v1.
T1 neutral seed · T2 lattice-size · T3 held-out RNG · T4 collision order
T5 long run · T6 quench-mechanism diagnostic.
"""
import numpy as np, sys, time, json

C_WAVE2 = np.float32(1.0/3.0)
G_C     = np.float32(0.0854245431028543695)
K_GEN   = np.float32(1.5163860591519780)
K_MAN   = np.float32(0.5054620197173260)

def lap18(F, out):
    np.multiply(F, np.float32(-4.0), out=out)
    acc = np.zeros_like(F)
    for ax in range(3):
        acc += np.roll(F, 1, ax); acc += np.roll(F, -1, ax)
    out += acc*np.float32(1.0/3.0)
    acc[:] = 0
    for i in range(3):
        for j in range(i+1, 3):
            for si in (1, -1):
                for sj in (1, -1):
                    acc += np.roll(np.roll(F, si, i), sj, j)
    out += acc*np.float32(1.0/6.0)
    return out

def make_seed(R0, L, mode):
    c = L//2
    ax = np.arange(L, dtype=np.int32) - c
    X, Y, Z = np.meshgrid(ax, ax, ax, indexing="ij")
    ball = (X*X + Y*Y + Z*Z) <= R0*R0
    if mode == "uniform":
        return np.where(ball, np.int8(1), np.int8(0))
    if mode == "neutral":                      # alternating +-1, charge balanced
        par = ((X + Y + Z) % 2 == 0)
        return np.where(ball, np.where(par, np.int8(1), np.int8(-1)), np.int8(0))
    raise ValueError(mode)

def run(R0, L, ticks=600, seed=20260803, shuffle=False, mode="uniform",
        label="", diag=False):
    rng = np.random.default_rng(seed)
    s = make_seed(R0, L, mode)
    J = [np.zeros((L,L,L), np.float32) for _ in range(3)]
    W = [np.zeros((L,L,L), np.float32) for _ in range(3)]
    tmp = np.empty((L,L,L), np.float32)
    N0 = int((s != 0).sum()); q0 = int(s.sum()); vol = L**3
    tr = []
    for t in range(ticks):
        g = [((np.roll(s, -1, a).astype(np.float32) - np.roll(s, 1, a))*np.float32(0.5))
             for a in range(3)]
        for k in range(3):
            lap18(J[k], tmp); W[k] += C_WAVE2*tmp - G_C*g[k]; J[k] += W[k]
        jmag = np.sqrt(J[0]**2 + J[1]**2 + J[2]**2)
        cand = (s == 0) & (jmag > K_GEN)
        nf = 0
        if cand.any():
            p = 1.0 - np.exp(-(jmag - K_GEN)/K_MAN)
            idx = np.flatnonzero((cand & (rng.random(cand.shape) < p)).ravel())
            if idx.size:
                if shuffle: rng.shuffle(idx)
                ijk = np.array(np.unravel_index(idx, (L,L,L))).T
                comp = np.stack([J[k].ravel()[idx] for k in range(3)], 1)
                am = np.argmax(np.abs(comp), 1)
                st = np.sign(comp[np.arange(len(idx)), am]).astype(np.int64)
                pj = ijk.copy(); r = np.arange(len(idx))
                pj[r, am] = (pj[r, am] + st) % L
                pidx = np.ravel_multi_index(pj.T, (L,L,L))
                ok = (s.ravel()[pidx] == 0); idx, pidx = idx[ok], pidx[ok]
                _, first = np.unique(pidx, return_index=True); o = np.sort(first)
                idx, pidx = idx[o], pidx[o]
                keep = ~np.isin(idx, pidx); idx, pidx = idx[keep], pidx[keep]
                if idx.size:
                    sc = np.maximum(0.0, 1.0 - K_GEN/jmag.ravel()[idx]).astype(np.float32)
                    for k in range(3):
                        J[k].ravel()[idx] *= sc
                        W[k].ravel()[idx] *= np.float32(0.5)
                        W[k].ravel()[pidx] *= np.float32(0.5)
                    sf = s.ravel(); sf[idx] = -1; sf[pidx] = 1; nf = idx.size
        if t % 100 == 0 or t == ticks-1:
            N = int((s != 0).sum())
            tr.append((t, N, float(jmag.max()), nf))
        if int((s != 0).sum()) > 0.40*vol: break
    N = int((s != 0).sum())
    out = dict(label=label, R0=R0, L=L, mode=mode, seed=seed, shuffle=shuffle,
               ticks=ticks, N0=N0, N=N, growth=(N-N0)/max(N0,1),
               R0_eff=(3*N0/(4*np.pi))**(1/3), R_eff=(3*N/(4*np.pi))**(1/3),
               q0=q0, q=int(s.sum()), maxJ=float(jmag.max()), trace=tr)
    if diag:
        hot = jmag > K_GEN
        occupied = int((hot & (s != 0)).sum()); void = int((hot & (s == 0)).sum())
        out["diag"] = dict(hot_total=int(hot.sum()), hot_occupied=occupied,
                           hot_void=void)
    print(f"  {label:<28} N {N0:>8,} -> {N:>8,}  ({out['growth']*100:+7.1f}%)  "
          f"R {out['R0_eff']:.2f}->{out['R_eff']:.2f}  max|J|={out['maxJ']:.4f}"
          + (f"  hot: {out['diag']['hot_void']} void / {out['diag']['hot_occupied']} occ"
             if diag else ""))
    return out

if __name__ == "__main__":
    which = sys.argv[1] if len(sys.argv) > 1 else "all"
    res = []
    if which in ("all", "t1"):
        print("=== T1 NEUTRAL (charge-balanced) SEED vs UNIFORM ===")
        for R0 in (4, 6, 8, 13):
            res.append(run(R0, 97, mode="neutral", label=f"neutral R0={R0}"))
    if which in ("all", "t2"):
        print("=== T2/T3/T4 CONTROLS on R0=16 ===")
        res.append(run(16, 161, label="R0=16 L=161 (lattice ctrl)"))
        res.append(run(16, 129, seed=7, label="R0=16 held-out seed 7"))
        res.append(run(16, 129, shuffle=True, label="R0=16 shuffled order"))
    if which in ("all", "t5"):
        print("=== T5/T6 LONG RUN + QUENCH DIAGNOSTIC ===")
        res.append(run(20, 161, ticks=1200, label="R0=20 long (1200t)", diag=True))
        res.append(run(16, 129, ticks=1200, label="R0=16 long (1200t)", diag=True))
    json.dump(res, open(f"protonucleus_controls_{which}.json","w"), indent=1, default=str)

"""Protonucleus continued-growth experiment. Protocol: PREREG_PROTONUCLEUS_GROWTH_v1.
Model taken verbatim from dag_engine.cpp (field) and transmutation_phases.cpp (genesis).
"""
import numpy as np, sys, json, time

C_WAVE2 = np.float32(1.0/3.0)
G_C     = np.float32(0.0854245431028543695)
K_GEN   = np.float32(1.5163860591519780)
K_MAN   = np.float32(0.5054620197173260)

def lap18(F, out):
    """18-point SC+FCC Moore stencil, exactly dag_engine.cpp:145-171."""
    np.multiply(F, np.float32(-4.0), out=out)
    for ax in range(3):
        out += np.roll(F, 1, ax); out += np.roll(F, -1, ax)
    out *= np.float32(1.0)          # faces already weighted below
    # redo with correct weights (faces 1/3, edges 1/6)
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

def grad_s(s):
    return [((np.roll(s, -1, a).astype(np.float32) - np.roll(s, 1, a)) * np.float32(0.5))
            for a in range(3)]

def run(R0, L, ticks=600, seed=20260803, shuffle=False, log_every=25):
    rng = np.random.default_rng(seed)
    c = L//2
    ax = np.arange(L, dtype=np.int32) - c
    X, Y, Z = np.meshgrid(ax, ax, ax, indexing="ij")
    s = np.where(X*X + Y*Y + Z*Z <= R0*R0, np.int8(1), np.int8(0))
    J  = [np.zeros((L,L,L), np.float32) for _ in range(3)]
    W  = [np.zeros((L,L,L), np.float32) for _ in range(3)]
    tmp = np.empty((L,L,L), np.float32)
    hist, N0 = [], int((s != 0).sum())
    vol = L**3
    for t in range(ticks):
        g = grad_s(s)
        for k in range(3):
            lap18(J[k], tmp)
            W[k] += C_WAVE2*tmp - G_C*g[k]
            J[k] += W[k]
        jmag = np.sqrt(J[0]**2 + J[1]**2 + J[2]**2)
        cand = (s == 0) & (jmag > K_GEN)
        nfire = 0
        if cand.any():
            p = 1.0 - np.exp(-(jmag - K_GEN)/K_MAN)
            fire = cand & (rng.random(cand.shape) < p)
            idx = np.flatnonzero(fire.ravel())
            if idx.size:
                if shuffle: rng.shuffle(idx)
                ijk = np.array(np.unravel_index(idx, (L,L,L))).T
                comp = np.stack([J[k].ravel()[idx] for k in range(3)], 1)
                axm = np.argmax(np.abs(comp), 1)
                step = np.sign(comp[np.arange(len(idx)), axm]).astype(np.int64)
                pj = ijk.copy()
                pj[np.arange(len(idx)), axm] = (pj[np.arange(len(idx)), axm] + step) % L
                pidx = np.ravel_multi_index(pj.T, (L,L,L))
                ok = (s.ravel()[pidx] == 0)
                idx, pidx = idx[ok], pidx[ok]
                # collision resolution: first candidate in order wins each partner
                _, first = np.unique(pidx, return_index=True)
                idx, pidx = idx[np.sort(first)], pidx[np.sort(first)]
                keep = ~np.isin(idx, pidx)          # a site cannot also be a partner
                idx, pidx = idx[keep], pidx[keep]
                if idx.size:
                    jm = jmag.ravel()[idx]
                    scale = np.maximum(0.0, 1.0 - K_GEN/jm).astype(np.float32)
                    for k in range(3):
                        Jf = J[k].ravel(); Wf = W[k].ravel()
                        Jf[idx] *= scale
                        Wf[idx] *= np.float32(0.5); Wf[pidx] *= np.float32(0.5)
                    sf = s.ravel(); sf[idx] = -1; sf[pidx] = 1
                    nfire = idx.size
        N = int((s != 0).sum())
        if t % log_every == 0 or t == ticks-1:
            hist.append(dict(t=t, N=N, R_eff=float((3*N/(4*np.pi))**(1/3)),
                             maxJ=float(jmag.max()), fires=int(nfire),
                             charge=int(s.sum()), E=float((jmag**2).sum())))
            print(f"  t={t:>4} N={N:>9,} R_eff={hist[-1]['R_eff']:>7.3f} "
                  f"max|J|={hist[-1]['maxJ']:>8.4f} fires={nfire:>7,} q={s.sum():>4}")
        if N > 0.40*vol:
            print(f"  STOP: N exceeded 40% of lattice at t={t}"); break
    return dict(R0=R0, L=L, N0=N0, hist=hist, final_N=int((s != 0).sum()))

if __name__ == "__main__":
    arms = [(8,129),(12,129),(13,129),(16,161),(20,201)]
    if len(sys.argv) > 1:
        arms = [tuple(int(x) for x in a.split(",")) for a in sys.argv[1:]]
    out = []
    for R0, L in arms:
        print(f"\n=== ARM R0={R0} L={L} ({L**3:,} sites) ===")
        t0 = time.time()
        out.append(run(R0, L))
        print(f"  [{time.time()-t0:.1f}s]  N: {out[-1]['N0']:,} -> {out[-1]['final_N']:,}")
    json.dump(out, open("protonucleus_growth_results.json","w"), indent=1)

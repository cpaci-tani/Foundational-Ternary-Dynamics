"""Re-screen C3 with the native Hodge channel added.
Protocol: PREREG_HODGE_CHANNEL_C3_RESCREEN_v1.md, locked at 20658ca7.
"""
import itertools, json
import numpy as np
from scipy.optimize import least_squares, minimize

EPS, CUT, ALPHA = 0.01, 1.5, 0.0072973525693
N, DMIN = 6, 0.5
RNG = np.random.default_rng(20260804)
PAIRS = [(i, j) for i in range(N) for j in range(i+1, N)]

# ---------- continuous Hodge kernel K(r) ----------------------------------
def _bz(nk):
    g = np.arange(nk)*2*np.pi/nk
    K1, K2, K3 = np.meshgrid(g, g, g, indexing='ij')
    M = 4 - (2/3)*(np.cos(K1)+np.cos(K2)+np.cos(K3)) \
          - (2/3)*(np.cos(K1)*np.cos(K2)+np.cos(K2)*np.cos(K3)+np.cos(K3)*np.cos(K1))
    S2 = np.sin(K1)**2+np.sin(K2)**2+np.sin(K3)**2
    R = np.zeros_like(M); nz = M > 1e-12; R[nz] = 3*S2[nz]/M[nz]
    kv = np.stack([K1.ravel(), K2.ravel(), K3.ravel()], 1)
    return kv, R.ravel()/ (nk**3)

KV, RW = _bz(16)

def Kfun(d):                      # d : (m,3) separations -> (m,) kernel
    ph = d @ KV.T                 # (m, nk^3)
    return np.cos(ph) @ RW

def dKfun(d):                     # gradient wrt d : (m,3)
    ph = d @ KV.T
    return -(np.sin(ph)*RW) @ KV

# ---------- kill condition 2: reproduce the converged lattice values ------
lat = {(0,0,0): 1.2476, (1,0,0): 0.15529, (1,1,0): 0.089847,
       (1,1,1): 0.045981, (2,0,0): -0.15874, (3,0,0): -0.015330}
print("KILL CONDITION 2 - continuous evaluator vs converged FFT values:")
bad = 0
for r, want in lat.items():
    got = float(Kfun(np.array([r], float))[0])
    flag = "ok" if abs(got-want) < 2e-3 else "MISMATCH"
    if flag != "ok": bad += 1
    print(f"   r={r}  K_cont={got:+.6f}  K_fft={want:+.6f}   {flag}")
if bad:
    print("\nSCREEN_INVALID: continuous kernel disagrees with the FFT values.")
    raise SystemExit(1)
print("   -> evaluator validated\n")

# ---------- combined energy ----------------------------------------------
def V(q):
    q = np.asarray(q, float); out = np.zeros_like(q); m = q < CUT
    out[m] = -16*EPS*(q[m]-1.5)**2*(q[m]-0.75); return out

def dV(q):
    q = np.asarray(q, float); out = np.zeros_like(q); m = q < CUT
    out[m] = -48*EPS*(q[m]-1.5)*(q[m]-1.0); return out

def energy(x, s, hodge=True):
    p = x.reshape(N,3); e = 0.0
    ii = np.array([a for a,_ in PAIRS]); jj = np.array([b for _,b in PAIRS])
    d = p[ii]-p[jj]; q = (d*d).sum(1)
    A = (1 - s[ii]*s[jj])/2.0
    e += float((A*V(q)).sum())
    if hodge:
        e += float((-ALPHA*s[ii]*s[jj]*Kfun(d)).sum())
    return e

def grad(x, s, hodge=True):
    p = x.reshape(N,3); g = np.zeros_like(p)
    ii = np.array([a for a,_ in PAIRS]); jj = np.array([b for _,b in PAIRS])
    d = p[ii]-p[jj]; q = (d*d).sum(1)
    A = (1 - s[ii]*s[jj])/2.0
    w = (A*dV(q)*2.0)[:,None]*d
    if hodge:
        w = w + (-ALPHA*s[ii]*s[jj])[:,None]*dKfun(d)
    for n,(a,b) in enumerate(PAIRS):
        g[a] += w[n]; g[b] -= w[n]
    return g.ravel()

def hess(x, s, hodge=True, h=1e-5):
    n = 3*N; H = np.zeros((n,n))
    for i in range(n):
        xp = x.copy(); xp[i] += h; xm = x.copy(); xm[i] -= h
        H[:,i] = (grad(xp,s,hodge)-grad(xm,s,hodge))/(2*h)
    return (H+H.T)/2

def interacting(x, s, hodge=True):
    p = x.reshape(N,3); out=[]
    for (a,b) in PAIRS:
        d = p[a]-p[b]; q = float(d@d)
        A = (1-s[a]*s[b])/2.0
        compact = (A != 0.0 and q < CUT)
        hodgep  = hodge and (s[a]*s[b] != 0)
        if compact or hodgep: out.append((a,b))
    return out

# ---------- enumerate the 62 isomorphism classes -------------------------
PIDX = {p:k for k,p in enumerate(PAIRS)}
masks=[]
for k in range(N,16):
    for es in itertools.combinations(range(15),k):
        deg=np.zeros(N,int)
        for e in es: i,j=PAIRS[e]; deg[i]+=1; deg[j]+=1
        if deg.min()>=2: masks.append(sum(1<<e for e in es))
masks=np.array(masks,dtype=np.int64); canon=masks.copy()
for perm in itertools.permutations(range(N)):
    tgt=np.array([PIDX[tuple(sorted((perm[i],perm[j])))] for i,j in PAIRS])
    out=np.zeros_like(masks)
    for b in range(15): out |= (((masks>>b)&1)<<tgt[b])
    np.minimum(canon,out,out=canon)
uniq=np.unique(canon)
print(f"non-isomorphic classes: {len(uniq)}")

def edges_of(m): return [PAIRS[b] for b in range(15) if (m>>b)&1]

def embed(edges, tries=30):
    best=None
    for _ in range(tries):
        p0=RNG.normal(scale=0.8,size=(N,3))
        def f(z):
            p=z.reshape(N,3)
            r=[np.linalg.norm(p[i]-p[j])-1.0 for i,j in edges]
            r+=[10.0*max(0.0,DMIN-np.linalg.norm(p[i]-p[j])) for i,j in PAIRS]
            return np.array(r)
        rr=least_squares(f,p0.reshape(-1),xtol=1e-15,ftol=1e-15,gtol=1e-15)
        p=rr.x.reshape(N,3)
        ee=max(abs(np.linalg.norm(p[i]-p[j])-1.0) for i,j in edges)
        sep=min(np.linalg.norm(p[i]-p[j]) for i,j in PAIRS)
        if ee<1e-10 and sep>=DMIN-1e-9 and (best is None or ee<best[1]): best=(p,ee)
    return best

def polarities(p, edges):
    E=set(edges); out=[]
    for s in itertools.product((-1,0,1),repeat=N):
        if s[0]<0: continue
        ok=True
        for i,j in PAIRS:
            q=float(np.sum((p[i]-p[j])**2)); b=(i,j) in E
            A=(1-s[i]*s[j])/2.0
            if b and (A==0.0 or q>=CUT): ok=False;break
            if (not b) and A!=0.0 and q<CUT: ok=False;break
        if ok: out.append(np.array(s))
    return out

def prestressed(x, s):
    """Is any interacting pair off its own minimum? -> pre-stressed (P-B)."""
    p=x.reshape(N,3); worst=0.0
    for (a,b) in interacting(x,s):
        d=p[a]-p[b]; q=float(d@d); r=np.sqrt(q)
        A=(1-s[a]*s[b])/2.0
        fr = A*float(dV(np.array([q]))[0])*2*r
        fr += float(-ALPHA*s[a]*s[b]*(dKfun(d[None,:])[0] @ (d/r)))
        worst=max(worst,abs(fr))
    return worst

res=[]; n_eq=0; n_ps=0; hits=[]
for m in uniq:
    edges=edges_of(int(m)); got=embed(edges)
    if got is None: continue
    p0,_=got
    for s in polarities(p0,edges)[:1]:
        r=minimize(lambda z: energy(z,s), p0.reshape(-1), jac=lambda z: grad(z,s),
                   method="L-BFGS-B", options=dict(maxiter=40000,ftol=1e-18,gtol=1e-16))
        x=r.x
        if np.linalg.norm(grad(x,s))>1e-7: continue
        pp=x.reshape(N,3)
        if min(np.linalg.norm(pp[i]-pp[j]) for i,j in PAIRS) < DMIN: continue
        n_eq+=1
        ed=interacting(x,s)
        R=np.zeros((len(ed),3*N))
        for e,(a,b) in enumerate(ed):
            d=pp[a]-pp[b]; R[e,3*a:3*a+3]=d; R[e,3*b:3*b+3]=-d
        rk=int(np.linalg.matrix_rank(R,tol=1e-8))
        st,fl=len(ed)-rk, 3*N-6-rk
        ps=prestressed(x,s)
        if ps>1e-9: n_ps+=1
        res.append(dict(B=len(ed),rank=rk,stress=st,flex=fl,prestress=float(ps)))

print(f"\nvalid combined-channel equilibria: {n_eq}")
print(f"  PRE-STRESSED (some pair off its own minimum): {n_ps} / {n_eq}")
if res:
    import collections
    sig=collections.Counter((d['B'],d['rank'],d['stress'],d['flex']) for d in res)
    print(f"  (B, rank, stress, flex) signatures:")
    for k,v in sorted(sig.items()): print(f"     {k}  x{v}")
    both=[d for d in res if d['stress']>0 and d['flex']>0]
    print(f"  with stress>0 AND flex>0: {len(both)}")
    print(f"  max prestress magnitude: {max(d['prestress'] for d in res):.3e}")
json.dump(res, open("hodge_rescreen.json","w"), indent=1)

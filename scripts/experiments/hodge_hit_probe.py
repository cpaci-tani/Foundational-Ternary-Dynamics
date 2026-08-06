"""Isolate the stress>0 & flex>0 candidate from the Hodge re-screen and decide it.
P-B first: if it is PRE-STRESSED, FTD-0789's trichotomy does not apply and no
n=2/4/inf classification is licensed -- that is the honest verdict, not a count.
"""
exec(open("hodge_channel_rescreen.py").read().split("# ---------- enumerate")[0])
import itertools, numpy as np
from scipy.optimize import least_squares, minimize
from scipy.linalg import null_space, orth

PIDX={p:k for k,p in enumerate(PAIRS)}
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
def edges_of(m): return [PAIRS[b] for b in range(15) if (m>>b)&1]
def embed(edges,tries=30):
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
def polarities(p,edges):
    E=set(edges); out=[]
    for s in itertools.product((-1,0,1),repeat=N):
        if s[0]<0: continue
        ok=True
        for i,j in PAIRS:
            q=float(np.sum((p[i]-p[j])**2)); b=(i,j) in E; A=(1-s[i]*s[j])/2.0
            if b and (A==0.0 or q>=CUT): ok=False;break
            if (not b) and A!=0.0 and q<CUT: ok=False;break
        if ok: out.append(np.array(s))
    return out
def prestressed(x,s):
    p=x.reshape(N,3); worst=0.0; detail=[]
    for (a,b) in interacting(x,s):
        d=p[a]-p[b]; q=float(d@d); r=np.sqrt(q); A=(1-s[a]*s[b])/2.0
        fr=A*float(dV(np.array([q]))[0])*2*r
        fr+=float(-ALPHA*s[a]*s[b]*(dKfun(d[None,:])[0] @ (d/r)))
        detail.append(((a,b),r,fr)); worst=max(worst,abs(fr))
    return worst, detail

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
        if min(np.linalg.norm(pp[i]-pp[j]) for i,j in PAIRS)<DMIN: continue
        ed=interacting(x,s)
        R=np.zeros((len(ed),3*N))
        for e,(a,b) in enumerate(ed):
            d=pp[a]-pp[b]; R[e,3*a:3*a+3]=d; R[e,3*b:3*b+3]=-d
        rk=int(np.linalg.matrix_rank(R,tol=1e-8)); st,fl=len(ed)-rk,3*N-6-rk
        if not (st>0 and fl>0): continue
        ps,detail=prestressed(x,s)
        print("="*70)
        print(f"CANDIDATE  B={len(ed)} rank={rk} stress={st} flex={fl}")
        print(f"  polarity s = {list(s)}")
        print(f"  |grad| = {np.linalg.norm(grad(x,s)):.3e}   E = {energy(x,s):.8f}")
        print(f"  MAX PAIR-FORCE (pre-stress) = {ps:.6e}")
        print(f"  per-pair radial force at equilibrium:")
        for (ab,rr_,fr) in detail:
            tag = "" if abs(fr)<1e-9 else "  <-- OFF ITS MINIMUM"
            print(f"     {ab}  r={rr_:.5f}  f_r={fr:+.4e}{tag}")
        if ps>1e-9:
            print("\n  VERDICT: PRE-STRESSED. FTD-0789's trichotomy assumes an")
            print("  UNSTRESSED equilibrium (every bond at its own minimum), so")
            print("  n = 2/4/infinity is NOT LICENSED here. P-B confirmed.")
        else:
            print("\n  unstressed -> trichotomy applies; running the relaxed probe")
        raise SystemExit(0)
print("no stress>0 & flex>0 candidate re-found")

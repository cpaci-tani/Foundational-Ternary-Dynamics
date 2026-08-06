"""Full sweep: ALL N=6 unit-distance frameworks with stress>0 AND flex>0.

n6_candidates_c3.py tested 5 REPRESENTATIVES (deduplicated by
(B,rank,flex,stress)); the enumeration found 701. Signature is not
isomorphism class, so 696 were untested. This sweeps every one.
Non-degenerate embeddings only (dmin=0.5) -- the first attempt returned
coincident vertices.
"""
import itertools, json
import numpy as np
from scipy.optimize import least_squares
from maxwell_c3_screen import (CUT, mask, energy, grad, hessian,
                               trivial_modes, null_beyond_trivial, relaxed_profile)
RNG = np.random.default_rng(20260804); N = 6; DMIN = 0.5
PAIRS = [(i,j) for i in range(N) for j in range(i+1,N)]

def embed(edges, tries=60):
    best=None
    for _ in range(tries):
        p0 = RNG.normal(scale=0.8, size=(N,3))
        def f(z):
            p=z.reshape(N,3)
            r=[np.linalg.norm(p[i]-p[j])-1.0 for i,j in edges]
            r+=[10.0*max(0.0,DMIN-np.linalg.norm(p[i]-p[j])) for i,j in PAIRS]
            return np.array(r)
        r=least_squares(f,p0.reshape(-1),xtol=1e-15,ftol=1e-15,gtol=1e-15)
        p=r.x.reshape(N,3)
        ee=max(abs(np.linalg.norm(p[i]-p[j])-1.0) for i,j in edges)
        sep=min(np.linalg.norm(p[i]-p[j]) for i,j in PAIRS)
        if ee<1e-10 and sep>=DMIN-1e-9 and (best is None or ee<best[1]): best=(p,ee)
    return best

def realizable(p, edges):
    E={tuple(sorted(e)) for e in edges}; out=[]
    for s in itertools.product((-1,0,1),repeat=N):
        if s[0]<0: continue
        A=mask(np.array(s)); ok=True
        for i,j in PAIRS:
            q=float(np.sum((p[i]-p[j])**2)); b=(i,j) in E
            if b and (A[i,j]==0.0 or q>=CUT): ok=False;break
            if (not b) and A[i,j]!=0.0 and q<CUT: ok=False;break
        if ok: out.append(s)
    return out

# regenerate the 701
def rigid_rank(p,edges):
    R=np.zeros((len(edges),3*N))
    for e,(i,j) in enumerate(edges):
        d=p[i]-p[j]; R[e,3*i:3*i+3]=d; R[e,3*j:3*j+3]=-d
    return np.linalg.matrix_rank(R,tol=1e-8)

cands=[]
for k in range(N, len(PAIRS)+1):
    for edges in itertools.combinations(PAIRS,k):
        deg=np.zeros(N,int)
        for i,j in edges: deg[i]+=1; deg[j]+=1
        if deg.min()<2: continue
        got=embed(list(edges),tries=8)
        if got is None: continue
        p,_=got; rk=rigid_rank(p,list(edges))
        st,fl=len(edges)-rk, 3*N-6-rk
        if st>0 and fl>0: cands.append((list(edges),p,st,fl))
print(f"non-degenerately embeddable with stress>0 and flex>0: {len(cands)}")

hits=[]; realizable_n=0; tested=0
for edges,p,st,fl in cands:
    sols=realizable(p,edges)
    if not sols: continue
    realizable_n+=1
    for s in sols[:1]:
        A=mask(np.array(s)); x0=p.reshape(-1)
        if np.linalg.norm(grad(x0,A))>1e-7: continue
        H=hessian(x0,A); N0,_=null_beyond_trivial(H,x0,1e-7)
        if N0.shape[1]==0: continue
        tested+=1; flat=0; exps=[]
        for kk in range(N0.shape[1]):
            pr=relaxed_profile(x0,A,N0[:,kk]); keep=[q for q in pr if q["valid"]]
            t=np.array([q["t"] for q in keep]); dE=np.array([q["dE"] for q in keep])
            if len(keep)<2 or np.max(np.abs(dE))<1e-12: flat+=1; continue
            m=np.abs(dE)>1e-14
            if m.sum()>=2: exps.append(float(np.polyfit(np.log(t[m]),np.log(np.abs(dE[m])),1)[0]))
        if flat==0 and exps and all(3.5<e<4.5 for e in exps):
            hits.append(dict(edges=edges,s=s,stress=st,flex=fl,exps=exps))
            print(f"  *** n=4 HIT: B={len(edges)} flex={fl} stress={st} s={s} exps={np.round(exps,3)}")
print(f"\nFTD-realizable: {realizable_n}   probed: {tested}   n=4 hits: {len(hits)}")
json.dump(hits, open("n6_full_sweep_hits.json","w"), indent=1, default=str)

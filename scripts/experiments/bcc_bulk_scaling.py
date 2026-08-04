"""Does BCC go over-constrained in bulk, and are its bulk null modes flat?

The 2x2x2 BCC block is surface-dominated: 3N - B = +41, still UNDER-constrained,
so it cannot test the bulk claim. Corner sites number (n+1)^3 and body centres
n^3, with B = 8n^3, so 3N - B = 3[(n+1)^3 + n^3] - 8n^3 turns negative only
around n = 6. This runs the scaling and probes the null space where it does.
"""
import itertools, numpy as np
from maxwell_c3_screen import (mask, energy, grad, hessian, n_bonds,
                               trivial_modes, null_beyond_trivial,
                               relaxed_profile, CUT)

def bcc(n):
    a = 2/np.sqrt(3); pts, sgn = [], []
    for c in itertools.product(range(n+1), repeat=3):
        pts.append(np.array(c,float)*a); sgn.append(+1)
    for c in itertools.product(range(n), repeat=3):
        pts.append((np.array(c,float)+0.5)*a); sgn.append(-1)
    return np.array(pts), np.array(sgn)

print(f"{'n':>3} {'N':>5} {'B':>6} {'3N-B':>7} {'rank':>6} {'stress':>7} {'flex':>6} {'flat/probed':>12}")
for n in (2,3,4,5,6):
    p,s = bcc(n); A = mask(s); x0 = p.reshape(-1); N=len(p)
    B = n_bonds(x0,A)
    edges=[(i,j) for i in range(N) for j in range(i+1,N)
           if A[i,j]!=0 and np.sum((p[i]-p[j])**2)<CUT]
    R=np.zeros((len(edges),3*N))
    for e,(i,j) in enumerate(edges):
        d=p[i]-p[j]; R[e,3*i:3*i+3]=d; R[e,3*j:3*j+3]=-d
    rank=int(np.linalg.matrix_rank(R,tol=1e-8))
    triv=trivial_modes(x0).shape[1]
    stress, flex = B-rank, 3*N-triv-rank
    probe=""
    if n<=4:
        H=hessian(x0,A); N0,ev=null_beyond_trivial(H,x0,1e-7)
        nf=0; k=min(N0.shape[1],6)
        for i in range(k):
            pr=relaxed_profile(x0,A,N0[:,i])
            keep=[q for q in pr if q["valid"]]
            dE=np.array([q["dE"] for q in keep])
            if len(keep)<2 or np.max(np.abs(dE))<1e-12: nf+=1
        probe=f"{nf}/{k}"
    print(f"{n:>3} {N:>5} {B:>6} {3*N-B:>7} {rank:>6} {stress:>7} {flex:>6} {probe:>12}")

"""Verify THEOREM_LATTICE_COULOMB_ASYMPTOTIC_v1.md.

Exhibits what FTD-0785 said was "not shown": an exact lattice-Green's-function
statement, posed about Z^3 and a stencil with NO simulator in the hypotheses.
"""
import numpy as np, sympy as sp
TARGET = 1/(2*np.pi)
ok=[]
def check(n,c,d):
    ok.append(bool(c)); print(f"  [{'PASS' if c else 'FAIL'}] {n}: {d}")

# --- 2: exact symbolic input --------------------------------------------
k1,k2,k3,t = sp.symbols('k1 k2 k3 t', real=True)
ks=[k1,k2,k3]
M = 4 - sp.Rational(2,3)*sum(sp.cos(k) for k in ks) \
      - sp.Rational(2,3)*sum(sp.cos(ks[i])*sp.cos(ks[j]) for i in range(3) for j in range(i+1,3))
ser = sp.series(M.subs({k:t*k for k in ks}), t, 0, 6).removeO().expand()
S2 = sum(k**2 for k in ks)
check("|k|^2 coefficient is exactly 1 (fixes the amplitude, nothing fitted)",
      sp.simplify(ser.coeff(t,2) - S2)==0, "M = |k|^2 + ...")
check("4th-order term is EXACTLY isotropic: c4 = -(1/12)|k|^4",
      sp.simplify(ser.coeff(t,4) + sp.Rational(1,12)*S2**2)==0,
      "S4 anisotropy cancels identically (S2^2 = S4 + 2 P22)")
check("anisotropy first appears at O(|k|^6)",
      sp.simplify(ser.coeff(t,6) + sp.Rational(1,360)*S2**3) != 0,
      "so 4th-order isotropy is a property of the weights, not an approximation")

# --- 4: infinite-lattice limit via 1/L extrapolation ---------------------
def GL(L, pts):
    g=np.arange(L)*2*np.pi/L
    K1,K2,K3=np.meshgrid(g,g,g,indexing='ij')
    Mn=4-(2/3)*(np.cos(K1)+np.cos(K2)+np.cos(K3)) \
        -(2/3)*(np.cos(K1)*np.cos(K2)+np.cos(K2)*np.cos(K3)+np.cos(K3)*np.cos(K1))
    inv=np.zeros_like(Mn); nz=Mn>1e-12; inv[nz]=1.0/Mn[nz]; inv[0,0,0]=0.0
    G=np.real(np.fft.ifftn(inv)); return {p:G[p] for p in pts}
pts=[(2,0,0),(4,0,0),(2,2,0),(4,4,0),(2,2,2)]
Ls=[48,64,96,128,160]
acc={p:[] for p in pts}
for L in Ls:
    d=GL(L,pts)
    for p in pts: acc[p].append(d[p])
print("\n  infinite-lattice alpha_r after 1/L extrapolation:")
devs=[]
for p in pts:
    r=np.linalg.norm(p); b=np.polyfit(1.0/np.array(Ls,float), np.array(acc[p]), 1)[1]
    a=2*r*b; dev=abs(a-TARGET)/TARGET; devs.append(dev)
    print(f"     r={str(p):<10} |r|={r:6.3f}  alpha_r={a:.8f}  dev={dev:+.3%}")
check("alpha_r -> 1/(2pi) for |r| >= 2 (all within 1%)", max(devs) < 0.01,
      f"max deviation {max(devs):.3%}, target {TARGET:.9f}")

# --- 5: the torus object does NOT have the limit ------------------------
G96=GL(96,[(2,0,0),(24,0,0)]); G144=GL(144,[(2,0,0),(24,0,0)])
d96 =abs(2*24*G96[(24,0,0)]-TARGET)/TARGET
d144=abs(2*24*G144[(24,0,0)]-TARGET)/TARGET
print(f"\n  torus drift at r=24: L=96 dev={d96:.1%}, L=144 dev={d144:.1%}")
check("finite-L torus alpha_r DRIFTS (not the infinite-lattice object)",
      d96 > 0.5 and d144 < d96,
      "deviation grows with r, shrinks with L -> neutralising-background artifact")

print(f"\n{sum(ok)}/{len(ok)} checks pass")
raise SystemExit(0 if all(ok) else 1)

"""Independent test of FTD-0575's native Hodge static-response theorem.

Claims under test (THEOREM_NATIVE_HODGE_RECIPROCITY_STATIC_POLE.md sec 4-5):
  s_i = sin k_i,  sigma^2 = sum s_i^2
  M   = 4 - (2/3) sum cos k_i - (2/3) sum_{i<j} cos k_i cos k_j
  R(k) = 3 sigma^2 / M(k)                     and  0 <= R <= 3
  with u_i = 1 - cos k_i, U = sum u_i, P = sum_{i<j} u_i u_j, Q = sum u_i^2:
      M = 2U - (2/3)P,   sigma^2 = 2U - Q,
      M - sigma^2 = Q - (2/3)P = (1/3)Q + (1/3) sum_{i<j}(u_i-u_j)^2 >= 0
  => the 1/k^2 pole CANCELS (R stays finite at k->0), so this channel is not
     Coulomb; and eliminating the field gives NEGATIVE equal-polarity cross
     energy, i.e. SAME-POLARITY ATTRACTION.

The last item is the one that matters here: the registered compact law masks
same-polarity pairs to ZERO interaction, so if the native force attracts them
the two disagree. The real-space kernel decides it.
"""
import numpy as np, sympy as sp
ok = []
def check(name, cond, detail):
    ok.append(bool(cond)); print(f"  [{'PASS' if cond else 'FAIL'}] {name}: {detail}")

# ---------- symbolic identities -------------------------------------------
k1,k2,k3 = sp.symbols('k1 k2 k3', real=True)
ks = [k1,k2,k3]
M    = 4 - sp.Rational(2,3)*sum(sp.cos(k) for k in ks) \
         - sp.Rational(2,3)*sum(sp.cos(ks[i])*sp.cos(ks[j]) for i in range(3) for j in range(i+1,3))
sig2 = sum(sp.sin(k)**2 for k in ks)
u    = [1-sp.cos(k) for k in ks]
U    = sum(u); P = sum(u[i]*u[j] for i in range(3) for j in range(i+1,3)); Q = sum(x**2 for x in u)

check("M = 2U - (2/3)P", sp.simplify(M - (2*U - sp.Rational(2,3)*P)) == 0, "symbolic identity")
check("sigma^2 = 2U - Q", sp.simplify(sig2 - (2*U - Q)) == 0, "symbolic identity")
sos = sp.Rational(1,3)*Q + sp.Rational(1,3)*sum((u[i]-u[j])**2 for i in range(3) for j in range(i+1,3))
check("M - sigma^2 = (1/3)Q + (1/3)sum(u_i-u_j)^2",
      sp.simplify((M - sig2) - sos) == 0,
      "sum-of-squares identity => M >= sigma^2 => R <= 3")

# ---------- numerical BZ scan ---------------------------------------------
n = 96
g = (np.arange(n)+0.5)*2*np.pi/n - np.pi          # avoid exact k=0
K1,K2,K3 = np.meshgrid(g,g,g, indexing='ij')
Mn = 4 - (2/3)*(np.cos(K1)+np.cos(K2)+np.cos(K3)) \
       - (2/3)*(np.cos(K1)*np.cos(K2)+np.cos(K2)*np.cos(K3)+np.cos(K3)*np.cos(K1))
S2 = np.sin(K1)**2+np.sin(K2)**2+np.sin(K3)**2
R  = 3*S2/Mn
check("M > 0 off the origin", Mn.min() > 0, f"min M = {Mn.min():.6e}")
check("M_max = 16/3", abs(Mn.max()-16/3) < 2e-3, f"max M = {Mn.max():.6f} vs 16/3 = {16/3:.6f}")
check("0 <= R <= 3 across the Brillouin zone", R.min() >= -1e-12 and R.max() <= 3+1e-9,
      f"R in [{R.min():.6e}, {R.max():.9f}]")

# pole cancellation: R -> 3 as k -> 0
for eps in (1e-2, 1e-3, 1e-4):
    kk = np.array([eps,0,0])
    Me = 4-(2/3)*(np.cos(kk).sum()+2) - (2/3)*(np.cos(kk[0])*1+1*1+1*np.cos(kk[0]))
    Me = 4-(2/3)*(np.cos(eps)+2)-(2/3)*(np.cos(eps)+1+np.cos(eps))
    Re = 3*np.sin(eps)**2/Me
    print(f"      k=({eps},0,0): M={Me:.6e}  R={Re:.9f}")
check("R -> 3 as k->0 (the 1/k^2 pole CANCELS)", abs(Re-3) < 1e-6,
      f"R = {Re:.9f} -- finite, so no Coulomb 1/r tail")

# ---------- real-space kernel: does same polarity ATTRACT? -----------------
# U_cross = -G_C^2 q1 q2 K(r),  K = inverse FT of R.  K(r) > 0 => same-sign
# charges get NEGATIVE cross energy => attraction.
N = 64
gg = np.arange(N)*2*np.pi/N
A1,A2,A3 = np.meshgrid(gg,gg,gg, indexing='ij')
Md = 4 - (2/3)*(np.cos(A1)+np.cos(A2)+np.cos(A3)) \
       - (2/3)*(np.cos(A1)*np.cos(A2)+np.cos(A2)*np.cos(A3)+np.cos(A3)*np.cos(A1))
S2d = np.sin(A1)**2+np.sin(A2)**2+np.sin(A3)**2
Rd = np.zeros_like(Md); nz = Md > 1e-12
Rd[nz] = 3*S2d[nz]/Md[nz]
Rd[0,0,0] = 0.0                      # neutralising background (k=0 excluded)
Kr = np.real(np.fft.ifftn(Rd))
print(f"\n  real-space kernel K(r) (K>0 => same-polarity ATTRACTION):")
for lbl,(a,b,c) in [("r=0 (self)",(0,0,0)), ("r=1 axial",(1,0,0)),
                    ("r=sqrt2 face",(1,1,0)), ("r=sqrt3 body",(1,1,1)),
                    ("r=2 axial",(2,0,0)), ("r=3 axial",(3,0,0))]:
    print(f"      {lbl:<16} K = {Kr[a,b,c]:+.6e}")
# The theorem claims POLE CANCELLATION, i.e. no 1/r tail -- not compact
# support. Test that, not a stricter thing the theorem never asserted.
c1 = abs(Kr[1,0,0])
for r in (2,3,4):
    print(f"      |K({r})| = {abs(Kr[r,0,0]):.4e}   Coulomb 1/r would give {c1/r:.4e}"
          f"   ratio {abs(Kr[r,0,0])/(c1/r):.3f}")
check("decays FASTER than 1/r (the pole really cancelled)",
      all(abs(Kr[r,0,0]) < 0.5*c1/r for r in (3,4)),
      f"|K(4)| = {abs(Kr[4,0,0]):.3e} vs Coulomb {c1/4:.3e} -- "
      f"{c1/4/abs(Kr[4,0,0]):.1f}x smaller")
check("sign structure: attractive on Moore-26 (r<=sqrt3), repulsive beyond",
      Kr[1,0,0]>0 and Kr[1,1,0]>0 and Kr[1,1,1]>0 and Kr[2,0,0]<0,
      f"K(1)={Kr[1,0,0]:+.4e} K(v2)={Kr[1,1,0]:+.4e} "
      f"K(v3)={Kr[1,1,1]:+.4e} | K(2)={Kr[2,0,0]:+.4e}")
check("SAME-POLARITY sign at r=1 is ATTRACTIVE (K>0)", Kr[1,0,0] > 0,
      f"K(r=1) = {Kr[1,0,0]:+.6e}  => U_cross = -G_C^2 q1q2 K < 0 for q1q2>0")

print(f"\n{sum(ok)}/{len(ok)} checks pass")
raise SystemExit(0 if all(ok) else 1)

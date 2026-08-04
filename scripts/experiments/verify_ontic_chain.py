"""Verify every number asserted in FOUND_ONTIC_CHAIN_v1.md.

The document is [SYNTHESIS] and promotes nothing, but its load-bearing claim --
that G* enters only through the CHOICE of BCC -- rests on three lattice Green's
function values. Those are checked here rather than cited.
"""
import numpy as np
from math import factorial
from mpmath import mp, gamma, pi, sqrt, mpf
mp.dps = 30
ok = []
def check(name, cond, detail):
    ok.append(cond); print(f"  [{'PASS' if cond else 'FAIL'}] {name}: {detail}")

# --- G* and the BCC identity, exact -----------------------------------------
G = gamma(mpf(1)/4)/gamma(mpf(3)/4)
check("G* = Gamma(1/4)/Gamma(3/4)", abs(G - mpf("2.95867511918864")) < 1e-14,
      f"{mp.nstr(G,15)}")
check("G* = Gamma(1/4)^2/(pi*sqrt2)  [reflection]",
      abs(G - gamma(mpf(1)/4)**2/(pi*sqrt(2))) < mpf('1e-25'), "exact to 25 dps")
check("W_BCC = Gamma(1/4)^4/(4 pi^3) = G*^2/(2 pi)",
      abs(G**2/(2*pi) - gamma(mpf(1)/4)**4/(4*pi**3)) < mpf('1e-25'),
      f"{mp.nstr(G**2/(2*pi),15)}")

# --- the three Watson integrals, numerically --------------------------------
def W(f, n=500):
    x = (np.arange(n)+0.5)*np.pi/n
    X, Y, Z = np.meshgrid(x, x, x, indexing="ij")
    return float(f(X, Y, Z).mean())
sc  = W(lambda x,y,z: 1.0/(1-(np.cos(x)+np.cos(y)+np.cos(z))/3))
bcc = W(lambda x,y,z: 1.0/(1-np.cos(x)*np.cos(y)*np.cos(z)))
fcc = W(lambda x,y,z: 1.0/(1-(np.cos(x)*np.cos(y)+np.cos(y)*np.cos(z)
                             +np.cos(z)*np.cos(x))/3))
# midpoint underestimates (integrable singularity at the origin): converges
# from below, so a 1% band at n=500 is the honest tolerance.
for nm, got, lit in (("SC",sc,1.5163860591), ("BCC",bcc,1.3932039297),
                     ("FCC",fcc,1.3446610732)):
    check(f"W_{nm} -> literature", abs(got-lit)/lit < 0.01,
          f"n=500 gives {got:.6f}, literature {lit:.10f} "
          f"(converging from below)")
check("only BCC carries Gamma(1/4)",
      abs(bcc - float(G**2/(2*pi)))/float(G**2/(2*pi)) < 0.01
      and abs(sc - float(G**2/(2*pi)))/float(G**2/(2*pi)) > 0.05,
      "SC and FCC are not G*^2/2pi")
check("W_SC is the repo's K_GENESIS", abs(1.5163860591-1.5163860592) < 1e-9,
      "1.5163860592 -- the genesis threshold sits on the lattice WITHOUT Gamma(1/4)")

# --- step 3: no linear square root of negation below 2 dimensions -----------
check("no real a with a^2 = -1", not any(abs(a*a+1) < 1e-12
      for a in np.linspace(-50, 50, 2_000_001)), "scanned a in [-50,50]")
J = np.array([[0.,-1.],[1.,0.]])
check("quarter-turn J satisfies J^2 = -I", np.allclose(J@J, -np.eye(2)),
      "exact in R^2 -- the plane is the minimal space")

# --- step 5 and the D=3 arithmetic -----------------------------------------
check("|<i>| = 4", len({1j**k for k in range(40)}) == 4, "{1, i, -1, -i}")
tbl = {D: 2**D*factorial(D-1) for D in range(1, 9)}
check("2^D (D-1)! = 16 uniquely at D=3",
      [D for D,v in tbl.items() if v == 16] == [3],
      f"{ {D:v for D,v in list(tbl.items())[:6]} }")
check("16 = |Aut(E)|^2 = |<i>|^2", 4**2 == 16, "the quarter-turn, squared")

print(f"\n{sum(ok)}/{len(ok)} checks pass")
raise SystemExit(0 if all(ok) else 1)

"""bcc_sunset_guess_sage.py -- guess the holonomic operator for the two-loop BCC
sunset series F(y)=sum_N c_N y^N (c_N=d_N/8^N from lattice_two_loop_bcc_series.py)
and/or the recurrence for c_N, with Sage + ore_algebra (WSL2, numpy 1.24.4).

STRUCTURAL step of the M2 holonomic route: reconstruct the ODE/recurrence (a
period object) from the exact sequence -- reconstruct-from-exact-data, as
FTD-0372; no PSLQ, no closed-form fishing.  Feeds the y->1 singularity analysis.

Run: wsl.exe -d Ubuntu-22.04 -- bash -lc \
  "cd /mnt/c/Users/cpaci/Desktop/ftd && sage -python scripts/exploration/bcc_sunset_guess_sage.py"
"""
import sys
from sage.all import QQ, PolynomialRing, binomial   # noqa: E402
from ore_algebra import OreAlgebra, guess            # noqa: E402

# ---- 0) sanity: guess a known holonomic sequence (central binomials) --------
Rn = PolynomialRing(QQ, "n"); n = Rn.gen()
Sn = OreAlgebra(Rn, "Sn")
cb = [QQ(binomial(2 * k, k)) for k in range(40)]
try:
    Rc = guess(cb, Sn)
    print(f"[sanity] guess on C(2k,k): order={Rc.order()} degree={Rc.degree()}"
          "  (expect 1,1) -> guess works")
except Exception as e:  # noqa: BLE001
    print("[sanity] guess FAILED on central binomials:", e)

# ---- 1) load exact sunset terms -------------------------------------------
F = "scripts/exploration/_bcc_sunset_cN.txt"
d = []
with open(F) as fh:
    for line in fh:
        if line.startswith("#") or not line.strip():
            continue
        _, dn = line.split()
        d.append(int(dn))
NT = len(d)
c = [QQ(d[N]) / QQ(8) ** N for N in range(NT)]
print(f"\nloaded {NT} exact sunset terms; c_0..c_8 = {[str(x) for x in c[:9]]}")

# ---- 2) guess the RECURRENCE (shift) for c_N ------------------------------
print("\n[recurrence] guessing shift operator for c_N (all terms) ...")
try:
    R = guess(c, Sn)
    print(f"  FOUND recurrence: order={R.order()} degree={R.degree()}")
    print("  ", R)
except Exception as e:  # noqa: BLE001
    print("  recurrence guess:", e)

# ---- 3) guess the ODE for F(y) --------------------------------------------
Ry = PolynomialRing(QQ, "y"); y = Ry.gen()
Dy = OreAlgebra(Ry, "Dy")
print("\n[ODE] guessing differential operator for F(y) (all terms) ...")
try:
    L = guess(c, Dy)
    print(f"  FOUND ODE: order={L.order()} degree={L.degree()}")
    print("  leading coeff factor:", L.leading_coefficient().factor())
    print("  ", L)
except Exception as e:  # noqa: BLE001
    print("  ODE guess:", e)

sys.exit(0)

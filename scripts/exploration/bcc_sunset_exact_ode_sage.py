"""bcc_sunset_exact_ode_sage.py -- reconstruct the EXACT (over QQ) annihilating
operator of the two-loop BCC sunset F(y)=sum c_N y^N from the exact c_N
(lattice_two_loop_bcc_series.py), and confirm the y=1 local exponents exactly
(not just mod p). Saves the operator for the Arb connection step (-> B).

Run: wsl.exe -d Ubuntu-22.04 -- bash -lc \
  "cd /mnt/c/Users/cpaci/Desktop/ftd && sage -python scripts/exploration/bcc_sunset_exact_ode_sage.py"
"""
import json
import sys
from sage.all import QQ, PolynomialRing   # noqa: E402
from ore_algebra import OreAlgebra, guess  # noqa: E402

d = []
for line in open("scripts/exploration/_bcc_sunset_cN.txt"):
    if line.startswith("#") or not line.strip():
        continue
    d.append(int(line.split()[1]))
NT = len(d)
c = [QQ(d[N]) / QQ(8) ** N for N in range(NT)]
print(f"loaded {NT} exact terms")

Ry = PolynomialRing(QQ, "y"); y = Ry.gen()
Dy = OreAlgebra(Ry, "Dy")

print("guessing EXACT minimal ODE for F(y) over QQ (may be slow) ...")
L = None
try:
    L = guess(c, Dy)
    print(f"  FOUND ODE over QQ: order={L.order()}, degree={L.degree()}")
except Exception as e:  # noqa: BLE001
    print("  QQ ODE guess:", e)

if L is None:
    print("  falling back: guess order-18 recurrence over QQ, then to_D ...")
    Rn = PolynomialRing(QQ, "n"); Sn = OreAlgebra(Rn, "Sn")
    R = guess(c, Sn)
    print(f"  recurrence over QQ: order={R.order()}, degree={R.degree()}")
    L = R.to_D(Dy)
    print(f"  to_D ODE: order={L.order()}, degree={L.degree()} (may be non-minimal)")

# exact singular points + local exponents at y=1 (confirm +/-1/4 over QQ)
print("\n  leading coeff factor (QQ):", L.leading_coefficient().factor())
for shift, lab in [(QQ(1), "y=1 PHYSICAL"), (QQ(-1), "y=-1"), (QQ(0), "y=0")]:
    try:
        Ls = L.annihilator_of_composition(y + shift) if shift != 0 else L
        ip = Ls.indicial_polynomial(y)
        roots = ip.roots(QQ, multiplicities=True)
        exps = sorted([(QQ(r), m) for r, m in roots])
        print(f"  {lab}: exponents = {exps}  (sum mult {sum(m for _,m in roots)})")
    except Exception as e:  # noqa: BLE001
        print(f"  {lab}: {e}")

# save exact operator coefficients (list of QQ polynomials -> list of coeff lists)
polys = []
for p in L.list():
    polys.append([str(QQ(cc)) for cc in p.list()])
with open("scripts/exploration/_bcc_sunset_exact_ode.json", "w") as f:
    json.dump({"order": int(L.order()), "degree": int(L.degree()),
               "polys": polys}, f)
print("\n  saved exact operator to scripts/exploration/_bcc_sunset_exact_ode.json")
sys.exit(0)

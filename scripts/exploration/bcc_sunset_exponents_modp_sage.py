"""bcc_sunset_exponents_modp_sage.py -- guess the MINIMAL ODE for F(y)=sum c_N y^N
over GF(p) and read the LOCAL EXPONENTS at the singular points (y=1 physical,
y=-1 parity, y=0), the genus discriminant (cf. FTD-0372/0373 for W_18).

Exponents come out mod p; rational_reconstruct maps small rationals (expect a
(1/2)Z lattice) back.  The exponent multiset at y=1 feeds the self-duality test.

Run: wsl.exe -d Ubuntu-22.04 -- bash -lc \
  "cd /mnt/c/Users/cpaci/Desktop/ftd && sage -python scripts/exploration/bcc_sunset_exponents_modp_sage.py [modp_file]"
"""
import sys
from sage.all import GF, PolynomialRing, QQ            # noqa: E402
from ore_algebra import OreAlgebra, guess               # noqa: E402

FN = sys.argv[1] if len(sys.argv) > 1 else \
    "scripts/exploration/_bcc_sunset_cN_modp_1048573.txt"
P = None; rows = []
for line in open(FN):
    if line.startswith("#"):
        for t in line.replace("=", " ").split():
            if t.isdigit() and int(t) > 1000:
                P = int(t); break
        continue
    if line.strip():
        rows.append(int(line.split()[1]))
if P is None:
    P = int(FN.split("_")[-1].split(".")[0])
NT = len(rows)
Fp = GF(P)
c = [Fp(x) for x in rows]
print(f"loaded {NT} terms mod p={P}")


def rat(x):
    """rational_reconstruct mod p -> QQ (small rationals), else str."""
    try:
        return Fp(x).rational_reconstruction()
    except Exception:
        return f"{int(x)}(modp)"


Ry = PolynomialRing(Fp, "y"); y = Ry.gen()
Dy = OreAlgebra(Ry, "Dy")
print("guessing MINIMAL ODE for F(y) over GF(p) ...")
try:
    L = guess(c, Dy)
except Exception as e:  # noqa: BLE001
    print("  ODE guess failed:", e, "\n  (need more terms -> use the 2400 build)")
    sys.exit(0)
print(f"  minimal ODE: order={L.order()}, degree={L.degree()}")
lc = L.leading_coefficient()
print(f"  leading coeff factors: {lc.factor()}")

for shift, label in [(Fp(1), "y=1 (mu^2->0 PHYSICAL)"),
                     (Fp(-1), "y=-1 (parity)"),
                     (Fp(0), "y=0")]:
    try:
        # move the singular point to 0 via y -> y + shift, then indicial at 0
        Ls = L.annihilator_of_composition(y + shift) if shift != 0 else L
        ip = Ls.indicial_polynomial(y)
        roots = ip.roots(multiplicities=True)
        exps = sorted([(rat(r), m) for r, m in roots], key=lambda t: str(t[0]))
        tot = sum(m for _, m in roots)
        print(f"\n  {label}: exponents (value, mult) = {exps}   (sum mult={tot})")
    except Exception as e:  # noqa: BLE001
        print(f"\n  {label}: indicial error: {e}")

print("\n(Local exponents at y=1 are the genus signature: half-integer lattice +"
      " MUM/log structure as for W_18; the multiset feeds the FTD-0373 self-"
      "duality test a+d=b+c. mod-p exponents rational-reconstructed.)")
sys.exit(0)

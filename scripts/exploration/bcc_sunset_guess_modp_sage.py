"""bcc_sunset_guess_modp_sage.py -- guess & HOLDOUT-CERTIFY the holonomic
recurrence for the two-loop BCC sunset c_N, working over GF(p) so large N is
cheap (c_N mod p from lattice_two_loop_bcc_modp.py).

Method: guess the shift operator from the first (NT-HOLD) terms, then use it to
regenerate the held-out last HOLD terms and compare -- a genuine holdout test
(an underdetermined/spurious operator fails it).  Reports order, degree, and
the leading-coefficient factorization mod p (singular structure of the ODE for
F(y): the roots include y=1, y=-1 (parity) and any others).

Run: wsl.exe -d Ubuntu-22.04 -- bash -lc \
  "cd /mnt/c/Users/cpaci/Desktop/ftd && sage -python scripts/exploration/bcc_sunset_guess_modp_sage.py [modp_file] [HOLD]"
"""
import sys
from sage.all import GF, PolynomialRing   # noqa: E402
from ore_algebra import OreAlgebra, guess  # noqa: E402

FN = sys.argv[1] if len(sys.argv) > 1 else \
    "scripts/exploration/_bcc_sunset_cN_modp_1048573.txt"
HOLD = int(sys.argv[2]) if len(sys.argv) > 2 else 250

# parse p from header/filename
P = None
rows = []
for line in open(FN):
    if line.startswith("#"):
        for tok in line.replace("=", " ").split():
            if tok.isdigit() and int(tok) > 1000:
                P = int(tok); break
        continue
    if line.strip():
        rows.append(int(line.split()[1]))
if P is None:
    P = int(FN.split("_")[-1].split(".")[0])
NT = len(rows)
print(f"loaded {NT} terms mod p={P}; holdout last {HOLD}")

Fp = GF(P)
c = [Fp(x) for x in rows]
Rn = PolynomialRing(Fp, "n"); n = Rn.gen()
Sn = OreAlgebra(Rn, "Sn")

ngess = NT - HOLD
print(f"guessing shift recurrence from first {ngess} terms over GF({P}) ...")
R = guess(c[:ngess], Sn)
r, deg = R.order(), R.degree()
print(f"  FOUND recurrence: order={r}, degree={deg}")

# holdout certification: regenerate the full sequence from the recurrence and
# compare to ALL known terms (including the HOLD terms it never saw)
try:
    gen = R.to_list(c[:r], NT)          # initial r terms -> generate NT terms
    # to_list needs enough initial conditions and a nonvanishing leading coeff;
    # count mismatches over the held-out tail specifically
    mism = [N for N in range(ngess, NT) if gen[N] != c[N]]
    print(f"  HOLDOUT: recurrence reproduces held-out terms N={ngess}..{NT-1}: "
          f"{'PASS (%d/%d)' % (HOLD - len(mism), HOLD) if not mism else 'FAIL (%d mismatch)' % len(mism)}")
except Exception as e:  # noqa: BLE001
    print("  holdout via to_list error:", e)
    # fallback: check the recurrence annihilates the tail directly
    coeffs = R.list()  # list of poly coeffs p_0(n)..p_r(n)
    def ann_ok(N):
        s = Fp(0)
        for k, pk in enumerate(coeffs):
            s += pk(N) * c[N + k]
        return s == 0
    bad = [N for N in range(ngess - r, NT - r) if not ann_ok(N)]
    print(f"  HOLDOUT (annihilation): {'PASS' if not bad else 'FAIL %d' % len(bad)}")

# singular structure of the ODE for F(y): convert rec -> diff op, factor lead
try:
    Ry = PolynomialRing(Fp, "y"); y = Ry.gen()
    Dy = OreAlgebra(Ry, "Dy")
    L = R.to_D(Dy)
    print(f"\n  ODE for F(y): order={L.order()}, degree={L.degree()}")
    lc = L.leading_coefficient()
    print(f"  leading coeff factors mod p: {lc.factor()}")
    print("  (roots include the singular points; y=1 is the mu^2->0 point,")
    print("   y=-1 the bipartite/parity point.)")
except Exception as e:  # noqa: BLE001
    print("  rec->ODE conversion error:", e)

sys.exit(0)

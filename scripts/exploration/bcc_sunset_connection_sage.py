"""bcc_sunset_connection_sage.py -- rigorous high-precision finite part B of the
two-loop BCC sunset via ore_algebra Arb analytic continuation.

F(y)=sum c_N y^N is analytic at y=0 (radius 1, singularities at y=+-1). Take its
initial data (Taylor coeffs) at the ORDINARY point y0=1/2 from the exact c_N, then
numerical_transition_matrix([1/2, 1]) transports to the local (Frobenius) basis at
the singular point y=1. Near y=1, F = -A_s log(1-y) + B + (vanishing (1-y)^{k/2,
k/4} terms). Reading the exponent-0 sector gives B (with the log coeff cross-
checking -A_s = -4/pi^2, and Im = pi*A_s = 4/pi from the log(y-1)=log(1-y)+i*pi
branch). Rigorous Arb balls => certified digits.

Run: wsl.exe -d Ubuntu-22.04 -- bash -lc \
  "cd /mnt/c/Users/cpaci/Desktop/ftd && sage -python scripts/exploration/bcc_sunset_connection_sage.py [bits]"
"""
import json
import sys
from sage.all import (QQ, PolynomialRing, RealBallField, ComplexBallField,
                      binomial, vector, pi, log, SR)  # noqa: E402
from ore_algebra import OreAlgebra  # noqa: E402

BITS = int(sys.argv[1]) if len(sys.argv) > 1 else 300

# rebuild exact operator
with open("scripts/exploration/_bcc_sunset_exact_ode.json") as f:
    D = json.load(f)
Ry = PolynomialRing(QQ, "y"); y = Ry.gen()
Dy = OreAlgebra(Ry, "Dy"); Dz = Dy.gen()
L = Dy.zero()
for r, coeffs in enumerate(D["polys"]):
    L += sum(QQ(c) * y ** j for j, c in enumerate(coeffs)) * Dz ** r
r = L.order()
print(f"operator order={r}, degree={L.degree()}; working at {BITS} bits")

# exact c_N
d = []
for line in open("scripts/exploration/_bcc_sunset_cN.txt"):
    if line.startswith("#") or not line.strip():
        continue
    d.append(int(line.split()[1]))
NT = len(d)
RBF = RealBallField(BITS)
c = [RBF(d[N]) / RBF(8) ** N for N in range(NT)]

# Taylor coeffs a_k of F at y0=1/2: a_k = sum_{N>=k} c_N C(N,k) (1/2)^{N-k}
y0 = QQ(1) / 2
ak = []
for k in range(r):
    s = RBF(0)
    for N in range(k, NT):
        s += c[N] * RBF(binomial(N, k)) * RBF(y0) ** (N - k)
    ak.append(s)
print(f"  F(1/2)=a_0 = {ak[0]}")
print(f"  a_1 = {ak[1]}   a_2 = {ak[2]}")

# transport to y=1 (singular): transition matrix maps local-basis coords 1/2 -> 1
eps = RBF(2) ** (-(BITS - 20))
print(f"\n  computing numerical_transition_matrix([1/2, 1], eps~2^-{BITS-20}) ...")
M = L.numerical_transition_matrix([y0, 1], eps)
mons = L.local_basis_monomials(1)
print("  local_basis_monomials(1) =", mons)

# coords of F in the ordinary-point basis at 1/2 are the Taylor coeffs a_k
CBF = ComplexBallField(BITS)
v = vector(CBF, [CBF(a) for a in ak])
res = M * v
print("\n  F coords in local basis at y=1 (index : monomial : coord):")
for i, mon in enumerate(mons):
    print(f"    [{i}] {mon} : {res[i]}")

# identify the exponent-0 constant (monomial == 1) and the log monomial
idx1 = next(i for i, m in enumerate(mons) if m == SR(1))
idxlog = next(i for i, m in enumerate(mons) if m == log(y - 1))
A_s = 4 / (RBF(pi) ** 2)
Bball = res[idx1].real()
logc = res[idxlog].real()
imc = res[idx1].imag()
print(f"\n  A_s = 4/pi^2 = {A_s}")
print(f"  log-coord (should = -A_s): {logc}")
print(f"  Im(const coord) (should = pi*A_s = 4/pi): {imc}")
# validation
ok_log = (logc + A_s).contains_zero()
ok_im = (imc - RBF(pi) * A_s).contains_zero()
print(f"  [validate] log-coord == -4/pi^2: {ok_log}; Im == 4/pi: {ok_im}")
print(f"\n  ===> B = {Bball}")
# save B midpoint to full precision for PSLQ
with open("scripts/exploration/_bcc_sunset_B.txt", "w") as f:
    f.write(f"# two-loop BCC sunset finite part B, {BITS}-bit Arb connection\n")
    f.write(f"# rad ~ {Bball.rad()}\n")
    f.write(Bball.mid().str(truncate=False) + "\n")
print(f"  saved B (midpoint) to scripts/exploration/_bcc_sunset_B.txt")
sys.exit(0)

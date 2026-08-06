"""FTD-0784 numerical certificate: moment expressibility of the
master-quadratic invariants and the surd identities, 30+ digits."""
from mpmath import mp, mpf, gamma, sqrt, pi, beta

mp.dps = 40
G = gamma(mpf(1)/4) / gamma(mpf(3)/4)
delta = sqrt(G*(4*G - 1))
xp, xm = 8*G**2 + 4*G*delta, 8*G**2 - 4*G*delta
m2 = beta(mpf(3)/4, mpf(1)/2) / beta(mpf(1)/4, mpf(1)/2)   # <x^2>
m1 = beta(mpf(1)/2, mpf(1)/2) / beta(mpf(1)/4, mpf(1)/2)   # <|x|>
checks = {
    "m2 = 4/G*^2":               abs(m2 - 4/G**2),
    "m1 = sqrt(pi)/G*":          abs(m1 - sqrt(pi)/G),
    "tr = 16G*^2 = 64/m2":       abs(64/m2 - (xp + xm)),
    "det = 16G*^3 = 64rt(pi)/(m2 m1)": abs(64*sqrt(pi)/(m2*m1) - xp*xm),
    "splitting = 8 G* delta":    abs((xp - xm) - 8*G*delta),
    "delta^2 = 4G*^2 - G*":      abs(delta**2 - (4*G**2 - G)),
}
for k, v in checks.items():
    print(f"  {k:36s} residual {float(v):.2e}")
assert all(v < mpf(10)**-27 for v in checks.values())
print(f"x+ = {xp}")
print(f"     CODATA 1/alpha = 137.035999177(21); rel dev "
      f"{float(abs(xp-mpf('137.035999177'))/mpf('137.035999177')):.3e}")
print("ALL PASS")

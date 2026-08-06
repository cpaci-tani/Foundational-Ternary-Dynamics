"""The decisive window: at each candidate eps, is there an amplitude range
simultaneously ABOVE BAND and acceptably QUARTIC?"""
from mpmath import mp, mpf, sqrt, pi, quad, sin, asin, gamma, findroot

mp.dps = 25
m_eff, d_sep = mpf(2)/3, 1/sqrt(mpf(2))
band_field, band_wave = 2*asin(1/sqrt(mpf(3))), mpf(2)
W_SC = (sqrt(6)/(32*pi**3))*gamma(mpf(1)/24)*gamma(mpf(5)/24)*gamma(mpf(7)/24)*gamma(mpf(11)/24)

def omega(A, eps):
    lam, mu6 = 24*eps, 32*eps
    g = lambda d: lam*(A**2+d**2) - mu6*(A**4+A**2*d**2+d**4)
    return 2*pi/(4*sqrt(m_eff/2)*quad(lambda th: 1/sqrt(g(A*sin(th))), [0, pi/2]))

def contamination(A): return mpf(4)/3 * A**2      # sextic/quartic at amplitude A

def window(eps, band, name):
    # omega rises then falls; find where it exceeds `band`
    grid = [d_sep*mpf(k)/200 for k in range(1, 200)]
    above = [(A, omega(A, eps)) for A in grid]
    above = [(A, w) for A, w in above if w > band]
    if not above:
        wmax = max(w for _, w in [(A, omega(A, eps)) for A in grid])
        print(f"    {name:<14} NO WINDOW (Omega_max = {float(wmax):.4f} < {float(band):.4f})")
        return
    A_lo, A_hi = above[0][0], above[-1][0]
    c_lo, c_hi = contamination(A_lo), contamination(A_hi)
    print(f"    {name:<14} window A in [{float(A_lo):.4f}, {float(A_hi):.4f}] "
          f"({float(A_lo/d_sep)*100:.0f}%-{float(A_hi/d_sep)*100:.0f}% of separatrix)")
    print(f"    {'':14} contamination at entry {float(c_lo)*100:5.1f}%  "
          f"-> {'USABLE' if c_lo < mpf('0.10') else 'contaminated' if c_lo < mpf('0.25') else 'BADLY contaminated'}")

cands = [("SELECTED", mpf("0.01")),
         ("E_F(1) field", mpf("0.00056112859596711728")),
         ("K_MANIFEST", W_SC/3),
         ("W_SC - 1", W_SC - 1),
         ("K_GENESIS", W_SC)]

print("=== IS THERE AN ABOVE-BAND *AND* CLEAN-QUARTIC WINDOW? ===")
for nm, e in cands:
    print(f"\n  eps = {float(e):.6f}   [{nm}]")
    window(e, band_field, "field band")
    window(e, band_wave,  "wave band")

print("\n\n=== THE EXACT LATTICE STRUCTURE ===")
G0, G1 = W_SC/6, W_SC/6 - mpf(1)/6
print(f"  SC lattice Green's function, coordination z = 6:")
print(f"    z*G(0) = W_SC     = {W_SC}   <- engine K_GENESIS, EXACT match")
print(f"    z*G(1) = W_SC - 1 = {W_SC - 1}   <- the nearest-neighbour analogue")
print(f"  because G(1) = G(0) - 1/z follows from the discrete Laplacian at the")
print(f"  origin: z[G(0) - G(1)] = 1. So the pair of scales is exact, not fitted.")
print(f"\n  If K_GENESIS = z*G(0) is the SELF-energy quantum (registered, exact),")
print(f"  then z*G(1) = W_SC - 1 is the NEAREST-NEIGHBOUR quantum by the same")
print(f"  normalisation -- the natural candidate for a nearest-neighbour bond depth.")
print(f"\n  eps_candidate = W_SC - 1 = {float(W_SC-1):.10f}")
print(f"  vs selected      eps     = 0.01          ({float((W_SC-1)/mpf('0.01')):.1f}x)")

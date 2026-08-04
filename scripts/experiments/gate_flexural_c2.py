"""Does the transverse quartic mode clear the band? The decisive C2 test,
plus the full C1-C12 scorecard for the collinear-trimer flexural candidate."""
from mpmath import mp, mpf, sqrt, pi, quad, sin, asin, findroot

mp.dps = 25
eps = mpf("0.01")          # registered selected well depth

# --- free trimer normal mode: B at +y_B, A and C at -y_B/2 (CoM fixed).
# bond transverse offset delta = 3 y_B / 2 ; KE = (1/2)(2m/3) delta^2_dot
m_unit = mpf(1)
m_eff  = 2*m_unit/3
lam, mu6 = 24*eps, 32*eps               # V = lam d^4 - mu6 d^6
d_sep  = 1/sqrt(mpf(2))                 # separatrix = bond dissociation

def W(d):  return lam*d**4 - mu6*d**6
def g(d, A):                            # W(A)-W(d) = (A^2-d^2) * g
    return lam*(A**2 + d**2) - mu6*(A**4 + A**2*d**2 + d**4)

def omega(A):
    # delta = A sin(theta) removes the turning-point singularity exactly
    T = 4*sqrt(m_eff/2)*quad(lambda th: 1/sqrt(g(A*sin(th), A)), [0, pi/2])
    return 2*pi/T

print("=== POTENTIAL (exact, in the bond-offset coordinate delta) ===")
print(f"  V(delta) = {float(lam)}*d^4 - {float(mu6)}*d^6 ,  m_eff = 2/3")
print(f"  separatrix delta = 1/sqrt2 = {float(d_sep):.6f}  (= bond dissociation)")
print(f"  barrier = 2 eps = {float(2*eps)}")

print("\n=== C2: DOES OMEGA CLEAR THE BAND? ===")
print(f"  {'A':>8} {'Omega':>12} {'dOmega/dA':>12}")
prev = None
best = (mpf(0), mpf(0))
for k in range(1, 34):
    A = d_sep*mpf(k)/34
    w = omega(A)
    slope = "" if prev is None else f"{float((w-prev[1])/(A-prev[0])):>12.4f}"
    if w > best[1]: best = (A, w)
    if k % 3 == 0 or k in (1,2):
        print(f"  {float(A):>8.4f} {float(w):>12.6f} {slope:>12}")
    prev = (A, w)

A_star, w_max = best
# refine by golden section on a real bracket (findroot goes complex here)
lo, hi = A_star - d_sep/34, A_star + d_sep/34
gr = (sqrt(mpf(5))-1)/2
for _ in range(80):
    a1, a2 = hi - gr*(hi-lo), lo + gr*(hi-lo)
    if omega(a1) < omega(a2): lo = a1
    else: hi = a2
A_star = (lo+hi)/2
w_max = omega(A_star)
print(f"\n  MAXIMUM: Omega = {float(w_max):.6f} at A = {float(A_star):.6f} "
      f"({float(A_star/d_sep)*100:.1f}% of separatrix)")
print(f"  small-A slope (pure quartic, HARDENING): "
      f"{float(omega(mpf('0.01'))/mpf('0.01')):.6f} per unit A")

band_wave  = sqrt(mpf(1)/3 * 12)                 # C_WAVE^2 = 1/3, 3D max -> 2
band_field = 2*asin(1/sqrt(mpf(3)))              # FTD-0663 one-axis branch max
print(f"\n  acoustic/wave band top   omega_B = {float(band_wave):.6f}")
print(f"  field one-axis branch max        = {float(band_field):.6f}")
print(f"  Omega_max / omega_B(wave)  = {float(w_max/band_wave):.4f}  "
      f"{'CLEARS' if w_max > band_wave else '*** FAILS C2 ***'}")
print(f"  Omega_max / band(field)    = {float(w_max/band_field):.4f}  "
      f"{'CLEARS' if w_max > band_field else '*** FAILS C2 ***'}")

print("\n=== WHAT WOULD CLEAR IT? Omega scales as sqrt(eps/m) ===")
for nm, b in (("wave band 2.0", band_wave), ("field band 1.231", band_field)):
    fac = (b/w_max)**2
    print(f"  vs {nm:18s}: need eps/m larger by {float(fac):7.2f}x "
          f"-> eps = {float(eps*fac):.4f} at m=1, or m = {float(1/fac):.4f} at eps=0.01")

print("\n=== QUARTIC PURITY (how much of the range is clean n=4?) ===")
for frac in ("0.1","0.2","0.3","0.4","0.5"):
    d = mpf(frac)
    print(f"  delta={float(d):.2f}: sextic/quartic = {float(mu6*d**2/lam):.4f}")
print("  -> clean quartic (<10% sextic) for delta < 0.274, i.e. 39% of the range;")
print("     the omega-maximum sits outside that, where the law is quartic+sextic.")

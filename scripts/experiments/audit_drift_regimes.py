"""Audit of the Opus 5 turns: re-audit correction, G*-less synthesis, Eisenstein reply."""
import numpy as np
from mpmath import mp, mpf, gamma, pi, sqrt, beta, cos as mcos, nstr

mp.dps = 25

print("=" * 74)
print("A1  The re-audit said 'harmonic + drift is genuinely not closed in q'.")
print("    Is that true for ALL drift, or only slow drift?")
print("=" * 74)
w, A = 2 * np.pi / 40.0, 3.0
t = np.arange(60000.0)
print(f"  A*w (max oscillatory velocity) = {A*w:.4f}")
for v in (0.01, 1000.0):
    q = A * np.sin(w * t) + v * t
    monotone = bool(np.all(np.diff(q) > 0))
    print(f"\n  slope v = {v}:  dq/dt in [{v - A*w:.3f}, {v + A*w:.3f}]  ->  monotone: {monotone}")
    if monotone:
        print("    q monotone => t = t(q) exists => qddot = F(q) IS single-valued.")
        print("    So this case IS closed - the re-audit's 'genuinely not closed' is")
        print("    WRONG here. But note: ANY monotone record is trivially 'closed'")
        print("    this way, so single-valuedness is VACUOUS for single-pass records.")
    else:
        a = -w * w * A * np.sin(w * t)
        # multivalued check: same q, different a
        order = np.argsort(q)
        qs, as_ = q[order], a[order]
        # spread of a within narrow q-windows
        nb = 300
        edges = np.linspace(qs[0], qs[-1], nb + 1)
        idx = np.digitize(qs, edges) - 1
        spreads = [np.ptp(as_[idx == k]) for k in range(nb) if (idx == k).sum() > 10]
        print(f"    median within-bin spread of qddot = {np.median(spreads):.4f}"
              f"  vs |qddot|max = {w*w*A:.4f}")
        print("    -> genuinely multi-valued; 'not closed' CORRECT for slow drift.")

print("""
  A1 VERDICT: the committed re-audit correction is INCOMPLETE. Three regimes:
    v < A*w   : non-monotone, multi-valued  -> not closed  (metric ~0 correct)
    v > A*w   : monotone, F(q) exists       -> closure VACUOUS (single-pass;
                M3 stationarity is uncomputable, exactly the 'n/a' v1 reported)
    pure line : zero-variance target        -> degenerate (0/0)
  The slope-1000 row currently sits under 'genuinely not closed', which is wrong.
  Gate A needs an explicit RECURRENCE PRECONDITION: coordinate values must be
  revisited, else the verdict is UNINFORMATIVE by construction.
""")

print("=" * 74)
print("A2  Eisenstein reply: verify the numbers, and the automorphism claim")
print("=" * 74)
G3 = gamma(mpf(1)/3) / gamma(mpf(5)/6)
Gs = gamma(mpf(1)/4) / gamma(mpf(3)/4)
print(f"  B(1/3,1/2) - sqrt(pi)G3* = {nstr(beta(mpf(1)/3, mpf(1)/2) - sqrt(pi)*G3, 5)}   (0 = identity holds)")
print(f"  <|x|^3> = {nstr(beta(mpf(4)/3, mpf(1)/2)/beta(mpf(1)/3, mpf(1)/2), 12)}   claimed 2/5 = 0.4")
print(f"  <|x|^6> = {nstr(beta(mpf(7)/3, mpf(1)/2)/beta(mpf(1)/3, mpf(1)/2), 12)}   claimed 16/55 = {nstr(mpf(16)/55, 12)}")
print(f"  G3* G*  = {nstr(G3*Gs, 10)}   vs b3 = 7: off by {nstr((G3*Gs/7 - 1)*100, 3)}%")
print("""
  Numbers verify. BUT: 'the n=3 clock is the Eisenstein clock, carrying a
  natural order-3 automorphism' repeats the exact pattern section 26.2 refuted
  at n=4: the C3 automorphism x -> omega x acts on the COMPLEXIFIED curve, not
  on the real orbit. The real orbit of the even potential |q|^3 has symmetry
  Z2 x Z2, same as any even potential. What IS solid (and stronger than what
  was claimed): on the x>0 branch, xdd = -x^2 is the equianharmonic
  Weierstrass equation (g2 = 0, j = 0, tau = rho) - a branch-level
  identification exactly parallel to cn at n=4. The automorphism phrasing
  should not survive into any document.
""")

print("=" * 74)
print("A3  G*-less synthesis: harmonic pendulum ratio exactly 2 -> 1.6%?")
print("=" * 74)
# harmonic clock: x = cos(theta); C(phi) = cos(phi)/2; Vbar = 1/2 - cos(phi)/2
barrier = mpf(1)          # Vbar(pi)-Vbar(0) = 1
curv = mpf(1)/2           # Vbar''(0) = 1/2
print(f"  harmonic barrier/curvature = {barrier/curv}   quartic = {nstr(48*pi/Gs**4, 10)}")
print(f"  discrimination = {nstr((2 - 48*pi/Gs**4)/2*100, 4)}%   (claimed 1.63%)")

print()
print("=" * 74)
print("A4  FTD-0780 precision: doc says spread 2.3e-4; independent SHM estimate")
print("    (previous run): a0=1.089698 (sd 5e-3), a1=1.091399, a2=1.091453")
print(f"    -> spread 1.6e-3. Estimator-dependent by ~7x; conclusion (needs")
print(f"       factor 4 = 3e-1) unchanged, but the doc should carry both.")

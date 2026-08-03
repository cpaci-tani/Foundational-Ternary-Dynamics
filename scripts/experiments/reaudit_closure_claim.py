"""Re-audit the refutation. Does 'the metric is degenerate on drift' hold?"""
import numpy as np

print("=" * 74)
print("Q1  Is 'harmonic oscillator + linear drift' ACTUALLY a closed system in q?")
print("=" * 74)
print("""
  The auditor's ground truth was: a = -w^2 q exactly, then ADD DRIFT, and called
  the result 'perfect closure' that the metric wrongly scored 0.

  But if  q(t) = A sin(wt) + v t  then
        qddot(t) = -w^2 A sin(wt) = -w^2 (q - v t).
  qddot depends on q AND t. It is NOT a single-valued function of q.
""")

w, A, v = 2 * np.pi / 40.0, 3.0, 0.01
t = np.arange(60000.0)
q = A * np.sin(w * t) + v * t
a = -w**2 * A * np.sin(w * t)          # exact acceleration of that trajectory

# same q value, different accelerations?
target = q[5000]
hits = np.where(np.abs(q - target) < 1e-4)[0]
print(f"  q = {target:.6f} is attained at {len(hits)} times.")
if len(hits) > 1:
    print(f"    accelerations there: {np.array2string(a[hits][:6], precision=5)}")
    print(f"    spread = {np.ptp(a[hits]):.6f}  vs |a|max = {np.abs(a).max():.6f}")
    print(f"    -> ratio {np.ptp(a[hits])/np.abs(a).max():.3f}")
print("""
  VERDICT: the map q -> qddot is genuinely MULTI-VALUED once drift is present.
  A harmonic oscillator plus drift is NOT closed in the coordinate q, so a
  closure metric returning ~0 is CORRECT, not degenerate.
""")

print("=" * 74)
print("Q2  So what IS the degenerate case?")
print("=" * 74)
line = 5.0 + 286.6 * t
print("""
  Pure straight line: qddot == 0 identically. That IS single-valued in q
  (trivially), so it IS closed - but the TARGET has zero variance, and
  R^2 = 1 - var(resid)/var(total) is 0/0. THAT is the degeneracy.
""")
print(f"  straight line: var(qddot) = {np.var(np.diff(line, 2)):.3e}  (exactly zero up to roundoff)")
print("""
  So the refutation conflated two different failures:
    (a) oscillator + drift  -> metric ~0 because NOT CLOSED   [metric correct]
    (b) pure straight line  -> metric ~0 because 0/0          [metric degenerate]
  Only (b) is a defect. q_active post-saturation is case (b).
""")

print("=" * 74)
print("Q3  Does detrending change the QUESTION being asked?")
print("=" * 74)
print("""
  v2 detrends before binning. For q = ramp + oscillation that removes the ramp
  and asks: 'is the oscillatory residual closed in the co-moving frame?'
  That is a legitimate question - free drift is a zero mode and removing it is a
  frame choice - but it is NOT the same question as 'is q itself a natural
  coordinate'. The answer can differ, and the choice must be declared.
""")
qd = q - np.polyval(np.polyfit(np.linspace(-1, 1, len(q)), q, 3), np.linspace(-1, 1, len(q)))
ad = np.full_like(qd, np.nan); ad[1:-1] = qd[2:] - 2 * qd[1:-1] + qd[:-2]
m = np.isfinite(ad)
print(f"  raw q      : corr(q, qddot)  = {np.corrcoef(q[m], a[m])[0,1]:+.4f}   (weak - not closed in q)")
print(f"  detrended  : corr(qd, qddot) = {np.corrcoef(qd[m], ad[m])[0,1]:+.4f}   (strong - closed in co-moving frame)")
print("""
  Both numbers are correct answers to DIFFERENT questions. Detrending is
  defensible for this corpus, but it must be stated as a frame choice.
""")

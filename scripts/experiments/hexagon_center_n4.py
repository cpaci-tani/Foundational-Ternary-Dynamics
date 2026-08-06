"""The minimum geometry candidate: planar hexagon + centre, unit edges.

Why this configuration. Under the registered compact law an unstressed
equilibrium forces every bond to r = 1, and support is 1.2247 -- between 1 and
sqrt(2) -- so no second neighbour can ever bond. The hexagon+centre is the
smallest configuration I can construct where every pair is EITHER at exactly 1
(ring neighbours, spokes) OR at sqrt(3) = 1.732 / 2.0 (alternating and
opposite vertices), i.e. cleanly outside support. Nothing unwanted bonds.

Polarity: the ring is a 6-cycle, hence bipartite, so it 2-colours +,-,+,-,+,-
and every ring edge is a full-strength opposite-polarity bond. The centre must
bond to BOTH signs, which only s = 0 can do -- mask (1 - s_i s_j)/2 = 1/2.
So the spokes are HALF stiffness. This uses the neutral state essentially.

It has a self-stress (omega_spoke = -omega_ring, solved by hand) and, being
planar, every out-of-plane displacement is an infinitesimal flex. Both of
Connelly's ingredients are present. The question is whether the quartic form
is positive DEFINITE on the flex space, or indefinite.
"""
import numpy as np
from maxwell_c3_screen import (EPS, mask, energy, grad, hessian, n_bonds,
                               trivial_modes, null_beyond_trivial,
                               relaxed_profile, bond_set)

# geometry: centre at origin, six vertices on the unit circle
ang = np.arange(6) * np.pi / 3
p = np.vstack([np.zeros(3), np.c_[np.cos(ang), np.sin(ang), np.zeros(6)]])
s = np.array([0, 1, -1, 1, -1, 1, -1])          # centre neutral, ring alternating
A = mask(s)
x0 = p.reshape(-1)

print("=== PAIR DISTANCES AND MASK ===")
print(f"  {'pair':>8} {'dist':>8} {'mask':>6} {'in support?':>12} {'bonded':>7}")
for i in range(7):
    for j in range(i + 1, 7):
        d = np.linalg.norm(p[i] - p[j])
        ins = d < np.sqrt(1.5)
        print(f"  {f'{i}-{j}':>8} {d:>8.5f} {A[i,j]:>6.2f} {str(ins):>12} "
              f"{str(bool(A[i,j] > 0 and ins)):>7}")

print(f"\n  B = {n_bonds(x0, A)}   E0 = {energy(x0, A):.10f}"
      f"   |grad| = {np.linalg.norm(grad(x0, A)):.3e}")
print(f"  (every bond sits at r = 1, so this is an UNSTRESSED equilibrium)")

H = hessian(x0, A)
N0, ev = null_beyond_trivial(H, x0, 1e-7)
T = trivial_modes(x0)
print(f"\n=== HESSIAN ===")
print(f"  eigenvalues: {np.round(np.sort(ev), 6)}")
print(f"  trivial = {T.shape[1]}   nontrivial null dim = {N0.shape[1]}")
print(f"  Maxwell 3N - B = {3*7 - n_bonds(x0, A)}")

print(f"\n=== RELAXED PROFILE ALONG EACH NULL DIRECTION ===")
print(f"  (the FTD-0787 guard: relax all other coords at every amplitude)")
verdicts = []
for k in range(N0.shape[1]):
    prof = relaxed_profile(x0, A, N0[:, k])
    keep = [q for q in prof if q["valid"]]
    t = np.array([q["t"] for q in keep]); dE = np.array([q["dE"] for q in keep])
    if len(keep) >= 2 and np.max(np.abs(dE)) > 1e-12:
        m = np.abs(dE) > 1e-14
        ex = np.polyfit(np.log(t[m]), np.log(np.abs(dE[m])), 1)[0] if m.sum() >= 2 else np.nan
        sign = "POSITIVE" if dE[-1] > 0 else "NEGATIVE"
        verdicts.append(("quartic" if 3.5 < ex < 4.5 else f"exp={ex:.2f}", sign))
        print(f"  dir {k}: exponent {ex:>6.3f}  {sign}  "
              f"dE = {[f'{v:.2e}' for v in dE]}")
    else:
        verdicts.append(("FLAT", "zero"))
        print(f"  dir {k}: EXACTLY FLAT  dE = {[f'{v:.2e}' for v in dE]}")

flat = sum(1 for v, _ in verdicts if v == "FLAT")
neg = sum(1 for _, s2 in verdicts if s2 == "NEGATIVE")
print(f"\n=== VERDICT ===")
print(f"  {flat}/{len(verdicts)} exactly flat, {neg}/{len(verdicts)} negative")
if flat == 0 and neg == 0 and all("quartic" in v for v, _ in verdicts):
    print("  n = 4  -- first-order flexible, second-order rigid, quartic")
    print("  POSITIVE DEFINITE on the whole null space. C3 REALIZED.")
elif neg > 0:
    print("  INDEFINITE -- some null directions LOWER the energy, so x0 is a")
    print("  saddle, not a minimum. Not n = 4; the configuration is unstable.")
else:
    print("  n = infinity in at least one direction -> quartic form is only")
    print("  positive SEMI-definite. Fails the criterion.")

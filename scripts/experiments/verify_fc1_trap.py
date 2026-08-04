"""Verify the load-bearing numbers of FTD-0796 (the FC-1 Bell trap).

RECONSTRUCTED 2026-08-04. The LEDGER row for FTD-0796 cited this filename as
provenance, but no such file existed in the repository. This script is an
independent reconstruction written from the row's claims, not a recovery of
the original. Every number the row asserts is recomputed here from scratch.

The claims under test:

  C1  If the four CHSH observables are elements of A_5 (FC-1's completeness
      clause) under one setting-independent measure, their pushforward IS a
      joint distribution on {-1,1}^4, so Fine (1982) forces |CHSH| <= 2.
      Claimed: LP over the 16-atom simplex returns max CHSH = 2.000000000000,
      over all eight CHSH forms likewise. NO locality assumption is used.

  C2  The quantum correlator table at the optimal angles is LP-INFEASIBLE
      (no joint distribution reproduces it).

  C3  The superdeterminism price is S <= min(2 + 3M, 4), with M the L1
      setting-spread. Claimed: M = 0 gives 2; M* = (2*sqrt(2) - 2)/3 =
      0.27614237 gives EXACTLY Tsirelson; M = 2/3 gives the PR box (4).

  C4  FTD's own registered correlator E(theta) = -(1 - 2|theta|/pi) is a
      triangle, not a cosine. Claimed: its global CHSH max is exactly
      2.0000000000 (a maximally correlated LOCAL model, 29.29% below
      Tsirelson), with a 0.2071 gap against -cos at 45 and 135 degrees and
      forced agreement at 0, 90, 180.

Run:  python scripts/experiments/verify_fc1_trap.py
"""
import itertools
import numpy as np
from scipy.optimize import linprog

TOL = 1e-9
results = []


def check(name, ok, detail):
    results.append((ok, name, detail))
    print(f"  [{'PASS' if ok else 'FAIL'}] {name}: {detail}")


# ---------------------------------------------------------------- C1 -------
# The 16 deterministic assignments of (A, A', B, B') in {-1,1}^4. Any joint
# distribution is a convex combination of these, so the LP maximum over the
# simplex equals the maximum over the atoms.
ATOMS = np.array(list(itertools.product([-1, 1], repeat=4)), dtype=float)  # A A' B B'

# The eight CHSH forms: three "+" terms and one "-" term, the minus in each
# of four positions, times an overall sign.
FORMS = []
for minus_pos in range(4):
    for overall in (+1, -1):
        signs = np.array([1.0, 1.0, 1.0, 1.0])
        signs[minus_pos] = -1.0
        FORMS.append(overall * signs)


def chsh_atom_values(signs):
    """S = s0*A.B + s1*A.B' + s2*A'.B + s3*A'.B' evaluated on each atom."""
    A, Ap, B, Bp = ATOMS[:, 0], ATOMS[:, 1], ATOMS[:, 2], ATOMS[:, 3]
    return signs[0] * A * B + signs[1] * A * Bp + signs[2] * Ap * B + signs[3] * Ap * Bp


maxima = []
for signs in FORMS:
    vals = chsh_atom_values(signs)
    # maximize c.p  s.t.  sum p = 1, p >= 0   (linprog minimizes)
    res = linprog(c=-vals, A_eq=np.ones((1, 16)), b_eq=[1.0],
                  bounds=[(0, None)] * 16, method="highs")
    assert res.success, "LP failed"
    maxima.append(-res.fun)

maxima = np.array(maxima)
check("C1 LP max CHSH over the 16-atom simplex",
      abs(maxima[0] - 2.0) < TOL,
      f"{maxima[0]:.12f} (expected exactly 2)")
check("C1 all eight CHSH forms",
      np.all(np.abs(maxima - 2.0) < TOL),
      f"max over forms = {maxima.max():.12f}, min = {maxima.min():.12f}")

# ---------------------------------------------------------------- C2 -------
# Quantum singlet correlators E(a,b) = -cos(a-b) at the CHSH-optimal angles.
a, ap, b, bp = 0.0, np.pi / 2, np.pi / 4, -np.pi / 4
E_target = {
    "ab": -np.cos(a - b), "abp": -np.cos(a - bp),
    "apb": -np.cos(ap - b), "apbp": -np.cos(ap - bp),
}
S_quantum = E_target["ab"] + E_target["abp"] + E_target["apb"] - E_target["apbp"]
check("C2 quantum S at optimal angles is Tsirelson",
      abs(abs(S_quantum) - 2 * np.sqrt(2)) < 1e-12,
      f"|S| = {abs(S_quantum):.12f} vs 2*sqrt(2) = {2*np.sqrt(2):.12f}")

# Feasibility: is there p >= 0, sum p = 1, reproducing all four correlators?
A, Ap, B, Bp = ATOMS[:, 0], ATOMS[:, 1], ATOMS[:, 2], ATOMS[:, 3]
A_eq = np.vstack([np.ones(16), A * B, A * Bp, Ap * B, Ap * Bp])
b_eq = np.array([1.0, E_target["ab"], E_target["abp"],
                 E_target["apb"], E_target["apbp"]])
feas = linprog(c=np.zeros(16), A_eq=A_eq, b_eq=b_eq,
               bounds=[(0, None)] * 16, method="highs")
check("C2 quantum correlator table is LP-infeasible",
      not feas.success,
      f"linprog status={feas.status} ({feas.message.strip()[:48]})")

# ---------------------------------------------------------------- C3 -------
M_star = (2 * np.sqrt(2) - 2) / 3
check("C3 M* = (2*sqrt(2)-2)/3",
      abs(M_star - 0.27614237) < 5e-9,
      f"{M_star:.10f} (row claims 0.27614237)")
check("C3 the bound at M* is exactly Tsirelson",
      abs(min(2 + 3 * M_star, 4) - 2 * np.sqrt(2)) < 1e-12,
      f"min(2+3M*, 4) = {min(2+3*M_star, 4):.12f}")
check("C3 M=0 gives the local bound, M=2/3 gives the PR box",
      abs(min(2 + 0, 4) - 2) < TOL and abs(min(2 + 3 * (2 / 3), 4) - 4) < TOL,
      f"S(0) = {min(2.0,4):.1f}, S(2/3) = {min(2+3*(2/3),4):.1f}")

# ---------------------------------------------------------------- C4 -------
def E_ftd(theta):
    """FTD's registered correlator: a triangle wave, not a cosine.
    theta is wrapped to [0, pi] (the angular separation)."""
    d = np.abs(np.mod(np.asarray(theta) + np.pi, 2 * np.pi) - np.pi)
    return -(1.0 - 2.0 * d / np.pi)


def chsh_ftd(angles):
    a, ap, b, bp = angles
    return E_ftd(a - b) + E_ftd(a - bp) + E_ftd(ap - b) - E_ftd(ap - bp)


# Only angle differences matter, so fix a = 0 and scan (a', b, b') on a dense
# grid. The triangle is piecewise linear, so the optimum sits at a breakpoint
# and a grid aligned to pi/N hits it exactly. Vectorised over the full cube.
grid = np.linspace(-np.pi, np.pi, 361)
Ap_g, B_g, Bp_g = np.meshgrid(grid, grid, grid, indexing="ij")
S_g = (E_ftd(0.0 - B_g) + E_ftd(0.0 - Bp_g)
       + E_ftd(Ap_g - B_g) - E_ftd(Ap_g - Bp_g))
flat = int(np.argmax(S_g))
best = float(S_g.ravel()[flat])
i, j, k = np.unravel_index(flat, S_g.shape)
best_at = (0.0, float(grid[i]), float(grid[j]), float(grid[k]))

check("C4 FTD triangle correlator global CHSH max is 2",
      abs(best - 2.0) < 1e-9,
      f"max = {best:.10f} at angles {np.round(np.degrees(best_at), 2)} deg")
check("C4 that is 29.29% below Tsirelson",
      abs((2 * np.sqrt(2) - best) / (2 * np.sqrt(2)) - 0.292893) < 1e-5,
      f"deficit = {(2*np.sqrt(2)-best)/(2*np.sqrt(2))*100:.4f}%")

gap45 = abs(E_ftd(np.pi / 4) - (-np.cos(np.pi / 4)))
gap135 = abs(E_ftd(3 * np.pi / 4) - (-np.cos(3 * np.pi / 4)))
check("C4 gap against -cos at 45 and 135 degrees",
      abs(gap45 - 0.2071) < 5e-5 and abs(gap135 - 0.2071) < 5e-5,
      f"{gap45:.6f} at 45 deg, {gap135:.6f} at 135 deg (row claims 0.2071)")

agree = [abs(E_ftd(t) - (-np.cos(t))) for t in (0.0, np.pi / 2, np.pi)]
check("C4 forced agreement at 0, 90, 180 degrees",
      max(agree) < 1e-12,
      f"max deviation = {max(agree):.2e}")

# --------------------------------------------------------------- summary ---
npass = sum(1 for ok, _, _ in results if ok)
print(f"\n{npass}/{len(results)} checks pass")
print("\nConclusion: FC-1 completeness alone forces CHSH <= 2. No locality")
print("assumption is used anywhere above, so nonlocality cannot lift the")
print("bound -- using it would break FC-1 itself.")
raise SystemExit(0 if npass == len(results) else 1)

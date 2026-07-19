"""Adjudication inputs for PREREG_LIGHT_DEFLECTION_CHANNEL_v1.

Reads the campaign_light_deflection CSV, computes the frozen quantities
(prereg §3 gates, §4 theta_gamma0 yardstick, §5 outcome-map inputs) and
prints them. It does NOT print a verdict word; the verdict is applied by
the analyst against prereg §5.

theta_gamma0 (frozen §4): null-geodesic deflection of the g00-only optical
metric. Engine map of record (phase_forces.cpp): local bandwidth
c_eff = C*sqrt(1 - L^2)  =>  index n = 1/sqrt(1 - L^2), ln n ~ L^2/2 for
small L. theta_gamma0 = integral |d/dy (ln n)| dx along the straight ray at
impact parameter b. Evaluation: fit the emitted single-ray latency profile
L(x) at offset b to the point-mass Poisson form A/sqrt((x-xc)^2 + b^2),
then differentiate the fit analytically in y.

Usage: python analyze_light_deflection_v1.py <csv>
"""
import math
import sys
from collections import defaultdict

CSV = sys.argv[1] if len(sys.argv) > 1 else "deflection_v1.csv"
XC = 48.0

rows = defaultdict(list)       # arm -> [(t, xc, yc, zc, E)]
summaries = {}                 # arm -> dict
gates = defaultdict(dict)      # arm -> {gate: value}
latency = defaultdict(list)    # arm -> [(x, L)]
prow = defaultdict(list)       # arm -> [(t,x,y,z,vx,vy,vz)]

for line in open(CSV, encoding="utf-8", errors="replace"):
    p = line.strip().split(",")
    if p[0] == "ROW":
        rows[p[1]].append((int(p[2]), float(p[3]), float(p[4]), float(p[5]), float(p[6])))
    elif p[0] == "SUMMARY" and p[1].startswith(("W", "C", "D")):
        d = {p[i]: float(p[i + 1]) for i in range(2, len(p) - 1, 2)}
        summaries[p[1]] = d
    elif p[0] == "SUMMARY":  # particle arm
        d = {p[i]: float(p[i + 1]) for i in range(2, len(p) - 1, 2)}
        summaries[p[1]] = d
    elif p[0] == "GATE":
        gates[p[1]][p[2]] = float(p[3])
    elif p[0] == "LATENCY":
        latency[p[1]].append((int(p[2]), float(p[3])))
    elif p[0] == "PROW":
        prow[p[1]].append(tuple(float(x) for x in p[2:]))

print("=== GATES ===")
for arm, g in gates.items():
    for k, v in g.items():
        print(f"  {arm:7s} {k:18s} {v:.6e}")

print("\n=== PACKET ARMS (angles theta = exit_vy - entry_vy over vx) ===")
def theta(arm, comp="vy"):
    d = summaries.get(arm)
    if not d:
        return None
    vx = d.get("exit_vx", 0.0)
    if vx == 0:
        return None
    return (d[f"exit_{comp}"] - d[f"entry_{comp}"]) / vx

for arm in ("C0", "W-b10", "W-b14", "D-b10"):
    ty, tz = theta(arm, "vy"), theta(arm, "vz")
    d = summaries.get(arm, {})
    print(f"  {arm:7s} theta_y = {ty:+.6e}  theta_z(null) = {tz:+.6e}  "
          f"dy = {d.get('dy', float('nan')):+.6e}  vx_exit = {d.get('exit_vx', float('nan')):.4f}")

print("\n=== S-ARM (v3 contamination) + FLOOR ===")
t_c0 = abs(theta("C0", "vy") or 0.0)
tz_w = max(abs(theta(a, "vz") or 0.0) for a in ("W-b10", "W-b14"))
t_s = abs(theta("S-b10", "vy") or 0.0) if "S-b10" in summaries else 0.0
floor = max(3 * t_c0, 3 * tz_w, 2 * t_s)
print(f"  theta_S (mass-only) = {theta('S-b10', 'vy') if 'S-b10' in summaries else float('nan'):+.6e}")
print(f"  3x|theta_C0|   = {3 * t_c0:.6e}")
print(f"  3x|theta_z(W)| = {3 * tz_w:.6e}")
print(f"  2x|theta_S|    = {2 * t_s:.6e}")
print(f"  FLOOR (angle)  = {floor:.6e}")

print("\n=== THETA_DIFF (v2/v3 primary observable) ===")
t_c0_signed = theta("C0", "vy") or 0.0
for a in ("W-b10", "W-b14", "D-b10"):
    tw = theta(a, "vy")
    if tw is not None:
        print(f"  theta_diff({a}) = {tw - t_c0_signed:+.6e}")

print("\n=== PARTICLE ARM (V2 gate) ===")
parm = next((a for a in summaries if a.startswith("P-")), None)
p = summaries.get(parm, {}) if parm else {}
print(f"  arm = {parm}  theta_p = {p.get('theta_p', float('nan')):+.6e}  "
      f"vy {p.get('vy_in', float('nan')):+.4e} -> {p.get('vy_out', float('nan')):+.4e}  "
      f"vx_out = {p.get('vx_out', float('nan')):+.4f}  "
      f"survived = {gates.get(parm, {}).get('particle_survived', 0):.0f}")
if floor > 0 and "theta_p" in p:
    print(f"  |theta_p| / FLOOR = {abs(p['theta_p']) / floor:.2f}  (V2 needs > 10 AND vx_out > 0.25)")

print("\n=== theta_gamma0 (frozen 4; fit L(x) ~ A/sqrt((x-xc)^2+b^2)) ===")
for arm, b in (("W-b10", 10.0), ("W-b14", 14.0)):
    prof = latency.get(arm)
    if not prof:
        continue
    # least-squares A over the ray profile (excluding the mass-adjacent core)
    num = den = 0.0
    for x, l in prof:
        r = math.sqrt((x - XC) ** 2 + b * b)
        if r < 6:
            continue
        f = 1.0 / r
        num += f * l
        den += f * f
    A = num / den if den else 0.0
    # theta_g0 = integral | d/dy (L^2/2) | dx at offset b, L = A/sqrt(u^2+b^2)
    #   d/dy (L^2/2) = -A^2 * b / (u^2 + b^2)^2   => integral = A^2*b * int du/(u^2+b^2)^2
    #   int_{-inf}^{inf} du/(u^2+b^2)^2 = pi/(2 b^3)
    tg0 = A * A * b * (math.pi / (2 * b ** 3))
    lmax = max(l for _, l in prof)
    tw = theta(arm, "vy") or 0.0
    print(f"  {arm}: A_fit = {A:.4e}  L_max(ray) = {lmax:.4e}  theta_gamma0 = {tg0:.6e}")
    print(f"         theta_w = {tw:+.6e}   theta_w/theta_gamma0 = "
          f"{tw / tg0 if tg0 else float('nan'):+.3f}   FLOOR/theta_gamma0 = "
          f"{floor / tg0 if tg0 else float('nan'):.3f}")

print("\n=== V3 packet integrity (window energy, entry vs exit fit midpoints) ===")
for arm in ("C0", "W-b10", "W-b14", "D-b10"):
    r = rows.get(arm)
    if not r:
        continue
    def e_at(t0):
        cands = [e for (t, _, _, _, e) in r if abs(t - t0) <= 2]
        return sum(cands) / len(cands) if cands else float("nan")
    e_in, e_out = e_at(20), e_at(92)
    print(f"  {arm:7s} E(t=20) = {e_in:.4e}  E(t=92) = {e_out:.4e}  "
          f"retention = {e_out / e_in if e_in else float('nan'):.3f}  (V3 needs >= 0.5)")

print("\n(verdict: apply prereg 5 to the numbers above)")

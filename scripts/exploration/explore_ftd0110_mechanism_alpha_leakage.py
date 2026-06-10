"""FTD-0110 Mechanism alpha (multi-block irrep leakage) -- quantitative test.

LEDGER: FTD-0259. Companion doc:
  docs/theory/03_derivations/foundational_mechanics/EXPLR_FTD0110_MECHANISM_ALPHA_LEAKAGE_CLOSED.md

Pre-stated predictions (in-session pre-registration, 2026-06-09, stated BEFORE compute):
  P1: per-block non-A1g leakage of the Green profile -> lambda(r) ~ (18/27)/r^2 = (2/3)/r^2
      (gradient/T1u term; 18/27 is pure 27-block geometry).
  P2: parameter-free shell-dephasing model M1, k(A) = 1/4 * exp(-Sum_{r=2}^{R(A)} lambda(r)),
      R(A) = (3 A^2/16 pi)^(1/3); success criterion >= 9/11 points within sigma = 0.018.
  P3: log vs small-power-law forms are empirically indistinguishable on the 11 points.

Outcomes (computed below): P1 VERIFIED (continuum); P2 FALSIFIED (3/11, RMS 0.043);
P3 CONFIRMED (RMS 0.0146 vs 0.0147; forms also agree at A=2000 -> operationally undecidable).
Bonus: the friction-formation form (gamma_L * sqrt(3) * R(A), parameter-free) also misses
(over-drifts mid-range); the drift ONSET matches the Langevin thermal crossover
A* = sqrt(M * T_L) ~ 13 (engine constants gamma_L=0.02, T_L=0.005, L=32 -> M*T_L = 163.84).

Quick-check platform only (Python); the canonical discriminator is the pre-registerable
engine re-run of the amplitude sweep with the thermostat OFF (see companion doc section 5).
"""
import numpy as np
from collections import defaultdict

# ---------------------------------------------------------------- lattice Green
L = 96
st = np.zeros((L, L, L))
for dx in (-1, 0, 1):
    for dy in (-1, 0, 1):
        for dz in (-1, 0, 1):
            n = abs(dx) + abs(dy) + abs(dz)
            if n == 1:
                st[dx % L, dy % L, dz % L] += 2.0   # 6 faces, weight 2
            elif n == 2:
                st[dx % L, dy % L, dz % L] += 1.0   # 12 edges, weight 1
st[0, 0, 0] = -24.0
F = np.fft.fftn(st)
src = np.zeros((L, L, L)); src[0, 0, 0] = 1.0
S = np.fft.fftn(src)
with np.errstate(divide="ignore", invalid="ignore"):
    Ghat = np.where(np.abs(F) > 1e-12, -S / F, 0.0)
G = np.real(np.fft.ifftn(Ghat))
co = np.indices((L, L, L))
d = np.minimum(co, L - co)
r_all = np.sqrt((d ** 2).sum(axis=0))
G -= G[r_all > L / 2 - 2].mean()    # far-field zero reference

# ------------------------------------------------- per-block A1g leakage lambda
def lam_profile(prof27, orb27):
    p2 = float((prof27 ** 2).sum())
    pa = prof27.copy()
    for o in range(4):
        m = orb27 == o
        pa[m] = prof27[m].mean()
    return 1.0 - float((pa ** 2).sum()) / p2

DELTAS = [(dx, dy, dz) for dx in (-1, 0, 1) for dy in (-1, 0, 1) for dz in (-1, 0, 1)]
ORB = np.array([abs(dx) + abs(dy) + abs(dz) for dx, dy, dz in DELTAS])

def lam_lattice(c):
    prof = np.array([G[(c[0] + dx) % L, (c[1] + dy) % L, (c[2] + dz) % L]
                     for dx, dy, dz in DELTAS])
    return lam_profile(prof, ORB)

def lam_continuum(rvec):
    prof = np.array([1.0 / np.linalg.norm(np.array(rvec) + np.array(dl)) for dl in DELTAS])
    return lam_profile(prof, ORB)

rmax = 15
bins = defaultdict(list)
for x in range(-rmax, rmax + 1):
    for y in range(-rmax, rmax + 1):
        for z in range(-rmax, rmax + 1):
            if max(abs(x), abs(y), abs(z)) <= 1:   # blocks containing the source excluded
                continue
            rr = np.sqrt(x * x + y * y + z * z)
            if rr <= rmax:
                bins[int(round(rr))].append(lam_lattice((x % L, y % L, z % L)))
lam_shell = {rr: float(np.mean(v)) for rr, v in sorted(bins.items())}

print("P1 -- continuum lemma check (lambda * r^2 -> 2/3 = 0.6667):")
for rr in (4, 8, 12, 16, 24):
    print(f"   r={rr:3d}: {lam_continuum((rr, 0, 0)) * rr * rr:.4f}")
print("P1 -- lattice (L=96 periodic; finite-L image contamination grows with r):")
for rr in sorted(lam_shell):
    print(f"   r={rr:3d}: lambda={lam_shell[rr]:.6f}  lambda*r^2={lam_shell[rr]*rr*rr:.4f}  (n={len(bins[rr])})")

# ------------------------------------------------------------- empirical k(A)
A_eng = np.array([2.00, 10.00, 15.00, 20.00, 28.77, 30.00, 33.05, 50.00, 62.42, 85.70, 117.93])
k_eng = np.array([0.250, 0.252, 0.224, 0.234, 0.253, 0.262, 0.245, 0.222, 0.224, 0.212, 0.206])
SIGMA = 0.018  # FTD-0110: k = 0.239 +/- 0.018

def R_of_A(A):
    return (3.0 * (A * A / 4.0) / (4.0 * np.pi)) ** (1.0 / 3.0)

# M1: per-shell-crossing energy dephasing (parameter-free)
def k_M1(A):
    R = R_of_A(A)
    s = sum(lam for rr, lam in lam_shell.items() if rr <= R)
    return 0.25 * np.exp(-s)

# M-friction: dissipation over ballistic formation time tau = sqrt(3)*R (parameter-free)
GAMMA_L = 0.02
def k_fric(A):
    return 0.25 * np.exp(-GAMMA_L * np.sqrt(3.0) * R_of_A(A))

print("\nModel battery vs engine data (sigma = 0.018):")
print("   A    | k_eng | M1 leak | M-fric |")
h1 = hf = 0
for A, k in zip(A_eng, k_eng):
    m1, mf = k_M1(A), k_fric(A)
    h1 += abs(m1 - k) <= SIGMA
    hf += abs(mf - k) <= SIGMA
    print(f"{A:7.2f} | {k:.3f} | {m1:.4f}  | {mf:.4f} |")
rms1 = float(np.sqrt(np.mean((np.array([k_M1(a) for a in A_eng]) - k_eng) ** 2)))
rmsf = float(np.sqrt(np.mean((np.array([k_fric(a) for a in A_eng]) - k_eng) ** 2)))
print(f"M1 (leakage):  {h1}/11 within sigma, RMS={rms1:.4f}  -> P2 pre-stated bar >=9/11: "
      f"{'PASS' if h1 >= 9 else 'FALSIFIED'}")
print(f"M-friction  :  {hf}/11 within sigma, RMS={rmsf:.4f}")

# P3: one-parameter log vs power forms
X = np.log(A_eng / 2.0)
c_log = float(np.linalg.lstsq(X[:, None], (1 - k_eng / 0.25)[:, None], rcond=None)[0][0][0])
eps = float(np.linalg.lstsq(X[:, None], (-np.log(k_eng / 0.25))[:, None], rcond=None)[0][0][0])
res_log = float(np.sqrt(np.mean((0.25 * (1 - c_log * X) - k_eng) ** 2)))
res_pow = float(np.sqrt(np.mean((0.25 * (A_eng / 2.0) ** (-eps) - k_eng) ** 2)))
print(f"\nP3 -- log coeff {c_log:.4f} (doc fit: 0.030), RMS={res_log:.4f}; "
      f"power eps={eps:.4f}, RMS={res_pow:.4f}")
for Abig in (2000.0,):
    print(f"   forms at A={Abig:.0f}: log -> {0.25*(1-c_log*np.log(Abig/2)):.4f}, "
          f"power -> {0.25*(Abig/2)**(-eps):.4f}   (indistinguishable -> undecidable)")

# Langevin thermal crossover (Mechanism gamma knee; engine constants, no tuning)
T_L, Lc = 0.005, 32
A_star = float(np.sqrt(Lc ** 3 * T_L))
print(f"\nMechanism-gamma knee: A* = sqrt(L^3 * T_L) = {A_star:.1f} "
      f"(observed drift onset between A=10 and A=15)")

"""Base-rate control for OT-3.3 / the extended polynomial look-elsewhere scan.

OT-3.3 (TRACKER_ONTIC_TRUTH.md) reports "0 dual-matchers across 2,871,576
polynomials" and is cited as numerical-uniqueness support for x_+ = 1/alpha.
Its sibling, the FTD-0319 adversarial scan, was audited by FTD-0791 and found
to sit exactly at its own chance base rate. **The equivalent control has never
been run against OT-3.3.** The tracker flags this itself:

    "a null expectation near zero and a null expectation near one look
     identical in the raw count."

This script is that control. It is the direct analogue of
`scripts/experiments/verify_look_elsewhere_baserate.py` (the FTD-0791 runner),
pointed at the EXT-A family of
`scripts/proofs/proof_polynomial_look_elsewhere_extended.py`.

MEASURED OBSERVABLE (locked in PREREG_OT33_BASERATE_v1.md):
    N_null := the expected number of DISTINCT non-master dual-matchers in the
    EXT-A family at the registered gate, when the target pair is displaced to
    locations carrying no FTD significance.

It reports, in order:
  1. Aliasing: nominal family size vs DISTINCT polynomials (n/d_n collapses).
  2. Observed matchers at the registered gate.
  3. Analytic null: local x_+ root density x gate width.
  4. Monte Carlo null: displaced targets, distribution of matcher counts.
  5. Eliminative power of the x_- leg (the retired N_c identification).

This is a verification instrument. It introduces no theorem, promotes nothing,
and moves no tag on its own.
"""

from __future__ import annotations

import hashlib
import sys
from pathlib import Path

import numpy as np

REPO = Path(__file__).resolve().parents[2]
TARGET_RUNNER = REPO / "scripts" / "proofs" / "proof_polynomial_look_elsewhere_extended.py"

# ---- registered constants, copied verbatim from the target runner ----------
G_STAR = 2.958675119188639
X_PLUS_TARGET = 137.0361714582        # the master quadratic's own tree root
X_MINUS_TARGET = 3.0239639163         # the retired x_- <-> N_c identification
X_PLUS_REL_TOL = 1.26e-6
X_MINUS_REL_TOL = 0.0080
CODATA_ALPHA_INV = 137.035999177      # for the docstring-vs-code check only

N_MAX = M_MAX = 64
D_MAX = 4
P_MAX = Q_MAX = 5

MC_DRAWS = 20000
MC_SEED = 20260804                    # locked in the pre-registration


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def build_family() -> tuple[np.ndarray, np.ndarray, int, int]:
    """Rebuild EXT-A exactly: P(x) = x^2 - (n/d_n) G*^p x + (m/d_m) G*^q."""
    n = np.arange(1, N_MAX + 1)
    d = np.arange(1, D_MAX + 1)
    p = np.arange(0, P_MAX + 1)
    gp = G_STAR ** p

    # b takes values (n/d_n)*G*^p ; c takes values (m/d_m)*G*^q — same grid.
    coef = (n[:, None] / d[None, :]).ravel()          # 64*4
    vals = (coef[:, None] * gp[None, :]).ravel()      # 64*4*6
    nominal = vals.size * vals.size

    B, C = np.meshgrid(vals, vals, indexing="ij")
    B, C = B.ravel(), C.ravel()

    disc = B * B - 4 * C
    ok = disc > 0
    r = np.sqrt(disc[ok])
    x_plus = (B[ok] + r) / 2
    x_minus = (B[ok] - r) / 2

    # DISTINCT polynomials: (n/d_n) aliases heavily (16/1 == 32/2 == 48/3 == 64/4).
    distinct = np.unique(np.round(np.stack([B, C]), 12), axis=1).shape[1]
    return x_plus, x_minus, nominal, distinct


def count_matchers(x_plus, x_minus, t_plus, t_minus) -> np.ndarray:
    """Boolean mask of dual-matchers against an arbitrary target pair."""
    return (np.abs(x_plus - t_plus) < X_PLUS_REL_TOL * t_plus) & (
        np.abs(x_minus - t_minus) < X_MINUS_REL_TOL * t_minus
    )


def main() -> int:
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")

    print("=" * 72)
    print("OT-3.3 BASE-RATE CONTROL")
    print("=" * 72)
    print(f"target runner : {TARGET_RUNNER.relative_to(REPO)}")
    print(f"  sha256      : {sha256(TARGET_RUNNER)}")
    print(f"this control  : {Path(__file__).name}")
    print(f"  sha256      : {sha256(Path(__file__))}")
    print()

    print("--- 0. registered target vs the experimental value ---")
    dev = abs(X_PLUS_TARGET - CODATA_ALPHA_INV) / CODATA_ALPHA_INV
    print(f"  X_PLUS_TARGET (code)  = {X_PLUS_TARGET}")
    print(f"  CODATA 1/alpha        = {CODATA_ALPHA_INV}")
    print(f"  separation            = {dev*1e6:.3f} ppm   gate = {X_PLUS_REL_TOL*1e6:.2f} ppm")
    print(f"  => the gate is {'CENTRED ON THE CLAIM, not the measurement' if dev > X_PLUS_REL_TOL else 'centred within tolerance of CODATA'}")
    print()

    x_plus, x_minus, nominal, distinct = build_family()
    print("--- 1. aliasing in the 'extended' family ---")
    print(f"  nominal EXT-A size          : {nominal:,}")
    print(f"  DISTINCT (b,c) polynomials  : {distinct:,}")
    print(f"  aliasing factor             : {nominal/distinct:.2f}x")
    print(f"  real-rooted pairs           : {x_plus.size:,}")
    print()

    print("--- 2. observed at the registered gate ---")
    obs = count_matchers(x_plus, x_minus, X_PLUS_TARGET, X_MINUS_TARGET)
    n_obs = int(obs.sum())
    uniq_obs = np.unique(np.round(np.stack([x_plus[obs], x_minus[obs]]), 9), axis=1).shape[1]
    print(f"  dual-matchers (with aliases): {n_obs}")
    print(f"  DISTINCT root pairs         : {uniq_obs}")
    print(f"  non-master distinct matchers: {max(0, uniq_obs - 1)}")
    print()

    print("--- 3. analytic null: local root density x gate width ---")
    win_lo, win_hi = X_PLUS_TARGET - 1.0, X_PLUS_TARGET + 1.0
    in_win = (x_plus > win_lo) & (x_plus < win_hi)
    dens = in_win.sum() / (win_hi - win_lo)
    gate_w = 2 * X_PLUS_REL_TOL * X_PLUS_TARGET
    print(f"  x_+ roots in [{win_lo:.1f}, {win_hi:.1f}] : {int(in_win.sum()):,}")
    print(f"  density                       : {dens:,.1f} per unit x")
    print(f"  gate width                    : {gate_w:.3e}")
    print(f"  EXPECTED x_+ hits at gate     : {dens*gate_w:.3f}")
    print()

    print("--- 4. Monte Carlo null: displaced targets ---")
    rng = np.random.default_rng(MC_SEED)
    t_plus = rng.uniform(110.0, 170.0, MC_DRAWS)
    # displace x_- across the same relative span the x_+ target is displaced over
    t_minus = rng.uniform(2.0, 4.5, MC_DRAWS)
    xs = np.sort(x_plus)
    hits_plus = np.searchsorted(xs, t_plus * (1 + X_PLUS_REL_TOL)) - np.searchsorted(
        xs, t_plus * (1 - X_PLUS_REL_TOL)
    )
    print(f"  x_+ leg alone : mean hits/target = {hits_plus.mean():.4f}"
          f"   P(>=1) = {np.mean(hits_plus >= 1):.4f}")

    dual = np.empty(MC_DRAWS, dtype=int)
    for i in range(MC_DRAWS):
        dual[i] = int(count_matchers(x_plus, x_minus, t_plus[i], t_minus[i]).sum())
    print(f"  BOTH legs     : mean hits/target = {dual.mean():.4f}"
          f"   P(>=1) = {np.mean(dual >= 1):.4f}")
    print(f"  N_null (the locked observable) = {dual.mean():.4f}")
    print()

    print("--- 5. eliminative power of the retired x_- leg ---")
    only_plus = np.abs(x_plus - X_PLUS_TARGET) < X_PLUS_REL_TOL * X_PLUS_TARGET
    both = only_plus & (np.abs(x_minus - X_MINUS_TARGET) < X_MINUS_REL_TOL * X_MINUS_TARGET)
    print(f"  x_+ gate alone              : {int(only_plus.sum())}")
    print(f"  x_+ AND x_- gate            : {int(both.sum())}")
    print(f"  removed by the x_- leg      : {int(only_plus.sum() - both.sum())}")
    print()

    print("=" * 72)
    print(f"OBSERVABLE  N_null = {dual.mean():.4f}")
    print("Outcome per PREREG_OT33_BASERATE_v1.md:")
    print("  A (OT-3.3 SURVIVES)      : N_null >= 1.0")
    print("  B (OT-3.3 UNINFORMATIVE) : N_null <  0.1")
    print("  C (INTERMEDIATE)         : 0.1 <= N_null < 1.0")
    verdict = "A" if dual.mean() >= 1.0 else ("B" if dual.mean() < 0.1 else "C")
    print(f"  => OUTCOME {verdict}")
    print("=" * 72)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

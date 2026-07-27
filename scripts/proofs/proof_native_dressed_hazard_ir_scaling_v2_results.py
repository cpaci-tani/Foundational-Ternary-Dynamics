"""Verify FTD-0436 dressed-hazard IR-scaling v2 results against the locked
preregistration (PREREG_NATIVE_DRESSED_HAZARD_IR_SCALING_v2.md).

Reads engine/results/ftd_0436/manifest.json + hash-locked CSVs, enforces
gates G1-G9, computes the locked phase-corrected estimator and the locked
two-model BIC contest, and prints the preregistered outcome.
"""

from __future__ import annotations

import csv
import json
import math
from collections import defaultdict
from hashlib import sha256
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / "engine/results/ftd_0436"
MANIFEST = RESULTS / "manifest.json"

VOLUMES = [48, 64, 96, 128, 192]
SEEDS = list(range(8))
BRACKET = 2
C_WAVE = 0.57735026918962576451
V1_H48 = 0.00351325
V1_SE48 = 0.00001889
P_GRID = [0.20 + 0.02 * i for i in range(141)]  # 0.20 .. 3.00

checks: list[tuple[str, bool]] = []


def check(name: str, ok: bool) -> None:
    checks.append((name, bool(ok)))


def pole(L: int) -> tuple[float, float, int]:
    k = 2.0 * math.pi / L
    M = 4.0 * math.sin(k / 2.0) ** 2
    omega = math.acos(max(-1.0, min(1.0, 1.0 - 0.5 * C_WAVE * C_WAVE * M)))
    t_star = round(math.pi / omega) - 1
    return omega, math.pi / omega - 1.0, t_star


def load_rows(path: Path) -> list[dict]:
    with path.open(newline="", encoding="utf-8") as fh:
        return [dict(r) for r in csv.DictReader(fh)]


def jackknife_se(values: list[float]) -> float:
    n = len(values)
    if n < 2:
        return 0.0
    means = [(sum(values) - values[i]) / (n - 1) for i in range(n)]
    mbar = sum(means) / n
    return math.sqrt((n - 1) / n * sum((m - mbar) ** 2 for m in means))


def main() -> None:
    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    check("MANIFEST identifier", manifest.get("identifier") == "FTD-0436")
    files = manifest["files"]
    for name, digest in files.items():
        p = RESULTS / name
        check(f"HASH {name}",
              p.exists()
              and sha256(p.read_bytes()).hexdigest() == digest)

    # ── ingest ───────────────────────────────────────────────────────
    # rows[(backend, L)][seed][tick] = row
    rows: dict[tuple[str, int], dict[int, dict[int, dict]]] = defaultdict(
        lambda: defaultdict(dict))
    planes: dict[tuple[str, int], dict[int, dict[int, complex]]] = (
        defaultdict(lambda: defaultdict(dict)))
    for name in files:
        if not name.endswith(".csv"):
            continue
        for r in load_rows(RESULTS / name):
            key = (r["actual_backend"], int(r["L"]))
            if "plane_loss_real" in r:
                planes[key][int(r["seed"])][int(r["x"])] = complex(
                    float(r["plane_loss_real"]), float(r["plane_loss_imag"]))
            else:
                rows[key][int(r["seed"])][int(r["tick"])] = r

    # Amendment A1 (2026-07-24, pre-data): primary backend is WSL2 CUDA for
    # L in {48, 64, 96} and Windows/MSVC CPU for L in {128, 192}.
    PRIMARY_BACKEND = {48: "gpu", 64: "gpu", 96: "gpu",
                       128: "cpu", 192: "cpu"}
    gpu_keys = [(PRIMARY_BACKEND[L], L) for L in VOLUMES]
    cpu_key = ("cpu", 48)
    check("MATRIX primary volumes present",
          all(k in rows for k in gpu_keys))
    check("MATRIX cpu reproduction present", cpu_key in rows)

    # ── G1-G7 structural (per registered dataset) ────────────────────
    for key in gpu_keys + [cpu_key]:
        backend, L = key
        omega, tau, t_star = pole(L)
        data = rows.get(key, {})
        ok_seeds = set(data.keys()) == set(SEEDS)
        ok = ok_seeds
        for seed in data:
            ticks = data[seed]
            expect = set(range(t_star + BRACKET + 1))
            ok = ok and set(ticks.keys()) == expect
            for t, r in ticks.items():
                ok = ok and r["execution_valid"] == "1"
                ok = ok and abs(float(r["omega"]) - omega) < 1e-12
                ok = ok and int(r["target_transition"]) == t_star
                ok = ok and abs(float(r["tau"]) - tau) < 1e-12
                ok = ok and float(r["phase_error"]) <= 0.5 * omega + 1e-14
                ok = ok and 0.0 <= float(r["min_site_probability"])
                ok = ok and float(r["max_site_probability"]) <= 0.1 + 1e-15
            # monotone occupancy, initial occupancy/neutrality
            occ = [int(ticks[t]["occupancy_after"]) for t in sorted(ticks)]
            ok = ok and all(a >= b for a, b in zip(occ, occ[1:]))
            r0 = ticks[0]
            ok = ok and int(r0["initial_occupancy"]) == L ** 3
            ok = ok and int(r0["initial_signed_state"]) == 0
            s0 = complex(float(r0["initial_source_real"]),
                         float(r0["initial_source_imag"]))
            ok = ok and abs(s0) >= 0.3
            if backend == "cpu":
                for t, r in ticks.items():
                    ok = ok and (int(r["history_evaporation"])
                                 == int(r["actual_removed"]))
                    ok = ok and int(r["history_other"]) == 0
        check(f"G1-G7 {backend} L={L}", ok)

    # ── G9 plane closure ─────────────────────────────────────────────
    for key in gpu_keys + [cpu_key]:
        backend, L = key
        _, _, t_star = pole(L)
        ok = True
        for seed in rows.get(key, {}):
            r = rows[key][seed].get(t_star)
            ok = ok and r is not None and float(
                r["plane_closure_rel"]) <= 1e-12
            ok = ok and len(planes.get(key, {}).get(seed, {})) == L
        check(f"G9 plane closure {backend} L={L}", ok)

    # ── conditional-expectation residuals (primary matrix) ──────────
    z_all: list[float] = []
    for key in gpu_keys:
        _, L = key
        _, _, t_star = pole(L)
        N = float(L ** 3)
        for t in range(t_star + BRACKET + 1):
            sa = sp = 0.0 + 0.0j
            vr = 0.0
            dn = er = 0.0
            for seed in SEEDS:
                r = rows[key][seed][t]
                s_before = complex(float(r["source_before_real"]),
                                   float(r["source_before_imag"]))
                loss = complex(float(r["expected_loss_real"]),
                               float(r["expected_loss_imag"]))
                s_after = complex(float(r["source_after_real"]),
                                  float(r["source_after_imag"]))
                sa += s_after
                sp += s_before - loss
                vr += float(r["removal_variance"])
                dn += float(r["actual_removed"])
                er += float(r["expected_removals"])
            n = float(len(SEEDS))
            z_s = abs(sa / n - sp / n) / max(1e-15, math.sqrt(vr) / (n * N))
            z_n = abs(dn / n - er / n) / max(1e-15, math.sqrt(vr) / n)
            z_all.extend([z_s, z_n])
    z_max = max(z_all)
    z_rms = math.sqrt(sum(z * z for z in z_all) / len(z_all))
    check("RESIDUAL max <= 6", z_max <= 6.0)
    check("RESIDUAL rms <= 2.5", z_rms <= 2.5)

    # ── CPU/CUDA L=48 agreement ──────────────────────────────────────
    agree = True
    _, _, t48 = pole(48)
    fields = ["source_before_real", "source_before_imag",
              "expected_loss_real", "expected_loss_imag", "source_hazard",
              "mean_local_energy", "actual_removed"]
    for seed in SEEDS:
        for t in range(t48 + BRACKET + 1):
            rg = rows[("gpu", 48)][seed][t]  # gpu L=48 is always primary
            rc = rows[cpu_key][seed][t]
            for f in fields:
                agree = agree and abs(float(rg[f]) - float(rc[f])) <= 1e-10
    check("CPU/CUDA L=48 agreement 1e-10", agree)

    # ── locked estimators ────────────────────────────────────────────
    h_phase: dict[int, tuple[float, float]] = {}
    h_tick: dict[int, float] = {}
    for key in gpu_keys:
        _, L = key
        omega, tau, t_star = pole(L)
        d = tau - t_star
        per_seed = []
        for seed in SEEDS:
            h = {t: float(rows[key][seed][t]["source_hazard"])
                 for t in (t_star - 1, t_star, t_star + 1)}
            a2 = (h[t_star + 1] - 2 * h[t_star] + h[t_star - 1]) / 2.0
            b1 = (h[t_star + 1] - h[t_star - 1]) / 2.0
            per_seed.append(h[t_star] + b1 * d + a2 * d * d)
        h_phase[L] = (sum(per_seed) / len(per_seed), jackknife_se(per_seed))
        # v1 estimator (ratio of ensemble means) for continuity gate G8
        S = Q = 0.0 + 0.0j
        for seed in SEEDS:
            r = rows[key][seed][t_star]
            S += complex(float(r["source_before_real"]),
                         float(r["source_before_imag"]))
            Q += complex(float(r["expected_loss_real"]),
                         float(r["expected_loss_imag"]))
        S /= len(SEEDS)
        Q /= len(SEEDS)
        h_tick[L] = (Q * S.conjugate()).real / abs(S) ** 2

    v1_style = []
    for seed in SEEDS:
        v1_style.append(float(rows[("gpu", 48)][seed][t48]["source_hazard"]))
    se48 = jackknife_se(v1_style)
    check("G8 continuity with FTD-0433 h_48",
          abs(h_tick[48] - V1_H48)
          <= 3.0 * math.sqrt(V1_SE48 ** 2 + se48 ** 2))

    print("\nPhase-corrected hazards:")
    for L in VOLUMES:
        m, se = h_phase[L]
        print(f"  L={L:4d}  h_phase={m:.8f} +- {se:.8f}   h_tick={h_tick[L]:.8f}")

    # ── locked model contest ─────────────────────────────────────────
    Ls = [float(L) for L in VOLUMES]
    hs = [h_phase[L][0] for L in VOLUMES]
    sig = [max(h_phase[L][1], 1e-12) for L in VOLUMES]

    # M1: weighted LS of ln h on ln L
    w = [(h / s) ** 2 for h, s in zip(hs, sig)]
    X = [[1.0, -math.log(L)] for L in Ls]
    y = [math.log(h) for h in hs]
    sxx = [[sum(w[i] * X[i][a] * X[i][b] for i in range(5))
            for b in range(2)] for a in range(2)]
    sxy = [sum(w[i] * X[i][a] * y[i] for i in range(5)) for a in range(2)]
    det = sxx[0][0] * sxx[1][1] - sxx[0][1] * sxx[1][0]
    a_hat = (sxy[0] * sxx[1][1] - sxy[1] * sxx[0][1]) / det
    p_hat = (sxx[0][0] * sxy[1] - sxx[1][0] * sxy[0]) / det
    chi2_1 = sum(w[i] * (y[i] - a_hat + p_hat * math.log(Ls[i])) ** 2
                 for i in range(5))

    # M2: floor + power on the locked p grid
    best = None
    for p in P_GRID:
        z = [L ** (-p) for L in Ls]
        wl = [1.0 / s ** 2 for s in sig]
        s00 = sum(wl)
        s01 = sum(wl[i] * z[i] for i in range(5))
        s11 = sum(wl[i] * z[i] * z[i] for i in range(5))
        t0 = sum(wl[i] * hs[i] for i in range(5))
        t1 = sum(wl[i] * hs[i] * z[i] for i in range(5))
        d2 = s00 * s11 - s01 * s01
        hinf = (t0 * s11 - t1 * s01) / d2
        c = (s00 * t1 - s01 * t0) / d2
        if hinf < 0.0:
            hinf = 0.0
            c = t1 / s11
        chi2 = sum(wl[i] * (hs[i] - hinf - c * z[i]) ** 2 for i in range(5))
        if best is None or chi2 < best[0]:
            best = (chi2, p, hinf, c)
    chi2_2, p2, hinf, c2 = best
    n = 5
    bic1 = chi2_1 + 2 * math.log(n)
    bic2 = chi2_2 + 3 * math.log(n)
    dbic = bic1 - bic2
    max_sig = max(sig)

    print(f"\nM1 power law: p={p_hat:.4f} chi2={chi2_1:.2f} BIC={bic1:.2f}")
    print(f"M2 floor+power: h_inf={hinf:.8f} p={p2:.2f} c={c2:.6f} "
          f"chi2={chi2_2:.2f} BIC={bic2:.2f}")
    print(f"dBIC (power - floor) = {dbic:+.2f}; 2*max sigma = {2*max_sig:.8f}")

    gates_pass = all(ok for _, ok in checks)
    if not gates_pass:
        outcome = "D — INVALID EXECUTION"
    elif dbic >= 10.0 and hinf > 2.0 * max_sig:
        outcome = "A — FINITE HAZARD FLOOR (tested range)"
    elif dbic <= -10.0:
        outcome = "B — POWER-LAW SUPPRESSION (tested range)"
    else:
        outcome = "C — UNRESOLVED"

    # ── secondary mechanism check at L=96 (non-gating) ───────────────
    mech = "NOT EVALUATED"
    key = ("gpu", 96)
    if key in planes and key in rows:
        L = 96
        _, _, ts96 = pole(L)
        walls = [-0.5, L / 2 - 0.5]
        contrib = defaultdict(float)
        total = 0.0
        for seed in SEEDS:
            r = rows[key][seed][ts96]
            S = complex(float(r["source_before_real"]),
                        float(r["source_before_imag"]))
            for x, pl in planes[key][seed].items():
                c = (pl * S.conjugate()).real / abs(S) ** 2 / len(SEEDS)
                dist = min(min(abs(x - wxy), L - abs(x - wxy))
                           for wxy in walls)
                contrib[dist] += c
                total += c
        near = sum(v for d, v in contrib.items() if d <= 8.0)
        far = sum(v for d, v in contrib.items() if d >= 16.0)
        mech_ok = (abs(near) <= 0.05 * abs(total)
                   and far >= 0.60 * total)
        mech = "CONFIRMED" if mech_ok else "NOT CONFIRMED"
        print(f"\nMechanism (L=96): near(d<=8)={near/total:+.4f} of h, "
              f"far(d>=16)={far/total:.4f} of h -> {mech}")

    failed = 0
    print()
    for name, passed in checks:
        print(("PASS  " if passed else "FAIL  ") + name)
        failed += not passed
    print(f"\nFTD-0436 result checks: {len(checks) - failed}/{len(checks)} "
          f"passed")
    print(f"OUTCOME: {outcome}")
    print(f"MECHANISM: {mech}")
    if failed:
        raise SystemExit(1)


if __name__ == "__main__":
    main()

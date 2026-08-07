#!/usr/bin/env python3
"""
atomic_next_three_campaigns.py -- FTD-0281/0282/0283 planning harness.

This file freezes the next three atomic-sector campaigns without running a
look-elsewhere scan. It has three jobs:

  * emit a provenance manifest for the three-track plan;
  * record the FTD-0282 exchange/correlation wall as a fixed negative-boundary
    test under the I1+I2+I3 import register;
  * run the FTD-0283 no-new-knob ladder only on a predeclared set of ions.

It does not compare against laboratory spectral lines, tune q values, vary
tolerances, or search for near-misses.
"""

from __future__ import annotations

import argparse
import datetime as _dt
import json
import platform
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
EXPLORATION = ROOT / "scripts" / "exploration"

OMEGA0 = 1.5
Q_UNIT = 0.3490
LADDER_L = 48
HYDROGENIC_ZS = [1, 2, 3]
HELIUM_LIKE_ZS = [2, 3]

TRACKS = [
    {
        "id": "FTD-0281",
        "name": "engine-native live-clock Coulomb spectroscopy",
        "kind": "engine diagnostic + preregistered FFT campaign",
        "primary_artifacts": [
            "engine/include/ftd/term_toggles.h",
            "engine/src/render_bridge.cpp",
            "engine/src/render_bridge_phases/phase_read.cpp",
            "engine/tests/test_db_clock_coulomb.cpp",
            "docs/theory/10_eft_program/preregistrations/engine_emergence_campaigns/PREREG_DB_CLOCK_COULOMB_SPECTROSCOPY_v1.md",
        ],
        "claim_ceiling": "hook-smoke only until the locked FFT campaign runs",
    },
    {
        "id": "FTD-0282",
        "name": "exchange/correlation wall",
        "kind": "negative-boundary record under fixed I1+I2+I3 imports",
        "primary_artifacts": [
            "scripts/exploration/atomic_next_three_campaigns.py",
            "docs/theory/10_eft_program/preregistrations/engine_emergence_campaigns/PREREG_ATOMIC_EXCHANGE_CORRELATION_WALL_v1.md",
        ],
        "claim_ceiling": "mean-field ceiling; exchange/correlation remain unrepresented",
    },
    {
        "id": "FTD-0283",
        "name": "no-new-knob atomic ladder",
        "kind": "fixed-cell dimensionless scaling test",
        "primary_artifacts": [
            "scripts/exploration/atomic_next_three_campaigns.py",
            "docs/theory/10_eft_program/preregistrations/engine_emergence_campaigns/PREREG_ATOMIC_NO_NEW_KNOB_LADDER_v1.md",
        ],
        "claim_ceiling": "dimensionless scaling only; no lab-unit calibration",
    },
]

BANNED_MOVES = [
    "no numerical near-miss or coincidence search",
    "no laboratory eV, wavelength, or NIST-line comparison",
    "no post-hoc tolerance changes",
    "no new q/omega0 tuning outside the frozen cells",
    "no unconditional FTD-derives-QM or FTD-derives-helium language",
    "no promotion of I1, I2, I3, FTD-0013, MC-T4.3, FC-1, or FTD-0270",
]


def run_git(args: list[str]) -> str:
    proc = subprocess.run(
        ["git", *args],
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if proc.returncode != 0:
        return f"<git failed: {' '.join(args)} :: {proc.stderr.strip()}>"
    return proc.stdout.strip()


def environment_info() -> dict[str, object]:
    info: dict[str, object] = {
        "python": sys.version,
        "platform": platform.platform(),
        "executable": sys.executable,
    }
    for name in ("numpy", "scipy"):
        try:
            module = __import__(name)
            info[name] = getattr(module, "__version__", "unknown")
        except Exception as exc:  # pragma: no cover - environment diagnostic
            info[name] = f"unavailable: {exc}"
    return info


def artifact_status(path: str, contains: str | None = None) -> dict[str, object]:
    p = ROOT / path
    text = p.read_text(encoding="utf-8", errors="replace") if p.exists() else ""
    return {
        "path": path,
        "exists": p.exists(),
        "contains": contains,
        "contains_ok": (contains in text) if contains else None,
    }


def build_manifest() -> dict[str, object]:
    status_short = run_git(["status", "--short"])
    return {
        "schema": "ftd.atomic_next_three.v1",
        "created_utc": _dt.datetime.now(_dt.timezone.utc).isoformat(),
        "repo": str(ROOT),
        "head": run_git(["rev-parse", "HEAD"]),
        "head_short": run_git(["rev-parse", "--short", "HEAD"]),
        "git_status_short": status_short,
        "dirty_tree": bool(status_short),
        "environment": environment_info(),
        "tracks": TRACKS,
        "fixed_cells": {
            "ftd_0283_ladder": {
                "omega0": OMEGA0,
                "q_unit": Q_UNIT,
                "L": LADDER_L,
                "hydrogenic_Z": HYDROGENIC_ZS,
                "helium_like_Z": HELIUM_LIKE_ZS,
            }
        },
        "banned_moves": BANNED_MOVES,
        "static_checks": static_checks(),
    }


def static_checks() -> list[dict[str, object]]:
    return [
        artifact_status("engine/include/ftd/term_toggles.h", "db_clock_coulomb"),
        artifact_status("engine/src/render_bridge.cpp", "solve_coulomb_poisson();"),
        artifact_status("engine/src/render_bridge_phases/phase_read.cpp", "omega_eff_sq"),
        artifact_status("engine/tests/test_db_clock_coulomb.cpp", "DBC-2"),
        artifact_status(
            "docs/theory/10_eft_program/preregistrations/engine_emergence_campaigns/PREREG_DB_CLOCK_COULOMB_SPECTROSCOPY_v1.md"
        ),
        artifact_status(
            "docs/theory/10_eft_program/preregistrations/engine_emergence_campaigns/PREREG_ATOMIC_EXCHANGE_CORRELATION_WALL_v1.md"
        ),
        artifact_status(
            "docs/theory/10_eft_program/preregistrations/engine_emergence_campaigns/PREREG_ATOMIC_NO_NEW_KNOB_LADDER_v1.md"
        ),
    ]


def write_json(payload: dict[str, object], out: Path | None) -> None:
    text = json.dumps(payload, indent=2, sort_keys=True) + "\n"
    if out:
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(text, encoding="utf-8")
        print(f"# wrote {out.resolve()}")
    else:
        print(text, end="")


def print_protocol() -> None:
    lines = [
        "# FTD-0281-A: engine hook smoke (no spectroscopy verdict)",
        "cmake --build engine/build --config Release --target test_db_clock_coulomb --parallel 24",
        "cd engine/build; ctest -j 24 -C Release -R \"db_clock_coulomb|render_bridge_golden\" --output-on-failure",
        "",
        "# FTD-0282: exchange/correlation wall record (no new imports)",
        "python scripts/exploration/atomic_next_three_campaigns.py --ftd-0282-wall-record --out scripts/exploration/results/atomic_next_three_YYYY-MM-DD/exchange_correlation_wall.json",
        "",
        "# FTD-0283: no-new-knob ladder record (fixed Z set; no lab lines)",
        "python scripts/exploration/atomic_next_three_campaigns.py --ftd-0283-ladder-record --out scripts/exploration/results/atomic_next_three_YYYY-MM-DD/no_new_knob_ladder.json",
    ]
    print("\n".join(lines))


def wall_record() -> dict[str, object]:
    return {
        "schema": "ftd.atomic_exchange_correlation_wall.v1",
        "id": "FTD-0282",
        "result_class": "NEGATIVE_BOUNDARY_UNDER_FIXED_IMPORTS",
        "imports": ["I1: omega0 clock scalar", "I2: scalar-potential coupling", "I3: mode occupancy"],
        "represented": [
            {
                "observable": "closed_shell_restricted_hartree_1s2",
                "status": "represented",
                "reason": "one spatial mode with two imported occupations mutually sourcing through Gauss",
            }
        ],
        "wall_observables": [
            {
                "observable": "dynamic_Pauli_exchange",
                "status": "unrepresented",
                "model_value": 0.0,
                "reason": "no antisymmetrized two-particle configuration space or spin-statistics operator exists under I1+I2+I3",
            },
            {
                "observable": "ortho_para_exchange_splitting",
                "status": "unrepresented",
                "model_value": 0.0,
                "reason": "the fixed Hamiltonian is spin-independent and carries no singlet/triplet spatial antisymmetry branch",
            },
            {
                "observable": "correlation_energy",
                "status": "unrepresented",
                "model_value": "not computable in the restricted Hartree state space",
                "reason": "correlation requires configuration-space entanglement beyond the single-density Hartree closure",
            },
        ],
        "verdict_logic": {
            "W-SCOPE": "pass iff no new import is introduced",
            "W-NEG": "pass iff exchange/correlation remain unrepresented rather than fitted",
            "W-LANG": "pass iff the result states this is a boundary, not a derivation upgrade",
        },
        "verdict_if_all_pass": "EXCHANGE-CORRELATION-WALL-CONFIRMED",
        "banned_moves": BANNED_MOVES,
    }


def import_atomic_modules():
    if str(EXPLORATION) not in sys.path:
        sys.path.insert(0, str(EXPLORATION))
    import derive_hydrogen_lattice_spectrum as hyd  # type: ignore
    import derive_helium_lattice_scf as he  # type: ignore
    import numpy as np  # type: ignore
    import scipy.sparse as sp  # type: ignore
    import scipy.sparse.linalg as spla  # type: ignore

    return hyd, he, np, sp, spla


def he_like_cell(hyd, he, np, sp, spla, L: int, q_unit: float, Z: int) -> dict[str, object]:
    """Restricted Hartree cell for a helium-like ion with nuclear strength Z*q."""
    a_lap = hyd.build_L18(L, periodic=True)
    v_nuc, _, _ = hyd.coulomb_well(L, Z * q_unit)
    kinetic = (-hyd.C2 / (2.0 * OMEGA0)) * a_lap

    def solve(vee_on: bool) -> dict[str, object]:
        rho = None
        e_prev = None
        eps = float("nan")
        e_ee = 0.0
        for it in range(60):
            if rho is None or not vee_on:
                vh = np.zeros((L, L, L))
            else:
                vh = he.hartree_potential(rho, q_unit)
            h = kinetic + sp.diags((v_nuc + vh).reshape(-1))
            vals, vecs = spla.eigsh(h, k=1, which="SA")
            eps = float(vals[0])
            psi = vecs[:, 0]
            rho_new = (psi * psi).reshape(L, L, L)
            rho_new /= rho_new.sum()
            rho = rho_new if rho is None else 0.5 * rho + 0.5 * rho_new
            rho /= rho.sum()
            if vee_on:
                e_ee = float(np.sum(rho * he.hartree_potential(rho, q_unit)))
            else:
                e_ee = 0.0
            e_tot = 2.0 * eps - e_ee
            if e_prev is not None and abs(e_tot - e_prev) < 1e-9:
                return {"E": e_tot, "eps": eps, "E_ee": e_ee, "iters": it + 1, "converged": True}
            e_prev = e_tot
        return {"E": e_prev, "eps": eps, "E_ee": e_ee, "iters": 60, "converged": False}

    scf = solve(True)
    ctrl = solve(False)
    e_nonint = float(ctrl["E"])
    sigma = float(scf["E"]) / e_nonint if e_nonint else float("nan")
    return {
        "Z": Z,
        "L": L,
        "q_unit": q_unit,
        "E_He_like": scf["E"],
        "E_nonint": e_nonint,
        "sigma": sigma,
        "converged": bool(scf["converged"] and ctrl["converged"]),
        "iters": scf["iters"],
    }


def ladder_record(include_helium_like: bool = True) -> dict[str, object]:
    hyd, he, np, sp, spla = import_atomic_modules()
    h_rows = []
    for z in HYDROGENIC_ZS:
        q = z * Q_UNIT
        res = hyd.spectrum(LADDER_L, q, OMEGA0, k_eigs=10, potential="lattice")
        if res is None:
            h_rows.append({"Z": z, "q": q, "tachyonic": True})
            continue
        _a, _omega, e, _vmin, n_bound = res
        gap12 = float(np.mean(e[1:5]) - e[0])
        h_rows.append(
            {
                "Z": z,
                "q": q,
                "tachyonic": False,
                "n_bound": int(n_bound),
                "E0": float(e[0]),
                "gap12": gap12,
                "gap12_over_Z2": gap12 / (z * z),
            }
        )

    valid_h = [r for r in h_rows if not r.get("tachyonic")]
    h_gaps = [float(r["gap12"]) for r in valid_h]
    h_scaled = [float(r["gap12_over_Z2"]) for r in valid_h]
    monotone = all(h_gaps[i] < h_gaps[i + 1] for i in range(len(h_gaps) - 1))
    scaled_spread = (
        (max(h_scaled) - min(h_scaled)) / abs(sum(h_scaled) / len(h_scaled))
        if h_scaled
        else float("inf")
    )

    he_rows = []
    if include_helium_like:
        for z in HELIUM_LIKE_ZS:
            he_rows.append(he_like_cell(hyd, he, np, sp, spla, LADDER_L, Q_UNIT, z))

    return {
        "schema": "ftd.atomic_no_new_knob_ladder.v1",
        "id": "FTD-0283",
        "fixed_inputs": {
            "omega0": OMEGA0,
            "q_unit": Q_UNIT,
            "L": LADDER_L,
            "hydrogenic_Z": HYDROGENIC_ZS,
            "helium_like_Z": HELIUM_LIKE_ZS if include_helium_like else [],
        },
        "hydrogenic": h_rows,
        "helium_like": he_rows,
        "gates": {
            "L-H1-nontachyonic": all(not r.get("tachyonic") for r in h_rows),
            "L-H2-gap-monotone-in-Z": monotone,
            "L-H3-Z2-scaled-spread-le-0.25": scaled_spread <= 0.25,
            "L-He1-SCF-converged": all(bool(r.get("converged")) for r in he_rows) if he_rows else None,
        },
        "diagnostics": {
            "hydrogenic_gap12_over_Z2_spread": scaled_spread,
        },
        "verdict_if_all_gates_pass": "NO-NEW-KNOB-LADDER-CONFIRMED",
        "banned_moves": BANNED_MOVES,
    }


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--manifest", action="store_true", help="write the three-track manifest")
    ap.add_argument("--verify-static", action="store_true", help="check static artifacts exist")
    ap.add_argument("--print-protocol", action="store_true", help="print locked commands")
    ap.add_argument("--ftd-0282-wall-record", action="store_true", help="write the fixed wall record")
    ap.add_argument("--ftd-0283-ladder-record", action="store_true", help="run fixed no-new-knob ladder cells")
    ap.add_argument("--skip-helium-like", action="store_true", help="FTD-0283 diagnostic escape hatch; records hydrogenic leg only")
    ap.add_argument("--out", type=Path, help="output JSON path")
    args = ap.parse_args(argv)

    did = False
    if args.print_protocol:
        print_protocol()
        did = True
    if args.verify_static:
        checks = static_checks()
        for item in checks:
            ok = item["exists"] and (item["contains_ok"] is not False)
            print(f"{'PASS' if ok else 'FAIL'}: {item['path']}")
        did = True
        if not all(item["exists"] and (item["contains_ok"] is not False) for item in checks):
            return 1
    if args.manifest:
        write_json(build_manifest(), args.out)
        did = True
    if args.ftd_0282_wall_record:
        write_json(wall_record(), args.out)
        did = True
    if args.ftd_0283_ladder_record:
        write_json(ladder_record(include_helium_like=not args.skip_helium_like), args.out)
        did = True

    if not did:
        ap.print_help()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

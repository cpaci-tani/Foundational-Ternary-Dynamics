#!/usr/bin/env python3
"""
atomic_sector_hardening.py -- FTD-0280 atomic-sector replay/manifest harness.

This is not a search script. It does not tune parameters, scan for near-misses,
or compare against laboratory helium lines. It hardens the conditional atomic
sector by checking the provenance of the locked FTD-0278/0279 artifacts and, on
explicit request, replaying their frozen record commands into a timestamped
output directory.

Modes:
  --manifest [--out path]     Write a JSON provenance manifest.
  --verify-locks              Check locked script hashes and prereg tags.
  --replay-records --out-dir  Re-run the locked FTD-0278/0279 record commands.
  --print-protocol            Print the next pre-registered campaign commands.

The replay mode can be slow. It is intentionally opt-in so a manifest check
cannot accidentally become a numerical measurement campaign.
"""

from __future__ import annotations

import argparse
import datetime as _dt
import json
import platform
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


@dataclass(frozen=True)
class LockedArtifact:
    name: str
    path: str
    sha256: str
    tag: str | None = None
    tag_commit: str | None = None


LOCKED_ARTIFACTS = [
    LockedArtifact(
        name="FTD-0278 hydrogen operator spectroscopy",
        path="scripts/exploration/derive_hydrogen_lattice_spectrum.py",
        sha256="8e953fac6b7dc251c21290f6e21d416c6e2a9d0e78d923a94e8953c73654573f",
        tag="preregister-hydrogen-lattice-spectrum-v1",
        tag_commit="6be49fe98a63164d50bb3c4dc6250ab4e9e2a33a",
    ),
    LockedArtifact(
        name="FTD-0279 helium restricted Hartree SCF",
        path="scripts/exploration/derive_helium_lattice_scf.py",
        sha256="ecfa2cd07cc23907867c2d97afcb6c1b1aeb0aa6506dc3e1308b16c912cd7714",
        tag="preregister-helium-lattice-scf-v1",
        tag_commit="310ad4ee2a275e0e6c5ecd00cbfd30b55e65f551",
    ),
    LockedArtifact(
        name="FTD-0278 preregistration",
        path="docs/theory/10_eft_program/preregistrations/engine_emergence_campaigns/PREREG_HYDROGEN_LATTICE_SPECTRUM_v1.md",
        sha256="6d644aabce4cbd6e54fb159e776eb4e046dfd422ed115dd5710dd7fb2d21c792",
    ),
    LockedArtifact(
        name="FTD-0279 preregistration",
        path="docs/theory/10_eft_program/preregistrations/engine_emergence_campaigns/PREREG_HELIUM_LATTICE_SCF_v1.md",
        sha256="f49262b727fb9c29fc08af52f0f432cd052fc326b32a77c90cd4b9ad3691019a",
    ),
]


RECORD_COMMANDS = [
    {
        "id": "FTD-0278",
        "expected_verdict": "HYDROGEN-CONFIRMED",
        "script": "scripts/exploration/derive_hydrogen_lattice_spectrum.py",
        "csv": "hydrogen_spectrum_replay.csv",
        "log": "hydrogen_spectrum_replay.log",
    },
    {
        "id": "FTD-0279",
        "expected_verdict": "HELIUM-CONFIRMED",
        "script": "scripts/exploration/derive_helium_lattice_scf.py",
        "csv": "helium_scf_replay.csv",
        "log": "helium_scf_replay.log",
    },
]


NEXT_PROTOCOL_COMMANDS = [
    "# FTD-0280-A: provenance manifest (no numerical run)",
    "python scripts/exploration/atomic_sector_hardening.py --verify-locks --manifest --out scripts/exploration/results/atomic_sector_hardening_YYYY-MM-DD/manifest.json",
    "",
    "# FTD-0280-B: locked replay of FTD-0278/0279 (same frozen scripts, no new gates)",
    "python scripts/exploration/atomic_sector_hardening.py --replay-records --out-dir scripts/exploration/results/atomic_sector_hardening_YYYY-MM-DD",
    "",
    "# Any L=96/128 Python operator/SCF attempt is DIAGNOSTIC_NOT_VERDICT until a v2",
    "# solver preregistration freezes memory/runtime gates and convergence settings.",
    "",
    "# FTD-0281 candidate: engine-native live-clock Coulomb spectroscopy",
    "# Implement only under a separate preregistration: default-off db_clock_coulomb toggle,",
    "# live Gauss potential around a locked charge, time-series FFT peaks vs FTD-0278 operator omega_n.",
    "",
    "# FTD-0282 candidate: exchange/correlation wall",
    "# Implement only under a separate preregistration: keep I1+I2+I3 fixed, predict that",
    "# correlation and ortho/para exchange remain absent unless a deeper statistics import is declared.",
    "",
    "# FTD-0283 candidate: no-new-knob atomic ladder",
    "# Implement only under a separate preregistration: fixed omega0/q/L/Z cells,",
    "# dimensionless scaling only, no laboratory line matching.",
    "",
    "# Implemented next-three harness:",
    "python scripts/exploration/atomic_next_three_campaigns.py --print-protocol",
]


FROZEN_PROTOCOL_SUMMARY = {
    "hydrogen_ftd_0278": {
        "omega0_record": 1.5,
        "record_qs": [1.1170, 0.9308, 0.6981],
        "record_Ls": [48, 64],
        "k_eigs": 10,
        "gates": {
            "G-1": "periodic eigenvalues match M(k) to < 1e-10",
            "F-A": "lattice/reference gap12 ratio within 1 +/- 0.05 in all 6 cells",
            "F-B": "L=64 Rydberg ratio strictly decreases across a0={2.5,3,4}; a0=4 endpoint in (1.0,1.40)",
            "F-C": "T1u spread <= 5% gap12 and A1g-T1u split <= 50% gap12",
            "F-E": "massless Dirichlet scaling exponent in [0.8,1.2]",
        },
        "non_verdict_sanity": [
            "omega2 > 0",
            "sorted energies",
            "positive gap12",
            "six valid cells",
            "n=2-only ladder check; n=3 not claimed",
        ],
    },
    "helium_ftd_0279": {
        "omega0": 1.5,
        "record_qs": [0.4654, 0.3490],
        "record_Ls": [48, 64],
        "gates": {
            "F-He-A": "|sigma_ENG - sigma_REF| <= 0.03 and |ion_ENG - ion_REF| <= 0.05 in all 4 cells",
            "F-He-B": "sigma_ENG in (0.60,0.80), and L=64 a0=8 closer to 0.7154 than a0=6",
            "F-He-C": "ionization ratio in (0.30,0.60) in all cells",
            "F-He-D": "He+ independent eigenpath agrees with E_nonint/2 to 1e-6 relative",
        },
        "reference_note": (
            "The continuum reference changes the nuclear well to spectral periodized "
            "Coulomb while the Hartree repulsion remains the engine-symbol convolution; "
            "this is locked behavior and must be disclosed in any replay."
        ),
    },
}


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


def sha256_file(path: Path) -> str:
    import hashlib

    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def artifact_status(item: LockedArtifact) -> dict[str, object]:
    path = ROOT / item.path
    actual = sha256_file(path) if path.exists() else None
    tag_actual = run_git(["rev-list", "-n", "1", item.tag]) if item.tag else None
    tag_ok = None
    if item.tag:
        tag_ok = tag_actual == item.tag_commit
    return {
        "name": item.name,
        "path": item.path,
        "exists": path.exists(),
        "expected_sha256": item.sha256,
        "actual_sha256": actual,
        "sha256_ok": actual == item.sha256,
        "tag": item.tag,
        "expected_tag_commit": item.tag_commit,
        "actual_tag_commit": tag_actual,
        "tag_ok": tag_ok,
    }


def build_manifest() -> dict[str, object]:
    status_short = run_git(["status", "--short"])
    return {
        "schema": "ftd.atomic_sector_hardening.v1",
        "created_utc": _dt.datetime.now(_dt.timezone.utc).isoformat(),
        "repo": str(ROOT),
        "head": run_git(["rev-parse", "HEAD"]),
        "head_short": run_git(["rev-parse", "--short", "HEAD"]),
        "git_status_short": status_short,
        "dirty_tree": bool(status_short),
        "environment": environment_info(),
        "epistemic_scope": {
            "result_class": "CONDITIONAL -- DERIVED-GIVEN-IMPOSED-INPUT",
            "imports": [
                "I1 clock scalar omega0 proportional to M_REST",
                "I2 scalar-potential coupling omega_eff^2 = omega0^2 + 2 omega0 V",
                "I3 mode occupancy for two-electron mean-field atoms",
            ],
            "banned_moves": [
                "no numerical near-miss or coincidence search",
                "no unconditional FTD derives QM/helium language",
                "no laboratory line comparison or unit calibration in this harness",
                "no exchange/correlation claim without a new preregistered import",
            ],
        },
        "frozen_protocol_summary": FROZEN_PROTOCOL_SUMMARY,
        "locked_artifacts": [artifact_status(item) for item in LOCKED_ARTIFACTS],
        "record_commands": [
            {
                **cmd,
                "argv": [
                    sys.executable,
                    cmd["script"],
                    "--record",
                    "--out",
                    f"<out-dir>/{cmd['csv']}",
                ],
            }
            for cmd in RECORD_COMMANDS
        ],
        "next_protocol_commands": NEXT_PROTOCOL_COMMANDS,
    }


def environment_info() -> dict[str, str | None]:
    def version(module_name: str) -> str | None:
        try:
            module = __import__(module_name)
        except Exception:
            return None
        return getattr(module, "__version__", None)

    return {
        "python": sys.version.replace("\n", " "),
        "executable": sys.executable,
        "platform": platform.platform(),
        "numpy": version("numpy"),
        "scipy": version("scipy"),
    }


def write_json(data: dict[str, object], out: str | None) -> None:
    text = json.dumps(data, indent=2, sort_keys=True) + "\n"
    if out:
        path = ROOT / out if not Path(out).is_absolute() else Path(out)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")
        print(f"# wrote {path}")
    else:
        print(text, end="")


def verify_locks() -> int:
    statuses = [artifact_status(item) for item in LOCKED_ARTIFACTS]
    ok = True
    for status in statuses:
        item_ok = bool(status["exists"]) and bool(status["sha256_ok"])
        if status["tag"] is not None:
            item_ok = item_ok and bool(status["tag_ok"])
        ok = ok and item_ok
        label = "PASS" if item_ok else "FAIL"
        print(f"{label}: {status['name']}")
        print(f"  path: {status['path']}")
        print(f"  sha256: {status['actual_sha256']}")
        if status["tag"] is not None:
            print(f"  tag {status['tag']}: {status['actual_tag_commit']}")
    return 0 if ok else 1


def parse_verdict(csv_path: Path, log_text: str) -> str | None:
    if csv_path.exists():
        for line in csv_path.read_text(encoding="utf-8", errors="replace").splitlines():
            parts = [part.strip() for part in line.split(",")]
            if parts and parts[0] == "VERDICT" and len(parts) > 1:
                return parts[1]
    match = re.search(r"VERDICT:\s*([A-Z0-9_-]+)", log_text)
    return match.group(1) if match else None


def replay_records(out_dir: str) -> int:
    target = ROOT / out_dir if not Path(out_dir).is_absolute() else Path(out_dir)
    target.mkdir(parents=True, exist_ok=True)
    summary: list[dict[str, object]] = []
    overall_ok = True
    for cmd in RECORD_COMMANDS:
        csv_path = target / cmd["csv"]
        log_path = target / cmd["log"]
        argv = [sys.executable, cmd["script"], "--record", "--out", str(csv_path)]
        print(f"# running {' '.join(argv)}")
        proc = subprocess.run(
            argv,
            cwd=ROOT,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )
        log_path.write_text(proc.stdout, encoding="utf-8")
        verdict = parse_verdict(csv_path, proc.stdout)
        ok = proc.returncode == 0 and verdict == cmd["expected_verdict"]
        overall_ok = overall_ok and ok
        summary.append(
            {
                "id": cmd["id"],
                "argv": argv,
                "returncode": proc.returncode,
                "expected_verdict": cmd["expected_verdict"],
                "actual_verdict": verdict,
                "ok": ok,
                "csv": str(csv_path),
                "log": str(log_path),
            }
        )
        print(f"# {cmd['id']}: verdict={verdict} expected={cmd['expected_verdict']} ok={ok}")
    write_json({"schema": "ftd.atomic_sector_replay.v1", "runs": summary}, str(target / "replay_summary.json"))
    return 0 if overall_ok else 1


def print_protocol() -> None:
    print("\n".join(NEXT_PROTOCOL_COMMANDS))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", action="store_true")
    parser.add_argument("--verify-locks", action="store_true")
    parser.add_argument("--replay-records", action="store_true")
    parser.add_argument("--print-protocol", action="store_true")
    parser.add_argument("--out", default=None, help="Manifest output path")
    parser.add_argument("--out-dir", default=None, help="Replay output directory")
    args = parser.parse_args()

    requested = any([args.manifest, args.verify_locks, args.replay_records, args.print_protocol])
    if not requested:
        parser.error("choose at least one mode")

    exit_code = 0
    if args.verify_locks:
        exit_code = max(exit_code, verify_locks())
    if args.manifest:
        write_json(build_manifest(), args.out)
    if args.print_protocol:
        print_protocol()
    if args.replay_records:
        if not args.out_dir:
            parser.error("--replay-records requires --out-dir")
        exit_code = max(exit_code, replay_records(args.out_dir))
    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())

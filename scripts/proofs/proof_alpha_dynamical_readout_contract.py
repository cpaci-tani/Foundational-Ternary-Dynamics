#!/usr/bin/env python3
"""
proof_alpha_dynamical_readout_contract.py

Static verifier for FTD-0284: Alpha Dynamical Readout Discriminator.

This script is intentionally not a numerical search. It computes the fixed
identities needed to state the dynamical-readout problem and checks that the
pre-registration contains the required honesty anchors before any later engine
measurement can be treated as a record.

Run:
  python scripts/proofs/proof_alpha_dynamical_readout_contract.py --verify-static
  python scripts/proofs/proof_alpha_dynamical_readout_contract.py --manifest
"""

from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import platform
import subprocess
import sys
from pathlib import Path

import mpmath as mp


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs" / "theory" / "10_eft_program" / "preregistrations" / "PREREG_ALPHA_DYNAMICAL_READOUT_v1.md"
SCRIPT_REL = "scripts/proofs/proof_alpha_dynamical_readout_contract.py"

REQUIRED_PREREG_ANCHORS = [
    "FTD-0242",
    "FTD-0244",
    "FTD-0284",
    "NATIVE-NULL",
    "DYNAMICAL-FOUND",
    "POSTULATE-W",
    "No CODATA input",
    "No near-miss",
    "No promotion",
    "g_match is not a derivation",
]

BANNED_ASSERTIONS = [
    "FTD derives physical alpha",
    "alpha is derived from P1-P5",
    "x_+ = 1/alpha [THEOREM]",
    "g_match is derived",
]


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


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


def fixed_identities() -> dict[str, str]:
    mp.mp.dps = 60
    g_star = mp.gamma(mp.mpf(1) / 4) / mp.gamma(mp.mpf(3) / 4)
    trace = 16 * g_star**2
    det = 16 * g_star**3
    disc = trace**2 - 4 * det
    x_plus = (trace + mp.sqrt(disc)) / 2
    alpha_tree = 1 / x_plus

    geometric_coulomb = 1 / (2 * mp.pi)
    g_match_sq = alpha_tree / geometric_coulomb
    g_match = mp.sqrt(g_match_sq)

    native_vs_tree = geometric_coulomb / alpha_tree

    return {
        "G_star": mp.nstr(g_star, 30),
        "trace_16G2": mp.nstr(trace, 30),
        "det_16G3": mp.nstr(det, 30),
        "x_plus": mp.nstr(x_plus, 30),
        "alpha_tree_1_over_x_plus": mp.nstr(alpha_tree, 30),
        "geometric_unit_coulomb_1_over_2pi": mp.nstr(geometric_coulomb, 30),
        "g_match_squared_2pi_over_x_plus": mp.nstr(g_match_sq, 30),
        "g_match_sqrt_2pi_over_x_plus": mp.nstr(g_match, 30),
        "native_unit_response_over_alpha_tree": mp.nstr(native_vs_tree, 30),
    }


def build_manifest() -> dict[str, object]:
    ids = fixed_identities()
    status_short = run_git(["status", "--short"])
    return {
        "schema": "ftd.alpha_dynamical_readout_contract.v1",
        "created_utc": dt.datetime.now(dt.timezone.utc).isoformat(),
        "repo": str(ROOT),
        "head": run_git(["rev-parse", "HEAD"]),
        "head_short": run_git(["rev-parse", "--short", "HEAD"]),
        "git_status_short": status_short,
        "dirty_tree": bool(status_short),
        "environment": {
            "python": sys.version,
            "platform": platform.platform(),
            "mpmath": getattr(mp, "__version__", "unknown"),
        },
        "artifact": {
            "path": SCRIPT_REL,
            "sha256": sha256_file(ROOT / SCRIPT_REL),
        },
        "pre_registration": {
            "path": str(PREREG.relative_to(ROOT)),
            "exists": PREREG.exists(),
        },
        "fixed_identities": ids,
        "classification": {
            "native_unit_coupling": "NATIVE-NULL unless a future P1-P5 engine mechanism produces a non-unit response without alpha input",
            "qed_match_coupling": "POSTULATE-W / imposed correspondence because g_match uses x_plus",
            "promotion_allowed": False,
        },
        "frozen_outcomes": [
            "NATIVE-NULL",
            "DYNAMICAL-FOUND",
            "POSTULATE-W",
            "INVALIDATED",
        ],
    }


def verify_static() -> list[tuple[str, bool, str]]:
    results: list[tuple[str, bool, str]] = []
    ids = fixed_identities()

    native_ratio = mp.mpf(ids["native_unit_response_over_alpha_tree"])
    g_match_sq = mp.mpf(ids["g_match_squared_2pi_over_x_plus"])

    results.append((
        "unit native geometric response is not the tree alpha",
        native_ratio > 20,
        f"ratio={mp.nstr(native_ratio, 12)}",
    ))
    results.append((
        "qed-match coupling is sub-unit and depends on x_plus",
        0 < g_match_sq < 1,
        f"g_match^2={mp.nstr(g_match_sq, 12)}",
    ))
    results.append((
        "pre-registration exists",
        PREREG.exists(),
        str(PREREG.relative_to(ROOT)),
    ))

    if PREREG.exists():
        text = PREREG.read_text(encoding="utf-8", errors="replace")
        script_sha = sha256_file(ROOT / SCRIPT_REL)
        for anchor in REQUIRED_PREREG_ANCHORS:
            results.append((f"prereg anchor present: {anchor}", anchor in text, ""))
        results.append((
            "prereg records this verifier sha256",
            script_sha in text,
            script_sha,
        ))
        for phrase in BANNED_ASSERTIONS:
            results.append((f"banned assertion absent: {phrase}", phrase not in text, ""))

    return results


def print_protocol() -> None:
    print("# FTD-0284 static contract verifier")
    print("python scripts/proofs/proof_alpha_dynamical_readout_contract.py --verify-static --manifest")
    print()
    print("# Future run-of-record is NOT authorized by this verifier alone.")
    print("# A later engine instrument must freeze: observable, toggles, coupling inputs,")
    print("# finite-size windows, and verdict gates before any measurement is interpreted.")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--verify-static", action="store_true", help="run contract and prereg checks")
    ap.add_argument("--manifest", action="store_true", help="print JSON manifest")
    ap.add_argument("--out", type=Path, help="optional manifest output path")
    ap.add_argument("--print-protocol", action="store_true", help="print frozen protocol summary")
    args = ap.parse_args()

    if not (args.verify_static or args.manifest or args.print_protocol):
        args.verify_static = True

    exit_code = 0
    if args.verify_static:
        print("=" * 78)
        print("FTD-0284 alpha dynamical readout contract -- static verifier")
        print("=" * 78)
        for name, passed, detail in verify_static():
            print(f"  [{'PASS' if passed else 'FAIL'}] {name}" + (f" -- {detail}" if detail else ""))
            if not passed:
                exit_code = 1
        print("=" * 78)

    if args.manifest:
        payload = build_manifest()
        text = json.dumps(payload, indent=2, sort_keys=True) + "\n"
        if args.out:
            args.out.parent.mkdir(parents=True, exist_ok=True)
            args.out.write_text(text, encoding="utf-8")
            print(f"# wrote {args.out.resolve()}")
        else:
            print(text, end="")

    if args.print_protocol:
        print_protocol()

    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())

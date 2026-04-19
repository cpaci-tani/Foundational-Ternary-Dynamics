"""Build engine/web/data/verify-manifest.json from FTD rows + measurements.

Three tiers, each with its own required-field contract (see §8 of the
Verify Panel Redesign spec). Failures raise TierAssertionError with a
specific message so the build either produces an honest manifest or
does not produce one at all.
"""
from __future__ import annotations
import json
import subprocess
from pathlib import Path
from datetime import datetime, timezone


class TierAssertionError(Exception):
    """Raised when a row violates its tier's required-field contract."""


HARD_REQUIRED = ("id", "tier", "epistemic", "question", "ftd_value", "formula",
                 "inputs_used", "theory_ref")
PARAMETRIC_REQUIRED = ("id", "tier", "epistemic", "question", "ftd_value",
                       "formula", "formula_source", "ftd_inputs", "theory_ref")
UNPREDICTED_REQUIRED = ("id", "tier", "epistemic", "question")


def _require_fields(row, fields, tier_name):
    for f in fields:
        if f not in row or row[f] in (None, ""):
            raise TierAssertionError(
                f"{tier_name} row '{row.get('id', '?')}' missing required field '{f}'"
            )


def _validate_row(row):
    tier = row.get("tier")
    if tier == "hard":
        _require_fields(row, HARD_REQUIRED, "hard")
    elif tier == "parametric":
        _require_fields(row, PARAMETRIC_REQUIRED, "parametric")
        if row.get("formula_source") != "SM":
            raise TierAssertionError(
                f"parametric row '{row['id']}' must declare formula_source='SM' "
                f"(got {row.get('formula_source')!r})"
            )
    elif tier == "unpredicted":
        _require_fields(row, UNPREDICTED_REQUIRED, "unpredicted")
        if "ftd_value" in row:
            raise TierAssertionError(
                f"unpredicted row '{row['id']}' must NOT carry an ftd_value"
            )
    else:
        raise TierAssertionError(
            f"row '{row.get('id', '?')}' has unknown tier {tier!r}"
        )


def _compute_delta(ftd, measured, sigma):
    """Return (absolute_delta, rel_error, pull_in_sigma, delta_ppb)."""
    abs_d = ftd - measured
    rel = abs_d / measured if measured != 0 else float("nan")
    pull = abs_d / sigma if sigma and sigma > 0 else None
    ppb = rel * 1e9
    return abs_d, rel, pull, ppb


def _git_commit():
    try:
        out = subprocess.check_output(
            ["git", "rev-parse", "--short", "HEAD"],
            cwd=Path(__file__).resolve().parents[2],
            text=True,
        ).strip()
        return out
    except Exception:
        return "unknown"


def build_manifest(*, ftd_rows, measurements, build_stamp):
    """Join FTD rows with measurements and emit the manifest structure.

    ftd_rows: list of dicts; each must carry 'id' and 'tier' at minimum.
    measurements: dict with 'measurements': [ {id, value, sigma, ...}, ... ].
    build_stamp: dict merged into manifest top level (commit, timestamp, etc.).
    """
    # Build id -> measurement map first, validate no duplicates.
    meas_by_id = {}
    for m in measurements.get("measurements", []):
        if m["id"] in meas_by_id:
            raise TierAssertionError(f"duplicate measurement id {m['id']!r}")
        meas_by_id[m["id"]] = m

    # Per-tier validation and joined-row construction.
    tiers = {"hard": [], "parametric": [], "unpredicted": []}
    ftd_ids = set()
    for row in ftd_rows:
        _validate_row(row)
        ftd_ids.add(row["id"])
        tier = row["tier"]
        meas = meas_by_id.get(row["id"])
        # Hard and parametric rows MUST have a matching measurement.
        if tier in ("hard", "parametric") and meas is None:
            raise TierAssertionError(
                f"{tier} row '{row['id']}' has no matching measurement"
            )
        joined = dict(row)
        if meas is not None:
            joined["measurement"] = {
                "value": meas["value"], "sigma": meas["sigma"],
                "units": meas.get("units", ""), "source": meas["source"],
                "url": meas.get("url", ""), "date": meas.get("date", ""),
            }
        if tier == "hard":
            abs_d, rel, pull, ppb = _compute_delta(
                row["ftd_value"], meas["value"], meas["sigma"])
            joined["delta"] = abs_d
            joined["rel_error"] = rel
            joined["pull"] = pull
            joined["delta_ppb"] = ppb
        elif tier == "parametric":
            abs_d, rel, _pull, ppb = _compute_delta(
                row["ftd_value"], meas["value"], meas["sigma"])
            joined["delta"] = abs_d
            joined["rel_error"] = rel
            joined["delta_ppb"] = ppb
            # Explicitly NO pull for parametric tier — see spec §4.2.
        elif tier == "unpredicted":
            joined["delta_ppb"] = None
        tiers[tier].append(joined)

    # Orphan measurements (no matching FTD row) are a build failure.
    orphans = [mid for mid in meas_by_id if mid not in ftd_ids]
    if orphans:
        raise TierAssertionError(
            f"measurements without matching FTD rows: {orphans}"
        )

    # Largest tensions come from the HARD tier only (spec §5).
    def _pull_key(r):
        p = r.get("pull")
        return abs(p) if p is not None else 0.0
    top_tensions = sorted(tiers["hard"], key=_pull_key, reverse=True)[:3]

    return {
        "schema_version": 1,
        "build_stamp": build_stamp,
        "counts": {k: len(v) for k, v in tiers.items()},
        "tiers": tiers,
        "top_tensions": [
            {"id": r["id"], "question": r["question"],
             "pull": r.get("pull"), "delta_ppb": r.get("delta_ppb")}
            for r in top_tensions
        ],
    }


def _ftd_rows_from_constants():
    """The canonical FTD-side rowset. Extend this function as new rows are added.

    Currently wires three seed rows (one per tier) to match the seed
    measurements.json. Expanding to the full ~30-row catalog is a
    post-MVP data-entry task and lives in this function so it is
    version-controlled alongside the constants it imports.
    """
    # Local imports so the test suite can call build_manifest() without
    # pulling in the full constants module.
    from scripts import constants as C

    return [
        {
            "id": "alpha_inv", "tier": "hard", "epistemic": "THEOREM",
            "question": "Does the fine structure constant follow from D=3 and varpi?",
            "ftd_value": float(C.ALPHA_INV),
            "formula": "Master quadratic x_+ solution, 7-term series",
            "inputs_used": ["D=3", "varpi (Gauss's lemniscate constant)"],
            "theory_ref": "docs/theory/03_derivations/DERIV_ALPHA_FROM_GSTAR.md",
        },
        {
            "id": "m_higgs", "tier": "parametric", "epistemic": "PARAMETRIC",
            "question": "Is the Higgs mass consistent with the EW scale FTD sets?",
            "ftd_value": float(C.M_HIGGS),
            "formula": "(N_eff / alpha^2) * m_e",
            "formula_source": "SM",
            "ftd_inputs": ["alpha", "N_eff = 3", "m_e (itself from alpha^11)"],
            "theory_ref": "docs/theory/05_particles/DERIV_HIGGS_MASS.md",
        },
        {
            "id": "v_us", "tier": "unpredicted", "epistemic": "OPEN",
            "question": "Does FTD predict the CKM element |V_us|?",
        },
    ]


def main():
    repo_root = Path(__file__).resolve().parents[2]
    measurements_path = repo_root / "engine/web/data/measurements.json"
    output_path = repo_root / "engine/web/data/verify-manifest.json"

    measurements = json.loads(measurements_path.read_text(encoding="utf-8"))
    ftd_rows = _ftd_rows_from_constants()
    build_stamp = {
        "commit": _git_commit(),
        "timestamp": datetime.now(timezone.utc).isoformat(timespec="seconds"),
        "ftd_version": "5.30",
    }
    manifest = build_manifest(
        ftd_rows=ftd_rows, measurements=measurements, build_stamp=build_stamp)
    output_path.write_text(
        json.dumps(manifest, indent=2, sort_keys=False) + "\n",
        encoding="utf-8",
    )
    print(f"wrote {output_path} — {sum(manifest['counts'].values())} rows")


if __name__ == "__main__":
    main()

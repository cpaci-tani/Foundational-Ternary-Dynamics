"""Independent FTD-0639 analytic dynamical-rest certificate."""
from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0639"
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_CONNECTED_BLOCK_ANALYTIC_DYNAMICAL_REST_v1.md"
SHA = "28B9E9415C49FD989A4FBC60B33D9588E8F6A13A52B78E36881A9379F18D8AF3"


def main() -> None:
    assert hashlib.sha256(PROTOCOL.read_bytes()).hexdigest().upper() == SHA
    summary = json.loads((RESULT / "ftd_0639_connected_block_analytic_dynamical_rest_v1.json").read_text())
    arms = list(csv.DictReader((RESULT / "ftd_0639_connected_block_analytic_dynamical_rest_arms_v1.csv").open()))
    ticks = list(csv.DictReader((RESULT / "ftd_0639_connected_block_analytic_dynamical_rest_ticks_v1.csv").open()))
    assert summary["protocol_sha256"] == SHA
    assert summary["verdict"] == "CONNECTED_BLOCK_ANALYTIC_DYNAMICAL_REST_CONSTRUCTIVE"
    assert summary["ticks_each_direction"] == 128
    assert len(arms) == 2 and len(ticks) == 512
    for arm in arms:
        assert all(arm[key] == "1" for key in ("valid", "coverage", "sector_preserved", "no_hops"))
        assert int(arm["max_multiplicity"]) <= 8 and int(arm["total_hops"]) == 0
        assert float(arm["max_impulse"]) <= 1e-9
        assert float(arm["max_state_excursion"]) <= 1e-8
        assert float(arm["max_center_displacement"]) <= 1e-10
        assert float(arm["max_energy_drift"]) <= 1e-12
        assert float(arm["max_common_residual"]) <= 1e-10
        assert float(arm["recovery"]) <= 1e-10
        rows = [row for row in ticks if row["orientation"] == arm["orientation"]]
        assert len(rows) == 256
        assert sum(row["direction"] == "1" for row in rows) == 128
        assert sum(row["direction"] == "-1" for row in rows) == 128
        assert max(float(row["residual"]) for row in rows) == float(arm["max_common_residual"])
    assert float(summary["covariance_residual"]) <= 1e-9
    print("FTD-0639 certificate: 512 common-action records qualify reversible dynamical rest")


if __name__ == "__main__":
    main()

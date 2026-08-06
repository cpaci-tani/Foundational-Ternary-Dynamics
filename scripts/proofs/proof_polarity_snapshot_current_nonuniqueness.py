"""Independent certificate for FTD-0719 snapshot/current non-uniqueness."""
from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0719"
SUMMARY = RESULT / "ftd_0719_polarity_snapshot_current_nonuniqueness_v1.json"
COVARIANCE = RESULT / "ftd_0719_polarity_snapshot_current_covariance_v1.csv"
RUNNER = ROOT / "engine/tests/test_polarity_snapshot_current_nonuniqueness.cpp"
PREREG = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_POLARITY_SNAPSHOT_CURRENT_NONUNIQUENESS_v1.md"
)
PROTOCOL = "DE13969105F196E64C61FC106945B372EBE63DA0230DB30E32526A4BC83E7B77"
HASHES = {
    SUMMARY: "0B0565A11F274A8BFF4D512662BDF570304E429A7AC456806372A199FA6187C7",
    COVARIANCE: "54B28CC7C366A4C85E022BA365CD3980752F1A0E8D99E163F5849881BFE84AC4",
    RUNNER: "18A7B59524A5827B915551DE85F35219CFE21A6679393AB405AF67850483F2CA",
    PREREG: PROTOCOL,
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


for path, expected in HASHES.items():
    assert sha256(path) == expected, path

summary = json.loads(SUMMARY.read_text(encoding="utf-8"))
assert summary["protocol_sha256"] == PROTOCOL
assert summary["verdict"] == (
    "POLARITY_SNAPSHOT_CURRENT_NONUNIQUENESS_THEOREM_WITNESSED"
)
assert summary["production_changed"] is False
for gate in (
    "construction_pass",
    "density_pass",
    "continuity_pass",
    "causal_pass",
    "solenoidal_pass",
    "nonzero_pass",
    "transverse_pass",
    "moment_pass",
    "witness_pass",
    "reversal_pass",
    "cubic_pass",
    "translation_pass",
):
    assert summary[gate] == 1, gate

assert summary["density_before_residual"] <= 1e-12
assert summary["density_after_residual"] <= 1e-12
assert summary["continuity_residual"] <= 1e-12
assert summary["difference_divergence_residual"] <= 1e-12
assert summary["current_moment_residual"] <= 1e-12
assert summary["reversal_residual"] <= 1e-12
assert summary["cubic_covariance_residual"] <= 1e-12
assert summary["translation_covariance_residual"] <= 1e-12
assert summary["current_difference_l2"] > 1e-6
assert summary["curl_difference_l2"] > 1e-6
assert summary["connection_witness"] > 1e-6
assert abs(
    summary["connection_witness"]
    - summary["current_difference_l2"] ** 2
) <= 1e-15

with COVARIANCE.open(newline="", encoding="utf-8") as handle:
    rows = list(csv.DictReader(handle))
assert len(rows) == 24
assert [int(row["rotation"]) for row in rows] == list(range(24))
assert max(float(row["residual"]) for row in rows) <= 1e-12

print("FTD-0719 polarity-snapshot current non-uniqueness certificate: PASS")
print(
    f"continuity={summary['continuity_residual']:.12e} "
    f"div_difference={summary['difference_divergence_residual']:.12e} "
    f"current={summary['current_difference_l2']:.12e} "
    f"curl={summary['curl_difference_l2']:.12e}"
)

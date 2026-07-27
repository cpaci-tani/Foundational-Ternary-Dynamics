"""Independent record certificate for FTD-0603.

The script does not call the C++ solver.  It checks the immutable protocol,
reconstructs every registered phase statistic from the sample CSV, verifies
the locked classification, and proves the zero-mean derivative of the exact
quadratic-coat Peierls polynomial over a centred cell.
"""

from __future__ import annotations

import csv
import hashlib
import json
from fractions import Fraction
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_NEUTRAL_PAIR_TRANSLATION_PHASE_BALANCE_v1.md"
)
RESULT = ROOT / (
    "engine/results/ftd_0603/ftd_0603_translation_phase_balance_v1.json"
)
SAMPLES = ROOT / (
    "engine/results/ftd_0603/ftd_0603_translation_phase_samples_v1.csv"
)
EXPECTED_PROTOCOL = (
    "9C88B2B593C2E31EA08999010E71EF85204ECB3F8C63AA248B7A86A937E16595"
)


def protocol_hash() -> str:
    raw = PREREG.read_bytes()
    prefix = raw[: raw.index(b"`protocol_sha256=")]
    return hashlib.sha256(prefix).hexdigest().upper()


def classify(m8: float, m16: float, m32: float) -> str:
    if m32 <= 1e-8 and (m32 <= 0.5 * m16 or m32 <= 1e-12):
        return "PHASE_BALANCED"
    if (
        m32 > 1e-8
        and abs(m32 - m16) <= 0.1 * m32
        and abs(m16 - m8) <= 0.2 * m16
    ):
        return "SECULAR"
    return "UNRESOLVED"


def exact_peierls_cell_identity() -> tuple[Fraction, Fraction, Fraction]:
    # Q(f)=f^4-f^2/2, so Q'(f)=4f^3-f.
    lo, hi = Fraction(-1, 2), Fraction(1, 2)
    antiderivative = lambda f: f**4 - f**2 / 2
    integral = antiderivative(hi) - antiderivative(lo)
    force_quarter = 4 * Fraction(1, 4) ** 3 - Fraction(1, 4)
    force_minus_quarter = (
        4 * Fraction(-1, 4) ** 3 - Fraction(-1, 4)
    )
    return integral, force_quarter, force_minus_quarter


def main() -> int:
    observed_hash = protocol_hash()
    record = json.loads(RESULT.read_text(encoding="utf-8"))
    with SAMPLES.open(newline="", encoding="utf-8") as handle:
        samples = list(csv.DictReader(handle))

    unique_arms = {
        (int(row["axis"]), int(row["resolution"]), int(row["phase_index"]))
        for row in samples
    }
    reconstructed: dict[int, dict[str, list[float] | float]] = {}
    for resolution in (8, 16, 32):
        matter_means: list[float] = []
        pseudo_means: list[float] = []
        for axis in range(3):
            group = [
                row for row in samples
                if int(row["resolution"]) == resolution
                and int(row["axis"]) == axis
            ]
            matter_means.append(
                sum(float(row["matter_parallel"]) for row in group) / resolution
            )
            pseudo_means.append(
                sum(float(row["pseudomomentum_parallel"]) for row in group)
                / resolution
            )
        reconstructed[resolution] = {
            "matter": matter_means,
            "pseudo": pseudo_means,
            "matter_max": max(abs(value) for value in matter_means),
            "pseudo_max": max(abs(value) for value in pseudo_means),
        }

    recorded_by_n = {item["N"]: item for item in record["resolutions"]}
    means_residual = 0.0
    for resolution in (8, 16, 32):
        observed = reconstructed[resolution]
        saved = recorded_by_n[resolution]
        means_residual = max(
            means_residual,
            *(abs(a - b) for a, b in zip(
                observed["matter"], saved["mean_matter"]
            )),
            *(abs(a - b) for a, b in zip(
                observed["pseudo"], saved["mean_pseudomomentum"]
            )),
        )

    matter_class = classify(*(
        float(reconstructed[n]["matter_max"]) for n in (8, 16, 32)
    ))
    pseudo_class = classify(*(
        float(reconstructed[n]["pseudo_max"]) for n in (8, 16, 32)
    ))
    minimum_inward = min(float(row["inward_impulse"]) for row in samples)
    minimum_separation = min(
        float(row["separation_decrease"]) for row in samples
    )
    negative_n32_x = sum(
        int(row["resolution"]) == 32
        and int(row["axis"]) == 0
        and float(row["inward_impulse"]) <= 1e-10
        for row in samples
    )
    integral, force_quarter, force_minus_quarter = exact_peierls_cell_identity()

    checks = {
        "protocol_hash": observed_hash == EXPECTED_PROTOCOL
            == record["protocol_sha256"],
        "all_samples_present": len(samples) == 168
            and len(unique_arms) == 168,
        "sample_statistics_reconstruct": means_residual <= 1e-18,
        "locked_classifications": matter_class == "UNRESOLVED"
            == record["matter_classification"]
            and pseudo_class == "UNRESOLVED"
            == record["pseudomomentum_classification"],
        "common_action": record["initializer_pass"]
            and record["common_pass"]
            and record["integer_periodicity_pass"]
            and record["worst_common_gate"] <= 1e-12
            and record["worst_integer_periodicity"] <= 1e-12,
        "attraction_not_robust": not record["attraction_robust"]
            and minimum_inward == record["minimum_inward_impulse"]
            and minimum_inward < -1e-10
            and minimum_separation
            == record["minimum_separation_decrease"]
            and minimum_separation < 0
            and negative_n32_x > 0,
        "locked_verdict": record["verdict"]
            == "TRANSLATION_PHASE_ATTRACTION_NOT_ROBUST",
        "exact_peierls_mean": integral == 0,
        "exact_peierls_sign_change": force_quarter < 0 < force_minus_quarter,
        "production_unchanged": record["production_changed"] is False,
    }
    report = {
        "ftd_id": "FTD-0603",
        "protocol_sha256": observed_hash,
        "sample_count": len(samples),
        "sample_mean_reconstruction_residual": means_residual,
        "matter_classification": matter_class,
        "pseudomomentum_classification": pseudo_class,
        "minimum_inward_impulse": minimum_inward,
        "minimum_separation_decrease": minimum_separation,
        "nonattractive_N32_x_phases": negative_n32_x,
        "exact_quadratic_coat_peierls_mean": str(integral),
        "force_sign_at_plus_quarter": str(force_quarter),
        "force_sign_at_minus_quarter": str(force_minus_quarter),
        "phase_robust_attraction_claim_licensed": False,
        "secular_momentum_defect_claim_licensed": False,
        "checks": checks,
        "certificate_pass": all(checks.values()),
    }
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0 if report["certificate_pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())

"""Independent run-record certificate for FTD-0728."""

from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_PERSISTENCE_COVARIANCE_CONVERGENCE_v1.md"
TEST = ROOT / "engine/tests/test_persistence_covariance_convergence.cpp"
JSON_PATH = ROOT / "engine/results/ftd_0728/ftd_0728_persistence_covariance_convergence_v1.json"
CSV_PATH = ROOT / "engine/results/ftd_0728/ftd_0728_persistence_covariance_convergence_v1.csv"

PREREG_SHA256 = "F2C1D17AE3DF79557E784D25C38904241719EB020E5818565FC37BBC4DA76412"
TEST_SHA256 = "F2294329F3DF1F45C8F5F8104A88E55384C76510FAE64D128825F294BAE1ABB5"
JSON_SHA256 = "3E9723FE36E23D07E23685BDEF20C0F07A491ED0C525897D774963C71A080F7D"
CSV_SHA256 = "72621EA4D7E843C915E26A7B065EBDCEDB39C73D04C14E02AFD1290D1F986F42"
VERDICT = "PERSISTENCE_COVARIANCE_PASSES_WITH_INCOMPLETE_CONVERGENCE"
MOMENTA = (0.0060, 0.0095, 0.0120)
PARENT_SPREAD = 1.1065308669344631e-9


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(condition: bool, message: str, checks: list[str]) -> None:
    if not condition:
        raise AssertionError(message)
    checks.append(message)


def close(actual: float, expected: float, tolerance: float = 1e-13) -> bool:
    return abs(actual - expected) <= tolerance * max(1.0, abs(expected))


def main() -> None:
    checks: list[str] = []
    check(sha256(PREREG) == PREREG_SHA256, "protocol hash locked", checks)
    check(sha256(TEST) == TEST_SHA256, "runner hash locked", checks)
    check(sha256(JSON_PATH) == JSON_SHA256, "JSON hash locked", checks)
    check(sha256(CSV_PATH) == CSV_SHA256, "CSV hash locked", checks)

    summary = json.loads(JSON_PATH.read_text(encoding="utf-8"))
    with CSV_PATH.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))

    check(summary["identifier"] == "FTD-0728", "identifier", checks)
    check(summary["protocol_sha256"] == PREREG_SHA256, "embedded protocol hash", checks)
    check(summary["verdict"] == VERDICT, "locked unresolved verdict", checks)
    check(summary["arm_count"] == 208 and len(rows) == 208, "208 locked histories", checks)
    check(summary["executed_arms"] == 208, "all histories executed", checks)
    check(summary["identity_pass_arms"] == 208, "all rowwise identities pass", checks)
    check(summary["inverse_pass_arms"] == 208, "all state-only inverses pass", checks)
    check(summary["recoil_pass_arms"] == 208, "all recoil arms pass", checks)

    directions = {
        "0_0_1", "0_1_-1", "0_1_0", "0_1_1", "1_-1_-1",
        "1_-1_0", "1_-1_1", "1_0_-1", "1_0_0", "1_0_1",
        "1_1_-1", "1_1_0", "1_1_1",
    }
    check({row["direction"] for row in rows} == directions, "13 Moore rays", checks)
    check({row["polarity"] for row in rows} == {"plus_minus", "minus_plus"}, "both polarity orders", checks)
    check({row["translation"] for row in rows} == {"origin", "shifted"}, "translation copies", checks)
    check(all(row["initialized"] == "1" for row in rows), "all initial dresses valid", checks)
    check(all(row["executed"] == "1" for row in rows), "all histories complete", checks)
    check(all(row["identity_pass"] == "1" for row in rows), "rowwise common-action gates", checks)
    check(all(row["inverse_pass"] == "1" for row in rows), "rowwise inverse gates", checks)
    check(all(row["recoil_pass"] == "1" for row in rows), "rowwise recoil gates", checks)

    max_common = max(float(row["max_common_residual"]) for row in rows)
    max_recoil = max(float(row["max_recoil_defect"]) for row in rows)
    max_inverse = max(float(row["inverse_recovery"]) for row in rows)
    max_balance = max(float(row["pair_field_balance"]) for row in rows)
    check(max_common <= 1e-10, "common residual gate", checks)
    check(max_recoil <= 1e-9, "recoil gate", checks)
    check(max_inverse <= 1e-8, "inverse gate", checks)
    check(max_balance <= 1e-8, "pair-plus-field balance gate", checks)
    check(close(max_common, summary["maximum_common_residual"]), "summary common maximum", checks)
    check(close(max_recoil, summary["maximum_recoil_defect"]), "summary recoil maximum", checks)
    check(close(max_inverse, summary["maximum_inverse_recovery"]), "summary inverse maximum", checks)
    check(close(max_balance, summary["maximum_pair_field_balance"]), "summary balance maximum", checks)

    spread = summary["maximum_scalar_history_spread"]
    check(spread <= 1e-9, "locked scalar covariance gate passes", checks)
    check(close(spread, 5.6798055148021831e-10), "scalar spread value locked", checks)
    check(close(summary["parent_scalar_history_spread"], PARENT_SPREAD), "parent spread locked", checks)
    ratio = summary["tight_to_parent_scalar_ratio"]
    check(close(ratio, spread / PARENT_SPREAD), "convergence ratio recomputed", checks)
    check(ratio > 0.2, "fivefold convergence gate fails", checks)
    check(summary["worst_covariance_family"] == "unbound", "worst family", checks)
    check(close(summary["worst_covariance_momentum"], 0.0120), "worst momentum", checks)
    check(summary["worst_covariance_direction"] == "0_1_-1", "worst direction", checks)
    check(summary["worst_covariance_polarity"] == "plus_minus", "worst polarity", checks)
    check(summary["worst_covariance_translation"] == "shifted", "worst translation", checks)
    check(summary["worst_covariance_tick"] == 92, "worst tick", checks)
    check(summary["worst_covariance_component"] == "separation", "worst component", checks)

    unbound = [row for row in rows if row["family"] == "unbound"]
    bound = [row for row in rows if row["family"] == "bound"]
    check(len(unbound) == 156 and len(bound) == 52, "156 unbound and 52 bound", checks)
    check(all(row["bound_control_pass"] == "1" for row in bound), "all bound controls persist", checks)
    check(all(row["tail_persistent"] == "1" for row in bound), "bound tail persistence", checks)
    check(all(row["localized_dressing_96"] == "1" for row in bound), "bound controls remain localized", checks)
    check(all(int(row["graph_transitions"]) == 0 for row in bound), "bound controls retain graph", checks)
    check(all(int(row["active_ticks"]) == 96 for row in bound), "bound controls active 96 ticks", checks)
    check(all(int(row["dynamic_median_radius2_48"]) == 2 for row in bound), "bound radius two at tick 48", checks)
    check(all(int(row["dynamic_median_radius2_96"]) == 2 for row in bound), "bound radius two at tick 96", checks)

    groups = {p: [row for row in unbound if float(row["momentum"]) == p] for p in MOMENTA}
    check(all(len(group) == 52 for group in groups.values()), "52 arms per unbound momentum", checks)
    for p in (0.0060, 0.0095):
        group = groups[p]
        check(all(row["tail_persistent"] == "1" for row in group), f"p={p:.4f} persists", checks)
        check(all(row["negative_sector"] == "1" for row in group), f"p={p:.4f} final negative", checks)
        check(all(int(row["graph_transitions"]) == 1 for row in group), f"p={p:.4f} enters once", checks)
        check(all(int(row["dynamic_median_radius2_48"]) == 3 for row in group), f"p={p:.4f} radius three at tick 48", checks)
        check(all(int(row["dynamic_median_radius2_96"]) >= 5 for row in group), f"p={p:.4f} field extends by tick 96", checks)
        check(all(row["localized_dressing_96"] == "0" for row in group), f"p={p:.4f} localized classifier fails", checks)
        check(all(float(row["final_pair_internal"]) < -1e-6 for row in group), f"p={p:.4f} negative final energy", checks)

    escape = groups[0.0120]
    check(all(row["tail_persistent"] == "0" for row in escape), "p=0.0120 not tail-persistent", checks)
    check(all(row["escape_control_pass"] == "0" for row in escape), "all escape controls contaminated", checks)
    check(all(int(row["graph_transitions"]) == 3 for row in escape), "all escape controls re-enter", checks)
    check(sum(row["negative_sector"] == "1" for row in escape) == 12, "12/52 escape arms finish negative", checks)
    check(all(int(row["dynamic_median_radius2_48"]) == 5 for row in escape), "escape radius five at tick 48", checks)
    check(min(int(row["dynamic_median_radius2_96"]) for row in escape) == 3, "escape radius minimum three at tick 96", checks)
    check(max(int(row["dynamic_median_radius2_96"]) for row in escape) == 12, "escape radius maximum twelve at tick 96", checks)

    check(summary["persistent_parent_arms"] == 104, "summary persistent parent count", checks)
    check(summary["localized_parent_arms"] == 0, "summary localized parent count", checks)
    check(summary["escape_control_pass_arms"] == 0, "summary escape-control count", checks)
    check(summary["negative_sector_unbound_arms"] == 116, "summary final negative count", checks)
    check(summary["bound_control_pass_arms"] == 52, "summary bound-control count", checks)

    summaries = summary["momentum_summaries"]
    check(len(summaries) == 3, "three momentum summaries", checks)
    for p, item in zip(MOMENTA, summaries):
        group = groups[p]
        check(close(item["momentum"], p), f"summary momentum p={p:.4f}", checks)
        check(item["arms"] == 52, f"summary arm count p={p:.4f}", checks)
        check(item["tail_persistent"] == sum(row["tail_persistent"] == "1" for row in group), f"summary persistence p={p:.4f}", checks)
        check(item["localized_dressing_96"] == sum(row["localized_dressing_96"] == "1" for row in group), f"summary localization p={p:.4f}", checks)
        check(item["graph_transitions"] == sum(int(row["graph_transitions"]) for row in group), f"summary transitions p={p:.4f}", checks)
        exports = [float(row["energy_export"]) for row in group]
        finals = [float(row["final_pair_internal"]) for row in group]
        check(close(item["minimum_energy_export"], min(exports)), f"summary export min p={p:.4f}", checks)
        check(close(item["maximum_energy_export"], max(exports)), f"summary export max p={p:.4f}", checks)
        check(close(item["minimum_final_pair_internal"], min(finals)), f"summary final min p={p:.4f}", checks)
        check(close(item["maximum_final_pair_internal"], max(finals)), f"summary final max p={p:.4f}", checks)

    print(f"FTD-0728 certificate: {len(checks)}/{len(checks)} checks PASS")
    print(f"verdict={VERDICT}")
    print(f"scalar_history_spread={spread:.17g}")
    print(f"tight_to_parent_ratio={ratio:.17g}")
    print("persistent_parent=104/104; localized_parent=0/104; escape_control=0/52")
    print("p=0.0120 graph_transitions=3 per arm; final_negative=12/52")


if __name__ == "__main__":
    main()


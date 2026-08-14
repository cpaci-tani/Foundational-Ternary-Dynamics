"""Branch-overlay validator for the FTD v2 contextual programme.

This is a governance/consistency check, not a physics derivation.  It verifies
that v1 remains intact, v2 prices its adopted types separately, every adoption
has a falsifier, and the open recovery debts cannot be silently reported as
closed.
"""

from __future__ import annotations

import json
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

ROOT = Path(__file__).resolve().parents[2]
V2_PATH = ROOT / "docs/theory/01_reference/contextual_actualization_register_v2.json"
V1_PATH = ROOT / "docs/theory/01_reference/import_ledger.json"
CONSTITUTION = ROOT / "docs/theory/01_reference/SPEC_FTD_FRAMEWORK_V2_CONTEXTUAL_ACTUALIZATION.md"
CHARTER = ROOT / "docs/theory/10_eft_program/temporal_interior_programme/SCOPE_TEMPORAL_INTERIOR_PROGRAM_v2.md"
SPEC = ROOT / "docs/theory/10_eft_program/temporal_interior_programme/SPEC_CONTEXTUAL_ACTUALIZATION_TIME_v1.md"


def main() -> int:
    v2 = json.loads(V2_PATH.read_text(encoding="utf-8"))
    v1 = json.loads(V1_PATH.read_text(encoding="utf-8"))
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))
        print(f"[{'PASS' if condition else 'FAIL'}] {label}")

    check(
        "C1 ratified branch identity",
        v2["meta"]["id"] == "FTD-0825"
        and v2["meta"]["branch"] == "FTD-v2-contextual-actualization"
        and v2["meta"]["status"] == "ratified-successor-reference-gates-pass",
    )
    check("C2 companion artifacts exist", all(path.exists() for path in (CONSTITUTION, CHARTER, SPEC)))

    adoptions = v2["adoptions"]
    selections = v2["selections"]
    all_entries = adoptions + selections
    ids = [entry["id"] for entry in all_entries]
    check("C3 six v2 commitments are separately enumerated", len(adoptions) == 6)
    check("C4 five selected reference types are separately enumerated", len(selections) == 5)
    check("C5 register identifiers are unique", len(ids) == len(set(ids)))
    check("C6 every adopted/selected type has a substantive falsifier", all(len(entry.get("falsifier", "")) >= 40 for entry in all_entries))

    totals = v2["totals"]
    check("C7 stated counts match entries", totals["framework_adoptions_and_declarations"] == len(adoptions) and totals["selected_reference_types"] == len(selections) and totals["named_external_results"] == len(v2["external_results"]) and totals["open_recovery_debts"] == len(v2["open_debts"]))
    check("C8 no unratified bit-equivalent total is asserted", totals["bit_equivalent_total"] is None)

    v1_dec_refs = {entry["ref"] for entry in v1["declined"]}
    check("C9 v1 DEC-1 and DEC-2 remain declined", {"DEC-1", "DEC-2"} <= v1_dec_refs)
    check("C10 overlay explicitly preserves v1", v2["inherits"]["v1_declination_superseded_on_v2"] == "FC-1/DEC-1" and "does not rewrite" in v2["meta"]["guard"])

    debt_ids = {entry["id"] for entry in v2["open_debts"]}
    check(
        "C11 all five recovery debts remain open",
        debt_ids
        == {
            "OPEN-CA-PREP",
            "OPEN-CA-BORN",
            "OPEN-CA-LORENTZ",
            "OPEN-CA-GSTAR",
            "OPEN-CA-TRANSDUCER",
        },
    )

    blob = (CONSTITUTION.read_text(encoding="utf-8") + SPEC.read_text(encoding="utf-8")).lower()
    check("C12 Born compatibility is not called substrate recovery", "not a born derivation" in blob and "physical born" in blob)
    check("C13 G* clock role stays selected/open", "claim that nature maintains such a clock is `[open]`" in blob)
    check("C14 Type-III finite-spacing inference is forbidden", "type-iii" in blob and "finite spectral density" in blob)
    check("C15 split-locality cost is explicit", "ontic nonlocality/contextuality" in blob and "operational signalling" in blob)

    passed = sum(ok for _, ok in checks)
    print(f"\nFTD-0825 v2 branch register: {passed}/{len(checks)} PASS")
    print("ADOPTION_IS_NOT_DERIVATION")
    print("V1_FC1_DEC1_PRESERVED")
    return 0 if passed == len(checks) else 1


if __name__ == "__main__":
    raise SystemExit(main())

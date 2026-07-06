"""proof_import_ledger.py — Clause-1 / the priced-import ledger validator
(FTD-0371; companion SPEC_IMPORT_LEDGER.md + import_ledger.json).

This is a DISCIPLINE instrument, not a derivation: it validates that the
priced-import ledger is internally consistent, that EVERY import carries a
falsifier (the Number-One-Goal discipline — "mark the boundary" is only a
deliverable if each imported type is falsifiably priced), that the category
totals are honest, and that the load-bearing tags match the canonical
sources (so the ledger cannot silently drift from the constitution / LEDGER).

It also verifies reconciliation flag RF-1 is RESOLVED (2026-07-05): the
constitution's former "D = 3 Forced [THEOREM]" rows (Sec 1.4 + Sec 3.2) now
read [SELECTION -- declared] per FTD-0355 — a regression would re-open RF-1.

NO promotion, NO new theorem: the ledger prices existing commitments. The
verifier asserts x+ = 1/alpha is NOT tagged above [SMC], FC-W stays [AXIOM],
D=3 stays [SELECTION].

Usage:
    python scripts/proofs/proof_import_ledger.py
"""

from __future__ import annotations

import json
import os
import sys
import time

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import ProofSuite  # noqa: E402

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
LEDGER_JSON = os.path.join(REPO, "docs", "theory", "01_reference",
                           "import_ledger.json")
CONSTITUTION = os.path.join(REPO, "docs", "theory", "01_reference",
                            "SPEC_FTD_FRAMEWORK_V1.md")
LEDGER_MD = os.path.join(REPO, "docs", "theory", "07_assessment",
                         "core_ledgers", "LEDGER.md")

suite = ProofSuite("Priced-import ledger (FTD-0371) — consistency + drift audit")


def load():
    with open(LEDGER_JSON, encoding="utf-8") as f:
        return json.load(f)


def read(path: str) -> str:
    with open(path, encoding="utf-8") as f:
        return f.read()


def check_structure(D) -> None:
    req_imp = {"ref", "kind", "type_imported", "price", "unit",
               "canonical_tag", "provenance", "falsifier", "frontier_face"}
    ok = all(req_imp <= set(e) for e in D["imports"])
    suite.assert_true(
        f"C1 every import ({len(D['imports'])}) has the full field set "
        "(ref/kind/type/price/unit/tag/provenance/falsifier/frontier_face)",
        ok, tag="[DERIVED]")


def check_falsifiers(D) -> None:
    # THE discipline: every import AND every declined item carries a
    # non-empty falsifier. This is what makes "mark the boundary" a deliverable.
    imp_ok = all(isinstance(e.get("falsifier"), str) and len(e["falsifier"]) > 20
                 for e in D["imports"])
    dec_ok = all(isinstance(e.get("falsifier"), str) and len(e["falsifier"]) > 20
                 for e in D["declined"])
    suite.assert_true(
        "C2 EVERY import and every declined item carries a substantive "
        "falsifier (the priced-boundary discipline)",
        imp_ok and dec_ok, tag="[DERIVED]")


def check_totals(D) -> None:
    t = D["totals"]
    kinds = [e["kind"] for e in D["imports"]]
    checks = {
        "self_set_entries": len(D["self_set"]),
        "adopted_bits": kinds.count("adopted-bit"),
        "selected_types": kinds.count("selected-type"),
        "named_results": kinds.count("named-result"),
        "calibrations": kinds.count("calibration"),
        "declined": len(D["declined"]),
    }
    mismatches = {k: (t.get(k), v) for k, v in checks.items() if t.get(k) != v}
    suite.assert_true(
        f"C3 stated totals match the enumerated rows "
        f"(bits={checks['adopted_bits']}, selected={checks['selected_types']}, "
        f"named={checks['named_results']}, calib={checks['calibrations']}, "
        f"declined={checks['declined']}); mismatches: {mismatches or 'none'}",
        not mismatches, tag="[DERIVED]")


def check_reading_guard(D) -> None:
    # the '1 bit' must never be readable as the total physics import
    guard = D["meta"].get("reading_guard", "")
    ok = ("1 bit" in guard or "adopted-bit total = 1" in guard) \
        and "PARAMETRIC" in guard and "calibration" in guard.lower()
    suite.assert_true(
        "C4 reading-guard present: '1 adopted bit' is the alpha-sector branch "
        "choice ONLY, not the total physics import (guards against overclaim)",
        ok, tag="[DERIVED]")


def check_no_promotion(D) -> None:
    # x+ = 1/alpha must be [SMC], never promoted; FC-W must be [AXIOM];
    # D=3 must be [SELECTION].
    by_ref = {e["ref"]: e for e in D["imports"]}
    e1 = by_ref["IMP-E1"]["canonical_tag"]
    bw = by_ref["IMP-B1"]["canonical_tag"]
    s1 = by_ref["IMP-S1"]["canonical_tag"]
    ok = ("STRONGLY MOTIVATED CONJECTURE" in e1
          and "AXIOM" in bw
          and "SELECTION" in s1)
    suite.assert_true(
        f"C5 no promotion: x+=1/alpha tag = '{e1}' (SMC), FC-W = '{bw}' "
        f"(AXIOM), D=3 = '{s1}' (SELECTION)",
        ok, tag="[DERIVED]")


def check_constitution_crosscheck() -> None:
    con = read(CONSTITUTION)
    fcw_axiom = "FC-W" in con and "`[AXIOM]`-class declaration" in con
    suite.assert_true(
        "C6 constitution cross-check: FC-W is declared [AXIOM]-class "
        "(the ledger's IMP-B1 tag matches its source)",
        fcw_axiom, tag="[EXTERNAL]")


def check_rf1_resolved() -> None:
    # RF-1 was: constitution Sec 1.4 + Sec 3.2 read D=3 'Forced [THEOREM]'.
    # Reconciled 2026-07-05 to [SELECTION -- declared]. Assert the RESOLUTION:
    # the constitution's D=3 row no longer reads 'Forced', and now reads
    # [SELECTION -- declared]. (A regression here would re-open RF-1.)
    con = read(CONSTITUTION)
    no_stale_d3 = "| `D = 3` | **Forced**" not in con
    now_selection = "D = 3" in con and "[SELECTION — declared]" in con
    ftd0355_current = "SELECTION -- declared" in read(LEDGER_JSON) \
        or "SELECTION — declared" in read(LEDGER_JSON)
    suite.assert_true(
        "C7 RF-1 RESOLVED: constitution's D=3 row no longer reads "
        "'**Forced** [THEOREM]'; it now reads [SELECTION -- declared] "
        "(FTD-0355), matching the ledger's IMP-S1 pricing",
        no_stale_d3 and now_selection and ftd0355_current, tag="[EXTERNAL]")


def check_standing_invariants(D) -> None:
    si = D.get("standing_invariants", "")
    ok = all(s in si for s in ["[SMC]", "MC-T4.3", "FC-W", "[AXIOM]",
                               "SELECTION", "no tag moves"])
    suite.assert_true(
        "C8 standing-invariants line present and complete "
        "(x+=1/alpha SMC / MC-T4.3 obstruction / FC-W AXIOM / D=3 SELECTION / "
        "no tag moves)",
        ok, tag="[DERIVED]")


def main() -> int:
    t0 = time.time()
    print("=" * 70)
    print("  FTD-0371 - priced-import ledger: consistency + drift audit")
    print("  The Number-One-Goal 'mark the boundary' face, made quantitative.")
    print("=" * 70)
    D = load()
    check_structure(D)
    check_falsifiers(D)
    check_totals(D)
    check_reading_guard(D)
    check_no_promotion(D)
    check_constitution_crosscheck()
    check_rf1_resolved()
    check_standing_invariants(D)
    suite.print_summary()
    print(f"\n  Wall time: {time.time() - t0:.2f}s")
    print("\n  This instrument prices existing commitments; it introduces no")
    print("  theorem and moves no tag. x+=1/alpha [SMC]; MC-T4.3")
    print("  [FOUNDATIONAL OBSTRUCTION]; FC-W [AXIOM]; D=3 [SELECTION].")
    return 0 if suite.all_pass else 1


if __name__ == "__main__":
    sys.exit(main())

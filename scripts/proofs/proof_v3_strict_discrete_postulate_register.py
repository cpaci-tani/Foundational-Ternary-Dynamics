"""Consistency verifier for the ratified FTD-v3 postulate register.

This verifies document/register structure only.  It proves no physical
emergence claim; the owner's ratification act is recorded separately as
FTD-1023.
"""

import json
import re
import sys
from pathlib import Path


sys.stdout.reconfigure(encoding="utf-8")

ROOT = Path(__file__).resolve().parents[2]
REGISTER_PATH = (
    ROOT
    / "docs/theory/01_reference/strict_discrete_common_action_register_v3.json"
)

checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    status = "PASS" if condition else "FAIL"
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{status}] {name}{suffix}")


register = json.loads(REGISTER_PATH.read_text(encoding="utf-8"))
meta = register["meta"]
constitution_path = ROOT / meta["constitution"]
verifier_path = ROOT / meta["verifier"]
constitution = constitution_path.read_text(encoding="utf-8")
postulates = register["postulates"]


check(
    "C1 ratified branch identity",
    meta["branch"] == "FTD-v3-strict-discrete-common-action"
    and meta["status"] == "ratified-successor-constitution-R1-R6-scoped-gates-pass"
    and meta["ledger_row"] == "FTD-1023",
)
check(
    "C2 constitution/register/verifier paths exist",
    constitution_path.exists() and REGISTER_PATH.exists() and verifier_path.exists(),
)
check(
    "C3 v1 and v2 remain unchanged predecessors",
    set(meta["predecessors_unchanged"])
    == {"FTD-v1", "FTD-v2-contextual-actualization"},
)

ids = [p["id"] for p in postulates]
check("C4 exactly five rewritten postulates P1--P5", ids == [f"P{i}" for i in range(1, 6)])
check("C5 postulate ids are unique", len(ids) == len(set(ids)))

required_fields = {
    "id",
    "name",
    "tag",
    "statement",
    "independent_price",
    "failure_condition",
}
missing = {
    p["id"]: sorted(required_fields - set(p))
    for p in postulates
    if required_fields - set(p)
}
check("C6 every postulate has the complete audit contract", not missing, str(missing))
check(
    "C7 every postulate explicitly carries v3 axiom status",
    all("AXIOM" in p["tag"] and "v3" in p["tag"] and "draft" not in p["tag"] for p in postulates),
)
check(
    "C8 every postulate prices at least one independent clause",
    all(p["independent_price"] for p in postulates),
)
check(
    "C9 every postulate has a nonempty failure condition",
    all(p["failure_condition"].strip() for p in postulates),
)

by_id = {p["id"]: p for p in postulates}
check(
    "C10 P1 prices D=3 exactly once",
    "spatial dimension three" in by_id["P1"]["independent_price"]
    and sum(
        "spatial dimension three" in p["independent_price"] for p in postulates
    )
    == 1,
)
check(
    "C11 P2 separates global order from local clocks",
    "global-tick/local-clock distinction" in by_id["P2"]["independent_price"]
    and "not the global tick itself" in by_id["P2"]["statement"],
)
check(
    "C12 P3 enforces finite alphabets and no primitive continuum",
    by_id["P3"]["finite_alphabet_required"] is True
    and "no primitive continuum" in by_id["P3"]["independent_price"],
)
check(
    "C13 P4 is a causal ceiling rather than an active-stencil theorem",
    "maximum site dependency support" in by_id["P4"]["statement"],
)
phi_spec = ROOT / by_id["P5"]["instantiation"]
phi_reference = ROOT / by_id["P5"]["executable_reference"]
check(
    "C14 P5 names the selected exact instantiation",
    by_id["P5"]["instantiated"] is True
    and "INSTANTIATED" in by_id["P5"]["tag"]
    and phi_spec.exists()
    and phi_reference.exists(),
)
check(
    "C15 recovery contract target firewall covers alpha/Born/mass/metric",
    {"physical alpha", "Born weights", "particle masses or catalog identifiers", "continuum metric or lensing targets"}
    <= set(register["closure_contract"]["forbidden_rule_inputs"]),
)
check(
    "C16 P5 contains the explicit many-to-one expiry contract",
    by_id["P5"]["requires_explicit_expiry_map"] is True
    and "many-to-one" in by_id["P5"]["statement"]
    and bool(by_id["P5"]["expiry_map"].strip()),
)
required_targets = {
    "continuous flux and conjugate momentum",
    "non-tautological action or history functional",
    "physical preparation measure and event probabilities",
    "matter and material clocks",
    "electromagnetic field and operational coupling",
    "gravity, light response, and geometry",
}
closure = register["closure_contract"]
check(
    "C17 non-postulate closure contract carries the recovery targets",
    "not an additional physical postulate" in closure["status"]
    and required_targets <= set(closure["required_derived_targets"]),
)

ontic_blob = " ".join(register["ontic_primitives"]).lower()
forbidden_ontic = ("real-valued flux", "latency", "hilbert", "born", "metric")
check(
    "C18 ontic primitive list contains no banned continuum/target type",
    not any(term in ontic_blob for term in forbidden_ontic),
    ontic_blob,
)

not_ontic_blob = " ".join(register["not_ontic_primitives"]).lower()
check(
    "C19 removed primitive types are explicitly quarantined",
    all(term in not_ontic_blob for term in ("flux", "latency", "born", "metric", "alpha")),
)
not_inherited = set(register["not_inherited_as_bedrock"])
check(
    "C20 FC-W, ACT-1, primitive J, and v2 adoptions are not inherited",
    {"FC-W", "ACT-1", "primitive continuous J", "v2 FC-CA1 through FC-CA7"}
    <= not_inherited,
)
check(
    "C21 six explicit ratification blockers R1--R6",
    [row.split()[0] for row in register["ratification_blockers"]]
    == [f"R{i}" for i in range(1, 7)],
)
check(
    "C22 branch guards prohibit promotion and target coding",
    any("No v1 or v2 tag moves" in row for row in register["branch_guards"])
    and any("No physical alpha" in row for row in register["branch_guards"]),
)

heading_ids = re.findall(r"^##\s+\d+\.\s+P([1-5])\s+—", constitution, flags=re.MULTILINE)
check("C23 constitution contains one ordered heading for P1--P5", heading_ids == list("12345"), str(heading_ids))
check(
    "C24 constitution is visibly ratified at scoped R1--R6 gates",
    "RATIFIED SUCCESSOR CONSTITUTION — R1--R6 FORMAL GATES PASS" in constitution
    and "The owner has\nratified the branch as FTD-1023" in constitution
    and "R5 — blocked wave/action recovery" in constitution
    and "R6 — target firewall" in constitution
    and all(register["ratification_status"][f"R{i}"].startswith("closed") for i in range(1, 7)),
)
check(
    "C25 constitution folds expiry and closure without hiding their prices",
    "certified non-injective case of the selected `Phi`" in constitution
    and "NOT A SIXTH POSTULATE" in constitution
    and "full carrier inventory and transition table" in constitution,
)

passed = sum(ok for _, ok, _ in checks)
print(f"\n{passed}/{len(checks)} register checks pass")
raise SystemExit(0 if passed == len(checks) else 1)

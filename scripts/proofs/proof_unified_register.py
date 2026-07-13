"""Verifier for the Unified Axiom Register (FTD-0386, Stage U1).

Recomputes the register's structural invariants against import_ledger.json
(the single source of truth for prices/falsifiers). Introduces no theorem;
checks assembly consistency only.
"""
import json
import sys
from pathlib import Path

sys.stdout.reconfigure(encoding="utf-8")

ROOT = Path(__file__).resolve().parents[2]
REG_PATH = ROOT / "docs/theory/01_reference/unified_axiom_register.json"
LEDGER_PATH = ROOT / "docs/theory/01_reference/import_ledger.json"

checks = []


def check(name, ok, detail=""):
    checks.append((name, ok, detail))
    print(f"[{'PASS' if ok else 'FAIL'}] {name}" + (f" -- {detail}" if detail and not ok else ""))


reg = json.loads(REG_PATH.read_text(encoding="utf-8"))
led = json.loads(LEDGER_PATH.read_text(encoding="utf-8"))

# C1: identity + companion files exist
check(
    "C1 meta identity + companion files",
    reg["meta"]["id"] == "FTD-0386"
    and (ROOT / reg["meta"]["renderer"]).exists()
    and (ROOT / reg["meta"]["verifier"]).exists(),
)

# C2: layer counts (5 postulates, 5 commitments, 4 selected, 3 calibrations, 4 named results, 4 posits)
layers = reg["register"]
counts = {k: len(v) for k, v in layers.items()}
check(
    "C2 layer counts 5/5/4/3/4/4",
    counts
    == {
        "L0_postulates": 5,
        "L1_commitments": 5,
        "L2_selected_types": 4,
        "L3_calibrations": 3,
        "L4_named_results": 4,
        "L5_dynamics_posits": 4,
    },
    str(counts),
)

# C3: L2/L3/L4 ledger_refs equal the import ledger's kind-sets exactly
by_kind = {}
for imp in led["imports"]:
    by_kind.setdefault(imp["kind"], set()).add(imp["ref"])
check(
    "C3 L2 == ledger selected-type set",
    {e["ledger_ref"] for e in layers["L2_selected_types"]} == by_kind.get("selected-type", set()),
)
check(
    "C4 L3 == ledger calibration set",
    {e["ledger_ref"] for e in layers["L3_calibrations"]} == by_kind.get("calibration", set()),
)
check(
    "C5 L4 == ledger named-result set",
    {e["ledger_ref"] for e in layers["L4_named_results"]} == by_kind.get("named-result", set()),
)

# C6: FC-W carries the adopted-bit ref; FC-1/FC-2 carry the declined refs
fc = {e["id"]: e for e in layers["L1_commitments"]}
declined_refs = {d["ref"] for d in led["declined"]}
check(
    "C6 FC-W -> adopted bit; FC-1/FC-2 -> declined lines",
    fc["FC-W"]["ledger_ref"] in by_kind.get("adopted-bit", set())
    and {fc["FC-1"]["ledger_ref"], fc["FC-2"]["ledger_ref"]} == declined_refs,
)

# C7: non-members -- declined match ledger; pending purchases are P6C-* and absent from all layers
reg_ids = {e["id"] for v in layers.values() for e in v}
pend = reg["non_members"]["pending_purchases"]
check(
    "C7 declined match + P6C pending absent from register",
    {d["ledger_ref"] for d in reg["non_members"]["declined"]} == declined_refs
    and all(p["id"].startswith("P6C-") for p in pend)
    and not any(p["id"] in reg_ids for p in pend)
    and all("NOT adopted" in p["status"] for p in pend),
)

# C8: every conditional-statement 'given' resolves to a register entry or a priced import-ledger line
# (rows may consume priced non-axiom lines, e.g. the empirical bridges IMP-E2/E3)
resolvable = (
    reg_ids
    | {e.get("ledger_ref") for v in layers.values() for e in v if e.get("ledger_ref")}
    | {imp["ref"] for imp in led["imports"]}
)
bad_refs = [
    (row["id"], g)
    for row in reg["conditional_statement"]
    for g in row["given"]
    if g not in resolvable
]
check("C8 all 'given' refs resolve to register entries", not bad_refs, str(bad_refs))

# C9: no bare-[THEOREM] tag on any row whose 'given' goes beyond L0+FC-0
core = {"P1", "P2", "P3", "P4", "P5", "FC-0"}
offenders = [
    row["id"]
    for row in reg["conditional_statement"]
    if set(row["given"]) - core and row["tag"].strip() == "[THEOREM]"
]
check("C9 no bare [THEOREM] on beyond-core conditional rows", not offenders, str(offenders))

# C10: reading guard + standing invariants present; banned overclaim phrasing absent
blob = json.dumps(reg).lower()
check(
    "C10 guards present, overclaim absent",
    "adoption is never derivation" in blob
    and "no tag moves" in blob
    and "ftd derives alpha" not in blob
    and "derives the standard model" not in blob,
)

# C11: every entry carries id/name/statement/tag/source
missing = [
    e.get("id", "?")
    for v in layers.values()
    for e in v
    if not all(e.get(k) for k in ("id", "name", "statement", "tag", "source"))
]
check("C11 every register entry fully populated", not missing, str(missing))

n_pass = sum(1 for _, ok, _ in checks if ok)
print(f"\n{n_pass}/{len(checks)} checks pass")
sys.exit(0 if n_pass == len(checks) else 1)

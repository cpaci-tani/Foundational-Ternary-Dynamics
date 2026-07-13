"""Verifier for the Adoption Pricing Rules (FTD-0387, Unification Annex Stage U0).

Recomputes the FC-W calibration, checks the predicate floor equals the FC-W
point, checks the illustrative ordering is monotonic in the yield-to-cost index
y/c with each stated outcome matching the floor rule, and asserts the document
ratifies nothing and adopts nothing. Checks internal consistency of a proposed
decision rule; introduces no theorem, prices no claim.
"""
import json
import sys
from pathlib import Path

sys.stdout.reconfigure(encoding="utf-8")

ROOT = Path(__file__).resolve().parents[2]
DATA = ROOT / "docs/theory/01_reference/adoption_pricing.json"
LEDGER = ROOT / "docs/theory/01_reference/import_ledger.json"

checks = []


def check(name, ok, detail=""):
    checks.append((name, ok))
    print(f"[{'PASS' if ok else 'FAIL'}] {name}" + (f" -- {detail}" if detail and not ok else ""))


d = json.loads(DATA.read_text(encoding="utf-8"))
led = json.loads(LEDGER.read_text(encoding="utf-8"))

# C1: identity + companion files exist
check(
    "C1 meta identity + companion files",
    d["meta"]["id"] == "FTD-0387"
    and (ROOT / d["meta"]["renderer"]).exists()
    and (ROOT / d["meta"]["verifier"]).exists(),
)

# C2: status/tag declare DRAFT + awaiting ratification (no premature authority)
blob = json.dumps(d).lower()
check(
    "C2 declared DRAFT awaiting ratification",
    "draft" in d["meta"]["tag"].lower()
    and "ratif" in d["meta"]["status"].lower()
    and "ratification_gate" in d,
)

# C3: the five import-ledger currencies are all addressed in D5
led_kinds = {imp["kind"] for imp in led["imports"]}  # adopted-bit, selected-type, named-result, calibration, empirical-identification
priced = {row["currency"].split("-proven")[0].split("-open")[0] for row in d["D5_currency"]["bit_equivalents"]}
check(
    "C3 every import-ledger currency addressed by D5",
    led_kinds.issubset(priced),
    f"missing {led_kinds - priced}",
)

# C4: calibration excluded from the predicate (irreducible floor)
calib = next(r for r in d["D5_currency"]["bit_equivalents"] if r["currency"] == "calibration")
check("C4 calibration excluded from predicate (grade-0 floor)", "exclud" in calib["cost"].lower())

# C5: parametric yield weight is exactly 0 (F10 guard)
par = next(w for w in d["D6_predicate"]["yield_weights"] if "PARAMETRIC" in w["kind"])
check("C5 parametric yield weight = 0", par["weight"] == 0.0)

# C6: FC-W calibration recomputes to the marginal point (y == c == 1)
rt = d["D7_fcw_calibration"]
c_fcw = rt["cost_bits"]
y_fcw = rt["yield_gapclasses"] * rt["yield_weight"]
marginal = (y_fcw == c_fcw)
check(
    "C6 FC-W calibration recomputes to y == c (marginal point)",
    c_fcw == 1 and y_fcw == 1 and marginal and "MARGINAL" in rt["verdict"],
    f"c={c_fcw} y={y_fcw}",
)

# C7: predicate floor is anchored to the FC-W point (weakest adoption on record)
check(
    "C7 floor anchored to FC-W point",
    "floor" in (rt["verdict"] + rt["calibration_role"]).lower() and "1 bit" in rt["calibration_role"],
)

# C8: the calibration is consistency-only and adjudicates nothing (tag not moved)
check(
    "C8 calibration confirms FC-W standing, moves no tag",
    "not_an_adjudication" in rt
    and "no tag moves" in rt["consistency"].lower()
    and "fc-w stays [axiom]" in d["standing_invariants"].lower(),
)

# C9: illustrative ranking is monotonic -- rank improves as compression (yield/cost) improves,
# the stated ratio matches the recomputed one, and the verdict matches the floor threshold
cands = d["illustrative_ranking"]["candidates"]
floor = d["illustrative_ranking"]["floor_ratio"]
by_rank = sorted(cands, key=lambda c: c["rank"])
recomputed = [c["yield_est_gapclasses"] / c["cost_est_bits"] for c in by_rank]
ratio_ok = all(abs(c["ratio"] - r) < 1e-9 for c, r in zip(by_rank, recomputed))
monotonic = all(recomputed[i] >= recomputed[i + 1] for i in range(len(recomputed) - 1))


def verdict_ok(c):
    r = c["ratio"]
    v = c["verdict"]
    return (r > floor and v == "pass") or (r == floor and v == "marginal") or (r < floor and v.startswith("fail"))


verdicts_ok = all(verdict_ok(c) for c in cands)
check(
    "C9 ranking monotonic; stated ratio == recomputed; verdict matches floor",
    ratio_ok and monotonic and verdicts_ok,
    f"ratios={recomputed}",
)

# C10: ranking is declared non-binding and covers exactly the five Stage-U2 candidates
ids = {c["id"] for c in cands}
check(
    "C10 ranking non-binding + the five P6C candidates",
    "non-binding" in d["illustrative_ranking"]["status"].lower()
    and ids == {"P6C-G", "P6C-C", "P6C-M", "P6C-F", "P6C-U"},
)

# C11: no overclaim -- adopts nothing, promotes nothing, revises no existing price
check(
    "C11 adopts/promotes/revises nothing",
    "adopts nothing" in d["meta"]["status"].lower()
    and "introduces no claim" in d["standing_invariants"].lower()
    and "revises none of them" in d["meta"]["purpose"].lower(),
)

n = sum(1 for _, ok in checks if ok)
print(f"\n{n}/{len(checks)} checks pass")
sys.exit(0 if n == len(checks) else 1)

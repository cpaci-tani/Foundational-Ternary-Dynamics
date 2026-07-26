"""FTD-0509 -- Import bit-currency reconciliation (frozen gates).

Implements exactly the gates of PREREG_IMPORT_BIT_CURRENCY_RECONCILIATION_v1.md
(git tag preregister-import-bit-currency-v1). No other checks.

Run:  python scripts/proofs/proof_import_bit_currency.py
"""

import json
import math
import os
import sys

LEDGER_JSON = os.path.join("docs", "theory", "01_reference", "import_ledger.json")
GRID = {2: range(1, 64), 8: range(1, 22)}
results = {}


def gate(name, cond, detail=""):
    results[name] = bool(cond)
    print(f"  [{'PASS' if cond else 'FAIL'}] {name}" + (f" -- {detail}" if detail else ""))


def min_capacity_bits(N, m):
    """Minimal integer payload bits that exactly reverse N m-way merges via
    the registered radix control h' = m*h + b (worst-case digits)."""
    # Worst case h after N pushes is m^N - 1; capacity = bits to hold it.
    hmax = m ** N - 1
    return hmax.bit_length()


def g1_rate():
    ok = True
    for m, Ns in GRID.items():
        for N in Ns:
            cap = min_capacity_bits(N, m)
            expected = math.ceil(N * math.log2(m))
            if cap != expected:
                ok = False
                print(f"    deviation: m={m} N={N} cap={cap} expected={expected}")
    gate("G1 capacity = ceil(N*log2(m)) exact across frozen grid", ok)


def g2_unit(ledger):
    imp_b1 = next(i for i in ledger["imports"] if i.get("ref") == "IMP-B1")
    declared = imp_b1["price"]
    derived = math.log2(2)
    ok = (imp_b1["unit"] == "bit") and (declared == derived)
    gate("G2 IMP-B1 declared price == log2(2), conversion constant 1",
         ok, f"declared={declared} bit, derived={derived}")


def g3_scaling(ledger):
    # Price functions over the grid under the derived-cost model.
    def classify(P):
        vals = [P(N) for N in GRID[2]]
        if all(v == vals[0] for v in vals):
            return "intensive"
        slopes = {vals[i + 1] - vals[i] for i in range(len(vals) - 1)}
        return "extensive" if all(s > 0 for s in slopes) else "irregular"

    P_impb1 = lambda N: math.log2(2)          # one global 2-element fiber
    P_dec2 = lambda N: N * math.log2(2)       # per-event section (FTD-0499 S2)
    P_dec1 = lambda N: N * math.log2(2)       # record-capacity floor (FTD-0508 Cor 3)

    c_b1, c_d2, c_d1 = classify(P_impb1), classify(P_dec2), classify(P_dec1)
    print(f"    IMP-B1: {c_b1}; DEC-2 object: {c_d2}; DEC-1 floor: {c_d1}")
    gate("G3 scaling classification computed (intensive/extensive per cell defs)",
         c_b1 in ("intensive", "extensive") and c_d2 in ("intensive", "extensive")
         and c_d1 in ("intensive", "extensive"))
    return c_b1, c_d2, c_d1


def g0_guard(ledger):
    imp_b1 = next(i for i in ledger["imports"] if i.get("ref") == "IMP-B1")
    dec_refs = {d["ref"] for d in ledger["declined"]}
    ok = (imp_b1["price"] == 1 and {"DEC-1", "DEC-2"} <= dec_refs)
    gate("G0 guard: IMP-B1 price untouched; DEC-1/DEC-2 remain declined", ok)


def main():
    print("FTD-0509 import bit-currency reconciliation (frozen gates)")
    ledger = json.load(open(LEDGER_JSON, encoding="utf-8"))
    g0_guard(ledger)
    g1_rate()
    g2_unit(ledger)
    c_b1, c_d2, c_d1 = g3_scaling(ledger)

    all_gates = all(results.values())
    if not all_gates:
        verdict = "C"
    elif c_b1 == "intensive" and c_d2 == "extensive" and c_d1 == "extensive":
        verdict = "B"
    elif c_b1 == c_d2 == c_d1 == "intensive":
        verdict = "A"
    else:
        verdict = "C"
    print(f"\nOUTCOME CELL: {verdict}")
    print(f"gates: {sum(results.values())}/{len(results)} PASS")
    sys.exit(0 if all_gates else 1)


if __name__ == "__main__":
    main()

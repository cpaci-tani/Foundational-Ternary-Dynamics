"""Analyze toggle bisection: which toggle drives which feature?

Compares each "FULL_minus_X" config to FULL_PHYSICS baseline, and each
"DEFAULTS_plus_X" to DEFAULTS_ONLY. Identifies which toggle is responsible
for each observed change in cluster size, color content, matter/antimatter ratio.
"""
from __future__ import annotations
import json
from pathlib import Path

JSON = Path(r"C:/Users/cpaci/Desktop/ftd/toggle_bisection.json")

def diff(baseline, target):
    """Difference: how does target differ from baseline?"""
    return {
        "n_total": target["n_total"] - baseline["n_total"],
        "n_R": target["n_R"] - baseline["n_R"],
        "n_G": target["n_G"] - baseline["n_G"],
        "n_B": target["n_B"] - baseline["n_B"],
        "n_none": target["n_none"] - baseline["n_none"],
        "n_matter": target["n_matter"] - baseline["n_matter"],
        "n_antimatter": target["n_antimatter"] - baseline["n_antimatter"],
    }

def main():
    if not JSON.exists():
        print(f"No data at {JSON}")
        return
    with JSON.open() as f:
        runs = json.load(f)["runs"]
    by_name = {r["config"]: r for r in runs}

    full = by_name.get("FULL_PHYSICS")
    defaults = by_name.get("DEFAULTS_ONLY")
    print(f"FULL_PHYSICS:   n={full['n_total']:3d}  R={full['n_R']:2d} G={full['n_G']:2d} B={full['n_B']:2d}  matter={full['n_matter']} anti={full['n_antimatter']}")
    print(f"DEFAULTS_ONLY:  n={defaults['n_total']:3d}  R={defaults['n_R']:2d} G={defaults['n_G']:2d} B={defaults['n_B']:2d}  matter={defaults['n_matter']} anti={defaults['n_antimatter']}")
    print()

    # Negative bisection: FULL minus one toggle → effect of REMOVING that toggle
    print("=" * 80)
    print("  NEGATIVE BISECTION: full physics MINUS one toggle (effect of removing)")
    print("=" * 80)
    print(f"{'config':<35}{'n':<5}{'dn':<6}{'R':<4}{'G':<4}{'B':<4}{'none':<6}{'M':<4}{'anti':<6}{'effect'}")
    print("-" * 95)
    for r in runs:
        if not r["config"].startswith("FULL_minus_"):
            continue
        toggle = r["config"].replace("FULL_minus_", "")
        d = diff(full, r)
        # Identify effect summary
        effect_parts = []
        if d["n_total"] != 0: effect_parts.append(f"n {'+' if d['n_total']>0 else ''}{d['n_total']}")
        if d["n_R"] != 0: effect_parts.append(f"R{'+' if d['n_R']>0 else ''}{d['n_R']}")
        if d["n_G"] != 0: effect_parts.append(f"G{'+' if d['n_G']>0 else ''}{d['n_G']}")
        if d["n_B"] != 0: effect_parts.append(f"B{'+' if d['n_B']>0 else ''}{d['n_B']}")
        if d["n_antimatter"] != 0:
            effect_parts.append(f"antimatter {'+' if d['n_antimatter']>0 else ''}{d['n_antimatter']}")
        effect = ", ".join(effect_parts) if effect_parts else "no change"
        sign = "+" if d['n_total'] > 0 else ""
        print(f"{r['config']:<35}{r['n_total']:<5}{sign}{d['n_total']:<5}"
              f"{r['n_R']:<4}{r['n_G']:<4}{r['n_B']:<4}{r['n_none']:<6}"
              f"{r['n_matter']:<4}{r['n_antimatter']:<6}{effect}")
    print()

    # Positive bisection: DEFAULTS + one toggle → effect of ADDING that toggle
    print("=" * 80)
    print("  POSITIVE BISECTION: defaults PLUS one toggle (effect of adding)")
    print("=" * 80)
    print(f"{'config':<35}{'n':<5}{'dn':<6}{'R':<4}{'G':<4}{'B':<4}{'none':<6}{'M':<4}{'anti':<6}{'effect'}")
    print("-" * 95)
    for r in runs:
        if not r["config"].startswith("DEFAULTS_plus_"):
            continue
        toggle = r["config"].replace("DEFAULTS_plus_", "")
        d = diff(defaults, r)
        effect_parts = []
        if d["n_total"] != 0: effect_parts.append(f"n {'+' if d['n_total']>0 else ''}{d['n_total']}")
        if d["n_R"] != 0: effect_parts.append(f"R{'+' if d['n_R']>0 else ''}{d['n_R']}")
        if d["n_G"] != 0: effect_parts.append(f"G{'+' if d['n_G']>0 else ''}{d['n_G']}")
        if d["n_B"] != 0: effect_parts.append(f"B{'+' if d['n_B']>0 else ''}{d['n_B']}")
        if d["n_antimatter"] != 0:
            effect_parts.append(f"antimatter{'+' if d['n_antimatter']>0 else ''}{d['n_antimatter']}")
        effect = ", ".join(effect_parts) if effect_parts else "no change"
        sign = "+" if d['n_total'] > 0 else ""
        print(f"{r['config']:<35}{r['n_total']:<5}{sign}{d['n_total']:<5}"
              f"{r['n_R']:<4}{r['n_G']:<4}{r['n_B']:<4}{r['n_none']:<6}"
              f"{r['n_matter']:<4}{r['n_antimatter']:<6}{effect}")
    print()

    # Conclusions
    print("=" * 80)
    print("  ATTRIBUTION")
    print("=" * 80)
    for r in runs:
        if r["config"].startswith("FULL_minus_"):
            d = diff(full, r)
            if abs(d["n_total"]) > 0:
                toggle = r["config"].replace("FULL_minus_", "")
                action = "REDUCES" if d["n_total"] < 0 else "INCREASES"
                print(f"  {toggle:<20} (when present) {action} cluster size by {abs(d['n_total'])}")
            elif d["n_antimatter"] != 0:
                toggle = r["config"].replace("FULL_minus_", "")
                action = "ENABLES" if d["n_antimatter"] < 0 else "SUPPRESSES"
                print(f"  {toggle:<20} (when present) {action} antimatter by {abs(d['n_antimatter'])}")

if __name__ == "__main__":
    main()

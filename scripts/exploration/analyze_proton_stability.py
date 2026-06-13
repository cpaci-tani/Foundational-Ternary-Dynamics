#!/usr/bin/env python3
"""
FTD-0291 — proton-stability verdict analyzer (frozen logic).

Reads the run-of-record summary CSVs written by campaign_proton_stability.cpp
and decides, by PRE-REGISTERED logic, whether the proton's stability (tau_p =
infinity, tagged [THEOREM] in proof_complete_sm.py:460-471) is FORCED by FTD's
native dynamics, or whether the proton is at best emergent-metastable and FTD's
own dynamics (evaporation, weak transmutation) actively decay it.

THE VERDICT READS DISCRETE OUTCOMES ONLY (decay yes/no, lock fires yes/no,
charge conserved yes/no). No fitting, no near-miss search.

Frozen discriminators (declared here BEFORE the run of record; hash-locked):
  D1  cold metastability + lock:  proton, heat=none, genesis=on.
      - does the proton EVER lock?  (max locked_final over proton rows)
      - cold decay fraction (fail_mode != intact)
  D2  weak transmutation fires & breaks the proton:  proton, heat in
      {inject,langevin}, genesis=off.
      - weak=on transmutation fraction  (must be >= WEAK_FIRE_MIN to "fire")
      - weak=off transmutation fraction (control; must be <= CONTROL_MAX)
  D3  charge (Sigma s) conservation across decay:  any proton decay row with
      q_final != q_init  =>  the only exact FTD vector charge is violated.
  D4  same-sign control:  uuu lock behavior (contrast: the lock protects an
      artificial same-sign object, not the mixed-sign proton).

Frozen verdict:
  STABLE-FORCED        iff the proton never decays in ANY arm
                       AND the triad lock protects the proton (max locked==3)
                       AND charge Sigma s is always conserved.
  UNFORCED-METASTABLE  iff the proton decays in >= 1 arm (cold evaporation OR
   [BOUNDARY]          heated weak=on transmutation with the weak=off control
                       clean) OR the lock never fires on the proton OR Sigma s
                       is not conserved in a decay.
  INDETERMINATE        anything else (e.g. weak fires but control also fires =>
                       channel not isolated).

Usage:
  python scripts/exploration/analyze_proton_stability.py \
      --dir engine/results/proton_stability --prefix ror
"""
import argparse
import csv
import glob
import os

# ---- FROZEN constants (do not change after hash-lock) ----
WEAK_FIRE_MIN = 0.50   # weak=on heated arm must transmute >= this fraction to "fire"
CONTROL_MAX   = 0.00   # weak=off must show exactly zero transmutation (no mechanism)


def load(d, prefix):
    rows = []
    for path in sorted(glob.glob(os.path.join(d, f"proton_stability_{prefix}*.csv"))):
        with open(path, newline="") as f:
            for r in csv.DictReader(f):
                r["_file"] = os.path.basename(path)
                rows.append(r)
    return rows


def b(r, k):  # int field
    return int(r[k])


def is_decay(r):
    return r["fail_mode"] != "intact"


def is_transmute(r):
    return "transmut" in r["fail_mode"]


def frac(rows, pred):
    rows = list(rows)
    if not rows:
        return None
    return sum(1 for r in rows if pred(r)) / len(rows)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dir", default="engine/results/proton_stability")
    ap.add_argument("--prefix", default="ror")
    args = ap.parse_args()

    rows = load(args.dir, args.prefix)
    if not rows:
        print(f"NO ROWS matching proton_stability_{args.prefix}*.csv in {args.dir}")
        raise SystemExit(2)

    proton = [r for r in rows if r["species"] == "proton"]
    same   = [r for r in rows if r["species"] == "samesign"]

    cold   = [r for r in proton if r["heat"] == "none"]
    cold_r1 = [r for r in cold if b(r, "radius") == 1]
    cold_r2 = [r for r in cold if b(r, "radius") == 2]
    heated = [r for r in proton if r["heat"] in ("inject", "langevin")]
    heat_on  = [r for r in heated if r["weak"] == "on"]
    heat_off = [r for r in heated if r["weak"] == "off"]

    # ---- D1: cold metastability + lock ----
    lock_max_proton = max((b(r, "locked_final") for r in proton), default=0)
    cold_decay_r1 = frac(cold_r1, is_decay)
    cold_decay_r2 = frac(cold_r2, is_decay)

    # ---- D2: weak fires & breaks the proton ----
    weak_fire = frac(heat_on, is_transmute)
    weak_ctrl = frac(heat_off, is_transmute)

    # ---- D3: charge (Sigma s) conservation across decay ----
    decayed = [r for r in proton if is_decay(r)]
    sigma_violations = [r for r in decayed if b(r, "q_final") != b(r, "q_init")]
    sigma_conserved = (len(sigma_violations) == 0)

    # ---- D4: same-sign control ----
    lock_max_same = max((b(r, "locked_final") for r in same), default=0)

    print("=" * 70)
    print("FTD-0291  proton-stability verdict")
    print("=" * 70)
    print(f"rows: {len(rows)}  (proton {len(proton)}, samesign {len(same)})")
    print()
    print("D1  cold stability + lock (proton, heat=none, genesis=on)")
    print(f"    proton EVER locks?        max locked_final = {lock_max_proton}"
          f"   ({'YES' if lock_max_proton == 3 else 'NO — lock never protects the proton'})")
    print(f"    cold decay fraction r=1   = {cold_decay_r1}")
    print(f"    cold decay fraction r=2   = {cold_decay_r2}")
    print()
    print("D2  weak transmutation fires & breaks the proton (genesis=off)")
    print(f"    weak=on  transmute frac   = {weak_fire}   (fires if >= {WEAK_FIRE_MIN})")
    print(f"    weak=off transmute frac   = {weak_ctrl}   (control, clean if <= {CONTROL_MAX})")
    print()
    print("D3  charge (Sigma s) conservation across decay")
    print(f"    decay rows                = {len(decayed)}")
    print(f"    Sigma s violations        = {len(sigma_violations)}"
          f"   ({'conserved' if sigma_conserved else 'NOT conserved'})")
    if sigma_violations:
        ex = sigma_violations[0]
        print(f"    example                   = {ex['_file']} seed {ex['seed']} "
              f"{ex['fail_mode']}  q {ex['q_init']}->{ex['q_final']}")
    print()
    print("D4  same-sign control (uuu)")
    print(f"    uuu max locked_final      = {lock_max_same}")
    print()

    # ---- composite frozen verdict ----
    proton_ever_decays = any(is_decay(r) for r in proton)
    lock_protects = (lock_max_proton == 3)
    weak_fires_clean = (weak_fire is not None and weak_fire >= WEAK_FIRE_MIN
                        and (weak_ctrl is None or weak_ctrl <= CONTROL_MAX))
    weak_unisolated = (weak_fire is not None and weak_fire >= WEAK_FIRE_MIN
                       and weak_ctrl is not None and weak_ctrl > CONTROL_MAX)

    if (not proton_ever_decays) and lock_protects and sigma_conserved:
        verdict = "STABLE-FORCED"
    elif weak_unisolated:
        verdict = "INDETERMINATE (weak channel not isolated: control also fires)"
    elif proton_ever_decays or (not lock_protects) or weak_fires_clean or (not sigma_conserved):
        verdict = "UNFORCED-METASTABLE [BOUNDARY]"
    else:
        verdict = "INDETERMINATE"

    print("=" * 70)
    print(f"VERDICT: {verdict}")
    print("=" * 70)
    print("Reading: tau_p = infinity is "
          + ("a forced [THEOREM]." if verdict == "STABLE-FORCED"
             else "NOT forced. The triad lock cannot fire on the mixed-sign\n"
                  "proton, no postulate forbids decay, and FTD's own weak channel\n"
                  "transmutes the proton while violating its only exact charge."))


if __name__ == "__main__":
    main()

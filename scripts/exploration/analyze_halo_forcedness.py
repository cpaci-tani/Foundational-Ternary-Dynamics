#!/usr/bin/env python3
"""
FTD-0290 - halo-exponent forcedness analyzer (frozen BOUNDARY verdict).

Is the dark-matter (lossless, selective-ON) halo exponent p (|J|(r) ~ norm*r^p over
r in [7,23]) a FORCED geometric invariant, or a finite-size / regime artifact? This
mirrors the FTD-0269 "shape forced vs calibration tuned" methodology.

Two regimes, across the frozen L-grid {64,96,128,160}:
  - LOSSLESS  (selective ON)  = the §4.2 dark-matter mechanism.
  - DAMPED    (selective OFF) = uniform-damped near-field, the forced-control.

Localization ratio C = r_eff / (L/2): LOCALIZED if C < 0.5, BOX-FILLING if C > 0.8.

The VERDICT reads only the exponent p and the ratio C; amplitude (norm, J_peak,
E_field) is report-only. Reads the SUMMARY CSV (one row per cell).

FROZEN constants (declared before the run of record; do not change post hoc):
"""
import argparse
import csv
import math
import sys
from collections import defaultdict
from pathlib import Path

# ---- FROZEN verdict constants (pre-registered) ----------------------------
R_LO, R_HI = 7, 23        # frozen radial fit window (fit is in the campaign)
L_CONV_TOL = 0.10         # |p(Lmax) - p(Lmax-1)| convergence tolerance
LOCALIZED = 0.5           # C < this  => localized envelope
BOXFILL = 0.8             # C > this  => fills the periodic box
R2_MIN = 0.95             # minimum fit quality to treat p as a power law
SHAPE_TOL = 0.10          # R2 shape sub-check tolerance
DOC_PEXP = -0.69          # the canonical doc value (reported, not a gate)
# ---------------------------------------------------------------------------

DEF_TOL = 1e-6


def _f(row, key, d=float("nan")):
    try:
        return float(row[key])
    except (KeyError, ValueError, TypeError):
        return d


def near(a, b, tol=DEF_TOL):
    return abs(a - b) <= tol * max(1.0, abs(b))


def regime_table(det, selective, dmp0, gc0, kb0):
    """Per-L (p, r2, r_eff, C) for one regime at default constants / full stencil."""
    rows = [r for r in det if r.get("selective") == selective
            and r.get("stencil") == "full"
            and near(_f(r, "DAMPING"), dmp0) and near(_f(r, "G_C"), gc0)
            and near(_f(r, "K_B"), kb0)]
    out = []
    for r in sorted(rows, key=lambda r: _f(r, "L")):
        L = int(_f(r, "L")); reff = _f(r, "r_eff")
        out.append({"L": L, "p": _f(r, "exponent"), "r2": _f(r, "r2"),
                    "r_eff": reff, "C": reff / (L / 2.0) if L else float("nan")})
    return out


def conv(tbl):
    return abs(tbl[-1]["p"] - tbl[-2]["p"]) if len(tbl) >= 2 else float("nan")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--csv", required=True)
    ap.add_argument("--shells", default=None)
    args = ap.parse_args()
    path = Path(args.csv)
    if not path.exists():
        print(f"no CSV at {path}"); return 1
    rows = list(csv.DictReader(open(path, encoding="utf-8")))
    det = [r for r in rows if r.get("arm") == "det"]
    if not det:
        print("no det rows"); return 1

    # defaults = modal (DAMPING,G_C,K_B) among full-stencil det rows
    base_pool = [r for r in det if r.get("stencil") == "full"]
    dmp0 = _f(base_pool[0], "DAMPING"); gc0 = _f(base_pool[0], "G_C"); kb0 = _f(base_pool[0], "K_B")

    lossless = regime_table(det, "on", dmp0, gc0, kb0)
    damped = regime_table(det, "off", dmp0, gc0, kb0)

    print("=" * 74)
    print("FTD-0290 - halo-exponent forcedness analysis (BOUNDARY verdict)")
    print("=" * 74)
    print(f"frozen: window r in [{R_LO},{R_HI}], L-conv tol {L_CONV_TOL}, "
          f"localized C<{LOCALIZED}, box-fill C>{BOXFILL}; doc p={DOC_PEXP}")

    def show(name, tbl):
        print(f"\n{name}:")
        if not tbl:
            print("  (no cells)"); return
        for r in tbl:
            kind = "localized" if r["C"] < LOCALIZED else ("box-fill" if r["C"] > BOXFILL else "mixed")
            print(f"  L={r['L']:4d}  p={r['p']:+.4f}  R²={r['r2']:.4f}  "
                  f"r_eff={r['r_eff']:6.2f}  C={r['C']:.2f} ({kind})")
        if len(tbl) >= 2:
            print(f"  |p({tbl[-1]['L']}) - p({tbl[-2]['L']})| = {conv(tbl):.4f}")

    show("LOSSLESS (selective ON = dark-matter halo, §4.2)", lossless)
    show("DAMPED (selective OFF = forced-control near-field)", damped)

    # ---- R0: forced-control (damped regime localized + convergent) ------
    r0 = None
    if len(damped) >= 2:
        loc_ok = all(r["C"] < LOCALIZED for r in damped[-2:])
        conv_ok = conv(damped) <= L_CONV_TOL
        r0 = loc_ok and conv_ok
        print(f"\n(R0) FORCED-CONTROL: damped localized(last2)={loc_ok}, "
              f"conv={conv(damped):.4f}<= {L_CONV_TOL} -> {'PASS' if r0 else 'FAIL'}")

    # ---- R1: lossless dark-matter halo gate -----------------------------
    r1 = None
    if len(lossless) >= 2:
        loc = all(r["C"] < LOCALIZED for r in lossless[-2:])
        fill = all(r["C"] > BOXFILL for r in lossless[-2:])
        cv = conv(lossless)
        if loc and cv <= L_CONV_TOL:
            r1 = "HALO-FORCED"
        elif fill and cv > L_CONV_TOL:
            r1 = "HALO-TUNED"
        else:
            r1 = "INDETERMINATE"
        print(f"\n(R1) LOSSLESS GATE: localized(last2)={loc}, box-fill(last2)={fill}, "
              f"conv={cv:.4f} -> {r1}")

    # ---- R2: shape sub-check on the forced (damped) regime --------------
    print("\n(R2) SHAPE SUB-CHECK (damped/forced regime; stencil + DAMPING at fixed L):")
    base_off = next((r for r in det if r.get("selective") == "off" and r.get("stencil") == "full"
                     and near(_f(r, "DAMPING"), dmp0) and int(_f(r, "L", -1)) == 96), None)
    if base_off is None:
        base_off = next((r for r in damped[::-1]), None) and \
            next((r for r in det if r.get("selective") == "off" and r.get("stencil") == "full"), None)
    p_off_base = _f(base_off, "exponent") if base_off else float("nan")
    shape_rows = [r for r in det if r.get("selective") == "off"
                  and (r.get("stencil") != "full" or not near(_f(r, "DAMPING"), dmp0))]
    for r in shape_rows:
        why = (f"stencil={r.get('stencil')}" if r.get("stencil") != "full"
               else f"DAMPING={_f(r,'DAMPING'):.4g}")
        dp = _f(r, "exponent") - p_off_base
        print(f"     {why:24s} L={int(_f(r,'L'))}  p={_f(r,'exponent'):+.4f}  dp_vs_base={dp:+.4f}")
    if not shape_rows:
        print("     (no stencil/DAMPING cells present)")

    # ---- Composite ------------------------------------------------------
    print("\n" + "=" * 74)
    if r0 and r1 == "HALO-TUNED":
        verdict = ("HALO-TUNED-BOUNDARY  (dark-matter halo exponent is a finite-size "
                   "box artifact; only the damped Coulomb near-field is forced)")
    elif r1 == "HALO-FORCED":
        verdict = "HALO-FORCED  (lossless halo has a forced localized exponent -> open Step 2)"
    elif r0 is False:
        verdict = "INDETERMINATE  (forced-control R0 failed; instrument cannot resolve)"
    else:
        verdict = f"INDETERMINATE  (R0={r0}, R1={r1})"
    print(f"FTD-0290 VERDICT: {verdict}")
    print("=" * 74)
    print("(No promotions. A BOUNDARY is a map of where the dark-matter claim stops "
          "being forced.)")
    return 0


if __name__ == "__main__":
    sys.exit(main())

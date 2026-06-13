#!/usr/bin/env python3
"""
FTD-0290 - compile-time constant sweep for the halo-exponent forcedness audit.

DAMPING, G_C, K_B are constexpr (not runtime toggles), so the only way to sweep
them is edit-header -> rebuild -> run -> restore (the run_kinetic_drain_sweeps_native
pattern). Each (constant, factor) cell:
  1. scales the constant's definition by `factor`,
  2. rebuilds campaign_halo_forcedness,
  3. runs the DETERMINISTIC baseline cell (single locked particle, L, selective=on,
     stencil=full) tagging the row knob=<constant> value=<factor>,
  4. restores the header (always, in finally).

The campaign records the now-scaled ftd::DAMPING/G_C/K_B into every CSV row, so the
analyzer self-identifies which constant each row varied. Rows APPEND to the same
halo_forcedness_<tag>.csv, so run this AFTER the baseline/L-conv invocations.

DAMPING/G_C are shape controls (verdict-bearing); K_B is the amplitude control
(report-only, the R3 positive check). Restored definitions are byte-identical to
the originals (git diff must be clean after this script).

Usage:
  python scripts/exploration/run_halo_constant_sweeps.py \
      --constants=DAMPING --factors=0.5,2.0 --L=128 --ticks=4000 --tag=v1
  python scripts/exploration/run_halo_constant_sweeps.py \
      --constants=DAMPING,G_C,K_B --factors=0.5,2.0 --L=128 --ticks=4000 --tag=v1
"""
import argparse
import os
import subprocess
import sys

# constant -> (header path, exact definition string, scaled-definition builder)
TARGETS = {
    "DAMPING": (
        "engine/include/ftd/ontic/gauge_couplings.h",
        "inline constexpr double DAMPING = ALPHA;",
        lambda f: f"inline constexpr double DAMPING = ({f}*ALPHA);",
    ),
    "G_C": (
        "engine/include/ftd/ontic/gauge_couplings.h",
        "inline constexpr double G_C = 0.0854245431028543695;",
        lambda f: f"inline constexpr double G_C = ({f}*0.0854245431028543695);",
    ),
    "K_B": (
        "engine/include/ftd/ontic/particle_masses.h",
        "inline constexpr double K_B = 0.511;",
        lambda f: f"inline constexpr double K_B = ({f}*0.511);",
    ),
}

EXE = r".\engine\build\Release\campaign_halo_forcedness.exe"


def edit(path, old, new):
    with open(path, "r", encoding="utf-8") as fh:
        content = fh.read()
    if old not in content:
        raise ValueError(f"target not found in {path}:\n  {old}")
    with open(path, "w", encoding="utf-8") as fh:
        fh.write(content.replace(old, new, 1))


def rebuild():
    print("  [build] campaign_halo_forcedness ...")
    r = subprocess.run(
        "cmake --build engine/build --config Release "
        "--target campaign_halo_forcedness --parallel 32",
        shell=True, capture_output=True, text=True)
    if r.returncode != 0:
        sys.stdout.write(r.stdout[-2000:]); sys.stderr.write(r.stderr[-2000:])
        raise RuntimeError("build failed")
    print("  [build] ok")


def run_cell(const, factor, L, ticks, sor, tag, output_dir, selective):
    cmd = (f"{EXE} --arm=det --L={L} --ticks={ticks} --selective={selective} "
           f"--stencil=full --toggles=minimal --cpu --sor={sor} "
           f"--knob={const} --value={factor} --tag={tag} --output-dir={output_dir}")
    print(f"  [run] {const} x{factor}: {cmd}")
    r = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    sys.stdout.write(r.stdout)
    if r.returncode != 0:
        sys.stderr.write(r.stderr[-2000:])
        raise RuntimeError("run failed")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--constants", default="DAMPING")
    ap.add_argument("--factors", default="0.5,2.0")
    ap.add_argument("--L", type=int, default=128)
    ap.add_argument("--ticks", type=int, default=4000)
    ap.add_argument("--sor", type=int, default=150)
    ap.add_argument("--selective", default="off",
                    help="forced-control regime for the DAMPING sub-check (default off)")
    ap.add_argument("--tag", default="v1")
    ap.add_argument("--output-dir", default="engine/results/halo_forcedness/")
    args = ap.parse_args()

    consts = [c.strip() for c in args.constants.split(",") if c.strip()]
    factors = [float(x) for x in args.factors.split(",") if x.strip()]
    for c in consts:
        if c not in TARGETS:
            print(f"unknown constant {c}; known: {list(TARGETS)}"); return 1

    os.makedirs(args.output_dir, exist_ok=True)
    print(f"=== halo constant sweep: {consts} x {factors}  L={args.L} ticks={args.ticks} ===")

    for const in consts:
        path, old, build_new = TARGETS[const]
        with open(path, "r", encoding="utf-8") as fh:
            original = fh.read()
        try:
            for factor in factors:
                print(f"\n--- {const} x {factor} ---")
                edit(path, old, build_new(factor))
                try:
                    rebuild()
                    run_cell(const, factor, args.L, args.ticks, args.sor,
                             args.tag, args.output_dir, args.selective)
                finally:
                    with open(path, "w", encoding="utf-8") as fh:
                        fh.write(original)  # restore byte-for-byte after each cell
        except Exception as e:
            print(f"ERROR on {const}: {e}")
            with open(path, "w", encoding="utf-8") as fh:
                fh.write(original)
            return 1

    # final rebuild at default constants so the on-disk binary is the unscaled one
    print("\n--- restoring default-constant binary ---")
    rebuild()
    print("\n=== sweep complete; headers restored ===")
    return 0


if __name__ == "__main__":
    sys.exit(main())

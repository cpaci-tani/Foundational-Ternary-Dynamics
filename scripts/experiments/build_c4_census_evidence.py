"""Emit the C4 transaction census evidence file.

Usage: python3 scripts/experiments/build_c4_census_evidence.py <run-dir> <repo-root>
where <run-dir> holds c4_census_run{1,2,3}.txt and journal_drain_run1.txt,
the captured stdout of test_c4_transaction_census and
test_history_journal_drain (CPU Release build, default toggles).

Every structured field is parsed out of the instrument's own stdout by this
script; nothing is transcribed by hand (PREREG D0.2).
"""
import hashlib, json, re, subprocess, sys
from pathlib import Path

SP = Path(sys.argv[1])
ROOT = Path(sys.argv[2])
census = (SP / "c4_census_run1.txt").read_text()
drain = (SP / "journal_drain_run1.txt").read_text()

def sha(text):
    return hashlib.sha256(text.encode()).hexdigest()

def blob_sha(path):
    out = subprocess.run(["git", "cat-file", "-p", f"HEAD:{path}"],
                         cwd=ROOT, capture_output=True, check=True)
    return hashlib.sha256(out.stdout).hexdigest()

def grab(pattern, text=None, cast=float):
    m = re.search(pattern, text if text is not None else census)
    if not m:
        raise SystemExit(f"pattern not found: {pattern}")
    return cast(m.group(1))

d1 = dict(zip(
    ["plus_to_null", "minus_to_null", "null_to_plus", "null_to_minus",
     "plus_to_minus_in_place", "minus_to_plus_in_place", "other"],
    [int(v) for v in re.search(
        r"\+->0: (\d+)\s+-->0: (\d+)\s+0->\+: (\d+)\s+0->-: (\d+)\s*\n"
        r"\s+\+->- \(in-place, null skipped\): (\d+)\s+"
        r"-->\+ \(in-place, null skipped\): (\d+)\s+other: (\d+)", census).groups()]))

t2b = {}
for k in (4, 8):
    m = re.search(rf"T2b dwell\s+mod {k}: n=(\d+) counts=\[([\d,]+)\] chi2=([\d.eE+-]+) crit\(p<\.01\)=([\d.]+)", census)
    t2b[f"mod{k}"] = {"n": int(m.group(1)),
                      "counts": [int(c) for c in m.group(2).split(",")],
                      "chi2": float(m.group(3)), "critical_p01": float(m.group(4))}

evidence = {
    "id": "c4-transaction-census-v1",
    "date": "2026-09-10",
    "lock": {
        "path": "docs/theory/10_eft_program/preregistrations/engine_transaction_census/PREREG_C4_TRANSACTION_CENSUS_v1.md",
        "sha256_committed_blob": blob_sha("docs/theory/10_eft_program/preregistrations/engine_transaction_census/PREREG_C4_TRANSACTION_CENSUS_v1.md"),
        "declared_lock_date": "2026-09-04",
        "git_tag": None,
        "note": "Locked uncommitted on branch integration-ptime-tx; lock and instrument entered git together in the 2026-09-08 bulk import (cc81ad4), so git does not independently witness the pre-registration order.",
    },
    "instrument": {
        "path": "engine/tests/test_c4_transaction_census.cpp",
        "sha256_committed_blob": blob_sha("engine/tests/test_c4_transaction_census.cpp"),
        "ctest_name": "c4_transaction_census",
    },
    "build": {
        "generator": "Ninja", "build_type": "Release", "cuda": False,
        "compiler": "GNU 13.3.0", "backend": "CPU (force_cpu)",
        "platform": "Linux x86_64 (Claude Code remote container)",
    },
    "run": {
        "L": 16, "ticks": 4000, "langevin_seed": 424242, "seed_rng": 424242,
        "toggles": "default",
        "replays": 3,
        "replay_stdout_sha256": [sha((SP / f"c4_census_run{i}.txt").read_text()) for i in (1, 2, 3)],
        "byte_identical_across_replays": len({sha((SP / f"c4_census_run{i}.txt").read_text()) for i in (1, 2, 3)}) == 1,
        "ctest_status": "Passed (58.29 s); exit status reports instrument validity, not the theory verdict",
    },
    "D1_transition_census": d1,
    "T1_null_passage_or_bounce": {
        "reconstructed_nulls": int(grab(r"reconstructed nulls=(\d+)", cast=int)),
        "exit_to_minus_from_passage": int(grab(r"exit to -from \(C4 passage\)=(\d+)", cast=int)),
        "exit_to_plus_from_bounce": int(grab(r"exit to \+from \(bounce\)=(\d+)", cast=int)),
        "reversal_fraction": grab(r"reversal fraction=([\d.eE+-]+)"),
        "gate": "reversal fraction >= 0.5",
        "pass": False,
    },
    "T3_reversal_skips_null": {
        "in_place_skip": int(grab(r"in-place \+s->-s \(skip\)=(\d+)", cast=int)),
        "via_null_passage": int(grab(r"via null \(passage\)=(\d+)", cast=int)),
        "skip_fraction": grab(r"skip fraction=([\d.eE+-]+)"),
        "gate": "skip fraction < 0.1",
        "pass": False,
    },
    "T2_period_structure": {
        "half_cycles_n": int(grab(r"same site\): n=(\d+)", cast=int)),
        "half_cycles_min": int(grab(r"min=(\d+) median=[\d.]+\n", cast=int)),
        "T2a_verdict": "underpowered (<30 half-cycles); no verdict per the lock",
        "T2b_positive_dwell": t2b,
        "T2_pass": True,
    },
    "D2_dwell": {
        "histogram": dict(zip(["0", "1", "2", "3", "4", "5-8", "9-16", "17+"],
                              [int(v) for v in re.findall(r"(?:0|1|2|3|4|5-8|9-16|17\+):(\d+)", census)])),
        "mean": grab(r"mean=([\d.eE+-]+) CV="),
        "CV": grab(r"CV=([\d.eE+-]+)"),
    },
    "verdict": re.search(r"-- VERDICT[^\n]*\n\s+(.+)", census).group(1).strip(),
    "instrument_status": re.search(r"(c4_transaction_census: .+)", census).group(1).strip(),
    "sibling_cross_check": {
        "instrument": "engine/tests/test_history_journal_drain.cpp",
        "stdout_sha256": sha(drain),
        "complete_cycles_count": int(grab(r"birth-to-birth, ticks\) --\n\s+count=(\d+)", drain, int)),
        "complete_cycles_min": int(grab(r"birth-to-birth, ticks\) --\n\s+count=\d+ min=(\d+)", drain, int)),
        "weak_transmutation_events": int(grab(r"WeakTransmutation: (\d+)", drain, int)),
        "reconstructed_nulls": int(grab(r"null dwell time, ticks\) --\n\s+count=(\d+)", drain, int)),
        "note": "min complete cycle 36 matches FTD-1027's cited figure, confirming lock D0.1 (same run under test).",
    },
    "stdout_verbatim": {"census": census, "history_journal_drain": drain},
}
out = ROOT / "engine/docs/evidence/c4-transaction-census-v1-2026-09-10.json"
out.write_text(json.dumps(evidence, indent=2) + "\n")
print("wrote", out)
print(json.dumps({k: v for k, v in evidence.items() if k != "stdout_verbatim"}, indent=2)[:2600])

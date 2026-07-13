"""Pre-registration tag census — the tag-census invariant, mechanized.

Three-way reconciliation (LOCK-STD / charter gate, adopted Arc 1 of the
Consumption Program, FTD-0384):

    git tags (`preregister-*`)  <->  REF_PREREGISTER_MANIFEST.md rows
                                <->  docs/theory tag citations

Classifications per tag string:
  OK               tag exists in git AND appears in the manifest
  ORPHAN-TAG       tag exists in git but has no manifest row
  CITED-NONEXISTENT tag cited somewhere in docs/theory (incl. the manifest)
                   but does NOT resolve in git — the counterfeit signal:
                   a lock that was declared/cited but never cut
  MANIFEST-GHOST   subset of CITED-NONEXISTENT where the citer is the
                   manifest itself

Dispositions: docs/theory/10_eft_program/preregister_census_dispositions.json
maps a tag string to {"status": ..., "note": ..., "date": ...}. A dispositioned
item is reported but does not fail the gate. Legal disposition statuses:
  retracted | re-lock-scheduled | anchored-late | executed-verdict-booked |
  historical-superseded | planned-unlocked-no-verdict

Exit 0 iff every ORPHAN-TAG and CITED-NONEXISTENT item is dispositioned.
The census never reads prose semantics — it reconciles names only. That is
the point: it cannot be gamed by wording.
"""

from __future__ import annotations

import json
import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
MANIFEST = REPO / "docs" / "theory" / "10_eft_program" / "REF_PREREGISTER_MANIFEST.md"
DISPOSITIONS = REPO / "docs" / "theory" / "10_eft_program" / "preregister_census_dispositions.json"
TAG_RE = re.compile(r"preregister-[A-Za-z0-9][A-Za-z0-9-]*[A-Za-z0-9]")


def git_tags() -> set[str]:
    out = subprocess.run(["git", "tag", "-l", "preregister-*"], cwd=REPO,
                         capture_output=True, text=True, check=True).stdout
    return {t.strip() for t in out.splitlines() if t.strip()}


def tags_in_file(path: Path) -> set[str]:
    try:
        return set(TAG_RE.findall(path.read_text(encoding="utf-8", errors="replace")))
    except OSError:
        return set()


def doc_citations() -> dict[str, list[str]]:
    cites: dict[str, list[str]] = defaultdict(list)
    for path in (REPO / "docs" / "theory").rglob("*.md"):
        rel = path.relative_to(REPO).as_posix()
        for tag in tags_in_file(path):
            cites[tag].append(rel)
    return cites


def main() -> int:
    tags = git_tags()
    manifest_tags = tags_in_file(MANIFEST)
    cites = doc_citations()
    dispositions = {}
    if DISPOSITIONS.exists():
        dispositions = json.loads(DISPOSITIONS.read_text(encoding="utf-8"))

    all_names = tags | manifest_tags | set(cites)
    rows = []
    failures = 0
    for name in sorted(all_names):
        in_git = name in tags
        in_manifest = name in manifest_tags
        citers = cites.get(name, [])
        if in_git and in_manifest:
            status = "OK"
        elif in_git and not in_manifest:
            status = "ORPHAN-TAG"
        elif not in_git and in_manifest:
            status = "MANIFEST-GHOST"
        else:
            status = "CITED-NONEXISTENT"
        disp = dispositions.get(name)
        gate_fail = status != "OK" and disp is None
        if gate_fail:
            failures += 1
        rows.append((name, status, "yes" if disp else "-",
                     (disp or {}).get("status", ""), len(citers)))

    width = max(len(r[0]) for r in rows) if rows else 20
    print(f"{'tag':<{width}}  {'status':<18} {'disp?':<6} {'disposition':<26} cites")
    for name, status, has_disp, dstat, ncites in rows:
        marker = "" if (status == "OK" or has_disp == "yes") else "  <-- GATE FAIL"
        print(f"{name:<{width}}  {status:<18} {has_disp:<6} {dstat:<26} {ncites}{marker}")

    n_ok = sum(1 for r in rows if r[1] == "OK")
    print(f"\ntags in git: {len(tags)} | names seen anywhere: {len(all_names)} | "
          f"OK: {n_ok} | non-OK dispositioned: {sum(1 for r in rows if r[1] != 'OK' and r[2] == 'yes')} | "
          f"UNDISPOSITIONED FAILURES: {failures}")
    print("CENSUS: " + ("GREEN" if failures == 0 else "RED"))
    return 0 if failures == 0 else 1


if __name__ == "__main__":
    sys.exit(main())

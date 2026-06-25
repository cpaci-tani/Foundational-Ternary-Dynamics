#!/usr/bin/env python3
"""FTD registry-integrity guardrail.

Purpose: stop the id/FC collisions that nearly recurred in 2026-06 — proposing
`FC-3` when FC-3 was already taken, and reusing `FTD-0309` when 0309-0313 were
already defined. Both happened because an agent trusted the hardcoded
"Next free id" line in CLAUDE.md instead of the corpus.

Rule of use: run this BEFORE minting any new FTD-NNNN id or FC-N commitment.
Trust ITS computed next-free id over any number written in CLAUDE.md.

Scope (deliberately narrow + robust — format-independent):
  * next-free FTD id      = max(referenced across docs/) + 1
  * stale-CLAUDE.md check = does CLAUDE.md's "Next free id" match reality?
  * FC register listing    = which FC-N commitments exist (read before adding one)

Out of scope: duplicate-definition / dangling-cite detection. The LEDGER is a
heterogeneous chronological changelog (ids appear in free prose, and one id may
legitimately lead several sub-rows of a multi-part arc), so reliable detection
needs a semantic read, not a regex. Those were verified clean by the
2026-06-25 three-agent integrity audit; re-run that kind of review to re-verify.

Exit 0 = clean, 1 = stale metadata found.
"""
import re
import sys
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[2]
DOCS = ROOT / "docs"

id_re = re.compile(r"FTD-(\d{4})")
ids = set()
for p in DOCS.rglob("*.md"):
    ids.update(int(m) for m in id_re.findall(p.read_text(errors="ignore")))

max_id = max(ids)
next_free = max_id + 1

fc_re = re.compile(r"FC-(\d)\b")
fcs = sorted({int(x) for p in DOCS.rglob("*.md")
              for x in fc_re.findall(p.read_text(errors="ignore"))})

claude = (ROOT / "CLAUDE.md").read_text(errors="ignore")
m = re.search(r"[Nn]ext free id FTD-(\d{4})", claude)
stale = bool(m) and int(m.group(1)) != next_free

print(f"FTD ids referenced : {len(ids)}  (max FTD-{max_id:04d})")
print(f"NEXT FREE FTD ID   : FTD-{next_free:04d}   <- use this, not CLAUDE.md")
print(f"FC register exists : {['FC-%d' % n for n in fcs]}  <- check before adding FC-N")
if stale:
    print(f"STALE: CLAUDE.md says next-free FTD-{m.group(1)}; actual is FTD-{next_free:04d}")
print("RESULT:", "STALE METADATA" if stale else "clean")
sys.exit(1 if stale else 0)

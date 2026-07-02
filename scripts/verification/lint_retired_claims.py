"""
lint_retired_claims.py -- propagation lint for retired/demoted claim strings.

Greps the LIVE corpus (git-tracked text files, excluding archives, dated
audit records, changelogs, and frozen pre-registrations) for a maintained
blocklist of claim spellings that have been retired, retracted, or demoted
on the record. A hit means a live document still asserts a claim whose
demotion of record never reached it -- exactly the drift class that the
FTD-0351/0356 propagation passes had to clean up by hand.

Blocklist seeded 2026-07-02 (FTD-0360) from that session's demotions:

  bell-violation-live    "Bell violation S = 2sqrt(2)" as a live assertion
                         (canon: S = 2sqrt(2) is imported standard QM
                         conditional on the [SELECTION] singlet; the
                         substrate is local/classical, S <= 2 natively)
  sloop-bell-verified    "sLoop produces Bell violations ... VERIFIED"
                         (RETIRED 2026-07-01, FTD-0347)
  six-theorem-grade      "six theorem-grade" spine count (reconciled
                         2026-07-01 to seven theorem-grade + two tiered,
                         SPEC_ALGEBRAIC_SPINE.md S0 count convention)
  two-three-uniqueness   "(2,3)-uniqueness [THEOREM]" (RETRACTED
                         2026-07-01, FTD-0351; corrected constraint set
                         {a<b<2a} u {b=2a+1} [THEOREM]; (2,3) is a
                         W-CRIT-2-conditional [SELECTION])
  five-fold-convergence  "five-fold independent convergence" (corrected
                         2026-07-01: one structural fact, five
                         vocabularies -- AUDIT_REDTEAM_DISSECTION S2)
  alpha-now-derived      "alpha is now a DERIVED" (no alpha is derived
                         anywhere; x+ = 1/alpha stays [SMC], FTD-0013)
  sigma-old-spelling     -ln(x_-/(x_-+1)) corrupted string-tension formula
                         (corrected 2026-07-01, FTD-0348 to
                         -ln(I1(beta)/I0(beta)) at beta = x_-)
  nyquist-origin         "Nyquist-mode degeneracy origin" as Theorem 7's
                         headline status (superseded 2026-07-01, FTD-0350:
                         [THEOREM at all L >= 2] conditional on the
                         stencil-consistency [SELECTION]; L >= 4 ambiguity
                         closed as a proven masking artifact)

Exceptions live in lint_retired_claims_exceptions.txt next to this script
(format documented there). Every exception is a deliberate, dated decision
-- typically a controller-owned ledger row that records the claim's own
history, a correction banner that quotes the retired spelling, or a
compiled artifact whose regeneration is tracked elsewhere.

Usage:
    python scripts/verification/lint_retired_claims.py

Exit codes:
    0  clean (no unwhitelisted hits)
    1  unwhitelisted hits found (listed on stdout)
    2  environment/usage error

Adding a new retired claim: append a Rule to BLOCKLIST with the demotion
id/date in `ref`, run the lint, then fix or whitelist every hit it finds
in the same commit. Do not delete rules -- retired claims stay linted to
prevent zombie re-emergence ([CLOSED NEGATIVE] discipline).
"""
from __future__ import annotations

import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parents[2]
EXCEPTIONS_FILE = Path(__file__).with_name("lint_retired_claims_exceptions.txt")

# --------------------------------------------------------------------------
# Blocklist
# --------------------------------------------------------------------------

@dataclass(frozen=True)
class Rule:
    rule_id: str
    pattern: re.Pattern
    ref: str  # demotion of record


BLOCKLIST: list[Rule] = [
    Rule(
        "bell-violation-live",
        re.compile(r"Bell violations?\s*\(?\s*S\s*=\s*2\s*(?:√\s*2|\\sqrt\{?2\}?)"),
        "FTD-0023 [SELECTION] / FC-1: imported QM conditional on the singlet; native S<=2",
    ),
    Rule(
        "sloop-bell-verified",
        # The assertion form. The corrected table rows read
        # "...Bell violations** | RETIRED ..." / "...Bell violations} & RETIRED..."
        # and are excluded by the lookahead.
        re.compile(r"sLoop produces Bell violations(?!\**\}?\s*[|&]\s*RETIRED)"),
        "RETIRED 2026-07-01, FTD-0347",
    ),
    Rule(
        "sloop-bell-verified",
        re.compile(r"sLoop[^\n]{0,120}Bell violations?[^\n]{0,120}VERIFIED"),
        "RETIRED 2026-07-01, FTD-0347",
    ),
    Rule(
        "six-theorem-grade",
        re.compile(r"six theorem-grade", re.IGNORECASE),
        "count reconciled 2026-07-01: seven theorem-grade + two tiered (spine S0)",
    ),
    Rule(
        "two-three-uniqueness",
        re.compile(r"\(2,\s*3\)[- ]uniqueness[^\n]{0,60}\[THEOREM\]"),
        "RETRACTED 2026-07-01, FTD-0351",
    ),
    Rule(
        "two-three-uniqueness",
        re.compile(r"\(2,\s*3\) is a \[THEOREM\]"),
        "RETRACTED 2026-07-01, FTD-0351",
    ),
    Rule(
        "five-fold-convergence",
        re.compile(r"five-?fold independent convergence", re.IGNORECASE),
        "corrected 2026-07-01: one structural fact, five vocabularies",
    ),
    Rule(
        "alpha-now-derived",
        re.compile(r"(?:alpha|α|\\alpha)\s+is now a DERIVED", re.IGNORECASE),
        "no alpha is derived anywhere; FTD-0013 stays [SMC]",
    ),
    Rule(
        "sigma-old-spelling",
        re.compile(r"ln\s*\(\s*x_?[-−–]\s*/\s*\(\s*x_?[-−–]\s*\+\s*1\s*\)"),
        "corrupted formula corrected 2026-07-01, FTD-0348",
    ),
    Rule(
        "sigma-old-spelling",
        re.compile(r"\\frac\{x_[-−]\}\{x_[-−]\s*\+\s*1\}"),
        "corrupted formula corrected 2026-07-01, FTD-0348",
    ),
    Rule(
        "nyquist-origin",
        re.compile(r"Nyquist-mode degeneracy origin"),
        "Theorem 7 retagged 2026-07-01, FTD-0350 ([THEOREM at all L >= 2], conditional)",
    ),
]

# --------------------------------------------------------------------------
# Corpus selection
# --------------------------------------------------------------------------

TEXT_EXTENSIONS = {
    ".md", ".qmd", ".tex", ".txt", ".rst",
    ".py", ".js", ".mjs", ".ts", ".html", ".css",
    ".h", ".hpp", ".c", ".cpp", ".cu", ".cuh",
    ".json", ".yaml", ".yml", ".toml", ".ipynb", ".bib",
}

# Path-substring / regex exclusions: NOT part of the live corpus.
EXCLUDE_PATH_SUBSTRINGS = (
    "/archive/",          # archived provenance (retractions quote old claims)
    "archive/",           # top-level archives
    "/audits/",           # dated audit ledgers
    "docs/audits/",       # sweep/audit ledgers
    "/preregistrations/", # frozen pre-registrations (hash-locked)
    ".claude/",           # session worktrees / scratch
)
EXCLUDE_FILENAME_PATTERNS = (
    re.compile(r"^PREREG_"),                 # frozen pre-registrations
    re.compile(r"^CHANGELOG"),               # append-only history
    re.compile(r"^AUDIT_.*20\d\d-\d\d"),     # dated audit records
    re.compile(r"^lint_retired_claims"),     # this lint + its exceptions file
)


def tracked_files() -> list[Path]:
    out = subprocess.run(
        ["git", "ls-files", "-z"],
        cwd=REPO_ROOT, capture_output=True, check=True,
    ).stdout.decode("utf-8", errors="replace")
    files = []
    for rel in out.split("\0"):
        if not rel:
            continue
        p = Path(rel)
        if p.suffix.lower() not in TEXT_EXTENSIONS:
            continue
        posix = p.as_posix()
        if any(sub in posix or posix.startswith(sub) for sub in EXCLUDE_PATH_SUBSTRINGS):
            continue
        if any(pat.search(p.name) for pat in EXCLUDE_FILENAME_PATTERNS):
            continue
        files.append(p)
    return files


# --------------------------------------------------------------------------
# Exceptions
# --------------------------------------------------------------------------

def load_exceptions() -> dict[tuple[str, str], str]:
    """Return {(rule_id, posix_path): reason}."""
    exceptions: dict[tuple[str, str], str] = {}
    if not EXCEPTIONS_FILE.exists():
        return exceptions
    for lineno, raw in enumerate(
        EXCEPTIONS_FILE.read_text(encoding="utf-8").splitlines(), 1
    ):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        parts = [p.strip() for p in line.split("|", 2)]
        if len(parts) != 3:
            print(f"ERROR: malformed exception at line {lineno}: {raw!r}", file=sys.stderr)
            sys.exit(2)
        rule_id, path, reason = parts
        exceptions[(rule_id, path)] = reason
    return exceptions


# --------------------------------------------------------------------------
# Main
# --------------------------------------------------------------------------

def main() -> int:
    exceptions = load_exceptions()
    used_exceptions: set[tuple[str, str]] = set()
    hits: list[tuple[str, int, str, str]] = []  # (path, lineno, rule_id, excerpt)

    files = tracked_files()
    for path in files:
        try:
            text = (REPO_ROOT / path).read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        posix = path.as_posix()
        for rule in BLOCKLIST:
            for m in rule.pattern.finditer(text):
                key = (rule.rule_id, posix)
                if key in exceptions:
                    used_exceptions.add(key)
                    continue
                lineno = text.count("\n", 0, m.start()) + 1
                line = text.splitlines()[lineno - 1].strip()
                excerpt = line[:160] + ("..." if len(line) > 160 else "")
                hits.append((posix, lineno, rule.rule_id, excerpt))

    # De-duplicate (same line can match twice via sibling patterns of one rule)
    hits = sorted(set(hits))

    print(f"lint_retired_claims: scanned {len(files)} tracked text files, "
          f"{len(BLOCKLIST)} blocklist patterns, {len(exceptions)} exceptions")

    stale = set(exceptions) - used_exceptions
    if stale:
        print(f"\nNOTE: {len(stale)} exception(s) no longer match anything "
              f"(candidates for removal):")
        for rule_id, path in sorted(stale):
            print(f"  - {rule_id} | {path}")

    if not hits:
        print("\nCLEAN: no unwhitelisted retired-claim strings in the live corpus.")
        return 0

    print(f"\nFAIL: {len(hits)} unwhitelisted hit(s):\n")
    rules_by_id = {}
    for r in BLOCKLIST:
        rules_by_id.setdefault(r.rule_id, r.ref)
    for posix, lineno, rule_id, excerpt in hits:
        print(f"  {posix}:{lineno}: [{rule_id}] {excerpt}")
    print("\nEach hit asserts a claim retired on the record:")
    for rule_id in sorted({h[2] for h in hits}):
        print(f"  [{rule_id}] -> {rules_by_id[rule_id]}")
    print("\nFix the document to the demotion of record, or add a dated "
          "exception with a reason to lint_retired_claims_exceptions.txt.")
    return 1


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Generate a markdown briefing of recent FTD theory corpus changes.

Read-only: no numerical searches, no claim promotion.
"""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from collections import defaultdict
from datetime import datetime, timezone
from pathlib import Path

# Paths (relative to repo root) watched for canonical changes.
CANONICAL_PATTERNS = (
    "docs/theory/07_assessment/core_ledgers/LEDGER.md",
    "docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md",
    "docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md",
    "docs/theory/01_reference/SPEC_FTD_FRAMEWORK_V1.md",
    "docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md",
    "docs/theory/01_reference/SPEC_DOCTRINE_LEDGER.md",
)

WATCH_PATHS = (
    "docs/theory",
    "docs/WHERE_WE_LEFT_OFF.md",
    "CLAUDE.md",
)

WWLO_UPDATE_RE = re.compile(
    r"^\*\*(?:Latest update|Prior latest update):\*\*",
    re.MULTILINE,
)


def find_repo_root(start: Path) -> Path:
    """Walk up from start until a .git directory is found."""
    current = start.resolve()
    for _ in range(12):
        if (current / ".git").is_dir():
            return current
        if current.parent == current:
            break
        current = current.parent
    raise SystemExit(f"error: no git repository found from {start}")


def run_git(root: Path, *args: str) -> str:
    result = subprocess.run(
        ["git", "-C", str(root), *args],
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        raise SystemExit(f"git failed ({result.returncode}): {' '.join(args)}\n{result.stderr}")
    return result.stdout


def parse_since(value: str) -> str:
    """Accept git --since forms: 7d, 2 weeks, 2026-06-01."""
    return value.strip()


def extract_wwlo_headlines(root: Path, max_lines: int = 5) -> list[str]:
    path = root / "docs" / "WHERE_WE_LEFT_OFF.md"
    if not path.is_file():
        return [f"(missing: {path.relative_to(root)})"]

    text = path.read_text(encoding="utf-8", errors="replace")
    matches = list(WWLO_UPDATE_RE.finditer(text))
    if not matches:
        # Fallback: first non-empty lines after title.
        lines = [ln.strip() for ln in text.splitlines() if ln.strip()]
        return lines[1 : 1 + max_lines]

    headlines: list[str] = []
    for i, match in enumerate(matches[:max_lines]):
        start = match.start()
        end = matches[i + 1].start() if i + 1 < len(matches) else len(text)
        block = text[start:end].strip()
        # Collapse internal newlines to keep one headline per update.
        one_line = re.sub(r"\s+", " ", block)
        headlines.append(one_line)
    return headlines


def git_log_commits(root: Path, since: str, max_commits: int) -> list[dict]:
    fmt = "%H%x1f%ci%x1f%s"
    raw = run_git(
        root,
        "log",
        f"--since={since}",
        f"--max-count={max_commits}",
        f"--pretty=format:{fmt}",
        "--name-only",
        "--",
        *WATCH_PATHS,
    ).strip()

    if not raw:
        return []

    commits: list[dict] = []
    blocks = re.split(r"\n(?=[0-9a-f]{40}\x1f)", raw)
    for block in blocks:
        block = block.strip()
        if not block:
            continue
        lines = block.splitlines()
        header = lines[0]
        parts = header.split("\x1f")
        if len(parts) < 3:
            continue
        commit_hash, date_str, subject = parts[0], parts[1], parts[2]
        files = [ln.strip() for ln in lines[1:] if ln.strip()]
        commits.append(
            {
                "hash": commit_hash[:12],
                "date": date_str[:10],
                "subject": subject,
                "files": files,
            }
        )
    return commits


def normalize_path(path: str) -> str:
    return path.replace("\\", "/")


def is_canonical(path: str) -> bool:
    p = normalize_path(path)
    if p in CANONICAL_PATTERNS:
        return True
    if "/INDEX_" in p and p.startswith("docs/theory/"):
        return True
    return False


def collect_changed_files(commits: list[dict]) -> dict[str, set[str]]:
    by_file: dict[str, set[str]] = defaultdict(set)
    for commit in commits:
        for f in commit["files"]:
            by_file[normalize_path(f)].add(commit["hash"])
    return by_file


def detect_additions_renames(root: Path, since: str) -> list[str]:
    raw = run_git(
        root,
        "log",
        f"--since={since}",
        "--diff-filter=AR",
        "--name-only",
        "--pretty=format:",
        "--",
        "docs/theory",
    )
    paths = sorted({normalize_path(ln.strip()) for ln in raw.splitlines() if ln.strip()})
    return paths


def suggest_reads(
    changed_files: dict[str, set[str]],
    canonical_changed: list[str],
    commits: list[dict],
) -> list[str]:
    suggestions: list[str] = []
    seen: set[str] = set()

    def add(path: str) -> None:
        if path not in seen:
            seen.add(path)
            suggestions.append(path)

    add("docs/WHERE_WE_LEFT_OFF.md")

    for path in canonical_changed:
        add(path)

    # Analysis / pre-reg from recent commits.
    for commit in commits:
        for f in commit["files"]:
            p = normalize_path(f)
            if p.startswith("docs/theory/") and (
                "/ANALYSIS_" in p or "/PREREG_" in p or "/AUDIT_" in p
            ):
                add(p)

    # Cluster INDEX for touched clusters.
    clusters = sorted(
        {
            p.split("/")[2]
            for p in changed_files
            if p.startswith("docs/theory/") and len(p.split("/")) > 2
        }
    )
    for cluster in clusters[:3]:
        index_glob = f"docs/theory/{cluster}/INDEX_"
        for p in changed_files:
            if p.startswith(index_glob):
                add(p)

    if "docs/theory/07_assessment/core_ledgers/LEDGER.md" not in seen:
        add("docs/theory/07_assessment/core_ledgers/LEDGER.md")

    return suggestions[:5]


def render_markdown(
    root: Path,
    since: str,
    wwlo: list[str],
    commits: list[dict],
    added: list[str],
    canonical_changed: list[str],
    suggestions: list[str],
    changed_files: dict[str, set[str]],
) -> str:
    now = datetime.now(timezone.utc).strftime("%Y-%m-%d %H:%M UTC")
    lines = [
        "# FTD Theory Briefing",
        "",
        f"Generated: {now}",
        f"Repo: `{root}`",
        f"Window: `--since {since}`",
        "",
        "## Session Headline",
        "",
    ]
    if wwlo:
        for item in wwlo:
            lines.append(f"- {item}")
    else:
        lines.append("- (no WHERE_WE_LEFT_OFF headlines found)")

    lines.extend(["", "## Recent Theory Commits", ""])
    if not commits:
        lines.append(f"_No commits touching watched paths since {since}._")
    else:
        for c in commits:
            lines.append(f"### `{c['hash']}` ({c['date']}) — {c['subject']}")
            for f in c["files"][:12]:
                flag = " **[canonical]**" if is_canonical(f) else ""
                lines.append(f"- `{f}`{flag}")
            if len(c["files"]) > 12:
                lines.append(f"- _…and {len(c['files']) - 12} more files_")
            lines.append("")

    lines.extend(["## Canonical File Changes", ""])
    if canonical_changed:
        for p in canonical_changed:
            hashes = ", ".join(sorted(changed_files.get(p, [])))
            lines.append(f"- `{p}` (commits: {hashes})")
    else:
        lines.append("_No canonical status-bearing files changed in window._")

    lines.extend(["", "## New / Renamed Under docs/theory", ""])
    if added:
        for p in added[:20]:
            lines.append(f"- `{p}`")
        if len(added) > 20:
            lines.append(f"- _…and {len(added) - 20} more_")
    else:
        lines.append("_None detected._")

    lines.extend(["", "## Suggested Reads", ""])
    for i, p in enumerate(suggestions, 1):
        lines.append(f"{i}. `{p}`")

    lines.extend(
        [
            "",
            "---",
            "_Read-only briefing. Claim tags remain authoritative in LEDGER.md._",
            "",
        ]
    )
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--root",
        type=Path,
        default=None,
        help="FTD repository root (default: auto-detect from script location)",
    )
    parser.add_argument(
        "--since",
        default="7d",
        help="Git --since window (default: 7d)",
    )
    parser.add_argument(
        "--max-commits",
        type=int,
        default=20,
        help="Maximum commits to include (default: 20)",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=None,
        help="Optional path to write briefing (e.g. docs/internal/theory_briefing_latest.md)",
    )
    args = parser.parse_args()

    if args.root is None:
        # scripts/theory/sync_theory_briefing.py -> repo root is two levels up.
        args.root = find_repo_root(Path(__file__).resolve().parent.parent.parent)
    else:
        args.root = find_repo_root(args.root.resolve())

    since = parse_since(args.since)
    wwlo = extract_wwlo_headlines(args.root)
    commits = git_log_commits(args.root, since, args.max_commits)

    changed_files = collect_changed_files(commits)
    canonical_changed = sorted(
        p for p in changed_files if is_canonical(p)
    )
    added = detect_additions_renames(args.root, since)
    suggestions = suggest_reads(changed_files, canonical_changed, commits)

    report = render_markdown(
        args.root,
        since,
        wwlo,
        commits,
        added,
        canonical_changed,
        suggestions,
        changed_files,
    )

    if args.output:
        out_path = args.output if args.output.is_absolute() else args.root / args.output
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(report, encoding="utf-8")
        print(f"wrote {out_path}", file=sys.stderr)

    # Windows consoles may default to cp1252; force UTF-8 for briefing output.
    if hasattr(sys.stdout, "reconfigure"):
        try:
            sys.stdout.reconfigure(encoding="utf-8")
        except Exception:
            pass
    print(report)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

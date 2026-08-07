"""Guards for TRACKER_OPEN_ITEMS_INDEX.md (2026-08-06).

Mirrors test_ledger_index.py's core guards for LEDGER_INDEX.md: the index
must stay a faithful, in-sync rebuild of TRACKER_OPEN_ITEMS.md, and must
never silently drop an item heading.
"""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))

from theory.build_open_items_index import parse_tracker  # noqa: E402

TRACKER = ROOT / "docs" / "theory" / "07_assessment" / "core_ledgers" / "TRACKER_OPEN_ITEMS.md"
INDEX = ROOT / "docs" / "theory" / "07_assessment" / "core_ledgers" / "TRACKER_OPEN_ITEMS_INDEX.md"
BUILDER = ROOT / "scripts" / "theory" / "build_open_items_index.py"


@pytest.fixture(scope="module")
def tracker_text() -> str:
    return TRACKER.read_text(encoding="utf-8")


def test_index_is_in_sync() -> None:
    """TRACKER_OPEN_ITEMS_INDEX.md must be a faithful rebuild of the current tracker."""
    result = subprocess.run(
        [sys.executable, str(BUILDER), "--check"],
        capture_output=True, text=True, cwd=str(ROOT),
    )
    assert result.returncode == 0, (
        f"index out of sync (exit {result.returncode}):\n"
        f"STDOUT: {result.stdout}\nSTDERR: {result.stderr}"
    )


def test_every_item_heading_is_parsed(tracker_text: str) -> None:
    """No `### ` item heading may be silently skipped."""
    raw_headings = re.findall(r"^### (.+)$", tracker_text, re.MULTILINE)
    items = parse_tracker(tracker_text)
    assert len(items) == len(raw_headings), (
        f"parser found {len(items)} items but the tracker has "
        f"{len(raw_headings)} '### ' headings"
    )


def test_numbered_items_keep_their_number(tracker_text: str) -> None:
    """A heading like '### 1.9a Foo' must not be parsed as number='1.9a' Title='Foo'
    turning into a bare word -- regression guard for the §10 legacy-heading bug
    where any first word (e.g. 'G*', 'Prior') was mistaken for an item number."""
    items = parse_tracker(tracker_text)
    for it in items:
        if it.number != "—":
            assert re.fullmatch(r"\d+(\.\d+)?[a-z]?", it.number), (
                f"suspicious parsed number {it.number!r} for item {it.title!r}"
            )


def test_index_open_closed_counts_are_consistent() -> None:
    """The summary line's open/closed counts must match the per-section counts."""
    text = INDEX.read_text(encoding="utf-8")
    total_open = int(re.search(r"\*\*(\d+) open, (\d+) closed", text).group(1))
    total_closed = int(re.search(r"\*\*(\d+) open, (\d+) closed", text).group(2))
    section_open = sum(int(m) for m in re.findall(r"\*\*(\d+) open / \d+ total\.\*\*", text))
    assert total_open == section_open, (
        f"top-line open count {total_open} != sum of per-section open counts {section_open}"
    )
    assert total_open + total_closed == len(parse_tracker(TRACKER.read_text(encoding="utf-8")))

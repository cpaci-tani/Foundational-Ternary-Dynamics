"""Guards for the working/resolved tracker split and generated working index."""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))

from theory.build_open_items_index import gfm_slug, parse_tracker  # noqa: E402

TRACKER = ROOT / "docs" / "theory" / "07_assessment" / "core_ledgers" / "TRACKER_OPEN_ITEMS.md"
RESOLVED = ROOT / "docs" / "theory" / "07_assessment" / "core_ledgers" / "TRACKER_RESOLVED_ITEMS.md"
INDEX = ROOT / "docs" / "theory" / "07_assessment" / "core_ledgers" / "TRACKER_OPEN_ITEMS_INDEX.md"
BUILDER = ROOT / "scripts" / "theory" / "build_open_items_index.py"


@pytest.fixture(scope="module")
def tracker_texts() -> tuple[str, str]:
    return (TRACKER.read_text(encoding="utf-8"), RESOLVED.read_text(encoding="utf-8"))


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


def test_every_item_heading_is_parsed(tracker_texts: tuple[str, str]) -> None:
    """No `### ` heading in either tracker may be silently skipped."""
    for tracker_text in tracker_texts:
        raw_headings = re.findall(r"^### (.+)$", tracker_text, re.MULTILINE)
        items = parse_tracker(tracker_text)
        assert len(items) == len(raw_headings)


def test_numbered_items_keep_their_number(tracker_texts: tuple[str, str]) -> None:
    """A heading like '### 1.9a Foo' must not be parsed as number='1.9a' Title='Foo'
    turning into a bare word -- regression guard for the §10 legacy-heading bug
    where any first word (e.g. 'G*', 'Prior') was mistaken for an item number."""
    for tracker_text in tracker_texts:
        for it in parse_tracker(tracker_text):
            if it.number != "—":
                assert re.fullmatch(r"\d+(\.\d+)?[a-z]?", it.number), (
                    f"suspicious parsed number {it.number!r} for item {it.title!r}"
                )


def test_index_counts_and_links_are_consistent(tracker_texts: tuple[str, str]) -> None:
    """The index covers working headings and reports resolved provenance."""
    text = INDEX.read_text(encoding="utf-8")
    working = parse_tracker(tracker_texts[0])
    resolved = parse_tracker(tracker_texts[1])
    summary = re.search(r"\*\*(\d+) working / (\d+) resolved or retired\*\*", text)
    assert summary is not None
    assert (int(summary.group(1)), int(summary.group(2))) == (len(working), len(resolved))
    assert sum(map(int, re.findall(r"\*\*(\d+) working headings\.\*\*", text))) == len(working)
    assert len(re.findall(r"\]\(TRACKER_OPEN_ITEMS\.md#", text)) == len(working)
    for item in working:
        assert f"TRACKER_OPEN_ITEMS.md#{gfm_slug(item.raw_heading)}" in text


def test_mixed_open_and_pending_verification_stay_working(
    tracker_texts: tuple[str, str],
) -> None:
    working_items = parse_tracker(tracker_texts[0])
    resolved_items = parse_tracker(tracker_texts[1])
    headings = [item.raw_heading for item in working_items + resolved_items]
    assert len(headings) == len(set(headings))
    working = {item.number for item in working_items}
    resolved = {item.number for item in resolved_items}
    assert {"1.9g", "1.12", "4.2", "7.7", "1.7"} <= working
    assert {"1.9g", "1.12", "4.2", "7.7", "1.7"}.isdisjoint(resolved)
    assert "7.4" in resolved and "7.4" not in working
    assert "## §5 Theory" not in tracker_texts[0]
    assert "## §5 Theory" in tracker_texts[1]

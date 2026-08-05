"""Guards for the LEDGER index + restructure (2026-08-04).

These tests exist because the ledger already decayed once: the Quick index was
specified as a browsable table, grew to 1.21 MB of 30 KB rows, and its keyword
sector classifier silently fell 37% behind the corpus. Each test below pins one
of the properties that decay violated.
"""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))

from verification.parsers.ledger_parser import parse_ledger, load_detail_blocks  # noqa: E402
from verification.parsers.ledger_taxonomy import (  # noqa: E402
    CATEGORIES,
    category_for,
)

LEDGER = ROOT / "docs" / "theory" / "07_assessment" / "core_ledgers" / "LEDGER.md"
INDEX = ROOT / "docs" / "theory" / "07_assessment" / "core_ledgers" / "LEDGER_INDEX.md"
BUILDER = ROOT / "scripts" / "theory" / "build_ledger_index.py"
RESTRUCTURER = ROOT / "scripts" / "theory" / "restructure_ledger.py"

ROW = re.compile(r"^\|\s*(FTD-\d{4}[A-Za-z0-9\-]*)\s*\|")


@pytest.fixture(scope="module")
def ledger_text() -> str:
    return LEDGER.read_text(encoding="utf-8")


@pytest.fixture(scope="module")
def row_ids(ledger_text: str) -> list[str]:
    return [m.group(1) for m in (ROW.match(ln) for ln in ledger_text.splitlines()) if m]


def test_every_ledger_id_has_a_category(row_ids: list[str]) -> None:
    """The taxonomy must never fall behind the ledger — the old classifier did."""
    unassigned = [i for i in row_ids if category_for(i) is None]
    assert not unassigned, (
        f"{len(unassigned)} ledger ids have no category: {unassigned[:10]}.\n"
        "Extend RANGES or EXCEPTIONS in "
        "scripts/verification/parsers/ledger_taxonomy.py"
    )


def test_every_category_is_populated(row_ids: list[str]) -> None:
    """A category with no rows is a taxonomy that has drifted off the corpus."""
    used = {category_for(i) for i in row_ids}
    empty = sorted(set(CATEGORIES) - used)
    assert not empty, f"categories with zero rows: {empty}"


def test_index_is_in_sync() -> None:
    """LEDGER_INDEX.md must be a faithful rebuild of the current LEDGER.md."""
    result = subprocess.run(
        [sys.executable, str(BUILDER), "--check"],
        capture_output=True, text=True, cwd=str(ROOT),
    )
    assert result.returncode == 0, (
        f"index out of sync (exit {result.returncode}):\n"
        f"STDOUT: {result.stdout}\nSTDERR: {result.stderr}"
    )


def test_restructure_is_lossless() -> None:
    """Detail blocks, anchors and links must all still resolve."""
    result = subprocess.run(
        [sys.executable, str(RESTRUCTURER), "--check"],
        capture_output=True, text=True, cwd=str(ROOT),
    )
    assert result.returncode == 0, (
        f"restructure check failed (exit {result.returncode}):\n"
        f"STDOUT: {result.stdout}\nSTDERR: {result.stderr}"
    )


def test_index_covers_every_row(row_ids: list[str]) -> None:
    """No claim may be missing from the index — that is the whole point of it."""
    index_ids = set(re.findall(r"`(FTD-\d{4}[A-Za-z0-9\-]*)`", INDEX.read_text(encoding="utf-8")))
    missing = sorted(set(row_ids) - index_ids)
    assert not missing, f"{len(missing)} ledger rows absent from the index: {missing[:10]}"


def test_quick_index_rows_stay_browsable(ledger_text: str) -> None:
    """Rows must not silently grow back into 30 KB single-line essays."""
    oversized = [
        (m.group(1), len(ln))
        for ln in ledger_text.splitlines()
        if (m := ROW.match(ln)) and len(ln) > 4000
    ]
    assert not oversized, (
        f"{len(oversized)} Quick-index rows exceed 4000 chars: {oversized[:5]}.\n"
        "Run: python scripts/theory/restructure_ledger.py"
    )


def test_moved_rows_are_recoverable(ledger_text: str) -> None:
    """Every truncated row must resolve to a detail block holding its full text."""
    details = load_detail_blocks(ledger_text)
    linked = set(re.findall(r"\[…full record →\]\(#([a-z0-9\-]+)\)", ledger_text))
    dangling = sorted(linked - set(details))
    assert not dangling, f"row links with no detail block: {dangling[:10]}"
    assert details, "no detail blocks found; has the restructure been reverted?"


def test_index_never_hides_a_verdict() -> None:
    """The index must show a row's COMPLETE tag set, never a truncated prefix.

    Regression guard: clipping the verbatim bracket text at 90 chars made 170
    rows read as `[THEOREM] …` while a `[CLOSED NEGATIVE]` / `[RETRACTED]` /
    `[FOUNDATIONAL OBSTRUCTION]` later in the same cell fell off the end. In a
    corpus that is majority negative results, that turns the navigation aid
    into an overclaim generator.
    """
    rows = {r["id"]: r for r in parse_ledger(LEDGER)}
    index = INDEX.read_text(encoding="utf-8")
    row_re = re.compile(
        r"^\| `(FTD-\d{4}[A-Za-z0-9\-]*)` \| (.*?) \| (.*?) \| L\d+ \|$", re.M
    )
    mismatched = []
    for m in row_re.finditer(index):
        ftd_id, shown = m.group(1), m.group(2)
        row = rows.get(ftd_id)
        if row is None:
            continue
        if {t.strip() for t in shown.split(",")} != set(row["tags"]):
            mismatched.append(ftd_id)
    assert not mismatched, (
        f"{len(mismatched)} index rows show a tag set that differs from the "
        f"parsed row: {mismatched[:10]}"
    )


def test_parser_reads_full_text_not_the_truncated_cell(ledger_text: str) -> None:
    """Dependencies live in the moved text; the parser must follow the link.

    Regression guard: reading only the table cost `deps` on 369 rows.
    """
    rows = {r["id"]: r for r in parse_ledger(LEDGER)}
    details = load_detail_blocks(ledger_text)
    assert details, "no detail blocks to check against"

    checked = 0
    for anchor, (_tag, record) in details.items():
        ftd_id = anchor[: -len("-record")].upper()
        row = rows.get(ftd_id)
        if row is None:
            continue
        expected = {d for d in re.findall(r"\b(FTD-\d{4})\b", record) if d != ftd_id}
        if not expected:
            continue
        assert expected <= set(row["deps"]), (
            f"{ftd_id}: deps {sorted(expected - set(row['deps']))[:5]} present in the "
            "detail block but missing from the parsed row"
        )
        checked += 1
    assert checked > 100, f"only {checked} rows exercised this guard; expected >100"

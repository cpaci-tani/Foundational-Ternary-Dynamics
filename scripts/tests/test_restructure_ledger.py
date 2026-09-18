"""Guards for `scripts/theory/restructure_ledger.py` (2026-09-17).

`test_ledger_index.py::test_quick_index_rows_stay_browsable` names this script
as its remedy. On 2026-09-17 the remedy was unusable: a second run re-detected
every row it had already moved (the truncated prefix plus link suffix exceeds
the cell budget), emitted a second `## Full row records` section with a
duplicate anchor per row, and its own lossless verifier refused to write. The
one over-4000-char row of the day was tightened by hand instead.

These tests pin the properties that make the tool safe to run repeatedly on
the canonical ledger: a second pass moves nothing and changes no byte, an
anchor is never minted twice, hand-maintained blocks are left alone, and the
selection can be narrowed to exactly the rows the browsability test flags.
"""

from __future__ import annotations

import importlib.util
import os
import re
import subprocess
import sys
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "scripts" / "theory" / "restructure_ledger.py"
LEDGER = ROOT / "docs" / "theory" / "07_assessment" / "core_ledgers" / "LEDGER.md"

ANCHOR = re.compile(r'<a id="([^"]+)"></a>')
ROW = re.compile(r"^\|\s*(FTD-\d{4}[A-Za-z0-9\-]*)\s*\|")


@pytest.fixture(scope="module")
def rl():
    """The script imported as a module (scripts/theory is not a package)."""
    spec = importlib.util.spec_from_file_location("restructure_ledger", SCRIPT)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def row(ftd_id: str, short: str, tag: str, desc: str) -> str:
    return f"| {ftd_id} | {short} | {tag} | {desc} |"


def make_ledger(rows: list[str], extra_sections: str = "") -> str:
    """A minimal LEDGER.md with the structure the script relies on."""
    return "\n".join([
        "# LEDGER (fixture)",
        "",
        "## Quick index",
        "",
        "| ID | Short name | Tag | Record |",
        "|----|------------|-----|--------|",
        *rows,
        "",
        extra_sections,
        "## Maintenance log",
        "",
        "- fixture entry",
        "",
    ])


LONG_DESC = ("NEW 2026-09-17 — depends on FTD-0900 and FTD-0901; evidence in "
             "`scripts/proofs/proof_fixture.py`. " + "filler word " * 40).strip()
SHORT_DESC = "RESOLVED — in budget."


def rows_of(text: str) -> dict[str, str]:
    return {m.group(1): ln for ln in text.splitlines() if (m := ROW.match(ln))}


# --------------------------------------------------------------------------
# Idempotency: the property that was broken.
# --------------------------------------------------------------------------

def test_second_pass_moves_nothing_and_changes_no_byte(rl) -> None:
    text = make_ledger([
        row("FTD-0001", "over budget", "[THEOREM]", LONG_DESC),
        row("FTD-0002", "in budget", "[CLOSED NEGATIVE]", SHORT_DESC),
        row("FTD-0003", "also over", "[SELECTION]", LONG_DESC + " more"),
    ])

    first, moved_first, _ = rl.restructure(text)
    assert [m["id"] for m in moved_first] == ["FTD-0001", "FTD-0003"]
    assert rl.verify(text, first) == []

    second, moved_second, _ = rl.restructure(first)
    assert moved_second == [], "rows already holding a detail block were moved again"
    assert second == first, "a no-op pass must be byte-identical"
    assert first.count(rl.SECTION_HEADING) == 1
    anchors = ANCHOR.findall(second)
    assert len(anchors) == len(set(anchors)) == 2


def test_new_rows_join_the_existing_section_without_a_second_heading(rl) -> None:
    """A row added after the first restructure lands in the same section."""
    base = make_ledger([row("FTD-0001", "first", "[THEOREM]", LONG_DESC)])
    first, _, _ = rl.restructure(base)

    # Append a fresh over-budget row below the existing one, as a later
    # session booking a new claim would.
    grown = first.replace(
        rows_of(first)["FTD-0001"],
        rows_of(first)["FTD-0001"] + "\n" + row("FTD-0002", "later", "[DERIVED]", LONG_DESC),
        1,
    )
    second, moved, _ = rl.restructure(grown)

    assert [m["id"] for m in moved] == ["FTD-0002"]
    assert second.count(rl.SECTION_HEADING) == 1
    assert rl.verify(grown, second) == []
    heading_at = second.index(rl.SECTION_HEADING)
    assert second.index('<a id="ftd-0001-record"></a>') > heading_at
    assert second.index('<a id="ftd-0002-record"></a>') > second.index('<a id="ftd-0001-record"></a>')
    assert second.index("## Maintenance log") > second.index('<a id="ftd-0002-record"></a>')


def test_hand_maintained_block_is_never_duplicated_or_overwritten(rl) -> None:
    """The FTD-0207 pattern: an anchor exists, the row was rewritten by hand
    without the link, and a provenance note sits between anchor and Tag."""
    kept_note = "**Retired by owner instruction.** The record below is historical provenance."
    hand_block = "\n".join([
        "## Full row records",
        "",
        "### FTD-0002 — full record",
        "",
        '<a id="ftd-0002-record"></a>',
        "",
        kept_note,
        "",
        "**Tag.** [INFRASTRUCTURE] original tag text",
        "",
        "**Record.** original record text that the owner chose to keep",
        "",
        "---",
        "",
    ])
    unlinked_row = row("FTD-0002", "retired", "[INFRASTRUCTURE]", LONG_DESC)
    text = make_ledger([
        row("FTD-0001", "over budget", "[THEOREM]", LONG_DESC),
        unlinked_row,
    ], extra_sections=hand_block)

    rewritten, moved, report = rl.restructure(text)

    assert [m["id"] for m in moved] == ["FTD-0001"]
    assert rows_of(rewritten)["FTD-0002"] == unlinked_row, "hand-rewritten row must stay byte-identical"
    assert kept_note in rewritten, "the owner's provenance note must survive"
    anchors = ANCHOR.findall(rewritten)
    assert len(anchors) == len(set(anchors))
    assert report["unlinked_anchor"] == ["FTD-0002"]
    assert rl.verify(text, rewritten) == []


def test_drift_between_a_linked_row_and_its_block_is_reported(rl) -> None:
    """A hand edit to a linked row's table cell is invisible to the parser
    (the block wins), so the tool must surface it rather than guess."""
    base = make_ledger([row("FTD-0001", "drifting", "[THEOREM]", LONG_DESC)])
    first, _, _ = rl.restructure(base)
    edited_row = rows_of(first)["FTD-0001"].replace("[THEOREM]", "[THEOREM] + [RETRACTED]", 1)
    drifted = first.replace(rows_of(first)["FTD-0001"], edited_row, 1)

    rewritten, moved, report = rl.restructure(drifted)

    assert moved == []
    assert rewritten == drifted
    assert [d["id"] for d in report["drift"]] == ["FTD-0001"]


# --------------------------------------------------------------------------
# Selection: fix exactly the rows the browsability test complains about.
# --------------------------------------------------------------------------

def test_only_over_selects_by_full_row_length(rl) -> None:
    essay = ("lorem ipsum " * 400).strip()          # > 4000 chars in one cell
    tolerated = row("FTD-0002", "tolerated", "[THEOREM]", LONG_DESC)  # > cell budget, < 4000
    text = make_ledger([
        row("FTD-0001", "essay", "[CLOSED NEGATIVE]", essay),
        tolerated,
        row("FTD-0003", "short", "[THEOREM]", SHORT_DESC),
    ])
    assert len(rows_of(text)["FTD-0001"]) > 4000 > len(tolerated)

    rewritten, moved, _ = rl.restructure(text, only_over=4000)

    assert [m["id"] for m in moved] == ["FTD-0001"]
    assert rows_of(rewritten)["FTD-0002"] == tolerated
    assert all(len(ln) <= 4000 for ln in rewritten.splitlines() if ROW.match(ln))
    assert rl.verify(text, rewritten) == []


# --------------------------------------------------------------------------
# CLI: the exact commands the test message and the docstring advertise.
# --------------------------------------------------------------------------

def run_cli(*args: str) -> subprocess.CompletedProcess:
    return subprocess.run(
        [sys.executable, str(SCRIPT), *args],
        capture_output=True, text=True, encoding="utf-8", cwd=str(ROOT),
    )


def moved_count(stdout: str) -> int:
    m = re.search(r"rows moved to detail blocks\s*:\s*(\d+)", stdout)
    assert m, f"no moved-count line in output:\n{stdout}"
    return int(m.group(1))


def test_cli_help_renders_on_a_non_utf8_console() -> None:
    """The docstring carries `→`; on a cp1252 Windows console argparse printed
    it before stdout was reconfigured and died with UnicodeEncodeError."""
    result = subprocess.run(
        [sys.executable, str(SCRIPT), "--help"],
        capture_output=True, text=True, encoding="utf-8", errors="replace",
        cwd=str(ROOT), env={**os.environ, "PYTHONIOENCODING": "cp1252"},
    )
    assert result.returncode == 0, result.stdout + result.stderr
    assert "--only-over" in result.stdout and "--ledger" in result.stdout


def test_cli_dry_run_after_a_write_moves_zero_rows(tmp_path: Path) -> None:
    fixture = tmp_path / "LEDGER.md"
    fixture.write_text(make_ledger([
        row("FTD-0001", "over budget", "[THEOREM]", LONG_DESC),
        row("FTD-0002", "in budget", "[CLOSED NEGATIVE]", SHORT_DESC),
    ]), encoding="utf-8")

    preview = run_cli("--ledger", str(fixture), "--dry-run")
    assert preview.returncode == 0, preview.stdout + preview.stderr
    assert moved_count(preview.stdout) == 1

    written = run_cli("--ledger", str(fixture))
    assert written.returncode == 0, written.stdout + written.stderr
    after_first = fixture.read_text(encoding="utf-8")
    assert after_first.count("## Full row records") == 1

    again = run_cli("--ledger", str(fixture), "--dry-run")
    assert again.returncode == 0, again.stdout + again.stderr
    assert moved_count(again.stdout) == 0
    assert "Refusing to write" not in again.stdout + again.stderr
    assert fixture.read_text(encoding="utf-8") == after_first

    check = run_cli("--ledger", str(fixture), "--check")
    assert check.returncode == 0, check.stdout + check.stderr


def test_cli_dry_run_runs_clean_on_the_canonical_ledger() -> None:
    """The remedy the browsability test names must actually be runnable."""
    result = run_cli("--dry-run")
    assert result.returncode == 0, result.stdout + result.stderr
    assert "FAIL:" not in result.stdout + result.stderr
    assert "--dry-run: nothing written." in result.stdout


def test_cli_only_over_4000_moves_nothing_while_the_ledger_is_browsable() -> None:
    """At HEAD no row exceeds 4000 chars, so the test's remedy must be a no-op."""
    oversized = [ln for ln in LEDGER.read_text(encoding="utf-8").splitlines()
                 if ROW.match(ln) and len(ln) > 4000]
    if oversized:
        pytest.skip("ledger currently has oversized rows; browsability test owns that")
    result = run_cli("--dry-run", "--only-over", "4000")
    assert result.returncode == 0, result.stdout + result.stderr
    assert moved_count(result.stdout) == 0

"""Focused checks for local and tracked-checkout index link validation."""

from __future__ import annotations

from pathlib import Path
import runpy
import subprocess
import sys

import pytest


SCRIPT = Path(__file__).resolve().parents[1] / "verification" / "verify_index_links.py"


def run_checker(root: Path, *args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        [sys.executable, str(SCRIPT), "--root", str(root), *args],
        cwd=root,
        capture_output=True,
        text=True,
    )


@pytest.fixture
def repo(tmp_path: Path) -> Path:
    subprocess.run(["git", "init", "-q", str(tmp_path)], check=True)
    theory = tmp_path / "docs" / "theory"
    native = theory / "10_eft_program"
    native.mkdir(parents=True)
    (theory / "META_INDEX.md").write_text(
        "[Local](local.md#section) [Remote](https://example.org/page)\n",
        encoding="utf-8",
    )
    (native / "INDEX_FTD_NATIVE_EFT.md").write_text(
        "[Tracked](tracked.md?view=1) [Folder](collection/)\n", encoding="utf-8"
    )
    assessment = theory / "07_assessment"
    assessment.mkdir()
    (assessment / "INDEX_07_ASSESSMENT.md").write_text(
        "[Tracked](../10_eft_program/tracked.md)\n", encoding="utf-8"
    )
    (theory / "local.md").write_text("# Local\n", encoding="utf-8")
    (native / "tracked.md").write_text("# Tracked\n", encoding="utf-8")
    (native / "collection").mkdir()
    (native / "collection/item.md").write_text("# Item\n", encoding="utf-8")
    (tmp_path / "README.md").write_text("[Theory](docs/theory/META_INDEX.md)\n", encoding="utf-8")
    (tmp_path / "CLAUDE.md").write_text("[Theory](docs/theory/META_INDEX.md)\n", encoding="utf-8")
    (tmp_path / "REPOSITORY_MAP.md").write_text("[Theory](docs/theory/META_INDEX.md)\n", encoding="utf-8")
    (tmp_path / "docs/WHERE_WE_LEFT_OFF.md").write_text(
        "[Theory](theory/META_INDEX.md)\n", encoding="utf-8"
    )
    subprocess.run(
        ["git", "add", "-f", "README.md", "CLAUDE.md", "REPOSITORY_MAP.md",
         "docs/WHERE_WE_LEFT_OFF.md", "docs/theory/META_INDEX.md",
         "docs/theory/10_eft_program/INDEX_FTD_NATIVE_EFT.md",
         "docs/theory/07_assessment/INDEX_07_ASSESSMENT.md",
         "docs/theory/10_eft_program/tracked.md",
         "docs/theory/10_eft_program/collection/item.md"],
        cwd=tmp_path,
        check=True,
    )
    return tmp_path


def test_import_does_not_run_checks(capsys: pytest.CaptureFixture[str]) -> None:
    runpy.run_path(str(SCRIPT), run_name="import_check")
    assert capsys.readouterr().out == ""


def test_local_mode_accepts_existing_untracked_link(repo: Path) -> None:
    result = run_checker(repo)
    assert result.returncode == 0, result.stdout + result.stderr
    assert "Verified 1 file links. Broken: 0" in result.stdout


def test_tracked_mode_reports_untracked_link(repo: Path) -> None:
    result = run_checker(repo, "--tracked")
    assert result.returncode == 1
    assert "[UNTRACKED] Local -> local.md#section" in result.stdout
    subprocess.run(["git", "add", "-f", "docs/theory/local.md"], cwd=repo, check=True)
    assert run_checker(repo, "--tracked").returncode == 0


def test_missing_target_or_index_fails(repo: Path) -> None:
    (repo / "docs/theory/local.md").unlink()
    result = run_checker(repo)
    assert result.returncode == 1
    assert "[BROKEN] Local" in result.stdout
    (repo / "docs/theory/10_eft_program/INDEX_FTD_NATIVE_EFT.md").unlink()
    assert run_checker(repo).returncode == 1
    assert "[MISSING INDEX]" in run_checker(repo, "--tracked").stdout


def test_tracked_mode_requires_index_itself(repo: Path) -> None:
    subprocess.run(
        ["git", "rm", "--cached", "docs/theory/META_INDEX.md"],
        cwd=repo,
        check=True,
        capture_output=True,
    )
    result = run_checker(repo, "--tracked")
    assert result.returncode == 1
    assert "[UNTRACKED INDEX]" in result.stdout

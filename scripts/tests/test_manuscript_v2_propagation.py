"""Focused checks for the report-only manuscript propagation inventory."""

from __future__ import annotations

import importlib.util
from pathlib import Path
import subprocess
import sys


SCRIPT = Path(__file__).resolve().parents[1] / "verification" / "check_manuscript_v2_propagation.py"


def test_inventory_and_strict_exit(tmp_path: Path) -> None:
    chapters = tmp_path / "dissemination/manuscript_v2"
    source = chapters / "src/chapters"
    vol1 = chapters / "vol1/src/chapters"
    vol2 = chapters / "vol2/src/chapters"
    for directory in (source, vol1, vol2):
        directory.mkdir(parents=True)
    (source / "01.qmd").write_text("canonical\n", encoding="utf-8")
    (source / "02.qmd").write_text("source only\n", encoding="utf-8")
    (vol1 / "01.qmd").write_text("volume edit\n", encoding="utf-8")
    (vol1 / "orphan.qmd").write_text("orphan\n", encoding="utf-8")
    (vol2 / "01.qmd").write_text("canonical\n", encoding="utf-8")

    spec = importlib.util.spec_from_file_location("manuscript_inventory", SCRIPT)
    assert spec and spec.loader
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    report = module.inventory(tmp_path)
    assert report["source_only"] == ["02.qmd"]
    assert report["volumes"]["vol1"]["different"] == ["01.qmd"]
    assert report["volumes"]["vol1"]["missing_source"] == ["orphan.qmd"]
    assert report["volumes"]["vol2"]["different"] == []

    command = [sys.executable, str(SCRIPT), "--root", str(tmp_path)]
    assert subprocess.run(command, capture_output=True).returncode == 0
    assert subprocess.run([*command, "--strict"], capture_output=True).returncode == 1


def test_missing_tree_is_not_reported_as_clean(tmp_path: Path) -> None:
    result = subprocess.run(
        [sys.executable, str(SCRIPT), "--root", str(tmp_path)],
        capture_output=True,
        text=True,
    )
    assert result.returncode == 2
    assert "Missing consolidated chapter directory" in result.stderr

"""Keep superseded epistemic reports out of the active test surface."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def test_superseded_verification_is_archived_as_text() -> None:
    for name in ("run_rigorous_verification", "test_epistemic_classification"):
        assert not (ROOT / "scripts" / "tests" / f"{name}.py").exists()
        archived = ROOT / "scripts" / "retired" / f"{name}.py.txt"
        source = archived.read_text(encoding="utf-8")
        assert "\nRETIRED HISTORICAL SOURCE" in source[:150]
        if name == "run_rigorous_verification":
            assert "raise SystemExit(" in source.split("import sys", 1)[0]

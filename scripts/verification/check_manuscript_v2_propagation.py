"""Report whether manuscript v2 volume chapters match the consolidated source."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[2]
MANUSCRIPT = Path("dissemination/manuscript_v2")


def inventory(root: Path) -> dict[str, object]:
    manuscript = root / MANUSCRIPT
    source = manuscript / "src/chapters"
    if not source.is_dir():
        raise FileNotFoundError(f"Missing consolidated chapter directory: {source}")
    source_names = {path.name for path in source.glob("*.qmd")}
    volumes: dict[str, dict[str, object]] = {}
    volume_names: set[str] = set()

    for name in ("vol1", "vol2"):
        chapters = manuscript / name / "src/chapters"
        if not chapters.is_dir():
            raise FileNotFoundError(f"Missing volume chapter directory: {chapters}")
        copies = sorted(chapters.glob("*.qmd"))
        volume_names.update(path.name for path in copies)
        missing = [path.name for path in copies if path.name not in source_names]
        different = [
            path.name for path in copies
            if path.name in source_names and path.read_bytes() != (source / path.name).read_bytes()
        ]
        volumes[name] = {
            "copies": len(copies),
            "missing_source": missing,
            "different": different,
        }

    return {
        "source_chapters": len(source_names),
        "source_only": sorted(source_names - volume_names),
        "volumes": volumes,
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--strict", action="store_true", help="fail on missing or divergent volume copies")
    parser.add_argument("--json", action="store_true", help="print the structured inventory")
    parser.add_argument("--root", type=Path, default=ROOT, help=argparse.SUPPRESS)
    args = parser.parse_args(argv)

    try:
        report = inventory(args.root.resolve())
    except FileNotFoundError as exc:
        print(exc, file=sys.stderr)
        return 2
    if args.json:
        print(json.dumps(report, indent=2))
    else:
        print(f"Consolidated chapters: {report['source_chapters']}")
        for name, result in report["volumes"].items():
            print(
                f"{name}: {result['copies']} copies, "
                f"{len(result['missing_source'])} without source, "
                f"{len(result['different'])} divergent"
            )
            for filename in result["missing_source"]:
                print(f"  MISSING SOURCE: {filename}")
            for filename in result["different"]:
                print(f"  DIFFERS: {filename}")
        print(f"Consolidated-only chapters: {len(report['source_only'])}")

    has_drift = any(
        result["missing_source"] or result["different"]
        for result in report["volumes"].values()
    )
    return 1 if args.strict and has_drift else 0


if __name__ == "__main__":
    raise SystemExit(main())

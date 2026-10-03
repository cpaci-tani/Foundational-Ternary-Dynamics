"""Explicit hosted compute boundary; ordinary local pytest remains unchanged.

Enable with ``-p scripts.ci.pytest_budget --ci-budget``. Only named exhaustive
functions are deselected, with counts and reasons reported. No assertion is
rewritten, sampled, weakened, or reported as passed by this plugin.
"""

from __future__ import annotations

import ast
import json
from pathlib import Path


MANIFEST = Path(__file__).with_name("manual_tests.json")


def read_manifest(root: Path) -> dict[str, str]:
    data = json.loads(MANIFEST.read_text(encoding="utf-8"))
    if data["schema_version"] != 1:
        raise ValueError("Unsupported CI compute boundary schema")
    entries = data["functions"]
    known = {}
    for entry in entries:
        path, function = entry["node"].split("::")
        if not path.startswith("scripts/tests/") or ".." in Path(path).parts or not function.startswith("test_"):
            raise ValueError(f"Invalid CI compute boundary: {entry['node']}")
        if entry["node"] in known:
            raise ValueError(f"Duplicate CI compute boundary: {entry['node']}")
        functions = {node.name for node in ast.parse((root / path).read_text(encoding="utf-8")).body
                     if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef))}
        if function not in functions or not entry["reason"].strip():
            raise ValueError(f"Stale CI compute boundary or missing reason: {entry['node']}")
        known[entry["node"]] = entry["reason"]
    return known


def select(items: list, root: Path, known: dict[str, str]) -> tuple[list, list, dict[str, int]]:
    kept, excluded, counts = [], [], {}
    for item in items:
        path = Path(item.path).resolve().relative_to(root.resolve()).as_posix()
        function = getattr(item, "originalname", None) or item.name.split("[", 1)[0]
        key = f"{path}::{function}"
        if key in known:
            excluded.append(item)
            counts[key] = counts.get(key, 0) + 1
        else:
            kept.append(item)
    return kept, excluded, counts


def pytest_addoption(parser):
    parser.getgroup("FTD hosted compute").addoption(
        "--ci-budget", action="store_true", default=False,
        help="deselect explicitly listed exhaustive certificates; report exact counts")


def pytest_collection_modifyitems(config, items):
    if not config.getoption("--ci-budget"):
        return
    root = Path(__file__).resolve().parents[2]
    try:
        known = read_manifest(root)
        kept, excluded, counts = select(items, root, known)
    except (ValueError, KeyError, OSError) as error:
        import pytest
        raise pytest.UsageError(f"CI compute boundary invalid: {error}") from error
    items[:] = kept
    config._ftd_ci_budget = (known, counts, len(kept), len(excluded))
    if excluded:
        config.hook.pytest_deselected(items=excluded)


def pytest_terminal_summary(terminalreporter, exitstatus, config):
    if not hasattr(config, "_ftd_ci_budget"):
        return
    known, counts, selected, deselected = config._ftd_ci_budget
    terminalreporter.section("hosted compute boundary")
    terminalreporter.write_line(f"Selected {selected} ordinary cases; deferred {deselected} exhaustive cases.")
    terminalreporter.write_line(
        "Run unfiltered `python -m pytest scripts/tests/` locally, or dispatch CI with full=true. "
        "Deferred cases have no pass certificate from this run.")
    for key, count in sorted(counts.items()):
        terminalreporter.write_line(f"  {key}: {count} case(s): {known[key]}")

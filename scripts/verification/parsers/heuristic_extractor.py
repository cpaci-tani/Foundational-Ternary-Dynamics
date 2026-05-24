"""
E3: bare print / assert heuristic walker (best-effort).

The other ~85% of verify_*.py scripts and ~62% of proof_*.py scripts do
not use check() or ProofSuite.assert_*() patterns -- they rely on bare
`assert <condition>` statements with optional descriptive messages, or
on `print()` statements that report computed values.

Per the plan §Phase 2 (heuristic extractor): this is honestly low-fidelity
infrastructure.  Rather than emit per-identity records with noisy /
incomplete participant lists, we emit ONE *script-summary* identity per
file, recording:

  - the script's file path
  - the number of bare-assert statements found
  - the number of f-string prints found (proxy for "informal claims")
  - participants = identifiers used at the module top level (a coarse
    approximation; covers imported constants like G_STAR, PI, etc.)

Consumers see a single E3 node per script with `extractor: "E3"` so
provenance is honest -- they know this is a coverage-record, not a
parsed-identity-list.
"""
from __future__ import annotations

import ast
from pathlib import Path

from .object_aliases import canonicalise


def extract(script_path: Path) -> list[dict]:
    """Emit at most one summary identity per script, only if it has
    >= 3 bare asserts AND does not already use check()/ProofSuite."""
    try:
        src = script_path.read_text(encoding="utf-8")
        tree = ast.parse(src, filename=str(script_path))
    except (OSError, SyntaxError):
        return []

    # If this script uses check() or ProofSuite.assert_*(), skip -- E1/E2
    # already cover it.
    uses_structured = False
    for node in ast.walk(tree):
        if isinstance(node, ast.Call):
            if isinstance(node.func, ast.Name) and node.func.id == "check":
                uses_structured = True; break
            if isinstance(node.func, ast.Attribute) and node.func.attr in {
                "assert_close", "assert_equal", "assert_true", "add",
            }:
                uses_structured = True; break
    if uses_structured:
        return []

    n_asserts = sum(1 for n in ast.walk(tree) if isinstance(n, ast.Assert))
    n_prints = sum(
        1 for n in ast.walk(tree)
        if isinstance(n, ast.Call) and isinstance(n.func, ast.Name) and n.func.id == "print"
    )

    if n_asserts < 3 and n_prints < 5:
        return []  # too sparse to be informative

    # Module-level identifiers: imports, top-level assignments.
    top_level_names: set[str] = set()
    for stmt in tree.body:
        if isinstance(stmt, ast.Assign):
            for tgt in stmt.targets:
                if isinstance(tgt, ast.Name):
                    top_level_names.add(tgt.id)
        elif isinstance(stmt, ast.Import):
            for alias in stmt.names:
                top_level_names.add(alias.asname or alias.name.split(".")[0])
        elif isinstance(stmt, ast.ImportFrom):
            for alias in stmt.names:
                top_level_names.add(alias.asname or alias.name)

    participants = sorted({canonicalise(p) for p in top_level_names
                           if p.isupper() or p[:1].isupper()})

    return [{
        "id": f"{script_path.name}:summary",
        "label": f"{script_path.name}: {n_asserts} asserts + {n_prints} prints (heuristic summary)",
        "source_file": str(script_path),
        "source_line": 1,
        "participants": participants,
        "extractor": "E3",
        "ledger_ref": None,
        "n_asserts": n_asserts,
        "n_prints": n_prints,
    }]


def extract_all(script_paths: list[Path]) -> list[dict]:
    out: list[dict] = []
    for p in script_paths:
        out.extend(extract(p))
    return out


if __name__ == "__main__":
    import sys
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")
    root = Path(__file__).resolve().parents[3]
    candidates = sorted(
        list((root / "scripts" / "verification").glob("verify_*.py")) +
        list((root / "scripts" / "proofs").glob("proof_*.py"))
    )
    hits = 0
    for p in candidates:
        ids = extract(p)
        if ids:
            hits += 1
            print(f"  {ids[0]['n_asserts']:3d} asserts  {ids[0]['n_prints']:3d} prints  "
                  f"{p.relative_to(root)}")
    print(f"\nE3 emitted summaries for {hits}/{len(candidates)} scripts "
          f"(scripts not covered by E1/E2).")

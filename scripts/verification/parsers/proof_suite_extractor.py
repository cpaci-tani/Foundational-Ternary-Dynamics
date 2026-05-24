"""
E2: AST walker for the ProofSuite method-chain pattern in scripts/proofs/.

Detects calls of the form:
    <receiver>.assert_close(name, got, expected, tol, tag)
    <receiver>.assert_equal(name, got, expected, tag)
    <receiver>.assert_true(name, condition, tag)
    <receiver>.add(name, claim, value, expected, tolerance, tag)

The receiver is typically `s` or `suite`; we match on the *method name*,
not the receiver, so any ProofSuite-like usage is captured.

Source-of-truth for the method signatures: scripts/proofs/common.py (read
only, not modified).  See ProofSuite.assert_close / assert_equal /
assert_true / add definitions.
"""
from __future__ import annotations

import ast
from pathlib import Path

from .object_aliases import canonicalise
from .check_pattern_extractor import _collect_identifiers, EXCLUDE_NAMES

PROOF_SUITE_METHODS = {"assert_close", "assert_equal", "assert_true", "add"}

# Extra excludes specific to the proofs corpus (ProofSuite helpers).
EXTRA_EXCLUDES = {
    "s", "suite", "self", "cls", "result", "results", "proof", "MACHINE_EPS",
    "PERCENT_01", "PERCENT_1", "PERCENT_5", "PERCENT_10", "PERCENT_15",
    "PPM_1", "PPM_10", "tag", "claim", "name", "got", "expected", "tol",
    "tolerance", "condition", "value",
}
ALL_EXCLUDES = EXCLUDE_NAMES | EXTRA_EXCLUDES


def extract(script_path: Path) -> list[dict]:
    """Extract identities from a single Python script via ProofSuite calls."""
    try:
        src = script_path.read_text(encoding="utf-8")
        tree = ast.parse(src, filename=str(script_path))
    except (OSError, SyntaxError):
        return []

    identities: list[dict] = []
    for node in ast.walk(tree):
        if not isinstance(node, ast.Call):
            continue
        if not isinstance(node.func, ast.Attribute):
            continue
        if node.func.attr not in PROOF_SUITE_METHODS:
            continue
        if not node.args:
            continue

        # First arg is `name` (the label, a string).
        label_arg = node.args[0]
        if isinstance(label_arg, ast.Constant) and isinstance(label_arg.value, str):
            label = label_arg.value
        elif isinstance(label_arg, ast.JoinedStr):
            parts = []
            for v in label_arg.values:
                if isinstance(v, ast.Constant):
                    parts.append(str(v.value))
                else:
                    parts.append("<expr>")
            label = "".join(parts)
        else:
            label = f"<dynamic label @{node.lineno}>"

        # Participants from remaining args + keyword values.
        raw_ids: set[str] = set()
        for a in node.args[1:]:
            raw_ids |= _collect_identifiers(a)
        for kw in node.keywords:
            if kw.value is not None:
                raw_ids |= _collect_identifiers(kw.value)
        raw_ids = {p for p in raw_ids if p not in ALL_EXCLUDES}
        participants = sorted({canonicalise(p) for p in raw_ids})

        identities.append({
            "id": f"{script_path.name}:{node.lineno}",
            "label": label,
            "source_file": str(script_path),
            "source_line": node.lineno,
            "participants": participants,
            "extractor": "E2",
            "ledger_ref": None,
        })

    return identities


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
    candidates = sorted((root / "scripts" / "proofs").glob("proof_*.py"))
    hits = 0
    total = 0
    for p in candidates:
        ids = extract(p)
        if ids:
            hits += 1
            total += len(ids)
            if len(ids) >= 10:
                print(f"  {len(ids):4d}  {p.relative_to(root)}")
    print(f"\n{hits}/{len(candidates)} proof_*.py scripts have ProofSuite identities (total {total}).")

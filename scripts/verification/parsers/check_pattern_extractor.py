"""
E1: AST walker for the `check(label, computed, claim, ...)` pattern.

Generalises scripts/verification/extract_synonymy_graph.py to scan multiple
files.  Yields identity records suitable for the math node map's
layers.identities + layers.objects.

Each identity record:
    {
      "id":            "<file>:<line>",
      "label":         "A1 G* = Gamma(1/4)/Gamma(3/4)",
      "source_file":   "scripts/verification/verify_gstar_paper.py",
      "source_line":   46,
      "participants":  ["Gstar", "Gamma_quarter", ...],   # canonicalised
      "extractor":     "E1",
      "ledger_ref":    None,                              # populated by the orchestrator
    }
"""
from __future__ import annotations

import ast
import re
from pathlib import Path

from .object_aliases import canonicalise

# Names to exclude (Python plumbing + script-internal helpers).
EXCLUDE_NAMES = {
    "True", "False", "None", "int", "float", "str", "list", "tuple", "dict",
    "range", "len", "abs", "min", "max", "sum", "mp", "mpf", "mpc",
    "check", "results", "hyp4f3", "hyp5f4", "fabs", "fsum", "fprod",
    "binomial", "tol", "note", "label", "Path", "json", "subprocess", "sys",
    "exp", "log", "sin", "cos", "tan",  # transcendental funcs — not "objects"
}

LABEL_RE = re.compile(r"^([A-Z]+\d+[a-zA-Z]*(?:\.\d+)?)\s+(.*)$")


def _collect_identifiers(node: ast.AST) -> set[str]:
    """Walk an AST subtree and return every Name/Attribute leaf id."""
    ids: set[str] = set()
    for sub in ast.walk(node):
        if isinstance(sub, ast.Name):
            ids.add(sub.id)
        elif isinstance(sub, ast.Attribute):
            cur: ast.AST = sub
            while isinstance(cur, ast.Attribute):
                cur = cur.value
            if isinstance(cur, ast.Name):
                ids.add(cur.id)
        elif isinstance(sub, ast.Call) and isinstance(sub.func, ast.Name):
            ids.add(sub.func.id)
    return ids


def extract(script_path: Path) -> list[dict]:
    """Extract identities from a single Python script via the check() pattern.

    Returns [] if the script does not use the check() pattern at all.
    """
    try:
        src = script_path.read_text(encoding="utf-8")
        tree = ast.parse(src, filename=str(script_path))
    except (OSError, SyntaxError):
        return []

    identities: list[dict] = []
    for node in ast.walk(tree):
        if not (isinstance(node, ast.Call) and isinstance(node.func, ast.Name)):
            continue
        if node.func.id != "check":
            continue
        if not node.args:
            continue

        # Label = first arg (string literal or f-string).
        label_arg = node.args[0]
        if isinstance(label_arg, ast.Constant) and isinstance(label_arg.value, str):
            full_label = label_arg.value
        elif isinstance(label_arg, ast.JoinedStr):
            parts = []
            for v in label_arg.values:
                if isinstance(v, ast.Constant):
                    parts.append(str(v.value))
                else:
                    parts.append("<expr>")
            full_label = "".join(parts)
        else:
            full_label = f"<dynamic label @{node.lineno}>"

        # Participants from remaining args + keyword values.
        raw_ids: set[str] = set()
        for a in node.args[1:]:
            raw_ids |= _collect_identifiers(a)
        for kw in node.keywords:
            if kw.value is not None:
                raw_ids |= _collect_identifiers(kw.value)
        raw_ids = {p for p in raw_ids if p not in EXCLUDE_NAMES}
        participants = sorted({canonicalise(p) for p in raw_ids})

        identities.append({
            "id": f"{script_path.name}:{node.lineno}",
            "label": full_label,
            "source_file": str(script_path),
            "source_line": node.lineno,
            "participants": participants,
            "extractor": "E1",
            "ledger_ref": None,
        })

    return identities


def extract_all(script_paths: list[Path]) -> list[dict]:
    """Run extract() on a list of scripts; return concatenated identities."""
    out: list[dict] = []
    for p in script_paths:
        out.extend(extract(p))
    return out


if __name__ == "__main__":
    import sys
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")
    root = Path(__file__).resolve().parents[3]
    candidate_files = list((root / "scripts" / "verification").glob("verify_*.py")) + \
                      [root / "scripts" / "verification" / "verify_gstar_paper.py"]
    candidate_files = sorted(set(candidate_files))
    hits = 0
    for p in candidate_files:
        ids = extract(p)
        if ids:
            print(f"  {len(ids):4d}  {p.relative_to(root)}")
            hits += 1
    print(f"\n{hits}/{len(candidate_files)} scripts contain check() identities.")

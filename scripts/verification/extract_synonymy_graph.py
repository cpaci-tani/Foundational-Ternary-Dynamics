"""
extract_synonymy_graph.py

Parses scripts/verification/verify_gstar_paper.py and extracts the synonymy
graph implicit in its 117 numerical checks.

A *synonymy* in this corpus is a verified equality A = B where A and B name
different mathematical objects (e.g. G* = Gamma(1/4)/Gamma(3/4), or
varpi = pi * G_G). The graph has:

  - one node per named mathematical object that appears in any check
  - one identity-node per check() call (each check is one identity)
  - bipartite edges between identity-nodes and the object-nodes they touch

Emits scripts/verification/results/synonymy_graph.json with:

  {
    "schema_version": 1,
    "source_file": "scripts/verification/verify_gstar_paper.py",
    "source_commit": <git HEAD sha, recorded at extraction time>,
    "n_checks": <int>,
    "n_objects": <int>,
    "objects": [
        {"name": "Gstar", "valence": 33, "first_seen_line": 29},
        ...
    ],
    "checks": [
        {"id": "A1", "label": "G* = Gamma(1/4)/Gamma(3/4)",
         "line": 46, "participants": ["Gstar", "gamma", "mpf"]},
        ...
    ],
    "edges_bipartite": [
        ["A1", "Gstar"], ["A1", "gamma"], ...
    ],
    "edges_pairwise": [
        ["Gstar", "gamma", "A1"], ["Gstar", "mpf", "A1"], ...
    ]
  }

Run:
    python scripts/verification/extract_synonymy_graph.py

Path II Session A3 of the multi-session coordinated arc
(.claude/plans/let-s-proceed-on-the-eager-rocket.md).
"""
from __future__ import annotations

import ast
import json
import re
import subprocess
import sys
from collections import Counter, defaultdict
from pathlib import Path


# Names to exclude from the graph -- standard-library and helper noise.
# Everything else encountered in a check() call is treated as a math object.
EXCLUDE_NAMES = {
    # Python builtins
    "True", "False", "None", "int", "float", "str", "list", "tuple", "dict",
    "range", "len", "abs", "min", "max", "sum",
    # mpmath helpers that aren't math objects per se
    "mp", "mpf", "mpc",
    # script-internal helpers
    "check", "results", "hyp4f3", "hyp5f4", "fabs", "fsum", "fprod",
    "binomial",
    # check() keyword names
    "tol", "note", "label",
}


# A label like "A1 G* = Gamma(1/4)/Gamma(3/4)" splits into id "A1" and
# the descriptive remainder.
LABEL_RE = re.compile(r"^([A-Z]+\d+[a-zA-Z]*(?:\.\d+)?)\s+(.*)$")


def collect_identifiers(node: ast.AST) -> set[str]:
    """Walk an AST subtree and return every Name/Attribute identifier."""
    ids: set[str] = set()
    for sub in ast.walk(node):
        if isinstance(sub, ast.Name):
            ids.add(sub.id)
        elif isinstance(sub, ast.Attribute):
            # For X.Y patterns (e.g. mp.dps, mp.inf), record the leftmost
            # base Name -- the attribute is typically a config/method.
            cur: ast.AST = sub
            while isinstance(cur, ast.Attribute):
                cur = cur.value
            if isinstance(cur, ast.Name):
                ids.add(cur.id)
        elif isinstance(sub, ast.Call):
            # Calls are walked by ast.walk already, but record the callee
            # name explicitly in case the head is a Name like gamma(...)
            if isinstance(sub.func, ast.Name):
                ids.add(sub.func.id)
    return ids


def git_head_sha() -> str:
    try:
        out = subprocess.check_output(
            ["git", "rev-parse", "HEAD"],
            cwd=Path(__file__).resolve().parents[2],
            text=True,
        ).strip()
        return out
    except Exception:
        return "(unknown -- not a git working tree, or git unavailable)"


def main():
    repo_root = Path(__file__).resolve().parents[2]
    src_path = repo_root / "scripts" / "verification" / "verify_gstar_paper.py"
    out_dir = repo_root / "scripts" / "verification" / "results"
    out_path = out_dir / "synonymy_graph.json"
    out_dir.mkdir(parents=True, exist_ok=True)

    src = src_path.read_text(encoding="utf-8")
    tree = ast.parse(src, filename=str(src_path))

    checks: list[dict] = []
    object_first_seen: dict[str, int] = {}
    object_valence: Counter[str] = Counter()

    for node in ast.walk(tree):
        if not (isinstance(node, ast.Call) and isinstance(node.func, ast.Name)):
            continue
        if node.func.id != "check":
            continue
        if not node.args:
            continue

        # arg0 = label string literal (or attribute access -- we accept both)
        label_arg = node.args[0]
        if isinstance(label_arg, ast.Constant) and isinstance(label_arg.value, str):
            full_label = label_arg.value
        elif isinstance(label_arg, ast.JoinedStr):
            # f-strings -- reconstruct best-effort by concatenating literal parts
            parts = []
            for v in label_arg.values:
                if isinstance(v, ast.Constant):
                    parts.append(str(v.value))
                else:
                    parts.append("<expr>")
            full_label = "".join(parts)
        else:
            full_label = "<dynamic label>"

        m = LABEL_RE.match(full_label)
        if m:
            check_id, desc = m.group(1), m.group(2)
        else:
            check_id, desc = full_label[:6], full_label

        # Participating identifiers across all positional args after label
        # plus the keyword-arg values.
        participants: set[str] = set()
        for a in node.args[1:]:
            participants |= collect_identifiers(a)
        for kw in node.keywords:
            if kw.value is not None:
                participants |= collect_identifiers(kw.value)

        # Filter out excluded names.
        participants = {p for p in participants if p not in EXCLUDE_NAMES}

        line = node.lineno
        for p in participants:
            if p not in object_first_seen:
                object_first_seen[p] = line
        for p in participants:
            object_valence[p] += 1

        checks.append({
            "id": check_id,
            "label": desc,
            "line": line,
            "participants": sorted(participants),
        })

    objects = [
        {
            "name": name,
            "valence": object_valence[name],
            "first_seen_line": object_first_seen[name],
        }
        for name in sorted(object_valence, key=lambda n: (-object_valence[n], n))
    ]

    edges_bipartite = [
        [c["id"], p] for c in checks for p in c["participants"]
    ]

    # Pairwise: for each identity, emit every unordered pair of participants
    # with the identity ID as a label/witness. Useful for centrality.
    edges_pairwise: list[list[str]] = []
    for c in checks:
        ps = sorted(c["participants"])
        for i in range(len(ps)):
            for j in range(i + 1, len(ps)):
                edges_pairwise.append([ps[i], ps[j], c["id"]])

    payload = {
        "schema_version": 1,
        "source_file": "scripts/verification/verify_gstar_paper.py",
        "source_commit": git_head_sha(),
        "extraction_script": "scripts/verification/extract_synonymy_graph.py",
        "n_checks": len(checks),
        "n_objects": len(objects),
        "n_edges_bipartite": len(edges_bipartite),
        "n_edges_pairwise": len(edges_pairwise),
        "objects": objects,
        "checks": checks,
        "edges_bipartite": edges_bipartite,
        "edges_pairwise": edges_pairwise,
    }
    out_path.write_text(json.dumps(payload, indent=2, ensure_ascii=False), encoding="utf-8")

    # Console summary
    print(f"Wrote {out_path.relative_to(repo_root)}")
    print(f"  n_checks         = {len(checks)}")
    print(f"  n_objects        = {len(objects)}")
    print(f"  n_edges_bipart   = {len(edges_bipartite)}")
    print(f"  n_edges_pairwise = {len(edges_pairwise)}")
    print()
    print("Top-15 high-valence objects:")
    for o in objects[:15]:
        print(f"  {o['valence']:4d}  {o['name']:<20s}  (first seen line {o['first_seen_line']})")
    print()
    print("Isolated / low-valence objects (valence <= 2):")
    iso = [o for o in objects if o["valence"] <= 2]
    for o in iso:
        print(f"  {o['valence']:4d}  {o['name']:<20s}  (first seen line {o['first_seen_line']})")
    return 0


if __name__ == "__main__":
    sys.exit(main())

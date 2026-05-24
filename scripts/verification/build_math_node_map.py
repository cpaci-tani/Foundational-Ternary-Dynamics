"""
build_math_node_map.py -- orchestrator for the FTD math node-map.

Reads the canonical sources (LEDGER.md, SPEC_ALGEBRAIC_SPINE.md, the AST-
extractable Python script corpus) and merges them into a single
multi-layer JSON file at scripts/verification/results/math_node_map.json.

Phase 1 (this commit) populates:
  - layers.ledger    -- all FTD-NNNN claims, with tag normalisation
  - layers.theorems  -- the 9 spine theorems + 4 subsidiaries
  - edges.theorem-anchors-ledger
  - edges.ledger-depends-on

Phase 2 will populate:
  - layers.objects   -- mathematical constants/functions appearing in identities
  - layers.identities -- per-script check() / assert_close() / heuristic findings
  - edges.object-in-identity
  - edges.identity-witnesses-theorem

Phase 3 will add the cached force-directed layout coordinates per node.

Run:
    python scripts/verification/build_math_node_map.py

Output:
    scripts/verification/results/math_node_map.json
"""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

# Force UTF-8 on Windows consoles.
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT / "scripts"))

from verification.parsers.ledger_parser import parse_ledger, EPISTEMIC_COLORS, classify_sector
from verification.parsers.spine_parser import parse_spine
from verification.parsers import check_pattern_extractor as e1
from verification.parsers import proof_suite_extractor as e2
from verification.parsers import heuristic_extractor as e3
from verification.parsers.object_aliases import canonical_object_records, OBJECTS


SCHEMA_VERSION = 2

# Phase 1 schema (matches plan §Phase 1).  Phase 2 will extend layers.objects +
# layers.identities and the corresponding edge types.
EMPTY_SCHEMA = {
    "schema_version": SCHEMA_VERSION,
    "source_commit": None,
    "layers": {
        "objects": [],
        "identities": [],
        "theorems": [],
        "ledger": [],
    },
    "edges": [],
    "sectors": [
        "pure-math/structure",
        "pure-math/G*-family",
        "pure-math/master-quadratic",
        "pure-math/modular-FQCR",
        "pure-math/Watson-Catalan",
        "pure-math/CM-curves",
        "pure-math/unclassified",
        "physics/EM-alpha",
        "physics/QCD",
        "physics/EW-Higgs",
        "physics/flavor",
        "physics/gravity",
        "physics/cosmology",
        "physics/QM-foundations",
        "engine-bridge",
    ],
    "epistemic_colors": EPISTEMIC_COLORS,
}


def git_head_sha() -> str:
    try:
        return subprocess.check_output(
            ["git", "rev-parse", "HEAD"],
            cwd=REPO_ROOT, text=True,
        ).strip()
    except Exception:
        return "(unknown -- git unavailable)"


def _build_ledger_script_index(ledger_rows: list[dict]) -> dict[str, str]:
    """Map script basename -> FTD-NNNN of the row that mentions it."""
    idx: dict[str, str] = {}
    for r in ledger_rows:
        for sr in r.get("script_refs", []):
            base = Path(sr).name
            # Earliest-row wins for shared scripts (LEDGER is in roughly
            # chronological order by FTD-NNNN).
            idx.setdefault(base, r["id"])
    return idx


def _link_identities_to_ledger(identities: list[dict], script_index: dict[str, str]) -> None:
    """Populate identity['ledger_ref'] in-place via script-basename lookup."""
    for ident in identities:
        base = Path(ident["source_file"]).name
        if base in script_index:
            ident["ledger_ref"] = script_index[base]


def build() -> dict:
    out = json.loads(json.dumps(EMPTY_SCHEMA))
    out["source_commit"] = git_head_sha()

    # -------------------- Layer 4: LEDGER claims --------------------
    ledger_path = REPO_ROOT / "docs" / "theory" / "07_assessment" / "LEDGER.md"
    ledger_rows = parse_ledger(ledger_path)
    out["layers"]["ledger"] = ledger_rows
    ledger_ids = {r["id"] for r in ledger_rows}
    script_to_ledger = _build_ledger_script_index(ledger_rows)

    # -------------------- Layer 3: spine theorems --------------------
    spine_path = REPO_ROOT / "docs" / "theory" / "01_reference" / "SPEC_ALGEBRAIC_SPINE.md"
    theorems = parse_spine(spine_path)
    out["layers"]["theorems"] = theorems

    # -------------------- Layer 2: identities (E1 + E2 + E3) --------------------
    verify_scripts = sorted((REPO_ROOT / "scripts" / "verification").glob("verify_*.py"))
    proof_scripts = sorted((REPO_ROOT / "scripts" / "proofs").glob("proof_*.py"))

    identities: list[dict] = []
    identities.extend(e1.extract_all(verify_scripts))
    identities.extend(e2.extract_all(proof_scripts))
    identities.extend(e3.extract_all(verify_scripts + proof_scripts))

    _link_identities_to_ledger(identities, script_to_ledger)
    out["layers"]["identities"] = identities

    # -------------------- Layer 1: objects --------------------
    # Start with canonical object records (Phase 2 alias table).
    obj_records = canonical_object_records()
    canonical_ids = {o["id"] for o in obj_records}

    # Discover additional objects not in the alias table (long tail).
    discovered: dict[str, int] = {}
    for ident in identities:
        for p in ident.get("participants", []):
            discovered[p] = discovered.get(p, 0) + 1

    # Merge canonical + discovered, computing per-object valence.
    by_id = {o["id"]: o for o in obj_records}
    for name, count in discovered.items():
        if name in by_id:
            by_id[name]["valence"] = count
        else:
            # Non-canonical (e.g. obscure local variable).  Only promote to a
            # node if it shows up in >= 5 identities -- otherwise it is noise
            # from script-internal variables that the alias table hasn't
            # canonicalised.  (The alias table covers all the load-bearing
            # mathematical objects; high-valence unknowns are candidates for
            # future alias-table entries.)
            if count >= 5:
                by_id[name] = {
                    "id": name, "name": name, "kind": "unknown",
                    "valence": count, "sector": None,
                }
    # Drop canonical objects with zero observed valence to keep the graph clean.
    objects = [o for o in by_id.values() if o["valence"] > 0]
    # Sector classification by name (best-effort).
    for o in objects:
        if o.get("sector") is None:
            o["sector"] = classify_sector(o["name"] + " " + o["id"])
    out["layers"]["objects"] = objects
    obj_ids = {o["id"] for o in objects}

    # -------------------- Edges --------------------
    # theorem -> ledger
    for t in theorems:
        if t["ledger_ref"] and t["ledger_ref"] in ledger_ids:
            out["edges"].append({
                "type": "theorem-anchors-ledger",
                "from": t["id"], "to": t["ledger_ref"],
            })

    # ledger -> ledger (deps)
    for r in ledger_rows:
        for d in r.get("deps", []):
            if d in ledger_ids and d != r["id"]:
                out["edges"].append({
                    "type": "ledger-depends-on",
                    "from": r["id"], "to": d,
                })

    # object -> identity
    for ident in identities:
        for p in ident.get("participants", []):
            if p in obj_ids:
                out["edges"].append({
                    "type": "object-in-identity",
                    "from": p, "to": ident["id"],
                })

    # identity -> theorem (heuristic: by ledger_ref crosswalk)
    theorem_by_ledger: dict[str, str] = {
        t["ledger_ref"]: t["id"] for t in theorems if t["ledger_ref"]
    }
    for ident in identities:
        ref = ident.get("ledger_ref")
        if ref and ref in theorem_by_ledger:
            out["edges"].append({
                "type": "identity-witnesses-theorem",
                "from": ident["id"], "to": theorem_by_ledger[ref],
            })

    # identity -> ledger (direct, when not already routed via a theorem)
    for ident in identities:
        ref = ident.get("ledger_ref")
        if ref and ref in ledger_ids:
            out["edges"].append({
                "type": "identity-witnesses-ledger",
                "from": ident["id"], "to": ref,
            })

    return out


def summarise(data: dict) -> None:
    L = data["layers"]
    print(f"source_commit:      {data['source_commit']}")
    print(f"layers.objects:     {len(L['objects'])}")
    print(f"layers.identities:  {len(L['identities'])}")
    if L["objects"]:
        from collections import Counter
        kind_counts = Counter(o.get("kind") for o in L["objects"])
        print(f"  by kind:          {dict(kind_counts)}")
    if L["identities"]:
        from collections import Counter
        ext_counts = Counter(i.get("extractor") for i in L["identities"])
        print(f"  by extractor:     {dict(ext_counts)}")
        linked = sum(1 for i in L["identities"] if i.get("ledger_ref"))
        print(f"  linked-to-ledger: {linked}/{len(L['identities'])}")
    print(f"layers.theorems:    {len(L['theorems'])}")
    print(f"layers.ledger:      {len(L['ledger'])}")
    print(f"edges.total:        {len(data['edges'])}")
    edge_types = {}
    for e in data["edges"]:
        edge_types[e["type"]] = edge_types.get(e["type"], 0) + 1
    for t, n in edge_types.items():
        print(f"  {t:<32}  {n}")


def main():
    out_path = REPO_ROOT / "scripts" / "verification" / "results" / "math_node_map.json"
    out_path.parent.mkdir(parents=True, exist_ok=True)
    data = build()
    out_path.write_text(
        json.dumps(data, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )
    print(f"Wrote {out_path.relative_to(REPO_ROOT)}")
    summarise(data)
    return 0


if __name__ == "__main__":
    sys.exit(main())

"""
Mermaid-block renderer for the math node map.

Reads scripts/verification/results/math_node_map.json and emits the
per-sector Mermaid graph blocks that go into
docs/theory/09_mathematical/NODE_MAP_FTD_MATH.md.

Each sector gets one `graph LR` block showing:
  - the spine theorems anchored in that sector (diamond nodes)
  - the LEDGER claims in that sector (rectangular nodes, colored by epistemic tag)
  - the edges between them (theorem-anchors-ledger + ledger-depends-on)

Identities are NOT included in Mermaid blocks: 901 nodes would exceed
GitHub's Mermaid render budget.  Objects appear in a separate appendix
block (the high-valence backbone).
"""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parents[3]
JSON_PATH = REPO_ROOT / "scripts" / "verification" / "results" / "math_node_map.json"


def _safe_id(s: str) -> str:
    """Make an identifier Mermaid-safe."""
    return re.sub(r"[^A-Za-z0-9_]", "_", s)


def _safe_label(s: str) -> str:
    """Mermaid label-safe: strip pipes / brackets / quotes; truncate."""
    s = s.replace("[", "(").replace("]", ")").replace("|", "/")
    s = s.replace('"', "'").replace("\n", " ").replace("<", "{").replace(">", "}")
    if len(s) > 60:
        s = s[:57] + "..."
    return s


def render_sector_block(data: dict, sector: str) -> str:
    """Render one Mermaid `graph LR` block for a single sector."""
    theorems_in = [t for t in data["layers"]["theorems"]
                   if t.get("ledger_ref") and any(
                       r["id"] == t["ledger_ref"] and r["sector"] == sector
                       for r in data["layers"]["ledger"])]
    ledger_in = [r for r in data["layers"]["ledger"] if r["sector"] == sector]
    ledger_ids = {r["id"] for r in ledger_in}

    lines = ["```mermaid", "graph LR"]

    # Node declarations
    for t in theorems_in:
        nid = _safe_id(t["id"])
        label = _safe_label(f"{t['id']}: {t['name']}")
        lines.append(f'    {nid}{{{{ "{label}" }}}}')
    for r in ledger_in[:40]:  # cap per-sector to keep Mermaid happy
        nid = _safe_id(r["id"])
        label = _safe_label(f"{r['id']}: {r['short_name']}")
        color = r.get("epistemic_color", "#bdbdbd")
        lines.append(f'    {nid}["{label}"]')
        lines.append(f'    style {nid} fill:{color},color:white,stroke:#222,stroke-width:1px')

    # Edges (only those endpoints both visible in this sector block)
    visible = {_safe_id(t["id"]) for t in theorems_in} | {_safe_id(r["id"]) for r in ledger_in[:40]}
    for e in data["edges"]:
        if e["type"] not in ("theorem-anchors-ledger", "ledger-depends-on"):
            continue
        a = _safe_id(e["from"]); b = _safe_id(e["to"])
        if a in visible and b in visible:
            arrow = "==>" if e["type"] == "theorem-anchors-ledger" else "-->"
            lines.append(f"    {a} {arrow} {b}")

    if len(ledger_in) > 40:
        lines.append(f'    note["...{len(ledger_in) - 40} more LEDGER rows in this sector (see HTML map)"]')

    lines.append("```")
    return "\n".join(lines)


def render_object_backbone(data: dict, top_n: int = 30) -> str:
    """Render a Mermaid block for the top-N highest-valence objects."""
    objects = sorted(data["layers"]["objects"], key=lambda o: -o.get("valence", 0))[:top_n]
    if not objects:
        return "```mermaid\ngraph LR\n    note[\"no objects in graph\"]\n```"

    # Pairwise co-participation: for each pair of objects appearing together in
    # >= 3 identities, draw an edge.  Keeps the block readable.
    obj_ids = {o["id"] for o in objects}
    co_count: dict[tuple[str, str], int] = {}
    for ident in data["layers"]["identities"]:
        present = [p for p in ident.get("participants", []) if p in obj_ids]
        for i in range(len(present)):
            for j in range(i + 1, len(present)):
                key = tuple(sorted((present[i], present[j])))
                co_count[key] = co_count.get(key, 0) + 1

    lines = ["```mermaid", "graph LR"]
    for o in objects:
        nid = _safe_id(o["id"])
        label = _safe_label(f"{o.get('name', o['id'])} (v={o['valence']})")
        lines.append(f'    {nid}[("{label}")]')
    drawn = 0
    for (a, b), n in sorted(co_count.items(), key=lambda x: -x[1]):
        if n < 3 or drawn > 80:
            break
        lines.append(f'    {_safe_id(a)} --- {_safe_id(b)}')
        drawn += 1
    lines.append("```")
    return "\n".join(lines)


def render_full_doc(data: dict) -> str:
    sectors_present = sorted({r["sector"] for r in data["layers"]["ledger"]})
    sector_counts = {s: sum(1 for r in data["layers"]["ledger"] if r["sector"] == s)
                     for s in sectors_present}

    sections = [
        "# NODE MAP — FTD math connectivity",
        "",
        "**Tag:** [INFRASTRUCTURE / METHODOLOGY] — descriptive navigation, not theorem-production.",
        f"**Generated from:** `scripts/verification/results/math_node_map.json` (commit `{data.get('source_commit', '?')[:7]}`).",
        "**Renderer:** `scripts/verification/parsers/mermaid_renderer.py` via "
        "`scripts/verification/build_math_node_map.py`.",
        "",
        "> **Scope discipline.** This document is descriptive: it shows which LEDGER claims and "
        "spine theorems sit in each sector and how they depend on each other. The full "
        "multi-layer graph (with identities + objects + epistemic-tag overlay) is in the "
        "interactive HTML at `dissemination/interactive/math_node_map.html`. The Markdown "
        "Mermaid blocks below cap each sector at 40 LEDGER rows for renderer-budget reasons; "
        "the full set is in the JSON + HTML.",
        "",
        "---",
        "",
        "## §1 — Reading guide",
        "",
        f"**Nodes:** {len(data['layers']['theorems'])} spine theorems (T1–T9, S1–S4) + "
        f"{len(data['layers']['ledger'])} LEDGER claims + "
        f"{len(data['layers']['objects'])} mathematical objects + "
        f"{len(data['layers']['identities'])} identities.",
        f"**Edges:** {len(data['edges'])} total across 5 types "
        "(theorem→ledger anchor, ledger→ledger deps, identity→theorem witness, "
        "identity→ledger witness, object→identity participation).",
        "",
        "**Sectors (with row count):**",
        "",
    ]
    sections.extend([f"- `{s}` — {sector_counts.get(s, 0)} LEDGER rows" for s in sectors_present])
    sections.append("")
    sections.append("**Epistemic tags appearing:**")
    sections.append("")
    from collections import Counter
    tag_counts = Counter(r.get("primary_tag", "UNKNOWN") for r in data["layers"]["ledger"])
    for tag, n in tag_counts.most_common():
        color = data["epistemic_colors"].get(tag, "#888")
        sections.append(f"- `{tag}` ({n}, color {color})")
    sections.append("")
    sections.append("---")
    sections.append("")
    sections.append("## §2 — Per-sector Mermaid blocks")
    sections.append("")
    sections.append("<!-- AUTO-GENERATED: math_node_map -->")
    sections.append("")

    for sector in sectors_present:
        sections.append(f"### {sector}")
        sections.append("")
        sections.append(render_sector_block(data, sector))
        sections.append("")

    sections.append("---")
    sections.append("")
    sections.append("## §3 — Object backbone (top-30 by valence)")
    sections.append("")
    sections.append("Edges = pairs of objects co-participating in ≥ 3 verified identities. "
                    "Shows the connective tissue of the corpus (G_G ↔ π ↔ G\\* ↔ Γ-tower).")
    sections.append("")
    sections.append(render_object_backbone(data, top_n=30))
    sections.append("")
    sections.append("---")
    sections.append("")
    sections.append("## §4 — Reproduction")
    sections.append("")
    sections.append("```sh")
    sections.append("# Rebuild the canonical JSON:")
    sections.append("python scripts/verification/build_math_node_map.py")
    sections.append("")
    sections.append("# Recompute the force-directed layout (caches x,y per node):")
    sections.append("python dissemination/interactive/math_node_map_layout.py")
    sections.append("")
    sections.append("# Regenerate this Markdown:")
    sections.append("python -m scripts.verification.parsers.mermaid_renderer")
    sections.append("```")
    sections.append("")
    sections.append("---")
    sections.append("")
    sections.append("## §5 — Cross-references")
    sections.append("")
    sections.append("- `scripts/verification/results/math_node_map.json` — canonical machine-readable graph.")
    sections.append("- `dissemination/interactive/math_node_map.html` — interactive Plotly.js viewer "
                    "(filterable by layer, sector, epistemic tag, search).")
    sections.append("- `docs/theory/09_mathematical/ROADMAP_IDENTITY_PRIORITIES.md` — G\\*-paper-scoped "
                    "synonymy graph (predecessor; covers `verify_gstar_paper.py` only).")
    sections.append("- `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md` — canonical 9-theorem spine "
                    "(source for layers.theorems).")
    sections.append("- `docs/theory/07_assessment/LEDGER.md` — canonical claim registry "
                    "(source for layers.ledger).")
    sections.append("- LEDGER row FTD-0207 — this node map's provenance entry.")
    sections.append("")

    return "\n".join(sections)


def main():
    data = json.loads(JSON_PATH.read_text(encoding="utf-8"))
    doc = render_full_doc(data)
    out_path = REPO_ROOT / "docs" / "theory" / "09_mathematical" / "NODE_MAP_FTD_MATH.md"
    out_path.write_text(doc, encoding="utf-8")
    print(f"Wrote {out_path.relative_to(REPO_ROOT)} ({len(doc)} chars)")
    return 0


if __name__ == "__main__":
    sys.exit(main())

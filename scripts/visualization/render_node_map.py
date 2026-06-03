"""
matplotlib SVG/PNG renderer for the FTD math node map.

Reads scripts/verification/results/math_node_map.json (which already carries
the cached force-directed layout coordinates per node) and writes 5
publication-quality figure variants to
scripts/visualization/results/math_node_map/.

Variants:
  full_overview.{svg,png}     -- all four layers, full corpus
  spine_only.{svg,png}        -- 9 spine theorems + 4 subsidiaries + their
                                  LEDGER anchors + ledger-depends-on edges
  arc_b1_context.{svg,png}    -- FTD-0198 / 0204 / 0205 / 0206 cluster
  catalan_frontier.{svg,png}  -- FTD-0206 (Catalan PREREG) + neighbours
  sector_<name>.{svg,png}     -- one per LEDGER sector (~13 figures)

Usage:
    python scripts/visualization/render_node_map.py
"""
from __future__ import annotations

import json
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parents[2]
JSON_PATH = REPO_ROOT / "scripts" / "verification" / "results" / "math_node_map.json"
OUT_DIR = REPO_ROOT / "scripts" / "visualization" / "results" / "math_node_map"

LAYER_STYLE = {
    "theorems":   {"marker": "D", "default_color": "#1565c0", "size": 200, "edgecolor": "#0d47a1"},
    "ledger":     {"marker": "o", "default_color": "#37474f", "size":  60, "edgecolor": "#222"},
    "objects":    {"marker": "s", "default_color": "#6a1b9a", "size":  40, "edgecolor": "#4a148c"},
    "identities": {"marker": ".", "default_color": "#bbbbbb", "size":   8, "edgecolor": None},
}

EDGE_STYLE = {
    "theorem-anchors-ledger":     {"color": "#1565c0", "width": 1.5, "alpha": 0.8},
    "ledger-depends-on":          {"color": "#90a4ae", "width": 0.6, "alpha": 0.5},
    "identity-witnesses-theorem": {"color": "#00897b", "width": 0.4, "alpha": 0.6},
    "identity-witnesses-ledger":  {"color": "#aed581", "width": 0.3, "alpha": 0.4},
    "object-in-identity":         {"color": "#ce93d8", "width": 0.2, "alpha": 0.3},
}


def _all_nodes_by_id(data: dict) -> dict[str, dict]:
    out: dict[str, dict] = {}
    for layer in ("theorems", "ledger", "objects", "identities"):
        for n in data["layers"][layer]:
            n["_layer"] = layer
            out[n["id"]] = n
    return out


def _node_color(n: dict) -> str:
    return n.get("epistemic_color") or LAYER_STYLE[n["_layer"]]["default_color"]


def _draw(ax, data: dict, node_filter, edge_filter, title: str, *,
          show_labels: list[str] | None = None, label_max: int = 80) -> None:
    """Draw a subset onto the given axes."""
    nodes = [n for n in _all_nodes_by_id(data).values() if node_filter(n)]
    visible_ids = {n["id"] for n in nodes}

    # Edges first (under nodes).
    for e in data["edges"]:
        if not edge_filter(e):
            continue
        if e["from"] not in visible_ids or e["to"] not in visible_ids:
            continue
        a = next(n for n in nodes if n["id"] == e["from"])
        b = next(n for n in nodes if n["id"] == e["to"])
        style = EDGE_STYLE.get(e["type"], {"color": "#aaa", "width": 0.5, "alpha": 0.3})
        ax.plot([a["x"], b["x"]], [a["y"], b["y"]],
                color=style["color"], linewidth=style["width"], alpha=style["alpha"],
                zorder=1)

    # Nodes per layer.
    for layer in ("identities", "objects", "ledger", "theorems"):
        layer_nodes = [n for n in nodes if n["_layer"] == layer]
        if not layer_nodes:
            continue
        xs = [n["x"] for n in layer_nodes]
        ys = [n["y"] for n in layer_nodes]
        cs = [_node_color(n) for n in layer_nodes]
        ax.scatter(xs, ys, c=cs,
                   s=LAYER_STYLE[layer]["size"],
                   marker=LAYER_STYLE[layer]["marker"],
                   edgecolors=LAYER_STYLE[layer]["edgecolor"] or "none",
                   linewidths=0.5, zorder=3, label=layer)

    # Labels (only the small-set views).
    if show_labels is not None:
        targets = [n for n in nodes if n["id"] in show_labels][:label_max]
        for n in targets:
            label = n.get("short_name") or n.get("name") or n["id"]
            if len(label) > 40:
                label = label[:37] + "..."
            ax.annotate(label, (n["x"], n["y"]),
                        xytext=(4, 4), textcoords="offset points",
                        fontsize=7, color="#222", zorder=4)

    ax.set_title(title, fontsize=12)
    ax.set_xticks([]); ax.set_yticks([])
    ax.set_aspect("equal")
    ax.legend(loc="upper right", fontsize=8, framealpha=0.7)


def _save(fig, name: str) -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    for ext in ("svg", "png"):
        path = OUT_DIR / f"{name}.{ext}"
        fig.savefig(path, dpi=200, bbox_inches="tight", facecolor="white")
    plt.close(fig)


def _box(ax, x: float, y: float, w: float, h: float, *,
         title: str, body: str, tag: str, fill: str, edge: str) -> tuple[float, float]:
    patch = mpatches.FancyBboxPatch(
        (x, y), w, h,
        boxstyle="round,pad=0.012,rounding_size=0.018",
        facecolor=fill,
        edgecolor=edge,
        linewidth=1.45,
        zorder=3,
    )
    ax.add_patch(patch)
    ax.text(x + 0.018, y + h - 0.04, title,
            ha="left", va="top", fontsize=10.5, fontweight="bold",
            color="#263238", zorder=4)
    ax.text(x + 0.018, y + h - 0.083, body,
            ha="left", va="top", fontsize=8.2, linespacing=1.22,
            color="#37474f", zorder=4)
    return (x + w / 2.0, y + h / 2.0)


def _arrow(ax, start: tuple[float, float], end: tuple[float, float], *,
           color: str = "#607d8b", rad: float = 0.0, lw: float = 1.7) -> None:
    ax.add_patch(mpatches.FancyArrowPatch(
        start, end,
        arrowstyle="-|>",
        mutation_scale=13,
        linewidth=lw,
        color=color,
        connectionstyle=f"arc3,rad={rad}",
        shrinkA=12,
        shrinkB=12,
        zorder=2,
    ))


def render_full_overview(data: dict) -> None:
    fig, ax = plt.subplots(figsize=(14, 10), dpi=120)
    _draw(ax, data,
          node_filter=lambda n: True,
          edge_filter=lambda e: e["type"] != "object-in-identity",
          title="FTD math node map -- full overview "
                f"(commit {data.get('source_commit', '?')[:7]})")
    _save(fig, "full_overview")
    print("  full_overview.{svg,png}")


def render_spine_only(data: dict) -> None:
    fig, ax = plt.subplots(figsize=(13.5, 7.2), dpi=140)
    ax.set_xlim(0, 1)
    ax.set_ylim(0, 1)
    ax.axis("off")

    fig.suptitle(
        "FTD construction map: seed -> G* -> master quadratic -> audited spine",
        fontsize=15,
        fontweight="bold",
        y=0.965,
        color="#263238",
    )
    ax.text(
        0.5, 0.895,
        "A dependency view for §I.5.  Arrows read backward as the audit trail; "
        "physical readouts remain outside this theorem-grade construction.",
        ha="center", va="center", fontsize=9.5, color="#455a64",
    )

    styles = {
        "axiom": ("#f3e8ff", "#6a1b9a"),
        "construction": ("#e3f2fd", "#1565c0"),
        "theorem": ("#e8f5e9", "#2e7d32"),
        "tiered": ("#fff8e1", "#ef6c00"),
        "boundary": ("#f5f5f5", "#546e7a"),
    }

    seed = _box(
        ax, 0.045, 0.665, 0.18, 0.165,
        title="Seed",
        body="quarter-turn read as Z[i]\none explicit modelling root",
        tag="[AXIOM]",
        fill=styles["axiom"][0],
        edge=styles["axiom"][1],
    )
    integer_four = _box(
        ax, 0.285, 0.665, 0.18, 0.165,
        title="Integer 4",
        body="order/unit count\n16 = 4^2 enters the assembly",
        tag="framework data",
        fill=styles["construction"][0],
        edge=styles["construction"][1],
    )
    gstar = _box(
        ax, 0.525, 0.665, 0.18, 0.165,
        title="G*",
        body="Gamma(1/4)/Gamma(3/4)\nfour independent constructions",
        tag="[THEOREM]",
        fill=styles["theorem"][0],
        edge=styles["theorem"][1],
    )
    quadratic = _box(
        ax, 0.765, 0.665, 0.19, 0.165,
        title="Master quadratic",
        body="x^2 - 16G*^2 x + 16G*^3\nroots x+, x-; no alpha readout",
        tag="[THEOREM]",
        fill=styles["theorem"][0],
        edge=styles["theorem"][1],
    )

    _arrow(ax, (seed[0] + 0.09, seed[1]), (integer_four[0] - 0.09, integer_four[1]))
    _arrow(ax, (integer_four[0] + 0.09, integer_four[1]), (gstar[0] - 0.09, gstar[1]))
    _arrow(ax, (gstar[0] + 0.09, gstar[1]), (quadratic[0] - 0.095, quadratic[1]))

    gstar_id = _box(
        ax, 0.085, 0.405, 0.25, 0.145,
        title="G*-identity branch",
        body="T1 G* identity\nT5 Watson identity\nT9 Q(G*) pi-free",
        tag="theorem-grade",
        fill=styles["theorem"][0],
        edge=styles["theorem"][1],
    )
    quadratic_branch = _box(
        ax, 0.385, 0.405, 0.25, 0.145,
        title="Quadratic branch",
        body="T2 master polynomial\nT8 harmonic tower\nT4 coefficient 16, value-level",
        tag="mixed tags",
        fill=styles["tiered"][0],
        edge=styles["tiered"][1],
    )
    geometry_branch = _box(
        ax, 0.685, 0.405, 0.25, 0.145,
        title="Geometry and finite-L branch",
        body="T3 CM uniqueness, tiered\nT6 geometric Coulomb identity\nT7 ultralocality at L = 2",
        tag="mixed tags",
        fill=styles["tiered"][0],
        edge=styles["tiered"][1],
    )
    subsidiaries = _box(
        ax, 0.235, 0.205, 0.25, 0.135,
        title="Subsidiary anchors",
        body="S1 D = 3  |  S2 Moore integers\nS3 a_phys no-go  |  S4 Phase H",
        tag="supporting",
        fill=styles["construction"][0],
        edge=styles["construction"][1],
    )
    ledger = _box(
        ax, 0.545, 0.205, 0.25, 0.135,
        title="LEDGER audit",
        body="each claim keeps its FTD-NNNN row\nand its current epistemic tag",
        tag="provenance",
        fill=styles["boundary"][0],
        edge=styles["boundary"][1],
    )

    _arrow(ax, (gstar[0], gstar[1] - 0.08), (gstar_id[0], gstar_id[1] + 0.07),
           color="#2e7d32", rad=0.08)
    _arrow(ax, (quadratic[0] - 0.03, quadratic[1] - 0.08),
           (quadratic_branch[0], quadratic_branch[1] + 0.07),
           color="#ef6c00", rad=-0.02)
    _arrow(ax, (quadratic[0] + 0.03, quadratic[1] - 0.08),
           (geometry_branch[0], geometry_branch[1] + 0.07),
           color="#ef6c00", rad=-0.08)
    _arrow(ax, (integer_four[0], integer_four[1] - 0.08),
           (subsidiaries[0], subsidiaries[1] + 0.07),
           color="#1565c0", rad=0.08)
    for src in (gstar_id, quadratic_branch, geometry_branch, subsidiaries):
        _arrow(ax, (src[0], src[1] - 0.07), (ledger[0], ledger[1] + 0.065),
               color="#78909c", rad=0.08 if src[0] < ledger[0] else -0.08, lw=1.2)

    audit = mpatches.FancyBboxPatch(
        (0.065, 0.04), 0.87, 0.095,
        boxstyle="round,pad=0.014,rounding_size=0.018",
        facecolor="#ffffff",
        edgecolor="#cfd8dc",
        linewidth=1.0,
        zorder=1,
    )
    ax.add_patch(audit)
    ax.text(
        0.5, 0.092,
        "Audit rule: every theorem-grade node traces back to the seed plus explicit constructions.\n"
        "The physics identification x+ <-> 1/alpha is intentionally not promoted by this map.",
        ha="center", va="center", fontsize=9.2, color="#37474f",
    )

    legend_x = 0.065
    for i, (label, key) in enumerate([
        ("axiom/root", "axiom"),
        ("construction", "construction"),
        ("theorem-grade", "theorem"),
        ("tiered or bounded", "tiered"),
        ("audit/provenance", "boundary"),
    ]):
        fill, edge = styles[key]
        x = legend_x + i * 0.17
        ax.add_patch(mpatches.Rectangle((x, 0.855), 0.022, 0.022,
                                        facecolor=fill, edgecolor=edge, linewidth=1.0))
        ax.text(x + 0.028, 0.866, label, ha="left", va="center",
                fontsize=7.8, color="#455a64")

    _save(fig, "spine_only")
    print("  spine_only.{svg,png}")


def render_arc_b1_context(data: dict) -> None:
    seeds = {"FTD-0198", "FTD-0204", "FTD-0205", "FTD-0206"}
    # Add immediate deps + dependents.
    expand = set(seeds)
    for e in data["edges"]:
        if e["type"] != "ledger-depends-on":
            continue
        if e["from"] in seeds: expand.add(e["to"])
        if e["to"] in seeds:   expand.add(e["from"])
    fig, ax = plt.subplots(figsize=(11, 8), dpi=120)
    _draw(ax, data,
          node_filter=lambda n: n["id"] in expand,
          edge_filter=lambda e: e["type"] in ("theorem-anchors-ledger", "ledger-depends-on"),
          title="ARC-B1 closure context (FTD-0198 / 0204 / 0205 / 0206 cluster)",
          show_labels=expand)
    _save(fig, "arc_b1_context")
    print("  arc_b1_context.{svg,png}")


def render_catalan_frontier(data: dict) -> None:
    seeds = {"FTD-0206", "FTD-0001", "FTD-0002", "FTD-0006", "FTD-0007", "FTD-0013", "FTD-0202"}
    expand = set(seeds)
    for e in data["edges"]:
        if e["type"] != "ledger-depends-on":
            continue
        if e["from"] in seeds: expand.add(e["to"])
        if e["to"] in seeds:   expand.add(e["from"])
    fig, ax = plt.subplots(figsize=(10, 8), dpi=120)
    _draw(ax, data,
          node_filter=lambda n: n["id"] in expand,
          edge_filter=lambda e: e["type"] in ("theorem-anchors-ledger", "ledger-depends-on"),
          title="Catalan algebraic-independence frontier (Conjecture 19.2, FTD-0206)",
          show_labels=expand)
    _save(fig, "catalan_frontier")
    print("  catalan_frontier.{svg,png}")


def render_per_sector(data: dict) -> None:
    sectors = sorted({r["sector"] for r in data["layers"]["ledger"]})
    for sector in sectors:
        rows = [r for r in data["layers"]["ledger"] if r["sector"] == sector]
        if len(rows) < 2:
            continue
        ids = {r["id"] for r in rows}
        # Also include anchored theorems.
        ids |= {t["id"] for t in data["layers"]["theorems"]
                if t.get("ledger_ref") in ids}
        fig, ax = plt.subplots(figsize=(10, 7), dpi=120)
        _draw(ax, data,
              node_filter=lambda n: n["id"] in ids,
              edge_filter=lambda e: e["type"] in ("theorem-anchors-ledger", "ledger-depends-on"),
              title=f"Sector: {sector} ({len(rows)} LEDGER rows)",
              show_labels=ids if len(ids) <= 40 else None)
        safe = sector.replace("/", "_").replace("*", "star")
        _save(fig, f"sector_{safe}")
        print(f"  sector_{safe}.{{svg,png}}")


def main():
    data = json.loads(JSON_PATH.read_text(encoding="utf-8"))
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    print(f"Rendering math node map figures to {OUT_DIR.relative_to(REPO_ROOT)}/")
    render_full_overview(data)
    render_spine_only(data)
    render_arc_b1_context(data)
    render_catalan_frontier(data)
    render_per_sector(data)
    print("Done.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

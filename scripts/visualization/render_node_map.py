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
    spine_ids = {t["id"] for t in data["layers"]["theorems"]}
    anchored_ledger = {t["ledger_ref"] for t in data["layers"]["theorems"] if t.get("ledger_ref")}
    visible_ids = spine_ids | anchored_ledger

    fig, ax = plt.subplots(figsize=(12, 9), dpi=120)
    _draw(ax, data,
          node_filter=lambda n: n["id"] in visible_ids,
          edge_filter=lambda e: e["type"] in ("theorem-anchors-ledger", "ledger-depends-on"),
          title="FTD algebraic spine + anchored LEDGER claims",
          show_labels=visible_ids,
          label_max=30)
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

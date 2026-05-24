"""
Force-directed (Fruchterman-Reingold) layout helper for the math node map.

Computes 2D (x, y) coordinates per node based on the edge structure in
scripts/verification/results/math_node_map.json, and caches them back into
the JSON under each node's `x` / `y` field.

The HTML renderer (math_node_map.html) reads these cached coordinates;
re-runs of build_math_node_map.py + this layout pass regenerate them
deterministically (numpy seed = 42).

Algorithm: ~50-line Fruchterman-Reingold implementation in pure numpy.
No networkx / graphviz dependency.

Usage:
    python dissemination/interactive/math_node_map_layout.py
"""
from __future__ import annotations

import json
import sys
from pathlib import Path

import numpy as np

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")

REPO_ROOT = Path(__file__).resolve().parents[2]
JSON_PATH = REPO_ROOT / "scripts" / "verification" / "results" / "math_node_map.json"


def _collect_node_ids(data: dict) -> list[str]:
    """Stable ordering: theorems first, then ledger, then objects, then identities."""
    ids: list[str] = []
    for layer in ("theorems", "ledger", "objects", "identities"):
        ids.extend(n["id"] for n in data["layers"][layer])
    return ids


def _build_adjacency(data: dict, id_to_idx: dict[str, int]) -> np.ndarray:
    """Build a symmetric adjacency matrix from the edges list."""
    n = len(id_to_idx)
    adj = np.zeros((n, n), dtype=np.float32)
    for e in data["edges"]:
        a = id_to_idx.get(e["from"])
        b = id_to_idx.get(e["to"])
        if a is not None and b is not None and a != b:
            adj[a, b] = 1.0
            adj[b, a] = 1.0
    return adj


def fruchterman_reingold(adj: np.ndarray, iters: int = 200,
                         area: float = 1.0, seed: int = 42) -> np.ndarray:
    """Classic FR layout.  Returns (n, 2) coordinate array in [-0.5, 0.5]^2."""
    n = adj.shape[0]
    rng = np.random.default_rng(seed)
    pos = rng.uniform(-0.5, 0.5, size=(n, 2)).astype(np.float32)

    k = np.sqrt(area / max(n, 1))
    t = 0.1  # initial temperature
    cooling = (t / iters)

    for it in range(iters):
        # Pairwise displacement (vectorised).
        delta = pos[:, None, :] - pos[None, :, :]            # (n, n, 2)
        dist = np.linalg.norm(delta, axis=2) + 1e-9          # (n, n)
        # Repulsive forces ~ k^2 / d for all pairs.
        rep = (k * k) / dist
        # Attractive forces ~ d^2 / k for adjacent pairs.
        att = (dist * dist / k) * adj
        force = rep - att                                    # net magnitude
        # Direction = delta normalised; sum across j.
        disp = (delta / dist[..., None] * force[..., None]).sum(axis=1)
        # Limit step by temperature t.
        disp_norm = np.linalg.norm(disp, axis=1) + 1e-9
        disp = disp / disp_norm[:, None] * np.minimum(disp_norm, t)[:, None]
        pos = pos + disp
        # Cool the temperature.
        t = max(t - cooling, cooling)
        # Keep within bounds.
        pos = np.clip(pos, -0.5, 0.5)
    return pos


def main():
    data = json.loads(JSON_PATH.read_text(encoding="utf-8"))
    ids = _collect_node_ids(data)
    if not ids:
        print("No nodes to lay out.")
        return 1
    id_to_idx = {nid: i for i, nid in enumerate(ids)}
    adj = _build_adjacency(data, id_to_idx)
    print(f"Laying out {len(ids)} nodes with {int(adj.sum() / 2)} undirected edges...")
    pos = fruchterman_reingold(adj, iters=300, seed=42)

    # Write coordinates back into each node record.
    for layer_name in ("theorems", "ledger", "objects", "identities"):
        for node in data["layers"][layer_name]:
            idx = id_to_idx[node["id"]]
            node["x"] = float(pos[idx, 0])
            node["y"] = float(pos[idx, 1])

    JSON_PATH.write_text(
        json.dumps(data, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )
    print(f"Wrote layout to {JSON_PATH.relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

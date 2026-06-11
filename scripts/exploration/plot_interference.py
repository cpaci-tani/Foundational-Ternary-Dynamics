#!/usr/bin/env python3
"""
plot_interference.py -- Visualize the FTD-0110 genesis interference effect.
Runs the wave equation on the 27-block and plots flux magnitudes to show
why BCC corners fire while FCC edges do not.
"""

import numpy as np
import matplotlib.pyplot as plt
from radial_genesis_cascade import StochasticLatticeField, K_GENESIS

def plot_flux_trajectories(A=14, L=16, ticks=15):
    # Use a deterministic run to get the clean wave propagation
    # (By setting a fixed seed, we get a representative trajectory)
    field = StochasticLatticeField(L, A, seed=42)
    
    c = L // 2
    idx_center = field._idx(c, c, c)
    idx_face = field._idx(c+1, c, c)
    idx_edge = field._idx(c+1, c+1, c)
    idx_corner = field._idx(c+1, c+1, c+1)
    
    history_center = []
    history_face = []
    history_edge = []
    history_corner = []
    
    # Track which tick they manifest (if they do)
    manifest_ticks = {'center': None, 'face': None, 'edge': None, 'corner': None}
    
    for t in range(ticks):
        history_center.append(np.linalg.norm(field.flux[idx_center]))
        history_face.append(np.linalg.norm(field.flux[idx_face]))
        history_edge.append(np.linalg.norm(field.flux[idx_edge]))
        history_corner.append(np.linalg.norm(field.flux[idx_corner]))
        
        # Check manifestation
        if field.state[idx_center] != 0 and manifest_ticks['center'] is None: manifest_ticks['center'] = t
        if field.state[idx_face] != 0 and manifest_ticks['face'] is None: manifest_ticks['face'] = t
        if field.state[idx_edge] != 0 and manifest_ticks['edge'] is None: manifest_ticks['edge'] = t
        if field.state[idx_corner] != 0 and manifest_ticks['corner'] is None: manifest_ticks['corner'] = t
        
        field.tick()
        
    plt.figure(figsize=(10, 6))
    
    time = np.arange(ticks)
    plt.plot(time, history_center, 'k-', linewidth=2, label='Center (r=0.0)')
    plt.plot(time, history_face, 'b-', linewidth=2, label='SC Face (r=1.000)')
    plt.plot(time, history_edge, 'r-', linewidth=2, label='FCC Edge (r=1.414) - INTERFERENCE MINIMUM')
    plt.plot(time, history_corner, 'g-', linewidth=2, label='BCC Corner (r=1.732) - CONSTRUCTIVE')
    
    plt.axhline(y=K_GENESIS, color='k', linestyle='--', alpha=0.5, label='K_GENESIS Threshold')
    
    # Mark manifestations
    if manifest_ticks['center'] is not None:
        plt.plot(manifest_ticks['center'], history_center[manifest_ticks['center']], 'ko', markersize=8)
    if manifest_ticks['face'] is not None:
        plt.plot(manifest_ticks['face'], history_face[manifest_ticks['face']], 'bo', markersize=8)
    if manifest_ticks['edge'] is not None:
        plt.plot(manifest_ticks['edge'], history_edge[manifest_ticks['edge']], 'ro', markersize=8)
    if manifest_ticks['corner'] is not None:
        plt.plot(manifest_ticks['corner'], history_corner[manifest_ticks['corner']], 'go', markersize=8)

    plt.title(f'Wave Interference in Genesis Throttle (A={A})')
    plt.xlabel('Ticks')
    plt.ylabel('Flux Magnitude |J|')
    plt.yscale('log')
    plt.grid(True, alpha=0.3)
    plt.legend()
    
    import os
    # Save to the artifacts directory so the AI can embed it
    # We use a hardcoded path based on the conversation ID, or just relative to cwd
    artifact_path = r"C:\Users\cpaci\.gemini\antigravity\brain\284848ef-2c38-4b75-ace8-33b9a87081e1\interference_plot.png"
    plt.savefig(artifact_path, dpi=300, bbox_inches='tight')
    print(f"Plot saved to {artifact_path}")

if __name__ == "__main__":
    plot_flux_trajectories()

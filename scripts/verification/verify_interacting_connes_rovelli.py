"""
Step 7: Interacting Connes-Rovelli Test
==================================================================================

Companion script for SPEC_QFT_GRT_BRIDGE_ROADMAP.md (Step 7 of Critical Path).

Step 7: Re-evaluate the Connes-Rovelli identification (modular time = physical time)
using the Interacting Hamiltonian and the corresponding interacting classical tick.

We test if the presence of the Hermitian momentum coupling (which pushes the 
spectrum to GUE) causes the classical FTD Verlet integrator to align better 
with exact quantum modular flow, or if they diverge faster.

Output:
- Console: verification results
- Figure: docs/papers/src/figures/FTD_Interacting_Connes_Rovelli.png
"""

import numpy as np
import sys
import os
from scipy import linalg as sla

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))

# ============================================================================
# Core Math Operators
# ============================================================================

def build_1d_laplacian(N, periodic=True):
    L = np.zeros((N, N), dtype=float)
    for i in range(N):
        L[i, i] = -2.0
        L[i, (i + 1) % N] = 1.0
        L[i, (i - 1) % N] = 1.0
    if not periodic:
        L[0, N - 1] = 0.0
        L[N - 1, 0] = 0.0
    return L

def build_1d_derivative(N, periodic=True):
    D = np.zeros((N, N), dtype=float)
    for i in range(N):
        D[i, (i + 1) % N] = 0.5
        D[i, (i - 1) % N] = -0.5
    if not periodic:
        D[0, N - 1] = 0.0
        D[N - 1, 0] = 0.0
    return D

def build_interacting_hamiltonian(N, c_wave=0.4, g_c=0.5, p_manifest=np.exp(-np.pi), seed=42):
    rng = np.random.RandomState(seed)
    L = build_1d_laplacian(N)
    H_free = -(c_wave**2 / 2.0) * L
    
    s_vals = rng.choice([-1, 0, 1], size=N, p=[p_manifest/2, 1 - p_manifest, p_manifest/2])
    S = np.diag(s_vals)
    D = build_1d_derivative(N)
    
    H_coupling = -1j * (g_c / 2.0) * (S @ D + D @ S)
    H_full = H_free + H_coupling
    
    return H_full, H_free, H_coupling, L, D, S

# ============================================================================
# Time Evolution
# ============================================================================

def ftd_tick_interacting(psi, psi_vel, L, D, S, c_wave=0.4, g_c=0.5):
    """
    Simulate one FTD tick of wave function evolution with interaction.
    acc = c^2 * L * psi - i * (g_c / 2) * (S*D + D*S) * psi
    """
    acc = (c_wave**2) * (L @ psi) - 1j * (g_c / 2.0) * ((S @ D + D @ S) @ psi)
    
    psi_vel_new = psi_vel + acc
    psi_new = psi + psi_vel_new
    
    return psi_new, psi_vel_new

def modular_time_evolution(psi, H, t):
    """
    Exact unitary evolution: psi(t) = exp(-i*H*t) psi(0)
    """
    U = sla.expm(-1j * H * t)
    return U @ psi

# ============================================================================
# Main Verification
# ============================================================================

def run_section(N=64, c_wave=0.4, g_c=1.0):
    print("=" * 70)
    print("SECTION 7: INTERACTING CONNES-ROVELLI TEST")
    print("=" * 70)
    
    H_full, H_free, H_coupling, L, D, S = build_interacting_hamiltonian(N, c_wave, g_c, seed=123)
    
    # Create test wave function (Gaussian wave packet)
    x = np.arange(N, dtype=float)
    x0 = N / 2
    sigma = N / 8
    k0 = 2 * np.pi / (N / 4)
    psi_0 = np.exp(-(x - x0)**2 / (2 * sigma**2)) * np.exp(1j * k0 * x)
    psi_0 = psi_0 / np.linalg.norm(psi_0)
    
    # --- Interacting FTD tick evolution (1 step) ---
    psi_vel = np.zeros_like(psi_0)
    psi_tick, _ = ftd_tick_interacting(psi_0, psi_vel, L, D, S, c_wave, g_c)
    psi_tick_norm = psi_tick / np.linalg.norm(psi_tick)
    
    # --- High-resolution Modular Flow Overlap Search ---
    # We use a much finer grid to avoid the t=0.0 aliasing from the free-field test
    t_values = np.linspace(0.001, 1.0, 5000)
    overlaps = []
    
    # Precompute eigendecomposition for fast expm
    evals, evecs = np.linalg.eigh(H_full)
    
    for t in t_values:
        U = evecs @ np.diag(np.exp(-1j * evals * t)) @ evecs.conj().T
        psi_mod = U @ psi_0
        psi_mod_norm = psi_mod / np.linalg.norm(psi_mod)
        overlap = np.abs(np.dot(psi_mod_norm.conj(), psi_tick_norm))**2
        overlaps.append(overlap)
        
    overlaps = np.array(overlaps)
    best_idx = np.argmax(overlaps)
    t_best = t_values[best_idx]
    best_overlap = overlaps[best_idx]
    
    # --- High-resolution Modular Flow Overlap Search (FREE) ---
    evals_free, evecs_free = np.linalg.eigh(H_free)
    t_values_free = np.linspace(0.001, 1.0, 5000)
    overlaps_free_search = []
    
    psi_tick_free, _ = ftd_tick_interacting(psi_0, np.zeros_like(psi_0), L, D, S, c_wave, g_c=0.0)
    psi_tick_free_norm = psi_tick_free / np.linalg.norm(psi_tick_free)
    
    for t in t_values_free:
        U = evecs_free @ np.diag(np.exp(-1j * evals_free * t)) @ evecs_free.conj().T
        psi_mod = U @ psi_0
        psi_mod_norm = psi_mod / np.linalg.norm(psi_mod)
        overlap = np.abs(np.dot(psi_mod_norm.conj(), psi_tick_free_norm))**2
        overlaps_free_search.append(overlap)
        
    t_best_free = t_values_free[np.argmax(overlaps_free_search)]

    print(f"  Lattice sites:    {N}")
    print(f"  Coupling g_c:     {g_c}")
    print(f"  Search grid:      {len(t_values)} points between {t_values[0]} and {t_values[-1]}")
    print(f"\n  Best match time (Free):        t_mod = {t_best_free:.6f}")
    print(f"  Best match time (Interacting): t_mod = {t_best:.6f}")
    print(f"  Max Overlap (Interacting):     {best_overlap:.8f}")
    
    # --- Multi-step comparison ---
    print(f"\n  Multi-step divergence test:")
    print(f"  {'Steps':>6} | {'Free Overlap':>15} | {'Interacting Overlap':>20}")
    print("  " + "-" * 55)
    
    steps_to_test = [1, 2, 5, 10, 20]
    
    free_overlaps = []
    int_overlaps = []
    
    for n_steps in steps_to_test:
        # Free FTD Tick
        psi_ftd_free = psi_0.copy()
        vel_free = np.zeros_like(psi_0)
        for _ in range(n_steps):
            psi_ftd_free, vel_free = ftd_tick_interacting(psi_ftd_free, vel_free, L, D, S, c_wave, g_c=0.0)
        psi_ftd_free /= np.linalg.norm(psi_ftd_free)
        
        # Free Modular Flow
        U_free = evecs_free @ np.diag(np.exp(-1j * evals_free * (n_steps * t_best_free))) @ evecs_free.conj().T
        psi_mod_free = U_free @ psi_0
        psi_mod_free /= np.linalg.norm(psi_mod_free)
        
        free_overlap = np.abs(np.dot(psi_mod_free.conj(), psi_ftd_free))**2
        free_overlaps.append(free_overlap)
        
        # Interacting FTD Tick
        psi_ftd_int = psi_0.copy()
        vel_int = np.zeros_like(psi_0)
        for _ in range(n_steps):
            psi_ftd_int, vel_int = ftd_tick_interacting(psi_ftd_int, vel_int, L, D, S, c_wave, g_c)
        psi_ftd_int /= np.linalg.norm(psi_ftd_int)
        
        # Interacting Modular Flow
        U_int = evecs @ np.diag(np.exp(-1j * evals * (n_steps * t_best))) @ evecs.conj().T
        psi_mod_int = U_int @ psi_0
        psi_mod_int /= np.linalg.norm(psi_mod_int)
        
        int_overlap = np.abs(np.dot(psi_mod_int.conj(), psi_ftd_int))**2
        int_overlaps.append(int_overlap)
        
        print(f"  {n_steps:>6} | {free_overlap:>15.8f} | {int_overlap:>20.8f}")

    print("\n  INTERPRETATION:")
    print("  " + "-" * 50)
    # Check divergence rate at step index 3 (which is n_steps=10)
    idx_eval = 3
    if int_overlaps[idx_eval] < free_overlaps[idx_eval] - 0.05:
        print("  [DIVERGENCE SHRINKS? NO] The interacting Hamiltonian causes the")
        print("  classical Verlet integrator to diverge FASTER from the quantum")
        print("  modular flow than the free field did.")
        print("  This confirms the Connes-Rovelli identification is NOT EXACT for")
        print("  the classical lattice discretization.")
    elif int_overlaps[idx_eval] > free_overlaps[idx_eval] + 0.05:
        print("  [DIVERGENCE SHRINKS? YES] The interacting Hamiltonian actually")
        print("  forces the classical tick to align BETTER with quantum modular flow!")
    else:
        print("  [NEUTRAL] The interaction preserves the baseline divergence rate.")
        
    return {
        't_values': t_values,
        'overlaps': overlaps,
        't_best': t_best,
        'steps': steps_to_test,
        'free_overlaps': free_overlaps,
        'int_overlaps': int_overlaps
    }

def generate_figure(res):
    try:
        import matplotlib
        matplotlib.use('Agg')
        import matplotlib.pyplot as plt
    except ImportError:
        print("  [SKIP] matplotlib not available")
        return

    fig, axes = plt.subplots(1, 2, figsize=(12, 5))
    fig.suptitle('Interacting Connes-Rovelli Test', fontsize=14, fontweight='bold')
    
    # Panel A: Overlap search
    ax = axes[0]
    ax.plot(res['t_values'], res['overlaps'], 'b-', lw=2)
    ax.axvline(x=res['t_best'], color='r', linestyle='--', label=f't_mod = {res["t_best"]:.4f}')
    ax.set_title('Single-Tick Fidelity')
    ax.set_xlabel('Modular Time Parameter $t$')
    ax.set_ylabel(r'$|\langle\psi_{tick}|\psi_{mod}(t)\rangle|^2$')
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    # Panel B: Multi-step divergence
    ax = axes[1]
    ax.plot(res['steps'], res['free_overlaps'], 'k.--', markersize=10, label='Free Field')
    ax.plot(res['steps'], res['int_overlaps'], 'r.-', markersize=10, label='Interacting Field')
    ax.set_title('Multi-Step Divergence')
    ax.set_xlabel('Number of FTD Ticks')
    ax.set_ylabel('Fidelity vs Exact Modular Flow')
    ax.legend()
    ax.grid(True, alpha=0.3)
    ax.set_ylim(0, 1.05)
    
    plt.tight_layout()
    fig_dir = os.path.join(os.path.dirname(__file__), '..', '..', 'docs', 'papers', 'src', 'figures')
    os.makedirs(fig_dir, exist_ok=True)
    png_path = os.path.join(fig_dir, 'FTD_Interacting_Connes_Rovelli.png')
    
    fig.savefig(png_path, bbox_inches='tight', dpi=150)
    print(f"  Figure saved: {png_path}")
    plt.close(fig)

def main():
    print("*" * 70)
    print("  FTD INTERACTING CONNES-ROVELLI TEST (QFT-GRT BRIDGE STEP 7)")
    print("*" * 70)
    
    res = run_section(N=128, g_c=0.5)
    generate_figure(res)
    
    print("\n" + "*" * 70)
    print("  VERIFICATION COMPLETE")
    print("*" * 70)

if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""
19-Test Battery for the 27-State Interacting Ternary Cube
==========================================================

H = H1 x I x I + I x H1 x I + I x I x H1 + J(ZZ x I + Z x Z + I x ZZ)

Tests across 8 categories: quantum chaos, eigenstate structure, transport,
entanglement, symmetry, FTD specialness, phase transitions, wild cards.

Epistemic status:
  [THEOREM]     Symmetry verifications, entanglement at J=0
  [SELECTION]   Parameter choices, test design
  [CONJECTURE]  Framework number matches (look-elsewhere caveat applies)
"""

import sys
import os
import io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

import numpy as np
from scipy.signal import find_peaks
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from constants import (
    G_STAR, VARPI_CLASSICAL, GAMMA_QUARTER, GAMMA_HALF,
    N_c, N_base, b_3, N_eff, PF, X_PLUS, X_MINUS
)

OUTPUT_DIR = os.path.join(os.path.dirname(__file__), '..', '..', 'output')
os.makedirs(OUTPUT_DIR, exist_ok=True)

ALPHA = 1.0 / X_PLUS

# =============================================================================
# CORE INFRASTRUCTURE
# =============================================================================

# Ontic chain
PI_D = 4.0 * VARPI_CLASSICAL**2 / G_STAR**2
MU = (PI_D + G_STAR) / 2.0
DELTA_FTD = (PI_D - G_STAR) / 2.0
BETA_FTD = VARPI_CLASSICAL - MU

# 27 states: (a,b,c) with a,b,c in {0,1,2} = {+, 0, -}
STATES = [(a, b, c) for a in range(3) for b in range(3) for c in range(3)]
LABELS_CH = ['+', '0', '-']

def z_val(idx):
    return 1 if idx == 0 else (0 if idx == 1 else -1)

def state_offset(s):
    return tuple(1 if x == 0 else (0 if x == 1 else -1) for x in s)

def moore_shell(s):
    return sum(1 for x in state_offset(s) if x != 0)

SHELL_INDICES = {0: [], 1: [], 2: [], 3: []}
for i, s in enumerate(STATES):
    SHELL_INDICES[moore_shell(s)].append(i)

def build_h1(delta, beta, g):
    return np.array([[delta, g, 0], [g, beta, g], [0, g, -delta]])

def build_z3():
    return np.diag([1.0, 0.0, -1.0])

def build_h27(delta, beta, g, J):
    I3 = np.eye(3)
    H1 = build_h1(delta, beta, g)
    Z = build_z3()
    H = (np.kron(np.kron(H1, I3), I3) +
         np.kron(np.kron(I3, H1), I3) +
         np.kron(np.kron(I3, I3), H1))
    if abs(J) > 1e-15:
        H += J * (np.kron(np.kron(Z, Z), I3) +
                  np.kron(np.kron(Z, I3), Z) +
                  np.kron(np.kron(I3, Z), Z))
    return H

def diag(H):
    return np.linalg.eigh(H)

def evolve_probs(evals, evecs, init_idx, t_vals):
    """Vectorized time evolution. Returns (T, 27) probability array."""
    coeffs = evecs[init_idx, :]  # <init|eig_m>
    phases = np.exp(-1j * np.outer(t_vals, evals))  # (T, 27)
    psi_t = (phases * coeffs[np.newaxis, :]) @ evecs.T  # (T, 27)
    return np.abs(psi_t)**2

def transfer_amplitude(evals, evecs, i, j, t_vals):
    """P(i->j)(t) via spectral decomposition."""
    amp = evecs[i, :].conj() * evecs[j, :]
    phases = np.exp(-1j * np.outer(t_vals, evals))
    return np.abs((phases * amp[np.newaxis, :]).sum(axis=1))**2

def shell_pops(probs):
    """Sum probabilities by Moore shell. probs shape (27,) or (T,27)."""
    if probs.ndim == 1:
        return np.array([probs[SHELL_INDICES[s]].sum() for s in range(4)])
    return np.array([[p[SHELL_INDICES[s]].sum() for s in range(4)] for p in probs])

def bipartite_entropy(psi):
    """Von Neumann entropy tracing out axis 3. psi is 27-vector."""
    M = psi.reshape(9, 3)  # (a,b) x c
    rho = M @ M.conj().T   # 9x9
    eigv = np.linalg.eigvalsh(rho)
    eigv = eigv[eigv > 1e-15]
    return -np.sum(eigv * np.log(eigv))

def header(title, num):
    print()
    print("=" * 78)
    print(f"  TEST {num}: {title}")
    print("=" * 78)

# =============================================================================
# TEST 1: Level Spacing Statistics
# =============================================================================

def test_level_spacing(delta, beta, g):
    header("LEVEL SPACING STATISTICS (r-ratio)", 1)

    fig, axes = plt.subplots(1, 3, figsize=(15, 4))
    J_vals = [0, 0.05, 0.2]

    for panel, J in enumerate(J_vals):
        H = build_h27(delta, beta, g, J)
        evals = np.linalg.eigvalsh(H)

        # Remove near-degeneracies
        spacings = np.diff(evals)
        spacings = spacings[spacings > 1e-8]

        if len(spacings) < 3:
            print(f"  J={J}: too few non-degenerate spacings ({len(spacings)})")
            continue

        # Normalize
        s = spacings / spacings.mean()

        # r-ratio
        r_vals = []
        for i in range(len(s) - 1):
            r = min(s[i], s[i+1]) / max(s[i], s[i+1])
            r_vals.append(r)
        r_mean = np.mean(r_vals) if r_vals else 0

        print(f"  J={J:.3f}: {len(s)} spacings, <r> = {r_mean:.4f} "
              f"(Poisson=0.386, GOE=0.536)")

        ax = axes[panel]
        ax.hist(s, bins=10, density=True, alpha=0.7, color='steelblue', edgecolor='black')
        x = np.linspace(0, max(s)*1.1, 200)
        ax.plot(x, np.exp(-x), 'r--', label='Poisson', linewidth=2)
        ax.plot(x, (np.pi*x/2)*np.exp(-np.pi*x**2/4), 'g--', label='Wigner', linewidth=2)
        ax.set_title(f'J={J} (<r>={r_mean:.3f})', fontsize=10)
        ax.set_xlabel('s (normalized spacing)')
        ax.legend(fontsize=8)
        ax.set_xlim(0, max(3, max(s)*1.1))

    fig.suptitle('Level Spacing Statistics: Integrable (J=0) vs Interacting', fontsize=12)
    fig.tight_layout()
    path = os.path.join(OUTPUT_DIR, 'ternary_cube_tests_level_spacing.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print(f"  Figure: {path}")


# =============================================================================
# TEST 2: Spectral Form Factor
# =============================================================================

def test_sff(delta, beta, g):
    header("SPECTRAL FORM FACTOR", 2)

    fig, ax = plt.subplots(figsize=(8, 5))
    t_vals = np.logspace(-1, 2.5, 3000)

    for J, color, label in [(0, 'steelblue', 'J=0'), (0.05, 'orange', 'J=0.05'),
                             (0.2, 'red', 'J=0.2')]:
        evals = np.linalg.eigvalsh(build_h27(delta, beta, g, J))
        sff = np.abs(np.sum(np.exp(-1j * np.outer(t_vals, evals)), axis=1))**2 / 27**2
        ax.plot(t_vals, sff, color=color, label=label, linewidth=0.8, alpha=0.8)

    ax.axhline(1.0/27, color='gray', linestyle='--', label='Plateau = 1/27')
    ax.set_xscale('log')
    ax.set_yscale('log')
    ax.set_xlabel('t')
    ax.set_ylabel('SFF(t)')
    ax.set_title('Spectral Form Factor: |Tr(e^{-iHt})|^2 / N^2')
    ax.legend()
    ax.set_ylim(1e-4, 1.5)

    path = os.path.join(OUTPUT_DIR, 'ternary_cube_tests_sff.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print(f"  Figure: {path}")


# =============================================================================
# TEST 3: Inverse Participation Ratio
# =============================================================================

def test_ipr(delta, beta, g):
    header("INVERSE PARTICIPATION RATIO", 3)

    fig, ax = plt.subplots(figsize=(9, 5))
    colors = ['#44aaff', '#44cc66', '#ffd54f', '#ff8844', '#ff4432']
    J_vals = [0, 0.05, 0.1, 0.2, 0.5]

    for J, c in zip(J_vals, colors):
        evals, evecs = diag(build_h27(delta, beta, g, J))
        iprs = np.sum(np.abs(evecs)**4, axis=0)  # IPR for each eigenstate
        ax.scatter(range(27), iprs, c=c, s=30, label=f'J={J}', alpha=0.7, zorder=3)

    ax.axhline(1.0/27, color='gray', linestyle='--', label='Fully delocalized')
    ax.axhline(1.0/9, color='gray', linestyle=':', alpha=0.5, label='One shell (9)')
    ax.set_xlabel('Eigenstate index (by energy)')
    ax.set_ylabel('IPR = sum |psi|^4')
    ax.set_title('Inverse Participation Ratio vs Interaction Strength')
    ax.legend(fontsize=8)

    path = os.path.join(OUTPUT_DIR, 'ternary_cube_tests_ipr.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print(f"  Figure: {path}")


# =============================================================================
# TEST 4: Shell Projection
# =============================================================================

def test_shell_projection(delta, beta, g):
    header("SHELL PROJECTION OF EIGENSTATES", 4)

    fig, axes = plt.subplots(1, 2, figsize=(14, 5))
    shell_colors = ['#ffd54f', '#44aaff', '#44cc66', '#ff4432']
    shell_names = ['Center', 'SC', 'FCC', 'BCC']

    for panel, (J, title) in enumerate([(0, 'J=0 (separable)'), (0.1, 'J=0.1 (interacting)')]):
        evals, evecs = diag(build_h27(delta, beta, g, J))
        ax = axes[panel]

        bottoms = np.zeros(27)
        for s in range(4):
            weights = np.sum(np.abs(evecs[SHELL_INDICES[s], :])**2, axis=0)
            ax.bar(range(27), weights, bottom=bottoms, color=shell_colors[s],
                   label=shell_names[s], width=0.8)
            bottoms += weights

        # Flag shell-pure states
        n_pure = 0
        for n in range(27):
            for s in range(4):
                w = np.sum(np.abs(evecs[SHELL_INDICES[s], n])**2)
                if w > 0.95:
                    n_pure += 1
        print(f"  J={J}: {n_pure} shell-pure eigenstates (>95% on one shell)")

        ax.set_xlabel('Eigenstate index')
        ax.set_ylabel('Shell weight')
        ax.set_title(title)
        ax.legend(fontsize=8, loc='upper right')

    fig.suptitle('Moore Shell Projection of Eigenstates', fontsize=12)
    fig.tight_layout()
    path = os.path.join(OUTPUT_DIR, 'ternary_cube_tests_shell_projection.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print(f"  Figure: {path}")


# =============================================================================
# TEST 5: Shannon Entropy
# =============================================================================

def test_shannon_entropy(delta, beta, g):
    header("SHANNON ENTROPY OF EIGENSTATES", 5)

    S_max = np.log(27)
    print(f"  S_max = ln(27) = {S_max:.6f}")
    print()

    for J in [0, 0.05, 0.1]:
        evals, evecs = diag(build_h27(delta, beta, g, J))
        print(f"  J={J}:")
        print(f"  {'#':>4} {'Energy':>12} {'Shannon S':>12} {'S/S_max':>10}")
        print("  " + "-" * 40)
        for n in range(27):
            p = np.abs(evecs[:, n])**2
            p = p[p > 1e-15]
            S = -np.sum(p * np.log(p))
            print(f"  {n:>4} {evals[n]:>12.6f} {S:>12.6f} {S/S_max:>10.4f}")
        print()


# =============================================================================
# TEST 6: Corner-to-Corner Transfer Heatmap
# =============================================================================

def test_corner_transfer(delta, beta):
    header("CORNER-TO-CORNER TRANSFER HEATMAP", 6)

    ng, nj = 30, 30
    g_vals = np.linspace(0.01, 1.0, ng)
    j_vals = np.linspace(0, 0.3, nj)
    t_vals = np.linspace(0, 100, 500)
    heatmap = np.zeros((nj, ng))

    for ig, g in enumerate(g_vals):
        for ij, J in enumerate(j_vals):
            evals, evecs = diag(build_h27(delta, beta, g, J))
            tr = transfer_amplitude(evals, evecs, 0, 26, t_vals)
            heatmap[ij, ig] = tr.max()

    fig, ax = plt.subplots(figsize=(9, 6))
    im = ax.pcolormesh(g_vals, j_vals, heatmap, cmap='inferno', shading='auto')
    plt.colorbar(im, ax=ax, label='Max P(|+++> -> |--->)')
    ax.plot(DELTA_FTD, 0.05, '*', color='cyan', markersize=15, zorder=5, label='FTD point')
    ax.set_xlabel('g (coupling)')
    ax.set_ylabel('J (inter-axis)')
    ax.set_title('Corner-to-Opposite-Corner Max Transfer')
    ax.legend()

    path = os.path.join(OUTPUT_DIR, 'ternary_cube_tests_corner_transfer.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)

    # Report FTD value
    evals, evecs = diag(build_h27(delta, beta, DELTA_FTD, 0.05))
    tr_ftd = transfer_amplitude(evals, evecs, 0, 26, t_vals).max()
    print(f"  FTD max corner transfer: {tr_ftd:.6f}")
    print(f"  Global max in heatmap: {heatmap.max():.6f}")
    print(f"  Figure: {path}")


# =============================================================================
# TEST 7: Quantum Speed Limit
# =============================================================================

def test_speed_limit(delta, beta, J):
    header("QUANTUM SPEED LIMIT", 7)

    g_vals = np.linspace(0.02, 1.5, 60)
    t_vals = np.linspace(0, 200, 2000)
    threshold = 0.5

    fig, ax = plt.subplots(figsize=(9, 5))

    for Jv, color, label in [(0, 'steelblue', 'J=0'), (J, 'orange', f'J={J}')]:
        transit_times = []
        qsl_bounds = []
        for g in g_vals:
            H = build_h27(delta, beta, g, Jv)
            evals, evecs = diag(H)
            tr = transfer_amplitude(evals, evecs, 0, 26, t_vals)
            above = np.where(tr >= threshold)[0]
            transit = t_vals[above[0]] if len(above) > 0 else np.nan
            transit_times.append(transit)

            # Mandelstam-Tamm bound
            psi0 = np.zeros(27); psi0[0] = 1.0
            E_mean = psi0 @ H @ psi0
            E2_mean = psi0 @ H @ H @ psi0
            dE = np.sqrt(E2_mean - E_mean**2)
            qsl = np.pi / (2 * dE) if dE > 1e-10 else np.nan
            qsl_bounds.append(qsl)

        ax.plot(g_vals, transit_times, 'o-', color=color, markersize=3, label=f'Actual ({label})')
        ax.plot(g_vals, qsl_bounds, '--', color=color, alpha=0.5, label=f'MT bound ({label})')

    ax.set_xlabel('g')
    ax.set_ylabel('Transit time to P > 0.5')
    ax.set_title('Quantum Speed Limit: |+++> to |--->')
    ax.legend(fontsize=8)
    ax.set_ylim(0, 200)

    path = os.path.join(OUTPUT_DIR, 'ternary_cube_tests_speed_limit.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print(f"  Figure: {path}")


# =============================================================================
# TEST 8: Revival Analysis
# =============================================================================

def test_revival(delta, beta, g, J):
    header("REVIVAL ANALYSIS", 8)

    H = build_h27(delta, beta, g, J)
    evals, evecs = diag(H)
    psi0 = np.zeros(27, dtype=complex); psi0[0] = 1.0
    coeffs = evecs.T @ psi0

    t_vals = np.linspace(0, 500, 5000)
    phases = np.exp(-1j * np.outer(t_vals, evals))
    overlap = np.abs(np.sum(phases * np.abs(coeffs[np.newaxis, :])**2, axis=1))**2

    # Find peaks
    peaks, props = find_peaks(overlap, height=0.3, prominence=0.05)
    peak_times = t_vals[peaks]

    print(f"  Found {len(peaks)} revival peaks above 0.3")
    if len(peaks) > 0:
        print(f"  Peak times: {', '.join(f'{t:.1f}' for t in peak_times[:10])}")
        intervals = np.diff(peak_times)
        if len(intervals) > 0:
            print(f"  Intervals: {', '.join(f'{d:.1f}' for d in intervals[:8])}")

    fig, ax = plt.subplots(figsize=(10, 4))
    ax.plot(t_vals, overlap, 'steelblue', linewidth=0.8)
    if len(peaks) > 0:
        ax.plot(peak_times, overlap[peaks], 'ro', markersize=5, label=f'{len(peaks)} revivals')
    ax.set_xlabel('t')
    ax.set_ylabel('|<psi(0)|psi(t)>|^2')
    ax.set_title(f'Autocorrelation / Revival (g={g}, J={J})')
    ax.legend()

    path = os.path.join(OUTPUT_DIR, 'ternary_cube_tests_revival.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print(f"  Figure: {path}")


# =============================================================================
# TEST 9: Bipartite Entanglement Entropy
# =============================================================================

def test_entanglement(delta, beta, g):
    header("BIPARTITE ENTANGLEMENT ENTROPY", 9)

    fig, axes = plt.subplots(1, 2, figsize=(14, 5))
    S_max = np.log(3)

    for panel, (J, title) in enumerate([(0, 'J=0 (must be zero)'), (0.1, 'J=0.1')]):
        evals, evecs = diag(build_h27(delta, beta, g, J))
        entropies = []
        for n in range(27):
            psi = evecs[:, n]
            entropies.append(bipartite_entropy(psi))

        ax = axes[panel]
        colors = ['#ff4432' if s > 0.01 else '#44aaff' for s in entropies]
        ax.bar(range(27), entropies, color=colors, width=0.8)
        ax.axhline(S_max, color='gray', linestyle='--', label=f'S_max=ln(3)={S_max:.3f}')
        ax.set_xlabel('Eigenstate index')
        ax.set_ylabel('Bipartite S')
        ax.set_title(title)
        ax.legend(fontsize=8)

        n_entangled = sum(1 for s in entropies if s > 0.01)
        print(f"  J={J}: {n_entangled}/27 entangled eigenstates, max S = {max(entropies):.6f}")

    fig.suptitle('Bipartite Entanglement Entropy (trace out axis 3)', fontsize=12)
    fig.tight_layout()
    path = os.path.join(OUTPUT_DIR, 'ternary_cube_tests_entanglement.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print(f"  Figure: {path}")


# =============================================================================
# TEST 10: Ground State Entanglement vs J
# =============================================================================

def test_gs_entanglement(delta, beta, g):
    header("GROUND STATE ENTANGLEMENT vs J", 10)

    J_vals = np.linspace(0, 0.5, 100)
    S_gs = []
    gaps = []

    for J in J_vals:
        evals, evecs = diag(build_h27(delta, beta, g, J))
        S_gs.append(bipartite_entropy(evecs[:, 0]))
        gaps.append(evals[1] - evals[0])

    fig, ax1 = plt.subplots(figsize=(9, 5))
    ax2 = ax1.twinx()
    ax1.plot(J_vals, S_gs, 'steelblue', linewidth=2, label='S_GS (entropy)')
    ax2.plot(J_vals, gaps, 'r--', linewidth=1.5, label='E1-E0 (gap)')
    ax1.set_xlabel('J')
    ax1.set_ylabel('Bipartite entropy S', color='steelblue')
    ax2.set_ylabel('Spectral gap', color='red')
    ax1.set_title('Ground State Entanglement and Spectral Gap vs J')
    lines1, labels1 = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax1.legend(lines1 + lines2, labels1 + labels2, loc='upper left')

    i_max = np.argmax(S_gs)
    print(f"  Max S_GS = {S_gs[i_max]:.6f} at J = {J_vals[i_max]:.4f}")
    print(f"  Gap at max S: {gaps[i_max]:.6f}")

    path = os.path.join(OUTPUT_DIR, 'ternary_cube_tests_gs_entanglement.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print(f"  Figure: {path}")


# =============================================================================
# TEST 11: Parity Sectors
# =============================================================================

def test_parity(delta, beta, g, J):
    header("PARITY SECTORS", 11)

    # Build inversion operator P: |a,b,c> -> |2-a,2-b,2-c>
    P = np.zeros((27, 27))
    for a in range(3):
        for b in range(3):
            for c in range(3):
                n = a*9 + b*3 + c
                m = (2-a)*9 + (2-b)*3 + (2-c)
                P[n, m] = 1.0

    assert np.allclose(P @ P, np.eye(27)), "P^2 != I"

    H = build_h27(delta, beta, g, J)
    commutator = np.linalg.norm(H @ P - P @ H)
    print(f"  ||[H, P]|| = {commutator:.2e}  (should be ~0)")

    # Projectors
    P_even = (np.eye(27) + P) / 2
    P_odd = (np.eye(27) - P) / 2
    dim_even = int(round(np.trace(P_even)))
    dim_odd = int(round(np.trace(P_odd)))
    print(f"  Even sector: dim = {dim_even}")
    print(f"  Odd sector:  dim = {dim_odd}")
    print(f"  Total: {dim_even + dim_odd} = 27")

    # Diagonalize each sector
    evals_full = np.linalg.eigvalsh(H)
    H_even = P_even @ H @ P_even
    H_odd = P_odd @ H @ P_odd
    evals_even = np.sort(np.linalg.eigvalsh(H_even))
    evals_odd = np.sort(np.linalg.eigvalsh(H_odd))

    # Filter out zeros from projection
    evals_even_real = evals_even[np.abs(evals_even) > 1e-12]
    evals_odd_real = evals_odd[np.abs(evals_odd) > 1e-12]
    # Actually need eigenvalues of the restricted matrix, not the projected one
    # Use eigenvalues of H in the even subspace
    evals_h, evecs_h = diag(H)
    parity_eigs = np.array([evecs_h[:, n] @ P @ evecs_h[:, n] for n in range(27)])
    n_even = np.sum(parity_eigs > 0.5)
    n_odd = np.sum(parity_eigs < -0.5)
    print(f"  Eigenstates with parity +1: {n_even}")
    print(f"  Eigenstates with parity -1: {n_odd}")

    # Ground state parity
    gs_parity = evecs_h[:, 0] @ P @ evecs_h[:, 0]
    print(f"  Ground state parity: {gs_parity:+.6f}")


# =============================================================================
# TEST 12: S_3 Permutation Sectors
# =============================================================================

def test_permutation(delta, beta, g, J):
    header("S_3 PERMUTATION SECTORS", 12)

    # Build transposition operators
    def build_swap(perm):
        S = np.zeros((27, 27))
        for a in range(3):
            for b in range(3):
                for c in range(3):
                    abc = [a, b, c]
                    pbc = [abc[perm[0]], abc[perm[1]], abc[perm[2]]]
                    n = a*9 + b*3 + c
                    m = pbc[0]*9 + pbc[1]*3 + pbc[2]
                    S[n, m] = 1.0
        return S

    P12 = build_swap([1, 0, 2])  # swap axes 1,2
    P13 = build_swap([2, 1, 0])  # swap axes 1,3
    P23 = build_swap([0, 2, 1])  # swap axes 2,3

    H = build_h27(delta, beta, g, J)

    for name, Pop in [('P12', P12), ('P13', P13), ('P23', P23)]:
        comm = np.linalg.norm(H @ Pop - Pop @ H)
        print(f"  ||[H, {name}]|| = {comm:.2e}")

    # Count distinct eigenvalues of P12 acting on eigenstates
    evals_h, evecs_h = diag(H)
    p12_eigs = np.array([evecs_h[:, n] @ P12 @ evecs_h[:, n] for n in range(27)])

    # S_3 has irreps: trivial (dim 1), sign (dim 1), standard (dim 2)
    # In 27-dim rep, count multiplicities
    # Symmetric states: invariant under all permutations
    S_sym = (np.eye(27) + P12 + P13 + P23 +
             P12 @ P13 + P13 @ P12) / 6
    dim_sym = int(round(np.trace(S_sym)))
    print(f"  Fully symmetric (trivial rep): dim = {dim_sym}")

    # Antisymmetric
    S_anti = (np.eye(27) - P12 - P13 - P23 +
              P12 @ P13 + P13 @ P12) / 6
    dim_anti = int(round(np.trace(S_anti)))
    print(f"  Fully antisymmetric (sign rep): dim = {dim_anti}")

    dim_standard = 27 - dim_sym - dim_anti
    print(f"  Standard rep (remainder): dim = {dim_standard}")
    print(f"  Total: {dim_sym} + {dim_anti} + {dim_standard} = 27")


# =============================================================================
# TEST 13: Dark States
# =============================================================================

def test_dark_states(delta, beta, g):
    header("DARK STATES (zero amplitude on center |000>)", 13)

    center_idx = 13  # |000>
    print(f"  {'J':>6} {'Dark count':>12} {'Dark energies (first 5)':>40}")
    print("  " + "-" * 60)

    for J in [0, 0.05, 0.1, 0.2, 0.5]:
        evals, evecs = diag(build_h27(delta, beta, g, J))
        center_amps = np.abs(evecs[center_idx, :])**2
        dark = np.where(center_amps < 1e-6)[0]
        energies_str = ', '.join(f'{evals[d]:.4f}' for d in dark[:5])
        print(f"  {J:>6.3f} {len(dark):>12} {energies_str:>40}")


# =============================================================================
# TEST 14: Monte Carlo Comparison
# =============================================================================

def test_monte_carlo(delta, beta, g, J):
    header("MONTE CARLO: IS FTD SPECIAL?", 14)
    print("  [SELECTION] Parameter ranges are a modeling choice, not a derivation.")

    t_vals = np.linspace(0, 100, 300)
    N_samples = 1000

    # FTD figure of merit
    evals_ftd, evecs_ftd = diag(build_h27(delta, beta, g, J))
    F_ftd = transfer_amplitude(evals_ftd, evecs_ftd, 0, 26, t_vals).max()

    # Random samples
    rng = np.random.default_rng(42)
    F_random = []
    for _ in range(N_samples):
        d = rng.uniform(0.01, 0.5)
        b = rng.uniform(-1.0, 0.5)
        gi = rng.uniform(0.01, 1.0)
        Ji = rng.uniform(0, 0.3)
        ev, evc = diag(build_h27(d, b, gi, Ji))
        F_random.append(transfer_amplitude(ev, evc, 0, 26, t_vals).max())

    F_random = np.array(F_random)
    percentile = np.mean(F_random <= F_ftd) * 100

    print(f"  FTD max corner transfer: {F_ftd:.6f}")
    print(f"  Random mean: {F_random.mean():.6f}, std: {F_random.std():.6f}")
    print(f"  FTD percentile rank: {percentile:.1f}%")

    fig, ax = plt.subplots(figsize=(8, 5))
    ax.hist(F_random, bins=40, color='steelblue', alpha=0.7, edgecolor='black', density=True)
    ax.axvline(F_ftd, color='red', linewidth=2, label=f'FTD = {F_ftd:.4f} ({percentile:.0f}th %ile)')
    ax.set_xlabel('Max corner-to-corner transfer')
    ax.set_ylabel('Density')
    ax.set_title(f'FTD vs {N_samples} Random Parameter Sets')
    ax.legend()

    path = os.path.join(OUTPUT_DIR, 'ternary_cube_tests_monte_carlo.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print(f"  Figure: {path}")


# =============================================================================
# TEST 15: Framework Number Hunt
# =============================================================================

def test_framework_numbers(delta, beta, g, J):
    header("FRAMEWORK NUMBER HUNT", 15)
    print("  [CONJECTURE] Near-misses are observations, not derivations.")
    print("  Look-elsewhere: ~700 comparisons x ~15 targets => ~10 expected at 0.1%")
    print()

    evals = np.linalg.eigvalsh(build_h27(delta, beta, g, J))
    targets = {
        'N_c': N_c, 'N_base': N_base, 'b_3': b_3, 'N_eff': N_eff,
        '1/alpha': X_PLUS, 'x_minus': X_MINUS, 'G*': G_STAR,
        'varpi': VARPI_CLASSICAL, 'pi': PI_D, '27': 27, '47': 47,
        'phi': (1+np.sqrt(5))/2,
    }

    matches = []
    tol = 0.001  # 0.1%

    # Check ratios
    for i in range(27):
        for j in range(27):
            if i == j or abs(evals[j]) < 0.01:
                continue
            ratio = evals[i] / evals[j]
            for name, val in targets.items():
                if abs(val) < 0.01:
                    continue
                if abs(ratio / val - 1) < tol:
                    matches.append(('ratio', f'E[{i}]/E[{j}]', ratio, name, val))

    # Check gaps
    for i in range(27):
        for j in range(i+1, 27):
            gap = abs(evals[j] - evals[i])
            if gap < 0.01:
                continue
            for name, val in targets.items():
                if abs(val) < 0.01:
                    continue
                if abs(gap / val - 1) < tol:
                    matches.append(('gap', f'|E[{j}]-E[{i}]|', gap, name, val))

    print(f"  Matches found at 0.1% tolerance: {len(matches)}")
    if matches:
        print(f"  {'Type':<8} {'Expression':<18} {'Value':>12} {'Target':>10} {'Target val':>12} {'Error':>10}")
        print("  " + "-" * 72)
        for mtype, expr, val, tname, tval in matches[:20]:
            err = abs(val / tval - 1) * 100
            print(f"  {mtype:<8} {expr:<18} {val:>12.6f} {tname:>10} {tval:>12.6f} {err:>9.4f}%")


# =============================================================================
# TEST 16: Energy Gap vs J
# =============================================================================

def test_energy_gap(delta, beta, g):
    header("ENERGY GAP vs J", 16)

    J_vals = np.linspace(0, 1.0, 200)
    gap1 = []
    gap2 = []

    for J in J_vals:
        evals = np.linalg.eigvalsh(build_h27(delta, beta, g, J))
        gap1.append(evals[1] - evals[0])
        gap2.append(evals[2] - evals[0])

    fig, ax = plt.subplots(figsize=(9, 5))
    ax.plot(J_vals, gap1, 'steelblue', linewidth=2, label='E1 - E0')
    ax.plot(J_vals, gap2, 'orange', linewidth=2, label='E2 - E0')
    ax.axhline(0, color='gray', linestyle='--', alpha=0.5)
    ax.set_xlabel('J')
    ax.set_ylabel('Energy gap')
    ax.set_title('Spectral Gap vs Inter-Axis Coupling')
    ax.legend()

    min_gap1 = min(gap1)
    j_min = J_vals[np.argmin(gap1)]
    print(f"  Min gap (E1-E0) = {min_gap1:.6f} at J = {j_min:.4f}")

    path = os.path.join(OUTPUT_DIR, 'ternary_cube_tests_energy_gap.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print(f"  Figure: {path}")


# =============================================================================
# TEST 17: Order Parameter
# =============================================================================

def test_order_parameter(delta, beta, g):
    header("ORDER PARAMETER: <ZZ> and <Z>", 17)

    I3 = np.eye(3)
    Z = build_z3()
    ZZ_12 = np.kron(np.kron(Z, Z), I3)
    ZZ_13 = np.kron(np.kron(Z, I3), Z)
    ZZ_23 = np.kron(np.kron(I3, Z), Z)
    Z_1 = np.kron(np.kron(Z, I3), I3)

    J_vals = np.linspace(0, 1.0, 200)
    zz_avg = []
    z1_avg = []

    for J in J_vals:
        evals, evecs = diag(build_h27(delta, beta, g, J))
        gs = evecs[:, 0]
        zz = (gs @ ZZ_12 @ gs + gs @ ZZ_13 @ gs + gs @ ZZ_23 @ gs) / 3
        z1 = gs @ Z_1 @ gs
        zz_avg.append(zz)
        z1_avg.append(z1)

    fig, ax = plt.subplots(figsize=(9, 5))
    ax.plot(J_vals, zz_avg, 'steelblue', linewidth=2, label='<ZZ> (pair correlation)')
    ax.plot(J_vals, z1_avg, 'orange', linewidth=2, label='<Z_1> (single-site)')
    ax.axhline(0, color='gray', linestyle='--', alpha=0.5)
    ax.set_xlabel('J')
    ax.set_ylabel('Expectation value')
    ax.set_title('Order Parameter vs Inter-Axis Coupling')
    ax.legend()

    path = os.path.join(OUTPUT_DIR, 'ternary_cube_tests_order_parameter.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print(f"  Figure: {path}")


# =============================================================================
# TEST 18: Loschmidt Echo
# =============================================================================

def test_loschmidt(delta, beta, g, J):
    header("LOSCHMIDT ECHO", 18)

    H1 = build_h27(delta, beta, g, J)
    evals1, evecs1 = diag(H1)
    psi0 = np.zeros(27); psi0[0] = 1.0
    c1 = evecs1.T @ psi0

    t_vals = np.linspace(0, 200, 2000)
    fig, ax = plt.subplots(figsize=(10, 5))

    for dJ, color in [(0.001, 'steelblue'), (0.01, 'orange'), (0.05, 'red')]:
        H2 = build_h27(delta, beta, g, J + dJ)
        evals2, evecs2 = diag(H2)
        c2 = evecs2.T @ psi0
        overlap = evecs2.T @ evecs1  # <m2|n1>

        echo = np.zeros(len(t_vals))
        for ti, t in enumerate(t_vals):
            fwd = c1 * np.exp(-1j * evals1 * t)  # coeffs in H1 basis at time t
            state_in_2 = overlap @ fwd  # transform to H2 basis
            bwd = state_in_2 * np.exp(+1j * evals2 * t)  # reverse evolve
            echo[ti] = np.abs(np.dot(c2.conj(), bwd))**2

        ax.plot(t_vals, echo, color=color, linewidth=0.8, label=f'dJ={dJ}')

    ax.set_xlabel('t')
    ax.set_ylabel('Loschmidt echo L(t)')
    ax.set_title('Loschmidt Echo: Sensitivity to Perturbation')
    ax.set_yscale('log')
    ax.set_ylim(1e-4, 1.5)
    ax.legend()

    path = os.path.join(OUTPUT_DIR, 'ternary_cube_tests_loschmidt.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print(f"  Figure: {path}")


# =============================================================================
# TEST 19: Specific Heat (Schottky Anomalies)
# =============================================================================

def test_specific_heat(delta, beta, g, J):
    header("SPECIFIC HEAT (SCHOTTKY ANOMALIES)", 19)

    evals = np.linalg.eigvalsh(build_h27(delta, beta, g, J))
    E = evals - evals[0]  # shift so E_0 = 0

    T_vals = np.logspace(-2, 1.5, 300)
    C_vals = []

    for T in T_vals:
        boltz = np.exp(-E / T)
        Z = np.sum(boltz)
        U = np.sum(E * boltz) / Z
        E2 = np.sum(E**2 * boltz) / Z
        C = (E2 - U**2) / T**2
        C_vals.append(C)

    C_vals = np.array(C_vals)

    # Find peaks
    peaks, _ = find_peaks(C_vals, prominence=0.1)
    peak_T = T_vals[peaks]
    peak_C = C_vals[peaks]

    print(f"  Schottky peaks at T = {', '.join(f'{t:.4f}' for t in peak_T)}")
    print(f"  Peak heights: {', '.join(f'{c:.4f}' for c in peak_C)}")

    fig, ax = plt.subplots(figsize=(9, 5))
    ax.plot(T_vals, C_vals, 'steelblue', linewidth=2)
    if len(peaks) > 0:
        ax.plot(peak_T, peak_C, 'ro', markersize=8, zorder=5)
        for t, c in zip(peak_T, peak_C):
            ax.annotate(f'T={t:.3f}', (t, c), (t*1.5, c+0.3), fontsize=8,
                       arrowprops=dict(arrowstyle='->', color='red'))
    ax.set_xscale('log')
    ax.set_xlabel('Temperature T')
    ax.set_ylabel('Specific heat C(T)')
    ax.set_title('Specific Heat: Schottky Anomalies Reveal Energy Scales')

    path = os.path.join(OUTPUT_DIR, 'ternary_cube_tests_specific_heat.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print(f"  Figure: {path}")


# =============================================================================
# MAIN
# =============================================================================

if __name__ == "__main__":
    print("=" * 78)
    print("  27-STATE TERNARY CUBE: 19-TEST BATTERY")
    print("  H = H1xIxI + IxH1xI + IxIxH1 + J(ZZxI + ZxIZ + IxZZ)")
    print("=" * 78)
    print()
    print(f"  FTD parameters:")
    print(f"    Delta = {DELTA_FTD:.10f}")
    print(f"    beta  = {BETA_FTD:.10f}")
    print(f"    mu    = {MU:.10f}")
    print(f"    g_default = Delta = {DELTA_FTD:.6f}")
    print(f"    J_default = 0.05")

    delta = DELTA_FTD
    beta = BETA_FTD
    g = DELTA_FTD  # g = Delta (matched coupling)
    J = 0.05       # weak interaction

    tests = [
        ("Level spacing",       lambda: test_level_spacing(delta, beta, g)),
        ("Spectral form factor",lambda: test_sff(delta, beta, g)),
        ("IPR",                 lambda: test_ipr(delta, beta, g)),
        ("Shell projection",    lambda: test_shell_projection(delta, beta, g)),
        ("Shannon entropy",     lambda: test_shannon_entropy(delta, beta, g)),
        ("Corner transfer",     lambda: test_corner_transfer(delta, beta)),
        ("Speed limit",         lambda: test_speed_limit(delta, beta, J)),
        ("Revival",             lambda: test_revival(delta, beta, g, J)),
        ("Entanglement",        lambda: test_entanglement(delta, beta, g)),
        ("GS entanglement",     lambda: test_gs_entanglement(delta, beta, g)),
        ("Parity sectors",      lambda: test_parity(delta, beta, g, J)),
        ("S_3 permutation",     lambda: test_permutation(delta, beta, g, J)),
        ("Dark states",         lambda: test_dark_states(delta, beta, g)),
        ("Monte Carlo",         lambda: test_monte_carlo(delta, beta, g, J)),
        ("Framework numbers",   lambda: test_framework_numbers(delta, beta, g, J)),
        ("Energy gap vs J",     lambda: test_energy_gap(delta, beta, g)),
        ("Order parameter",     lambda: test_order_parameter(delta, beta, g)),
        ("Loschmidt echo",      lambda: test_loschmidt(delta, beta, g, J)),
        ("Specific heat",       lambda: test_specific_heat(delta, beta, g, J)),
    ]

    passed = 0
    failed = 0
    for i, (name, fn) in enumerate(tests):
        try:
            fn()
            passed += 1
        except Exception as e:
            print(f"\n  *** TEST {i+1} ({name}) FAILED: {e}")
            import traceback
            traceback.print_exc()
            failed += 1

    print()
    print("=" * 78)
    print(f"  BATTERY COMPLETE: {passed}/19 passed, {failed} failed")
    print("=" * 78)

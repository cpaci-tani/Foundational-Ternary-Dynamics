#!/usr/bin/env python3
"""
Cube Chain Experiments: N coupled ternary cubes
================================================

Center-tunnel coupling won the 2-cube comparison.
Now scale up: chains of N=2..10 cubes with periodic boundaries.

Experiments:
  1. Entanglement scaling S(N) — volume law or area law?
  2. Band structure E(k) — dispersion, effective mass, speed of sound
  3. Correlation function <n_center(0) * n_center(r)> — correlation length
  4. Excitation propagation — wavefront speed vs c_CFL
  5. Ground state structure — how does the vacuum entangle across the chain?
  6. Phase diagram — coupling strength vs chain behavior
"""

import numpy as np
import sys, os, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from scipy.sparse import lil_matrix, eye as speye
from scipy.sparse.linalg import eigsh

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from constants import G_STAR, VARPI_CLASSICAL, N_c, b_3, N_eff

OUTPUT_DIR = os.path.join(os.path.dirname(__file__), '..', '..', 'output')
os.makedirs(OUTPUT_DIR, exist_ok=True)

PI_D = 4.0 * VARPI_CLASSICAL**2 / G_STAR**2
delta = (PI_D - G_STAR) / 2.0
beta = VARPI_CLASSICAL - (PI_D + G_STAR) / 2.0
g_hop = delta
J_int = 0.05
c_cfl = 1.0 / np.sqrt(3)

I3 = np.eye(3)
Z3 = np.diag([1., 0., -1.])
I27 = np.eye(27)

STATES = [(a, b, c) for a in range(3) for b in range(3) for c in range(3)]

def moore_shell(s):
    off = tuple(1 if x == 0 else (0 if x == 1 else -1) for x in s)
    return sum(1 for x in off if x != 0)

SHELL_IDX = {sh: [i for i in range(27) if moore_shell(STATES[i]) == sh] for sh in range(4)}
SHELL_NAMES = ['Center', 'SC/U(1)', 'FCC/SU(2)', 'BCC/SU(3)']
SHELL_COLORS = ['#ffd54f', '#44aaff', '#44cc66', '#ff4432']

CENTER = 13  # |000>

def build_h27():
    H1 = np.array([[delta, g_hop, 0], [g_hop, beta, g_hop], [0, g_hop, -delta]])
    H = (np.kron(np.kron(H1, I3), I3) +
         np.kron(np.kron(I3, H1), I3) +
         np.kron(np.kron(I3, I3), H1))
    H += J_int * (np.kron(np.kron(Z3, Z3), I3) +
                  np.kron(np.kron(Z3, I3), Z3) +
                  np.kron(np.kron(I3, Z3), Z3))
    return H

H_SINGLE = build_h27()
EV_SINGLE, EVC_SINGLE = np.linalg.eigh(H_SINGLE)


# =============================================================================
# BLOCH HAMILTONIAN (single-particle band structure for ANY N)
# =============================================================================

def bloch_hamiltonian(k, t_inter):
    """H(k) = H_single + t * V_hop * e^{ik} + t * V_hop^dag * e^{-ik}
    Center-tunnel: V_hop[a,a] = 1 for all a (identity = full state swap)."""
    V_hop = t_inter * I27  # center-tunnel = all states can swap
    return H_SINGLE + V_hop * np.exp(1j * k) + V_hop.conj().T * np.exp(-1j * k)


def compute_bands(t_inter, n_k=100):
    """Compute all 27 bands E_n(k) over the Brillouin zone."""
    k_vals = np.linspace(-np.pi, np.pi, n_k)
    bands = np.zeros((n_k, 27))
    for i, k in enumerate(k_vals):
        bands[i, :] = np.linalg.eigvalsh(bloch_hamiltonian(k, t_inter))
    return k_vals, bands


# =============================================================================
# N-CUBE CHAIN (sparse, periodic boundaries)
# =============================================================================

def build_chain_sparse(N, t_inter):
    """Build N*27 x N*27 Hamiltonian for periodic chain. Returns sparse matrix."""
    dim = N * 27
    H = lil_matrix((dim, dim))

    # On-site: each cube gets H_single
    for n in range(N):
        offset = n * 27
        for i in range(27):
            for j in range(27):
                if abs(H_SINGLE[i, j]) > 1e-15:
                    H[offset + i, offset + j] = H_SINGLE[i, j]

    # Nearest-neighbor hopping (center-tunnel: all states swap)
    for n in range(N):
        n_next = (n + 1) % N  # periodic
        for a in range(27):
            H[n * 27 + a, n_next * 27 + a] += t_inter
            H[n_next * 27 + a, n * 27 + a] += t_inter

    return H.tocsr()


def build_chain_dense(N, t_inter):
    """Dense version for small N."""
    dim = N * 27
    H = np.zeros((dim, dim))
    for n in range(N):
        o = n * 27
        H[o:o+27, o:o+27] = H_SINGLE
    for n in range(N):
        o1 = n * 27
        o2 = ((n + 1) % N) * 27
        for a in range(27):
            H[o1 + a, o2 + a] += t_inter
            H[o2 + a, o1 + a] += t_inter
    return H


# =============================================================================
# EXPERIMENT 1: BAND STRUCTURE
# =============================================================================

def experiment_band_structure(t_inter=0.05):
    print('=' * 78)
    print('  EXPERIMENT 1: BAND STRUCTURE E(k)')
    print('=' * 78)
    print()

    fig, axes = plt.subplots(1, 3, figsize=(15, 5))

    for pi, t in enumerate([0.01, 0.05, 0.2]):
        k_vals, bands = compute_bands(t, n_k=200)
        ax = axes[pi]

        # Color bands by which shell the corresponding eigenstate comes from at k=0
        ev0, evc0 = np.linalg.eigh(bloch_hamiltonian(0, t))
        for n in range(27):
            # Determine dominant shell of this band at k=0
            probs = np.abs(evc0[:, n])**2
            shell_w = [probs[SHELL_IDX[s]].sum() for s in range(4)]
            dominant = np.argmax(shell_w)
            ax.plot(k_vals, bands[:, n], color=SHELL_COLORS[dominant],
                    linewidth=1, alpha=0.7)

        ax.set_xlabel('k')
        ax.set_ylabel('E(k)')
        ax.set_title('t = %.3f' % t, fontsize=10)
        ax.set_xlim(-np.pi, np.pi)

        # Bandwidths
        bws = [bands[:, n].max() - bands[:, n].min() for n in range(27)]
        print('  t=%.3f: max bandwidth = %.6f, mean = %.6f' %
              (t, max(bws), np.mean(bws)))

        # Effective mass at k=0: m* = hbar^2 / (d^2E/dk^2)
        # d^2E/dk^2 at k=0: finite difference
        dk = k_vals[1] - k_vals[0]
        mid = len(k_vals) // 2
        for n in [0, 13, 26]:  # lowest, middle, highest band
            d2E = (bands[mid+1, n] - 2*bands[mid, n] + bands[mid-1, n]) / dk**2
            meff = 1.0 / d2E if abs(d2E) > 1e-10 else float('inf')
            print('    Band %d: curvature = %.4f, m* = %.4f' % (n, d2E, meff))

    # Speed of sound: v_s = dE/dk at k=0 for the lowest acoustic band
    k_vals, bands = compute_bands(0.05, n_k=200)
    mid = len(k_vals) // 2
    dk = k_vals[1] - k_vals[0]
    v_s = abs(bands[mid+1, 0] - bands[mid-1, 0]) / (2 * dk)
    print()
    print('  Speed of sound (lowest band at t=0.05): v_s = %.6f' % v_s)
    print('  CFL speed: c = %.6f' % c_cfl)
    print('  Ratio v_s / c = %.4f' % (v_s / c_cfl))

    fig.suptitle('Band Structure: 27 Bloch Bands (colored by Moore shell)', fontsize=11)
    fig.tight_layout()
    path = os.path.join(OUTPUT_DIR, 'cube_chain_bands.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print('  Figure: %s' % path)
    print()


# =============================================================================
# EXPERIMENT 2: ENTANGLEMENT SCALING
# =============================================================================

def experiment_entanglement_scaling(t_inter=0.05):
    print('=' * 78)
    print('  EXPERIMENT 2: ENTANGLEMENT SCALING S(N)')
    print('=' * 78)
    print()
    print('  NOTE: This is a single-particle model (dim = N*27, not 27^N).')
    print('  Entanglement is measured as the effective spread of the GS')
    print('  across cubes, using the participation entropy.')
    print()

    N_vals = [2, 3, 4, 5, 6, 8, 10]
    S_vals = []
    ipr_vals = []

    for N in N_vals:
        dim = N * 27
        if dim <= 300:
            H = build_chain_dense(N, t_inter)
            ev, evc = np.linalg.eigh(H)
            gs = evc[:, 0]
        else:
            H = build_chain_sparse(N, t_inter)
            ev, evc = eigsh(H, k=1, which='SA')
            gs = evc[:, 0]

        probs = np.abs(gs)**2

        # Cube-level probabilities: how spread is the GS across cubes?
        cube_probs = np.array([probs[m*27:(m+1)*27].sum() for m in range(N)])
        cube_probs = cube_probs[cube_probs > 1e-15]

        # Shannon entropy of cube distribution
        S_cube = -np.sum(cube_probs * np.log(cube_probs))
        S_vals.append(S_cube)

        # IPR across cubes
        ipr = np.sum(cube_probs**2)
        ipr_vals.append(ipr)

        # Internal entropy: within each cube, how spread across shells?
        print('  N=%2d (%4d states): S_cube = %.6f  IPR_cube = %.6f  (1/IPR = %.1f cubes)' %
              (N, dim, S_cube, ipr, 1.0/ipr))

    print()

    # Scaling
    if len(S_vals) >= 3:
        coeffs = np.polyfit(np.log(N_vals[:len(S_vals)]), S_vals, 1)
        print('  Fit: S_cube = %.4f * ln(N) + %.4f' % (coeffs[0], coeffs[1]))
        print('  S ~ ln(N) = area law in 1D (logarithmic).')
        print('  S ~ N = volume law.')
        print()

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(13, 5))

    ax1.plot(N_vals[:len(S_vals)], S_vals, 'o-', color='steelblue', linewidth=2, markersize=8)
    ax1.plot(N_vals[:len(S_vals)], [np.log(N) for N in N_vals[:len(S_vals)]],
             '--', color='gray', alpha=0.5, label='ln(N)')
    ax1.set_xlabel('N (number of cubes)')
    ax1.set_ylabel('S_cube (Shannon entropy of cube distribution)')
    ax1.set_title('Ground State Spread Across Chain')
    ax1.legend()

    ax2.plot(N_vals[:len(ipr_vals)], [1.0/ipr for ipr in ipr_vals],
             'o-', color='orange', linewidth=2, markersize=8)
    ax2.plot(N_vals[:len(ipr_vals)], N_vals[:len(ipr_vals)],
             '--', color='gray', alpha=0.5, label='N (fully delocalized)')
    ax2.set_xlabel('N')
    ax2.set_ylabel('1/IPR (effective number of cubes)')
    ax2.set_title('Participation: How Many Cubes Does GS Occupy?')
    ax2.legend()

    fig.tight_layout()
    path = os.path.join(OUTPUT_DIR, 'cube_chain_entanglement.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print('  Figure: %s' % path)
    print()


# =============================================================================
# EXPERIMENT 3: CORRELATION FUNCTION
# =============================================================================

def experiment_correlations(t_inter=0.05):
    print('=' * 78)
    print('  EXPERIMENT 3: VACUUM CORRELATION FUNCTION')
    print('=' * 78)
    print()

    N = 10  # 270 states
    H = build_chain_dense(N, t_inter)
    ev, evc = np.linalg.eigh(H)
    gs = evc[:, 0]
    probs = np.abs(gs)**2

    # C(r) = <n_center(0) * n_center(r)> - <n_center(0)><n_center(r)>
    # n_center(site m) = 1 if cube m is in state CENTER, 0 otherwise

    # <n_center(m)> = sum over all joint states where cube m is in CENTER
    def center_expectation(site):
        total = 0
        for idx in range(N * 27):
            # Which cube states? idx = sum of cube_states
            # idx in dense representation: for a chain of N cubes each with 27 states,
            # state idx decomposes as: cube 0 has state idx % 27, cube 1 has (idx//27) % 27, ...
            # WAIT: that's only true for a tensor product basis, which is what build_chain_dense uses.
            # Actually no: build_chain_dense uses a SITE basis where idx = cube*27 + internal_state.
            # So the state vector has dimension N*27, NOT 27^N.
            # This is a SINGLE-PARTICLE model, not a many-body tensor product!
            pass

        # Our chain Hamiltonian is in the SINGLE-PARTICLE sector.
        # It's a 1-particle model: one excitation hopping on N*27 sites.
        # The ground state is a single-particle state spread over all N*27 sites.
        # "n_center(m)" = projector onto center state of cube m = P[m*27 + CENTER]
        return probs[site * 27 + CENTER]

    # Single-particle correlations
    means = [center_expectation(m) for m in range(N)]
    print('  <n_center(m)> for m = 0..%d:' % (N-1))
    for m in range(N):
        print('    site %d: %.6f' % (m, means[m]))

    # In a single-particle model, <n(0)*n(r)> = 0 for r != 0
    # (the particle can only be in one place)
    # The correlator is trivially 0 - <n(0)><n(r)> = -<n(0)><n(r)>
    # This is the WRONG framework for two-point correlations.
    # We need to put this in the MANY-BODY context.

    print()
    print('  NOTE: The chain Hamiltonian is in the single-particle sector.')
    print('  Two-point correlations require the many-body formulation.')
    print('  Instead, computing WAVEFUNCTION SPREAD:')
    print()

    # Wavefunction amplitude on center state of each cube
    center_amps = np.array([abs(gs[m * 27 + CENTER])**2 for m in range(N)])
    print('  Ground state center-site amplitude by cube:')
    for m in range(N):
        bar = '#' * int(center_amps[m] * 200)
        print('    cube %d: %.6f  %s' % (m, center_amps[m], bar))

    # Shell distribution of GS across the chain
    print()
    print('  Ground state shell distribution (averaged over all cubes):')
    for sh in range(4):
        total = sum(probs[m * 27 + i] for m in range(N) for i in SHELL_IDX[sh])
        print('    %s: %.6f' % (SHELL_NAMES[sh], total))

    # Decay of overlap: how does the GS amplitude decay with cube distance?
    # For periodic chain, look at the amplitude pattern
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(13, 5))

    ax1.bar(range(N), center_amps, color='#ffd54f', edgecolor='black')
    ax1.set_xlabel('Cube index')
    ax1.set_ylabel('P(center)')
    ax1.set_title('Ground State: Center Amplitude per Cube (N=%d)' % N)

    # Full shell profile
    for sh in range(4):
        shell_profile = [sum(probs[m * 27 + i] for i in SHELL_IDX[sh]) for m in range(N)]
        ax2.plot(range(N), shell_profile, 'o-', color=SHELL_COLORS[sh],
                 label=SHELL_NAMES[sh], markersize=5)
    ax2.set_xlabel('Cube index')
    ax2.set_ylabel('Shell population')
    ax2.set_title('Ground State: Shell Profile Across Chain')
    ax2.legend(fontsize=8)

    fig.tight_layout()
    path = os.path.join(OUTPUT_DIR, 'cube_chain_correlations.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print('  Figure: %s' % path)
    print()


# =============================================================================
# EXPERIMENT 4: EXCITATION PROPAGATION
# =============================================================================

def experiment_propagation(t_inter=0.05):
    print('=' * 78)
    print('  EXPERIMENT 4: EXCITATION PROPAGATION')
    print('=' * 78)
    print()

    N = 10
    dim = N * 27
    H = build_chain_dense(N, t_inter)
    ev, evc = np.linalg.eigh(H)

    # Initial: BCC excitation on cube 0
    psi0 = np.zeros(dim, dtype=complex)
    psi0[0 * 27 + 0] = 1.0  # |+++> on cube 0

    coeffs = evc.T @ psi0
    t_vals = np.linspace(0, 100, 300)

    # Track center-of-mass of excitation
    cube_pops = np.zeros((len(t_vals), N))
    shell_on_wavefront = np.zeros((len(t_vals), 4))

    for ti, t in enumerate(t_vals):
        phases = np.exp(-1j * ev * t)
        psi_t = evc @ (coeffs * phases)
        probs = np.abs(psi_t)**2

        for m in range(N):
            cube_pops[ti, m] = probs[m*27:(m+1)*27].sum()

        # Shell analysis of the leading cube (first cube with >1% population, beyond cube 0)
        for m in range(1, N):
            if cube_pops[ti, m] > 0.01:
                for sh in range(4):
                    shell_on_wavefront[ti, sh] = sum(probs[m*27+i] for i in SHELL_IDX[sh])
                break

    # Wavefront position: center of mass
    positions = np.arange(N)
    com = np.array([np.sum(cube_pops[ti, :] * positions) / max(np.sum(cube_pops[ti, :]), 1e-15)
                    for ti in range(len(t_vals))])

    # Wavefront speed: derivative of COM
    dt = t_vals[1] - t_vals[0]
    velocity = np.gradient(com, dt)

    fig, axes = plt.subplots(2, 2, figsize=(13, 10))

    # Heatmap of cube populations over time
    ax = axes[0, 0]
    im = ax.pcolormesh(positions, t_vals, cube_pops, cmap='inferno', shading='auto')
    plt.colorbar(im, ax=ax, label='P(cube)')
    ax.set_xlabel('Cube index')
    ax.set_ylabel('Time t')
    ax.set_title('Excitation Heatmap')

    # Center of mass
    ax = axes[0, 1]
    ax.plot(t_vals, com, 'steelblue', linewidth=2)
    ax.set_xlabel('t')
    ax.set_ylabel('Center of mass (cube index)')
    ax.set_title('Wavefront Position')

    # Velocity
    ax = axes[1, 0]
    ax.plot(t_vals, velocity, 'orange', linewidth=1)
    ax.axhline(c_cfl, color='red', linestyle='--', label='c_CFL = 1/sqrt(3)')
    ax.set_xlabel('t')
    ax.set_ylabel('Velocity (cubes/tick)')
    ax.set_title('Wavefront Velocity')
    ax.legend()
    ax.set_ylim(-0.1, 1.0)

    # Shell composition of wavefront
    ax = axes[1, 1]
    for sh in range(4):
        ax.plot(t_vals, shell_on_wavefront[:, sh], color=SHELL_COLORS[sh],
                linewidth=1.5, label=SHELL_NAMES[sh])
    ax.set_xlabel('t')
    ax.set_ylabel('Shell population on wavefront')
    ax.set_title('What Force Carries the Excitation?')
    ax.legend(fontsize=8)

    fig.suptitle('Excitation Propagation: BCC on Cube 0 -> Chain (N=%d)' % N, fontsize=12)
    fig.tight_layout()
    path = os.path.join(OUTPUT_DIR, 'cube_chain_propagation.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)

    # Report peak velocity
    v_max = np.max(np.abs(velocity[10:]))  # skip initial transient
    print('  Peak propagation velocity: %.6f cubes/tick' % v_max)
    print('  CFL speed: %.6f' % c_cfl)
    print('  Ratio v/c: %.4f' % (v_max / c_cfl))
    print('  Figure: %s' % path)
    print()


# =============================================================================
# EXPERIMENT 5: SPECTRAL GAP SCALING
# =============================================================================

def experiment_gap_scaling(t_inter=0.05):
    print('=' * 78)
    print('  EXPERIMENT 5: SPECTRAL GAP SCALING')
    print('=' * 78)
    print()

    N_vals = [2, 3, 4, 5, 6, 8, 10]
    gaps = []

    for N in N_vals:
        dim = N * 27
        if dim <= 300:
            H = build_chain_dense(N, t_inter)
            ev = np.linalg.eigvalsh(H)
            gap = ev[1] - ev[0]
        else:
            H = build_chain_sparse(N, t_inter)
            ev = eigsh(H, k=4, which='SA', return_eigenvectors=False)
            ev = np.sort(ev)
            gap = ev[1] - ev[0]

        gaps.append(gap)
        print('  N=%2d (%4d states): gap = %.8f' % (N, dim, gap))

    print()

    # Scaling: gap ~ 1/N (gapless) or gap ~ const (gapped)?
    if len(gaps) >= 3:
        # Fit gap = a / N + b
        N_arr = np.array(N_vals[:len(gaps)], dtype=float)
        gap_arr = np.array(gaps)
        coeffs = np.polyfit(1.0 / N_arr, gap_arr, 1)
        print('  Fit: gap = %.6f/N + %.6f' % (coeffs[0], coeffs[1]))
        print('  Extrapolated gap at N->inf: %.6f' % coeffs[1])
        if coeffs[1] > 0.01:
            print('  -> GAPPED (massive excitations)')
        else:
            print('  -> possibly GAPLESS (massless excitations)')
        print()

    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(N_vals[:len(gaps)], gaps, 'o-', color='steelblue', linewidth=2, markersize=8)
    ax.set_xlabel('N (chain length)')
    ax.set_ylabel('Spectral gap (E1 - E0)')
    ax.set_title('Spectral Gap vs Chain Length')

    path = os.path.join(OUTPUT_DIR, 'cube_chain_gap.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print('  Figure: %s' % path)
    print()


# =============================================================================
# EXPERIMENT 6: COUPLING PHASE DIAGRAM
# =============================================================================

def experiment_phase_diagram():
    print('=' * 78)
    print('  EXPERIMENT 6: COUPLING PHASE DIAGRAM')
    print('=' * 78)
    print()

    N = 4  # 108 states, fast
    t_vals = np.linspace(0, 0.5, 50)
    gaps = []
    entropies = []
    bandwidths = []

    for t in t_vals:
        H = build_chain_dense(N, t)
        ev, evc = np.linalg.eigh(H)
        gaps.append(ev[1] - ev[0])

        # Cube-spread entropy
        gs = evc[:, 0]
        probs = np.abs(gs)**2
        cube_p = np.array([probs[m*27:(m+1)*27].sum() for m in range(N)])
        cube_p = cube_p[cube_p > 1e-15]
        entropies.append(-np.sum(cube_p * np.log(cube_p)))

        # Bandwidth from Bloch
        _, bands = compute_bands(t, n_k=50)
        bw = max(bands[:, n].max() - bands[:, n].min() for n in range(27))
        bandwidths.append(bw)

    fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(9, 10), sharex=True)

    ax1.plot(t_vals, gaps, 'steelblue', linewidth=2)
    ax1.set_ylabel('Spectral gap')
    ax1.set_title('Phase Diagram: N=%d Chain vs Coupling Strength' % N)
    ax1.axvline(0.05, color='red', linestyle='--', alpha=0.5, label='FTD default')
    ax1.legend()

    ax2.plot(t_vals, entropies, 'orange', linewidth=2)
    ax2.set_ylabel('Entanglement S')
    ax2.axhline(np.log(2), color='gray', linestyle='--', alpha=0.3, label='ln(2)')
    ax2.legend()

    ax3.plot(t_vals, bandwidths, '#44cc66', linewidth=2)
    ax3.set_xlabel('t_inter (coupling strength)')
    ax3.set_ylabel('Max bandwidth')

    fig.tight_layout()
    path = os.path.join(OUTPUT_DIR, 'cube_chain_phase_diagram.png')
    fig.savefig(path, dpi=150, bbox_inches='tight')
    plt.close(fig)

    # Find any phase transition (gap minimum)
    min_gap_idx = np.argmin(gaps)
    print('  Min gap = %.6f at t = %.4f' % (gaps[min_gap_idx], t_vals[min_gap_idx]))
    print('  Max entanglement = %.6f at t = %.4f' %
          (max(entropies), t_vals[np.argmax(entropies)]))
    print('  Figure: %s' % path)
    print()


# =============================================================================
# MAIN
# =============================================================================

if __name__ == "__main__":
    print('=' * 78)
    print('  CUBE CHAIN EXPERIMENTS')
    print('  N coupled ternary cubes (center-tunnel model)')
    print('=' * 78)
    print()

    experiment_band_structure()
    experiment_entanglement_scaling()
    experiment_correlations()
    experiment_propagation()
    experiment_gap_scaling()
    experiment_phase_diagram()

    print('=' * 78)
    print('  ALL EXPERIMENTS COMPLETE')
    print('=' * 78)

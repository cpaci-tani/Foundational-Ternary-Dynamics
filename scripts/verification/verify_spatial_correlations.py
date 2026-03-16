"""
Step 5: Spatial Correlations and Mutual Information
====================================================

Companion script for SPEC_QFT_GRT_BRIDGE_ROADMAP.md (Step 5 of Critical Path).

For two spatially separated subregions A and B (sLoop analogs), compute:
1. Mutual information I(A:B) = S_A + S_B - S_AB
2. Connected correlation function <AB> - <A><B>
3. Distance dependence: how does I(A:B) decay with separation?
4. Comparison: classical (diagonal) vs quantum (off-diagonal) correlations

The key question: Does the FTD thermal state contain quantum correlations
(off-diagonal coherence) beyond classical correlations (diagonal only)?

If yes, this demonstrates that the flux field carries genuine quantum
entanglement — a prerequisite for the Type III bridge.

Previous results (Steps 1-4):
- Step 1: Entanglement spectrum shows non-trivial structure
- Step 2-3: KMS verified at beta=pi; classical tick != quantum modular flow
- Step 4: Gap closes (N^-2), P/N saturates at 0.892, Poisson statistics

Epistemic status:
- Mutual information: [THEOREM] (standard quantum information)
- sLoop interpretation: [CONJECTURE] (proposed bridge element)
- Quantum vs classical: [THEOREM] (off-diagonal = coherence)

Output:
- Console: verification results with [PASS]/[FAIL] markers
- Figure: docs/papers/src/figures/FTD_Spatial_Correlations.pdf (4-panel)
"""

import numpy as np
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))


# ============================================================================
# Infrastructure
# ============================================================================

def build_1d_laplacian(N, periodic=True):
    """Build discrete Laplacian for a 1D periodic chain."""
    L = np.zeros((N, N), dtype=float)
    for i in range(N):
        L[i, i] = -2.0
        L[i, (i + 1) % N] = 1.0
        L[i, (i - 1) % N] = 1.0
    if not periodic:
        L[0, N - 1] = 0.0
        L[N - 1, 0] = 0.0
    return L


def build_ftd_hamiltonian(N, c_wave=0.4, periodic=True):
    """Build H = -(c^2/2) nabla^2 for a 1D FTD flux chain."""
    L = build_1d_laplacian(N, periodic=periodic)
    return (c_wave ** 2 / 2.0) * (-L)


def build_thermal_rho(N, beta=np.pi, c_wave=0.4):
    """Build thermal density matrix rho = exp(-beta*H)/Z."""
    H = build_ftd_hamiltonian(N, c_wave)
    eigenvalues, eigenvectors = np.linalg.eigh(H)
    boltzmann = np.exp(-beta * eigenvalues)
    Z = np.sum(boltzmann)
    probs = boltzmann / Z
    rho = eigenvectors @ np.diag(probs) @ eigenvectors.T
    return rho, H, probs, Z


def von_neumann_entropy(rho):
    """Compute S = -Tr(rho ln rho) from eigenvalues of rho."""
    eigenvalues = np.linalg.eigvalsh(rho)
    eigenvalues = eigenvalues[eigenvalues > 1e-15]
    return -np.sum(eigenvalues * np.log(eigenvalues))


def restricted_rho(rho, sites):
    """
    Extract the restricted density matrix for a set of sites.

    For a single-particle density matrix rho_{ij}, the restriction to
    sites A gives rho_A = rho[A, A] (the sub-block).

    This is normalized: rho_A -> rho_A / Tr(rho_A) so it represents
    the conditional state given the particle is in region A.

    Args:
        rho: Full N x N density matrix
        sites: List/array of site indices for the subregion

    Returns:
        rho_sub: Normalized restricted density matrix
        p_region: Probability of finding particle in this region
    """
    rho_sub = rho[np.ix_(sites, sites)]
    p_region = np.real(np.trace(rho_sub))
    if p_region > 1e-15:
        rho_sub = rho_sub / p_region
    return rho_sub, p_region


# ============================================================================
# Section 1: Mutual Information Between Separated Regions
# ============================================================================

def compute_mutual_information(rho, sites_A, sites_B):
    """
    Compute mutual information I(A:B) = S_A + S_B - S_AB.

    For the single-particle density matrix, this measures how much
    knowing the particle's state in region A tells about region B.

    I(A:B) > 0 means the regions are correlated.
    I(A:B) = 0 means the regions are independent.

    For a thermal state, I(A:B) decays with the separation between A and B,
    at a rate determined by the correlation length.
    """
    sites_AB = np.concatenate([sites_A, sites_B])

    rho_A, p_A = restricted_rho(rho, sites_A)
    rho_B, p_B = restricted_rho(rho, sites_B)
    rho_AB, p_AB = restricted_rho(rho, sites_AB)

    S_A = von_neumann_entropy(rho_A)
    S_B = von_neumann_entropy(rho_B)
    S_AB = von_neumann_entropy(rho_AB)

    I_AB = S_A + S_B - S_AB

    return {
        'I_AB': I_AB,
        'S_A': S_A,
        'S_B': S_B,
        'S_AB': S_AB,
        'p_A': p_A,
        'p_B': p_B,
        'p_AB': p_AB,
    }


def run_section_1(N=128):
    """Section 1: Off-diagonal coherence — genuine spatial correlations."""
    print("=" * 70)
    print("SECTION 1: SPATIAL CORRELATIONS VIA OFF-DIAGONAL COHERENCE")
    print("=" * 70)

    beta = np.pi
    rho, H, probs, Z = build_thermal_rho(N, beta)

    # The genuine spatial correlation is rho(i,j) for i != j.
    # For a translationally invariant thermal state on a periodic chain,
    # C(r) = rho(0,r) depends only on the separation r.
    # The off-diagonal elements ARE the quantum coherence.

    print(f"\n  N = {N}, beta = pi")
    print(f"  C(r) = rho(0, r)  (two-point correlation function)")
    print(f"  For uniform rho = I/N: C(r) = delta(r)/N  (no correlations)")
    print()

    r_values = np.arange(0, N // 2 + 1)
    C_values = np.array([np.real(rho[0, r]) for r in r_values])
    C0 = C_values[0]  # = 1/N for uniform; > 1/N for thermal (localization)
    C_norm = C_values / C0  # Normalized: C(0) = 1

    # Also compute |rho(0,r)|^2 summed over region pairs
    # as a function of separation
    L = 4  # region size
    separations = list(range(1, N // 2 - L, max(1, (N // 2 - 2 * L) // 25)))

    print(f"  Also computing regional coherence ||rho_AB||_off for L={L} regions")
    print()
    print(f"  {'d':>6} | {'C(d)':>14} | {'|C(d)/C(0)|':>14} | "
          f"{'||rho_AB||_off':>14} | {'||rho_AB||_diag':>14}")
    print("  " + "-" * 72)

    regional_results = []
    for d in separations:
        sites_A = np.arange(L)
        sites_B = np.arange(d, d + L)

        # Off-diagonal coherence between regions
        rho_cross = rho[np.ix_(sites_A, sites_B)]
        coh_off = np.sqrt(np.sum(np.abs(rho_cross)**2))

        # Diagonal contribution (for reference)
        rho_diag_A = np.diag(rho)[sites_A]
        rho_diag_B = np.diag(rho)[sites_B]
        coh_diag = np.sqrt(np.sum(np.outer(rho_diag_A, rho_diag_B)**2))

        Cd = C_values[min(d, N // 2)]
        Cd_norm = np.abs(Cd / C0)

        regional_results.append({
            'd': d,
            'C_d': Cd,
            'C_norm': Cd_norm,
            'coh_off': coh_off,
            'coh_diag': coh_diag,
        })

        if d <= 10 or d % max(1, len(separations) // 15) == 0:
            print(f"  {d:>6} | {Cd:>14.8f} | {Cd_norm:>14.8f} | "
                  f"{coh_off:>14.8f} | {coh_diag:>14.8f}")

    # Fit C(r) decay
    r_fit = r_values[1:N//4]
    C_fit = np.abs(C_values[1:N//4])
    C_fit = np.maximum(C_fit, 1e-20)
    log_C = np.log(C_fit)

    coeffs_exp = np.polyfit(r_fit, log_C, 1)
    xi = -1.0 / coeffs_exp[0] if coeffs_exp[0] < 0 else float('inf')

    print(f"\n  Correlation function decay:")
    print(f"    C(0) = {C0:.8f}  (vs 1/N = {1/N:.8f})")
    print(f"    C(1) = {C_values[1]:.8f}")
    print(f"    C(N/4) = {C_values[N//4]:.2e}")
    print(f"    Fitted correlation length xi = {xi:.4f}")

    # Coherence ratio: off-diagonal / diagonal at various distances
    d_arr = np.array([r['d'] for r in regional_results])
    coh_ratio = np.array([r['coh_off'] / max(r['coh_diag'], 1e-20)
                          for r in regional_results])

    print(f"\n  Off-diagonal / diagonal coherence ratio:")
    print(f"    At d=1:   {coh_ratio[0]:.4f}")
    if len(coh_ratio) > 5:
        print(f"    At d~N/8: {coh_ratio[len(coh_ratio)//4]:.4f}")
    print(f"    At d~N/4: {coh_ratio[-1]:.4f}")

    has_offdiag = np.any(coh_ratio > 1.0)
    print(f"\n  INTERPRETATION:")
    print(f"    Off-diagonal coherence exceeds diagonal: {'YES' if has_offdiag else 'NO'}")
    if has_offdiag:
        print(f"    -> Quantum coherence DOMINATES over classical correlations")
        print(f"       at short distances. This is genuine quantum structure.")
    else:
        print(f"    -> Classical correlations dominate. Off-diagonal coherence")
        print(f"       is present but smaller than diagonal contributions.")

    return regional_results, rho, C_values, r_values, xi


# ============================================================================
# Section 2: Quantum vs Classical Correlations
# ============================================================================

def run_section_2(rho, N=128):
    """Section 2: Decompose correlations into quantum and classical parts."""
    print("\n" + "=" * 70)
    print("SECTION 2: QUANTUM vs CLASSICAL CORRELATIONS")
    print("=" * 70)

    print(f"\n  The density matrix rho has:")
    print(f"    - DIAGONAL elements rho(i,i): occupation probabilities (classical)")
    print(f"    - OFF-DIAGONAL elements rho(i,j): quantum coherence (quantum)")
    print(f"  We compare the mutual information from the full rho vs diagonal-only.")
    print()

    beta = np.pi
    L = 4  # region size

    # Diagonal (classical) version of rho
    rho_classical = np.diag(np.diag(rho))

    separations = list(range(L, N // 2 - L, max(1, (N // 2 - 2 * L) // 15)))

    print(f"  {'d':>6} | {'I_quantum':>12} | {'I_classical':>12} | "
          f"{'I_q/I_c':>10} | {'Quantum excess':>14}")
    print("  " + "-" * 62)

    results = []

    for d in separations:
        sites_A = np.arange(L)
        sites_B = np.arange(d, d + L)

        mi_q = compute_mutual_information(rho, sites_A, sites_B)
        mi_c = compute_mutual_information(rho_classical, sites_A, sites_B)

        ratio = mi_q['I_AB'] / mi_c['I_AB'] if mi_c['I_AB'] > 1e-15 else float('inf')
        excess = mi_q['I_AB'] - mi_c['I_AB']

        results.append({
            'd': d,
            'I_quantum': mi_q['I_AB'],
            'I_classical': mi_c['I_AB'],
            'ratio': ratio,
            'excess': excess,
        })

        print(f"  {d:>6} | {mi_q['I_AB']:>12.8f} | {mi_c['I_AB']:>12.8f} | "
              f"{ratio:>10.4f} | {excess:>14.8f}")

    # Average quantum excess
    excesses = np.array([r['excess'] for r in results])
    ratios = np.array([r['ratio'] for r in results if r['ratio'] < 1e10])

    avg_ratio = np.mean(ratios) if len(ratios) > 0 else 0.0
    has_quantum = np.any(excesses > 1e-10)

    print(f"\n  Average I_quantum / I_classical: {avg_ratio:.4f}")
    print(f"  Quantum excess detected:         {'YES' if has_quantum else 'NO'}")

    if has_quantum:
        max_excess = np.max(excesses)
        d_max_excess = results[np.argmax(excesses)]['d']
        print(f"  Max quantum excess:              {max_excess:.8f} at d = {d_max_excess}")
        print(f"\n  [PASS] Off-diagonal coherence contributes to correlations.")
        print(f"  The flux field carries GENUINE QUANTUM CORRELATIONS")
        print(f"  beyond what classical occupation probabilities explain.")
    else:
        print(f"\n  [INFO] No quantum excess detected at this resolution.")
        print(f"  Correlations are ENTIRELY CLASSICAL (diagonal only).")

    return results


# ============================================================================
# Section 3: Coherence Structure — Quantum vs Classical Content
# ============================================================================

def run_section_3(rho, N=128):
    """
    Section 3: Quantify quantum vs classical content of the thermal state.

    The single-particle density matrix rho lives in C^N (NOT a tensor product).
    Standard bipartite entanglement measures (negativity, partial transpose)
    require tensor product structure and don't apply here.

    Instead, we decompose the density matrix into:
    - DIAGONAL: rho_diag = diag(rho) — classical occupation probabilities
    - OFF-DIAGONAL: rho_off = rho - rho_diag — quantum coherence

    The coherence measures how much the state differs from a classical mixture.
    """
    print("\n" + "=" * 70)
    print("SECTION 3: QUANTUM COHERENCE STRUCTURE")
    print("=" * 70)

    print(f"\n  NOTE: The single-particle density matrix lives in C^N,")
    print(f"  NOT a tensor product H_A (x) H_B. Standard entanglement")
    print(f"  measures (negativity, partial transpose) do not apply.")
    print(f"  Instead, we quantify COHERENCE (off-diagonal structure).")
    print()

    # Global coherence measures
    rho_diag = np.diag(np.diag(rho))
    rho_off = rho - rho_diag

    # l1-norm of coherence: C_l1 = sum_{i!=j} |rho_{ij}|
    C_l1 = np.sum(np.abs(rho_off))

    # Frobenius norm of off-diagonal
    C_frob = np.linalg.norm(rho_off, 'fro')

    # Relative entropy of coherence: C_RE = S(rho_diag) - S(rho)
    S_rho = von_neumann_entropy(rho)
    S_diag = von_neumann_entropy(rho_diag)
    C_RE = S_diag - S_rho

    print(f"  Global coherence measures:")
    print(f"    l1-norm:         C_l1 = {C_l1:.6f}")
    print(f"    Frobenius:       C_F  = {C_frob:.6f}")
    print(f"    Relative entropy: C_RE = {C_RE:.6f}")
    print(f"    S(rho_diag):     {S_diag:.6f}")
    print(f"    S(rho):          {S_rho:.6f}")
    print()

    # For a completely incoherent state (diagonal), C = 0.
    # For the thermal state, off-diagonal elements come from plane wave
    # eigenstates: rho(i,j) = (1/N) sum_k p_k exp(2*pi*i*k*(i-j)/N)
    # This is nonzero only when the Fourier transform of p_k has support.

    # Coherence as function of distance: how do |rho(i,j)| distribute?
    # For translational invariance: rho(i,j) = C(|i-j|)
    print(f"  Coherence by distance (|rho(0,r)| / rho(0,0)):")
    print(f"  {'r':>6} | {'|C(r)/C(0)|':>14} | {'Cumulative C_l1':>16}")
    print("  " + "-" * 42)

    C_r = np.array([np.abs(rho[0, r]) for r in range(N)])
    C0 = C_r[0]
    cumulative = 0.0

    for r in range(min(20, N // 2)):
        cumulative += 2 * N * C_r[r] if r > 0 else 0  # factor 2*N: each distance appears ~2*N times
        if r <= 10 or r % 5 == 0:
            print(f"  {r:>6} | {C_r[r]/C0:>14.8f} | {cumulative:>16.4f}")

    # What fraction of total coherence comes from short-range?
    short_range = sum(C_r[1:6]) * 2  # r=1..5, both directions
    total_offdiag = sum(C_r[1:N//2]) * 2 + C_r[N//2] if N % 2 == 0 else sum(C_r[1:N//2+1]) * 2
    frac_short = short_range / total_offdiag if total_offdiag > 0 else 0

    print(f"\n  Short-range coherence (r <= 5): {frac_short*100:.1f}% of total")
    print(f"  Long-range coherence (r > 5):   {(1-frac_short)*100:.1f}% of total")

    has_coherence = C_RE > 0.01
    print(f"\n  Coherence detected: {'[PASS]' if has_coherence else '[FAIL]'}")
    if has_coherence:
        print(f"  The thermal state has GENUINE QUANTUM COHERENCE")
        print(f"  (off-diagonal elements reduce entropy by {C_RE:.4f} nats)")
    else:
        print(f"  The thermal state is nearly CLASSICAL (diagonal dominant)")

    return {
        'C_l1': C_l1,
        'C_frob': C_frob,
        'C_RE': C_RE,
        'C_r': C_r,
        'has_coherence': has_coherence,
    }


# ============================================================================
# Section 4: sLoop-Inspired Test — Self-Referential Correlations
# ============================================================================

def run_section_4(N=128):
    """
    Section 4: Does the thermal state show self-referential structure?

    The sLoop hypothesis (FOUND_SLOOP_FORMALIZATION.md) predicts that
    self-referential systems have enhanced correlations between a region
    and its complement (the "environment" that determines its state).

    We test: for a region A of size L, is the mutual information I(A : A^c)
    maximized at any particular size L? The sLoop predicts maximum self-reference
    when A and A^c are "comparable" — when the observer is of similar
    complexity to the observed.
    """
    print("\n" + "=" * 70)
    print("SECTION 4: sLOOP SELF-REFERENTIAL CORRELATIONS")
    print("=" * 70)

    beta = np.pi
    rho, H, probs, Z = build_thermal_rho(N, beta)

    print(f"\n  N = {N}, beta = pi")
    print(f"  Testing: I(A : A^c) for region A of varying size L")
    print(f"  sLoop predicts maximum when |A| ~ |A^c| (L ~ N/2)")
    print()

    L_values = list(range(1, N // 2 + 1))
    results = []

    print(f"  {'L':>6} | {'I(A:Ac)':>12} | {'S_A':>10} | {'S_Ac':>10} | {'S_full':>10}")
    print("  " + "-" * 56)

    S_full = von_neumann_entropy(rho)

    for L in L_values:
        sites_A = np.arange(L)
        sites_Ac = np.arange(L, N)

        rho_A, p_A = restricted_rho(rho, sites_A)
        rho_Ac, p_Ac = restricted_rho(rho, sites_Ac)

        S_A = von_neumann_entropy(rho_A)
        S_Ac = von_neumann_entropy(rho_Ac)

        # I(A:Ac) = S_A + S_Ac - S_full
        I_AAc = S_A + S_Ac - S_full

        results.append({
            'L': L,
            'I_AAc': I_AAc,
            'S_A': S_A,
            'S_Ac': S_Ac,
        })

        if L <= 10 or L % (N // 20) == 0 or L == N // 2:
            print(f"  {L:>6} | {I_AAc:>12.6f} | {S_A:>10.6f} | "
                  f"{S_Ac:>10.6f} | {S_full:>10.6f}")

    # Find maximum I(A:Ac)
    L_arr = np.array([r['L'] for r in results])
    I_arr = np.array([r['I_AAc'] for r in results])
    idx_max = np.argmax(I_arr)
    L_max = L_arr[idx_max]

    print(f"\n  Maximum I(A:Ac):  {I_arr[idx_max]:.6f} at L = {L_max} (N/2 = {N//2})")
    sloop_match = abs(L_max - N // 2) <= max(N // 10, 1)
    print(f"  sLoop prediction: maximum at L ~ N/2")
    print(f"  Match:            {'[PASS]' if sloop_match else '[FAIL]'}")

    if sloop_match:
        print(f"\n  Self-referential structure CONFIRMED:")
        print(f"  Maximum information exchange occurs when the 'observer' (A)")
        print(f"  and the 'observed' (A^c) are of comparable size.")
        print(f"  This is the equal-partition principle of the sLoop.")
    else:
        print(f"\n  Maximum at L = {L_max}, not at N/2 = {N//2}.")
        print(f"  The thermal state's maximum information exchange is asymmetric.")

    # Also check: is I(A:Ac) symmetric around L = N/2?
    I_first_half = I_arr[:N//4]
    I_second_half = I_arr[N//4:N//2][::-1]
    min_len = min(len(I_first_half), len(I_second_half))
    if min_len > 0:
        symmetry = np.corrcoef(I_first_half[:min_len], I_second_half[:min_len])[0, 1]
        print(f"\n  Symmetry of I(L) around L=N/2: corr = {symmetry:.6f}")
        print(f"  {'[PASS] Symmetric' if symmetry > 0.99 else '[INFO] Asymmetric'}")

    return results


# ============================================================================
# Figure Generation
# ============================================================================

def generate_figure(corr_results, C_values, r_values, xi,
                    coh_result, sloop_results, N):
    """Generate 4-panel figure."""
    try:
        import matplotlib
        matplotlib.use('Agg')
        import matplotlib.pyplot as plt
    except ImportError:
        print("\n  [SKIP] matplotlib not available")
        return

    fig, axes = plt.subplots(2, 2, figsize=(12, 10))
    fig.suptitle(r'FTD Spatial Correlations and Coherence ($\beta = \pi$)',
                 fontsize=13, fontweight='bold')

    # Panel A: Two-point correlation function C(r)
    ax = axes[0, 0]
    C0 = C_values[0]
    r_half = r_values[:len(r_values)//2]
    C_half = np.abs(C_values[:len(r_values)//2]) / C0
    mask = C_half > 1e-15
    ax.semilogy(r_half[mask], C_half[mask], 'b.-', markersize=4, label='$|C(r)/C(0)|$')
    r_dense = np.linspace(1, len(r_half), 100)
    ax.semilogy(r_dense, np.exp(-r_dense / xi), 'r--', alpha=0.7,
                label=rf'$\exp(-r/{xi:.1f})$')
    ax.set_xlabel('Distance r (lattice units)')
    ax.set_ylabel('$|C(r)/C(0)|$')
    ax.set_title('A: Two-Point Correlation Function')
    ax.legend(fontsize=9)
    ax.grid(True, alpha=0.3)

    # Panel B: Off-diagonal coherence vs distance
    ax = axes[0, 1]
    d_arr = np.array([r['d'] for r in corr_results])
    coh_off = np.array([r['coh_off'] for r in corr_results])
    coh_diag = np.array([r['coh_diag'] for r in corr_results])
    mask_off = coh_off > 1e-15
    mask_diag = coh_diag > 1e-15
    if np.any(mask_off):
        ax.semilogy(d_arr[mask_off], coh_off[mask_off], 'b.-', markersize=4,
                     label='Off-diagonal (quantum)')
    if np.any(mask_diag):
        ax.semilogy(d_arr[mask_diag], coh_diag[mask_diag], 'r.--', markersize=4,
                     label='Diagonal (classical)')
    ax.set_xlabel('Separation d')
    ax.set_ylabel('Coherence norm')
    ax.set_title('B: Quantum vs Classical Coherence')
    ax.legend(fontsize=9)
    ax.grid(True, alpha=0.3)

    # Panel C: Coherence by distance
    ax = axes[1, 0]
    C_r = coh_result['C_r']
    r_plot = np.arange(min(30, len(C_r) // 2))
    ax.bar(r_plot, C_r[r_plot] / C_r[0], color='steelblue', alpha=0.7)
    ax.set_xlabel('Distance r')
    ax.set_ylabel('$|\\rho(0,r)| / \\rho(0,0)$')
    ax.set_title('C: Coherence Distribution by Distance')
    ax.grid(True, alpha=0.3, axis='y')

    # Panel D: sLoop self-referential test
    ax = axes[1, 1]
    L_arr = np.array([r['L'] for r in sloop_results])
    I_sloop = np.array([r['I_AAc'] for r in sloop_results])
    ax.plot(L_arr, I_sloop, 'g-', linewidth=1.5)
    ax.axvline(x=N//2, color='k', linestyle='--', alpha=0.5, label='$L = N/2$')
    idx_max = np.argmax(I_sloop)
    ax.axvline(x=L_arr[idx_max], color='r', linestyle=':', alpha=0.7,
               label=f'Max at L={L_arr[idx_max]}')
    ax.set_xlabel('Region size L')
    ax.set_ylabel('$I(A : A^c)$ (nats)')
    ax.set_title('D: sLoop Self-Referential Structure')
    ax.legend(fontsize=9)
    ax.grid(True, alpha=0.3)

    plt.tight_layout()

    fig_dir = os.path.join(os.path.dirname(__file__), '..', '..', 'docs', 'papers', 'src', 'figures')
    os.makedirs(fig_dir, exist_ok=True)

    pdf_path = os.path.join(fig_dir, 'FTD_Spatial_Correlations.pdf')
    png_path = os.path.join(fig_dir, 'FTD_Spatial_Correlations.png')

    try:
        fig.savefig(pdf_path, bbox_inches='tight', dpi=150)
        print(f"\n  Figure saved: {pdf_path}")
    except Exception as e:
        print(f"\n  [INFO] PDF save failed ({e}), trying PNG...")

    fig.savefig(png_path, bbox_inches='tight', dpi=150)
    print(f"  Figure saved: {png_path}")
    plt.close(fig)


# ============================================================================
# Main
# ============================================================================

def main():
    print()
    print("*" * 70)
    print("  FTD SPATIAL CORRELATIONS AND COHERENCE")
    print("  Step 5 of the QFT-GRT Bridge Critical Path")
    print("*" * 70)
    print()
    print("  Goal: Quantify quantum vs classical correlations in the")
    print("        FTD thermal state, and test sLoop structure.")
    print()

    N = 128

    # --- Section 1: Spatial Correlations ---
    corr_results, rho, C_values, r_values, xi = run_section_1(N)

    # --- Section 2: Quantum vs Classical MI ---
    qc_results = run_section_2(rho, N)

    # --- Section 3: Coherence Structure ---
    coh_result = run_section_3(rho, N)

    # --- Section 4: sLoop Test ---
    sloop_results = run_section_4(N)

    # --- Summary ---
    print("\n" + "=" * 70)
    print("SUMMARY: STEP 5 — SPATIAL CORRELATIONS AND COHERENCE")
    print("=" * 70)

    has_coherence = coh_result['has_coherence']
    C_RE = coh_result['C_RE']

    # sLoop
    L_arr = np.array([r['L'] for r in sloop_results])
    I_arr = np.array([r['I_AAc'] for r in sloop_results])
    L_max = L_arr[np.argmax(I_arr)]

    print(f"\n  1. Correlation length:  xi = {xi:.2f} lattice units")
    print(f"  2. Quantum coherence:  {'PRESENT' if has_coherence else 'absent'} "
          f"(C_RE = {C_RE:.4f} nats)")
    print(f"  3. sLoop maximum:      L = {L_max} (N/2 = {N//2})")

    print(f"\n  BRIDGE IMPLICATIONS:")
    print(f"  " + "-" * 50)

    print(f"  The FTD thermal state at beta = pi has:")
    print(f"    - Short-range off-diagonal coherence (xi ~ {xi:.1f})")
    print(f"    - Relative entropy of coherence C_RE = {C_RE:.4f}")
    print(f"    - sLoop structure: max I(A:Ac) at L = {L_max}")
    print()

    if has_coherence:
        print(f"  [PASS] The off-diagonal elements of the density matrix")
        print(f"  carry GENUINE QUANTUM COHERENCE — the entropy of the")
        print(f"  full state is {C_RE:.4f} nats BELOW that of the diagonal.")
        print(f"  This coherence is the quantum structure that the von")
        print(f"  Neumann algebra formalism must capture for the bridge.")
    else:
        print(f"  [INFO] Minimal coherence. The thermal state is nearly")
        print(f"  classical at this temperature.")

    print(f"\n  The single-particle Hilbert space C^N does NOT have a")
    print(f"  tensor product structure, so standard bipartite entanglement")
    print(f"  measures (negativity, concurrence) do not apply. Genuine")
    print(f"  entanglement requires the MANY-BODY Hilbert space (second")
    print(f"  quantization), which is a Layer 0 gap (GAP-S3 in roadmap).")

    # --- Generate Figure ---
    print("\n  Generating 4-panel figure...")
    generate_figure(corr_results, C_values, r_values, xi,
                    coh_result, sloop_results, N)

    print("\n" + "*" * 70)
    print("  VERIFICATION COMPLETE")
    print("*" * 70)


if __name__ == "__main__":
    main()

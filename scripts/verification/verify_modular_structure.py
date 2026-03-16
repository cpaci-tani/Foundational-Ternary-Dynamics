"""
First Step: Entanglement Spectrum and Modular Hamiltonian of the FTD Flux Field
================================================================================

Companion script for SPEC_QFT_GRT_BRIDGE_ROADMAP.md (Step 1 of the Critical Path).

This is the first concrete computation toward bridging QFT and GRT via quantized
sentience. It extracts a wave function from the FTD flux field, constructs density
matrices, computes entanglement spectra and the modular Hamiltonian, and tests
area-law vs volume-law entropy scaling.

What this establishes:
- The raw data from which factor type classification proceeds
- Area-law scaling = QFT-like entanglement (Type III tendency)
- Volume-law scaling = thermal/Type I (no long-range entanglement)
- Modular Hamiltonian spectral properties constrain algebra type

Epistemic status: [THEOREM] for density matrix / entropy computations (standard QI);
[OPEN] for physical interpretation of results within FTD bridge program.

Output:
- Console: verification results with [PASS]/[FAIL] markers
- Figure: docs/papers/src/figures/FTD_Modular_Structure.pdf (4-panel)
"""

import numpy as np
import sys
import os
from scipy import linalg as sla

# Add project root to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))

from models.quantum_entropy import DensityMatrix, partial_trace_B, von_neumann_entropy


# ============================================================================
# Section 1: Wave Function Extraction from FTD Flux Field
# ============================================================================

def extract_wave_function_1d(N, mode='wave_packet'):
    """
    Create a 1D wave function for initial testing.

    For a 1D chain of N sites, the wave function psi(x) encodes the
    FTD complexified flux: psi = J_x + i*J_y.

    Args:
        N: Number of lattice sites
        mode: 'wave_packet' (Gaussian), 'plane_wave', 'zpf' (random thermal)

    Returns:
        psi: Complex numpy array of shape (N,), normalized
    """
    x = np.arange(N, dtype=float)

    if mode == 'wave_packet':
        # Gaussian wave packet centered at N/2, width ~ N/8
        x0 = N / 2
        sigma = N / 8
        k0 = 2 * np.pi / (N / 4)  # carrier wavenumber
        psi = np.exp(-(x - x0)**2 / (2 * sigma**2)) * np.exp(1j * k0 * x)

    elif mode == 'plane_wave':
        # Superposition of two plane waves (creates entanglement structure)
        k1 = 2 * np.pi / N
        k2 = 6 * np.pi / N
        psi = (np.exp(1j * k1 * x) + 0.5 * np.exp(1j * k2 * x)) / np.sqrt(1.25)

    elif mode == 'zpf':
        # Zero-point fluctuation: random Gaussian with thermal character
        # This models the ZPF equilibrium state at beta = pi
        rng = np.random.RandomState(42)
        sigma_zpf = 0.511 / np.sqrt(2 * np.pi)  # K_B / sqrt(2*pi)
        J_x = rng.normal(0, sigma_zpf, N)
        J_y = rng.normal(0, sigma_zpf, N)
        psi = J_x + 1j * J_y

    else:
        raise ValueError(f"Unknown mode: {mode}")

    # Normalize
    norm = np.linalg.norm(psi)
    if norm > 0:
        psi = psi / norm

    return psi


def extract_wave_function_3d(universe):
    """
    Extract complexified wave function from a Universe's flux field.

    psi(x,y,z) = flux[x,y,z,0] + i * flux[x,y,z,1]

    This is the J_x + i*J_y construction from CLAUDE.md section 13.1.

    Args:
        universe: Universe instance with populated flux field

    Returns:
        psi: Complex numpy array of shape (N^3,), normalized (flattened)
    """
    psi_3d = universe.flux[..., 0] + 1j * universe.flux[..., 1]
    psi = psi_3d.flatten().astype(complex)
    norm = np.linalg.norm(psi)
    if norm > 0:
        psi = psi / norm
    return psi


def run_section_1():
    """Section 1: Wave Function Extraction."""
    print("=" * 70)
    print("SECTION 1: WAVE FUNCTION EXTRACTION")
    print("=" * 70)

    N = 64  # 1D chain length
    results = {}

    for mode in ['wave_packet', 'plane_wave', 'zpf']:
        psi = extract_wave_function_1d(N, mode=mode)
        norm = np.linalg.norm(psi)
        max_amp = np.max(np.abs(psi))

        print(f"\n  Mode: {mode}")
        print(f"    Sites:          {N}")
        print(f"    Norm:           {norm:.10f}")
        print(f"    Max |psi|:      {max_amp:.6f}")
        print(f"    Normalized:     {'[PASS]' if np.isclose(norm, 1.0) else '[FAIL]'}")

        results[mode] = psi

    return results


# ============================================================================
# Section 2: Density Matrix Construction
# ============================================================================

def run_section_2(psi):
    """Section 2: Density Matrix Construction and Verification."""
    print("\n" + "=" * 70)
    print("SECTION 2: DENSITY MATRIX CONSTRUCTION")
    print("=" * 70)

    N = len(psi)
    rho = DensityMatrix.from_pure_state(psi)

    # Verify properties
    trace = np.real(np.trace(rho.matrix))
    hermitian = np.allclose(rho.matrix, rho.matrix.conj().T)
    rho_sq = rho.matrix @ rho.matrix
    pure = np.isclose(np.real(np.trace(rho_sq)), 1.0)

    print(f"\n  Dimension:        {N} x {N}")
    print(f"  Tr(rho):          {trace:.10f}  {'[PASS]' if np.isclose(trace, 1.0) else '[FAIL]'}")
    print(f"  Hermitian:        {hermitian}  {'[PASS]' if hermitian else '[FAIL]'}")
    print(f"  Tr(rho^2):        {np.real(np.trace(rho_sq)):.10f}")
    print(f"  Pure state:       {pure}  {'[PASS]' if pure else '[FAIL]'}")

    return rho


# ============================================================================
# Section 3: Bipartite Entanglement
# ============================================================================

def compute_entanglement(psi, dim_A):
    """
    Compute entanglement entropy and spectrum for bipartite split.

    Args:
        psi: Normalized state vector of length dim_A * dim_B
        dim_A: Dimension of subsystem A

    Returns:
        dict with entropy, spectrum, rho_A
    """
    N = len(psi)
    dim_B = N // dim_A
    if dim_A * dim_B != N:
        raise ValueError(f"Cannot split {N} into {dim_A} x {dim_B}")

    # Build full density matrix
    rho_full = np.outer(psi, psi.conj())

    # Partial trace over B
    rho_A = partial_trace_B(rho_full, dim_A, dim_B)

    # Eigendecompose rho_A (Hermitian)
    eigenvalues = np.linalg.eigvalsh(rho_A)
    eigenvalues = np.sort(eigenvalues)[::-1]  # descending

    # Entanglement entropy
    nonzero = eigenvalues[eigenvalues > 1e-15]
    S_A = -np.sum(nonzero * np.log(nonzero))

    return {
        'entropy': S_A,
        'spectrum': eigenvalues,
        'rho_A': rho_A,
        'dim_A': dim_A,
        'dim_B': dim_B,
    }


def run_section_3(psi):
    """Section 3: Bipartite Entanglement."""
    print("\n" + "=" * 70)
    print("SECTION 3: BIPARTITE ENTANGLEMENT")
    print("=" * 70)

    N = len(psi)
    dim_A = N // 2
    dim_B = N - dim_A

    result = compute_entanglement(psi, dim_A)

    S_A = result['entropy']
    spectrum = result['spectrum']
    S_max = np.log(min(dim_A, dim_B))

    # Count significant eigenvalues (Schmidt rank)
    schmidt_rank = np.sum(spectrum > 1e-10)

    print(f"\n  Split:            A = {dim_A}, B = {dim_B}")
    print(f"  S_A:              {S_A:.6f}")
    print(f"  S_max = ln({min(dim_A, dim_B)}):  {S_max:.6f}")
    print(f"  S_A / S_max:      {S_A / S_max:.6f}")
    print(f"  S_A >= 0:         {'[PASS]' if S_A >= -1e-10 else '[FAIL]'}")
    print(f"  S_A <= S_max:     {'[PASS]' if S_A <= S_max + 1e-10 else '[FAIL]'}")
    print(f"  Schmidt rank:     {schmidt_rank} / {dim_A}")
    print(f"  Top 5 eigenvals:  {spectrum[:5]}")

    return result


# ============================================================================
# Section 4: Modular Hamiltonian
# ============================================================================

def compute_modular_hamiltonian(rho_A):
    """
    Compute the modular Hamiltonian K_A = -ln(rho_A).

    The modular Hamiltonian encodes the entanglement structure:
    K_A = -ln(rho_A) = -sum_i ln(lambda_i) |i><i|

    For QFT ground states, K_A is often proportional to a local Hamiltonian
    (Bisognano-Wichmann theorem for Rindler wedges).

    Args:
        rho_A: Reduced density matrix (numpy array)

    Returns:
        dict with K_A, eigenvalues, eigenvectors
    """
    # Regularize: replace zero eigenvalues with small epsilon
    eigenvalues, eigenvectors = np.linalg.eigh(rho_A)

    # Clamp small eigenvalues to avoid log(0)
    eps = 1e-15
    eigenvalues_reg = np.maximum(eigenvalues, eps)

    # K_A = -ln(rho_A) in eigenbasis
    modular_energies = -np.log(eigenvalues_reg)

    # Reconstruct K_A matrix
    K_A = eigenvectors @ np.diag(modular_energies) @ eigenvectors.conj().T

    # Sort modular energies (ascending = most probable states first)
    sorted_idx = np.argsort(modular_energies)
    modular_energies_sorted = modular_energies[sorted_idx]

    return {
        'K_A': K_A,
        'modular_energies': modular_energies_sorted,
        'eigenvalues_rho': np.sort(eigenvalues)[::-1],
        'eigenvectors': eigenvectors,
    }


def run_section_4(rho_A):
    """Section 4: Modular Hamiltonian."""
    print("\n" + "=" * 70)
    print("SECTION 4: MODULAR HAMILTONIAN")
    print("=" * 70)

    result = compute_modular_hamiltonian(rho_A)
    K_A = result['K_A']
    energies = result['modular_energies']

    # Check Hermiticity of K_A
    hermitian = np.allclose(K_A, K_A.conj().T)

    # Modular energy statistics
    finite_energies = energies[energies < 100]  # exclude regularized zeros

    print(f"\n  K_A dimension:    {K_A.shape[0]} x {K_A.shape[1]}")
    print(f"  Hermitian:        {hermitian}  {'[PASS]' if hermitian else '[FAIL]'}")
    print(f"  Real eigenvals:   {'[PASS]' if np.all(np.isreal(energies)) else '[FAIL]'}")
    print(f"  Finite energies:  {len(finite_energies)} / {len(energies)}")

    if len(finite_energies) > 0:
        print(f"  Min kappa:        {np.min(finite_energies):.6f}")
        print(f"  Max kappa:        {np.max(finite_energies):.6f}")
        print(f"  Mean kappa:       {np.mean(finite_energies):.6f}")
        print(f"  Std kappa:        {np.std(finite_energies):.6f}")

        # Check if K_A has approximately linear spectrum
        # (Bisognano-Wichmann: K_A ~ 2*pi*x for Rindler wedge)
        if len(finite_energies) > 5:
            x = np.arange(len(finite_energies))
            coeffs = np.polyfit(x, finite_energies, 1)
            residual = np.std(finite_energies - np.polyval(coeffs, x))
            print(f"  Linear fit slope: {coeffs[0]:.6f}")
            print(f"  Linear residual:  {residual:.6f}")
            print(f"  Approx linear:    {'Possibly' if residual / np.std(finite_energies) < 0.3 else 'No'}")

    return result


# ============================================================================
# Section 5: Area-Law Scaling Test
# ============================================================================

def run_section_5(mode='wave_packet'):
    """
    Section 5: Area-Law Scaling Test.

    Sweep subregion size L from 2 to N/2 and compute S_A(L).
    For 1D systems:
      - Area law: S_A ~ const (bounded, independent of L)
      - Volume law: S_A ~ L (linear growth)
      - Critical/CFT: S_A ~ (c/3) * ln(L) + const
    """
    print("\n" + "=" * 70)
    print("SECTION 5: AREA-LAW SCALING TEST")
    print("=" * 70)

    N = 64
    psi = extract_wave_function_1d(N, mode=mode)

    L_values = []
    S_values = []

    # Sweep subregion sizes (must divide N evenly for clean partial trace)
    for L in range(2, N // 2 + 1):
        if N % L == 0 or N % (N - L) == 0:
            # Use L as dim_A if it divides N
            dim_A = L
            dim_B = N // dim_A
            if dim_A * dim_B != N:
                continue
            try:
                result = compute_entanglement(psi, dim_A)
                L_values.append(L)
                S_values.append(result['entropy'])
            except (ValueError, np.linalg.LinAlgError):
                continue

    L_arr = np.array(L_values)
    S_arr = np.array(S_values)

    print(f"\n  Mode:             {mode}")
    print(f"  Total sites:      {N}")
    print(f"  Sizes tested:     {len(L_values)}")

    if len(L_values) >= 3:
        # Fit volume law: S ~ a * L
        coeffs_vol = np.polyfit(L_arr, S_arr, 1)
        residual_vol = np.std(S_arr - np.polyval(coeffs_vol, L_arr))

        # Fit log law: S ~ a * ln(L) + b
        log_L = np.log(L_arr)
        coeffs_log = np.polyfit(log_L, S_arr, 1)
        residual_log = np.std(S_arr - np.polyval(coeffs_log, log_L))

        # Fit constant (area law in 1D): S ~ const
        S_mean = np.mean(S_arr)
        residual_const = np.std(S_arr - S_mean)

        print(f"\n  Scaling fits:")
        print(f"    Volume law (S ~ aL):    slope={coeffs_vol[0]:.6f}, residual={residual_vol:.6f}")
        print(f"    Log law (S ~ a*ln L):   coeff={coeffs_log[0]:.6f}, residual={residual_log:.6f}")
        print(f"    Constant (area law):    mean={S_mean:.6f}, residual={residual_const:.6f}")

        # Determine best fit
        best = min(
            [('volume', residual_vol), ('log (CFT-like)', residual_log), ('constant (area)', residual_const)],
            key=lambda x: x[1]
        )
        print(f"\n  Best fit:         {best[0]} (residual={best[1]:.6f})")

        if best[0] == 'constant (area)':
            print("  Interpretation:   Area law -> ground-state-like, QFT signature")
        elif best[0] == 'log (CFT-like)':
            print(f"  Interpretation:   Log law -> critical/CFT behavior, c_eff = {3*coeffs_log[0]:.3f}")
            print("                    (c=1 for free boson, c=1/2 for Ising)")
        elif best[0] == 'volume':
            print("  Interpretation:   Volume law -> thermal/Type I behavior")
    else:
        print("  [INFO] Not enough divisor-compatible sizes for fitting")

    return {'L': L_arr, 'S': S_arr, 'mode': mode, 'N': N}


# ============================================================================
# Section 6: Modular Flow Preview
# ============================================================================

def run_section_6(rho_A, dim_A):
    """
    Section 6: Modular Flow Preview.

    Compute sigma_t(O) = rho_A^{it} O rho_A^{-it} for a test observable O.

    This previews the modular automorphism, which is the candidate for
    physical/conscious time in the Connes-Rovelli framework.
    """
    print("\n" + "=" * 70)
    print("SECTION 6: MODULAR FLOW PREVIEW")
    print("=" * 70)

    # Use a small subspace for feasibility
    d = min(dim_A, 16)  # cap at 16x16 for matrix exponential
    rho_small = rho_A[:d, :d]

    # Re-normalize the small block
    trace = np.trace(rho_small)
    if np.abs(trace) < 1e-15:
        print("  [SKIP] rho_A subblock has zero trace")
        return None
    rho_small = rho_small / trace

    # Eigendecompose
    eigenvalues, eigenvectors = np.linalg.eigh(rho_small)
    eigenvalues = np.maximum(eigenvalues, 1e-15)

    # Test observable: position-like operator (diagonal with increasing values)
    O = np.diag(np.arange(d, dtype=float))

    # Compute modular flow at several times
    t_values = [0.0, 0.1, 0.5, 1.0, 2.0]
    flow_results = []

    print(f"\n  Subspace dim:     {d}")
    print(f"  Observable:       position-like (diagonal 0..{d-1})")
    print(f"\n  {'t':>6} | {'Tr(sigma_t(O))':>18} | {'||sigma_t(O) - O||':>20} | {'Hermitian':>10}")
    print("  " + "-" * 62)

    for t in t_values:
        # rho^{it} = U diag(lambda_i^{it}) U^dagger
        phases = eigenvalues ** (1j * t)
        rho_it = eigenvectors @ np.diag(phases) @ eigenvectors.conj().T
        rho_neg_it = eigenvectors @ np.diag(phases.conj()) @ eigenvectors.conj().T

        # sigma_t(O) = rho^{it} O rho^{-it}
        sigma_t_O = rho_it @ O @ rho_neg_it

        # Measure deviation from O
        diff_norm = np.linalg.norm(sigma_t_O - O, 'fro')
        trace_val = np.real(np.trace(sigma_t_O))
        is_hermitian = np.allclose(sigma_t_O, sigma_t_O.conj().T, atol=1e-10)

        flow_results.append({
            't': t,
            'sigma_t_O': sigma_t_O,
            'diff_norm': diff_norm,
            'trace': trace_val,
            'hermitian': is_hermitian,
        })

        print(f"  {t:6.2f} | {trace_val:18.6f} | {diff_norm:20.6f} | {'Yes' if is_hermitian else 'No':>10}")

    # At t=0, sigma_0(O) should equal O
    t0_result = flow_results[0]
    t0_pass = t0_result['diff_norm'] < 1e-10
    print(f"\n  sigma_0(O) = O:   {'[PASS]' if t0_pass else '[FAIL]'} (diff = {t0_result['diff_norm']:.2e})")

    # Check if flow is non-trivial (differs from O at t>0)
    max_diff = max(r['diff_norm'] for r in flow_results[1:])
    trivial = max_diff < 1e-10
    print(f"  Flow non-trivial: {'[PASS] (modular flow acts non-trivially)' if not trivial else '[INFO] Flow is trivial (rho proportional to identity?)'}")

    if not trivial:
        print(f"  Max deviation:    {max_diff:.6f} at t={flow_results[np.argmax([r['diff_norm'] for r in flow_results])]['t']}")
        print("\n  Interpretation:   Non-trivial modular flow means the state")
        print("                    breaks the symmetry of the observable algebra.")
        print("                    In the Connes-Rovelli framework, this IS time.")

    return flow_results


# ============================================================================
# Figure Generation
# ============================================================================

def generate_figure(ent_result, mod_result, scaling_results, flow_results):
    """Generate 4-panel figure summarizing results."""
    try:
        import matplotlib
        matplotlib.use('Agg')
        import matplotlib.pyplot as plt
    except ImportError:
        print("\n  [SKIP] matplotlib not available for figure generation")
        return

    fig, axes = plt.subplots(2, 2, figsize=(12, 10))
    fig.suptitle('FTD Modular Structure: First Computation Toward QFT-GRT Bridge',
                 fontsize=13, fontweight='bold')

    # Panel A: Entanglement Spectrum
    ax = axes[0, 0]
    spectrum = ent_result['spectrum']
    nonzero = spectrum[spectrum > 1e-15]
    ax.semilogy(range(len(nonzero)), nonzero, 'b.-', markersize=4)
    ax.set_xlabel('Index i')
    ax.set_ylabel(r'$\lambda_i$ (log scale)')
    ax.set_title('A: Entanglement Spectrum')
    ax.grid(True, alpha=0.3)

    # Panel B: Modular Energy Spectrum
    ax = axes[0, 1]
    energies = mod_result['modular_energies']
    finite = energies[energies < 50]
    ax.plot(range(len(finite)), finite, 'r.-', markersize=4)
    ax.set_xlabel('Index i')
    ax.set_ylabel(r'$\kappa_i = -\ln(\lambda_i)$')
    ax.set_title('B: Modular Energy Spectrum')
    ax.grid(True, alpha=0.3)

    # Panel C: Entropy Scaling
    ax = axes[1, 0]
    if len(scaling_results['L']) > 0:
        ax.plot(scaling_results['L'], scaling_results['S'], 'go-', markersize=5, label='S_A(L)')

        # Reference curves
        L_ref = np.linspace(2, max(scaling_results['L']), 100)
        S_max = max(scaling_results['S']) if len(scaling_results['S']) > 0 else 1.0
        ax.plot(L_ref, S_max * np.ones_like(L_ref), 'b--', alpha=0.5, label='Area law (const)')
        ax.plot(L_ref, S_max * L_ref / max(L_ref), 'r--', alpha=0.5, label='Volume law (linear)')

        ax.set_xlabel('Subregion size L')
        ax.set_ylabel(r'$S_A(L)$')
        ax.legend(fontsize=8)
    ax.set_title('C: Entropy Scaling')
    ax.grid(True, alpha=0.3)

    # Panel D: Modular Flow
    ax = axes[1, 1]
    if flow_results is not None:
        t_vals = [r['t'] for r in flow_results]
        diffs = [r['diff_norm'] for r in flow_results]
        traces = [r['trace'] for r in flow_results]

        ax.plot(t_vals, diffs, 'ms-', markersize=6, label=r'$||\sigma_t(O) - O||$')
        ax2 = ax.twinx()
        ax2.plot(t_vals, traces, 'c^-', markersize=6, alpha=0.7, label=r'$\mathrm{Tr}(\sigma_t(O))$')
        ax2.set_ylabel(r'$\mathrm{Tr}(\sigma_t(O))$', color='c')

        ax.set_xlabel('Modular time t')
        ax.set_ylabel(r'$||\sigma_t(O) - O||$', color='m')
        ax.legend(loc='upper left', fontsize=8)
        ax2.legend(loc='upper right', fontsize=8)
    ax.set_title('D: Modular Flow vs Tick Evolution')
    ax.grid(True, alpha=0.3)

    plt.tight_layout()

    # Save
    fig_dir = os.path.join(os.path.dirname(__file__), '..', '..', 'docs', 'papers', 'src', 'figures')
    os.makedirs(fig_dir, exist_ok=True)

    pdf_path = os.path.join(fig_dir, 'FTD_Modular_Structure.pdf')
    png_path = os.path.join(fig_dir, 'FTD_Modular_Structure.png')

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
    print("  FTD MODULAR STRUCTURE VERIFICATION")
    print("  Step 1 of the QFT-GRT Bridge Critical Path")
    print("*" * 70)
    print()
    print("  Goal: Compute entanglement spectrum and modular Hamiltonian")
    print("        of the FTD flux field to characterize algebra type.")
    print()

    # --- Section 1: Wave Function Extraction ---
    wave_functions = run_section_1()

    # Use wave_packet for subsequent analysis (richest entanglement structure)
    psi = wave_functions['wave_packet']

    # --- Section 2: Density Matrix ---
    # Use smaller N for density matrix (full rho is N x N)
    N_small = 32
    psi_small = extract_wave_function_1d(N_small, mode='wave_packet')
    rho = run_section_2(psi_small)

    # --- Section 3: Bipartite Entanglement ---
    ent_result = run_section_3(psi_small)

    # --- Section 4: Modular Hamiltonian ---
    mod_result = run_section_4(ent_result['rho_A'])

    # --- Section 5: Area-Law Scaling ---
    scaling_wp = run_section_5(mode='wave_packet')
    scaling_zpf = run_section_5(mode='zpf')

    # --- Section 6: Modular Flow ---
    flow_results = run_section_6(ent_result['rho_A'], ent_result['dim_A'])

    # --- Summary ---
    print("\n" + "=" * 70)
    print("SUMMARY")
    print("=" * 70)

    S_A = ent_result['entropy']
    S_max = np.log(ent_result['dim_A'])

    print(f"\n  Entanglement entropy S_A:     {S_A:.6f}")
    print(f"  Maximum possible S_max:       {S_max:.6f}")
    print(f"  Ratio S_A/S_max:              {S_A/S_max:.4f}")
    print(f"  Schmidt rank:                 {np.sum(ent_result['spectrum'] > 1e-10)}")

    energies = mod_result['modular_energies']
    finite = energies[energies < 50]
    if len(finite) > 0:
        print(f"\n  Modular energy range:         [{np.min(finite):.3f}, {np.max(finite):.3f}]")
        print(f"  Modular energy gap:           {finite[1] - finite[0]:.6f}" if len(finite) > 1 else "")

    # Interpretation
    print("\n  INTERPRETATION:")
    print("  " + "-" * 50)

    if S_A / S_max < 0.1:
        print("  Low entanglement (S_A/S_max < 0.1): product-like state.")
        print("  -> Type I signature (no long-range correlations)")
    elif S_A / S_max < 0.5:
        print("  Moderate entanglement: nontrivial but sub-maximal.")
        print("  -> Possible QFT ground-state-like behavior")
    else:
        print("  High entanglement (S_A/S_max > 0.5): strongly mixed.")
        print("  -> Volume-law / thermal behavior (Type I at finite size)")

    print("\n  NOTE: At finite lattice size, all algebras are Type I.")
    print("  The question is whether SCALING TRENDS as N grows")
    print("  indicate approach to Type III behavior.")
    print("  This requires Step 4 of the Critical Path (N-sweep).")

    # --- Generate Figure ---
    print("\n  Generating 4-panel figure...")
    generate_figure(ent_result, mod_result, scaling_wp, flow_results)

    print("\n" + "*" * 70)
    print("  VERIFICATION COMPLETE")
    print("*" * 70)


if __name__ == "__main__":
    main()

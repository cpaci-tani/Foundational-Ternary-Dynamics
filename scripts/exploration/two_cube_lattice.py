#!/usr/bin/env python3
"""
Two-Cube Lattice: How do adjacent voxels interact?
====================================================

729-state system (27 x 27 joint Hilbert space).
Three coupling models compared head-to-head:
  Model 1: BCC Contact (strong force glue)
  Model 2: Center Tunnel (gravitational coupling)
  Model 3: CFL Hopping (light-speed propagation)

Seven analyses per model: spectrum, binding, vacuum correlation,
entanglement, excitation propagation, channel selectivity, dispersion.
"""

import numpy as np
import sys, os, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from constants import (G_STAR, VARPI_CLASSICAL, X_PLUS, X_MINUS,
                       N_c, N_base, b_3, N_eff)

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
LB = ['+', '0', '-']

def sl(n):
    a, b, c = STATES[n]
    return '|%s%s%s>' % (LB[a], LB[b], LB[c])

def state_offset(s):
    return tuple(1 if x == 0 else (0 if x == 1 else -1) for x in s)

def moore_shell(s):
    return sum(1 for x in state_offset(s) if x != 0)

SHELL_INDICES = {sh: [i for i in range(27) if moore_shell(STATES[i]) == sh] for sh in range(4)}
SHELL_NAMES = ['Center', 'SC/U(1)', 'FCC/SU(2)', 'BCC/SU(3)']

def build_h1(d, b, g_):
    return np.array([[d, g_, 0], [g_, b, g_], [0, g_, -d]])

def build_h27(d, b, g_, J_):
    H1 = build_h1(d, b, g_)
    H = (np.kron(np.kron(H1, I3), I3) +
         np.kron(np.kron(I3, H1), I3) +
         np.kron(np.kron(I3, I3), H1))
    if abs(J_) > 1e-15:
        H += J_ * (np.kron(np.kron(Z3, Z3), I3) +
                    np.kron(np.kron(Z3, I3), Z3) +
                    np.kron(np.kron(I3, Z3), Z3))
    return H


# =============================================================================
# TWO-CUBE HAMILTONIAN
# =============================================================================

def facing_bcc_pairs():
    """BCC states of cube A facing cube B along axis 0.
    A's face: axis0 offset = +1 (index 0 = '+')
    B's face: axis0 offset = -1 (index 2 = '-')
    Match by axes 1,2."""
    pairs = []
    for i in range(27):
        if STATES[i][0] != 0:  # axis0 must be '+' for cube A face
            continue
        if moore_shell(STATES[i]) != 3:  # must be BCC
            continue
        # Find matching state in B: flip axis0 from + to -
        j_state = (2, STATES[i][1], STATES[i][2])
        j = STATES.index(j_state)
        if moore_shell(STATES[j]) == 3:
            pairs.append((i, j))
    return pairs


def build_v_inter(model, t_inter):
    """Build the 729x729 inter-cube coupling matrix."""
    V = np.zeros((729, 729))

    if model == 'bcc_contact':
        pairs = facing_bcc_pairs()
        for i_A, j_B in pairs:
            # |i_A, gs_B> <-> |gs_A, j_B>... no.
            # Actually: state swap. Cube A state i couples to Cube B state j.
            # In the joint basis |a, b> = index a*27 + b:
            # Coupling: |i_A, any_B> <-> |any_A, j_B> is too general.
            # The physical coupling: excitation hops from cube A site i to cube B site j.
            # This is: for all b, |i, b> <-> |b_modified, j>... too complex.
            #
            # Simplest physically motivated: single-particle hopping.
            # The BCC state of cube A can jump to the matching BCC state of cube B.
            # |i_A, b_B> -> |(i->center)_A, (center->j)_B> is the process,
            # but for simplicity, use direct swap: |i, b> <-> |replaced_i_with_center, b_with_j>
            #
            # CLEANEST: Just couple the single-particle states directly.
            # V |a, b> += t * delta(a, i_A) * |center_A, b> ... no.
            #
            # Actually the simplest correct coupling:
            # Site i of cube A talks to site j of cube B.
            # V = t * |i><j|_A acting on cube A tensor identity on B... no.
            #
            # The standard many-body hopping:
            # c_i_A^dag c_j_B + h.c. in second quantization.
            # In first quantization with the FULL cube states:
            # We need a projector. But our "particles" ARE the cube states.
            #
            # SIMPLEST MODEL: inter-cube coupling is a matrix element
            # V_{(a,b),(a',b')} = t * sum_{facing pairs (i,j)} [
            #   delta(a,i)*delta(a',center)*delta(b,center)*delta(b',j)
            #   + h.c. ]
            # Meaning: excitation jumps from site i of A to site j of B,
            # leaving A in center and B acquiring j.
            #
            # Even simpler: just let the corresponding SITES of the two cubes talk.
            # V_{(a,b),(a',b')} = t * delta(a,i)*delta(a',j)*delta(b,b') ... nope wrong.
            #
            # OK. Most physically transparent:
            # The two cubes are adjacent. The boundary between them is the BCC face.
            # A BCC excitation on cube A can hop to cube B.
            # This means: |i_A, gs_B> <-> |gs_A, j_B> with amplitude t.
            pass

        # Direct implementation: for each facing pair (i,j),
        # the state where cube A is in state i and cube B is in ANY state b
        # couples to the state where cube A has lost the excitation (center)
        # and cube B has gained it.
        # But this is complicated because the cube is a MULTI-PARTICLE system internally.
        #
        # THE RIGHT MODEL for first quantization:
        # |a, b> couples to |a', b'> with amplitude t when:
        # a is a facing BCC state AND a' is its "neutralized" version (a with axis0 flipped to 0)
        # AND b' is b with the corresponding axis0 activated.
        # This is too complex.
        #
        # PRAGMATIC: use the swap operator. The inter-cube interaction
        # swaps amplitude between the facing BCC states of the two cubes.
        # V = t * sum_{facing (i,j)} (|i><j| tensor I27 + I27 tensor |j><i|) applied as:
        # For state |a,b>: V|a,b> = t*[delta(a=i)*|j,b> + delta(b=j)*|a,i>] for each pair
        #
        # EVEN MORE PRAGMATIC: treat it as Heisenberg-like coupling.
        # The observable "is cube X in BCC state i?" has operator P_i = |i><i|.
        # Couple: V = t * sum_{pairs} P_i^A x P_j^B
        # This is diagonal and just shifts energies. Not interesting enough.
        #
        # FINAL DECISION: use simple HOPPING between corresponding states.
        # For each facing pair (i_A, j_B):
        # V|i_A, b_B> += t * |j_A, b_B>... no that's wrong too.
        #
        # Let me use the standard condensed matter convention:
        # H_hop = -t * sum_{<A,B>, sigma} c_{A,sigma}^dag c_{B,sigma} + h.c.
        # where sigma runs over the internal states.
        # In first quantization: |sigma_A, vac_B> <-> |vac_A, sigma_B>
        # So the hopping is between |sigma, vac> and |vac, sigma>.
        #
        # For BCC contact: sigma runs over facing BCC pairs only.
        gs = 13  # center state = vacuum
        for i_A, j_B in pairs:
            # |i_A, gs_B> <-> |gs_A, j_B>
            idx1 = i_A * 27 + gs  # cube A excited, cube B vacuum
            idx2 = gs * 27 + j_B  # cube A vacuum, cube B excited
            V[idx1, idx2] += t_inter
            V[idx2, idx1] += t_inter

    elif model == 'center_tunnel':
        # Center of A talks to center of B
        # |center_A, b> <-> |b, center_B> ... no, that swaps everything.
        # Actually: vacuum tunneling means the vacuum itself hops.
        # |gs_A, gs_B> is the joint vacuum. The coupling is:
        # any state on A can tunnel to B IF it goes through the vacuum.
        # Simplest: |a, gs_B> <-> |gs_A, a> for ALL states a.
        gs = 13
        for a in range(27):
            idx1 = a * 27 + gs  # A in state a, B in vacuum
            idx2 = gs * 27 + a  # A in vacuum, B in state a
            V[idx1, idx2] += t_inter
            V[idx2, idx1] += t_inter

    elif model == 'cfl_hopping':
        # All states hop, weighted by proximity to boundary
        gs = 13
        for a in range(27):
            sh = moore_shell(STATES[a])
            # Weight: boundary states (BCC, shell 3) hop strongest
            # Center (shell 0) hops weakest
            weight = c_cfl ** (3 - sh)  # BCC: c^0=1, FCC: c^1, SC: c^2, Center: c^3
            idx1 = a * 27 + gs
            idx2 = gs * 27 + a
            V[idx1, idx2] += t_inter * weight
            V[idx2, idx1] += t_inter * weight

    return V


def build_h_2cube(delta, beta, g, J, t_inter, model):
    """Full 729x729 two-cube Hamiltonian."""
    H_single = build_h27(delta, beta, g, J)
    H = np.kron(H_single, I27) + np.kron(I27, H_single)
    H += build_v_inter(model, t_inter)
    return H


def bipartite_entropy_729(psi):
    """Von Neumann entropy tracing out cube B."""
    M = psi.reshape(27, 27)
    rho_A = M @ M.conj().T
    eigv = np.linalg.eigvalsh(rho_A)
    eigv = eigv[eigv > 1e-15]
    return -np.sum(eigv * np.log(eigv))


def shell_pops_cube(psi_27):
    """Shell populations for a 27-dim state vector."""
    probs = np.abs(psi_27)**2
    return np.array([probs[SHELL_INDICES[s]].sum() for s in range(4)])


# =============================================================================
# ANALYSES
# =============================================================================

def run_analyses(delta, beta, g, J, t_inter_default=0.05):
    H_single = build_h27(delta, beta, g, J)
    ev_single, evc_single = np.linalg.eigh(H_single)
    gs_single = 13  # center state

    models = ['bcc_contact', 'center_tunnel', 'cfl_hopping']
    model_names = ['BCC Contact (Strong)', 'Center Tunnel (Gravity)', 'CFL Hopping (Light)']

    # ==================================================================
    # 1. SPECTRUM COMPARISON
    # ==================================================================
    print('=' * 78)
    print('  1. SPECTRUM COMPARISON (t_inter = %.4f)' % t_inter_default)
    print('=' * 78)
    print()

    fig1, axes1 = plt.subplots(1, 3, figsize=(15, 5))

    for mi, (model, mname) in enumerate(zip(models, model_names)):
        H2 = build_h_2cube(delta, beta, g, J, t_inter_default, model)
        ev2 = np.linalg.eigvalsh(H2)

        # Reference: uncoupled = all pairwise sums
        ev_ref = np.sort([ev_single[i] + ev_single[j]
                          for i in range(27) for j in range(27)])

        # How many degeneracies were broken?
        n_ref = len(set(np.round(ev_ref, 6)))
        n_coupled = len(set(np.round(ev2, 6)))

        print('  %s:' % mname)
        print('    Uncoupled distinct: %d' % n_ref)
        print('    Coupled distinct: %d' % n_coupled)
        print('    Degeneracies broken: %d' % (n_coupled - n_ref))
        print('    Spectral shift: %.6e' % (ev2[0] - ev_ref[0]))
        print()

        ax = axes1[mi]
        ax.plot(ev_ref, np.arange(729), 'b-', alpha=0.3, linewidth=0.5, label='Uncoupled')
        ax.plot(ev2, np.arange(729), 'r-', alpha=0.5, linewidth=0.5, label='Coupled')
        ax.set_xlabel('Energy')
        ax.set_ylabel('State index')
        ax.set_title(mname, fontsize=9)
        ax.legend(fontsize=7)

    fig1.suptitle('Spectrum: Uncoupled vs Coupled (t=%.3f)' % t_inter_default, fontsize=11)
    fig1.tight_layout()
    path1 = os.path.join(OUTPUT_DIR, 'two_cube_spectrum.png')
    fig1.savefig(path1, dpi=150, bbox_inches='tight')
    plt.close(fig1)
    print('  Figure: %s' % path1)
    print()

    # ==================================================================
    # 2-3. BINDING ENERGY + VACUUM CORRELATION vs t_inter
    # ==================================================================
    print('=' * 78)
    print('  2-3. BINDING ENERGY & VACUUM CORRELATION vs COUPLING')
    print('=' * 78)
    print()

    t_vals = np.linspace(0, 0.2, 40)
    fig2, (ax2a, ax2b) = plt.subplots(1, 2, figsize=(13, 5))

    for mi, (model, mname) in enumerate(zip(models, model_names)):
        bindings = []
        vac_corrs = []
        for t in t_vals:
            H2 = build_h_2cube(delta, beta, g, J, t, model)
            ev2, evc2 = np.linalg.eigh(H2)
            bindings.append(ev2[0] - 2 * ev_single[0])

            # Vacuum correlation: <P_center_A * P_center_B>
            gs2 = evc2[:, 0]
            probs = np.abs(gs2)**2
            # P(center_A AND center_B) = prob of state (13, 13) = index 13*27+13
            p_both = probs[13 * 27 + 13]
            # P(center_A) = sum over b of prob(13, b)
            p_A = sum(probs[13 * 27 + b] for b in range(27))
            # P(center_B) = sum over a of prob(a*27 + 13)
            p_B = sum(probs[a * 27 + 13] for a in range(27))
            vac_corrs.append(p_both - p_A * p_B)

        colors = ['red', 'gold', 'steelblue']
        ax2a.plot(t_vals, bindings, color=colors[mi], linewidth=2, label=mname)
        ax2b.plot(t_vals, vac_corrs, color=colors[mi], linewidth=2, label=mname)

        if t_vals[-1] > 0:
            print('  %s at t=%.3f:' % (mname, t_vals[-1]))
            print('    Binding: %.6e' % bindings[-1])
            print('    Vac correlation: %.6e' % vac_corrs[-1])
            print()

    ax2a.set_xlabel('t_inter')
    ax2a.set_ylabel('Binding energy')
    ax2a.set_title('Binding Energy: E_gs(2-cube) - 2*E_gs(1-cube)')
    ax2a.axhline(0, color='gray', linestyle='--', alpha=0.5)
    ax2a.legend(fontsize=7)

    ax2b.set_xlabel('t_inter')
    ax2b.set_ylabel('Vacuum correlation')
    ax2b.set_title('Connected vacuum correlator')
    ax2b.axhline(0, color='gray', linestyle='--', alpha=0.5)
    ax2b.legend(fontsize=7)

    fig2.tight_layout()
    path2 = os.path.join(OUTPUT_DIR, 'two_cube_binding.png')
    fig2.savefig(path2, dpi=150, bbox_inches='tight')
    plt.close(fig2)
    print('  Figure: %s' % path2)
    print()

    # ==================================================================
    # 4. ENTANGLEMENT ENTROPY vs t_inter
    # ==================================================================
    print('=' * 78)
    print('  4. INTER-CUBE ENTANGLEMENT vs COUPLING')
    print('=' * 78)
    print()

    fig3, ax3 = plt.subplots(figsize=(9, 5))

    for mi, (model, mname) in enumerate(zip(models, model_names)):
        entropies = []
        for t in t_vals:
            H2 = build_h_2cube(delta, beta, g, J, t, model)
            _, evc2 = np.linalg.eigh(H2)
            entropies.append(bipartite_entropy_729(evc2[:, 0]))

        ax3.plot(t_vals, entropies, color=colors[mi], linewidth=2, label=mname)
        print('  %s: max S = %.6f at t=%.3f' % (mname, max(entropies), t_vals[np.argmax(entropies)]))

    ax3.set_xlabel('t_inter')
    ax3.set_ylabel('Entanglement entropy S(rho_A)')
    ax3.set_title('Inter-Cube Entanglement')
    ax3.axhline(np.log(27), color='gray', linestyle='--', alpha=0.3, label='S_max=ln(27)')
    ax3.legend(fontsize=8)

    path3 = os.path.join(OUTPUT_DIR, 'two_cube_entanglement.png')
    fig3.savefig(path3, dpi=150, bbox_inches='tight')
    plt.close(fig3)
    print('  Figure: %s' % path3)
    print()

    # ==================================================================
    # 5-6. EXCITATION PROPAGATION + CHANNEL SELECTIVITY
    # ==================================================================
    print('=' * 78)
    print('  5-6. EXCITATION PROPAGATION & CHANNEL SELECTIVITY')
    print('=' * 78)
    print()

    fig4, axes4 = plt.subplots(2, 3, figsize=(15, 8))
    t_evol = np.linspace(0, 150, 500)

    # Initial state: cube A in |+++> (BCC corner), cube B in ground state
    bcc_corner = 0  # |+++>
    gs_vec = evc_single[:, 0]  # single-cube ground state

    psi0 = np.zeros(729, dtype=complex)
    for b in range(27):
        psi0[bcc_corner * 27 + b] = gs_vec[b]
    psi0 /= np.linalg.norm(psi0)

    for mi, (model, mname) in enumerate(zip(models, model_names)):
        H2 = build_h_2cube(delta, beta, g, J, t_inter_default, model)
        ev2, evc2 = np.linalg.eigh(H2)
        coeffs = evc2.T @ psi0

        # Shell populations on each cube over time
        shell_A = np.zeros((len(t_evol), 4))
        shell_B = np.zeros((len(t_evol), 4))

        for ti, t in enumerate(t_evol):
            phases = np.exp(-1j * ev2 * t)
            psi_t = evc2 @ (coeffs * phases)

            # Reduced state on cube A: trace out B
            M = psi_t.reshape(27, 27)
            rho_A_diag = np.real(np.sum(np.abs(M)**2, axis=1))
            # Reduced state on cube B: trace out A
            rho_B_diag = np.real(np.sum(np.abs(M)**2, axis=0))

            for sh in range(4):
                shell_A[ti, sh] = rho_A_diag[SHELL_INDICES[sh]].sum()
                shell_B[ti, sh] = rho_B_diag[SHELL_INDICES[sh]].sum()

        # Plot cube A shells
        ax_a = axes4[0, mi]
        ax_b = axes4[1, mi]
        shell_colors = ['#ffd54f', '#44aaff', '#44cc66', '#ff4432']

        for sh in range(4):
            ax_a.plot(t_evol, shell_A[:, sh], color=shell_colors[sh], linewidth=1)
            ax_b.plot(t_evol, shell_B[:, sh], color=shell_colors[sh], linewidth=1,
                      label=SHELL_NAMES[sh])

        ax_a.set_title('Cube A: %s' % mname, fontsize=8)
        ax_a.set_ylabel('Shell pop')
        ax_a.set_ylim(-0.05, 1.05)
        ax_b.set_title('Cube B: %s' % mname, fontsize=8)
        ax_b.set_xlabel('t')
        ax_b.set_ylabel('Shell pop')
        ax_b.set_ylim(-0.05, 0.5)
        if mi == 2:
            ax_b.legend(fontsize=7, loc='upper right')

        # Channel selectivity: what shell does the excitation arrive in on B?
        max_B = shell_B.max(axis=0)
        arrival_shell = np.argmax(max_B)
        print('  %s:' % mname)
        print('    Excitation starts: cube A BCC (|+++>)')
        print('    Max arrival on cube B: %s (max P = %.4f)' %
              (SHELL_NAMES[arrival_shell], max_B[arrival_shell]))
        print('    All shells max: Center=%.4f SC=%.4f FCC=%.4f BCC=%.4f' %
              tuple(max_B))
        print()

    fig4.suptitle('Excitation Propagation: BCC on Cube A -> Cube B', fontsize=11)
    fig4.tight_layout()
    path4 = os.path.join(OUTPUT_DIR, 'two_cube_propagation.png')
    fig4.savefig(path4, dpi=150, bbox_inches='tight')
    plt.close(fig4)
    print('  Figure: %s' % path4)
    print()

    # ==================================================================
    # 7. DISPERSION RELATION (Bloch bands)
    # ==================================================================
    print('=' * 78)
    print('  7. DISPERSION RELATION (periodic 2-cube chain)')
    print('=' * 78)
    print()

    fig5, axes5 = plt.subplots(1, 3, figsize=(15, 5))
    k_vals = np.linspace(0, np.pi, 50)

    for mi, (model, mname) in enumerate(zip(models, model_names)):
        # Bloch Hamiltonian: H(k) = H_single + V_hop * e^{ik} + V_hop^dag * e^{-ik}
        # For our hopping: V_hop is the 27x27 hopping matrix (A->B direction)
        V_hop = np.zeros((27, 27))

        if model == 'bcc_contact':
            for i_A, j_B in facing_bcc_pairs():
                V_hop[gs_single, j_B] += t_inter_default  # |i> -> |gs>, |gs> -> |j>
                # Actually: single-particle hopping from state i to state j
                # V_hop[i_A, j_B] += t_inter_default... but this isn't quite right
                # for Bloch bands of the single-particle sector.
                # For simplicity, use the center-mediated hopping:
                gs = 13
                V_hop[i_A, gs] += t_inter_default * 0.5
                V_hop[gs, j_B] += t_inter_default * 0.5

        elif model == 'center_tunnel':
            gs = 13
            for a in range(27):
                V_hop[a, a] += t_inter_default

        elif model == 'cfl_hopping':
            for a in range(27):
                sh = moore_shell(STATES[a])
                weight = c_cfl ** (3 - sh)
                V_hop[a, a] += t_inter_default * weight

        bands = np.zeros((len(k_vals), 27))
        for ki, k in enumerate(k_vals):
            H_k = H_single + V_hop * np.exp(1j * k) + V_hop.conj().T * np.exp(-1j * k)
            bands[ki, :] = np.linalg.eigvalsh(H_k)

        ax = axes5[mi]
        for band in range(27):
            ax.plot(k_vals, bands[:, band], 'steelblue', linewidth=0.5, alpha=0.6)
        ax.set_xlabel('k')
        ax.set_ylabel('E(k)')
        ax.set_title(mname, fontsize=9)
        ax.set_xlim(0, np.pi)
        ax.set_xticks([0, np.pi / 2, np.pi])
        ax.set_xticklabels(['0', 'pi/2', 'pi'])

        # Bandwidth
        bw = bands[-1, :].max() - bands[0, :].min()
        max_band_bw = max(bands[:, n].max() - bands[:, n].min() for n in range(27))
        print('  %s: max single-band bandwidth = %.6f' % (mname, max_band_bw))

    fig5.suptitle('Dispersion Relations: E(k) for 27 Bloch Bands', fontsize=11)
    fig5.tight_layout()
    path5 = os.path.join(OUTPUT_DIR, 'two_cube_dispersion.png')
    fig5.savefig(path5, dpi=150, bbox_inches='tight')
    plt.close(fig5)
    print('  Figure: %s' % path5)
    print()


# =============================================================================
# MAIN
# =============================================================================

if __name__ == "__main__":
    print('=' * 78)
    print('  TWO-CUBE LATTICE EXPERIMENT')
    print('  729-state joint system (27 x 27)')
    print('  Three coupling models compared head-to-head')
    print('=' * 78)
    print()
    print('  Parameters:')
    print('    Delta = %.6f, beta = %.6f, g = %.6f, J = %.6f' %
          (delta, beta, g_hop, J_int))
    print('    c_CFL = 1/sqrt(3) = %.6f' % c_cfl)
    print()
    print('  BCC facing pairs (axis 0):')
    for i, j in facing_bcc_pairs():
        print('    %s (A) <-> %s (B)' % (sl(i), sl(j)))
    print()

    run_analyses(delta, beta, g_hop, J_int, t_inter_default=0.05)

    print('=' * 78)
    print('  EXPERIMENT COMPLETE')
    print('=' * 78)

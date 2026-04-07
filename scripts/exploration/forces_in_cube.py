#!/usr/bin/env python3
"""
All Four Forces + Time in the 27-State Ternary Cube
=====================================================

Moore shells = gauge groups = force sectors:
  Center (1)  -> Gravity       (spin-2, G_N = 1/(b3+Nc)^2)
  SC     (6)  -> U(1) EM       (photon, alpha = 1/x+)
  FCC    (12) -> SU(2) Weak    (W/Z, sin^2(theta_W) = Nc/Neff)
  BCC    (8)  -> SU(3) Strong  (gluon, alpha_s = b3/(b3+4*Neff))

Time: CFL condition c = 1/sqrt(3), tick processes G*^2 energy per DOF
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
mu = (PI_D + G_STAR) / 2.0
g_hop = delta
J_int = 0.05

I3 = np.eye(3)
Z = np.diag([1.0, 0.0, -1.0])

def build_h1(d, b, g_):
    return np.array([[d, g_, 0], [g_, b, g_], [0, g_, -d]])

def build_h27(d, b, g_, J_):
    H1 = build_h1(d, b, g_)
    H = (np.kron(np.kron(H1, I3), I3) +
         np.kron(np.kron(I3, H1), I3) +
         np.kron(np.kron(I3, I3), H1))
    if abs(J_) > 1e-15:
        H += J_ * (np.kron(np.kron(Z, Z), I3) +
                    np.kron(np.kron(Z, I3), Z) +
                    np.kron(np.kron(I3, Z), Z))
    return H

ST = [(a, b, c) for a in range(3) for b in range(3) for c in range(3)]
ms = lambda s: sum(1 for x in s if x != 1)
zv = lambda idx: 1 if idx == 0 else (0 if idx == 1 else -1)
LB = ['+', '0', '-']
sl = lambda n: '|%s%s%s>' % (LB[ST[n][0]], LB[ST[n][1]], LB[ST[n][2]])

SHELLS = {sh: [i for i in range(27) if ms(ST[i]) == sh] for sh in range(4)}

# Coupling constants
alpha_em = 1.0 / X_PLUS
sin2_w = float(N_c) / float(N_eff)
alpha_s = float(b_3) / float(b_3 + 4 * N_eff)
G_N = 1.0 / float(b_3 + N_c)**2
c_lattice = 1.0 / np.sqrt(3)

H = build_h27(delta, beta, g_hop, J_int)
ev_full, evc_full = np.linalg.eigh(H)

# ==============================================================================
print('=' * 78)
print('  ALL FOUR FORCES IN THE 27-STATE TERNARY CUBE')
print('=' * 78)
print()

# 1. THE FORCE MAP
print('  1. FORCE MAP: Shell -> Gauge Group -> Coupling')
print()
forces = [
    ('Center', 0, 1, 'Gravity', 'Graviton', G_N, '1/(b3+Nc)^2 = 1/100'),
    ('SC', 1, 6, 'U(1) EM', 'Photon', alpha_em, '1/x+ = alpha'),
    ('FCC', 2, 12, 'SU(2) Weak', 'W/Z', sin2_w, 'Nc/Neff = 3/13'),
    ('BCC', 3, 8, 'SU(3) Strong', 'Gluon', alpha_s, 'b3/(b3+4Neff) = 7/59'),
]

print('  %-8s Sh Cnt %-10s %-8s %-10s Formula' % ('Shell', 'Gauge', 'Boson', 'Coupling'))
print('  ' + '-' * 72)
for name, sh, cnt, gauge, boson, coupling, formula in forces:
    print('  %-8s %d  %2d  %-10s %-8s %.6f   %s' %
          (name, sh, cnt, gauge, boson, coupling, formula))
print()

# Coupling hierarchy
print('  Coupling hierarchy (relative to G*):')
for name, _, _, gauge, _, coupling, _ in forces:
    cg = coupling * G_STAR
    print('    %-10s alpha=%.6f  alpha*G*=%.6f  log10(alpha)=%+.2f' %
          (gauge, coupling, cg, np.log10(coupling)))
print()

# 2. SHELL-RESTRICTED HAMILTONIANS
print('  2. SHELL-RESTRICTED EIGENVALUE SPECTRA')
print()

for name, sh, cnt, gauge, _, coupling, _ in forces:
    idx = SHELLS[sh]
    H_sh = np.array([[H[i, j] for j in idx] for i in idx])
    ev_sh = np.linalg.eigvalsh(H_sh)
    bw = ev_sh[-1] - ev_sh[0] if len(ev_sh) > 1 else 0
    print('  %s (%s, %d states):' % (name, gauge, cnt))
    print('    Eigenvalues: %s' % ', '.join('%.5f' % e for e in ev_sh))
    print('    Bandwidth: %.6f' % bw)
    # Ratio of bandwidth to coupling constant
    if coupling > 0:
        print('    Bandwidth/alpha: %.4f' % (bw / coupling))
    print()

# 3. INTER-SHELL COUPLING MATRIX
print('  3. INTER-SHELL COUPLING (force-to-force transition amplitudes)')
print()
shell_names = ['Gravity', 'U(1)', 'SU(2)', 'SU(3)']
print('  From\\To    %-10s %-10s %-10s %-10s' % tuple(shell_names))
print('  ' + '-' * 52)
for sh1 in range(4):
    row = '  %-10s' % shell_names[sh1]
    for sh2 in range(4):
        idx1, idx2 = SHELLS[sh1], SHELLS[sh2]
        block = np.array([[H[i, j] for j in idx2] for i in idx1])
        # RMS coupling per pair
        rms = np.sqrt(np.mean(block**2))
        row += ' %-10.6f' % rms
    print(row)
print()

# 4. TRANSPORT FROM CENTER THROUGH EACH FORCE CHANNEL
print('  4. TRANSPORT: Center -> each force sector')
print()
t_vals = np.linspace(0, 100, 500)
center = 13

for name, sh, cnt, gauge, _, coupling, _ in forces:
    idx = SHELLS[sh]
    # Total population transfer to this shell
    shell_transfer = np.zeros(len(t_vals))
    for tidx in idx:
        amp = evc_full[center, :].conj() * evc_full[tidx, :]
        phases = np.exp(-1j * np.outer(t_vals, ev_full))
        tr = np.abs((phases * amp[np.newaxis, :]).sum(axis=1))**2
        shell_transfer += tr
    max_tr = shell_transfer.max()
    t_max = t_vals[np.argmax(shell_transfer)]
    print('  |000> -> %s shell: max P = %.6f at t = %.1f' %
          (gauge, max_tr, t_max))
print()

# 5. CFL AND TIME STRUCTURE
print('  5. CFL SPEED OF LIGHT AND THE TICK')
print()
print('  c = 1/sqrt(D) = 1/sqrt(3) = %.10f' % c_lattice)
print('  c^2 = 1/D = 1/3 = %.10f' % c_lattice**2)
print()
print('  G* and the near-coincidence c^2 ~ 1/G*:')
print('    1/G* = %.10f' % (1 / G_STAR))
print('    1/3  = %.10f' % (1.0 / 3))
print('    Deviation: 1/G* - 1/3 = %.6e' % (1 / G_STAR - 1.0 / 3))
print('    Fractional: %.4f%%' % ((1 / G_STAR - 1.0 / 3) / (1.0 / 3) * 100))
print()

# G* dimensional triad
print('  G* DIMENSIONAL TRIAD (energy per DOF):')
print('    G*^1 = %.6f  (FLUX: spatial amplitude per DOF)' % G_STAR)
print('    G*^2 = %.6f  (TIME: energy quantum per DOF per tick)' % G_STAR**2)
print('    G*^3 = %.6f  (ACTION: spatiotemporal record per DOF)' % G_STAR**3)
print()
print('  Vieta relations connect forces to time budget:')
print('    x+ + x- = 16*G*^2 = %.4f (total energy per tick)' % (16 * G_STAR**2))
print('    x+ * x- = 16*G*^3 = %.4f (total action per tick)' % (16 * G_STAR**3))
print('    P/S = G* (harmonic balance = circle scale)')
print()

# 6. TIME AS A SECOND LATTICE
print('=' * 78)
print('  6. TIME AS A SECOND LATTICE: THE 27x27 SPACETIME CUBE')
print('=' * 78)
print()
print('  The spatial cube has 27 states on (S^1)^3.')
print('  Time adds a SECOND index: the tick number.')
print()
print('  The full spacetime structure is a 27-state chain:')
print('    |a,b,c; t> where (a,b,c) is the spatial state and t is the tick.')
print()
print('  Time evolution: |psi(t+1)> = e^{-iH} |psi(t)>')
print('  The propagator U = e^{-iH} maps one spatial slice to the next.')
print()

# Compute the propagator for one tick
U = np.zeros((27, 27), dtype=complex)
for n in range(27):
    U[:, n] = evc_full[:, :] @ (np.exp(-1j * ev_full) * evc_full[n, :])
# Simpler: U = evecs @ diag(exp(-i*E)) @ evecs^T
U = evc_full @ np.diag(np.exp(-1j * ev_full)) @ evc_full.T

print('  One-tick propagator U = exp(-iH):')
print('    |U| should be unitary: ||U*U^dag - I|| = %.2e' %
      np.linalg.norm(U @ U.conj().T - np.eye(27)))
print()

# How much does one tick mix shells?
print('  Shell-to-shell transfer in ONE TICK:')
print('  (probability of ending in each shell, starting from a shell)')
print()
print('  Start\\End  Center    SC       FCC      BCC')
for sh1 in range(4):
    idx1 = SHELLS[sh1]
    row = '  %-10s' % shell_names[sh1]
    for sh2 in range(4):
        idx2 = SHELLS[sh2]
        # Average over starting states in sh1, sum over ending states in sh2
        total = 0
        for i in idx1:
            for j in idx2:
                total += abs(U[j, i])**2
            # Normalize: probability of ending in sh2
        total /= len(idx1)
        row += ' %.5f ' % total
    print(row)

print()

# The KEY: how many ticks to reach BCC from center?
print('  TICK COUNT TO REACH BCC FROM CENTER:')
state = np.zeros(27, dtype=complex)
state[center] = 1.0
for tick in range(20):
    shell_pops = [sum(abs(state[i])**2 for i in SHELLS[sh]) for sh in range(4)]
    if tick < 5 or tick % 5 == 0 or shell_pops[3] > 0.01:
        print('    tick %2d: Center=%.4f SC=%.4f FCC=%.4f BCC=%.4f' %
              (tick, shell_pops[0], shell_pops[1], shell_pops[2], shell_pops[3]))
    state = U @ state

print()

# 7. THE SPACETIME LATTICE: spatial cube x temporal chain
print('  7. THE SPACETIME LATTICE')
print()
print('  Spatial lattice: 27 states, 3 axes, Moore neighborhood')
print('  Temporal lattice: tick chain, propagator U = exp(-iH)')
print()
print('  Full spacetime: 27 x T states (T ticks)')
print('  The propagator U encodes:')
print('    - Diagonal: how much each state persists (survival amplitude)')
print('    - Off-diagonal: how states mix (transition amplitude)')
print()

# Survival amplitudes (diagonal of U)
print('  Survival amplitudes |U_nn| per shell:')
for sh in range(4):
    idx = SHELLS[sh]
    survivals = [abs(U[i, i]) for i in idx]
    print('    %s: mean |U_nn| = %.6f (range [%.4f, %.4f])' %
          (shell_names[sh], np.mean(survivals), min(survivals), max(survivals)))

print()

# The spacetime metric: ds^2 = -c^2 dt^2 + dx^2
# In lattice units: c = 1/sqrt(3), dx = 1 (lattice spacing), dt = 1 (tick)
# Light cone: after N ticks, can reach distance N*c = N/sqrt(3) lattice units
# To reach BCC corner (distance sqrt(3)), need sqrt(3)/c = sqrt(3)*sqrt(3) = 3 ticks

print('  CAUSAL STRUCTURE:')
print('    c = 1/sqrt(3) = %.6f lattice units per tick' % c_lattice)
print('    Light-travel distance in 1 tick: %.6f' % c_lattice)
print('    Distance to SC shell (d=1):     1.000')
print('    Distance to FCC shell (d=sqrt2): %.6f' % np.sqrt(2))
print('    Distance to BCC shell (d=sqrt3): %.6f' % np.sqrt(3))
print()
print('    Ticks to reach each shell (light-cone):')
for name, _, _, _, _, _, _ in forces:
    sh = [f[1] for f in forces if f[0] == name][0]
    r = [0, 1, np.sqrt(2), np.sqrt(3)][sh]
    ticks = r / c_lattice if r > 0 else 0
    print('      %s (r=%.3f): %.3f ticks (= %.3f * sqrt(3))' %
          (name, r, ticks, ticks / np.sqrt(3) if ticks > 0 else 0))

print()
print('    BCC corners are exactly sqrt(3)/c = 3 ticks away.')
print('    It takes EXACTLY N_c ticks for light to reach the strong force shell.')
print('    This is the lattice-causal meaning of N_c = 3.')
print()

# Summary
print('=' * 78)
print('  SUMMARY: THE FORCE HIERARCHY IS THE SHELL HIERARCHY')
print('=' * 78)
print()
print('  Shell  Force     Coupling   Distance  Ticks  States  Role')
print('  ' + '-' * 72)
print('  0      Gravity   %.6f   0          0      1       Anchor (vacuum)' % G_N)
print('  1      U(1) EM   %.6f   1          %.1f    6       Phase rotation' % (alpha_em, 1/c_lattice))
print('  2      SU(2) Wk  %.6f   sqrt(2)    %.1f   12       Isospin mixing' % (sin2_w, np.sqrt(2)/c_lattice))
print('  3      SU(3) St  %.6f   sqrt(3)    %.1f    8       Color confinement' % (alpha_s, np.sqrt(3)/c_lattice))
print()
print('  Forces are ORDERED by distance from center:')
print('    Gravity < EM < Weak < Strong')
print('  This is the INVERSE of the usual strength ordering!')
print('  In the cube, the strongest force is the FARTHEST from the anchor.')
print('  The weakest force (gravity) IS the anchor.')
print()
print('  Gravity is not a force in the usual sense.')
print('  Gravity is what it means to BE the center.')
print('  The center state |000> has no charge (Q=0), no color,')
print('  and it defines the origin of the coordinate system.')
print('  Gravity = the potential well created by beta < 0.')
print('  Gravity = the cost of being NOT at G*.')


if __name__ == "__main__":
    pass

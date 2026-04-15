"""Moore Laplacian dark-mass spectrum for FTD Wave 5.5 ad hoc.

Computes the 27-site 3x3x3 Moore Laplacian exactly, classifies each
eigenvalue by its S_3 representation content (trivial = visible, sign =
dark singlet, standard = dark doublet), and applies the FTD mass ansatz
m = mu(n=17) * sqrt(lambda) to predict the 8 dark mass lines.
"""

import numpy as np
from itertools import product, permutations

# ----------------------------------------------------------------------
# 27-site Moore Laplacian
# ----------------------------------------------------------------------
sites = list(product(range(3), range(3), range(3)))
N = 27
idx = {s: n for n, s in enumerate(sites)}

A = np.zeros((N, N), dtype=int)
for a in sites:
    for b in sites:
        if a != b and max(abs(a[0]-b[0]), abs(a[1]-b[1]), abs(a[2]-b[2])) == 1:
            A[idx[a], idx[b]] = 1
L = np.diag(A.sum(axis=1)) - A

# ----------------------------------------------------------------------
# S_3 projectors (axis-permutation subgroup)
# ----------------------------------------------------------------------
def perm_matrix(sigma):
    P = np.zeros((N, N), dtype=int)
    for s in sites:
        s_new = tuple(s[sigma[k]] for k in range(3))
        P[idx[s_new], idx[s]] = 1
    return P

S3_perms = list(permutations([0, 1, 2]))
sign_of = {}
for sigma in S3_perms:
    inv = sum(1 for i in range(3) for j in range(i+1, 3) if sigma[i] > sigma[j])
    sign_of[sigma] = 1 if inv % 2 == 0 else -1

P_triv = sum(perm_matrix(sigma) for sigma in S3_perms) / 6.0
P_sign = sum(sign_of[sigma] * perm_matrix(sigma) for sigma in S3_perms) / 6.0
P_std = np.eye(N) - P_triv - P_sign

# ----------------------------------------------------------------------
# Classify eigenvalues
# ----------------------------------------------------------------------
eigs, V = np.linalg.eigh(L)
eigs_r = np.round(eigs, decimals=6)
unique_eigs = sorted(set(eigs_r))

print("="*85)
print("Moore Laplacian spectrum (3x3x3 cube, open boundary, 26-connectivity)")
print("="*85)
print(f"{'lambda':>12} {'mult':>5} {'vis(triv)':>12} {'sign':>6} {'std':>6} {'type':>12}")
print("-"*85)

results = []
for u in unique_eigs:
    mask = np.abs(eigs - u) < 1e-5
    mult = int(mask.sum())
    sub_vecs = V[:, mask]
    triv_w = round(float(np.trace(sub_vecs.T @ P_triv @ sub_vecs)))
    sign_w = round(float(np.trace(sub_vecs.T @ P_sign @ sub_vecs)))
    std_w = round(float(np.trace(sub_vecs.T @ P_std @ sub_vecs)))

    tag = "visible" if sign_w == 0 and std_w == 0 else \
          "DARK" if triv_w == 0 else \
          "mixed"
    print(f"{u:>12.4f} {mult:>5d} {triv_w:>12d} {sign_w:>6d} {std_w:>6d} {tag:>12}")
    results.append((u, mult, triv_w, sign_w, std_w))

total_vis = sum(r[2] for r in results)
total_sign = sum(r[3] for r in results)
total_std = sum(r[4] for r in results)
print("-"*85)
print(f"Totals: visible={total_vis}  sign={total_sign}  standard={total_std}  "
      f"(sum={total_vis+total_sign+total_std})")

# ----------------------------------------------------------------------
# Mass prediction
# ----------------------------------------------------------------------
print("\n" + "="*85)
print("FTD dark sector mass prediction")
print("="*85)

# Ladder scale
m_P = 1.2209e19          # GeV
alpha = 1 / 137.036
n_dark = 17
mu_dark_GeV = m_P * np.sqrt(2 * np.pi) * alpha**n_dark
mu_dark_neV = mu_dark_GeV * 1e18  # GeV -> neV

print(f"\nLadder scale  mu(n=17) = m_P * sqrt(2*pi) * alpha^17 = {mu_dark_neV:.3f} neV")
print(f"Ansatz        m(lambda) = mu(17) * sqrt(lambda)")
print(f"                      This gives no free prefactor.\n")

print(f"{'#':>4} {'lambda':>10} {'deg':>5} {'mass (neV)':>14} {'sign':>6} {'std':>5} notes")
print("-"*85)

dark_lines = []
line = 0
for u, mult, triv, sgn, std in results:
    dark_deg = sgn + std
    if dark_deg == 0:
        continue
    line += 1
    mass = mu_dark_neV * np.sqrt(u)
    note = ""
    if sgn == 1:
        note = f"<-- SIGN REP at lambda={int(u)} (= N_eff)"
    dark_lines.append((line, u, dark_deg, mass, sgn, std, note))
    print(f"{line:>4d} {u:>10.4f} {dark_deg:>5d} {mass:>14.3f} {sgn:>6d} {std:>5d} {note}")

print("\n--- Spectrum Summary ---")
print(f"Distinct dark mass lines:  {line}")
print(f"Total dark states:         {sum(l[2] for l in dark_lines)} (= 17)")
print(f"Mass range:                {min(l[3] for l in dark_lines):.2f}  -  "
      f"{max(l[3] for l in dark_lines):.2f} neV")

total_mass = sum(l[2] * l[3] for l in dark_lines)
mean_mass = total_mass / sum(l[2] for l in dark_lines)
print(f"Mean dark mass:            {mean_mass:.2f} neV")
print(f"Sum of all 17 masses:      {total_mass:.1f} neV")

print("\n--- Degeneracy pattern ---")
print("  [", ", ".join(str(l[2]) for l in dark_lines), "]")
print(f"  (sum = {sum(l[2] for l in dark_lines)})")

# Cross check: mass ratios between adjacent lines
print("\n--- Mass ratios (consecutive) ---")
for i in range(1, len(dark_lines)):
    r = dark_lines[i][3] / dark_lines[i-1][3]
    print(f"  m_{i+1}/m_{i} = {r:.4f}")

# Verify the framework-integer coincidence: 13 is an eigenvalue AND is N_eff AND is where sign rep lives
print("\n--- Framework integer check ---")
integer_eigenvalues = [u for u, *_ in results if abs(u - round(u)) < 1e-6]
print(f"Integer eigenvalues: {integer_eigenvalues}")
print("Framework integers in FTD: N_c=3, N_base=4, b_3=7, N_eff=13, D=47, 3^D=27")
for u, mult, triv, sgn, std in results:
    if abs(u - round(u)) < 1e-6:
        print(f"  lambda = {int(u):>2}  "
              f"(triv={triv}, sign={sgn}, std={std})  "
              f"{'<-- contains sign rep!' if sgn==1 else ''}")

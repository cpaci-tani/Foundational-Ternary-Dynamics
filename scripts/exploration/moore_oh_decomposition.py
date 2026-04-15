"""Decompose Moore Laplacian eigenvalues into O_h irreps.

For the 3x3x3 Moore neighborhood with open boundary (27 sites, 26-connectivity),
compute all 13 Laplacian eigenvalues, classify each by O_h irrep (A1g, A2g, Eg,
T1g, T2g, A1u, A2u, Eu, T1u, T2u) and by S_3 subgroup (trivial, sign, standard).

Used for the FTD dark-matter-from-17-Moore-states derivation (Wave 5.5 ad hoc).
"""

import numpy as np
from itertools import product, permutations

# ----------------------------------------------------------------------
# Build the 27-site Moore Laplacian
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
# Build the O_h group action on 27 sites.
# O_h = S_3 semi-direct (Z_2)^3: signed permutations of the 3 axes.
# Inversion on coordinate i: i -> 2-i.
# ----------------------------------------------------------------------
def oh_matrix(sigma, signs):
    P = np.zeros((N, N), dtype=int)
    for s in sites:
        reordered = tuple(s[sigma[k]] for k in range(3))
        flipped = tuple(reordered[k] if signs[k] == 1 else 2 - reordered[k] for k in range(3))
        P[idx[flipped], idx[s]] = 1
    return P

def cycle_decomp(sigma):
    seen = [False]*3
    cycles = []
    for i in range(3):
        if seen[i]:
            continue
        c = [i]
        j = sigma[i]
        while j != i:
            c.append(j)
            seen[j] = True
            j = sigma[j]
        seen[i] = True
        cycles.append(c)
    return cycles

S3_perms = list(permutations([0, 1, 2]))
sign_groups = list(product([1, -1], repeat=3))
Oh_elements = [(sigma, signs) for sigma in S3_perms for signs in sign_groups]
assert len(Oh_elements) == 48

# ----------------------------------------------------------------------
# Conjugacy class assignment
# Classes of O_h (Schoenflies): E, 8C3, 6C2p, 6C4, 3C2, i, 6S4, 8S6, 3sh, 6sd
# ----------------------------------------------------------------------
def class_of_g(sigma, signs):
    if sigma == (0, 1, 2) and signs == (1, 1, 1):
        return 0  # E
    if sigma == (0, 1, 2) and signs == (-1, -1, -1):
        return 5  # i

    if sigma == (0, 1, 2):
        n_neg = sum(1 for x in signs if x == -1)
        if n_neg == 2:
            return 4  # 3C2
        if n_neg == 1:
            return 8  # 3sigma_h

    cycles = cycle_decomp(sigma)
    if len(cycles) == 1 and len(cycles[0]) == 3:
        n_neg = sum(1 for x in signs if x == -1)
        det = (-1)**n_neg * 1
        if det == 1:
            return 1  # 8C3
        else:
            return 7  # 8S6

    if any(len(c) == 2 for c in cycles):
        n_neg = sum(1 for x in signs if x == -1)
        if n_neg == 0:
            return 2  # 6C2'
        if n_neg == 2:
            return 3  # 6C4
        fixed_axes = [i for i in range(3) if sigma[i] == i]
        if len(fixed_axes) == 1:
            fixed = fixed_axes[0]
            sign_on_fixed = signs[fixed]
            if n_neg == 1:
                return 9 if sign_on_fixed == -1 else 6
            if n_neg == 3:
                return 6 if sign_on_fixed == -1 else 9
    return -1

class_count = [0]*10
for sigma, signs in Oh_elements:
    c = class_of_g(sigma, signs)
    if c >= 0:
        class_count[c] += 1

class_sizes = [1, 8, 6, 6, 3, 1, 6, 8, 3, 6]
print(f"Class sizes computed: {class_count}")
print(f"Class sizes expected: {class_sizes}")

# ----------------------------------------------------------------------
# O_h character table
# Cols: E, 8C3, 6C2', 6C4, 3C2, i, 6S4, 8S6, 3s_h, 6s_d
# ----------------------------------------------------------------------
char_table = {
    "A1g": [ 1,  1,  1,  1,  1,  1,  1,  1,  1,  1],
    "A2g": [ 1,  1, -1, -1,  1,  1, -1,  1,  1, -1],
    "Eg":  [ 2, -1,  0,  0,  2,  2,  0, -1,  2,  0],
    "T1g": [ 3,  0, -1,  1, -1,  3,  1,  0, -1, -1],
    "T2g": [ 3,  0,  1, -1, -1,  3, -1,  0, -1,  1],
    "A1u": [ 1,  1,  1,  1,  1, -1, -1, -1, -1, -1],
    "A2u": [ 1,  1, -1, -1,  1, -1,  1, -1, -1,  1],
    "Eu":  [ 2, -1,  0,  0,  2, -2,  0,  1, -2,  0],
    "T1u": [ 3,  0, -1,  1, -1, -3, -1,  0,  1,  1],
    "T2u": [ 3,  0,  1, -1, -1, -3,  1,  0,  1, -1],
}

# One representative per class (first seen)
class_reps = [None]*10
for sigma, signs in Oh_elements:
    c = class_of_g(sigma, signs)
    if c >= 0 and class_reps[c] is None:
        class_reps[c] = oh_matrix(sigma, signs)

# S_3 projectors for visible/sign/standard classification
def perm_matrix_s3(sigma):
    P = np.zeros((N, N), dtype=int)
    for s in sites:
        s_new = tuple(s[sigma[k]] for k in range(3))
        P[idx[s_new], idx[s]] = 1
    return P

sign_of = {}
for sigma in S3_perms:
    inv = sum(1 for i in range(3) for j in range(i+1, 3) if sigma[i] > sigma[j])
    sign_of[sigma] = 1 if inv % 2 == 0 else -1

P_triv = sum(perm_matrix_s3(sigma) for sigma in S3_perms) / 6.0
P_sign = sum(sign_of[sigma] * perm_matrix_s3(sigma) for sigma in S3_perms) / 6.0
P_std = np.eye(N) - P_triv - P_sign

# ----------------------------------------------------------------------
# Eigenvalue analysis
# ----------------------------------------------------------------------
eigs, V = np.linalg.eigh(L)
eigs_r = np.round(eigs, decimals=4)
unique_eigs = sorted(set(eigs_r))

print("\n" + "="*100)
print(f"{'eigenvalue':>12} {'mult':>5} {'O_h irreps':>30} {'S_3 (trv/sgn/std)':>22} {'visible?':>10}")
print("-"*100)

total_vis = 0
total_sign = 0
total_std = 0
results = []
for u in unique_eigs:
    mask = np.abs(eigs - u) < 1e-5
    mult = int(mask.sum())
    sub_vecs = V[:, mask]
    P_eig = sub_vecs @ sub_vecs.T

    chis = [np.trace(P_eig @ class_reps[c]) for c in range(10)]
    decomp = {}
    for irrep_name, irrep_chars in char_table.items():
        m = sum(class_sizes[c] * chis[c] * irrep_chars[c] for c in range(10)) / 48
        if abs(m) > 0.01:
            decomp[irrep_name] = round(m.real)

    triv_w = round(np.trace(sub_vecs.T @ P_triv @ sub_vecs).real)
    sign_w = round(np.trace(sub_vecs.T @ P_sign @ sub_vecs).real)
    std_w = round(np.trace(sub_vecs.T @ P_std @ sub_vecs).real)

    total_vis += triv_w
    total_sign += sign_w
    total_std += std_w

    decomp_str = " + ".join(f"{k}" if v == 1 else f"{v}{k}" for k, v in decomp.items())
    s3_str = f"{triv_w}/{sign_w}/{std_w}"
    dark_tag = "visible" if triv_w == mult else ("mixed" if triv_w > 0 else "DARK")
    print(f"{u:>12.4f} {mult:>5d} {decomp_str:>30} {s3_str:>22} {dark_tag:>10}")
    results.append((u, mult, decomp, triv_w, sign_w, std_w))

print("-"*100)
print(f"Totals: visible = {total_vis}, sign = {total_sign}, standard = {total_std}")
print(f"Check:  27 states = {total_vis} vis + {total_sign} sign + {total_std} std = {total_vis+total_sign+total_std}")

# ----------------------------------------------------------------------
# Dark mass prediction
# ----------------------------------------------------------------------
print("\n" + "="*100)
print("DARK MASS PREDICTION (FTD Wave 5.5 ad hoc)")
print("="*100)

# Ladder scale
m_P = 1.2209e19  # GeV
alpha = 1/137.036
n_dark = 17
mu_dark = m_P * np.sqrt(2 * np.pi) * alpha**n_dark
print(f"\nLadder scale mu(n=17) = m_P * sqrt(2*pi) * alpha^17")
print(f"                      = {mu_dark:.4e} GeV")
print(f"                      = {mu_dark*1e9:.4f} eV")
print(f"                      = {mu_dark*1e9*1e9:.3f} neV")

# For each distinct eigenvalue, report the dark lines
print(f"\n{'line':>5} {'lambda':>10} {'O_h':>8} {'sign':>5} {'std':>5} {'total dark deg':>16} {'mass (neV)':>14}")
print("-"*80)

line = 0
for u, mult, decomp, triv, sgn, std in results:
    dark_deg = sgn + std
    if dark_deg == 0:
        continue
    line += 1
    # Mass from ansatz: m = mu(17) * sqrt(lambda)
    mass_neV = mu_dark * 1e18 * np.sqrt(u)
    oh_label = " + ".join(k for k, v in decomp.items() if "u" in k or k in ("Eg", "T1g", "T2g"))
    print(f"{line:>5d} {u:>10.4f} {oh_label:>8} {sgn:>5d} {std:>5d} {dark_deg:>16d} {mass_neV:>14.3f}")

print("\n--- summary ---")
dark_masses = []
for u, mult, decomp, triv, sgn, std in results:
    if sgn + std > 0:
        dark_masses.append((u, sgn + std, mu_dark * 1e18 * np.sqrt(u)))

print(f"Number of distinct dark mass lines: {len(dark_masses)}")
print(f"Total dark state count:             {sum(d for _,d,_ in dark_masses)}")
print(f"Lightest dark mass:  {min(m for _,_,m in dark_masses):.3f} neV")
print(f"Heaviest dark mass:  {max(m for _,_,m in dark_masses):.3f} neV")
print(f"Mean dark mass:      {sum(d*m for _,d,m in dark_masses)/sum(d for _,d,_ in dark_masses):.3f} neV")

# The sign-rep line is at lambda = N_eff = 13
for u, mult, decomp, triv, sgn, std in results:
    if sgn == 1:
        m_sign = mu_dark * 1e18 * np.sqrt(u)
        print(f"\nSIGN REP (pseudoscalar singlet):")
        print(f"  eigenvalue = {u:.4f}  (= N_eff = 13? {abs(u-13)<0.001})")
        print(f"  O_h irrep  = {list(decomp.keys())}")
        print(f"  mass       = {m_sign:.3f} neV")
        print(f"  degenerate with {std}-dim standard doublet at same eigenvalue")

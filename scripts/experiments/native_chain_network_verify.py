"""native_chain_network_verify.py — lift a joint-network candidate to unit
stations and verify it as a native zero-tension stressed framework.

Generic over topologies: a candidate is (joints, chains) with integer chain
lengths matching the joint distances. The lift places stations at unit
spacing along every chain. Verification gates, all of which must pass:

  G1  polarity: the unit graph is 2-colorable (all chain cycles even);
  G2  geometry: bonded pairs (consecutive stations) at distance exactly 1;
      every other opposite-polarity pair >= CUTOFF (else it would interact
      off-minimum and the configuration is not a zero-tension equilibrium);
      every same-polarity pair >= FLOOR (capacity); no coincidences;
  G3  stress: coker(R) >= 1, the stress is uniform along each chain and
      satisfies joint equilibrium (the lifted contracted stress);
  G4  blocking: transverse bowing flexes of stressed chains have
      <omega, kappa(q)> != 0 and quartic coefficient E4(q) > 0
      (n = 4, first-order flexible + second-order blocked).

Verdict NATIVE_N4_CANDIDATE_VERIFIED or the first failed gate.
[VERIFICATION INSTRUMENT — registers nothing]
"""
from __future__ import annotations

import sys
import numpy as np

FLOOR_SAME = 0.40
CUTOFF = 1.30
TOL = 1e-9


def build(joints: dict, chains: list, scale: int = 1):
    """joints: name -> np.array(3); chains: list of (a, b, L) with L the
    integer chain length after scaling. Returns positions, edges, colors,
    chain_of_edge."""
    pos = []
    names = []
    color = {}
    idx = {}
    for nm, p in joints.items():
        idx[nm] = len(pos)
        pos.append(np.array(p, dtype=float) * scale)
        names.append(nm)
    edges = []
    chain_of_edge = []
    for ci, (a, b, L) in enumerate(chains):
        L = L * scale
        pa, pb = pos[idx[a]], pos[idx[b]]
        u = (pb - pa) / L
        prev = idx[a]
        for k in range(1, L):
            pos.append(pa + k * u)
            names.append(f"{a}{b}.{k}")
            edges.append((prev, len(pos) - 1))
            chain_of_edge.append(ci)
            prev = len(pos) - 1
        edges.append((prev, idx[b]))
        chain_of_edge.append(ci)
    P = np.array(pos)
    # 2-coloring by BFS over unit edges
    n = len(P)
    adj = [[] for _ in range(n)]
    for i, j in edges:
        adj[i].append(j)
        adj[j].append(i)
    col = np.full(n, -1, dtype=int)
    col[0] = 0
    stack = [0]
    ok = True
    while stack:
        v = stack.pop()
        for w in adj[v]:
            if col[w] == -1:
                col[w] = 1 - col[v]
                stack.append(w)
            elif col[w] == col[v]:
                ok = False
    return P, edges, col, ok, np.array(chain_of_edge), names, idx


def rigidity(P, edges):
    n = len(P)
    R = np.zeros((len(edges), 3 * n))
    for e, (i, j) in enumerate(edges):
        d = P[j] - P[i]
        u = d / np.linalg.norm(d)
        R[e, 3 * i:3 * i + 3] = -u
        R[e, 3 * j:3 * j + 3] = +u
    return R


def verify(joints, chains, scale, label):
    print("=" * 72)
    print(f"CANDIDATE: {label} (scale x{scale})")
    print("=" * 72)
    P, edges, col, twocol, chain_ids, names, idx = build(joints, chains, scale)
    n = len(P)
    print(f"bodies N = {n}, unit bonds = {len(edges)}, chains = {len(chains)}")

    # G1 polarity
    if not twocol:
        print("G1 FAIL: unit graph not 2-colorable (odd cycle)")
        return False
    print("G1 PASS: polarity 2-coloring consistent")

    # G2 geometry — exhaustive pair check
    bond_set = set(tuple(sorted(e)) for e in edges)
    d2 = np.sum((P[:, None, :] - P[None, :, :]) ** 2, axis=2)
    iu = np.triu_indices(n, 1)
    worst_bond = 0.0
    min_same = np.inf
    min_opp_nonbond = np.inf
    bad = []
    for i, j in zip(*iu):
        dij = np.sqrt(d2[i, j])
        if (int(i), int(j)) in bond_set or (int(j), int(i)) in bond_set:
            worst_bond = max(worst_bond, abs(dij - 1.0))
            continue
        if col[i] == col[j]:
            min_same = min(min_same, dij)
            if dij < FLOOR_SAME:
                bad.append(("same<floor", names[i], names[j], dij))
        else:
            min_opp_nonbond = min(min_opp_nonbond, dij)
            if dij < CUTOFF:
                bad.append(("opp<cutoff", names[i], names[j], dij))
    print(f"  bonded length error max     : {worst_bond:.3e}")
    print(f"  min same-polarity distance  : {min_same:.4f} (floor {FLOOR_SAME})")
    print(f"  min opp non-bonded distance : {min_opp_nonbond:.4f} "
          f"(cutoff {CUTOFF})")
    if worst_bond > 1e-8 or bad:
        for kind, ni, nj, dij in bad[:8]:
            print(f"    G2 violation [{kind}] {ni} -- {nj}  d = {dij:.4f}")
        print(f"G2 FAIL: {len(bad)} clearance violations")
        return False
    print("G2 PASS: zero-tension equilibrium of the single-scale law")

    # G3 stress
    R = rigidity(P, edges)
    U, S, Vt = np.linalg.svd(R, full_matrices=True)
    coker_dim = int(np.sum(S < 1e-8)) + (R.shape[0] - len(S)
                                         if R.shape[0] > R.shape[1] else 0)
    tail = S[-min(4, len(S)):]
    print(f"  smallest singular values of R: {tail}")
    if S.min() > 1e-8:
        print("G3 FAIL: no self-stress (coker(R) = 0)")
        return False
    om = U[:, np.argmin(S)]
    # uniformity along chains
    uni = []
    for ci in range(len(chains)):
        w = om[chain_ids == ci]
        uni.append((w.max() - w.min(), w.mean()))
    max_spread = max(u[0] for u in uni)
    print(f"  coker dim (numeric)          : {int(np.sum(S < 1e-8))}")
    print(f"  stress uniformity per chain  : max spread {max_spread:.2e}")
    print(f"  chain stresses (mean)        : "
          f"{[f'{u[1]:+.4f}' for u in uni]}")
    eq_res = np.linalg.norm(om @ R)
    print(f"  equilibrium residual |omega R|: {eq_res:.2e}")
    if max_spread > 1e-6:
        print("G3 FAIL: stress not chain-uniform (not the lifted stress)")
        return False
    print("G3 PASS: nonzero chain-uniform self-stress at zero tension")

    # G4 blocking of chain-bowing flexes
    print("  bowing-flex blocking per stressed chain:")
    k_metric = np.ones(len(edges))
    okall = True
    for ci, (a, b, L) in enumerate(chains):
        Ls = L * scale
        w_mean = uni[ci][1]
        # transverse sine bump on the chain interior
        pa, pb = P[idx[a]], P[idx[b]]
        u = (pb - pa) / np.linalg.norm(pb - pa)
        t = np.array([u[1] - u[2], u[2] - u[0], u[0] - u[1]])
        if np.linalg.norm(t) < 1e-9:
            t = np.array([0.0, 1.0, 0.0])
        t = t - (t @ u) * u
        t /= np.linalg.norm(t)
        q = np.zeros((n, 3))
        # find this chain's interior station indices in order
        sts = [i for i, nm in enumerate(names)
               if nm.startswith(f"{a}{b}.")]
        for k, i in enumerate(sts, 1):
            q[i] = np.sin(np.pi * k / Ls) * t
        # first-order flex check
        first = np.linalg.norm(R @ q.ravel())
        # kappa and blocking
        kap = np.zeros(len(edges))
        for e, (i, j) in enumerate(edges):
            dq = q[i] - q[j]
            ell = np.linalg.norm(P[i] - P[j])
            kap[e] = 0.5 * (dq @ dq) / ell
        block = om @ kap
        # E4 = 0.5 * || P_coker kappa ||^2 in the k-metric (k = 1)
        stress_basis = U[:, S < 1e-8] if np.sum(S < 1e-8) else om[:, None]
        proj = stress_basis @ (stress_basis.T @ kap)
        E4 = 0.5 * float(proj @ proj)
        flag = "BLOCKED (n=4)" if abs(block) > 1e-8 else "extends"
        if abs(w_mean) > 1e-6 and abs(block) < 1e-8:
            okall = False
        print(f"    chain {a}-{b} (L={Ls}, w={w_mean:+.4f}): "
              f"|R q| = {first:.1e}, <omega,kappa> = {block:+.4e}, "
              f"E4 = {E4:.4e}  {flag}")
    if not okall:
        print("G4 FAIL: a stressed chain's bowing flex is unblocked")
        return False
    print("G4 PASS: stressed-chain bowing flexes are individually blocked")

    # G5 [ADDED 2026-08-07 after the wheel refutation] — the criterion the
    # per-chain test G4 misses: (a) FULL flex-space blocking — E4 vanishes
    # on any flex combination with <omega, kappa(q)> = 0, and such
    # combinations are zero-energy MECHANISMS unless none exist; test the
    # blocking form's definiteness on the whole nontrivial flex space.
    # (b) COMPRESSION-BUCKLING — a compressed chain of length >= 2 is a
    # free-hinge linkage and buckles at zero cost (the escape found
    # empirically by native_wheel_clock_sim.py: E ~ (sum kappa_tension -
    # sum kappa_compression)^2 has the mechanism cone delta = u). Every
    # compressed member must be a SINGLE bond.
    # flex basis: full SVD null space minus trivial motions
    U2, S2, V2t = np.linalg.svd(R, full_matrices=True)
    null_dim = 3 * n - int(np.sum(S2 > 1e-8))
    flexes = V2t[int(np.sum(S2 > 1e-8)):, :].T          # (3n, null_dim)
    # remove trivial motions (3 translations + 3 rotations)
    triv = []
    for a in range(3):
        v = np.zeros((n, 3))
        v[:, a] = 1.0
        triv.append(v.ravel())
    for a in range(3):
        v = np.zeros((n, 3))
        ax = np.zeros(3)
        ax[a] = 1.0
        for i in range(n):
            v[i] = np.cross(ax, P[i] - P.mean(axis=0))
        triv.append(v.ravel())
    T = np.array(triv).T
    Q_, _ = np.linalg.qr(np.hstack([T, flexes]))
    nontriv = Q_[:, T.shape[1]:T.shape[1] + (null_dim - 6)]
    # blocking quadratic form omega(q,q) on the nontrivial flex space
    def om_form(qv):
        q = qv.reshape(n, 3)
        s = 0.0
        for e, (i, j) in enumerate(edges):
            ell = np.linalg.norm(P[i] - P[j])
            dq = q[i] - q[j]
            s += om[e] * (dq @ dq) / ell
        return s
    dim = nontriv.shape[1]
    M = np.zeros((dim, dim))
    for aI in range(dim):
        for bI in range(aI, dim):
            v = om_form((nontriv[:, aI] + nontriv[:, bI]))
            va = om_form(nontriv[:, aI])
            vb = om_form(nontriv[:, bI])
            M[aI, bI] = M[bI, aI] = 0.5 * (v - va - vb)
    ev = np.linalg.eigvalsh(M)
    n_zero = int(np.sum(np.abs(ev) < 1e-8))
    sign_mixed = ev.min() < -1e-8 and ev.max() > 1e-8
    print(f"  G5a full-flex blocking form: dim {dim}, eigenvalue range "
          f"[{ev.min():+.4f}, {ev.max():+.4f}], zeros {n_zero}")
    comp_long = [ci for ci in range(len(chains))
                 if uni[ci][1] * np.sign(uni[np.argmax([abs(u[1]) for u in uni])][1]) < 0
                 and chains[ci][2] * scale >= 2]
    # sign convention: identify tension = majority sign of rim-like members;
    # simpler: a compressed CHAIN is one whose stress sign differs from a
    # single-bond-only stress being irrelevant — report both signs:
    signs = [np.sign(u[1]) for u in uni]
    long_chains = [ci for ci in range(len(chains)) if chains[ci][2] * scale >= 2]
    both_signs_on_long = (any(signs[ci] > 0 for ci in long_chains) and
                          any(signs[ci] < 0 for ci in long_chains))
    print(f"  G5b compressed multi-bond chains: "
          f"{'PRESENT (free-hinge buckling escape)' if both_signs_on_long or (long_chains and n_zero + int(sign_mixed) > 0) else 'none'}")
    if sign_mixed or n_zero > 0:
        print("G5 FAIL: the blocking form is not definite on the flex space —")
        print("  zero/sign-mixed directions are mechanism escapes (the wheel")
        print("  failure mode: E ~ (difference of kappa sums)^2).")
        print()
        print("VERDICT: STATIC_STRESSED_EQUILIBRIUM_ONLY — a genuine native")
        print("zero-tension self-stressed equilibrium (G1-G3), but NOT a")
        print("second-order-rigid n = 4 clock: coupled flexes escape at zero")
        print("energy. C3 remains unrealized by this object.")
        return False
    print("G5 PASS: blocking form definite on the whole flex space")
    print()
    print("VERDICT: NATIVE_N4_CANDIDATE_VERIFIED — a zero-tension,")
    print("single-scale, bipartite-legal framework with self-stress and")
    print("quartic (n = 4) modes blocked on the FULL flex space.")
    return True


def hexagon_wheel(s: int = 2):
    """The regular hexagon wheel at integer side s: hub at the origin,
    six corners at radius s, six rim chains (length s) + six spokes
    (length s). Exact 360-degree hub closure via equilateral triangles;
    every joint's pairwise chain angle >= 60 deg; parity-native for even s
    (all chain triangles sum 3s).

    Registered-law clearances (q = squared separation, support q < 3/2):
    nearest opposite-polarity non-bonded pair sits at q = 3 (verified
    exhaustively below) — twice the support edge."""
    from math import cos, sin, pi
    joints = {"H": np.array([0.0, 0.0, 0.0])}
    for k in range(6):
        joints[f"J{k}"] = np.array([s * cos(k * pi / 3),
                                    s * sin(k * pi / 3), 0.0])
    chains = []
    for k in range(6):
        chains.append(("H", f"J{k}", s))                       # spokes
        chains.append((f"J{k}", f"J{(k + 1) % 6}", s))         # rim
    return joints, chains


if __name__ == "__main__":
    joints, chains = hexagon_wheel(s=2)
    ok = verify(joints, chains, scale=1,
                label="regular hexagon wheel, s = 2 (native single-scale)")
    sys.exit(0 if ok else 1)

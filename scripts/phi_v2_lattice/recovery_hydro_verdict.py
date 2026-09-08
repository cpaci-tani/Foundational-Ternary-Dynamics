# scripts/phi_v2_lattice/recovery_hydro_verdict.py
"""Pre-registered verdict clauses for gate H1 (spec section 3.2, fixed before any result).

Operators act on moment functionals from the right: a row a (a functional on the
7-space) evolves as a -> a (I + eps M1 + eps^2 M2). The density functional is the
constant weight expressed in the W basis. The closure block is the smallest row
space containing it and invariant under every listed operator.
"""
from __future__ import annotations

from fractions import Fraction
import math

import flint
import numpy as np

from . import recovery_hydro_dispersion as D

N = 192
VERDICT_LABELS = ("NS-class isotropic", "anisotropic momentum hydrodynamics", "diffusive only", "other")


def _fraction(q) -> Fraction:
    return Fraction(int(q.p), int(q.q))


def _text(q) -> str:
    return str(_fraction(q))


def density_functional(W):
    """Row a with a W = (1,...,1); raises if the constant weight is outside the row space."""
    ones = D.Exact.from_rows([[1] * W.ncols()])
    gram = W * W.transpose()
    a = (ones * W.transpose()) * gram.inv()
    if a * W != ones:
        raise ValueError("constant weight is not in the span of W")
    return a


def _rank(rows_mat) -> int:
    return rows_mat.rank()


def _stack(top, extra_row):
    rows = [[top[i, j] for j in range(top.ncols())] for i in range(top.nrows())]
    rows.append([extra_row[0, j] for j in range(extra_row.ncols())])
    return flint.fmpq_mat(rows)


def closure_block(start, operators):
    """Smallest row space containing `start` and closed under right-multiplication."""
    block = start
    changed = True
    while changed:
        changed = False
        for M in operators:
            for i in range(block.nrows()):
                row = flint.fmpq_mat([[block[i, j] for j in range(block.ncols())]]) * M
                candidate = _stack(block, row)
                if candidate.rank() > block.rank():
                    block = candidate
                    changed = True
    return block


def restrict(block, M):
    """X with block M = X block; raises if the block is not invariant."""
    Bt = block.transpose()
    X = (block * M * Bt) * (block * Bt).inv()
    if X * block != block * M:
        raise ValueError("block is not invariant under the operator")
    return X


def _charpoly_coeffs(X):
    return [c for c in X.charpoly().coeffs()]  # low to high


def _direction_norm2(direction) -> int:
    return sum(int(x) * int(x) for x in direction)


def _sound_structure(X):
    """(s, ok): charpoly x^4 - s x^2 with s > 0 and X^3 = s X."""
    coeffs = _charpoly_coeffs(X)
    if len(coeffs) != 5:
        return None, False
    c0, c1, c2, c3, c4 = coeffs
    s = -c2
    ok = (c0 == 0 and c1 == 0 and c3 == 0 and c4 == 1 and s > 0)
    if ok:
        ok = (X * X * X == X * s)
    return s, ok


def _transverse_polynomial(X, Y, s):
    I4 = D.Exact.identity(4)
    PL = (X * X) * flint.fmpq(int(s.q), int(s.p))   # X^2 / s, exact
    PT = I4 - PL
    T = PT * Y * PT
    coeffs = _charpoly_coeffs(T)  # degree 4 with two zero roots from PL's kernel
    # divide by x^2 exactly: coefficients of x^0 and x^1 must vanish
    if coeffs[0] != 0 or coeffs[1] != 0:
        raise ValueError("transverse operator does not annihilate the longitudinal pair")
    return coeffs[2], coeffs[3], coeffs[4]  # c0', c1', leading (=1)


def _longitudinal_damping(X, Y, s):
    PL = (X * X) * flint.fmpq(int(s.q), int(s.p))
    trace = sum((PL * Y * PL)[i, i] for i in range(4))
    return trace / 2 - s / 2


def exact_verdict(dispersions: dict) -> dict:
    """Apply the fixed clauses to exact 7x7 operators keyed by direction."""
    directions = tuple(dispersions)
    W = dispersions[directions[0]].W
    a = density_functional(W)
    operators = [m for n in directions for m in (dispersions[n].M1, dispersions[n].M2)]
    block = closure_block(a, operators)
    dim = block.nrows()
    m1_only_block = closure_block(a, [dispersions[n].M1 for n in directions])
    out = {"block_dimension": dim, "block_rows": [[_text(block[i, j]) for j in range(block.ncols())] for i in range(dim)],
           "block_dimension_first_order_only": m1_only_block.nrows(),
           "clauses": {"block_dimension_is_four": dim == 4},
           "charpoly_M1": {str(n): [_text(c) for c in _charpoly_coeffs(dispersions[n].M1)] for n in directions},
           "charpoly_M2": {str(n): [_text(c) for c in _charpoly_coeffs(dispersions[n].M2)] for n in directions}}
    all_first_order_zero = all(dispersions[n].M1 == D.Exact.zeros(W.nrows(), W.nrows()) for n in directions)
    if all_first_order_zero:
        out["label"] = "diffusive only"
        out["density_diffusion_charpoly_on_block"] = {
            str(n): [_text(c) for c in _charpoly_coeffs(restrict(block, dispersions[n].M2))] for n in directions}
        return out
    if dim != 4:
        out["label"] = "other"
        out["charpoly_M1_on_block"] = {
            str(n): [_text(c) for c in _charpoly_coeffs(restrict(block, dispersions[n].M1))] for n in directions}
        return out
    s_norm, sound_ok, trans, longi = {}, True, {}, {}
    for n in directions:
        X = restrict(block, dispersions[n].M1)
        Y = restrict(block, dispersions[n].M2)
        s, ok = _sound_structure(X)
        sound_ok = sound_ok and ok
        if not ok:
            continue
        n2 = _direction_norm2(n)
        s_norm[str(n)] = _text(s * flint.fmpq(1, n2))
        c0, c1, _ = _transverse_polynomial(X, Y, s)
        trans[str(n)] = [_text(c1 * flint.fmpq(1, n2)), _text(c0 * flint.fmpq(1, n2 * n2))]
        longi[str(n)] = _text(_longitudinal_damping(X, Y, s) * flint.fmpq(1, n2))
    out["clauses"]["sound_pair_all_directions"] = sound_ok
    out["sound_speed_squared_normalized"] = s_norm
    out["transverse_polynomial_normalized"] = trans
    out["longitudinal_damping_normalized"] = longi
    same_speed = sound_ok and len(set(s_norm.values())) == 1
    same_trans = sound_ok and len({tuple(v) for v in trans.values()}) == 1
    same_long = sound_ok and len(set(longi.values())) == 1
    out["clauses"].update({"sound_speed_direction_independent": same_speed,
                           "transverse_isotropic": same_trans,
                           "longitudinal_isotropic": same_long,
                           "clause_2_holds": sound_ok and same_speed})
    if sound_ok and same_speed and same_trans and same_long:
        out["label"] = "NS-class isotropic"
    elif sound_ok and same_speed:
        out["label"] = "anisotropic momentum hydrodynamics"
    else:
        out["label"] = "other"
    return out


def _to_complex_array(M, n=7):
    return np.array([[complex(float(int(M[i, j].p)) / float(int(M[i, j].q)), 0.0) for j in range(n)]
                     for i in range(n)])


def _cluster_indices(values, tol=1e-9):
    """Union-find clustering of `values` (a sequence of complex numbers) within `tol`."""
    parent = list(range(len(values)))

    def find(x):
        while parent[x] != x:
            parent[x] = parent[parent[x]]
            x = parent[x]
        return x

    for i in range(len(values)):
        for j in range(i + 1, len(values)):
            if abs(values[i] - values[j]) < tol:
                ri, rj = find(i), find(j)
                if ri != rj:
                    parent[ri] = rj
    groups: dict = {}
    for i in range(len(values)):
        groups.setdefault(find(i), []).append(i)
    return list(groups.values())


def mode_table_float64(M1, M2, direction) -> list[dict]:
    """Float64 degenerate-perturbation mode report for one direction (booking only).

    NOT a certified or exact result -- a double-precision report for booking
    purposes, since the label "other" (block dimension != 4) leaves no
    4-dimensional NS-class block for exact per-mode speeds/damping/diffusion.
    M1, M2 are the exact 7x7 flint.fmpq_mat operators, cast to numpy
    complex128 via each entry's `.p`/`.q`.

    Eigendecomposes M1 directly (numpy `eig`) for its "right" eigenpairs, and
    M1.T (numpy `eig`; identical to M1's conjugate-transpose here since M1 is
    real) for the Hermitian left-eigenvector candidates, matched to each
    right-eigenvalue cluster by its complex conjugate. Eigenvalues of M1 are
    grouped into clusters (tolerance 1e-9); within each cluster the candidate
    left/right eigenvector bases are biorthonormalized (`W_c @ V_c = I`) and
    used to project M2 into a small matrix `W_c @ M2 @ V_c`, whose eigenvalues
    are the cluster's `mu2` values (standard degenerate perturbation theory).

    Returns one dict per (cluster, mu2-eigenvalue) pair with keys: `mu1` (the
    cluster's mu1, complex as [re, im]), `mu1_normalized` (= mu1 / |n|),
    `mu2` (complex as [re, im]), `damping` (= Re(mu2) - Re(mu1)^2 / 2),
    `damping_normalized` (= damping / |n|^2), and `multiplicity` (the
    cluster's size).
    """
    dim = 7
    M1f = _to_complex_array(M1, dim)
    M2f = _to_complex_array(M2, dim)
    norm = math.sqrt(sum(int(x) * int(x) for x in direction))
    norm2 = norm * norm

    vals1, R = np.linalg.eig(M1f)
    vals1t, Lraw = np.linalg.eig(M1f.T)
    clusters = _cluster_indices(list(vals1))

    table = []
    for cluster in clusters:
        mu1_cluster = complex(np.mean(vals1[cluster]))
        target = np.conj(mu1_cluster)
        left_idx = [k for k in range(dim) if abs(vals1t[k] - target) < 1e-9]
        if len(left_idx) < len(cluster):
            left_idx = sorted(range(dim), key=lambda k: abs(vals1t[k] - target))[:len(cluster)]
        Vc = R[:, cluster]
        Lc = Lraw[:, left_idx]
        Gc = Lc.conj().T @ Vc
        Wc = np.linalg.solve(Gc, Lc.conj().T)
        M2c = Wc @ M2f @ Vc
        mu2_vals = np.linalg.eigvals(M2c)
        multiplicity = len(cluster)
        for mu2 in mu2_vals:
            mu2_re = float(mu2.real)
            damping = mu2_re - (mu1_cluster.real ** 2) / 2.0
            table.append({
                "mu1": [float(mu1_cluster.real), float(mu1_cluster.imag)],
                "mu1_normalized": [float(mu1_cluster.real) / norm, float(mu1_cluster.imag) / norm],
                "mu2": [mu2_re, float(mu2.imag)],
                "damping": damping,
                "damping_normalized": damping / norm2,
                "multiplicity": multiplicity,
            })
    return table


def _status(delta_ball) -> str:
    """Status of an EQUALITY clause from the enclosure of a difference.

    REFUTED: the enclosure excludes zero, so the inequality is rigorously proved
    (this is how anisotropy is PROVED). CONSISTENT: the enclosure contains zero
    and is narrow. UNDECIDED: the enclosure contains zero but is too wide.
    """
    if abs(delta_ball.mid()) > delta_ball.rad():
        return "REFUTED"
    return "CONSISTENT" if delta_ball.rad() < flint.arb("1e-30") else "UNDECIDED"


def certified_verdict(dispersions: dict, block, prec: int = 256) -> dict:
    """Ball-arithmetic evaluation of the clauses on the exact block basis.

    Well-defined for any block dimension b = block.nrows(): the block-invariance
    residual and the certified characteristic polynomials (full 7x7 and the b x b
    block restriction) are always computed. The three sound-pair/isotropy clauses
    and their `values` are only meaningful for the pre-registered b == 4 case; for
    any other b they report the string "NOT APPLICABLE (block dimension {b} != 4)"
    and an empty `values` dict, rather than raising.

    This function's own matrix arithmetic (products, inverse, charpoly) must not
    run at the caller's ambient `flint.ctx.prec` (module default 53 bits, i.e.
    double precision, since `certified_dispersion` restores the caller's saved
    precision before returning its input operators) -- doing so silently
    reintroduces double-precision-scale radii on top of tight input balls. `prec`
    (default 256, matching `certified_dispersion`'s default) is applied for the
    duration of this call and the caller's ambient precision is restored in
    `finally`, the same pattern as `certified_dispersion`.
    """
    saved = flint.ctx.prec
    flint.ctx.prec = prec
    try:
        directions = tuple(dispersions)
        Bq = block
        b = Bq.nrows()
        B = flint.arb_mat([[D.Certified.scalar(_fraction(Bq[i, j])) for j in range(Bq.ncols())] for i in range(b)])
        Gi = flint.arb_mat([[D.Certified.scalar(_fraction((Bq * Bq.transpose()).inv()[i, j])) for j in range(b)]
                            for i in range(b)])
        report = {"block_dimension": b}
        s_values, trans_values, long_values, invariance = {}, {}, {}, {}
        charpoly_m1_full, charpoly_m2_full, charpoly_m1_block = {}, {}, {}
        max_radius = None
        for n in directions:
            M1, M2 = dispersions[n].M1, dispersions[n].M2
            X = (B * M1 * B.transpose()) * Gi
            Y = (B * M2 * B.transpose()) * Gi
            res = X * B - B * M1
            best = None
            for i in range(res.nrows()):
                for j in range(res.ncols()):
                    bound = abs(res[i, j].mid()) + res[i, j].rad()
                    if best is None or bound > best:
                        best = bound
            invariance[str(n)] = str(best)
            c1_coeffs = M1.charpoly().coeffs()
            c2_coeffs = M2.charpoly().coeffs()
            charpoly_m1_full[str(n)] = [str(c) for c in c1_coeffs]
            charpoly_m2_full[str(n)] = [str(c) for c in c2_coeffs]
            for c in c1_coeffs + c2_coeffs:
                r = c.rad()
                if max_radius is None or r > max_radius:
                    max_radius = r
            charpoly_m1_block[str(n)] = [str(c) for c in X.charpoly().coeffs()]
            if b != 4:
                continue
            coeffs = X.charpoly().coeffs()
            s = -coeffs[2]
            n2 = _direction_norm2(n)
            s_values[n] = s / n2
            I4 = flint.arb_mat(4, 4)
            for i in range(4):
                I4[i, i] = 1
            PL = (X * X) * (flint.arb(1) / s)
            PT = I4 - PL
            tc = (PT * Y * PT).charpoly().coeffs()
            trans_values[n] = (tc[3] / n2, tc[2] / (n2 * n2))
            trace = sum((PL * Y * PL)[i, i] for i in range(4))
            long_values[n] = (trace / 2 - s / 2) / n2
        report["block_invariance_max_residual"] = invariance
        report["charpoly_M1_certified"] = charpoly_m1_full
        report["charpoly_M2_certified"] = charpoly_m2_full
        report["charpoly_M1_block_certified"] = charpoly_m1_block
        report["max_charpoly_radius"] = str(max_radius)
        if b != 4:
            na = f"NOT APPLICABLE (block dimension {b} != 4)"
            report["sound_speed_direction_independent"] = na
            report["transverse_isotropic"] = na
            report["longitudinal_isotropic"] = na
            report["values"] = {}
            return report
        first = directions[0]
        report["sound_speed_direction_independent"] = _worst(
            [_status(s_values[n] - s_values[first]) for n in directions[1:]])
        report["transverse_isotropic"] = _worst(
            [_status(trans_values[n][k] - trans_values[first][k]) for n in directions[1:] for k in range(2)])
        report["longitudinal_isotropic"] = _worst(
            [_status(long_values[n] - long_values[first]) for n in directions[1:]])
        report["values"] = {str(n): {"sound_speed_squared_normalized": str(s_values[n]),
                                     "transverse_c1_normalized": str(trans_values[n][0]),
                                     "transverse_c0_normalized": str(trans_values[n][1]),
                                     "longitudinal_damping_normalized": str(long_values[n])}
                            for n in directions}
        return report
    finally:
        flint.ctx.prec = saved


def _worst(statuses) -> str:
    if "REFUTED" in statuses:
        return "REFUTED"
    if "UNDECIDED" in statuses:
        return "UNDECIDED"
    return "CONSISTENT"

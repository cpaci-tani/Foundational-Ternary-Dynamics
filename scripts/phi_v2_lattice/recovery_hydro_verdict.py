# scripts/phi_v2_lattice/recovery_hydro_verdict.py
"""Pre-registered verdict clauses for gate H1 (spec section 3.2, fixed before any result).

Operators act on moment functionals from the right: a row a (a functional on the
7-space) evolves as a -> a (I + eps M1 + eps^2 M2). The density functional is the
constant weight expressed in the W basis. The closure block is the smallest row
space containing it and invariant under every listed operator.
"""
from __future__ import annotations

from fractions import Fraction

import flint

from . import recovery_hydro_dispersion as D

N = 192
VERDICT_LABELS = ("NS-class isotropic", "anisotropic momentum hydrodynamics", "diffusive only", "other")


def _fraction(q) -> Fraction:
    return Fraction(int(q.p), int(q.q))


def _text(q) -> str:
    return str(_fraction(q))


def density_functional(W):
    """Row a with a W = (1,...,1); raises if the constant weight is outside the row space."""
    ones = D.Exact.from_rows([[1] * N])
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
    out = {"block_dimension": dim, "block_rows": [[_text(block[i, j]) for j in range(7)] for i in range(dim)],
           "clauses": {"block_dimension_is_four": dim == 4},
           "charpoly_M1": {str(n): [_text(c) for c in _charpoly_coeffs(dispersions[n].M1)] for n in directions},
           "charpoly_M2": {str(n): [_text(c) for c in _charpoly_coeffs(dispersions[n].M2)] for n in directions}}
    all_first_order_zero = all(dispersions[n].M1 == D.Exact.zeros(7, 7) for n in directions)
    if all_first_order_zero:
        out["label"] = "diffusive only"
        out["density_diffusion_charpoly_on_block"] = {
            str(n): [_text(c) for c in _charpoly_coeffs(restrict(block, dispersions[n].M2))] for n in directions}
        return out
    if dim != 4:
        out["label"] = "other"
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


def _status(delta_ball) -> str:
    """Status of an EQUALITY clause from the enclosure of a difference.

    REFUTED: the enclosure excludes zero, so the inequality is rigorously proved
    (this is how anisotropy is PROVED). CONSISTENT: the enclosure contains zero
    and is narrow. UNDECIDED: the enclosure contains zero but is too wide.
    """
    if abs(delta_ball.mid()) > delta_ball.rad():
        return "REFUTED"
    return "CONSISTENT" if delta_ball.rad() < flint.arb("1e-30") else "UNDECIDED"


def certified_verdict(dispersions: dict, block) -> dict:
    """Ball-arithmetic evaluation of the clauses on the exact block basis."""
    directions = tuple(dispersions)
    Bq = block
    B = flint.arb_mat([[D.Certified.scalar(_fraction(Bq[i, j])) for j in range(7)] for i in range(Bq.nrows())])
    Gi = flint.arb_mat([[D.Certified.scalar(_fraction((Bq * Bq.transpose()).inv()[i, j])) for j in range(Bq.nrows())]
                        for i in range(Bq.nrows())])
    report = {}
    s_values, trans_values, long_values, invariance = {}, {}, {}, {}
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
    first = directions[0]
    report["block_invariance_max_residual"] = invariance
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


def _worst(statuses) -> str:
    if "REFUTED" in statuses:
        return "REFUTED"
    if "UNDECIDED" in statuses:
        return "UNDECIDED"
    return "CONSISTENT"

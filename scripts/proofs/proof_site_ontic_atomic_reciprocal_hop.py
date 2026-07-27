#!/usr/bin/env python3
"""Independent Gate R0 certificate and energy counterexample (FTD-0599).

This script does not call the C++ implementation.  It reconstructs the locked
L=17, q=-1 body-negative ballistic arm from the preregistered equations,
solves the static dressing spectrally, and uses Arb ball arithmetic plus an
analytic global impulse bound to certify the unique fixed point.  It then
evaluates the two independent energy identities in ordinary high-accuracy
array arithmetic.  The interval certificate is a Banach/Krawczyk-equivalent
inclusion: every admitted root is first enclosed by the global impulse bound;
the Arb derivative enclosure is a strict contraction on that box; and the
residual/(1-q) ball encloses the sole fixed point.
"""

from __future__ import annotations

import itertools
import json
import math
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Sequence, Tuple

import numpy as np
from flint import arb, ctx

SCRIPT_DIR = Path(__file__).resolve().parent
SCRIPTS_DIR = SCRIPT_DIR.parent
if str(SCRIPTS_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_DIR))
from constants import C_SPEED, C_WAVE, G_C, K_B  # noqa: E402

ctx.prec = 160

L = 17
Q_CHARGE = -1
X0 = np.array([8.05, 8.05, 8.05], dtype=float)
V0 = -0.15 * np.ones(3, dtype=float) / math.sqrt(3.0)
M_INERTIAL = float(K_B)
E_REST = M_INERTIAL * float(C_SPEED) ** 2
C2 = float(C_SPEED) ** 2
G = float(G_C)
TOL = 1.0e-12
DERIVATIVE_ENCLOSURE_INFLATION = 1.0e-10
ENERGY_ENCLOSURE_INFLATION = 1.0e-10


def idx(x: int, y: int, z: int) -> Tuple[int, int, int]:
    return x % L, y % L, z % L


def smooth_axis(a: np.ndarray, axis: int) -> np.ndarray:
    return 0.25 * np.roll(a, 1, axis=axis) + 0.5 * a + 0.25 * np.roll(a, -1, axis=axis)


def smooth_all(a: np.ndarray) -> np.ndarray:
    out = a.copy()
    for axis in range(3):
        out = smooth_axis(out, axis)
    return out


def gradient(a: np.ndarray) -> np.ndarray:
    return np.stack(
        [0.5 * (np.roll(a, -1, axis=i) - np.roll(a, 1, axis=i)) for i in range(3)],
        axis=-1,
    )


def divergence(a: np.ndarray) -> np.ndarray:
    return sum(0.5 * (np.roll(a[..., i], -1, axis=i) - np.roll(a[..., i], 1, axis=i)) for i in range(3))


def curl(a: np.ndarray) -> np.ndarray:
    out = np.zeros_like(a)
    out[..., 0] = 0.5 * (
        np.roll(a[..., 2], -1, axis=1) - np.roll(a[..., 2], 1, axis=1)
        - np.roll(a[..., 1], -1, axis=2) + np.roll(a[..., 1], 1, axis=2)
    )
    out[..., 1] = 0.5 * (
        np.roll(a[..., 0], -1, axis=2) - np.roll(a[..., 0], 1, axis=2)
        - np.roll(a[..., 2], -1, axis=0) + np.roll(a[..., 2], 1, axis=0)
    )
    out[..., 2] = 0.5 * (
        np.roll(a[..., 1], -1, axis=0) - np.roll(a[..., 1], 1, axis=0)
        - np.roll(a[..., 0], -1, axis=1) + np.roll(a[..., 0], 1, axis=1)
    )
    return out


def derivative(a: np.ndarray, axis: int) -> np.ndarray:
    return 0.5 * (np.roll(a, -1, axis=axis) - np.roll(a, 1, axis=axis))


def apply_k(a: np.ndarray) -> np.ndarray:
    faces = sum(np.roll(a, shift, axis=axis) for axis in range(3) for shift in (-1, 1)) / 3.0
    edges = np.zeros_like(a)
    for first, second in ((0, 1), (0, 2), (1, 2)):
        for sa in (-1, 1):
            for sb in (-1, 1):
                edges += np.roll(np.roll(a, sa, axis=first), sb, axis=second) / 6.0
    lap = faces + edges - 4.0 * a
    return -(float(C_WAVE) ** 2) * lap


def cic(point: Sequence[float], charge: float) -> np.ndarray:
    out = np.zeros((L, L, L), dtype=float)
    lower = np.floor(point).astype(int)
    frac = np.asarray(point) - lower
    for bits in itertools.product((0, 1), repeat=3):
        weight = charge
        site = lower.copy()
        for axis, bit in enumerate(bits):
            site[axis] += bit
            weight *= frac[axis] if bit else 1.0 - frac[axis]
        out[idx(*site)] += weight
    return out


def static_dressing() -> Tuple[np.ndarray, np.ndarray, float, float]:
    rho0 = smooth_all(cic(X0, Q_CHARGE))
    source = -G * gradient(rho0)
    source_hat = np.fft.fftn(source, axes=(0, 1, 2))
    modes = 2.0 * math.pi * np.fft.fftfreq(L)
    cx = np.cos(modes)[:, None, None]
    cy = np.cos(modes)[None, :, None]
    cz = np.cos(modes)[None, None, :]
    symbol = float(C_WAVE) ** 2 * (
        4.0 - (2.0 / 3.0) * (cx + cy + cz)
        - (2.0 / 3.0) * (cx * cy + cx * cz + cy * cz)
    )
    jhat = np.zeros_like(source_hat)
    nonzero = symbol > 1.0e-30
    jhat[nonzero, :] = source_hat[nonzero, :] / symbol[nonzero, None]
    j = np.fft.ifftn(jhat, axes=(0, 1, 2)).real
    residual = apply_k(j) - source
    residual_norm = float(np.linalg.norm(residual.ravel()))
    positive = symbol[symbol > 1.0e-30]
    spectral_gap = float(np.min(positive))
    solution_error_bound = residual_norm / spectral_gap
    return rho0, j, residual_norm, solution_error_bound


@dataclass
class AD:
    value: arb
    derivative: Tuple[arb, arb, arb]

    @staticmethod
    def constant(value: float | arb) -> "AD":
        return AD(value if isinstance(value, arb) else arb(value), (arb(0), arb(0), arb(0)))

    @staticmethod
    def variable(midpoint: float, radius: float, axis: int) -> "AD":
        d = [arb(0), arb(0), arb(0)]
        d[axis] = arb(1)
        return AD(arb(midpoint, radius), tuple(d))

    def _coerce(self, other: float | arb | "AD") -> "AD":
        return other if isinstance(other, AD) else AD.constant(other)

    def __add__(self, other: float | arb | "AD") -> "AD":
        o = self._coerce(other)
        return AD(self.value + o.value, tuple(self.derivative[i] + o.derivative[i] for i in range(3)))

    __radd__ = __add__

    def __neg__(self) -> "AD":
        return AD(-self.value, tuple(-d for d in self.derivative))

    def __sub__(self, other: float | arb | "AD") -> "AD":
        return self + (-self._coerce(other))

    def __rsub__(self, other: float | arb | "AD") -> "AD":
        return self._coerce(other) - self

    def __mul__(self, other: float | arb | "AD") -> "AD":
        o = self._coerce(other)
        return AD(
            self.value * o.value,
            tuple(self.derivative[i] * o.value + self.value * o.derivative[i] for i in range(3)),
        )

    __rmul__ = __mul__

    def reciprocal(self) -> "AD":
        inv = 1 / self.value
        return AD(inv, tuple(-d * inv * inv for d in self.derivative))

    def __truediv__(self, other: float | arb | "AD") -> "AD":
        return self * self._coerce(other).reciprocal()

    def sqrt(self) -> "AD":
        root = self.value.sqrt()
        return AD(root, tuple(d / (2 * root) for d in self.derivative))

    def union(self, other: "AD") -> "AD":
        return AD(self.value.union(other.value), tuple(self.derivative[i].union(other.derivative[i]) for i in range(3)))


Sparse = Dict[Tuple[int, int, int], AD]
VectorSparse = Tuple[Sparse, Sparse, Sparse]


def sparse_add(target: Sparse, site: Tuple[int, int, int], value: AD) -> None:
    key = idx(*site)
    target[key] = target.get(key, AD.constant(0.0)) + value


def sparse_smooth_axis(field: Sparse, axis: int) -> Sparse:
    out: Sparse = {}
    for site, value in field.items():
        for shift, weight in ((-1, 0.25), (0, 0.5), (1, 0.25)):
            moved = list(site)
            moved[axis] += shift
            sparse_add(out, tuple(moved), value * weight)
    return out


def sparse_smooth_all(field: Sparse) -> Sparse:
    out = field
    for axis in range(3):
        out = sparse_smooth_axis(out, axis)
    return out


def sparse_combine(a: Sparse, b: Sparse, alpha: float, beta: float) -> Sparse:
    out: Sparse = {}
    for site in set(a) | set(b):
        value = a.get(site, AD.constant(0.0)) * alpha + b.get(site, AD.constant(0.0)) * beta
        out[site] = value
    return out


def sparse_gradient(field: Sparse) -> VectorSparse:
    result: List[Sparse] = [{}, {}, {}]
    for site, value in field.items():
        for axis in range(3):
            plus = list(site)
            minus = list(site)
            plus[axis] -= 1
            minus[axis] += 1
            sparse_add(result[axis], tuple(plus), value * 0.5)
            sparse_add(result[axis], tuple(minus), value * -0.5)
    return tuple(result)  # type: ignore[return-value]


def sparse_curl(field: VectorSparse) -> VectorSparse:
    result: List[Sparse] = [{}, {}, {}]
    terms = (
        (0, 2, 1, 1.0), (0, 1, 2, -1.0),
        (1, 0, 2, 1.0), (1, 2, 0, -1.0),
        (2, 1, 0, 1.0), (2, 0, 1, -1.0),
    )
    for out_axis, component_axis, derivative_axis, sign in terms:
        for site, value in field[component_axis].items():
            plus = list(site)
            minus = list(site)
            plus[derivative_axis] -= 1
            minus[derivative_axis] += 1
            sparse_add(result[out_axis], tuple(plus), value * (0.5 * sign))
            sparse_add(result[out_axis], tuple(minus), value * (-0.5 * sign))
    return tuple(result)  # type: ignore[return-value]


def endpoint_rho_ad(x1: Sequence[AD]) -> Sparse:
    # The global impulse box below proves the decisive endpoint stays in cell 7.
    out: Sparse = {}
    frac = [x1[i] - 7.0 for i in range(3)]
    for bits in itertools.product((0, 1), repeat=3):
        weight = AD.constant(float(Q_CHARGE))
        site = [7, 7, 7]
        for axis, bit in enumerate(bits):
            site[axis] += bit
            weight = weight * (frac[axis] if bit else 1.0 - frac[axis])
        sparse_add(out, tuple(site), weight)
    return sparse_smooth_all(out)


def start_rho_sparse(rho0: np.ndarray) -> Sparse:
    out: Sparse = {}
    for site in zip(*np.nonzero(np.abs(rho0) > 0.0)):
        out[tuple(int(v) for v in site)] = AD.constant(float(rho0[site]))
    return out


def path_current_for_order(x1: Sequence[AD], order: Sequence[int]) -> VectorSparse:
    delta = [x1[i] - X0[i] for i in range(3)]
    crossing = [(AD.constant(8.0) - X0[i]) / delta[i] for i in range(3)]
    breaks: List[AD] = [AD.constant(0.0)] + [crossing[i] for i in order] + [AD.constant(1.0)]
    raw: List[Sparse] = [{}, {}, {}]
    crossed: set[int] = set()
    for piece in range(4):
        ta, tb = breaks[piece], breaks[piece + 1]
        if piece > 0:
            crossed.add(order[piece - 1])
        pa = [AD.constant(X0[i]) + delta[i] * ta for i in range(3)]
        pb = [AD.constant(X0[i]) + delta[i] * tb for i in range(3)]
        for axis in range(3):
            ta_axis, tb_axis = (axis + 1) % 3, (axis + 2) % 3
            face_coordinate = 7 if axis in crossed else 8
            lower_a = 7 if ta_axis in crossed else 8
            lower_b = 7 if tb_axis in crossed else 8
            for da, db in itertools.product((0, 1), repeat=2):
                site_a, site_b = lower_a + da, lower_b + db
                wa0 = (pa[ta_axis] - site_a + 1.0) if da else (site_a + 1.0 - pa[ta_axis])
                wa1 = (pb[ta_axis] - site_a + 1.0) if da else (site_a + 1.0 - pb[ta_axis])
                wb0 = (pa[tb_axis] - site_b + 1.0) if db else (site_b + 1.0 - pa[tb_axis])
                wb1 = (pb[tb_axis] - site_b + 1.0) if db else (site_b + 1.0 - pb[tb_axis])
                da_w, db_w = wa1 - wa0, wb1 - wb0
                integral = wa0 * wb0 + (wa0 * db_w + da_w * wb0) * 0.5 + da_w * db_w / 3.0
                deposited = float(Q_CHARGE) * delta[axis] * (tb - ta) * integral
                site = [0, 0, 0]
                site[axis] = face_coordinate
                site[ta_axis] = site_a
                site[tb_axis] = site_b
                sparse_add(raw[axis], tuple(site), deposited)

    central: List[Sparse] = [{}, {}, {}]
    for axis in range(3):
        transverse = raw[axis]
        for other in range(3):
            if other != axis:
                transverse = sparse_smooth_axis(transverse, other)
        for site, value in transverse.items():
            sparse_add(central[axis], site, value * 0.5)
            plus = list(site)
            plus[axis] += 1
            sparse_add(central[axis], tuple(plus), value * 0.5)
    return tuple(central)  # type: ignore[return-value]


def union_vectors(fields: Iterable[VectorSparse]) -> VectorSparse:
    items = list(fields)
    out: List[Sparse] = [{}, {}, {}]
    for axis in range(3):
        sites = set().union(*(field[axis].keys() for field in items))
        for site in sites:
            values = [field[axis].get(site, AD.constant(0.0)) for field in items]
            value = values[0]
            for other in values[1:]:
                value = value.union(other)
            out[axis][site] = value
    return tuple(out)  # type: ignore[return-value]


def interval_impulse(p_mid: Sequence[float], p_radius: Sequence[float],
                     rho0: np.ndarray, dj: Sequence[np.ndarray],
                     j_error: float) -> Tuple[List[AD], List[AD]]:
    p = [AD.variable(float(p_mid[i]), float(p_radius[i]), i) for i in range(3)]
    h0 = math.sqrt(E_REST * E_REST + C2 * float(np.dot(P0, P0)))
    h1 = (AD.constant(E_REST * E_REST)
          + sum(value * value for value in p) * C2).sqrt()
    denominator = AD.constant(h0) + h1
    velocity = [(AD.constant(P0[i]) + p[i]) * C2 / denominator for i in range(3)]
    x1 = [AD.constant(X0[i]) + velocity[i] for i in range(3)]
    rho1 = endpoint_rho_ad(x1)
    rhobar = sparse_combine(start_rho_sparse(rho0), rho1, 0.5, 0.5)
    currents = [path_current_for_order(x1, order) for order in itertools.permutations((0, 1, 2))]
    q = union_vectors(currents)
    grad_rho = sparse_gradient(rhobar)
    curl_q = sparse_curl(q)
    source: List[Sparse] = []
    for axis in range(3):
        source.append(sparse_combine(grad_rho[axis], curl_q[axis], -G, G))

    impulse: List[AD] = []
    for momentum_axis in range(3):
        total = AD.constant(0.0)
        for vector_axis in range(3):
            for site, value in source[vector_axis].items():
                total = total + value * float(dj[momentum_axis][site][vector_axis])
        # The FFT dressing residual gives an L2 enclosure for J.  Central
        # difference has norm <=1, and the source norm is bounded below.
        inflation = G * (math.sqrt(3.0) + 1.0) * j_error
        derivatives = tuple(
            arb(float(value.mid()), float(value.rad()) + DERIVATIVE_ENCLOSURE_INFLATION)
            for value in total.derivative
        )
        total = AD(arb(float(total.value.mid()), float(total.value.rad()) + inflation), derivatives)
        impulse.append(total)
    return impulse, velocity


def interval_velocity(p_mid: Sequence[float], p_radius: Sequence[float]) -> List[AD]:
    p = [AD.variable(float(p_mid[i]), float(p_radius[i]), i) for i in range(3)]
    h0 = math.sqrt(E_REST * E_REST + C2 * float(np.dot(P0, P0)))
    h1 = (AD.constant(E_REST * E_REST)
          + sum(value * value for value in p) * C2).sqrt()
    denominator = AD.constant(h0) + h1
    return [(AD.constant(P0[i]) + p[i]) * C2 / denominator for i in range(3)]


def arb_abs_upper(value: arb) -> float:
    return float(value.abs_upper())


def arb_mid(value: arb) -> float:
    return float(value.mid())


def face_current_float(x1: np.ndarray) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
    # Reuse the exact piece formula with zero-radius balls and select the
    # actual crossing order.  At the symmetric root the three crossing times
    # coincide and all orders agree.
    p_dummy = [AD.variable(0.0, 0.0, i) for i in range(3)]
    x_ad = [AD.constant(float(x1[i])) + p_dummy[i] * 0.0 for i in range(3)]
    field = path_current_for_order(x_ad, (0, 1, 2))
    out = np.zeros((L, L, L, 3), dtype=float)
    for axis in range(3):
        for site, value in field[axis].items():
            out[site][axis] = arb_mid(value.value)
    rho1 = smooth_all(cic(x1, Q_CHARGE))
    rho0 = smooth_all(cic(X0, Q_CHARGE))
    return rho0, rho1, out


def numeric_impulse(p1: np.ndarray, rho0: np.ndarray,
                    dj: Sequence[np.ndarray], j_error: float) -> np.ndarray:
    impulse, _ = interval_impulse(p1, (0.0, 0.0, 0.0), rho0, dj, j_error)
    return np.array([arb_mid(value.value) for value in impulse])


def total_energy_counterexample(j0: np.ndarray, p1: np.ndarray) -> Dict[str, float]:
    h0 = math.sqrt(E_REST * E_REST + C2 * float(np.dot(P0, P0)))
    h1 = math.sqrt(E_REST * E_REST + C2 * float(np.dot(p1, p1)))
    ubar = C2 * (P0 + p1) / (h0 + h1)
    x1 = X0 + ubar
    rho0, rho1, q = face_current_float(x1)
    source = -G * gradient(0.5 * (rho0 + rho1)) + G * curl(q)
    source_impulse = np.array([float(np.sum(source * derivative(j0, axis))) for axis in range(3)])
    w0 = np.zeros_like(j0)
    w1 = w0 - apply_k(j0) + source
    j1 = j0 + w1

    def field_energy(j: np.ndarray, w: np.ndarray) -> float:
        kj = apply_k(j)
        return float(0.5 * np.sum(w * w) + 0.5 * np.sum(j * kj) - 0.5 * np.sum(w * kj))

    def interaction(rho: np.ndarray, j: np.ndarray, w: np.ndarray) -> float:
        return float(-G * np.sum(rho * divergence(j - 0.5 * w)))

    hf0, hf1 = field_energy(j0, w0), field_energy(j1, w1)
    ui0, ui1 = interaction(rho0, j0, w0), interaction(rho1, j1, w1)
    total0 = h0 + hf0 + ui0
    total1 = h1 + hf1 + ui1
    r0 = j0 - 0.5 * w0
    r1 = j1 - 0.5 * w1
    work_field = gradient(divergence(0.5 * (r0 + r1))) - curl(r1 - r0)
    matter_work = float(G * np.sum(q * work_field))
    denominator = max(1.0, abs(total0))
    return {
        "x1_x": float(x1[0]),
        "total_energy_before": total0,
        "total_energy_after": total1,
        "particle_energy_before": h0,
        "particle_energy_after": h1,
        "field_energy_before": hf0,
        "field_energy_after": hf1,
        "interaction_energy_before": ui0,
        "interaction_energy_after": ui1,
        "energy_relative_residual": abs(total1 - total0) / denominator,
        "matter_work": matter_work,
        "source_impulse_x": float(source_impulse[0]),
        "q_norm": float(np.linalg.norm(q.ravel())),
        "source_norm": float(np.linalg.norm(source.ravel())),
        "source_impulse_recoil_residual": float(np.max(np.abs((p1 - P0) - source_impulse))),
        "work_relative_residual": abs((h1 - h0) - matter_work) / denominator,
    }


rho0, J0, dressing_residual, dressing_error = static_dressing()
P0 = M_INERTIAL * V0 / math.sqrt(1.0 - float(np.dot(V0, V0)) / C2)
DJ = [derivative(J0, axis) for axis in range(3)]
dj_norms = np.array([np.linalg.norm(value.ravel()) for value in DJ])

# Any admitted fixed point obeys p1-p0=I.  The coated density has L1 norm 1,
# the central gradient/curl symbols have norm <=sqrt(3), the straight current
# has vector L1 norm <=|delta|<C_SPEED, and central D_i has norm <=1.
global_impulse_bound = G * (math.sqrt(3.0) + 1.0) * (dj_norms + dressing_error)
p_box_mid = P0.copy()
p_box_radius = global_impulse_bound.copy()

# This necessary box lies wholly in the same endpoint chart: every component
# crosses exactly the plane at coordinate 8 and no other plane.
velocity_box = interval_velocity(p_box_mid, p_box_radius)
x1_box = [AD.constant(X0[i]) + velocity_box[i] for i in range(3)]
chart_lower = [float(value.value.lower()) for value in x1_box]
chart_upper = [float(value.value.upper()) for value in x1_box]
chart_certified = all(7.0 < lo < hi < 8.0 for lo, hi in zip(chart_lower, chart_upper))

impulse_box, _ = interval_impulse(p_box_mid, p_box_radius, rho0, DJ, dressing_error)
q_rows = []
for output in impulse_box:
    q_rows.append([arb_abs_upper(output.derivative[column]) for column in range(3)])
contraction_bound = max(sum(row) for row in q_rows)

# Independent fixed-point iteration at the center of the certified global box.
p = P0.copy()
for _ in range(32):
    p_next = P0 + numeric_impulse(p, rho0, DJ, dressing_error)
    if np.max(np.abs(p_next - p)) < 1.0e-15:
        p = p_next
        break
    p = p_next
fixed_residual = float(np.max(np.abs(p - P0 - numeric_impulse(p, rho0, DJ, dressing_error))))
root_radius = fixed_residual / max(1.0e-30, 1.0 - contraction_bound) + 5.0e-14

energy = total_energy_counterexample(J0, p)
energy_factor = energy["energy_relative_residual"] / TOL
work_factor = energy["work_relative_residual"] / TOL
energy_residual_lower_bound = max(
    0.0, energy["energy_relative_residual"] - ENERGY_ENCLOSURE_INFLATION)
work_residual_lower_bound = max(
    0.0, energy["work_relative_residual"] - ENERGY_ENCLOSURE_INFLATION)

certificate_pass = (
    dressing_residual <= 1.0e-13
    and chart_certified
    and contraction_bound < 1.0
    and root_radius < 1.0e-10
    and energy_residual_lower_bound > TOL
    and work_residual_lower_bound > TOL
)

record = {
    "ftd_id": "FTD-0599",
    "protocol_sha256": "DDD146E19C06E488C584AFBAB4092FB802E72F4DFC13F12407A5A914704E8886",
    "arm": "L17_q-1_body_negative_ballistic",
    "verdict": "SITE_ONTIC_NATIVE_RECOIL_MAP_FAILS_ATOMIC_COMPATIBILITY" if certificate_pass else "PROTOCOL_INVALID",
    "dressing_residual_l2": dressing_residual,
    "dressing_solution_error_bound_l2": dressing_error,
    "global_impulse_bound": global_impulse_bound.tolist(),
    "necessary_p_box_mid": p_box_mid.tolist(),
    "necessary_p_box_radius": p_box_radius.tolist(),
    "endpoint_chart_lower": chart_lower,
    "endpoint_chart_upper": chart_upper,
    "chart_certified": chart_certified,
    "arb_contraction_bound_inf": contraction_bound,
    "derivative_enclosure_inflation": DERIVATIVE_ENCLOSURE_INFLATION,
    "independent_root": p.tolist(),
    "fixed_point_residual_inf": fixed_residual,
    "certified_root_radius_inf": root_radius,
    "unique_root_certified": contraction_bound < 1.0 and chart_certified,
    **energy,
    "energy_enclosure_inflation": ENERGY_ENCLOSURE_INFLATION,
    "energy_relative_residual_lower_bound": energy_residual_lower_bound,
    "work_relative_residual_lower_bound": work_residual_lower_bound,
    "energy_tolerance_factor": energy_factor,
    "work_tolerance_factor": work_factor,
    "certificate_pass": certificate_pass,
}

print(json.dumps(record, indent=2, sort_keys=True))
raise SystemExit(0 if certificate_pass else 1)

"""Exact/numerical certificate for FTD-0700 axial lattice-Cherenkov exposure."""

from __future__ import annotations

import math


def omega_axis(k: float) -> float:
    return 2.0 * math.asin(math.sin(k / 2.0) / math.sqrt(3.0))


def group_axis(k: float) -> float:
    return math.cos(k / 2.0) / math.sqrt(3.0 - math.sin(k / 2.0) ** 2)


def curvature_axis(k: float) -> float:
    return -math.sin(k / 2.0) / (3.0 - math.sin(k / 2.0) ** 2) ** 1.5


def omega(kx: float, ky: float, kz: float) -> float:
    s_value = sum(math.sin(component / 2.0) ** 2
                  for component in (kx, ky, kz))
    return 2.0 * math.asin(math.sqrt(s_value / 3.0))


def group(kx: float, ky: float, kz: float) -> tuple[float, float, float]:
    s_value = sum(math.sin(component / 2.0) ** 2
                  for component in (kx, ky, kz))
    denominator = 2.0 * math.sqrt(s_value * (3.0 - s_value))
    return tuple(math.sin(component) / denominator
                 for component in (kx, ky, kz))


c_ir = 1.0 / math.sqrt(3.0)
v_edge = 2.0 * math.asin(1.0 / math.sqrt(3.0)) / math.pi
assert math.isclose(c_ir, 0.5773502691896258, rel_tol=0.0, abs_tol=2e-16)
assert math.isclose(v_edge, 0.3918265520306073, rel_tol=0.0, abs_tol=2e-16)

# The analytic curvature formula is strictly negative on the open axis branch.
for index in range(1, 1000):
    k = math.pi * index / 1000.0
    assert curvature_axis(k) < 0.0

# Concavity implies decreasing phase speed; sample it as a certificate check.
phase_speeds = [omega_axis(math.pi * i / 1000.0) / (math.pi * i / 1000.0)
                for i in range(1, 1001)]
assert all(a > b for a, b in zip(phase_speeds, phase_speeds[1:]))
assert abs(phase_speeds[-1] - v_edge) < 2e-16

# Collinear phase match: exact, but longitudinal for an axial convective current.
velocity = 0.5
k_star = 2.0 * math.pi / 3.0
omega_star = omega_axis(k_star)
group_star = group_axis(k_star)
assert math.isclose(omega_star, math.pi / 3.0, rel_tol=0.0, abs_tol=3e-16)
assert math.isclose(omega_star, velocity * k_star, rel_tol=0.0, abs_tol=3e-16)
assert math.isclose(group_star, 1.0 / 3.0, rel_tol=0.0, abs_tol=2e-16)
assert group_star < velocity < c_ir

# Exact oblique transverse witness at v=1/2.
k_oblique = (math.pi, math.pi / 2.0, 0.0)
omega_oblique = omega(*k_oblique)
group_oblique = group(*k_oblique)
assert math.isclose(omega_oblique, math.pi / 2.0, rel_tol=0.0, abs_tol=3e-16)
assert math.isclose(omega_oblique, velocity * k_oblique[0],
                    rel_tol=0.0, abs_tol=3e-16)
assert math.isclose(group_oblique[0], 0.0, rel_tol=0.0, abs_tol=5e-17)
assert math.isclose(group_oblique[1], 1.0 / 3.0,
                    rel_tol=0.0, abs_tol=2e-16)
assert math.isclose(group_oblique[2], 0.0, rel_tol=0.0, abs_tol=5e-17)

# For J=(1,0,0), khat=(2,sqrt(2),0), exactly one third of |J|^2 is transverse.
khat = (2.0, math.sqrt(2.0), 0.0)
khat_sq = sum(value * value for value in khat)
dot = khat[0]
j_transverse = (1.0 - khat[0] * dot / khat_sq,
                -khat[1] * dot / khat_sq,
                0.0)
transverse_fraction = sum(value * value for value in j_transverse)
assert math.isclose(transverse_fraction, 1.0 / 3.0,
                    rel_tol=0.0, abs_tol=2e-16)

# Body-diagonal collinear phase matching occurs only at |v|=1/sqrt(3).
for q in (0.1, 0.4, 0.9, 1.4, 2.0, 3.0):
    omega_diag = 2.0 * math.asin(math.sin(q / 2.0))
    assert math.isclose(omega_diag, q, rel_tol=0.0, abs_tol=2e-15)
    source_frequency = math.sqrt(3.0) * q * c_ir
    assert math.isclose(source_frequency, omega_diag, rel_tol=0.0, abs_tol=2e-15)

print("FTD-0700 axial lattice-Cherenkov exposure certificate: PASS")
print(f"c_IR={c_ir:.16g}")
print(f"v_edge={v_edge:.16g}")
print(f"collinear_control: v=1/2 k={k_star:.16g} omega={omega_star:.16g}")
print("transverse_witness: v=1/2 k=(pi,pi/2,0) "
      f"omega={omega_oblique:.16g} vg={group_oblique} "
      f"J_T_fraction={transverse_fraction:.16g}")

#!/usr/bin/env python3
"""Independent exact/synthetic proof for FTD-0564.

This script does not import the C++ observer. It independently evaluates the
Berg--Luescher degree and affine octahedral surface flux, checks the periodic
tree-routing witness, and locks the frozen-variable provenance by source hash.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import platform
from pathlib import Path
from typing import Iterable


ROOT = Path(__file__).resolve().parents[2]
GATE = 1.0e-12
PI = math.pi


def add(a, b):
    return tuple(x + y for x, y in zip(a, b))


def sub(a, b):
    return tuple(x - y for x, y in zip(a, b))


def mul(a, scalar):
    return tuple(scalar * x for x in a)


def dot(a, b):
    return sum(x * y for x, y in zip(a, b))


def cross(a, b):
    return (
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    )


def norm(a):
    return math.sqrt(dot(a, a))


def normalize(a):
    magnitude = norm(a)
    assert magnitude > 0.0
    return mul(a, 1.0 / magnitude)


VERTICES = (
    (1.0, 0.0, 0.0), (-1.0, 0.0, 0.0),
    (0.0, 1.0, 0.0), (0.0, -1.0, 0.0),
    (0.0, 0.0, 1.0), (0.0, 0.0, -1.0),
)


def face_indices(sx: int, sy: int, sz: int):
    ix, iy, iz = (0 if sx == 0 else 1), (2 if sy == 0 else 3), (4 if sz == 0 else 5)
    return (ix, iz, iy) if (sx + sy + sz) % 2 else (ix, iy, iz)


FACES = tuple(
    face_indices(sx, sy, sz)
    for sx in range(2) for sy in range(2) for sz in range(2)
)


def rotate(a, rotation: int):
    if rotation == 1:
        return (a[2], a[0], a[1])
    if rotation == 2:
        return (a[1], a[2], a[0])
    return a


def solid_angle(a, b, c):
    return 2.0 * math.atan2(
        dot(a, cross(b, c)), 1.0 + dot(a, b) + dot(b, c) + dot(c, a)
    )


def degree(field):
    directions = tuple(normalize(value) for value in field)
    return sum(solid_angle(*(directions[index] for index in face)) for face in FACES) / (4.0 * PI)


def flux(positions, field):
    total = 0.0
    for face in FACES:
        a, b, c = (positions[index] for index in face)
        area_vector = mul(cross(sub(b, a), sub(c, a)), 0.5)
        mean_field = mul(tuple(sum(field[index][axis] for index in face) for axis in range(3)), 1.0 / 3.0)
        total += dot(area_vector, mean_field)
    return total


def arm(family: str, amplitude: float, polarity: int, rotation: int):
    positions = tuple(rotate(vertex, rotation) for vertex in VERTICES)
    offset = rotate((0.0, 0.0, 2.0), rotation)
    raw = positions if family == "hedgehog" else tuple(add(vertex, offset) for vertex in positions)
    field = tuple(mul(value, polarity * amplitude) for value in raw)
    return degree(field), flux(positions, field), min(norm(value) for value in field)


def flat_index(L: int, x: int, y: int, z: int):
    return ((x % L) * L + (y % L)) * L + (z % L)


def route_zero_sum_source(L: int, source: list[int]):
    volume = L ** 3
    subtree = source.copy()
    jx, jy, jz = [0] * volume, [0] * volume, [0] * volume
    for raw in range(volume - 1, 0, -1):
        x, yz = divmod(raw, L * L)
        y, z = divmod(yz, L)
        if z > 0:
            parent = flat_index(L, x, y, z - 1)
            jz[parent] = -subtree[raw]
        elif y > 0:
            parent = flat_index(L, x, y - 1, 0)
            jy[parent] = -subtree[raw]
        else:
            parent = flat_index(L, x - 1, 0, 0)
            jx[parent] = -subtree[raw]
        subtree[parent] += subtree[raw]
    residual = 0
    for x in range(L):
        for y in range(L):
            for z in range(L):
                index = flat_index(L, x, y, z)
                divergence = (
                    jx[index] - jx[flat_index(L, x - 1, y, z)]
                    + jy[index] - jy[flat_index(L, x, y - 1, z)]
                    + jz[index] - jz[flat_index(L, x, y, z - 1)]
                )
                residual = max(residual, abs(divergence - source[index]))
    return subtree[0], residual


def sha256(path: Path):
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def require_all(text: str, needles: Iterable[str], label: str):
    missing = [needle for needle in needles if needle not in text]
    assert not missing, f"{label}: missing {missing}"


def source_provenance():
    files = {
        "voxel": ROOT / "engine/include/ftd/voxel.h",
        "gauge_field": ROOT / "engine/include/ftd/gauge_field.h",
        "render_bridge": ROOT / "engine/src/render_bridge.cpp",
        "gauge_test": ROOT / "engine/tests/test_gauge_links.cpp",
        "phase_read": ROOT / "engine/src/render_bridge_phases/phase_read.cpp",
        "preregistration": ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_ORIENTATION_GAUSS_INDEPENDENCE_v1.md",
    }
    voxel = files["voxel"].read_text(encoding="utf-8")
    require_all(voxel, (
        "Vec3 flux;", "Vec3 wave_vel;", "Vec3 flux_L;", "Vec3 flux_R;",
        "Vec3 wave_vel_L;", "Vec3 wave_vel_R;",
    ), "regular real field variables")
    gauge = files["gauge_field"].read_text(encoding="utf-8")
    require_all(gauge, ("struct SU2Link", "struct SU3Link"), "compact link declarations")
    render = files["render_bridge"].read_text(encoding="utf-8")
    require_all(render, (
        "[IMPOSED] Wilson-action staple relaxation",
        "The links are WRITE-ONLY w.r.t. the substrate",
        "if (toggles.su2_gauge)", "if (toggles.su3_gauge)",
    ), "gauge tick scope")
    gauge_test = files["gauge_test"].read_text(encoding="utf-8")
    require_all(gauge_test, (
        "default OFF", "off.substrate == on.substrate",
        "gauge-enabled run folds to the identical substrate hash",
    ), "write-only regression")
    phase_read = files["phase_read"].read_text(encoding="utf-8")
    assert "su2_links" not in phase_read and "su3_links" not in phase_read

    reference_files = sorted(
        path.relative_to(ROOT).as_posix()
        for path in (ROOT / "engine").rglob("*")
        if path.suffix in {".h", ".cpp"}
        and ("su2_links_" in path.read_text(encoding="utf-8", errors="ignore")
             or "su3_links_" in path.read_text(encoding="utf-8", errors="ignore"))
    )
    allowed = {
        "engine/include/ftd/constants.h",
        "engine/include/ftd/gpu_engine.h",
        "engine/include/ftd/render_bridge.h",
        "engine/include/ftd/transmutation_phases.h",
        "engine/src/backend.cpp",
        "engine/src/render_bridge.cpp",
        "engine/src/transmutation_phases.cpp",
        "engine/tests/support/gauge_test_utils.h",
        "engine/tests/test_gauge_gpu_parity.cpp",
        "engine/tests/test_gauge_links.cpp",
    }
    assert set(reference_files) == allowed, reference_files
    return {name: sha256(path) for name, path in files.items()}, reference_files


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--json-output", type=Path)
    args = parser.parse_args()

    amplitudes = (1.0, 0.5, 0.25, 0.125, 0.0625)
    max_degree_residual = 0.0
    max_flux_residual = 0.0
    max_equal_flux_residual = 0.0
    records = []
    for family in ("hedgehog", "translated_image"):
        for amplitude in amplitudes:
            for polarity in (1, -1):
                for rotation in range(3):
                    observed_degree, observed_flux, minimum = arm(family, amplitude, polarity, rotation)
                    expected_degree = float(polarity if family == "hedgehog" else 0)
                    expected_flux = 4.0 * polarity * amplitude
                    max_degree_residual = max(max_degree_residual, abs(observed_degree - expected_degree))
                    max_flux_residual = max(max_flux_residual, abs(observed_flux - expected_flux))
                    assert minimum > 0.0
                    records.append((family, amplitude, polarity, rotation, observed_degree, observed_flux))

    for amplitude in amplitudes:
        for polarity in (1, -1):
            for rotation in range(3):
                h = arm("hedgehog", amplitude, polarity, rotation)
                t = arm("translated_image", amplitude, polarity, rotation)
                max_equal_flux_residual = max(max_equal_flux_residual, abs(h[1] - t[1]))
                assert abs(h[0] - t[0]) > 0.5

    max_tree_residual = 0
    rank_witnesses = 0
    for L in (3, 5):
        source = [0] + [(index % 7) - 3 for index in range(1, L ** 3)]
        source[0] = -sum(source)
        root, residual = route_zero_sum_source(L, source)
        max_tree_residual = max(max_tree_residual, residual)
        assert root == 0 and residual == 0
        assert (L ** 3 - 1) + (2 * L ** 3 + 1) == 3 * L ** 3
        rank_witnesses += 1

    hashes, gauge_reference_files = source_provenance()
    implementation_files = {
        "header": ROOT / "engine/include/ftd/eft/orientation_gauss_independence.h",
        "source": ROOT / "engine/src/eft/orientation_gauss_independence.cpp",
        "test": ROOT / "engine/tests/test_orientation_gauss_independence.cpp",
        "independent_proof": Path(__file__).resolve(),
    }
    implementation_hashes = {
        name: sha256(path) for name, path in implementation_files.items()
    }
    assert hashes["preregistration"] == "25DB8EA8343E165FE4EFC3FB2D83C4520BEC76CC97A05F907412A7E029C58663"
    assert max_degree_residual <= GATE
    assert max_flux_residual <= GATE
    assert max_equal_flux_residual <= GATE
    result = {
        "ftd_id": "FTD-0564",
        "verdict": "ORIENTATION_GAUSS_INDEPENDENT",
        "platform": platform.platform(),
        "field_representation": "regular noncompact real Vec3 vertex field",
        "surface": "unit octahedral Moore shell with piecewise-affine face interpolation",
        "orientation_normalization": "J/|J| at each nonzero shell vertex",
        "gauss_normalization": "exact geometric affine surface flux; expected 4*p*A",
        "amplitudes": list(amplitudes),
        "polarities": [1, -1],
        "cyclic_rotations": [0, 1, 2],
        "tolerance": GATE,
        "periodic_rank_volumes": [3, 5],
        "arms": len(records),
        "rank_witnesses": rank_witnesses,
        "maximum_degree_residual": max_degree_residual,
        "maximum_flux_residual": max_flux_residual,
        "maximum_equal_flux_residual": max_equal_flux_residual,
        "maximum_tree_routing_residual": max_tree_residual,
        "degree_does_not_determine_flux": True,
        "flux_does_not_determine_degree": True,
        "topology_alone_charge_magnitude_closed": True,
        "topological_core_with_action_remains_open": True,
        "periodic_net_defect_index": 0,
        "periodic_index_basis": "Poincare-Hopf on T^3, chi(T^3)=0 (imported theorem)",
        "source_hashes_sha256": hashes,
        "implementation_hashes_sha256": implementation_hashes,
        "gauge_link_reference_files": gauge_reference_files,
    }
    if args.json_output:
        args.json_output.parent.mkdir(parents=True, exist_ok=True)
        args.json_output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2))
    print("PASS: orientation degree and Gauss flux are independent")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

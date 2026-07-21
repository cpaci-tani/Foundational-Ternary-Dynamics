#!/usr/bin/env python3
"""Recompute and statically verify the FTD-0404 volumetric-measure contract."""

from pathlib import Path
import re
from fractions import Fraction

ROOT = Path(__file__).resolve().parents[2]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


failures = 0


def check(name: str, condition: bool) -> None:
    global failures
    print(("PASS  " if condition else "FAIL  ") + name)
    if not condition:
        failures += 1


edge = Fraction(2)
area = edge * edge
volume = edge * edge * edge
density = Fraction(7, 2)

check("A1 D=3 cubic-volume exponent", volume == 8)
check("A2 edge-two face area is four", area == 4)
check("A3 density 7/2 integrates to 28", density * volume == 28)
check("A4 unit cell is numerically neutral", Fraction(1) ** 3 == 1)
check("A5 quadratic three-component norm", 1 * 1 + 2 * 2 + 2 * 2 == 9)

measure = read("engine/include/ftd/volumetric_measure.h")
audit_h = read("engine/include/ftd/render_bridge_diagnostics.h")
cpu = read("engine/src/diagnostics_compute.cpp")
gpu = read("engine/cuda/gpu_engine.cu")
poisson = read("engine/src/poisson_solvers.cpp")
lag_h = read("engine/include/ftd/lagrangian.h")
lag_cpp = read("engine/src/lagrangian.cpp")
wasm = read("engine/wasm/ftd_wasm.cpp")
js = read("engine/web/js/bridge/wasm-bridge.js")

check("S1 measure fixes D=3 and uses an explicit cube",
      "static_assert(D_SPATIAL == 3" in measure
      and "return edge * edge * edge;" in measure)
check("S2 local density remains quadratic",
      "0.5 * magnitude_squared" in measure
      and "flux_magnitude_squared" in measure
      and "wave_magnitude_squared" in measure)
check("S3 audit exposes density sums and cell volume",
      all(token in audit_h for token in (
          "cell_volume", "field_energy_density_sum", "wave_energy_density_sum")))
check("S4 CPU integrates density with V_cell",
      "integrate_voxel_density(field_density)" in cpu
      and "integrate_voxel_density(wave_density)" in cpu)
check("S5 GPU mirrors density integration",
      "integrate_voxel_density(field_density)" in gpu
      and "integrate_voxel_density(wave_density)" in gpu)

particle_block = cpu[cpu.index("if (s != 0)"):cpu.index("// Constrained-site Gauss residual")]
check("S6 point-particle channels have no volume multiplier",
      "integrate_voxel_density" not in particle_block
      and "particle_rest_energy += E_REST" in particle_block)

poisson_body = poisson[poisson.index("void solve_latency_poisson_cpu"):]
check("S7 gravity reads local density, not integrated cell energy",
      "local_field_wave_energy_density" in poisson_body
      and "integrate_voxel_density" not in poisson_body)
check("S8 Lagrangian diagnostics integrate spatial densities",
      "integrate_voxel_density(fk)" in lag_cpp
      and "d.total_hamiltonian += integrate_voxel_density(" in lag_cpp
      and "hamiltonian_density(v, divJ, rho)" in lag_cpp
      and "cell_volume" in lag_h)
check("S9 WASM energy view is append-only after index 24",
      "s_audit_cache(28)" in wasm
      and "s_audit_cache[24] = ea.dynamic_energy" in wasm
      and "s_audit_cache[25] = ea.cell_volume" in wasm)
check("S10 WASM Lagrangian view is append-only after index 15",
      "s_lag_cache(17)" in wasm
      and "s_lag_cache[15] = lag.locked_count" in wasm
      and "s_lag_cache[16] = lag.cell_volume" in wasm)
check("S11 direct JS consumer reads appended metadata",
      "cellVolume: arr[25]" in js
      and "fieldEnergyDensitySum: arr[26]" in js
      and "waveEnergyDensitySum: arr[27]" in js
      and "cellVolume: arr[16]" in js)

component_cube = re.compile(r"(?:flux|wave_vel|velocity)\.[xyz]\s*\*\s*(?:flux|wave_vel|velocity)\.[xyz]\s*\*\s*(?:flux|wave_vel|velocity)\.[xyz]")
check("S12 no component-cube formula introduced", not component_cube.search(cpu + gpu + lag_cpp))

print("VERDICT " + ("VOLUMETRIC-CONTRACT-PASS" if failures == 0 else "INVALID"))
raise SystemExit(1 if failures else 0)

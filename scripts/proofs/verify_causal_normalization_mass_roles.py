#!/usr/bin/env python3
"""Recompute and source-audit the locked FTD-0402 contract.

This is a contract verifier, not a numerical search.  Its arithmetic anchors
come only from the preregistered raw-lattice definitions.
"""

from __future__ import annotations

import re
import sys
from decimal import Decimal, getcontext
from fractions import Fraction
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
FAILURES: list[str] = []


def check(name: str, condition: bool) -> None:
    print(f"{'PASS' if condition else 'FAIL'}  {name}")
    if not condition:
        FAILURES.append(name)


def text(rel: str) -> str:
    return (ROOT / rel).read_text(encoding="utf-8")


def strip_cpp_comments(source: str) -> str:
    source = re.sub(r"/\*.*?\*/", "", source, flags=re.S)
    return re.sub(r"//.*", "", source)


def exact_anchors() -> None:
    c2 = Fraction(1, 3)
    check("A1 C^2 = 1/3", c2 == Fraction(1, 3))

    def rate2(u2: Fraction, l2: Fraction) -> Fraction:
        remaining = Fraction(1) - u2 / c2 - l2
        return max(remaining, Fraction(0))

    check("A2 rate^2(u=C,L=0) = 0", rate2(c2, Fraction(0)) == 0)
    check("A3 rate^2(u=C/2,L=0) = 3/4",
          rate2(c2 / 4, Fraction(0)) == Fraction(3, 4))
    for l2 in (Fraction(0), Fraction(1, 16), Fraction(9, 25)):
        check(f"A4 rest rate^2 = 1-L^2 for L^2={l2}",
              rate2(Fraction(0), l2) == 1 - l2)

    k_b = Fraction(511, 1000)
    e0 = k_b * c2
    check("A5 E_REST=M_INERTIAL*C^2=K_B/3", e0 == Fraction(511, 3000))

    # Exact squared flat invariant at beta=3/5.  gamma^2=25/16,
    # u^2=beta^2*C^2, E^2=gamma^2*E0^2, P^2=gamma^2*M^2*u^2.
    beta2 = Fraction(9, 25)
    gamma2 = 1 / (1 - beta2)
    lhs = gamma2 * e0 * e0
    rhs = e0 * e0 + c2 * gamma2 * k_b * k_b * beta2 * c2
    check("A6 E^2=E0^2+C^2 P^2", lhs == rhs)

    # Legendre identity, checked at rational B through high-precision Decimal:
    # p*u-L = E0*(beta^2/r + r) = E0*f/r, r^2=f-beta^2.
    getcontext().prec = 80
    beta2_d = Decimal(9) / Decimal(100)
    l2_d = Decimal(1) / Decimal(25)
    f_d = Decimal(1) - l2_d
    r_d = (f_d - beta2_d).sqrt()
    h_legendre = Decimal(1) * (beta2_d / r_d + r_d)
    h_closed = f_d / r_d
    check("A7 Born-Infeld Legendre identity", h_legendre == h_closed)


def source_contract() -> None:
    causal = text("engine/include/ftd/causal_kinematics.h")
    masses = text("engine/include/ftd/ontic/particle_masses.h")
    proper = text("engine/include/ftd/proper_time_rate.h")
    poisson_cpu = text("engine/src/poisson_solvers.cpp")
    poisson_gpu = text("engine/cuda/kernels_poisson.cu")
    forces_gpu = text("engine/cuda/kernels_forces.cu")
    audit_cpu = text("engine/src/diagnostics_compute.cpp")
    wasm = text("engine/wasm/ftd_wasm.cpp")

    required_causal = (
        "raw_speed2 / (C_SPEED * C_SPEED)",
        "beta2 + l2",
        "1.0 - causal_budget",
        "C_SPEED * causal_sqrt(f)",
        "beta2 / f",
    )
    check("S1 one raw causal interface contains every frozen map",
          all(token in causal for token in required_causal))
    check("S2 proper_time_rate compatibility header delegates",
          '#include "causal_kinematics.h"' in proper
          and "double proper_time_rate" not in proper)
    check("S3 explicit mass roles",
          all(token in masses for token in (
              "M_INERTIAL      = K_B",
              "E_REST          = M_INERTIAL * C_SPEED * C_SPEED",
              "M_GRAVITATIONAL = K_B",
              "M_REST = M_INERTIAL",
          )))
    check("S4 CPU/GPU Poisson consume M_GRAVITATIONAL",
          "M_GRAVITATIONAL" in poisson_cpu
          and "M_GRAVITATIONAL" in poisson_gpu)
    check("S5 GPU latency no longer advances tau or applies legacy clamp",
          "latency_tau_bandwidth_kernel" not in poisson_gpu
          and "C_SPEED * f_clamped" not in poisson_gpu)
    check("S6 GPU has one post-accumulation momentum integration",
          "integrate_forces_kernel" in forces_gpu
          and "launch_integrate_forces" in forces_gpu
          and "atomicAdd(&vel_" not in forces_gpu)
    check("S7 audit exposes exact normalized particle channels",
          all(token in audit_cpu for token in (
              "flat_particle_kinetic_energy",
              "gamma0 * M_INERTIAL",
              "particle_rest_energy += E_REST",
              "dynamic_energy = a.field_energy + a.wave_energy + a.particle_ke",
          )))
    check("S8 WASM fixed view is append-only 25 fields",
          "s_audit_cache(25)" in wasm
          and "s_audit_cache[18] = ea.charge_total" in wasm
          and "s_audit_cache[19] = ea.particle_rest_energy" in wasm
          and "s_audit_cache[24] = ea.dynamic_energy" in wasm)

    allowed = {
        ROOT / "engine/include/ftd/constants.h",
        ROOT / "engine/include/ftd/ontic/particle_masses.h",
        ROOT / "engine/web/js/constants.js",
    }
    offenders: list[str] = []
    roots = (ROOT / "engine/include", ROOT / "engine/src", ROOT / "engine/cuda",
             ROOT / "engine/wasm", ROOT / "engine/web/js")
    for base in roots:
        for path in base.rglob("*"):
            if path.suffix not in {".h", ".cpp", ".cu", ".js"} or path in allowed:
                continue
            if re.search(r"\bM_REST\b", strip_cpp_comments(path.read_text(encoding="utf-8"))):
                offenders.append(str(path.relative_to(ROOT)))
    check("S9 no production M_REST consumer", not offenders)
    if offenders:
        print("     offenders: " + ", ".join(offenders))


def main() -> int:
    print("FTD-0402 causal normalization / mass-role verifier")
    exact_anchors()
    source_contract()
    if FAILURES:
        print(f"VERDICT INVALID ({len(FAILURES)} failed checks)")
        return 1
    print("VERDICT CONTRACT-CHECKS-PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())

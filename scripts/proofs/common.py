"""
Shared infrastructure for the FTD Proof Suite.

All mathematical constants are computed from scratch here — proofs must
NOT import from the project's constants.py. Self-containment is essential:
the proof suite must be independently verifiable.
"""

from __future__ import annotations

import json
import math
import sys
from dataclasses import dataclass, field
from typing import Optional

import numpy as np
from scipy.special import gamma as scipy_gamma
from scipy.special import ellipk, ellipe

# =============================================================================
# Tolerances
# =============================================================================

MACHINE_EPS = 1e-14
PPM_1 = 1e-6
PPM_10 = 1e-5
PERCENT_01 = 0.001
PERCENT_1 = 0.01
PERCENT_5 = 0.05
PERCENT_10 = 0.10
PERCENT_15 = 0.15

# =============================================================================
# Self-Contained Mathematical Constants (computed from scratch)
# =============================================================================

# Layer -1: Self-referential seed
E = math.e

# Layer 0: Transcendental seeds
EULER_GAMMA = 0.5772156649015329  # Euler-Mascheroni
GAMMA_QUARTER = float(scipy_gamma(0.25))  # Gamma(1/4)

# Layer 1: Elliptic geometry
VARPI = GAMMA_QUARTER**2 / (2.0 * math.sqrt(2.0 * math.pi))  # lemniscate constant

# Gauss's constant: M = 1/AGM(1, sqrt(2))
# Use the identity: varpi = pi * M  =>  M = varpi / pi
# But pi here is the standard math.pi (we derive the "ontic pi" from G* later)
GAUSS_M = VARPI / math.pi

# Layer 2: Universal operator
G_STAR = 2.0 * math.sqrt(VARPI * GAUSS_M)  # = 2*varpi/sqrt(pi)

# Derived pi from ontic chain: pi = 4*varpi^2 / G*^2
PI_ONTIC = 4.0 * VARPI**2 / G_STAR**2

# Packing fraction
PF = PI_ONTIC / 4.0

# Layer 3: Master quadratic
COEFFICIENT = 16
DISC = 256.0 * G_STAR**4 - 64.0 * G_STAR**3
X_PLUS = (16.0 * G_STAR**2 + math.sqrt(DISC)) / 2.0
X_MINUS = (16.0 * G_STAR**2 - math.sqrt(DISC)) / 2.0

# Layer 4: Framework integers
D_SPATIAL = 3
N_C = int(math.floor(X_MINUS))  # = 3
N_GEN = N_C
N_F = 2 * N_GEN
N_BASE = 2**((D_SPATIAL + 1) // 2)  # = 4
B_3 = (11 * N_C - 2 * N_F) // 3  # = 7
N_EFF = B_3 + 2 * N_C  # = 13
D_CONSTRAINT = N_C * N_BASE**2 - 1  # = 47

# Layer 5: Coupling constants
ALPHA = 1.0 / X_PLUS
G_C = math.sqrt(ALPHA)
SIN2_WEINBERG = N_C / N_EFF
G_N = 1.0 / (B_3 + N_C)**2
ALPHA_S_MZ = B_3 / (B_3 + 4.0 * N_EFF)

# Layer 6: Mass scale
M_PLANCK = 1.22089e19  # GeV [EXTERNAL INPUT]
K_B = 0.511  # MeV (electron mass)

# Mass ratios
MU_RATIO = 3 * B_3 * (B_3 + N_C) - N_C  # = 207
TAU_RATIO = (N_EFF + N_BASE) * MU_RATIO - 2 * N_C * B_3  # = 3477

# Key elliptic integral
K_HALF = float(ellipk(0.5))  # K(1/sqrt(2))

# =============================================================================
# Experimental values (CODATA 2022 / PDG 2024)
# =============================================================================

CODATA_ALPHA_INV = 137.035999177
CODATA_SIN2_W = 0.23122
CODATA_ALPHA_S = 0.1179
EXP_M_E = 0.51100  # MeV
EXP_M_MU = 105.658  # MeV
EXP_M_TAU = 1776.86  # MeV
EXP_M_P = 938.272  # MeV
EXP_V_HIGGS = 246.22  # GeV
EXP_M_HIGGS = 125.25  # GeV (PDG 2024)
EXP_SIN2_12 = 0.307
EXP_SIN2_23 = 0.546
EXP_SIN2_13 = 0.02203
EXP_DM2_RATIO = 32.85
EXP_ALPHA_G = 5.906e-39

# =============================================================================
# Proof Result Infrastructure
# =============================================================================

TAGS = ("[AXIOM]", "[THEOREM]", "[SELECTION]", "[CONJECTURE]", "[IMPOSED]",
        "[CONDITIONAL]", "[EXTERNAL]")


@dataclass
class ProofResult:
    """A single verified claim in the proof chain."""
    name: str
    claim: str
    value: float
    expected: float
    tolerance: float
    tag: str = "[THEOREM]"
    passed: bool = False
    error: float = 0.0
    module: str = ""

    def __post_init__(self):
        if self.expected == 0.0:
            self.error = abs(self.value)
            self.passed = self.error < self.tolerance
        else:
            self.error = abs(self.value - self.expected) / abs(self.expected)
            self.passed = self.error < self.tolerance


@dataclass
class ProofSuite:
    """Collects and reports proof results from a single module."""
    name: str
    results: list[ProofResult] = field(default_factory=list)

    def add(self, name: str, claim: str, value: float, expected: float,
            tolerance: float, tag: str = "[THEOREM]") -> ProofResult:
        r = ProofResult(
            name=name, claim=claim, value=value, expected=expected,
            tolerance=tolerance, tag=tag, module=self.name
        )
        self.results.append(r)
        return r

    def assert_close(self, name: str, got: float, expected: float,
                     tol: float, tag: str = "[THEOREM]") -> ProofResult:
        return self.add(name, f"{name}: {got} ≈ {expected}", got, expected, tol, tag)

    def assert_equal(self, name: str, got: float, expected: float,
                     tag: str = "[THEOREM]") -> ProofResult:
        return self.add(name, f"{name}: {got} = {expected}",
                        got, expected, MACHINE_EPS, tag)

    def assert_true(self, name: str, condition: bool,
                    tag: str = "[THEOREM]") -> ProofResult:
        val = 1.0 if condition else 0.0
        return self.add(name, name, val, 1.0, MACHINE_EPS, tag)

    @property
    def passed(self) -> int:
        return sum(1 for r in self.results if r.passed)

    @property
    def failed(self) -> int:
        return sum(1 for r in self.results if not r.passed)

    @property
    def total(self) -> int:
        return len(self.results)

    @property
    def all_pass(self) -> bool:
        return self.failed == 0

    def by_tag(self, tag: str) -> list[ProofResult]:
        return [r for r in self.results if r.tag == tag]

    def summary(self) -> str:
        lines = [
            f"\n{'='*70}",
            f"  {self.name}",
            f"{'='*70}",
        ]
        for r in self.results:
            status = "PASS" if r.passed else "FAIL"
            err_str = ""
            if r.expected != 0 and r.expected != 1.0:
                if r.error < 1e-6:
                    err_str = f" ({r.error*1e6:.2f} ppm)"
                elif r.error < 0.01:
                    err_str = f" ({r.error*100:.4f}%)"
                else:
                    err_str = f" ({r.error*100:.2f}%)"
            lines.append(f"  {status:4s} {r.tag:14s} {r.name}{err_str}")

        lines.append(f"\n  Total: {self.total} | Passed: {self.passed} | Failed: {self.failed}")

        # Breakdown by tag
        for tag in TAGS:
            items = self.by_tag(tag)
            if items:
                p = sum(1 for i in items if i.passed)
                lines.append(f"    {tag:14s}: {p}/{len(items)} passed")

        lines.append(f"{'='*70}")
        return "\n".join(lines)

    def print_summary(self):
        print(self.summary())

    def to_json(self) -> str:
        return json.dumps([{
            "name": r.name, "claim": r.claim, "value": r.value,
            "expected": r.expected, "error": r.error, "tag": r.tag,
            "passed": r.passed, "module": r.module
        } for r in self.results], indent=2)


def merge_suites(name: str, suites: list[ProofSuite]) -> ProofSuite:
    """Merge multiple ProofSuites into one master suite."""
    master = ProofSuite(name=name)
    for s in suites:
        master.results.extend(s.results)
    return master

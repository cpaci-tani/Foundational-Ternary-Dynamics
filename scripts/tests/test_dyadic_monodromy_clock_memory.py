"""Independent regression checks for the FTD-0777 finite-cover certificate.

These tests validate construction-local mathematics only.  They do not make a
native FTD clock or memory claim.
"""

from __future__ import annotations

import importlib.util
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROOF_PATH = ROOT / "scripts" / "proofs" / "proof_dyadic_monodromy_clock_memory.py"


def load_proof_module():
    spec = importlib.util.spec_from_file_location("dyadic_monodromy_proof", PROOF_PATH)
    assert spec is not None and spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def test_locked_gate_table_passes() -> None:
    proof = load_proof_module()
    gates = proof.build_gate_table()
    assert len(gates) == 16
    assert all(gate.passed for gate in gates)


def test_single_tower_confounds_payload_and_epoch() -> None:
    proof = load_proof_module()
    for depth in range(1, 9):
        assert proof.monodromy(depth, 0, 1) == proof.monodromy(depth, 1, 0)


def test_two_tower_relative_payload_is_invariant() -> None:
    proof = load_proof_module()
    for depth in range(0, 9):
        modulus = 2**depth
        for reference in range(modulus):
            payload = (3 * reference + 1) % modulus
            relative = (payload - reference) % modulus
            for loops in (0, 1, modulus - 1, modulus, modulus + 1):
                moved_reference = proof.monodromy(depth, reference, loops)
                moved_payload = proof.monodromy(depth, payload, loops)
                assert (moved_payload - moved_reference) % modulus == relative


def test_depth_four_strobe_first_returns_at_sixteen() -> None:
    proof = load_proof_module()
    sheets = [proof.monodromy(4, 0, loops) for loops in range(17)]
    assert sheets[:16] == list(range(16))
    assert sheets[16] == 0

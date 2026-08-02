"""Exact regression wrapper for the minimal many-body matter-network proof."""

from __future__ import annotations

import sys
from pathlib import Path


PROOFS = Path(__file__).resolve().parents[1] / "proofs"
sys.path.insert(0, str(PROOFS))

from proof_minimal_many_body_matter_network import run_certificate  # noqa: E402


def test_exact_minimal_many_body_matter_network_certificate() -> None:
    certificate = run_certificate()
    assert not certificate.failures, certificate.failures
    assert certificate.checks >= 45

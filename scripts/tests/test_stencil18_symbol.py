"""Wraps proof_stencil18_symbol_isotropy.py: exact symbol of the engine's 18-point Laplacian."""
from __future__ import annotations
import subprocess, sys
from fractions import Fraction
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "scripts" / "proofs" / "proof_stencil18_symbol_isotropy.py"

def test_proof_script_passes():
    out = subprocess.run([sys.executable, str(SCRIPT)], capture_output=True, text=True, timeout=300)
    assert out.returncode == 0, out.stdout + out.stderr
    assert "PASS: stencil18 symbol" in out.stdout

def test_q6_coefficients_are_the_measured_directions():
    sys.path.insert(0, str(SCRIPT.parent))
    import proof_stencil18_symbol_isotropy as p
    assert p.SYMBOL_Q6 == {"axial": Fraction(-1, 360), "face": Fraction(-1, 240), "body": Fraction(-11, 3240)}

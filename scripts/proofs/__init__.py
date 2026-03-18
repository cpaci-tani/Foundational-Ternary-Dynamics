"""
FTD Ultimate Ontic Chain Proof Suite
=====================================

Self-contained proof package deriving all FTD constants from D=3 + varpi.
Each module proves a specific link in the chain; proof_10 runs everything.

Usage:
    python -m scripts.proofs.proof_10_ultimate_chain      # Full suite
    python -m scripts.proofs.proof_06_gstar_emergence     # Single module
"""

import io
import sys

# Reconfigure stdout/stderr to UTF-8 on Windows to handle Unicode math symbols.
# This runs when any proof module is imported via `python -m scripts.proofs.*`.
if sys.platform == 'win32' and hasattr(sys.stdout, 'buffer'):
    if sys.stdout.encoding and sys.stdout.encoding.lower() != 'utf-8':
        sys.stdout = io.TextIOWrapper(
            sys.stdout.buffer, encoding='utf-8', errors='replace', line_buffering=True
        )
    if sys.stderr.encoding and sys.stderr.encoding.lower() != 'utf-8':
        sys.stderr = io.TextIOWrapper(
            sys.stderr.buffer, encoding='utf-8', errors='replace', line_buffering=True
        )

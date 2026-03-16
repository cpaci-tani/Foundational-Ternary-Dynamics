"""
Foundational Ternary Dynamics - Verification Test Suite
========================================================

This test suite provides comprehensive verification of all FTD framework
predictions for public transparency and reproducibility.

Test Categories:
    1. Framework Integers - The four foundational integers {3, 4, 7, 13}
    2. Master Quadratic - Derivation of alpha from lemniscatic constant
    3. Particle Masses - All Standard Model mass predictions
    4. Coupling Constants - Fine structure, strong, weak, gravitational
    5. Cosmology - Inflation parameters, baryogenesis
    6. Mixing Matrices - CKM and PMNS matrix elements

Usage:
    python -m pytest tests/ -v

    Or run individual test files:
    python tests/test_master_quadratic.py

    Or run all tests with detailed output:
    python tests/run_all_tests.py

Requirements:
    - numpy
    - scipy
    - pytest (optional, for pytest runner)
"""

__version__ = "1.0.0"
__author__ = "FTD Framework"

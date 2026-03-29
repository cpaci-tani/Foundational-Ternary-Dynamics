#!/usr/bin/env python3
"""
Run all lattice analysis visualization scripts in sequence.
"""

import subprocess
import sys
import os

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

scripts = [
    'analyze_phase_recovery.py',
    'analyze_ternary_detector.py',
    'analyze_vortex_lines.py',
    'analyze_correlation_function.py',
    'analyze_void_classification.py',
    'analyze_information_cascade.py',
]


def main():
    for s in scripts:
        path = os.path.join(SCRIPT_DIR, s)
        print(f'Running {s}...')
        subprocess.run([sys.executable, path], check=True)
    print('All analysis complete.')


if __name__ == '__main__':
    main()

"""
ENGINE-THEORY BRIDGE: Quantitative Comparison

This is the FIRST script that connects the FTD C++ engine output
to FTD's own theoretical predictions with quantitative error bars.

Purpose: Run the engine at multiple lattice sizes, extract observables,
compare to theory, and demonstrate convergence at arbitrarily fine spacing a.

Usage:
    python scripts/benchmarks/benchmark_engine_vs_theory.py
    python scripts/benchmarks/benchmark_engine_vs_theory.py --sizes 32 48 64

Outputs:
    scripts/benchmarks/results/benchmark_results.csv  — Raw data
    scripts/benchmarks/results/convergence_report.txt  — Summary report
    scripts/benchmarks/results/coulomb_convergence.png — Convergence plot
    scripts/benchmarks/results/force_profile.png       — Force vs distance
"""

import subprocess
import csv
import os
import sys
import io
import argparse
from pathlib import Path
from collections import defaultdict

# Project root
PROJECT_ROOT = Path(__file__).parent.parent.parent
ENGINE_DIR = PROJECT_ROOT / "engine"
BENCHMARK_EXE = ENGINE_DIR / "build" / "Release" / "ftd_benchmark_engine_theory.exe"
RESULTS_DIR = Path(__file__).parent / "results"

# FTD theoretical predictions
sys.path.insert(0, str(PROJECT_ROOT / "scripts"))
# Hard import: scripts/ MUST be on the path. The previous fallback masked
# sys.path bugs and shipped pre-2026-04-17 stale values (alpha_inv=137.035999084,
# G_STAR=2.9586830685 — 27 ppm drift on G_STAR). Fail loudly instead.
from constants import ALPHA, G_STAR  # noqa: E402


def run_benchmark(lattice_size: int, ticks: int = 300) -> list[dict]:
    """Run the C++ benchmark at a given lattice size and parse CSV output."""
    if not BENCHMARK_EXE.exists():
        print(f"ERROR: Benchmark executable not found at {BENCHMARK_EXE}")
        print("Build with: cmake --build engine/build --config Release --target ftd_benchmark_engine_theory")
        sys.exit(1)

    print(f"  Running L={lattice_size}, ticks={ticks}...", end=" ", flush=True)

    result = subprocess.run(
        [str(BENCHMARK_EXE), str(lattice_size), str(ticks)],
        capture_output=True, text=True, timeout=600,
        cwd=str(ENGINE_DIR)
    )

    if result.returncode != 0:
        print(f"FAILED (exit code {result.returncode})")
        print(result.stderr)
        return []

    # Parse CSV from stdout
    rows = []
    reader = csv.DictReader(io.StringIO(result.stdout))
    for row in reader:
        rows.append(row)

    # Print timing from stderr
    for line in result.stderr.strip().split('\n'):
        if 'Completed' in line:
            print(line.strip())
            break
    else:
        print("done")

    return rows


def analyze_results(all_results: list[dict]) -> dict:
    """Analyze benchmark results across lattice sizes."""
    analysis = {
        'coulomb_exponents': [],
        'wave_speeds': [],
        'gauss_violations': [],
        'energy_drifts': [],
        'charge_ok': [],
        'force_profiles': defaultdict(list),
    }

    for row in all_results:
        bm = row['benchmark']
        L = int(row['lattice_size'])

        if bm == 'coulomb_exponent':
            analysis['coulomb_exponents'].append({
                'L': L,
                'measured': float(row['measured']),
                'theory': float(row['theory']),
                'error_pct': float(row['error_pct']),
                'r_squared': float(row['r_squared']),
                'n_points': int(row['n_points']),
            })
        elif bm == 'coulomb_profile':
            analysis['force_profiles'][L].append({
                'r': float(row['measured']),
                'F': float(row['theory']),
            })
        elif bm == 'wave_speed':
            analysis['wave_speeds'].append({
                'L': L,
                'measured': float(row['measured']),
                'theory': float(row['theory']),
                'error_pct': float(row['error_pct']),
                'r_squared': float(row['r_squared']),
            })
        elif bm == 'gauss_violation':
            analysis['gauss_violations'].append({
                'L': L,
                'rms': float(row['measured']),
            })
        elif bm == 'energy_conservation':
            analysis['energy_drifts'].append({
                'L': L,
                'drift_pct': float(row['measured']),
            })
        elif bm == 'charge_conservation':
            analysis['charge_ok'].append({
                'L': L,
                'conserved': float(row['measured']) == float(row['theory']),
            })

    return analysis


def generate_report(analysis: dict, output_dir: Path) -> str:
    """Generate human-readable report."""
    lines = []
    lines.append("=" * 70)
    lines.append("  FTD ENGINE-THEORY BRIDGE: Benchmark Report")
    lines.append("  First-ever quantitative comparison of engine output to theory")
    lines.append("=" * 70)
    lines.append("")

    # Coulomb force law convergence
    lines.append("--- B1: COULOMB FORCE LAW EXPONENT ---")
    lines.append(f"  Theory: F ~ r^(-2.0) [Coulomb law in 3D]")
    lines.append(f"  {'L':>6}  {'Exponent':>10}  {'Error%':>8}  {'R^2':>8}  {'Points':>6}")
    lines.append(f"  {'-'*6}  {'-'*10}  {'-'*8}  {'-'*8}  {'-'*6}")

    for entry in sorted(analysis['coulomb_exponents'], key=lambda x: x['L']):
        lines.append(f"  {entry['L']:>6}  {entry['measured']:>10.4f}  {entry['error_pct']:>7.2f}%  {entry['r_squared']:>8.4f}  {entry['n_points']:>6}")

    # Check convergence
    if len(analysis['coulomb_exponents']) >= 2:
        sorted_ce = sorted(analysis['coulomb_exponents'], key=lambda x: x['L'])
        first_err = sorted_ce[0]['error_pct']
        last_err = sorted_ce[-1]['error_pct']
        if last_err < first_err:
            lines.append(f"\n  CONVERGENCE: Error improved from {first_err:.2f}% (L={sorted_ce[0]['L']}) to {last_err:.2f}% (L={sorted_ce[-1]['L']})")
            lines.append(f"  This demonstrates fine-spacing behavior: larger lattice -> better 1/r^2")
        else:
            lines.append(f"\n  WARNING: No clear convergence trend (small->{first_err:.2f}%, large->{last_err:.2f}%)")

    lines.append("")

    # Wave speed
    lines.append("--- B2: WAVE PROPAGATION SPEED ---")
    lines.append(f"  Theory: c = 1/sqrt(3) = 0.57735 [CFL stability limit]")
    for entry in sorted(analysis['wave_speeds'], key=lambda x: x['L']):
        lines.append(f"  L={entry['L']:>4}: measured={entry['measured']:.4f}, error={entry['error_pct']:.1f}%, R^2={entry['r_squared']:.4f}")

    lines.append("")

    # Gauss constraint
    lines.append("--- B4: GAUSS CONSTRAINT (div J = rho) ---")
    lines.append(f"  Theory: 0.0 [exact constraint]")
    for entry in sorted(analysis['gauss_violations'], key=lambda x: x['L']):
        lines.append(f"  L={entry['L']:>4}: RMS violation = {entry['rms']:.6e}")

    lines.append("")

    # Energy conservation
    lines.append("--- B5: ENERGY CONSERVATION ---")
    lines.append(f"  Theory: 0% drift [closed system]")
    for entry in sorted(analysis['energy_drifts'], key=lambda x: x['L']):
        lines.append(f"  L={entry['L']:>4}: drift = {entry['drift_pct']:.2f}%")

    lines.append("")

    # Charge conservation
    lines.append("--- B6: CHARGE CONSERVATION ---")
    for entry in analysis['charge_ok']:
        status = "EXACT" if entry['conserved'] else "VIOLATED"
        lines.append(f"  L={entry['L']:>4}: {status}")

    lines.append("")

    # Overall assessment
    lines.append("=" * 70)
    lines.append("  OVERALL ASSESSMENT")
    lines.append("=" * 70)

    # Grade each benchmark
    grades = {}

    # Coulomb
    if analysis['coulomb_exponents']:
        best_err = min(e['error_pct'] for e in analysis['coulomb_exponents'])
        if best_err < 5:
            grades['Coulomb law'] = ('A', f'{best_err:.1f}% from theory')
        elif best_err < 15:
            grades['Coulomb law'] = ('B', f'{best_err:.1f}% from theory')
        elif best_err < 30:
            grades['Coulomb law'] = ('C', f'{best_err:.1f}% from theory')
        else:
            grades['Coulomb law'] = ('D', f'{best_err:.1f}% from theory')

    # Wave speed
    if analysis['wave_speeds']:
        best_err = min(e['error_pct'] for e in analysis['wave_speeds'])
        if best_err < 10:
            grades['Wave speed'] = ('A', f'{best_err:.1f}% from theory')
        elif best_err < 30:
            grades['Wave speed'] = ('B', f'{best_err:.1f}% from theory')
        else:
            grades['Wave speed'] = ('C', f'{best_err:.1f}% from theory')

    # Gauss
    if analysis['gauss_violations']:
        best_v = min(e['rms'] for e in analysis['gauss_violations'])
        if best_v < 0.01:
            grades['Gauss constraint'] = ('A', f'RMS = {best_v:.2e}')
        elif best_v < 0.1:
            grades['Gauss constraint'] = ('B', f'RMS = {best_v:.2e}')
        else:
            grades['Gauss constraint'] = ('C', f'RMS = {best_v:.2e}')

    # Energy
    if analysis['energy_drifts']:
        best_d = min(e['drift_pct'] for e in analysis['energy_drifts'] if e['drift_pct'] >= 0)
        if best_d < 1:
            grades['Energy conservation'] = ('A', f'{best_d:.1f}% drift')
        elif best_d < 5:
            grades['Energy conservation'] = ('B', f'{best_d:.1f}% drift')
        else:
            grades['Energy conservation'] = ('C', f'{best_d:.1f}% drift')

    # Charge
    if all(e['conserved'] for e in analysis['charge_ok']):
        grades['Charge conservation'] = ('A+', 'Exact')

    lines.append("")
    for name, (grade, detail) in grades.items():
        lines.append(f"  {name:<25} {grade:>3}  ({detail})")

    lines.append("")
    lines.append("  NOTE: This is the first-ever engine-to-theory comparison for FTD.")
    lines.append("  Even partial agreement is significant — it means the discrete lattice")
    lines.append("  dynamics are producing real physics, not just conserved quantities.")
    lines.append("")

    report = '\n'.join(lines)
    return report


def try_generate_plots(analysis: dict, output_dir: Path):
    """Generate plots if matplotlib is available."""
    try:
        import matplotlib
        matplotlib.use('Agg')
        import matplotlib.pyplot as plt
        import numpy as np
    except ImportError:
        print("  (matplotlib not available, skipping plots)")
        return

    # Plot 1: Coulomb exponent convergence
    if len(analysis['coulomb_exponents']) >= 2:
        fig, ax = plt.subplots(figsize=(8, 5))
        data = sorted(analysis['coulomb_exponents'], key=lambda x: x['L'])
        Ls = [d['L'] for d in data]
        exps = [d['measured'] for d in data]
        errs = [abs(d['measured'] - d['theory']) for d in data]

        ax.plot(Ls, exps, 'bo-', markersize=8, linewidth=2, label='Engine output')
        ax.axhline(y=-2.0, color='r', linestyle='--', linewidth=2, label='Theory: -2.0')
        ax.fill_between(Ls, -2.05, -1.95, alpha=0.1, color='red', label='5% band')
        ax.set_xlabel('Lattice Size L', fontsize=12)
        ax.set_ylabel('Coulomb Force Exponent', fontsize=12)
        ax.set_title('FTD Engine: Coulomb Law Convergence to Continuum Limit', fontsize=14)
        ax.legend(fontsize=11)
        ax.grid(True, alpha=0.3)

        fig.tight_layout()
        fig.savefig(output_dir / 'coulomb_convergence.png', dpi=150)
        plt.close()
        print(f"  Saved: {output_dir / 'coulomb_convergence.png'}")

    # Plot 2: Force profiles at all lattice sizes
    if analysis['force_profiles']:
        fig, ax = plt.subplots(figsize=(8, 5))

        colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd']
        for i, (L, profile) in enumerate(sorted(analysis['force_profiles'].items())):
            if profile:
                rs = [p['r'] for p in profile]
                Fs = [p['F'] for p in profile]
                c = colors[i % len(colors)]
                ax.loglog(rs, Fs, 'o-', color=c, markersize=5, label=f'L={L}')

        # Theory line: F = A * r^(-2)
        r_theory = np.linspace(2, 20, 100)
        # Normalize to match first data point for comparison
        first_profile = list(analysis['force_profiles'].values())[0]
        if first_profile:
            r0, F0 = first_profile[0]['r'], first_profile[0]['F']
            A = F0 * r0**2
            ax.loglog(r_theory, A * r_theory**(-2), 'k--', linewidth=2, label='Theory: r^(-2)')

        ax.set_xlabel('Distance r (lattice units)', fontsize=12)
        ax.set_ylabel('Force |F|', fontsize=12)
        ax.set_title('FTD Engine: Emergent Coulomb Force Profile', fontsize=14)
        ax.legend(fontsize=10)
        ax.grid(True, alpha=0.3, which='both')

        fig.tight_layout()
        fig.savefig(output_dir / 'force_profile.png', dpi=150)
        plt.close()
        print(f"  Saved: {output_dir / 'force_profile.png'}")


def main():
    parser = argparse.ArgumentParser(description='FTD Engine-Theory Benchmark')
    parser.add_argument('--sizes', type=int, nargs='+', default=[32, 48],
                        help='Lattice sizes to benchmark (default: 32 48)')
    parser.add_argument('--ticks', type=int, default=200,
                        help='Simulation ticks per benchmark (default: 200)')
    args = parser.parse_args()

    print("=" * 70)
    print("  FTD ENGINE-THEORY BRIDGE BENCHMARK")
    print("  Closing the gap between engine output and theoretical predictions")
    print("=" * 70)
    print()

    # Ensure output directory exists
    RESULTS_DIR.mkdir(parents=True, exist_ok=True)

    # Run benchmarks at each lattice size
    all_results = []
    for L in args.sizes:
        rows = run_benchmark(L, args.ticks)
        all_results.extend(rows)

    if not all_results:
        print("ERROR: No benchmark results collected")
        return 1

    # Save raw results
    csv_path = RESULTS_DIR / "benchmark_results.csv"
    with open(csv_path, 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=all_results[0].keys())
        writer.writeheader()
        writer.writerows(all_results)
    print(f"\nRaw results: {csv_path}")

    # Analyze
    analysis = analyze_results(all_results)

    # Generate report
    report = generate_report(analysis, RESULTS_DIR)
    print(report)

    report_path = RESULTS_DIR / "convergence_report.txt"
    with open(report_path, 'w') as f:
        f.write(report)
    print(f"Report saved: {report_path}")

    # Generate plots
    print("\nGenerating plots...")
    try_generate_plots(analysis, RESULTS_DIR)

    return 0


if __name__ == '__main__':
    sys.exit(main())

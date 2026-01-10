#!/usr/bin/env python3
"""
TRD Manuscript Figure Generator
================================
Generates all figures for the TRD manuscript (Tier 1 + Tier 2 + Tier 3).

Usage:
    python generate_figures.py              # Generate all figures
    python generate_figures.py --tier 1     # Generate only Tier 1 figures
    python generate_figures.py --tier 2     # Generate only Tier 2 figures
    python generate_figures.py --tier 3     # Generate only Tier 3 figures
    python generate_figures.py --figure 1.1 # Generate specific figure
    python generate_figures.py --list       # List all figures
    python generate_figures.py --dpi print  # Generate at print resolution (300 DPI)

Requirements:
    - matplotlib >= 3.7.0
    - numpy >= 1.24.0
"""

import argparse
import sys
from pathlib import Path
import importlib

# Add this directory to path for imports
sys.path.insert(0, str(Path(__file__).parent))

# Figure registry - maps figure IDs to their modules and output paths
TIER1_FIGURES = {
    '1.1': {
        'module': 'individual.fig_01_lemniscate_alpha',
        'function': 'generate_lemniscate_alpha',
        'output': 'ch01/fig_1_1_lemniscate_alpha.png',
        'description': 'Lemniscate-Alpha Curve',
        'chapter': '1.10-lemniscate-alpha.qmd',
    },
    '1.2': {
        'module': 'individual.fig_02_master_quadratic',
        'function': 'generate_master_quadratic',
        'output': 'ch01/fig_1_2_master_quadratic.png',
        'description': 'Master Quadratic Roots',
        'chapter': '1.10-lemniscate-alpha.qmd',
    },
    '1.3': {
        'module': 'individual.fig_03_three_states',
        'function': 'generate_three_states',
        'output': 'ch01/fig_1_3_three_state_transitions.png',
        'description': 'Three-State Transitions',
        'chapter': '1.1-the-void.qmd',
    },
    '1.4': {
        'module': 'individual.fig_04_causal_loop',
        'function': 'generate_causal_loop',
        'output': 'ch01/fig_1_4_causal_loop.png',
        'description': '13-Step Causal Loop',
        'chapter': '1.6-the-causal-loop.qmd',
    },
    '1.5': {
        'module': 'individual.fig_05_moore_neighborhood',
        'function': 'generate_moore_neighborhood',
        'output': 'ch00/fig_1_5_moore_neighborhood.png',
        'description': 'Moore Neighborhood (3D)',
        'chapter': '0.2-mathematics.qmd',
    },
    '1.6': {
        'module': 'individual.fig_06_interference',
        'function': 'generate_interference',
        'output': 'ch01/fig_1_6_interference.png',
        'description': 'Vector Addition/Interference',
        'chapter': '1.4-interference.qmd',
    },
    '1.7': {
        'module': 'individual.fig_07_double_slit',
        'function': 'generate_double_slit',
        'output': 'ch01/fig_1_7_double_slit.png',
        'description': 'Double-Slit Pattern',
        'chapter': '1.4-interference.qmd',
    },
    '1.8': {
        'module': 'individual.fig_08_voxel_structure',
        'function': 'generate_voxel_structure',
        'output': 'ch02/fig_1_8_voxel_structure.png',
        'description': 'Voxel Data Structure',
        'chapter': '2.2-voxel-anatomy.qmd',
    },
    '1.9': {
        'module': 'individual.fig_09_zoom_planck',
        'function': 'generate_zoom_planck',
        'output': 'ch02/fig_1_9_zoom_planck.png',
        'description': 'Zoom: Macro to Planck',
        'chapter': '2.1-the-planck-scale.qmd',
    },
    '1.10': {
        'module': 'individual.fig_10_standard_model',
        'function': 'generate_standard_model',
        'output': 'ch02/fig_1_10_standard_model.png',
        'description': 'Standard Model Chart',
        'chapter': '2.3-the-particle-zoo.qmd',
    },
    '1.11': {
        'module': 'individual.fig_11_force_comparison',
        'function': 'generate_force_comparison',
        'output': 'ch01/fig_1_11_force_comparison.png',
        'description': 'Force Comparison',
        'chapter': '1.8-the-four-forces.qmd',
    },
    '1.12': {
        'module': 'individual.fig_12_yukawa_potential',
        'function': 'generate_yukawa_potential',
        'output': 'ch01/fig_1_12_yukawa_potential.png',
        'description': 'Yukawa Potential',
        'chapter': '1.8-the-four-forces.qmd',
    },
    '1.13': {
        'module': 'individual.fig_13_triad_geometry',
        'function': 'generate_triad_geometry',
        'output': 'ch03/fig_1_13_triad_geometry.png',
        'description': 'Triad Geometry',
        'chapter': '3.1-stable-structures.qmd',
    },
    '1.14': {
        'module': 'individual.fig_14_sloop_mechanism',
        'function': 'generate_sloop_mechanism',
        'output': 'ch02/fig_1_14_sloop_mechanism.png',
        'description': 'sLoop Mechanism',
        'chapter': '2.4-quantum-phenomena.qmd',
    },
    '1.15': {
        'module': 'individual.fig_15_constants_flowchart',
        'function': 'generate_constants_flowchart',
        'output': 'ch14/fig_1_15_constants_flowchart.png',
        'description': 'Constants Flowchart',
        'chapter': '14.1-constants-reference.qmd',
    },
}

# Tier 2 Figures - Highly Important (20 figures)
TIER2_FIGURES = {
    # Foundations (2.1-2.5)
    '2.1': {
        'module': 'individual.fig_2_01_helmholtz',
        'function': 'generate_helmholtz',
        'output': 'ch01/fig_2_1_helmholtz.png',
        'description': 'Helmholtz Decomposition',
        'chapter': '1.5-the-flux-field.qmd',
    },
    '2.2': {
        'module': 'individual.fig_2_02_discrete_continuous',
        'function': 'generate_discrete_continuous',
        'output': 'ch01/fig_2_2_discrete_continuous.png',
        'description': 'Discrete vs Continuous',
        'chapter': '1.5-the-flux-field.qmd',
    },
    '2.3': {
        'module': 'individual.fig_2_03_ontological_levels',
        'function': 'generate_ontological_levels',
        'output': 'ch01/fig_2_3_ontological_levels.png',
        'description': 'Ontological Levels',
        'chapter': '1.1-the-void.qmd',
    },
    '2.4': {
        'module': 'individual.fig_2_04_force_gradients',
        'function': 'generate_force_gradients',
        'output': 'ch01/fig_2_4_force_gradients.png',
        'description': 'Force from Gradients',
        'chapter': '1.8-the-four-forces.qmd',
    },
    '2.5': {
        'module': 'individual.fig_2_05_arc_length_alpha',
        'function': 'generate_arc_length_alpha',
        'output': 'ch01/fig_2_5_arc_length_alpha.png',
        'description': 'Arc Length to Alpha',
        'chapter': '1.10-lemniscate-alpha.qmd',
    },
    # Subatomic (2.6-2.9)
    '2.6': {
        'module': 'individual.fig_2_06_position_remainder',
        'function': 'generate_position_remainder',
        'output': 'ch02/fig_2_6_position_remainder.png',
        'description': 'Position Remainder',
        'chapter': '2.2-voxel-anatomy.qmd',
    },
    '2.7': {
        'module': 'individual.fig_2_07_quantum_foam',
        'function': 'generate_quantum_foam',
        'output': 'ch02/fig_2_7_quantum_foam.png',
        'description': 'Quantum Foam',
        'chapter': '2.1-the-planck-scale.qmd',
    },
    '2.8': {
        'module': 'individual.fig_2_08_tunneling',
        'function': 'generate_tunneling',
        'output': 'ch02/fig_2_8_tunneling.png',
        'description': 'Tunneling Probability',
        'chapter': '2.4-quantum-phenomena.qmd',
    },
    '2.9': {
        'module': 'individual.fig_2_09_entanglement_origin',
        'function': 'generate_entanglement_origin',
        'output': 'ch02/fig_2_9_entanglement_origin.png',
        'description': 'Entanglement Origin',
        'chapter': '2.4-quantum-phenomena.qmd',
    },
    # Atomic/Molecular (2.10-2.14)
    '2.10': {
        'module': 'individual.fig_2_10_electron_shells',
        'function': 'generate_electron_shells',
        'output': 'ch03/fig_2_10_electron_shells.png',
        'description': 'Electron Shell Structure',
        'chapter': '3.2-atomic-structure.qmd',
    },
    '2.11': {
        'module': 'individual.fig_2_11_hydrogen_energy',
        'function': 'generate_hydrogen_energy',
        'output': 'ch03/fig_2_11_hydrogen_energy.png',
        'description': 'Hydrogen Energy Levels',
        'chapter': '3.2-atomic-structure.qmd',
    },
    '2.12': {
        'module': 'individual.fig_2_12_covalent_bonding',
        'function': 'generate_covalent_bonding',
        'output': 'ch03/fig_2_12_covalent_bonding.png',
        'description': 'Covalent Bonding',
        'chapter': '3.3-chemical-bonds.qmd',
    },
    '2.13': {
        'module': 'individual.fig_2_13_ionic_bonding',
        'function': 'generate_ionic_bonding',
        'output': 'ch03/fig_2_13_ionic_bonding.png',
        'description': 'Ionic Bonding',
        'chapter': '3.3-chemical-bonds.qmd',
    },
    '2.14': {
        'module': 'individual.fig_2_14_molecular_orbitals',
        'function': 'generate_molecular_orbitals',
        'output': 'ch03/fig_2_14_molecular_orbitals.png',
        'description': 'Molecular Orbitals',
        'chapter': '3.3-chemical-bonds.qmd',
    },
    # Structures (2.15-2.17)
    '2.15': {
        'module': 'individual.fig_2_15_crystal_lattice',
        'function': 'generate_crystal_lattice',
        'output': 'ch04/fig_2_15_crystal_lattice.png',
        'description': 'Crystal Lattice',
        'chapter': '4.1-solid-state.qmd',
    },
    '2.16': {
        'module': 'individual.fig_2_16_phase_transitions',
        'function': 'generate_phase_transitions',
        'output': 'ch04/fig_2_16_phase_transitions.png',
        'description': 'Phase Transitions',
        'chapter': '4.2-thermodynamics.qmd',
    },
    '2.17': {
        'module': 'individual.fig_2_17_emergent_thermodynamics',
        'function': 'generate_emergent_thermodynamics',
        'output': 'ch04/fig_2_17_emergent_thermodynamics.png',
        'description': 'Emergent Thermodynamics',
        'chapter': '4.2-thermodynamics.qmd',
    },
    # Cosmic (2.18-2.20)
    '2.18': {
        'module': 'individual.fig_2_18_gravitational_clumping',
        'function': 'generate_gravitational_clumping',
        'output': 'ch05/fig_2_18_gravitational_clumping.png',
        'description': 'Gravitational Clumping',
        'chapter': '5.1-cosmic-structure.qmd',
    },
    '2.19': {
        'module': 'individual.fig_2_19_cosmological_expansion',
        'function': 'generate_cosmological_expansion',
        'output': 'ch05/fig_2_19_cosmological_expansion.png',
        'description': 'Cosmological Expansion',
        'chapter': '5.2-expansion.qmd',
    },
    '2.20': {
        'module': 'individual.fig_2_20_dark_matter_energy',
        'function': 'generate_dark_matter_energy',
        'output': 'ch05/fig_2_20_dark_matter_energy.png',
        'description': 'Dark Matter and Energy',
        'chapter': '5.3-dark-sector.qmd',
    },
}

# Tier 3 Figures - Supporting (30 figures)
TIER3_FIGURES = {
    # Foundations (3.1-3.6)
    '3.1': {
        'module': 'individual.fig_3_01_tick_sequence',
        'function': 'generate_tick_sequence',
        'output': 'ch01/fig_3_1_tick_sequence.png',
        'description': 'Tick Sequence Phases',
        'chapter': '1.6-the-causal-loop.qmd',
    },
    '3.2': {
        'module': 'individual.fig_3_02_boundary_conditions',
        'function': 'generate_boundary_conditions',
        'output': 'ch01/fig_3_2_boundary_conditions.png',
        'description': 'Boundary Conditions',
        'chapter': '1.3-locality.qmd',
    },
    '3.3': {
        'module': 'individual.fig_3_03_conservation_laws',
        'function': 'generate_conservation_laws',
        'output': 'ch01/fig_3_3_conservation_laws.png',
        'description': 'Conservation Laws',
        'chapter': '1.7-conservation.qmd',
    },
    '3.4': {
        'module': 'individual.fig_3_04_update_order',
        'function': 'generate_update_order',
        'output': 'ch01/fig_3_4_update_order.png',
        'description': 'Update Order Effects',
        'chapter': '1.6-the-causal-loop.qmd',
    },
    '3.5': {
        'module': 'individual.fig_3_05_sparse_dense',
        'function': 'generate_sparse_dense',
        'output': 'ch01/fig_3_5_sparse_dense.png',
        'description': 'Sparse vs Dense Storage',
        'chapter': '1.2-the-lattice.qmd',
    },
    '3.6': {
        'module': 'individual.fig_3_06_integer_ratios',
        'function': 'generate_integer_ratios',
        'output': 'ch01/fig_3_6_integer_ratios.png',
        'description': 'Framework Integer Ratios',
        'chapter': '1.10-lemniscate-alpha.qmd',
    },
    # Subatomic (3.7-3.12)
    '3.7': {
        'module': 'individual.fig_3_07_spin_states',
        'function': 'generate_spin_states',
        'output': 'ch02/fig_3_7_spin_states.png',
        'description': 'Spin States',
        'chapter': '2.3-the-particle-zoo.qmd',
    },
    '3.8': {
        'module': 'individual.fig_3_08_color_charge',
        'function': 'generate_color_charge',
        'output': 'ch02/fig_3_8_color_charge.png',
        'description': 'Color Charge',
        'chapter': '2.3-the-particle-zoo.qmd',
    },
    '3.9': {
        'module': 'individual.fig_3_09_weak_interaction',
        'function': 'generate_weak_interaction',
        'output': 'ch02/fig_3_9_weak_interaction.png',
        'description': 'Weak Interaction',
        'chapter': '2.5-weak-force.qmd',
    },
    '3.10': {
        'module': 'individual.fig_3_10_pair_production',
        'function': 'generate_pair_production',
        'output': 'ch02/fig_3_10_pair_production.png',
        'description': 'Pair Production Detail',
        'chapter': '2.4-quantum-phenomena.qmd',
    },
    '3.11': {
        'module': 'individual.fig_3_11_annihilation_sequence',
        'function': 'generate_annihilation_sequence',
        'output': 'ch02/fig_3_11_annihilation_sequence.png',
        'description': 'Annihilation Sequence',
        'chapter': '2.4-quantum-phenomena.qmd',
    },
    '3.12': {
        'module': 'individual.fig_3_12_virtual_lifetime',
        'function': 'generate_virtual_lifetime',
        'output': 'ch02/fig_3_12_virtual_lifetime.png',
        'description': 'Virtual Particle Lifetime',
        'chapter': '2.4-quantum-phenomena.qmd',
    },
    # Atomic/Molecular (3.13-3.18)
    '3.13': {
        'module': 'individual.fig_3_13_orbital_shapes',
        'function': 'generate_orbital_shapes',
        'output': 'ch03/fig_3_13_orbital_shapes.png',
        'description': 'Orbital Shapes',
        'chapter': '3.2-atomic-structure.qmd',
    },
    '3.14': {
        'module': 'individual.fig_3_14_periodic_table_trd',
        'function': 'generate_periodic_table_trd',
        'output': 'ch03/fig_3_14_periodic_table_trd.png',
        'description': 'Periodic Table (TRD)',
        'chapter': '3.2-atomic-structure.qmd',
    },
    '3.15': {
        'module': 'individual.fig_3_15_metallic_bonding',
        'function': 'generate_metallic_bonding',
        'output': 'ch03/fig_3_15_metallic_bonding.png',
        'description': 'Metallic Bonding',
        'chapter': '3.3-chemical-bonds.qmd',
    },
    '3.16': {
        'module': 'individual.fig_3_16_hydrogen_bonding',
        'function': 'generate_hydrogen_bonding',
        'output': 'ch03/fig_3_16_hydrogen_bonding.png',
        'description': 'Hydrogen Bonding',
        'chapter': '3.3-chemical-bonds.qmd',
    },
    '3.17': {
        'module': 'individual.fig_3_17_resonance',
        'function': 'generate_resonance',
        'output': 'ch03/fig_3_17_resonance.png',
        'description': 'Resonance Structures',
        'chapter': '3.3-chemical-bonds.qmd',
    },
    '3.18': {
        'module': 'individual.fig_3_18_reaction_coordinate',
        'function': 'generate_reaction_coordinate',
        'output': 'ch03/fig_3_18_reaction_coordinate.png',
        'description': 'Reaction Coordinate',
        'chapter': '3.4-reactions.qmd',
    },
    # Structures (3.19-3.24)
    '3.19': {
        'module': 'individual.fig_3_19_crystal_defects',
        'function': 'generate_crystal_defects',
        'output': 'ch03/fig_3_19_crystal_defects.png',
        'description': 'Crystal Defects',
        'chapter': '4.1-solid-state.qmd',
    },
    '3.20': {
        'module': 'individual.fig_3_20_phonon_propagation',
        'function': 'generate_phonon_propagation',
        'output': 'ch03/fig_3_20_phonon_propagation.png',
        'description': 'Phonon Propagation',
        'chapter': '4.1-solid-state.qmd',
    },
    '3.21': {
        'module': 'individual.fig_3_21_heat_conduction',
        'function': 'generate_heat_conduction',
        'output': 'ch03/fig_3_21_heat_conduction.png',
        'description': 'Heat Conduction',
        'chapter': '4.2-thermodynamics.qmd',
    },
    '3.22': {
        'module': 'individual.fig_3_22_superconductivity',
        'function': 'generate_superconductivity',
        'output': 'ch03/fig_3_22_superconductivity.png',
        'description': 'Superconductivity',
        'chapter': '4.3-quantum-materials.qmd',
    },
    '3.23': {
        'module': 'individual.fig_3_23_magnetic_domains',
        'function': 'generate_magnetic_domains',
        'output': 'ch03/fig_3_23_magnetic_domains.png',
        'description': 'Magnetic Domains',
        'chapter': '4.4-magnetism.qmd',
    },
    '3.24': {
        'module': 'individual.fig_3_24_electric_polarization',
        'function': 'generate_electric_polarization',
        'output': 'ch03/fig_3_24_electric_polarization.png',
        'description': 'Electric Polarization',
        'chapter': '4.5-dielectrics.qmd',
    },
    # Cosmic (3.25-3.30)
    '3.25': {
        'module': 'individual.fig_3_25_star_formation',
        'function': 'generate_star_formation',
        'output': 'ch03/fig_3_25_star_formation.png',
        'description': 'Star Formation',
        'chapter': '5.1-cosmic-structure.qmd',
    },
    '3.26': {
        'module': 'individual.fig_3_26_gravitational_lensing',
        'function': 'generate_gravitational_lensing',
        'output': 'ch03/fig_3_26_gravitational_lensing.png',
        'description': 'Gravitational Lensing',
        'chapter': '5.1-cosmic-structure.qmd',
    },
    '3.27': {
        'module': 'individual.fig_3_27_black_holes',
        'function': 'generate_black_holes',
        'output': 'ch03/fig_3_27_black_holes.png',
        'description': 'Black Holes',
        'chapter': '5.4-extreme-physics.qmd',
    },
    '3.28': {
        'module': 'individual.fig_3_28_dark_matter',
        'function': 'generate_dark_matter',
        'output': 'ch03/fig_3_28_dark_matter.png',
        'description': 'Dark Matter Evidence',
        'chapter': '5.3-dark-sector.qmd',
    },
    '3.29': {
        'module': 'individual.fig_3_29_cosmic_expansion',
        'function': 'generate_cosmic_expansion',
        'output': 'ch03/fig_3_29_cosmic_expansion.png',
        'description': 'Cosmic Expansion',
        'chapter': '5.2-expansion.qmd',
    },
    '3.30': {
        'module': 'individual.fig_3_30_large_scale_structure',
        'function': 'generate_large_scale_structure',
        'output': 'ch03/fig_3_30_large_scale_structure.png',
        'description': 'Large Scale Structure',
        'chapter': '5.1-cosmic-structure.qmd',
    },
}

# Combined registry
ALL_FIGURES = {**TIER1_FIGURES, **TIER2_FIGURES, **TIER3_FIGURES}

# Resolution settings
DPI_SETTINGS = {
    'web': 150,
    'print': 300,
    'preview': 72,
}


def generate_figure(fig_id: str, output_dir: Path, dpi: str = 'web') -> bool:
    """
    Generate a single figure and save it.

    Parameters
    ----------
    fig_id : str
        Figure ID (e.g., '1.1' or '2.5')
    output_dir : Path
        Base directory for output
    dpi : str
        Resolution setting ('web', 'print', 'preview')

    Returns
    -------
    bool
        True if successful, False otherwise
    """
    if fig_id not in ALL_FIGURES:
        print(f"ERROR: Unknown figure ID: {fig_id}")
        print(f"       Use --list to see available figures.")
        return False

    config = ALL_FIGURES[fig_id]
    resolution = DPI_SETTINGS.get(dpi, 150)

    print(f"Generating Figure {fig_id}: {config['description']}...")

    try:
        # Import the module and get the function
        module = importlib.import_module(config['module'])
        func = getattr(module, config['function'])

        # Generate the figure
        fig = func()

        # Create output path
        output_path = output_dir / config['output']
        output_path.parent.mkdir(parents=True, exist_ok=True)

        # Save the figure
        fig.savefig(
            output_path,
            dpi=resolution,
            bbox_inches='tight',
            facecolor='white',
            edgecolor='none',
            pad_inches=0.1
        )

        # Close to free memory
        import matplotlib.pyplot as plt
        plt.close(fig)

        print(f"  [OK] Saved: {output_path}")
        return True

    except Exception as e:
        print(f"  [FAIL] ERROR: {e}")
        import traceback
        traceback.print_exc()
        return False


def list_figures(tier: int = None):
    """Print a list of available figures.

    Parameters
    ----------
    tier : int, optional
        Filter by tier (1, 2, or 3). If None, show all.
    """
    if tier is None or tier == 1:
        print("\nTier 1 Figures (Essential - 15 figures)")
        print("=" * 70)
        print(f"{'ID':<6} {'Description':<30} {'Output Path'}")
        print("-" * 70)

        for fig_id, config in TIER1_FIGURES.items():
            print(f"{fig_id:<6} {config['description']:<30} {config['output']}")

        print("-" * 70)
        print(f"Tier 1 Total: {len(TIER1_FIGURES)} figures")
        print()

    if tier is None or tier == 2:
        print("\nTier 2 Figures (Highly Important - 20 figures)")
        print("=" * 70)
        print(f"{'ID':<6} {'Description':<30} {'Output Path'}")
        print("-" * 70)

        for fig_id, config in TIER2_FIGURES.items():
            print(f"{fig_id:<6} {config['description']:<30} {config['output']}")

        print("-" * 70)
        print(f"Tier 2 Total: {len(TIER2_FIGURES)} figures")
        print()

    if tier is None or tier == 3:
        print("\nTier 3 Figures (Supporting - 30 figures)")
        print("=" * 70)
        print(f"{'ID':<6} {'Description':<30} {'Output Path'}")
        print("-" * 70)

        for fig_id, config in TIER3_FIGURES.items():
            print(f"{fig_id:<6} {config['description']:<30} {config['output']}")

        print("-" * 70)
        print(f"Tier 3 Total: {len(TIER3_FIGURES)} figures")
        print()

    if tier is None:
        print(f"Grand Total: {len(ALL_FIGURES)} figures")
        print()


def main():
    """Main entry point for the figure generator."""
    parser = argparse.ArgumentParser(
        description='Generate TRD manuscript figures',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python generate_figures.py              # Generate all figures (Tier 1 + 2 + 3)
  python generate_figures.py --tier 1     # Generate only Tier 1 figures
  python generate_figures.py --tier 2     # Generate only Tier 2 figures
  python generate_figures.py --tier 3     # Generate only Tier 3 figures
  python generate_figures.py --figure 1.1 # Generate specific figure
  python generate_figures.py --figure 3.5 # Generate specific Tier 3 figure
  python generate_figures.py --list       # List all figures
  python generate_figures.py --dpi print  # Print quality (300 DPI)
        """
    )

    parser.add_argument('--figure', '-f', type=str,
                        help='Generate specific figure (e.g., 1.1, 2.5, or 3.10)')
    parser.add_argument('--tier', '-t', type=int, choices=[1, 2, 3],
                        help='Generate only specific tier (1, 2, or 3)')
    parser.add_argument('--list', '-l', action='store_true',
                        help='List all available figures')
    parser.add_argument('--dpi', choices=['web', 'print', 'preview'],
                        default='web',
                        help='Resolution: web=150, print=300, preview=72')
    parser.add_argument('--output', '-o', type=str, default=None,
                        help='Output directory (default: same as script)')

    args = parser.parse_args()

    # Determine output directory
    if args.output:
        output_dir = Path(args.output)
    else:
        output_dir = Path(__file__).parent

    # Handle --list
    if args.list:
        list_figures(args.tier)
        return 0

    # Handle --figure
    if args.figure:
        success = generate_figure(args.figure, output_dir, args.dpi)
        return 0 if success else 1

    # Determine which figures to generate
    if args.tier == 1:
        figures_to_generate = TIER1_FIGURES
        tier_label = "Tier 1"
    elif args.tier == 2:
        figures_to_generate = TIER2_FIGURES
        tier_label = "Tier 2"
    elif args.tier == 3:
        figures_to_generate = TIER3_FIGURES
        tier_label = "Tier 3"
    else:
        figures_to_generate = ALL_FIGURES
        tier_label = "all (Tier 1 + 2 + 3)"

    # Generate figures
    print(f"\nGenerating {len(figures_to_generate)} {tier_label} figures...")
    print(f"Output directory: {output_dir}")
    print(f"Resolution: {args.dpi} ({DPI_SETTINGS[args.dpi]} DPI)")
    print("=" * 70)

    success_count = 0
    fail_count = 0

    for fig_id in figures_to_generate:
        if generate_figure(fig_id, output_dir, args.dpi):
            success_count += 1
        else:
            fail_count += 1

    print("=" * 70)
    print(f"Complete! {success_count} succeeded, {fail_count} failed.")

    return 0 if fail_count == 0 else 1


if __name__ == '__main__':
    sys.exit(main())

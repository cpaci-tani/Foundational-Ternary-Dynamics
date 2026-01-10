"""
Batch Render Script for TRD Animations
======================================

Renders all chapter animations in sequence.
"""

import subprocess
import sys
from pathlib import Path

# Quality settings
QUALITY = "-qh"  # -ql (low/480p), -qm (medium/720p), -qh (high/1080p), -qk (4k)

# Chapter files to render
CHAPTERS = [
    # Book 1: Foundations
    "chapters/book_01_foundations/ch_1_1_void.py",
    "chapters/book_01_foundations/ch_1_2_first_division.py",
    "chapters/book_01_foundations/ch_1_3_two_layers.py",
    "chapters/book_01_foundations/ch_1_4_interference.py",
    "chapters/book_01_foundations/ch_1_5_cycle.py",
    "chapters/book_01_foundations/ch_1_6_causal_loop.py",
    "chapters/book_01_foundations/ch_1_7_time_causality.py",
    "chapters/book_01_foundations/ch_1_8_four_forces.py",
    "chapters/book_01_foundations/ch_1_9_constants.py",
    # Book 2: Subatomic
    "chapters/book_02_subatomic/ch_2_1_planck_scale.py",
    "chapters/book_02_subatomic/ch_2_2_voxel_anatomy.py",
    "chapters/book_02_subatomic/ch_2_3_particle_zoo.py",
    "chapters/book_02_subatomic/ch_2_4_quantum_phenomena.py",
    # Book 3: Atomic
    "chapters/book_03_atomic/ch_3_1_stable_structures.py",
    "chapters/book_03_atomic/ch_3_2_periodic_table.py",
    "chapters/book_03_atomic/ch_3_3_electron_dynamics.py",
    "chapters/book_03_atomic/ch_3_4_nuclear_physics.py",
    # Book 4: Molecular
    "chapters/book_04_molecular/ch_4_1_chemical_bonds.py",
    "chapters/book_04_molecular/ch_4_2_simple_molecules.py",
    "chapters/book_04_molecular/ch_4_3_complex_molecules.py",
    "chapters/book_04_molecular/ch_4_4_macromolecules.py",
    # Book 5: States
    "chapters/book_05_states/ch_5_1_phases.py",
    "chapters/book_05_states/ch_5_2_phase_transitions.py",
    "chapters/book_05_states/ch_5_3_thermodynamics.py",
    # Book 6: Materials
    "chapters/book_06_materials/ch_6_1_crystals.py",
    "chapters/book_06_materials/ch_6_2_metals.py",
    "chapters/book_06_materials/ch_6_3_polymers.py",
    "chapters/book_06_materials/ch_6_4_composites.py",
    # Book 7: Planetary
    "chapters/book_07_planetary/ch_7_1_formation.py",
    "chapters/book_07_planetary/ch_7_2_structure.py",
    "chapters/book_07_planetary/ch_7_3_atmospheres.py",
    "chapters/book_07_planetary/ch_7_4_geology.py",
]


def render_chapter(chapter_path: str) -> bool:
    """Render all scenes in a chapter file."""
    print(f"\n{'='*60}")
    print(f"Rendering: {chapter_path}")
    print('='*60)

    cmd = [
        sys.executable, "-m", "manim",
        QUALITY,
        "--disable_caching",
        "-a",  # All scenes
        chapter_path,
    ]

    result = subprocess.run(cmd, capture_output=False)
    return result.returncode == 0


def main():
    """Render all chapters."""
    print("TRD Animation Batch Renderer")
    print(f"Quality: {QUALITY}")
    print(f"Chapters to render: {len(CHAPTERS)}")

    success = 0
    failed = []

    for chapter in CHAPTERS:
        if render_chapter(chapter):
            success += 1
        else:
            failed.append(chapter)

    print("\n" + "="*60)
    print("RENDER COMPLETE")
    print("="*60)
    print(f"Successful: {success}/{len(CHAPTERS)}")
    if failed:
        print(f"Failed: {len(failed)}")
        for f in failed:
            print(f"  - {f}")


if __name__ == "__main__":
    main()

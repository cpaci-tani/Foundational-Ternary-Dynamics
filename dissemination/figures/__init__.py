"""
TRD Manuscript Individual Figure Modules
========================================
Each module contains a single figure generation function.
"""

from .fig_01_lemniscate_alpha import generate_lemniscate_alpha
from .fig_02_master_quadratic import generate_master_quadratic
from .fig_03_three_states import generate_three_states
from .fig_04_causal_loop import generate_causal_loop
from .fig_05_moore_neighborhood import generate_moore_neighborhood
from .fig_06_interference import generate_interference
from .fig_07_double_slit import generate_double_slit
from .fig_08_voxel_structure import generate_voxel_structure
from .fig_09_zoom_planck import generate_zoom_planck
from .fig_10_standard_model import generate_standard_model
from .fig_11_force_comparison import generate_force_comparison
from .fig_12_yukawa_potential import generate_yukawa_potential
from .fig_13_triad_geometry import generate_triad_geometry
from .fig_14_sloop_mechanism import generate_sloop_mechanism
from .fig_15_constants_flowchart import generate_constants_flowchart

__all__ = [
    'generate_lemniscate_alpha',
    'generate_master_quadratic',
    'generate_three_states',
    'generate_causal_loop',
    'generate_moore_neighborhood',
    'generate_interference',
    'generate_double_slit',
    'generate_voxel_structure',
    'generate_zoom_planck',
    'generate_standard_model',
    'generate_force_comparison',
    'generate_yukawa_potential',
    'generate_triad_geometry',
    'generate_sloop_mechanism',
    'generate_constants_flowchart',
]

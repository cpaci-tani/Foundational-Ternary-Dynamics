# FTD Scientific Visualization Suite

A comprehensive collection of 24 professional scientific visualizations for presenting Foundational Ternary Dynamics (FTD) to global audiences.

## Quick Start

### Prerequisites

```bash
pip install manim plotly pyvista numpy scipy sympy networkx dash
```

### Rendering Animations

```bash
# Navigate to the manim_scenes directory
cd visualizations/manim_scenes

# Render a specific scene (low quality preview)
manim -pql scene_01_ternary_states.py TernaryStateScene

# Render high quality
manim -pqh scene_03_lemniscate_alpha.py LemniscateAlphaScene

# Render 4K
manim -qk scene_03_lemniscate_alpha.py LemniscateAlphaScene
```

### Interactive Dashboards

```bash
# Generate interactive HTML visualizations
cd visualizations/interactive
python mass_spectrum_dashboard.py --save

# Open the generated HTML files in a browser
```

## Directory Structure

```
visualizations/
├── README.md                  # This file
├── manim_scenes/              # Manim animation source files
│   ├── scene_01_ternary_states.py       # Ternary state system
│   ├── scene_02_flux_propagation.py     # Flux field waves
│   ├── scene_03_lemniscate_alpha.py     # FLAGSHIP - α derivation
│   ├── scene_04_four_integers.py        # Four integers cascade
│   ├── scene_06_gauge_emergence.py      # Gauge symmetry
│   ├── scene_07_ckm_matrix.py           # CKM quark mixing
│   ├── scene_09_cosmological_timeline.py # Void to stars
│   ├── scene_10_bell_sloop.py           # Bell inequality & sLoop
│   ├── scene_11_gravitational_hierarchy.py # Why gravity is weak
│   └── scene_24_grand_synthesis.py      # 5-minute epic
├── interactive/               # Plotly/Dash web applications
│   ├── mass_spectrum_dashboard.py
│   ├── mass_spectrum.html              # Generated
│   ├── error_comparison.html           # Generated
│   └── mass_dashboard.html             # Generated
├── static/                    # PNG/SVG exports
│   └── flux_propagation_mpl.png
├── videos/                    # MP4 animation exports
└── utils/                     # Shared utilities
    ├── __init__.py
    ├── ftd_colors.py          # Unified color scheme
    └── ftd_constants.py       # Verified physical constants
```

## Visualization Catalog

### TIER 1: FOUNDATIONAL (Implemented)

| # | Title | Package | Duration | Status |
|---|-------|---------|----------|--------|
| 1 | Ternary State System | Manim | 90s | ✅ Complete |
| 2 | Flux Field Propagation | PyVista | 60s | ✅ Complete |
| 3 | Lemniscate-Alpha Derivation | Manim | 180s | ✅ Complete (Flagship) |
| 4 | Four Integers Cascade | Manim | 120s | ✅ Complete |
| 5 | Mass Spectrum Dashboard | Plotly | Interactive | ✅ Complete |
| 6 | Gauge Symmetry Emergence | Manim | 150s | ✅ Complete |

### TIER 2: PREDICTIONS (Implemented)

| # | Title | Package | Duration | Status |
|---|-------|---------|----------|--------|
| 7 | CKM Matrix | Manim | 90s | ✅ Complete |
| 8 | PMNS/Neutrino Oscillations | Manim | 120s | 🔲 Planned |
| 9 | Cosmological Timeline | Manim | 180s | ✅ Complete |
| 10 | Bell Inequality/sLoop | Manim | 150s | ✅ Complete |
| 11 | Gravitational Hierarchy | Manim | 90s | ✅ Complete |
| 12 | Running Couplings | Plotly | Interactive | 🔲 Planned |

### TIER 3: ADVANCED (Planned)

| # | Title | Concept |
|---|-------|---------|
| 13 | Dimension Uniqueness | Why D = 3 |
| 14 | Proton Stability | τ_p > 10³⁵ yr |
| 15 | Dark Matter Mechanism | Sub-threshold flux |
| 16 | Consciousness Quadratic | Complex roots |
| 17 | Quantum Foam | Planck-scale dynamics |
| 18 | Phase Transitions | States of manifestation |

### TIER 4: PEDAGOGICAL

| # | Title | Type | Status |
|---|-------|------|--------|
| 19 | Lattice Explorer | Dash web app | 🔲 Planned |
| 20 | Derivation Navigator | NetworkX graph | 🔲 Planned |
| 21 | Parameter Sensitivity | Interactive sliders | 🔲 Planned |
| 22 | Historical Comparison | Timeline | 🔲 Planned |
| 23 | Predictions Dashboard | Sortable table | 🔲 Planned |
| 24 | Grand Synthesis | 5-minute epic | ✅ Complete |

## Color Scheme

The visualization suite uses a consistent color palette:

| Element | Hex Code | Use |
|---------|----------|-----|
| Void | `#888888` | State 0, neutral substrate |
| Matter | `#DD4444` | State +1, positive manifestation |
| Antimatter | `#4488DD` | State -1, negative manifestation |
| Flux | `#FFD700` | Flux field, highlights |
| Strong | `#FF6B35` | SU(3) / strong force |
| Weak | `#9B59B6` | SU(2) / weak force |
| EM | `#3498DB` | U(1) / electromagnetic |
| Gravity | `#27AE60` | Gravitational effects |
| Background | `#0D1117` | Deep space black |

## Key Physical Constants

From `utils/ftd_constants.py`:

```python
# Framework Integers
N_C = 3          # Color charges
N_BASE = 4       # Fermat boundary
B_3 = 7          # QCD beta coefficient
N_EFF = 13       # Effective DoF

# Lemniscatic Constant
G_STAR = 2.9586751192  # √2·Γ(1/4)²/(2π)

# Master Quadratic Roots
X_PLUS = 137.036   # → 1/α (fine structure)
X_MINUS = 3.024    # → N_c (color charges)
```

## Usage Examples

### Render the flagship animation (Lemniscate-Alpha):

```bash
manim -pqh scene_03_lemniscate_alpha.py LemniscateAlphaScene
```

Output: `media/videos/scene_03_lemniscate_alpha/1080p60/LemniscateAlphaScene.mp4`

### Render the Grand Synthesis (5-minute epic):

```bash
# High quality 1080p
manim -pqh scene_24_grand_synthesis.py GrandSynthesisScene

# 4K for publication
manim -qk scene_24_grand_synthesis.py GrandSynthesisScene

# Quick 2-minute version
manim -pql scene_24_grand_synthesis.py GrandSynthesisShort
```

### Render Bell Inequality / sLoop:

```bash
manim -pqh scene_10_bell_sloop.py BellSLoopScene
```

### Generate mass spectrum HTML:

```bash
python interactive/mass_spectrum_dashboard.py --save
```

Output: `interactive/mass_spectrum.html`

### Test PyVista flux visualization:

```bash
python manim_scenes/scene_02_flux_propagation.py --mode matplotlib
```

Output: `static/flux_propagation_mpl.png`

## Most Compelling Visualizations

For presentations, we recommend these five visualizations in order:

1. **Scene 24: Grand Synthesis** - The 5-minute epic that tells the complete story
2. **Scene 03: Lemniscate-Alpha** - The flagship derivation of α = 1/137.036
3. **Scene 10: Bell-sLoop** - How FTD achieves quantum correlations locally
4. **Scene 11: Gravitational Hierarchy** - Why gravity is 10³⁷ times weaker
5. **Scene 07: CKM Matrix** - Quark mixing from framework integers

## Technical Notes

### Manim Quality Settings

| Flag | Resolution | FPS | Use Case |
|------|------------|-----|----------|
| `-pql` | 480p | 15 | Quick preview |
| `-pqm` | 720p | 30 | Medium quality |
| `-pqh` | 1080p | 60 | High quality |
| `-qk` | 4K | 60 | Publication |

### Dependencies

- **Manim CE**: Community edition of 3Blue1Brown's animation library
- **Plotly**: Interactive web visualizations
- **PyVista**: 3D scientific visualization
- **NumPy/SciPy**: Numerical computation
- **SymPy**: Symbolic mathematics

## Contributing

To add a new visualization:

1. Create a new scene file in `manim_scenes/` or `interactive/`
2. Import colors from `utils/ftd_colors.py`
3. Import constants from `utils/ftd_constants.py`
4. Follow the existing code structure and naming conventions
5. Update this README with the new visualization

## License

Part of the Foundational Ternary Dynamics project.

---

*Created: January 2026*
*Author: FTD Visualization Suite*

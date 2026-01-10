# FTD Pedagogical Animations

High-quality Manim animations for Ternary Realization Dynamics pedagogy.

## Overview

This directory contains the animation library and chapter-specific scenes for the FTD video curriculum. All animations are rendered at 1080p60 with a dark cinematic theme.

## Directory Structure

```
animations/
├── lib/                    # Core animation library
│   ├── config.py           # Render settings (1080p60)
│   ├── colors.py           # Dark cinematic color scheme
│   ├── trd_scene.py        # Base scene class with timing markers
│   └── components/         # Reusable animation primitives
│       ├── voxel.py        # Glowing voxel visualization
│       ├── flux_field.py   # Vector field with glow trails
│       └── ...
│
├── chapters/               # Chapter-specific animations
│   ├── book_01_foundations/
│   ├── book_02_subatomic/
│   └── ...
│
├── standalone/             # Standalone concept animations
│   ├── g_star_derivation.py
│   ├── hilbert_space.py
│   └── ...
│
├── output/                 # Rendered videos
│   ├── 1080p60/
│   └── preview/
│
└── render.py               # Batch rendering script
```

## Quick Start

### Prerequisites

- Python 3.10+
- Manim Community Edition 0.19+
- FFmpeg

### Activate Manim Environment

```bash
cd /path/to/pbr_pedagogy
source .venv-manim/bin/activate  # Linux/Mac
# or
.\.venv-manim\Scripts\activate   # Windows
```

### Render a Single Scene

```bash
cd dissemination/animations

# Preview quality (fast)
python -m manim -pql chapters/book_01_foundations/ch_1_1_void.py VoidIntro

# Production quality (1080p60)
python -m manim -qh --fps 60 chapters/book_01_foundations/ch_1_1_void.py VoidIntro
```

### Render All Scenes

```bash
python render.py --quality production --parallel 4
```

## Visual Style

All animations use a **dark cinematic theme**:

- **Background**: Deep space blue-black (#0a0a14)
- **Matter (+1)**: Glowing red-pink (#ff4466) with white core
- **Antimatter (-1)**: Glowing cyan-blue (#44aaff) with white core
- **Void (0)**: Subtle gray (#2a2a3a)
- **Highlights**: Golden (#ffcc00)
- **Text**: Light gray (#e0e0e0)

Particles include multi-layer glow effects for cinematic appearance.

## Timing Markers

Each animation includes timing markers for voice-over synchronization:

```python
class MyScene(FTDScene):
    def construct(self):
        self.load_narration("1.2")  # Load from content/narration/

        self.add_marker("1.2.0.1", "intro")
        # ... animation ...
        self.wait_for_narration("1.2.0.1")

        self.export_markers()  # Save timing JSON
```

Markers are exported to `output/{chapter_id}_markers.json` for audio sync.

## Component Library

### VoxelMobject

```python
from lib.components import VoxelMobject

voxel = VoxelMobject(state=+1, size=1.0, show_glow=True)
self.play(voxel.genesis(+1))  # Animate 0 -> +1
self.play(voxel.evaporate())  # Animate +1 -> 0
```

### FluxFieldMobject

```python
from lib.components import FluxFieldMobject

flux = FluxFieldMobject(rows=10, cols=10, flux_func=my_func)
self.play(flux.accumulate(center=ORIGIN))  # Flux flows inward
self.play(flux.disperse())                  # Flux flows outward
```

### Lattice Components

```python
from lib.components import Lattice2D, Lattice3D, MooreNeighborhood

# 2D grid visualization
lattice = Lattice2D(rows=7, cols=7, spacing=0.8, show_glow=True)
self.play(lattice.highlight_moore_neighborhood(3, 3))

# 3D lattice with isometric projection
lattice_3d = Lattice3D(size=4, spacing=1.0)

# Moore neighborhood (26-connected) visualization
moore = MooreNeighborhood(spacing=1.2)
self.play(moore.animate_build())
```

### Wave Components

```python
from lib.components import WavePulse, StandingWave, InterferencePattern, FluxWave

# Expanding wave pulse
pulse = WavePulse(center=ORIGIN, max_radius=5.0, num_rings=4)
self.play(pulse.expand(run_time=3.0))

# Standing wave oscillation
wave = StandingWave(length=10.0, wavelength=2.0, amplitude=1.0)
self.play(wave.oscillate(run_time=4.0, cycles=2.0))

# Two-source interference pattern
pattern = InterferencePattern(source1=LEFT*2, source2=RIGHT*2)
self.play(pattern.animate_interference(run_time=5.0))

# Discrete flux wave on lattice
flux_wave = FluxWave(grid_size=15, spacing=0.6)
flux_wave.set_initial_pulse((7, 7))
self.play(flux_wave.propagate(run_time=4.0))
```

### Causal Loop Components

```python
from lib.components import CausalLoopDiagram, CausalLoopLegend, CAUSAL_LOOP_STEPS

# 13-step circular diagram
loop = CausalLoopDiagram(radius=3.0, node_radius=0.4)
self.play(Create(loop))
self.play(loop.highlight_step(5))  # Highlight SUPERPOSE
self.play(loop.animate_full_cycle(duration=6.0))
self.play(loop.show_phase_groups())

# Legend showing phase colors
legend = CausalLoopLegend(position=RIGHT * 5)
```

### Lemniscate Components

```python
from lib.components import (
    LemniscateWithGlow, LemniscateDecomposition,
    ArcLengthTracer, GStarReveal, LemniscateAlphaConnection,
    G_STAR, VARPI
)

# Basic lemniscate curve with glow
curve = LemniscateWithGlow(scale=2.0)
self.play(Create(curve))

# Harmonic decomposition
decomp = LemniscateDecomposition(modes=[1, 2, 4, 8, 16])
self.play(decomp.build_up_animation())

# Arc length tracing to G*
tracer = ArcLengthTracer(scale=2.0)
self.play(tracer.trace_animation(run_time=4.0))

# Complete G* reveal with equations
reveal = GStarReveal(scale=2.0)
self.play(reveal.reveal_sequence())

# Full alpha connection visualization
connection = LemniscateAlphaConnection()
self.play(connection.animate_connection())
```

### Master Quadratic Components

```python
from lib.components import (
    MasterQuadraticDiagram, QuadraticDerivation,
    AlphaHighlight, NcHighlight, X_PLUS, X_MINUS
)

# Full quadratic diagram with parabola and roots
diagram = MasterQuadraticDiagram(show_labels=True, show_equation=True)
self.play(Create(diagram))

# Step-by-step derivation
derivation = QuadraticDerivation()
self.play(derivation.reveal_animation())

# Highlight fine structure constant
alpha = AlphaHighlight()
self.play(FadeIn(alpha))

# Highlight color charge number
nc = NcHighlight()
self.play(FadeIn(nc))
```

### Scale Zoom Components

```python
from lib.components import (
    ScaleMarker, ScaleRuler, ScaleTransition,
    ScaleJourney, ZoomPulse, SCALE_LEVELS
)

# Logarithmic scale ruler (Planck to cosmic)
ruler = ScaleRuler(length=12.0, show_markers=True)
self.play(Create(ruler))

# Scale markers at specific levels
marker = ScaleMarker("atomic", radius=0.4)
self.play(FadeIn(marker))

# Transition between scales
transition = ScaleTransition(from_scale="planck", to_scale="atomic")
self.play(transition.transition_animation())

# Complete journey through all scales
journey = ScaleJourney()
self.play(journey.journey_animation(run_time=20.0))

# Zoom pulse effect
pulse = ZoomPulse(center=ORIGIN, color=FTD_COLORS["highlight"])
self.play(pulse.pulse_out())
self.play(pulse.pulse_in())
```

## Technical Specs

- **Resolution**: 1920x1080 (16:9)
- **Frame Rate**: 60 fps
- **Codec**: H.264 (libx264)
- **Format**: MP4

## License

Part of the FTD Pedagogy project.

# FTD Interactive Force Simulations

Interactive 2D educational simulations demonstrating the four fundamental forces as derived from Foundational Ternary Dynamics (FTD).

## Overview

These self-contained HTML simulations visualize the exact mathematical formulas for each force, derived from the FTD action principle **S[s,J]**.

## Simulations

### Dyadic Lacunary Curve Lab (`dyadic_lacunary_curve_lab.html`)
**Model:** finite dyadic Fourier readout `C_K(t)` plus geometric-tail controls

- Standalone canvas workbench for the `C_3` curve and its mutable Fourier family
- Direct per-mode editing of amplitudes, phases, enable flags, tail laws, render modes, and bookend-only sparse mode experiments
- Live signed area, centroid, length, turning, energy hierarchy, node estimates, Holder thresholds, max active frequency, and samples-per-cycle readouts
- Heuristic regime classifier for bookend, ribbon, shell, braid/web, rough-tail, and alias-risk states
- Click-based phase fiber microscope plus prefix-mode node genealogy strip
- Three.js 3D phase-lift mode with selectable `z` channels: phase lag, clock phase, speed, curvature, dominant mode, and area sweep
- Optional overlays for self-intersections, curvature events, mutable ribbon/tesseract shadow, epicycle chain, spectrum, and branch readout
- Stable absolute-grid camera during animation; phase motion no longer refits the view to sampled curve bounds
- Ultra-slow animation control with fine speed and phase increments for smooth phase-shadow drift
- Exploratory mathematics only; no FTD physics claim is promoted by coefficient mutations

### 0. Potential Core Explorer (`potential_core_explorer.html`)
**Model:** `P_c -> G(P_c) -> C -> G_C(P_c) -> B_C(P_c, r) -> M_C(P_c, r)`

- Self-contained 3D-style conceptual explorer
- Data-driven layer model for incremental edits
- Shows Potential Core, Generative Interior, Context State, contextual activation wedge, manifest boundary, and output rays
- Drag to rotate; sliders adjust context aperture, reach `r`, and output flow
- Intended as a running theory diagram, not a finished proof graphic

### 1. Gravity Simulation (`gravity_simulation.html`)
**Formula:** F_grav = G_N * nabla(rho_bar)

- N-body gravitational attraction
- Inverse-square law emergence from 3D geometry
- Orbital mechanics visualization
- Energy conservation tracking

### 2. Electromagnetic Simulation (`electromagnetic_simulation.html`)
**Formulas:**
- Coulomb: F_elec = -q * nabla(q_bar)
- Lorentz: F_mag = beta * (curl J) x J_hat

- Charged particle interactions (+/-)
- Electric field line visualization
- Fine structure constant alpha = 1/137.036 displayed
- Click to place charges interactively

### 3. Strong Force Simulation (`strong_force_simulation.html`)
**Formula:** F_strong = g_s^2 * exp(-m_pi * r) / r^2 * (1 + m_pi * r)

- Yukawa potential with exponential decay
- Color charge visualization (RGB)
- Quark confinement demonstration
- Real-time force vs. distance graph
- Drag quarks to see confinement!

### 4. Weak Force Simulation (`weak_force_simulation.html`)
**Formula:** S = |div J| + |curl J| + |grad rho|

- Stress-driven transmutation (+1 <-> -1)
- Stress field heatmap visualization
- Threshold-triggered particle transformation
- Neutrino emission effects
- Beta decay demonstration

### 5. Unified Forces Simulation (`unified_forces_simulation.html`)
**Action:** S[s,J] = sum_t sum_v L(s, J, grad J)

- All four forces active simultaneously
- Force hierarchy visualization (Strong > EM > Weak > Gravity)
- Toggle individual forces on/off
- Real-time force contribution breakdown
- Atom formation demonstrations

### 6. Discrete Universe & Continuum Emergence Simulator (`discrete_universe_simulator.html`)
**Action:** S[s,J] = sum_{x, t} ( 1/2|\nabla_{\text{discrete}} J|^2 - 1/2(\partial_t J)^2 - s|J| )

- Live 2D lattice voxel grid visualizing ternary state field $s(x)$ and vector flux field $\vec{J}(x)$
- Interactive scenarios: Coulomb potential emergence, relativistic speed capping ($c_{\text{lat}} = 1/\sqrt{2}$ limit), discrete wave interference, and voxel cluster genesis (FTD-0110)
- Real-time radial cross-section plot showing exact discrete-to-continuum potential curve matching
- Live adjustable parameter sliders for wave speed, genesis threshold, SOR Poisson damping, and F4 latent-heat drain toggles

## How to Use

1. **Open any HTML file directly in a web browser** - no server required
2. Use the control panels on the right to adjust parameters
3. Toggle visualizations (force vectors, trails, field overlays)
4. Try the preset configurations to see different physics scenarios

## Technology

- **p5.js** for 2D physics visualization
- **KaTeX** for mathematical formula rendering
- Self-contained HTML (no external dependencies except CDN libraries)

## Mathematical Foundations

All formulas are derived from the FTD action principle:

```
S[s,J] = sum_t sum_v [ 1/2|dt J|^2 - 1/2 c^2|grad J|^2
                       - lambda(div J - rho)^2
                       - V(|J|, s)
                       - g_c * s * (div J)
                       - mu * (s^2 - |s|) ]
```

Key derived constants:
- **alpha = 1/137.036** (fine structure constant, 1.26 ppm accuracy)
- **N_c = 3** (number of color charges)
- **sin^2(theta_W) = 3/13 = 0.231** (Weinberg angle)

## Force Hierarchy

| Force | Relative Strength | FTD Origin |
|-------|------------------|------------|
| Strong | 1 | Yukawa from confined flux |
| Electromagnetic | 1/137 | Gauss constraint |
| Weak | 10^-5 | Stress-driven transmutation |
| Gravity | 10^-39 | Flux density gradients |

## Educational Goals

These simulations aim to:
1. **Show the math** - formulas displayed with real-time values
2. **Visualize forces** - vector arrows, field lines, gradients
3. **Demonstrate emergence** - how forces arise from simple principles
4. **Enable exploration** - adjustable parameters and interactive controls

## License

Part of the Foundational Ternary Dynamics project.

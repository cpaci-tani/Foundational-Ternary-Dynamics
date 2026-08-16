# FTD Interactive Force Simulations

Interactive 2D educational simulations demonstrating the four fundamental forces as derived from Foundational Ternary Dynamics (FTD).

## Overview

These self-contained HTML simulations visualize the exact mathematical formulas for each force, derived from the FTD action principle **S[s,J]**.

## Simulations

### Dyadic Mode Configuration Atlas (`dyadic_mode_configuration_atlas.html`)
**Model:** ternary mode words `w_k in {-1,0,+1}` over finite dyadic Fourier clocks

- Standalone responsive atlas for enabling, disabling, and reversing mode chirality in exact patterns
- Pattern presets for prefixes, bookends, blocks, parity masks, Thue-Morse signs, and sparse supports
- Continuous one-mode transition ramps with ultra-slow playback and a sampled minimum-speed / exact-area profile
- Certified `C_3` octave-edge wall overlays for endpoint, tangent, speed-zero, and seed-count events when that exact slice is active
- Exact live fingerprints for support, trace multiplicity, quotient degree, signed area, cancellation, Fourier energies, symmetry, phase lift, and trigonal-relay eligibility
- Interactive geometric-byte map of all 256 eight-mode support masks, with exact 6,561-state balanced-ternary signed fingerprints and byte shift/complement operations
- Clickable `H(m,3)` configuration-phase slices for 2–6 mutable modes (9–729 states), with synchronized curve selection and coloring by signed area, support, quotient scale, pattern class, or sampled minimum speed
- Transition event ledger combining analytic area/support/quotient walls and certified C3 edge thresholds with explicitly sampled crossing-count and local-tube brackets
- Exact default `H(8,3)` census of all 52,488 one-trit edges, partitioned into 2,510 invariant fingerprint bins for the current C3-tail coefficients
- Interactive chamber quotient under selectable exact area, quotient, support, and chirality barriers; independently verified default chamber counts are 5, 8, 256, and 6,561 for key barrier selections
- Mouse camera with drag-pan and cursor-centered wheel zoom in 2D, plus orbit, right-drag pan, and wheel zoom in 3D
- Canonical wireframe tube surface around the lifted centerline, with mutable radius and orbit controls
- Live surface certification panel separating the exact embedded-centerline theorem from sampled curvature, Jacobian, normal-chord clearance, and reach bounds
- Three.js phase-lift toggle using `z=sin(2^g t)` at the quotient fundamental, with mutable depth, planar projection, and depth fibers
- Expanded live formula view for the complete current `x(t)`, `y(t)`, and lift `z(t)`, including the quotient domain, normalized one-dimensional phase, and evaluated point
- Equation Reel storyboard that captures exact formula text together with the full mode and camera state, supports write-on, cross-morph, pulse, and hold effects, and smoothly interpolates captured Fourier coefficients
- Live 16:9 composite preview with editable scene equations, ordering and timing, plus WebM recording and standalone reel JSON import/export
- Exact instantaneous mode-vector decomposition showing how the active Fourier vectors and partial sums assemble the planar point
- Sampled crossing-depth microscope with Newton-refined phase pairs, planar residual, fundamental-lift heights, depth separation, and linked 3D branch markers
- Sampled node and minimum-speed diagnostics kept explicitly separate from theorem-grade readouts
- Stable absolute grid, phase/speed/curvature/orientation coloring, JSON import/export, PNG capture, and equation-led presentation recording
- Mobile and desktop layouts; no server or external dependency is required
- Configuration-space mathematics only; no FTD physics claim is promoted by a ternary mode word

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

### Island of Stability — Computed Nuclide Landscape (`island_of_stability.html`)
**Model:** SEMF `B(Z,N)` (Wapstra coefficients [IMPOSED]) + Myers–Swiatecki 1966 shell correction + Viola–Seaborg alpha systematics (SPC 1989)

- Standalone canvas (Z,N) heatmap, Z 82–132 by N 110–204, colored by computed log10 alpha half-life; magic-number gridlines, SEMF beta-stability line, measured-nuclide markers
- The island emerges from the imported shell correction: a 0–100% shell slider makes it appear and vanish; a proton-closure hypothesis selector (114/120/126) moves its proton edge, exposing real model dependence
- Alpha Q-values from binding differences (B(4He) = 28.2957 MeV, AME2020); Viola–Seaborg half-lives; spontaneous fission as a qualitative barrier-proxy band (no quantitative T_SF); beta channel qualitative only
- Embedded 14-nuclide measured validation set (212Po … 294Og) with computed-vs-measured rows in the detail panel and a live validation table
- Ring viewport driven by real quantities: radius from A^(1/3), jitter from computed instability, harmonic amplitude from the shell correction; scan mode sweeps the beta-stability line
- Toggleable, visually distinct FTD annotation layers at corpus tags: Z=126 9-lobe = N_c^2 [CONJECTURE], SEMF-coefficient fits [PARAMETRIC], 82−50 = 32 = 2·N_base^2 [THEOREM]
- Claim Boundary panel separating imported parametric physics, computed outputs, presentation effects, and open questions
- Standard nuclear-physics parametrizations for pedagogy; the island emerges from imported shell corrections, and no FTD physics claim is promoted by this tool

### Unindexed standalone demos

These four exist, are real (non-trivial, multi-commit) artifacts, and are not referenced by any paper, script, or LEDGER row — added to this index on 2026-08-16 (repo cleanup pass) rather than removed, since none carry a superseded-by signal. Treat descriptions below as page titles only; none has been independently verified against current FTD claims the way the entries above have.

- **Mechanism β Visualizer** (`mechanism_beta_visualizer.html`) — "Mechanism β v2 — Back-Reaction & Threshold Shift" (Three.js visualization, added 2026-06-10).
- **Erdős Unit Distance Toy** (`erdos_unit_distance_toy.html`) — "Erdős Unit Distance Toy Model" (added 2026-06-03).
- **Chromium Geometry** (`chromium_geometry.html`) — "FTD: The Chromium Anomaly" (added 2026-06-18).
- **3D Aperiodic Monotile** (`3d_aperiodic_monotile.html`) — "FTD 3D Aperiodic Monotile (Advanced Logic Upgrade)" (added 2026-06-03).

### Theory Mindmap — retired 2026-08-16

`theory_mindmap.html` + `theory_mindmap.json` (generated by
`scripts/theory/build_theory_mindmap.py`, drift-checked by
`scripts/tests/test_theory_mindmap.py`) covered every git-tracked document
under `docs/theory/` as a two-mode map (radial structure tree + keyword
concept graph) from 2026-08-06 until removal — no longer needed, deleted
rather than archived per explicit owner instruction. `3d_theory_map.html` +
`graph.json` (the hand-built map this had itself superseded — see the note
inside `3d_theory_map.html`) are still present, retained for provenance only.

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

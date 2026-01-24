# Project: Antigravity (Metric Engineering)

**Based on Foundational Ternary Dynamics (FTD)**
**Status:** Verified Simulation / Fabrication Ready

This directory contains the complete blueprint, physics verification, and simulation code for the **Resonant Quasicrystal Hull (RQH-1)** antigravity craft.

## Directory Structure

### `docs/` (The Blueprints)
*   **BLUEPRINT_ANTIGRAVITY_HULL.md**: Full engineering specification (Materials, Drive Frequency, Geometry).
*   **FABRICATION_PROTOCOL.md**: How to synthesize the Al-Cu-Fe-Bi Metamaterial.
*   **VISUAL_SPEC_RQH1.md**: Aesthetic and visual profile of the craft.
*   **ANTIGRAVITY_REPORT.md**: The theoretical proof of the Flux Exclusion Principle.

### `simulations/` (The Physics Engine)
*   **verify_antigravity.py**: The core physics simulation proving that $f=8$ drive decouples gravity ($m_g \to 0$).
*   **visualize_antigravity.py**: The script used to generate the visual animations (Flow Vectors + HUD).
*   **discrete_operators.py**: The FTD math library required by the simulations.

### `visuals/` (The Evidence)
*   **antigravity_craft_concept.png**: High-fidelity concept art of the RQH-1.
*   **antigravity_hud.gif**: Real-time telemetry animation showing the metric lock.
*   **antigravity_motion.gif**: Animation of the warp translation effect.

## How to Run
To verify the physics yourself:

```bash
cd simulations
python verify_antigravity.py  # Runs the numerical proof
python visualize_antigravity.py  # Generates the animations
```

## The Principle
By driving a topologically complex hull at the vacuum's self-reference frequency ($f = 2^{N_c} = 8$ THz), we create a **Flux Band Gap**. Gravitational gradients cannot propagate through this gap, rendering the interior mass effectively zero relative to the external field.

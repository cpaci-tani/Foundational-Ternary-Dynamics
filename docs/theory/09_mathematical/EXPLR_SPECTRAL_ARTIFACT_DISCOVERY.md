# Spectral Artifact Discovery: 2D FFT Gauge-Group Selection Was Not Physics

## How a Square Grid Created the Illusion of Born-Rule Symmetry Filtering

**Date:** April 4, 2026
**Framework:** Foundational Ternary Dynamics v5.29
**Document Status:** Exploratory -- honest negative, artifact diagnosis
**Epistemic Class:** [EMERGENT] for all simulation results
**Category:** 9 (Mathematical Connections)

---

## Depends On

- [EXPLR_SPECTRAL_CIRCLE_TO_LEMNISCATE.md](EXPLR_SPECTRAL_CIRCLE_TO_LEMNISCATE.md) -- The valid circle-to-lemniscate Joukowski result
- Engine WASM build (`engine/web/wasm/ftd_core.wasm`) -- 3D cubic lattice simulation

---

## Honesty Note

This document records a **false lead** and its correction. The initial 2D analysis produced exciting results that appeared to show the Born rule selecting for Lie algebra gauge groups. Five rounds of increasingly sophisticated 2D analysis (15 figures, 4 independent metrics) all supported the finding. It was wrong. The 3D engine proved it was a square-grid FFT artifact. This is documented as an example of epistemic discipline: the engine corrected the theory.

---

## 1. What the 2D Analysis Showed

Gaussian wavepackets were placed at N equally-spaced positions on a ring in a 512x512 grid. The Born rule |psi|^2 was computed, then Fourier-transformed. The spectral peaks were analyzed for angular structure.

**Apparent findings:**
- Crystallographic N = {2, 3, 4, 6} showed high angular concentration (peaks clustered at Lie algebra root directions)
- N=5 showed dramatically lower concentration (3x drop) -- no simple Lie algebra has 5-fold symmetry
- N=7, 11, 13 (non-crystallographic primes) also showed low concentration
- Four independent metrics agreed: angular concentration, effective rank, entropy gap, fractal dimension
- The "Gauge Group Quadrant" scatter plot cleanly separated crystallographic from non-crystallographic N

**Interpretation (incorrect):** The Born rule acts as a gauge-group filter. Placing N particles in a ring and computing |psi|^2 automatically selects the root system of the Lie algebra for dimension N -- but only for crystallographic N values that admit Lie algebras.

---

## 2. Why It Was Wrong: The Square Grid

The 512x512 computational grid is a square lattice. Its discrete Fourier transform has a square Brillouin zone with special points:

- **Gamma point** (0, 0): center
- **X points** (pi/a, 0) and (0, pi/a): edge centers at 0, 90, 180, 270 degrees
- **M points** (pi/a, pi/a): corners at 45, 135, 225, 315 degrees

The M points (corners) have the highest density of states. Spectral peaks near 45 degree diagonals are preferentially enhanced by the grid. This creates a secondary C4 modulation that:

1. **Enhances** peaks near 45 degree diagonals (M-point bias)
2. **Suppresses** peaks near cardinal directions (X-point competition)
3. **Favors** N values whose rotational symmetry aligns with the grid (N = 2, 4, especially)
4. **Penalizes** N values whose symmetry is incommensurate with the grid (N = 5, 7)

Crystallographic N = {2, 3, 4, 6} are precisely the rotational symmetries compatible with a 2D square lattice. The "gauge-group selection" was the crystallographic restriction theorem applied to the FFT grid, not physics of the Born rule.

---

## 3. The Definitive Tests

### Test A: Rotation Invariance (`grid_artifact_test.py`)

Rotated the N=3 configuration by 0, 15, 30, 45, 60, 75, 90 degrees and tracked spectral peaks.

**Result:** Peaks rotate WITH the particle configuration (physics, not grid). However, the CONTRAST of peaks varies with rotation -- peaks are sharpest when particle positions align with grid diagonals (45 degrees). The grid adds a secondary modulation on top of the physical signal.

### Test B: Grid Size Independence

Ran the same N=3 configuration on grids of size 256, 384, 500, 512, 600, 700, 768, and 1024.

**Result:** The honeycomb pattern persists across all grid sizes (confirming the physical signal is real), but the angular concentration metric varies with grid size due to changing M-point positions.

### Test C: Real 3D Engine Validation (`real_3d_angular.py`)

Injected N = 2, 3, 4, 5, 6, 7, 8 particles into the actual 32^3 cubic lattice WASM engine, ran 40 ticks of wave propagation (movement disabled), and sampled the equatorial flux magnitude at 72 azimuthal angles.

**Result:**

| N | Concentration | Contrast | Peak Angles | Match Expected? |
|---|--------------|----------|-------------|-----------------|
| 2 | 0.081 | 13.2 | 0, 180 | Yes |
| 3 | 0.060 | 13.3 | 0, 120, 240 | Yes |
| 4 | 0.022 | 4.3 | 0, 90, 180, 270 | Yes |
| 5 | 0.019 | 4.3 | 0, 72, 144, 216, 288 | Yes |
| 6 | 0.010 | 3.0 | 0, 60, 120, 180, 240, 300 | Yes |
| 7 | 0.005 | 2.0 | 0, 51, 103, ... | Yes |
| 8 | 0.002 | 1.6 | 0, 45, 90, ... | Yes |

**ALL N values produce clean peaks at the correct particle angles, including N=5.**

Concentration decreases monotonically with N. Contrast decreases monotonically with N. No special role for crystallographic dimensions. The N=5 "dip" is absent in 3D.

---

## 4. What IS Real

Despite the artifact, several genuine results emerged from this investigation:

1. **The Born rule IS a nonlinear spectral transform.** |psi|^2 creates cross-terms in Fourier space that encode geometric relationships between particle positions. This is real and independent of the grid.

2. **N-fold symmetry IS preserved.** N particles at equal angles produce N-fold symmetric flux patterns in 3D. The cubic lattice does not break this symmetry.

3. **Concentration decreases with N.** More particles create more uniform flux distributions. This is a genuine physical result: many-body systems are less structured than few-body systems.

4. **The circle-to-lemniscate Joukowski result is valid.** The spectral transformation F[psi] = circle, F[|psi|^2] = lemniscate is algebraically exact and independent of the grid artifact. See EXPLR_SPECTRAL_CIRCLE_TO_LEMNISCATE.md.

---

## 5. Lessons Learned

1. **Always validate 2D spectral analysis against the 3D engine.** The engine is the ground truth. 2D projections can introduce artifacts that mimic physics.

2. **Multiple metrics agreeing does not prove correctness.** Four independent metrics (concentration, rank, entropy gap, fractal dimension) all showed the same pattern -- because all four were computed on the same biased grid. Correlated errors produce correlated false positives.

3. **The crystallographic restriction theorem is easy to rediscover.** Any computation on a square grid will preferentially select crystallographic symmetries. This is mathematics, not physics -- but it can look like physics if you don't check.

4. **Honest negatives are essential.** Documenting false leads prevents others from repeating them and demonstrates epistemic rigor.

---

## Scripts and Data

- `scripts/exploration/five_minds_followup.py` -- Round 1 (5 figures)
- `scripts/exploration/five_minds_round2.py` -- Round 2 (5 figures)
- `scripts/exploration/five_minds_round3.py` -- Round 3 systematic sweep (5 figures)
- `scripts/exploration/five_minds_round4.py` -- Round 4 cross-mind synthesis (5 figures)
- `scripts/exploration/five_minds_round5.py` -- Round 5 killer experiments (5 figures)
- `scripts/exploration/grid_artifact_test.py` -- Rotation invariance test (4 figures)
- `scripts/exploration/real_3d_angular.py` -- 3D engine validation (3 figures)
- All output figures in `output/` directory

---

## Cross-References

- **Valid spectral result:** [EXPLR_SPECTRAL_CIRCLE_TO_LEMNISCATE.md](EXPLR_SPECTRAL_CIRCLE_TO_LEMNISCATE.md) -- Born rule as Joukowski transform
- **Born rule derivation:** [FOUND_BORN_RULE_NULL_CONE.md](../02_foundations/FOUND_BORN_RULE_NULL_CONE.md) -- null-cone geometry (unaffected)
- **Gauge group derivation:** [DERIV_GAUGE_GROUPS.md](../03_derivations/DERIV_GAUGE_GROUPS.md) -- from Moore neighborhood J^2 decomposition (orthogonal to spectral analysis)

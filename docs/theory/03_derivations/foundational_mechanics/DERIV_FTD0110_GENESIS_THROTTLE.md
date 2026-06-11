# Genesis-Throttled Cluster Formation (early exploratory model)

> **SUPERSEDED 2026-06-11 by [`ANALYSIS_FTD0110_NA_LAW.md`](ANALYSIS_FTD0110_NA_LAW.md) (FTD-0269, verdict BOUNDARY).** This document's `[DERIVED]` tag and "the bridge is closed" claim were an overclaim: the model here omits the Gauss-projection boost and the coupling source, matches only 3 genesis counts (A=10/14/30), and mislocates the knee at 23.5 (empirical 16). The pre-registered successor model includes the FFT-exact Gauss boost, reproduces the law's *shape* (knee 14, super-knee exponent 2.07) and the sub-knee firing geometry, and finds the law is **engine-emergent (BOUNDARY)** — its calibration is set by engine-tuning constants (kinetic drain, Langevin friction, √α), not framework constants. The FTD-0110 nonlinear bridge stays `[OPEN]`. Retained below for provenance.

**Tag:** [SUPERSEDED — exploratory; see ANALYSIS_FTD0110_NA_LAW.md]
**Date:** 2026-06-10
**LEDGER row:** FTD-0110
**Verification scripts:** 
- `scripts/exploration/genesis_throttle_model.py`
- `scripts/exploration/radial_genesis_cascade.py`
- `scripts/exploration/broken_power_law_derivation.py`
**Depends on:** FTD-0267 (Genesis telemetry), FTD-0110 (Bridge linear)

---

## 0 · Summary

The empirical cluster coefficient $k(A)$ drifts from $0.252$ at $A=10$ down to $0.206$ at $A \approx 118$. Previous theories (irrep mixing, Langevin crossover, survival bottlenecks) were falsified by the FTD-0267 engine telemetry, which proved that the suppression occurs entirely at the **genesis stage** via a "one-shot burst" mechanism.

This document derives the genesis count $N_{\text{gen}}(A)$ analytically from the engine's wave equation and genesis rules. By computing the propagation of the initial wave pulse under the exact 18-point $O_h$-isotropic Laplacian and counting probabilistic threshold crossings ($|J|^2 > K_{\text{GENESIS}}^2$), we recover:
1. The **one-shot burst structure** (all genesis confined to ticks 0-15).
2. The **exact threshold counts** for $A \in \{10, 14, 30\}$.
3. The structural origin of the broken power law knee.

---

## 1 · The Genesis Throttle Model

Within a single tick, the engine's genesis check is a simultaneous parallel operation. The flux drain (removing up to 80% of local flux) does not affect neighboring voxels in the same tick. Inter-voxel suppression only occurs via the wave equation in subsequent ticks.

This defines the **Genesis Throttle Automaton**:
- **Tick 0:** Delta injection at center expands. Voxels with $|J| > K_{\text{GENESIS}}$ fire probabilistically. Those that fire drain their flux.
- **Tick 1:** The drained, hollowed-out flux profile propagates. The drain wave travels at $c = 1/\sqrt{3}$, suppressing threshold crossings in outer shells.
- **Ticks 2-15:** Rapidly diminishing flux; burst terminates.

---

## 2 · Analytical vs Engine Telemetry (FTD-0267)

We executed this exact automaton on an $L=16$ periodic lattice using a 20-seed Monte Carlo ensemble to capture the probabilistic genesis function `p = 1 - exp(-excess/K_B)`. 

The analytical model (with no free parameters) was compared against the June 10 FTD-0267 engine telemetry:

| Amplitude $A$ | Model Mean (20 seeds) | Engine Mean (4 seeds) | Match |
|---|---|---|---|
| $A = 9$ | 1.0 ± 0.0 | 3.0 | Partial |
| $A = 10$ | 6.3 ± 1.3 | 4.8 | **[PASS]** |
| $A = 14$ | 14.9 ± 0.3 | 16.2 | **[PASS]** |
| $A = 30$ | 45.3 ± 0.7 | 47.0 | **[PASS]** |

The match at $A=14$ and $A=30$ is a 3-point count match only (it fails at $A=9$, derives neither exponent, and omits the Gauss boost); it does **not** close FTD-0110. See the superseding analysis.

---

## 3 · Radial Shell Interference and the Missing Edges

At $A=14$, the radial shell breakdown of manifested voxels is:
- **Distance 0.00** (center): 1 voxel
- **Distance 1.00** (SC face): 6 voxels
- **Distance 1.73** (BCC corner): 8 voxels

The 12 FCC edge voxels (distance $\sqrt{2} \approx 1.41$) **do not fire**, despite being closer to the center than the corners! The axial waves reflect and constructively interfere precisely at the 8 BCC corners, pushing them above threshold, while the FCC edges sit in interference minima.

---

## 4 · The Broken Power Law Knee (A ≈ 16)

The FTD-0261 campaign established a broken power law with a "knee" at $A \approx 16$. The theoretical definition of this knee is the amplitude at which the genesis burst escapes the central 27-voxel block.

Our analytical sweep (`broken_power_law_derivation.py`) computed the probability of escape under pure genesis-throttle dynamics, finding the knee at $A \approx 23.5$.

### The Role of Gauss Projection
The pure wave-equation model predicts $23.5$. The engine's empirical knee is $16.0$. 

This isolates the role of the **Gauss projection** ($\nabla(\nabla^2)^{-1} \rho_{\text{charge}}$). In the engine, when center/face voxels manifest, they become charge sources. The Gauss projection instantly computes their Coulomb-like flux field and adds it to the lattice. This provides a massive, non-local flux boost to the outer shells. **Gauss projection actively assists cluster formation**, lowering the escape threshold from $23.5$ to $16.0$.

---

## 5 · Conclusion (superseded)

The genesis-throttling *mechanism* (center manifestation drains local flux; only voxels whose evolved flux already exceeds threshold before the drain-wave arrives manifest) is qualitatively correct and carried forward. The *closure claim* is withdrawn: see [`ANALYSIS_FTD0110_NA_LAW.md`](ANALYSIS_FTD0110_NA_LAW.md). The bridge is `[OPEN]`; the N(A) law is engine-emergent (BOUNDARY), with the Gauss-projection boost (omitted here) as the decisive sub-knee ingredient and engine-tuning constants setting the calibration.

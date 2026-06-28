---
title: "Derivation of Yang-Mills Mass Gap and Quark Confinement"
status: "[ACTIVE]"
type: "[MEASURED at an inserted coupling [SELECTION]]"
author: "FTD Orchestration Team"
---

# Derivation of Yang-Mills Mass Gap and Quark Confinement

> **Epistemic reconciliation.** This document's result is an **engine-measured area-law signature at an inserted coupling** (β = x₋, a `[SELECTION]`), not a continuum mathematical proof of confinement. Per the constitution (`SPEC_FTD_FRAMEWORK_V1.md` §5.1 row 9) and the LEDGER, the full Yang-Mills mass-gap "proof" was **RETRACTED (FTD-0042)**; the only surviving `[THEOREM]`-grade Yang-Mills claim is the narrow **per-voxel mass gap** (FTD-0044, `spec(H) ⊂ {0} ∪ [K_B, ∞)`). The frontmatter `type` and the "proof" language in §3 are reconciled downward accordingly; no numeric result is changed.

## 1. The Confinement Problem

A Millennium Prize problem in standard physics asks to prove why quarks are permanently confined within hadrons (the Mass Gap). In continuous QFT, the equations of Quantum Chromodynamics (QCD) become non-perturbative at low energies, making a first-principles proof of confinement exceedingly difficult. 

## 2. Discrete Topological Flux

In the FTD ternary architecture, SU(3) color gauge dynamics emerge naturally from the 26-neighbor Moore topological constraints. Rather than evaluating continuous gluon fields, we evaluate discrete "link variables" mapping flux across the `{-1, 0, 1}` lattice matrix.

To determine whether the binding force between two simulated quarks decreases with distance (like gravity) or increases (acting as a topological rubber band), we evaluate the Wilson Loop expectation amplitude $W(C)$ over a closed contour $C$.

![Wilson Loop Mass Gap Toymodel](../media/fig_mass_gap.png)

## 3. The Area Law Result

A massive $256^3$ spatial tensor volume was processed on GPU (RTX 5090) to measure the decay of $W(C)$ as the radius $R$ expanded.

The results demonstrated strict **Area-Law Scaling**:
$$ \langle W(C) \rangle \propto \exp(-\sigma \cdot A) $$
Where $A$ is the area enclosed by the loop and $\sigma$ is the string tension. 

If the force decreased with distance, the amplitude would scale by the Perimeter. Because it scales by the Area, separating two quarks demands an exponentially increasing amount of energy, inevitably snapping the "flux tube" and creating new quark pairs. This is an **engine-measured area-law signature** of confinement at the inserted coupling β = x₋ (a `[SELECTION]`) — strong evidence, but **not** a continuum first-principles proof: the coupling insertion is a selection rather than a derivation, and continuum-limit survival is unproven (cf. constitution §5.1 row 9; the full Yang-Mills proof is RETRACTED per FTD-0042, with only the per-voxel mass gap FTD-0044 surviving as `[THEOREM]`).

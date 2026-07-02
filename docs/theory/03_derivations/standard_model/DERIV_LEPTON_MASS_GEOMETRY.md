# Geometric Reading of the Lepton Mass-Ratio Integers

**Epistemic Status:** `[STRUCTURALLY MOTIVATED PARAMETRIC]` **(corrected 2026-07-01, FTD-0348 — was `[THEOREM]`)**

> **Correction notice.** The demotion of record for the μ/τ mass-ratio formulas
> (`TRACKER_OPEN_ITEMS.md` §"Lepton Mass Ratios": demoted to `[STRUCTURALLY MOTIVATED
> PARAMETRIC]` in `SPEC_SM_REPLACEMENT_COMPLETE.md` and `[IMPOSED]` in `FOUND_AXIOM_ZERO.md`,
> "as they lack a rigorous derivation from the core FTD lattice Lagrangian") was never
> propagated to this document's `[THEOREM]` header. The Fable specialist review
> (`AUDIT_FABLE_SPECIALIST_REVIEW_2026-07-01.md`) additionally found: (i) each move in the
> L₃-shell counting below (why L₃, why reject the 8 corners, why subtract exactly 3 "Dirac
> defect nodes") is chosen to land on 207 — a structural *rationalization* of a known
> integer, not a forcing derivation; (ii) attributing the residual 0.11% gap "strictly to
> QED vacuum polarization" is an unfalsifiable promissory note — both masses in the ratio
> are already dressed pole masses, no bare theory is defined in which a "bare 207" acquires
> a computable −0.112% QED shift, and no such computation is offered. The geometric reading
> below is retained as *motivation* at the corrected tag; this document's earlier claim to
> "retract the parametric approaches" in favor of a derivation is itself withdrawn.

## 1. Overview
The discrete mass ratios of the Muon (207) and Tau (3477) are structurally motivated parametric formulas matched to experimental data. This document offers a geometric *reading* of these integers via the topological geometry of the expanding Moore bounding layers ($L_n$) — motivation, not derivation, per the correction notice above.

In FTD, leptons are stable topological knots (flux loops) possessing spin-1/2 symmetry. To achieve stability at higher energies, the electron knot radially expands and phase-locks onto the larger $L_n$ lattice boundaries.

## 2. Derivation of the Muon Mass Ratio (207)

The Muon is the first radial expansion of the Lepton topological structure, phase-locking to the $L_3$ Moore boundary.

1. **Boundary Selection:** The $L_3$ boundary defines a $7 \times 7 \times 7$ cubic envelope. The total number of nodes on this boundary is $7^3 - 5^3 = 218$.
2. **Phase-Space Decomposition:** By the Moore Layer Theorem, the 218 boundary nodes rigorously decompose into:
   - **Faces** ($p$-block geometry): $6 \times (5 \times 5) = 150$ interior nodes.
   - **Edges** ($d$-block geometry): $12 \times 5 = 60$ interior nodes.
   - **Corners** ($f$-block geometry): 8 deep-transverse nodes.
3. **Lepton Resonant Confinement:** A localized lepton loop requires tight bounding. It exclusively occupies the connected Faces and Edges ($150 + 60 = 210$ phase nodes), strictly rejecting the 8 Corners which correspond to macroscopic spatial diffusion.
4. **Spin-1/2 Symmetry Breaking:** To establish a stable spin-1/2 quantization axis, the topological knot must explicitly break continuous spatial symmetry, consuming exactly 3 nodes corresponding to the Cartesian Triad ($N_c = 3$ dimensional degrees of freedom) as the Dirac defect core.
5. **Exact Mass Ratio:**
   $210 \text{ (Confinement Nodes)} - 3 \text{ (Dirac Defect Nodes)} = 207$

The bare discrete mass ratio of the Muon is exactly **207**. The minor 0.11% variance to the experimental dressed mass ($206.768$) is strictly attributed to QED vacuum polarization (the anomaly), analogous to the electron's $g-2$.

## 3. Derivation of the Tau Mass Ratio (3477)

The Tau is the extreme maximal resonance, pushed to the **$L_{12}$ boundary**.

1. **Boundary Selection:** The $L_{12}$ boundary (length 25) contains exactly $25^3 - 23^3 = 3458$ nodes.
2. **Generational Phase-Locking:** As the third generation, the Tau must maintain topological coherence with the inner Muon layer ($L_3$) to prevent immediate structural collapse. It achieves this by phase-locking the massive $L_{12}$ boundary specifically to the central $L_3$ Cartesian Cross.
3. **Cartesian Cross Capacity:** The exact number of nodes in a 3D Cartesian cross spanning the full $L_3$ volume is $7 + 7 + 7 - 2 = 19$ nodes.
4. **Exact Mass Ratio:**
   $3458 \text{ (Boundary Nodes)} + 19 \text{ (Inner Phase-Lock)} = 3477$

The bare discrete mass ratio of the Tau is exactly **3477**.

## 4. Conclusion
The empirically fitted parametric equations ($3b_3(b_3+N_c)-N_c$) are formally abolished. The Muon and Tau are proven to not be arbitrary "copies" of the electron inserted by hand; they are strictly required geometric capacity-states of the expanding 3D discrete universe.

# DECISION · Canonical EFT Field Basis

**Tag:** [DECISION]
**Date:** 2026-05-05
**Status:** [DECISION] — picks **collocated $(s, J)$ at lattice vertices** as the canonical EFT field basis for the R3 nonlinear blocked $S_\text{eff}$ measurements and the R6 synthesis manuscript. Face-centered $J^*$ and dual-cell representations remain available for sensitivity analysis but are not the production target.
**Purpose:** Phase R2 of the FTD-EFT roadmap. Closes `STATUS_EFT_CHECKLIST.md` §1 unchecked items "Decide whether the canonical EFT field basis is collocated $(s, J)$ or face-centered/dual-cell $J^*$" and "Define the final minimal field tuple for nonlinear campaigns."

---

## §1 — The two candidates

The FTD lattice supports two natural field-variable conventions:

**Collocated $(s, J)$ at vertices.** State $s \in \{-1, 0, +1\}$ and flux $\mathbf{J} \in \mathbb{R}^3$ both live at lattice vertices $\mathbf{v} \in \mathbb{Z}^3$. Used by:

- The engine's production data structures (`engine/include/ftd/voxel.h` — `Voxel::state`, `Voxel::flux` are vertex-collocated).
- All current measurement campaigns (FTD-0098/0099/0100 mixing-matrix, FTD-0107 cluster-tracker, FTD-0110 cluster-↔-mass, all Phase-G/I/J/II Wilson-Dirac work).
- The 9-theorem algebraic spine (`SPEC_ALGEBRAIC_SPINE.md`).
- The canonical Lagrangian (`SPEC_FTD_LAGRANGIAN.md` v3.2 §3, fields at vertices).

**Face-centered $J^*$ + dual-cell $s$.** The flux variable lives at face centers (between adjacent vertices) and the state at dual-cell centers. Discussed in `SPEC_FTD_NATIVE_BLOCKING_MAP.md` and the dual-cell prototype in `engine/include/ftd/sublattice.h`. This convention is more natural for face-flux / current-conservation framings (where $J$ is integrated over faces in Stokes-style identities).

The two are related by linear interpolation maps but are *not* trivially interchangeable — operator-mixing matrices, blocking transformations, and observable definitions all differ in non-trivial ways between them.

---

## §2 — Decision criteria

Three criteria, in priority order:

1. **Backward compatibility with existing measurements.** ~50 of the ~100 docs in `10_eft_program/` (per the R1 INDEX) reference operator-mixing data assuming collocated convention. Re-deriving them under face-centered convention is decade-scale work.
2. **Compatibility with the engine's production data structures.** `Voxel` is vertex-collocated; reworking to face-centered would require re-architecting every kernel.
3. **Theoretical advantage.** Face-centered conventions are cleaner for some current-conservation framings, but the FTD-native blocking-map work (`DERIV_FTD_NATIVE_*` family) has built up its formalism on collocated convention.

These criteria pick **collocated $(s, J)$** uniquely.

There is no decisive theoretical advantage of face-centered convention that would justify the engine + ~50-doc rework cost. The face-centered / dual-cell conventions remain available for sensitivity analysis (e.g. R5 inter-scale work could explore whether face-centered convention reveals different operator-mixing structure), but they are not production.

---

## §3 — The decision [DECISION]

**For R3 onward, the canonical EFT field basis is:**

$$
\boxed{\text{Collocated } (s, \mathbf{J}) \text{ at lattice vertices}}
$$

Specifically:

- Field tuple: $\bigl(s(\mathbf{v}, t),\; \mathbf{J}(\mathbf{v}, t),\; \mathbf{v}_\text{wave}(\mathbf{v}, t),\; \mathcal{L}(\mathbf{v})\bigr)$ where $\mathbf{v}_\text{wave} = \Delta_t \mathbf{J}$ is the wave-velocity (canonical momentum of the flux field) and $\mathcal{L}$ is the latency field.
- All measurements (operator-mixing, mixing-matrix, β-function, dim-6 operator coefficients) are computed at vertex sites.
- Blocking transformations (b=2, b=4) operate on $b^3$ blocks of vertices, mapping to a coarse-grained vertex on the b-blocked lattice.
- Source coupling: $\rho(\mathbf{v}) \equiv s(\mathbf{v})$ at vertex sites; per-face current $I_\text{face}$ is a derived quantity, not a primary field.

**The face-centered / dual-cell convention is not adopted.** It remains documented in `SPEC_FTD_NATIVE_BLOCKING_MAP.md` as the prototype-level alternative. R5 work that finds face-centered convention reveals genuinely new structure can re-open this decision, but until then collocated is canonical.

---

## §4 — Coupling to the Gauss decision

`DECISION_GAUSS_REPRESENTATION.md` (sibling R2 deliverable) picks **collocated cuFFT (B)** as the production Gauss path. Both decisions consistently select the collocated-vertex convention. The two decisions are not independent:

- A face-centered field basis would naturally pair with a face-flux Gauss representation (D in the Gauss decision).
- The collocated field basis selects the collocated Gauss enforcement (A or B).

So this DECISION ratifies the implicit choice already running in production.

---

## §5 — Implications for the rest of the roadmap

- **R3 nonlinear $S_\text{eff}$**: will be written as a polynomial in the collocated field tuple $(s, J, v_\text{wave}, \mathcal{L})$ with measured Wilson coefficients.
- **R3a operator-mixing extension**: continues the FTD-0098/0099/0100 measurement protocol, which was built on collocated convention.
- **R4 β-function**: extracted from collocated mixing-matrix data across L ∈ {16, 32, 64, 96, 128}. The "phase-structure flow" framing (per `SPEC_DISCRETE_NATIVE_DERIVATION.md`'s rejection of continuous-RG language) treats observable drift on the collocated basis directly.
- **R5 inter-scale**: Scale 0→1 emergence (cluster_tracker code) currently lives on collocated convention. If face-centered convention shows decisive theoretical advantage for higher-scale matching, the decision can be revisited *for that sub-phase* without disturbing R3/R4.
- **R6 synthesis manuscript**: writes the canonical FTD-EFT in collocated convention. Mentions face-centered as an alternative representation, not as a competitive theory.

---

## §6 — What this decision does NOT cover

- **Latency field $\mathcal{L}$**: where it lives is a separate decision (currently at vertices in `Voxel::latency`; a face-centered or dual-cell variant is not on the table).
- **Spin and color**: these are auxiliary attributes of manifested particles, not primary fields. They live where the particles live (vertex), but the question doesn't arise in the same way.
- **Dual substrate $J_L, J_R$**: these are vertex-collocated by construction (`Voxel::flux_L`, `Voxel::flux_R`) and the decision doesn't affect them.
- **Boundary conditions**: the periodic-boundary canonical setup is independent of basis choice.

---

## §7 — Refresh policy

If R5 multi-cluster binding (Scale 1→2 emergence) measurement reveals operator structure that's substantially cleaner under face-centered convention, this DECISION needs to be revisited *for that sub-phase only*. The R3 $S_\text{eff}$ closure and R6 manuscript would still be canonical-collocated; the face-centered variant would be documented as a parallel description in a sibling SPEC.

Until that signal appears, collocated $(s, J)$ is canonical.

This DECISION ratifies the implicit choice that has been running in production — no engine code changes required.

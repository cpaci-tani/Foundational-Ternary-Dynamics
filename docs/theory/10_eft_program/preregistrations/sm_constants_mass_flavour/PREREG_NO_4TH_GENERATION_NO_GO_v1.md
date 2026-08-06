# PREREG — No 4th Generation Fermions No-Go Formalization Campaign

**Status:** [PRE-REGISTRATION — hash-locked before execution]
**Date:** 2026-05-27
**Campaign ID:** FTD-0220
**Funder / Context:** Moore Layer Theorem / Standard Model generation count

---

## §1 — Objective & Process Protocol

This document pre-registers and lock-secures the design of the **No 4th Generation Fermions No-Go Formalization Campaign (FTD-0220)**.

The Moore Layer Theorem (`THEOREM_MOORE_LAYER_DECOMPOSITION.md`) establishes that the Standard Model's combinatorial features (forces, gauge groups, generations) arise natively from the geometric structure of the 3D ternary cubic lattice under the 26-neighbor Moore neighborhood. Specifically, the three generations of four fermions correspond to the $C(3, 2) = 3$ face-diagonal planes of the cuboctahedral layer ($k=2$), each containing $2^2 = 4$ sites.

To establish this as a rigorous first-principles no-go result rather than a post-hoc match, we must formalize why a **fourth generation of Standard Model fermions** is mathematically impossible under the FTD coordinate ontology, and show that any such discovery would immediately falsify the framework's discrete spatial structure.

### Methodological Protocol:
1. This pre-registration document is committed and git-tagged `preregister-no-4th-generation-no-go-v1` before any numerical code is executed or results are generated.
2. The SHA256 of this file is recorded in the manifest `REF_PREREGISTER_MANIFEST.md` and the campaign is reserved in `LEDGER.md`.
3. The verification script `scripts/exploration/verify_no_4th_generation.py` is written and executed.
4. The final outcomes and group-theoretic proofs are published in the result document `docs/theory/10_eft_program/FOUND_NO_4TH_GENERATION_NO_GO.md`.

---

## §2 — The Central Question

**Q-NO-4TH-GEN:** How does the 26-Moore neighborhood representation theory and the $O_h$ point-group polyhedral decomposition uniquely constrain the number of active fermion generations to exactly $C(D, 2) = 3$ at $D=3$ spatial dimensions, and does this mathematical structure gauge-theoretically and topologically exclude any standard fourth generation of fermions?

---

## §3 — Definitions & Formalisms

*   **Definition D1 (Moore Shell k=2 Generation Map):** The $k=2$ (cuboctahedral) layer of the $D$-dimensional Moore neighborhood consists of all sites with exactly two nonzero coordinates in the offset representation. The number of independent face-diagonal planes (each orthogonal under the standard coordinate projection) is:
    $$N_{\text{gen}} = C(D, 2) = \frac{D(D-1)}{2}$$
*   **Definition D2 (Per-Plane Site Multiplicity):** Each face-diagonal plane in the $k=2$ layer consists of sites where exactly two coordinate offsets are active ($\pm 1$). The number of sites per plane is:
    $$N_{\text{fermion}} = 2^2 = 4$$
    representing the four standard fermion types per generation (neutrino, electron, up-quark, down-quark).
*   **Definition D3 (Standard 4th Generation):** A fourth generation of fermions with standard gauge couplings (SU(3) color, SU(2) weak, U(1) hypercharge). In FTD, this requires adding a fourth orthogonal plane or setting $N_{\text{gen}} \ge 4$.
*   **Definition D4 (Dimension and Gauge Factorization):** The unique spatial dimension $D=3$ selected by the 26-Moore neighborhood representation theory, which factorizes the vector field components $J^2 = \sum J_i^2$ into the Standard Model gauge group U(1) x SU(2) x SU(3) by the concentric polyhedral shells at distance $\sqrt{k}$.

---

## §4 — Admissible Search Space & Symmetries

The mathematical search is strictly confined to:
1.  Discrete cubic coordinate lattices $\mathbb{Z}^D$ under the point group $O_h$ and its $D$-dimensional generalizations (hyper-octahedral groups).
2.  concentric polyhedral shells of the Moore neighborhood.
3.  Active fermions restricted to the $k=2$ (cuboctahedral) layer per the Moore Layer Theorem.

---

## §5 — Three Pre-Blessed Outcomes

*   **Outcome A (FOUND):** We rigorously prove that $N_{\text{gen}} = 3$ is uniquely selected under the $D=3$ Moore layer decomposition, and any 4th generation is algebraically and topologically excluded because $C(D, 2) = 4$ has no integer solution for $D$, and any $D > 3$ fails to preserve the U(1)xSU(2)xSU(3) gauge group factorization of the Moore neighborhood.
*   **Outcome B (UNDERDETERMINED):** We show that $N_{\text{gen}} = 3$ holds, but the exclusion of a fourth generation allows loopholes (e.g. sterile dark-matter states not bound to standard cuboctahedral planes).
*   **Outcome C (CLOSED-NEGATIVE):** We find that a fourth generation can be naturally accommodated without violating the axioms, point-group symmetries, or causality speed limits.

---

## §6 — Falsifiers

*   **F-a (Dimension flexibility):** Spatial dimensions $D \ge 4$ can yield the standard Standard Model gauge groups and generation structures, meaning $D=3$ is not uniquely forced.
*   **F-b (Integer solution existence):** There exists an integer dimension $D$ such that the number of face-diagonal planes $C(D, 2)$ is exactly 4, allowing a 4th generation.
*   **F-c (Alternative layer allocation):** A standard 4th generation can be constructed on the $D=3$ lattice using other Moore layers (octahedron $k=1$ or stella octangula $k=3$) without violating the gauge or parity symmetries.

---

## §7 — Banned Moves

*   **B-1 (Posterior dimension tuning):** Postulating non-integer or fractional coordinate dimensions to force a fourth generation.
*   **B-2 (Attribution trailer):** Banned AI co-author or generator footer additions to the commit message.

---

## §8 — Verification Procedure

1.  Write the verification script `verify_no_4th_generation.py` in Python.
2.  Compute the D-table properties up to $D=10$, showing that $C(D, 2) = 3$ is the unique solution for $D=3$, and no integer $D$ yields $C(D, 2) = 4$.
3.  Analyze the representation dimensions of $O_h$ and prove that the 12 cuboctahedron sites can only be partitioned into exactly 3 generations of 4 fermions under the $C_2$ projection symmetries.
4.  Run the full verification suite `proof_master_verification.py` and `test_all_physics.py` to ensure zero regressions.
5.  Publish the final verdict in `docs/theory/10_eft_program/FOUND_NO_4TH_GENERATION_NO_GO.md`.

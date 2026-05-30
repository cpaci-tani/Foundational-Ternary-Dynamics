# Pre-Registration — MC-T4.3 ARC-C1 Alpha Quantization Readout (v1)

**Tag:** [PRE-REGISTRATION] — this document locks the design of the ARC-C1 Alpha Quantization readout attempt against the central foundational obstruction MC-T4.3 (the operational electric charge readout rule under the `SPEC_ALPHA_READOUT_CONTRACT.md` framework). It contains **no result**. All three outcomes — FOUND / UNDERDETERMINED / CLOSED-NEGATIVE — are pre-blessed.

**Date:** 2026-05-27  
**Hash-lock target tag:** `preregister-alpha-readout-quantization-v1`  
**LEDGER row reservation:** FTD-0231  
**Companion docs:** `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md`, `../10_eft_program/AUDIT_CHARGE_QUANTIZATION_NO_CHEAT.md` (FTD-0231, charge quantization audit).

---

## §1 — Context and Doctrine

### 1.1 The Charge Normalization Obstruction
Prior attempts to derive the fine-structure constant ($\alpha$) from continuous action-based mechanisms or generic lattice operators faced a fundamental mismatch:
* **QED Normalization:** The physical charge $e_0$ is continuous and runs with energy scale, whereas FTD voxel state configurations are strictly discrete: $s(x, t) \in \{-1, 0, +1\}$.
* **Lehniscatic Asymmetry:** The physical coupling is characterized by modular periods such as $G^* = \Gamma(1/4)/\Gamma(3/4)$ over the Gaussian integers $\mathbb{Z}[i]$.

### 1.2 The Quantization / Readout Rule (Candidate C)
To resolve this, Candidate C proposes mapping the coupling not via a continuous gauge-field prefactor, but as a **discrete topological index (winding number)** on the $\mathbb{Z}[i]$-module structure of $V_{\text{complex}}$ in the BCC lattice:
$$ V_{\text{complex}} \cong \mathbb{Z}[i]^2 $$

By formulating the measurement of electric charge $e^2$ as a topological winding invariant, the coupling is structurally bound to the arithmetic periods of $\mathbb{Z}[i]$, deriving $1/x_+$ directly without target matching.

---

## §2 — The Question

**Q-ARC-C1.** Does there exist an ARC tuple `(P, A_obs, O_EM, R, C)` such that:
1. `A_obs` is the gauge-invariant observable algebra representing a topological winding index on the $\mathbb{Z}[i]$-complex manifold $V_{\text{complex}}$ of the BCC lattice;
2. the measurement functional `O_EM` reads the topological index $\text{Ind}(y)$ of projected configurations;
3. the readout map `R` derives the inverse fine-structure coupling $1/\alpha = x_+$ solely from the arithmetic invariants and periods of the Gaussian integers;
4. the construction is admissible under the hard exclusion rules of `SPEC_ALPHA_READOUT_CONTRACT.md`?

---

## §3 — Definitions & Admissible Primitives

- **D1 — ARC-C1 Tuple:** `ARC = (P, A_obs, O_EM, R, C)` per the FTD contract.
- **D2 — Topological Index Functional:** A functional mapping projected configurations in $V_{\text{complex}}$ to their winding numbers:
  $$ \text{Ind}(y) = \frac{1}{4} \sum_{k=0}^{3} \text{Im} \left( \frac{\langle y, J^k y \rangle}{\|y\|^2} \right) $$
- **D3 — Admissible Primitives:** Solely FTD-native ternary configurations, Gaussian integer coordinates, and cyclic rotation operator $J$.

---

## §4 — The Three Pre-Blessed Outcomes

### FOUND
An ARC-C1 tuple is exhibited with a rigorous derivation trace. The topological index maps directly to the modular periods of $\mathbb{Z}[i]$, yielding the master quadratic roots $x_\pm$ as stable fixed points. The relative error to the tree-level master quadratic is $0$.

### UNDERDETERMINED
A partial topological map is achieved, but the relation between the winding number and QED charge normalization is not fully derived or remains dependent on continuous field limits.

### CLOSED-NEGATIVE
The topological index fails to produce the master quadratic roots or requires manual parameter insertion violating the circularity rules.

---

## §5 — The Falsifier Checklist

The attempt is immediately falsified (proceeds to UNDERDETERMINED or CLOSED-NEGATIVE) if:
- **F-a:** Uses physical $\alpha$, CODATA $1/\alpha$, or a measured QED coupling as input.
- **F-b:** Contains a free parameter tuned to force the output to $137.036...$.
- **F-c:** Fails dominant-branch selection: no FTD-internal reason selects $x_+$ over $x_-$.
- **F-d:** Fails operational protocol: the observable has no physical measurement interpretation.
- **F-j:** Reverse-engineers the topological relation by inserting the target master quadratic as a scaffold.

---

## §6 — The 10-Step Method

The closure attempt must execute exactly these steps in order:
1. State the proposed ARC-C1 tuple `(P, A_obs, O_EM, R, C)`.
2. Construct the topological winding index $\text{Ind}(y)$ on the complex representation subspace $V_{\text{complex}}$.
3. Prove that the index values represent discrete topological windings under $\mathbb{Z}[i]$ rotations.
4. Define the operational measurement protocol connecting this winding index to electric charge.
5. Formulate the transfer map on the topological states.
6. Derive the characteristic equation of the transfer map.
7. Compare the characteristic equation to the master quadratic roots.
8. State the dominant-branch selection rule.
9. Apply the falsifier checklist mechanically.
10. Report the final verdict (FOUND / UNDERDETERMINED / CLOSED-NEGATIVE).

# Exploration: Topological-Drag α Derivation — Audit

**Date:** 2026-04-24
**Status:** [CONJECTURE] of the physical claim; [THEOREM] of the tautology observation
**Supersedes:** α-derivation claim in "Generated Document April 24, 2026 – 12:47 AM.pdf" (§2, labelled there as THEOREM 2)
**Depends on:** [DERIV_MASTER_QUADRATIC_GAP_EQUATION.md](../03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md), [SPEC_EFT_RECOVERY_PROGRAM.md](../10_eft_program/scopes_and_specs/SPEC_EFT_RECOVERY_PROGRAM.md)
**Ledger row:** FTD-0062

---

## 1. The proposed derivation

The PDF draft states:

> Master quadratic gives $x_+ = 137.036$. "Total topological drag" $\lambda_0 = 18/x_+ \approx 0.13135$. By Phase-G isotropy of the 18-point Moore Laplacian, this drag distributes evenly across 18 escape routes. Therefore
>
> $$ \alpha_{\mathrm{FTD}} = \frac{\lambda_0}{N_{\mathrm{routes}}} = \frac{18/x_+}{18} = \frac{1}{x_+} = \frac{1}{137.036}. $$
>
> "$\alpha$ is mathematically derived without continuum inputs."

## 2. The algebra, made explicit

Unpacking the definitions in order:

- Step 1. $x_+ = 137.036171\ldots$, the root of the master quadratic. This is [THEOREM] at the arithmetic level (see [DERIV_MASTER_QUADRATIC_GAP_EQUATION.md](../03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md)).
- Step 2. Define $\alpha \equiv 1/x_+$. This is the **original conjecture** the bridge program is trying to prove, tagged in the current [LEDGER.md](../07_assessment/core_ledgers/LEDGER.md) and in CLAUDE.md as [STRONGLY MOTIVATED CONJECTURE].
- Step 3. Define $\lambda_0 := 18 / x_+ = 18 \alpha$. This uses Step 2.
- Step 4. Divide: $\alpha = \lambda_0 / 18 = (18 \alpha) / 18 = \alpha$.

The algebraic chain is

$$ \alpha = \underbrace{\frac{\lambda_0}{18}}_{\text{Step 4}} = \underbrace{\frac{18\alpha}{18}}_{\text{substitute Step 3}} = \alpha. $$

This is a **tautology**. The quantity $\lambda_0$ is defined in Step 3 as $18\alpha$, and then Step 4 recovers $\alpha$ by dividing by 18. No information has been extracted from the engine. The physical content — that the dynamical "drag" quantity in the FTD engine equals $18\alpha$ — is **assumed in the definition of $\lambda_0$**, not derived.

**[THEOREM] (the derivation is circular as stated).** The chain $\alpha = \lambda_0/18$ with $\lambda_0 \equiv 18/x_+$ and $\alpha \equiv 1/x_+$ is a rearrangement of $\alpha = \alpha$. It does not establish $1/x_+ = \alpha_{\mathrm{QED}}$.

## 3. What "Phase-G isotropy" does and does not supply

The PDF appeals to Phase-G isotropy to justify the equipartition step "drag distributes evenly across 18 escape routes." Phase-G isotropy is a statement about the Green's function of the 18-point Moore Laplacian at long distance — specifically that the leading $1/r$ term is rotationally symmetric to $O(1/r^3)$.

Phase-G isotropy does **not** entail scalar equipartition of an unspecified "drag" quantity over 18 directions. These are different structural claims:

| Claim | Content | Status |
|---|---|---|
| Phase-G isotropy | The Green's function $G(\mathbf r)$ of $\Delta_{18}$ is rotationally symmetric to leading order | [THEOREM] |
| Scalar quantity distributes as sum over 18 face/edge directions | A coefficient $\lambda_0$ can be split into 18 equal parts | Requires a definition of what is being distributed, and a conservation or weight argument |
| Therefore $\alpha = \lambda_0/18$ | Combining the above | [OPEN] — the chain from Green's-function isotropy to scalar equipartition is not written down |

Even if one supplies an equipartition argument, Step 3's identification $\lambda_0 = 18/x_+$ still has to be **derived**, not assumed. Only then does the chain carry content.

## 4. What the engine actually measures

The proposed derivation is contradicted by direct measurement. The EFT Recovery Program ([SPEC_EFT_RECOVERY_PROGRAM.md](../10_eft_program/scopes_and_specs/SPEC_EFT_RECOVERY_PROGRAM.md)) pre-registered the measurement of the lattice correction to $\alpha$ on FTD's own engine and found

$$ \alpha_\infty \;\approx\; 3.6 \times \alpha_{\mathrm{ref}} \quad\text{across } L \in \{64, 128, 256, 384\}. $$

If the identity $\alpha_{\mathrm{FTD}} = 1/x_+$ were the correct dynamical outcome at the level of the engine, the measured $\alpha_\infty / \alpha_{\mathrm{ref}}$ would be $1 \pm$ finite-L corrections. Instead it plateaus at $\sim 3.6$ with three independent scaling laws agreeing on $\alpha_\infty \in [3.35, 3.74] \times \alpha_{\mathrm{ref}}$.

This is the documented reason the current FTD-to-QED-$\alpha$ bridge is [CLOSED NEGATIVE] (see [SPEC_FTD_EFT_BRIDGE_CONTRACT.md](../10_eft_program/SPEC_FTD_EFT_BRIDGE_CONTRACT.md) and FTD-0058 in the ledger). The PDF's §2 therefore does not just fail algebraically — it contradicts an engine measurement specifically designed to test the claim.

## 5. What would promote this to a theorem

**G1. Independent definition of $\lambda_0$.** Define $\lambda_0$ as a dynamical observable on the engine **without** reference to $x_+$. Candidates:
- Free-energy cost of manifesting a unit charge at the origin in a finite volume $L^3$.
- Coefficient of $1/r$ in the long-distance Green's function of $\Delta_{18}$.
- Damping rate extracted from a flux relaxation experiment at zero source.

**G2. Measurement.** Compute the chosen $\lambda_0$ on the engine over $L \in \{32, 64, 128, 256\}$, extrapolate, and report the continuum-limit value.

**G3. Comparison.** Ask whether that measured $\lambda_0$ satisfies $\lambda_0 = 18/x_+$. This is a **prediction** of the conjecture, testable without circular reasoning.

**G4. If confirmed.** Write the equipartition argument that converts $\lambda_0$ to $\alpha$. Pass-through at this step requires an argument tighter than "Phase-G is isotropic" — specifically a conservation law or sum rule that forces the 1/18 split.

**G5. Reconcile with the $\alpha_\infty \approx 3.6 \alpha_{\mathrm{ref}}$ measurement.** Either show the EFT Recovery Program's observable is a different quantity that need not equal $\alpha$, or identify a discrepancy in that measurement. Until one of these is done, the conjecture and the measurement disagree.

Until G1–G5 are passed, the claim is a restatement of "$x_+ = 1/\alpha$" in different notation, not a derivation.

## 6. Epistemic tag

| Piece | Tag | Justification |
|---|---|---|
| $x_+ = 137.036171\ldots$ root of master quadratic | [THEOREM] | Arithmetic |
| $x_+ = 1/\alpha_{\mathrm{QED}}$ | [STRONGLY MOTIVATED CONJECTURE] | Unchanged from LEDGER |
| "$\lambda_0 = 18/x_+$ is the engine's total topological drag" | [CONJECTURE] | Needs independent measurement |
| "Phase-G isotropy $\Rightarrow$ scalar equipartition over 18 routes" | [OPEN] | Derivation not written |
| "$\alpha_{\mathrm{FTD}} = \lambda_0/18 = 1/x_+$ is a derivation" | [CLOSED NEGATIVE (tautology)] | §2 above — the algebra is circular |
| Compatibility with engine-measured $\alpha_\infty \approx 3.6 \alpha_{\mathrm{ref}}$ | [OPEN NEGATIVE] | Disagreement at factor-of-3.6 level |

## 7. Relation to existing FTD work

- [CONJ_ALPHA_FROM_CM.md](CONJ_ALPHA_FROM_CM.md) already carries the $\alpha \leftrightarrow x_+$ bridge as [CONJECTURE]. This file adds the observation that the proposed "derivation via topological drag" is a tautology and does not move the claim along the tag hierarchy.
- [SPEC_EFT_RECOVERY_PROGRAM.md](../10_eft_program/scopes_and_specs/SPEC_EFT_RECOVERY_PROGRAM.md) records the 3.6× discrepancy.
- [OPEN_FTD_TO_EFT_BRIDGE_STATUS.md](../10_eft_program/OPEN_FTD_TO_EFT_BRIDGE_STATUS.md) (2026-04-22) explicitly states "the QED-alpha bridge is [CLOSED NEGATIVE] under the current projected action" and lists this kind of derivation attempt as not load-bearing.

---

*Filed 2026-04-24 in response to a PDF draft that labelled §2 as THEOREM 2. The tautology identification is a small theorem; the underlying bridge $x_+ \leftrightarrow \alpha$ remains conjectural exactly as the project ledger records.*

# Exploration: α/42 as a Candidate 174-ppm Proton/Electron Mass Correction

**Date:** 2026-04-24
**Status:** [CONJECTURE] — numerical match with no derivation
**Supersedes:** mass-gap closure claim in "Generated Document April 24, 2026 – 12:47 AM.pdf" (§3, labelled there as THEOREM 3)
**Depends on:** [DERIV_COMPLETE_PARTICLE_PHYSICS.md](../05_particles/DERIV_COMPLETE_PARTICLE_PHYSICS.md), [LEDGER.md:FTD-0016](../07_assessment/core_ledgers/LEDGER.md), [LEDGER.md:FTD-0060](../07_assessment/core_ledgers/LEDGER.md)
**Ledger row:** FTD-0063

---

## 1. The numerical observation

The current FTD structural formula (FTD-0016 in the ledger, tagged [STRONGLY MOTIVATED CONJECTURE]) gives

$$ \frac{m_p}{m_e}\bigg|_{\mathrm{FTD}} = \frac{N_{\mathrm{eff}}}{\alpha} + N_{\mathrm{base}} N_{\mathrm{eff}} + N_c = 13 \cdot 137.036 + 55 = 1836.468. $$

Experimental PDG value: $1836.15267344(15)$.

Residual: $+0.315 / 1836.15 \approx +172$ ppm.

The conjecture in the PDF draft is that this residual is $\alpha / 42$:

$$ \frac{\alpha}{42} = \frac{1}{137.036 \cdot 42} \approx 173.7 \text{ ppm}. $$

The proposed correction is

$$ \left(\frac{m_p}{m_e}\right)_{\mathrm{phys}} = 1836.468 \cdot \left(1 - \frac{\alpha}{42}\right) = 1836.149, $$

matching PDG to $0.0016\%$ ($\sim 2$ ppm).

## 2. Why the match is not a theorem

Three structural issues prevent promotion to [DERIVED] or [THEOREM].

### 2.1 It is a near-miss search

The ppm residual is a scalar $\approx 172$. The FTD integer catalog (framework integers, Heegner / class-number-1 integers, small products of master-quadratic coefficients, Moore-layer counts) contains dozens of small products with order-of-magnitude $10^{-4}$ when divided by $\alpha$. Representative candidates in the 100–200 ppm band:

| Denominator $d$ | $\alpha/d$ (ppm) |
|---|---|
| 36 | 202.7 |
| 38 | 192.0 |
| 40 | 182.4 |
| 42 | 173.7 |
| 44 | 165.9 |
| 47 | 155.3 |
| 48 | 152.0 |

The target is $\sim 172$ ppm. $d = 42$ is the closest entry in a set of nearby candidates. CLAUDE.md §"Epistemic Discipline" explicitly prohibits:

> Do NOT run numerical search scripts looking for near-misses or coincidences.
> Do NOT create substitution identities (plugging FTD values into formulas and calling the result a discovery).

The identification $d = 42 = 1 \cdot 2 \cdot 3 \cdot 7$ as "the first four Heegner integers" is a post-hoc label. Heegner numbers of class number 1 are $\{-1, -2, -3, -7, -11, -19, -43, -67, -163\}$. Selecting $\{1, 2, 3, 7\}$ as "the first four" requires dropping the sign and stopping at the 4th entry — a choice motivated by the target.

### 2.2 No mechanism is written

A derivation of a lattice self-energy correction at one-loop would start from the FTD action, write the self-energy diagram on the engine's 18-point Moore stencil, extract the finite-$L$ correction, and show the coefficient falls out of the geometry.

The PDF draft substitutes: "in lattice gauge theory, the lowest-order discrete finite-volume self-energy correction scales inversely with the structural background integer." That is a sentence, not a calculation. No diagram is drawn, no propagator is written, no Moore-stencil integral is evaluated. The coefficient $\alpha/42$ is asserted and then matched.

Compare with the corpus's genuine one-loop derivations: $c_1 = 9/47$, $c_2 = 5/64$, $c_3 = 4/141$ were computed from specific lattice Feynman diagrams with the denominators traceable to neighborhood counts. $\alpha/42$ is not in that category.

### 2.3 The "proton absorbs its drag" clause is post-hoc

The PDF asserts that the correction attaches to the electron inertia and not the proton, on the grounds that the proton "absorbs its lattice drag into the $T(10)$ bulk binding energy." This has to be true for the numbers to work: attaching the correction to the proton makes $m_p/m_e$ grow instead of shrink and the match fails.

No independent argument is given for why the correction sits on the electron DoF. The choice is made to match the target — that is a fit parameter, not a structural argument.

### 2.4 The engine contradicts the required order of magnitude

The EFT Recovery Program measured the lattice correction to $\alpha$ on FTD's own engine and found $\alpha_\infty \approx 3.6 \alpha_{\mathrm{ref}}$ (see [SPEC_EFT_RECOVERY_PROGRAM.md](../10_eft_program/scopes_and_specs/SPEC_EFT_RECOVERY_PROGRAM.md) and FTD-0058). A claimed lattice self-energy at the $\alpha/42 \approx 10^{-4}$ level sits at a completely different scale than the engine's actually-measured finite-$L$ correction. Either the engine's EFT Recovery observable is the wrong one for this comparison, or the $\alpha/42$ correction is on a separate mechanism the engine does not currently resolve, or the match is incidental.

## 3. What would promote this to a theorem

**G1. Derive the coefficient from a diagram.** Write the electron-self-energy loop on the Moore-18 stencil. Extract the finite-$L$ correction. Show the leading coefficient equals $1/42$ (or whatever the geometry gives) — without assuming the answer.

**G2. Explain the DoF choice.** Independently of the numerical match, derive why the correction attaches to the electron self-energy and not the proton's. Candidate mechanisms that would need to hold: electron as point defect vs. proton as extended composite, spin-½ vs spin-½ (no asymmetry there), binding-energy absorption (requires quantitative model).

**G3. Engine measurement.** Prepare p/e-like defect configurations on the engine, measure the mass shift as a function of $L$, extrapolate, and compare against the predicted $\alpha/42$. Do not calibrate by the target value.

**G4. Reconcile with the 3.6× α measurement.** Identify the relationship between the $\alpha/42$ claim and the EFT Recovery Program's $\alpha_\infty / \alpha_{\mathrm{ref}}$ plateau. One of the two needs to give.

**G5. Explain why $42 = 1 \cdot 2 \cdot 3 \cdot 7$ appears.** If the "first four Heegner integers" label is real, show where the selection rule $\{1, 2, 3, 7\}$ comes from in FTD axioms — not in the target value.

## 4. Honest status vs. the ledger

The current ledger states:

- **FTD-0016**: $m_p/m_e$ formula: [STRONGLY MOTIVATED CONJECTURE], 174 ppm residual unexplained.
- **FTD-0060** (2026-04-23): the conjectured $K_{\mathrm{comp}} = m_e/\pi$ correction CLOSED NEGATIVE; the 174-ppm gap **remains [OPEN]**.

This file adds FTD-0063 to the ledger as a second specific closed-negative **attempt** at that same gap, leaving the gap itself [OPEN].

## 5. Epistemic tag

| Piece | Tag | Justification |
|---|---|---|
| Bare formula $m_p/m_e = N_{\mathrm{eff}}/\alpha + N_{\mathrm{base}} N_{\mathrm{eff}} + N_c = 1836.468$ | [STRONGLY MOTIVATED CONJECTURE] | FTD-0016, unchanged |
| Residual $\approx 172$ ppm vs PDG | [OBSERVATION] | Arithmetic |
| Numerical match $\alpha/42 \approx 173.7$ ppm | [OBSERVATION] | Arithmetic |
| "$\alpha/42$ is the lattice self-energy correction" | [CONJECTURE] | No diagram, no mechanism |
| Correction attaches to electron DoF and not proton | [CONJECTURE] (post-hoc) | Chosen to match |
| 174-ppm gap closed by $\alpha/42$ | [CLOSED NEGATIVE as "derivation"] / [OPEN as "gap"] | §2 |

## 6. Preserved record

The numerical arithmetic of §1 is correct and worth keeping on the shelf as a CONJECTURE. The identification may eventually be derived — the project has seen that happen for other ratios. But labelling it [THEOREM] while the diagram is missing, the DoF assignment is post-hoc, and the engine disagrees by a factor of 36,000 in magnitude would contradict the ledger and the project's own epistemic discipline.

---

*Filed 2026-04-24 in response to a PDF draft that labelled §3 as THEOREM 3. Preserves the numerical observation as a [CONJECTURE] candidate; the 174-ppm gap itself remains [OPEN].*

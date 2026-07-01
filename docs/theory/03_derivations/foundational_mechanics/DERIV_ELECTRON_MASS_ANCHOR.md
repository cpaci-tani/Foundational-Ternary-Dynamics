# [STRONGLY MOTIVATED CONJECTURE] The Electron Mass Anchor ($16/3$)

**Epistemic Status:** `[STRONGLY MOTIVATED CONJECTURE]`

> **CORRECTION (adjudicated):** the `[THEOREM]` promotion (commit `fdc483d0`)
> is **RETRACTED** — the "Dimensional Equipartition" step is a substitution identity, not a
> forcing chain; it fails the FTD-0097/0189 look-elsewhere bar and the standing
> zero-promotion discipline. **Honest tag: `[STRONGLY MOTIVATED CONJECTURE]`.**
> **Genuine motivation:** the exponent $n=11$ is `[DERIVED]`, and $16/3$ is the simplest
> rational within 0.2% of the empirically-required prefactor at that exponent (1-of-2 within
> 1% across 6489 combinations) — real structural tightness, but no dynamical derivation of the
> prefactor. `[THEOREM]` is re-earnable only behind a pre-registered look-elsewhere control.

## 1. The Calibration Anchor
In the FTD dimensional mapping, the absolute rest mass of the electron serves as the calibration anchor bridging the dimensionless discrete lattice to the physical dimensioned continuum:
$$ m_e = m_P \cdot \sqrt{2\pi} \cdot \left(\frac{16}{3}\right) \cdot \alpha^{11} $$

The formula reproduces $m_e$ to **0.19%**. Two pieces carry genuine structural weight, and one does not:

## 2. What is genuinely motivated

### 2.1 The exponent $n = 11$ — `[SELECTION]` (corrected 2026-07-01 — was `[DERIVED]`, a tag collision now reconciled)
The $\alpha^{11}$ scaling follows from the multiset/dimensional-emergence argument
(MC-T3.2 closure; multiset theorem FTD-0084 plus two SM-hierarchy `[SELECTION]`s). A later,
more scrutinized audit (`DERIV_COLOR_BINDING_STRUCTURE_AND_ME_STATUS.md`, "promotion blockers
identified") found the specific ladder-walk ordering $4+4+3$ that produces $n=11$ is not
uniquely forced from first principles — it is `[SELECTION]` (a bottleneck), not `[DERIVED]`.
This document previously stated `[DERIVED]` here, contradicting that finding; the later,
more careful tag governs. This leg remains the *strongest-motivated* part of the formula
(a real multiset argument constrains the exponent to a small set of candidates), but it is a
selection among that set, not a forced derivation. See `proof_m_e_exponent_n11.py` for the
multiset construction (still valid); the uniqueness claim is what is downgraded.

### 2.2 The prefactor $16/3$ — simplest-rational tightness, not a derivation
Among 6489 rational-prefactor + integer-exponent combinations ($p, q \le 50$, $n \in [8, 14]$),
only **2** fit within 1%, and FTD's $(16/3, n=11)$ is the tighter of the two; at the exponent
$n=11$ the prefactor $16/3$ is the unique small rational that fits within 0.2% (the competitor
$43/8$ sits at 0.6%). This is real structural tightness — but tightness among peers is evidence
for a `[STRONGLY MOTIVATED CONJECTURE]`, **not** a dynamical derivation of the prefactor.

## 3. Why the "topological" re-spelling is not a derivation

The prefactor can be **re-written** as $16/3 = (N_{eff} + N_c)/N_c = (13 + 3)/3$. This is an
algebraic re-spelling of the same rational, not a derivation: it does not predict $16/3$ from
the lattice dynamics, and the "Dimensional Equipartition" reading (partition the bounded
phase-space uniformly across $N_c$ spatial dimensions, then divide by $N_c$) is **asserted**,
not forced. The choice to count a 16-node "$L_2$ cross + symmetry break" core and then divide
by $N_c = 3$ is one of many integer recipes that would land on $16/3$; nothing in the substrate
selects it over alternatives, and it was not checked against a look-elsewhere control. Under the
project's own epistemic discipline (no substitution identities; FTD-0097/0189 bar), this is a
motivated re-statement, not a theorem.

## 4. Conclusion
The electron-mass formula stays a `[STRONGLY MOTIVATED CONJECTURE]`: the exponent $n=11$ is
`[SELECTION]` (corrected 2026-07-01, see §2.1 — a multiset-constrained choice, not a forced
derivation), the prefactor $16/3$ is the tightest simple rational at that exponent, and the
$(N_{eff}+N_c)/N_c$ re-spelling is algebra. No "parametric insertion abolished" claim and no
"structurally derived axiom" status is warranted. Promotion to `[THEOREM]` requires a
pre-registered derivation that forces the prefactor without a look-elsewhere escape.

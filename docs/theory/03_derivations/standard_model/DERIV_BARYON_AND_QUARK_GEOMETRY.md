# Baryon and Quark Mass Geometry — Proton `[SMC]`, Quark masses `[PARAMETRIC]`

**Epistemic Status:** Proton ratio `[STRONGLY MOTIVATED CONJECTURE]` (via the prior
$N_{eff}/\alpha$ formula) · the $L_9$ "knot" re-spelling `[PARAMETRIC]` · the six quark masses
`[PARAMETRIC]`
**Version:** 1.1
**Date:** 2026-06-18 (corrected 2026-06-19)

> **CORRECTION 2026-06-19 (adjudicated):** the `[THEOREM]` promotion (commit `fdc483d0`)
> is **RETRACTED** — these are substitution identities, not forcing chains; they fail the
> FTD-0097/0189 look-elsewhere bar and the standing zero-promotion discipline.
> **Honest tags:** the proton mass ratio is a `[STRONGLY MOTIVATED CONJECTURE]` via the
> prior formula $m_p/m_e = N_{eff}/\alpha + N_{base}\cdot N_{eff} + N_c$ (≈173 ppm); the six
> quark masses are `[PARAMETRIC]` (tuned integer recipes; $m_t$ imports $Z=118$/Oganesson
> from chemistry). **Genuine motivation:** the prior proton formula uses three Moore integers
> *plus* $\alpha$, so it is harder to dismiss as a bare rational fit; the quark recipes have no
> such structural backing. `[THEOREM]` is re-earnable only behind a pre-registered
> look-elsewhere control.

## 1. Overview
In FTD, fractional charge ($\pm 1/3, \pm 2/3$) is associated with a geometric fracturing of the 3D Moore layers: quarks are incomplete topological defects that cannot exist independently and bind via $SU(3)$ color flux to form complete phase-space boundaries (baryons). The constructions below are recorded for provenance; their honest epistemic status is set in each section.

## 2. The Proton Mass Ratio

### 2.1 The motivated formula — `[STRONGLY MOTIVATED CONJECTURE]` (≈173 ppm)
The proton/electron ratio carried by the framework is
$$ \frac{m_p}{m_e} = \frac{N_{eff}}{\alpha} + N_{base}\cdot N_{eff} + N_c = 1781.47 + 52 + 3 = 1836.47, $$
a **173 ppm** match (5.8× experimental precision, ~30 ppm). Because it consumes three
independent Moore integers *and* $\alpha$ (not a bare $p/q$), it sits at the master-quadratic's
epistemic tier: `[STRONGLY MOTIVATED CONJECTURE]`, not a derivation. This is the preferred
form. See `proof_proton_electron_ratio.py` / `proof_complete_sm.py`.

### 2.2 The $L_9$ "phase-space knot" re-spelling — `[PARAMETRIC]`
The integer $1836$ can also be **written** as
$$ \underbrace{6\times 17^2 + 12\times 17}_{1938\ (L_9\ \text{Faces}+\text{Edges})} \;-\; \underbrace{6\times 17}_{102\ (\text{"}SU(3)\text{ edge defect"})} = 1836. $$
This is **strictly less informative** than the §2.1 formula: it is integer-only (zero sub-integer
content — it cannot reproduce the $.15$ in the measured $1836.15$), and the objects it invokes
($L_9$ "bounded phase space", the number $1938$, and the $102 = 1938 - 1836$ "defect") did not
exist in the framework before commit `fdc483d0` and were reverse-engineered to land on $1836$.
It is a substitution identity. Tag: `[PARAMETRIC]`. Do not cite it as a derivation; prefer the
§2.1 form.

## 3. The Quark Mass Spectrum — `[PARAMETRIC]` (all six)

**Reviewer / discipline flag.** All six quark masses are integer-combination *fits* with no
independent structural derivation on the lattice. Each recipe (1–4 hand-selected terms) is
chosen to land near a measured value; the reverse-engineering of integer combinations from
experimental masses is exactly the fishing pattern the project's epistemic-discipline rules
prohibit, and the family as a whole fails any look-elsewhere control (six tunable recipes over
the integer lattice will hit six targets by construction). Specific tells:

- **Up quark** "Unilateral Triad (4)" lands ~5% off the experimental $\approx 4.2\,m_e$ — outside
  even a loose band.
- **Top quark** maps to "$L_{118}$ phase space" — i.e. it **imports atomic number $Z = 118$
  (Oganesson) from the periodic table**, an external chemistry input with no substrate basis;
  it is still ~1% off.
- Strange/Charm/Bottom are 2–4-term superpositions of $L_n$ shells selected to match.

Correct reporting: "given the framework integers, integer-combination fits reproduce the six
quark masses to a few percent." No theorem status. Tag: `[PARAMETRIC]`.

| Quark | Recipe (as constructed) | Tag |
|---|---|---|
| Up | Unilateral Triad = 4 | `[PARAMETRIC]` (~5% off) |
| Down | $L_1$ Face = 9 | `[PARAMETRIC]` |
| Strange | $L_7$ Face + $L_2$ + Core = 183 | `[PARAMETRIC]` |
| Charm | $L_{10}$ + $L_2$ = 2484 | `[PARAMETRIC]` |
| Bottom | $L_{18}$ + $L_4$ + $L_1$ + Triad = 8170 | `[PARAMETRIC]` |
| Top | $L_{118}$ = 334170 (**imports $Z=118$**) | `[PARAMETRIC]` |

## 4. Conclusion
The proton ratio is a `[STRONGLY MOTIVATED CONJECTURE]` carried by the prior $N_{eff}/\alpha$
formula; the $L_9$ knot re-spelling is a less-informative `[PARAMETRIC]` substitution identity;
and the six quark masses remain `[PARAMETRIC]` integer fits with no structural derivation and a
fatal look-elsewhere exposure. There is no derivation of the baryon/quark mass hierarchy here.

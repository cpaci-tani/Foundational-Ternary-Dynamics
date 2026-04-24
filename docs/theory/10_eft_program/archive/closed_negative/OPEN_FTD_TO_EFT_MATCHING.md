# Closed Pivot: FTD-to-EFT Matching Principle

**Date:** 2026-04-22  
**Status:** [CURRENT-ACTION CLOSED NEGATIVE] / superseded by FTD-native electrodynamics  
**Purpose:** Record why the QED-alpha matching program is no longer the primary target, and point to the FTD-native replacement.

---

## Motivation

The April 2026 GPU audits separated three layers:

1. The CM/master-quadratic arithmetic is robust.
2. The tree-level `x_+ = 137.036171...` match to `1/alpha` is a strong arithmetic/conjectural identification at about 1.26 ppm.
3. The Structure-1 one-loop ppb correction is not currently scheme-independent.

The decisive negative cross-check is `AUDIT_STRUCTURE2_WARD_VALIDATION.md`: a Ward-valid Structure-2 two-U(1) scalar gauge completion, using bubble plus seagull terms, does not reproduce the Structure-1 ppb closure under natural scalar matter assumptions.

Therefore the next goal is not to recover a number. The project pivot is:

```text
stop trying to derive QED alpha,
define FTD-native electrodynamics and response observables instead.
```

See `SPEC_FTD_NATIVE_ELECTRODYNAMICS.md`.

---

## Former Required Output

A successful FTD-to-QED matching principle would have needed to specify, from FTD axioms or previously ledgered theorem/selection results:

| Component | Required decision |
|---|---|
| Continuum fields | Which fields emerge from `s` and `J`, and in what representation |
| Gauge group | Which U(1), SU(2), SU(3), or product factors are active in the EFT |
| Matter content | Spin, charge vector, multiplicity, and mass relation |
| Kinetic operator | Which lattice operator maps to the physical gauge kinetic term |
| Regulator | Which Brillouin zone/cutoff convention is physical, and why |
| Counterterms | Which divergences are subtracted or renormalized, and by what principle |
| Observable | Which eigenvalue or matrix element is identified with physical `1/alpha` |
| Error budget | Which corrections are controlled, estimated, or explicitly open |

That rule was not derived under the current projected action.

The replacement output is now:

```text
derive native FTD response observables:
    C_L^FTD    static source-flux response
    K_T^FTD    transverse flux stiffness/dispersion
    Z_j^FTD    transport-current normalization
    g_sJ^FTD   source-flux vertex/coupling
    flow laws  native scale dependence
```

without using CODATA alpha as the target.

---

## Non-Goals

Do not:

- scan charge assignments, masses, regulators, or discretizations to minimize the alpha residual
- treat a successful numerical fit as a derivation
- use the handoff bubble-only q=0 diagnostic as a physical observable
- call a Structure-1 correction universal unless an independent Structure-2 or matching argument reproduces it
- upgrade the ppb alpha claim without passing a Ward-valid gauge check

---

## Minimum Acceptance Tests

Any proposed matching principle must pass these checks:

1. **Ward identity:** gauge kinetic corrections must include all required seagull/contact terms.
2. **Scheme stability:** the claimed physical result must be stable under allowed regulator choices, or the regulator must be uniquely selected.
3. **Matter uniqueness:** charge, spin, multiplicity, and mass must be derived or selected without reference to the target alpha residual.
4. **Engine consistency:** the continuum fields must map back to explicit FTD state/flux degrees of freedom.
5. **Ledger compatibility:** the claim must not contradict FTD-0050, FTD-0056, or FTD-0058.
6. **No numerical search:** fixed verification computations are allowed only after the theoretical rule is written down.

---

## Current Status

Known negative constraints:

- FTD-0050: the master quadratic is not the characteristic polynomial of the current engine RG step.
- FTD-0056: the unrenormalized BCC one-loop tadpole residual has no continuum limit.
- FTD-0058: natural Ward-valid Structure-2 scalar gauge completions S2-A through S2-E do not reproduce Structure-1 ppb closure.

Known positive anchors:

- FTD-0001: the master quadratic polynomial and roots are theorem-level algebra.
- FTD-0002: the G* identity is theorem-level.
- FTD-0003: CM-curve uniqueness across class-number-1 fields is theorem-level within its scan.
- Structure-1 HMC confirms the scalar one-loop perturbative calculation within its own scheme.

Current bridge inventory:

- `docs/theory/10_eft_program/OPEN_FTD_TO_EFT_BRIDGE_STATUS.md` records the current state of the bridge segment by segment. Its conclusion is that the arithmetic core, gauge-like continuum dictionary, and negative Structure-2 audit are all meaningful, but the unique matching principle is still open.
- `docs/theory/10_eft_program/DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md` gives the first bridge-span result: the minimal state/flux dictionary supports a source-coupled vector EFT, but does not yet derive U(1) gauge redundancy.
- `docs/theory/10_eft_program/DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md` refines the gauge question: U(1) is best treated as an emergent redundancy of auxiliary transverse-potential variables after projecting physical flux, not as a microscopic property of `J`.
- `docs/theory/10_eft_program/DERIV_PROJECTED_EFT_MATTER_COUPLING.md` gives the matter/coupling candidate: native matter is signed source/worldline matter; projected coupling uses `rho` for the Coulomb sector and `j_T · A_T` for the radiative sector; Dirac matter is preferred for QED but remains selected.
- `docs/theory/10_eft_program/DERIV_PROJECTED_DIRAC_OPERATOR_AND_CHARGE_NORMALIZATION.md` fixes the projected Dirac candidate symbolically and shows that ternary charge gives integer `q`, not the coupling magnitude `e0`; identifying `e0^2` with `1/x_+` remains an open matching condition.
- `docs/theory/10_eft_program/OPEN_PROJECTED_EFT_RENORMALIZATION_AND_ALPHA_OBSERVABLE.md` states the final pre-computation gate: derive how `x_+` enters before running new alpha numerics. The current-action audits now close the projected-stiffness and response-eigenvalue routes negative.
- `docs/theory/10_eft_program/DERIV_PROJECTED_STIFFNESS_XPLUS_ATTEMPT.md` closes the projected-stiffness route negative under the current action: the native transverse sector is canonically normalized, so `K_T,0 = x_+` would be a new matching rule, not a derivation.
- `docs/theory/10_eft_program/DERIV_PROJECTED_RESPONSE_EIGENVALUE_XPLUS_ATTEMPT.md` closes the projected-response eigenvalue route negative under the current action: the master quadratic can be represented as a `2 x 2` characteristic polynomial, but the projected FTD action does not derive the physical two-sector response matrix.
- `docs/theory/10_eft_program/DERIV_SOURCE_CURRENT_NORMALIZATION_XPLUS_ATTEMPT.md` closes the source-current normalization route negative under the current action: ternary source transport fixes integer charge units and current conservation, but not `e0^2 = 1/x_+`.
- `docs/theory/10_eft_program/SPEC_FTD_NATIVE_ELECTRODYNAMICS.md` defines the replacement program: FTD-native source/flux observables are primary; QED and CODATA comparisons are external diagnostics.

---

## Working Hypothesis

FTD currently supports a robust arithmetic core and a native source/flux dynamics program. The QED-alpha bridge is not load-bearing under the current projected action.

Until then:

```text
tree-level x_+ match: robust conjectural arithmetic result
Structure-1 ppb correction: scheme-specific scalar-EFT result
Structure-2 scalar gauge completion: negative cross-check
unique FTD-to-QED alpha matching: closed negative under current action
replacement target: FTD-native electrodynamics
```

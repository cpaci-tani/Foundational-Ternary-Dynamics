# PRE-REGISTRATION -- Alpha Dynamical Readout Discriminator (FTD-0284)

**Status:** `[PRE-REGISTRATION -- LOCK PENDING]` -- design authored; commit/tag
must precede any engine run of record.  
**Date:** 2026-06-13  
**LEDGER id (reserved):** FTD-0284  
**Git tag (to be applied at lock):** `preregister-alpha-dynamical-readout-v1`  
**Result class (declared):** `[ALPHA-DYNAMICAL-READOUT DISCRIMINATOR]`  
**Static verifier:** `scripts/proofs/proof_alpha_dynamical_readout_contract.py`  
**Static verifier SHA256:** `5a4509ad24dc9be1354b31dfa5336eb573e0eaaafa2e8607fda489748f3af390`

---

## 0. Context

The latest alpha result is not a derivation. FTD-0242 established a
route-invariant MC-T4.3 boundary: the trace `16G*^2` and the clean odd scalar
`G*` are both FTD-native, but the `(Tr, Det) = (16G*^2, 16G*^3)` operator
assembly is not forced by the five postulates. FTD-0244 then closed the
substrate-native operator-calculus route theorem-negative: native operators can
live over `Q(G*)`, but the readout selection `W` is independent of that calculus.

So this pre-registration stops asking for a prettier structural operator. The
remaining honest question is dynamical:

> Can the engine, with no alpha-derived coupling input, select a source-flux
> response whose long-distance dimensionless Coulomb coefficient equals the
> master-quadratic alpha after the comparison is opened?

This document contains no measurement result and promotes nothing. FTD-0013
(`x_+ = 1/alpha`) remains `[STRONGLY MOTIVATED CONJECTURE]`; MC-T4.3 remains a
`[FOUNDATIONAL OBSTRUCTION]` unless a later record satisfies the gates below.

---

## 1. Frozen definitions

The Phase-G geometric-Coulomb theorem gives, for unit native source strength:

```text
V_engine(r, L) = -2 G_L(r)
alpha_r(r, L) = -V r = 2 r G_L(r)
lim_{L->infty, r/L->0} alpha_r = 1/(2 pi)
```

Define the native measured response coefficient:

```text
C_meas = finite-size-controlled limit of alpha_r(r, L)
C_geom = 1/(2 pi)
g_dyn^2 = C_meas / C_geom
```

The master-quadratic comparison value is:

```text
x_+ = larger root of x^2 - 16G*^2 x + 16G*^3 = 0
alpha_tree = 1/x_+
```

The external matching coupling would be:

```text
g_match^2 = alpha_tree / C_geom = 2 pi / x_+
g_match = sqrt(2 pi / x_+)
```

**g_match is not a derivation.** If a run obtains alpha by setting
`coulomb_charge_coupling`, `G_C`, or any equivalent source-flux input to
`g_match`, the verdict is `POSTULATE-W`, not `DYNAMICAL-FOUND`.

---

## 2. Admissible inputs

Allowed:

1. FTD postulates P1-P5 and current engine update rules.
2. The actual engine source/flux operator and Poisson/Gauss conventions.
3. Native unit source strength, `s in {-1,0,+1}`.
4. Predeclared finite-size windows and analysis functions.
5. The master-quadratic value `x_+` only as a post-measurement comparison target.

Not allowed as engine or analyzer inputs:

1. `ontic::ALPHA`, `ALPHA_EFT`, `G_C`, or any alpha-derived constant.
2. `coulomb_charge_coupling = sqrt(2 pi / x_+)` or any algebraically equivalent
   value.
3. CODATA alpha, laboratory `e`, `hbar`, or line data.
4. A tolerance chosen after seeing the run.

---

## 3. Future run requirements

This v1 locks the discriminator, not an engine record. A later run of record
must first freeze a measurement artifact with:

1. Exact toggles and code path, including proof that alpha-derived paths are off.
2. The source preparation, source separation windows, and finite-size sequence.
3. The estimator for `C_meas`, including all finite-size extrapolation choices.
4. Null controls: alpha-derived coupling OFF, explicit `g_match` insertion as a
   positive-control classified only as `POSTULATE-W`, and geometric unit coupling.
5. A hash of the measurement script/binary before execution.

No run that lacks those items can produce a claim stronger than
`DIAGNOSTIC_NOT_VERDICT`.

---

## 4. Frozen outcomes

### NATIVE-NULL

Outcome if the no-alpha-input engine yields the unit geometric response
`g_dyn^2 = 1` (or any stable non-alpha value) under the frozen estimator. This
does not weaken the master-quadratic arithmetic result; it says the current
dynamics do not derive physical alpha.

### DYNAMICAL-FOUND

Outcome only if all of the following are true:

1. The no-alpha-input preparation and toggles pass all static gates.
2. The estimator was frozen before the run.
3. The finite-size trend is stable under the frozen window.
4. The result is not produced by a normalization convention, explicit
   `g_match`, `G_C`, `ALPHA`, or CODATA input.
5. The post-measurement comparison to `alpha_tree = 1/x_+` passes the
   predeclared symbolic/effective-limit criterion.

An isolated finite-L numerical closeness is not enough. Without a native reason
for the coupling value, the verdict must be `NATIVE-NULL`, `POSTULATE-W`, or
`DIAGNOSTIC_NOT_VERDICT`.

### POSTULATE-W

Outcome if the only successful route is to supply the readout assembly or
matching coupling externally. This is a clean price tag for the missing sixth
postulate, not a derivation.

### INVALIDATED

Outcome if any banned move fires, if alpha-derived constants enter the
measurement path, or if the frozen estimator changes after seeing data.

---

## 5. Banned moves

1. **No CODATA input** in the engine, estimator, or verdict gate.
2. **No near-miss** or coincidence search over couplings, windows, radii, lattice
   sizes, toggles, or tolerances.
3. **No promotion** of FTD-0013, MC-T4.3, FTD-0242, FTD-0244, or engine
   `G_C = sqrt(alpha)` unless a later run satisfies `DYNAMICAL-FOUND`.
4. No substitution identity: inserting `x_+`, `alpha_tree`, `G_C`, or
   `g_match` and reporting a successful alpha extraction is `POSTULATE-W`.
5. No laboratory spectroscopy, eV, wavelength, or QED-loop comparison in this
   discriminator.
6. No use of the FQCR transfer matrix as a physical measurement operator.

---

## 6. Static verifier

Before any lock tag is applied, the static verifier must pass:

```powershell
python scripts/proofs/proof_alpha_dynamical_readout_contract.py --verify-static --manifest
```

The verifier checks:

1. Unit native geometric response is `1/(2 pi)`, not `1/x_+`.
2. `g_match^2 = 2 pi / x_+` is sub-unit and depends on the master quadratic.
3. This pre-registration contains the FTD-0242/FTD-0244 status anchors.
4. This file records the verifier SHA256.
5. Banned alpha-derivation assertions are absent.

---

## 7. Hash-lock declaration

This document and `scripts/proofs/proof_alpha_dynamical_readout_contract.py`
must be committed and tagged `preregister-alpha-dynamical-readout-v1` before any
FTD-0284 measurement script or engine binary is allowed to produce a run of
record. Any post-lock edit to Sections 1-5 invalidates v1 and requires a v2.

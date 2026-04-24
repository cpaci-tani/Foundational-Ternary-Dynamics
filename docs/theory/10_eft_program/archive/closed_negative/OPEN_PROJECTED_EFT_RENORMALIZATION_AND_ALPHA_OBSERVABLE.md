# Closed Gate: Projected EFT Renormalization and Alpha Observable

**Date:** 2026-04-22
**Status:** [CURRENT-ACTION CLOSED NEGATIVE] / superseded by FTD-native electrodynamics
**Purpose:** Record why the projected Dirac-QED alpha branch does not currently produce a physical alpha prediction.

---

## Executive result

The bridge has passed through the current-action pre-computation gate and found no derived physical alpha normalization.

We now have a symbolic projected EFT candidate:

```text
native source:        rho = s
native current:       j from signed state transport
field projection:     J = J_L[rho] + J_T,  J_T = P_T A
matter candidate:     projected Dirac spinor psi
coupling:             q e0 A_mu
```

Originally, a physical alpha prediction required four unresolved decisions:

```text
1. Which kinetic operator is physical?
2. Which regulator/cutoff cell is physical?
3. Which counterterms/renormalization condition are allowed?
4. Which observable is identified with alpha?
```

The current-action attempts now close the three physical `x_+` insertion routes negative:

```text
R1. projected stiffness                 closed negative
R2. source-current normalization        closed negative
R3. projected response eigenvalue       closed negative
```

Until a new normalization theorem or explicitly ledgered selection is added, no loop computation can classify the framework as deriving physical alpha. It can only evaluate a selected scheme.

The active replacement target is now `SPEC_FTD_NATIVE_ELECTRODYNAMICS.md`: define and compute FTD-native source/flux response observables instead of treating QED alpha as the required endpoint.

---

## The core ambiguity

The same projected EFT can be parametrized in either of these equivalent-looking forms:

```text
S_A = (K_A / 2) sum F_T^2
S_int = - sum j_T · A_T
```

or, after canonical normalization:

```text
S_A = (1 / 2) sum F_T,c^2
S_int = - e0 sum j_T · A_T,c.
```

These are related by:

```text
A_T,c = sqrt(K_A) A_T
e0^2  = 1 / K_A
```

if and only if the source-current normalization is fixed.

Therefore:

```text
alpha is not defined by q alone.
alpha is defined by the ratio between source normalization and field kinetic normalization.
```

This is the bridge location where `x_+` must enter if it is to become physical `1/alpha`.

---

## Allowed alpha observables

A projected electromagnetic alpha observable should be one of the following, chosen before computation:

### Observable A: static Coulomb response

Define alpha from the long-distance coefficient of the static potential:

```text
V(r) -> q1 q2 alpha_C / (4 pi r).
```

On the lattice this means extracting the small-k coefficient:

```text
V(k) ~ q1 q2 alpha_C / qhat^2.
```

Pros:

- Closest to native FTD source/flux dynamics.
- Directly tied to Gauss projection and Coulomb response.

Cons:

- Engine audits show engine-native alpha-like plateaus are lattice-geometry quantities, not automatically QED alpha.
- Needs field/source normalization before comparing to physical alpha.

### Observable B: transverse kinetic coefficient

Define alpha from the renormalized transverse inverse kinetic coefficient:

```text
K_T,R = 1 / alpha_T.
```

Matter loops correct:

```text
K_T,R(q) = K_T,0 + Pi_T(q) + counterterms.
```

Pros:

- Matches the Structure-2 Ward-valid logic.
- Gauge/projection-compatible.

Cons:

- Requires matter content, seagull/contact terms, and counterterms.
- Natural scalar version failed; Dirac version still symbolic.

### Observable C: Thomson scattering amplitude

Define alpha from the zero-momentum scattering amplitude of two unit charges:

```text
M(q -> 0) = q1 q2 e_R^2 / q^2.
```

Pros:

- Closest to physical CODATA alpha.
- Standard QED interpretation.

Cons:

- Requires full projected QED matching, charge renormalization, and matter mass thresholds.
- Not native to the current FTD engine.

### Observable D: arithmetic-only root

Do not define a physical EFT alpha. State only:

```text
x_+ = 137.036171...
```

as an arithmetic root with a striking empirical match.

Pros:

- Fully honest if matching cannot be derived.

Cons:

- Drops the physical alpha prediction beyond the tree-level numerical observation.

---

## Regulator choices

A future projected-Dirac calculation must choose one regulator family:

| Regulator | Meaning | Status |
|---|---|---|
| Engine-native `(SC+FCC)/2` | Use the actual engine stencil | faithful to engine, BCC arithmetic not automatic |
| BCC arithmetic sector | Use corner-sector operator tied to `G*` | faithful to master quadratic, not engine-native |
| Cubic projected-QED BZ | Use standard hypercubic lattice Dirac/QED BZ | faithful to lattice QED, selected bridge |
| Hybrid/composite | Derived map between engine and BCC sectors | open, must be proved before use |

The regulator choice must be justified structurally. It cannot be chosen by the alpha residual.

Current best candidate for a QED-facing test:

```text
cubic projected-QED BZ with explicit Wilson/naive/overlap fermion choice
```

Current best candidate for engine-native physics:

```text
engine `(SC+FCC)/2` operator, interpreted as lattice flux dynamics rather than QED alpha
```

These are different branches.

---

## Counterterm rules

The counterterm prescription must obey:

1. **Projection/Ward compatibility:** transverse corrections remain transverse.
2. **No bubble-only observables:** contact/seagull terms required by the selected matter action must be included.
3. **No target fitting:** finite counterterms cannot be chosen to force CODATA.
4. **Scheme declaration:** the renormalization condition must be stated before evaluation.

Possible renormalization conditions:

| Condition | Definition | Use |
|---|---|---|
| On-shell/Thomson | set alpha by `q -> 0` scattering | measurement-style, not prediction unless bare alpha derived |
| Bare-stiffness prediction | set `K_T,0 = x_+`, compute `K_T,R` | possible prediction if `K_T,0 = x_+` is derived |
| Matching-scale prediction | set `e0^2 = 1/x_+` at a derived scale | requires scale derivation |
| Arithmetic-only | no EFT renormalization | preserves tree-level match only |

---

## Current gate status

The current state is:

```text
Matter representation:     partial, Dirac preferred but selected
Gauge/projection:          partial, emergent U(1) projection supported
Charge integer q:          supported internally
Charge magnitude e0:       open
Kinetic normalization K_A: open
Regulator:                 open
Counterterms:              open
Alpha observable:          open
```

Therefore no new numerical alpha result should be called a framework prediction yet.

---

## Minimal non-search next step

The next non-computational decision should be one of:

```text
R1. Derive K_T,0 = x_+ from the projected flux action.
R2. Derive e0^2 = 1/x_+ from source-current normalization.
R3. Derive a projected kinetic matrix whose physical eigenvalue is x_+.
R4. Decide no such derivation exists and keep x_+ as arithmetic-only.
```

R1 was initially the most promising if the project wanted a physical alpha bridge, because it respects field rescaling:

```text
alpha emerges from stiffness normalization, not from assigning a magic charge.
```

But `DERIV_PROJECTED_STIFFNESS_XPLUS_ATTEMPT.md` closes R1 negative under the current action: the native projected transverse sector is canonically normalized and does not contain a factor `x_+`.

The live options after that attempt are:

```text
R2. Derive e0^2 = 1/x_+ from source-current normalization.
R3. Derive a projected kinetic matrix whose physical eigenvalue is x_+.
R4. Decide no such derivation exists and keep x_+ as arithmetic-only.
```

`DERIV_PROJECTED_RESPONSE_EIGENVALUE_XPLUS_ATTEMPT.md` then tests R3 and closes it negative under the current projected action. The master quadratic is naturally representable as a `2 x 2` characteristic polynomial, but the projected FTD action does not derive the required two-sector response matrix. The Helmholtz-projected longitudinal/source and transverse/radiative sectors are block diagonal at quadratic order.

The live options after R1 and R3 are therefore:

```text
R2. Derive e0^2 = 1/x_+ from source-current normalization.
R4. Decide no such derivation exists and keep x_+ as arithmetic-only.
```

`DERIV_SOURCE_CURRENT_NORMALIZATION_XPLUS_ATTEMPT.md` then tests R2 and closes it negative under the current projected action. Ternary source dynamics supplies signed integer charge units and a conserved transport current, but it does not fix the dimensionless physical coupling magnitude `e0`.

The current-action endpoint is therefore:

```text
R4. Keep x_+ as arithmetic-only unless a new matching theorem is derived.
```

---

## What this closes

This does not close the alpha bridge. It closes the ambiguity about what remains.

The remaining bridge is now one precise future-theory gate:

```text
derive a new source-current/action-measure theorem that fixes e0^2 = 1/x_+,
or classify x_+ as arithmetic-only under the current bridge.
```

If that future gate is passed, a fixed projected-Dirac loop calculation becomes meaningful.

If that gate is not passed, the honest endpoint is:

```text
x_+ is a robust arithmetic root matching 1/alpha at 1.26 ppm,
while ppb loop corrections are scheme-specific.
```

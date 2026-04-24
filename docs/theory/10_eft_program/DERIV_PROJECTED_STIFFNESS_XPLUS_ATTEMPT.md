# Projected Transverse Stiffness and x_+ Attempt

**Date:** 2026-04-22
**Status:** [CLOSED NEGATIVE under current action] / possible future matching rule
**Purpose:** Test whether the current FTD action derives `K_T,0 = x_+` for the projected transverse flux sector.

---

## Executive result

Under the current FTD action, the projected transverse stiffness is **not** derived as `x_+`.

The native field sector has canonical normalization:

```text
S_field ~ 1/2 |Delta_t J|^2 - 1/2 c^2 |grad J|^2
```

After Helmholtz projection:

```text
J = J_L[rho] + J_T
```

the transverse free sector remains:

```text
S_T ~ 1/2 |Delta_t J_T|^2 - 1/2 c^2 |grad J_T|^2
```

so, in native units:

```text
K_T,0 = 1
```

up to the conventional `c^2` factor and stencil normalization. There is no factor:

```text
K_T,0 = x_+ = 137.036...
```

in the action as presently written.

Therefore the R1 route from `OPEN_PROJECTED_EFT_RENORMALIZATION_AND_ALPHA_OBSERVABLE.md`:

```text
R1. Derive K_T,0 = x_+ from the projected flux action.
```

is closed negative **for the current action normalization**.

---

## What was tested

The target was:

```text
S_A = (K_T,0 / 2) sum F_T^2
K_T,0 ?= x_+
```

The relevant action terms are:

```text
field kinetic:   +1/2 |Delta_t J|^2
field gradient:  -1/2 c^2 |grad J|^2
constraint:      lambda_G (div J - rho)^2
source:          -g_c s div J
```

The constraint separates longitudinal and transverse sectors:

```text
J_L fixed by rho
J_T divergence-free
```

The source term couples to `div J`, so it affects the constrained longitudinal/source sector. It does not multiply the free transverse kinetic term.

The transverse projection therefore preserves the canonical coefficient of the free field term.

---

## Why `x_+` does not appear

The master quadratic gives:

```text
x^2 - 16 G*^2 x + 16 G*^3 = 0.
```

Its larger root `x_+` appears in the arithmetic/coupling documents as a candidate inverse coupling. But in the action:

```text
field normalization      fixed canonically
source coupling          g_c
Gauss constraint         lambda_G -> infinity
manifestation threshold  K_B
```

There is no term of the form:

```text
(x_+ / 2) |grad J_T|^2
```

or:

```text
(x_+ / 4) F_T^2.
```

The existing engine constants do use `ALPHA` and `G_C = sqrt(ALPHA)` in force/coupling paths, but that is exactly the direct coupling-normalization route, not a derivation of transverse stiffness from the free flux action.

---

## Field rescaling does not solve it

One can always rescale:

```text
J_T,c = sqrt(K) J_T
```

and rewrite the action with an arbitrary apparent stiffness `K`. But unless the source-current normalization is transformed and fixed independently, this is only a change of variables.

A physical coupling is determined by the ratio:

```text
source-current normalization / field kinetic normalization.
```

So declaring:

```text
K_T,0 = x_+
```

by field rescaling would be a convention, not a prediction.

---

## What would be needed to revive R1

R1 could become viable only if a new derivation shows that the projected transverse mode uses a different physical normalization from the native flux field.

Possible future routes:

1. **Energy-per-mode route:** derive that one canonical transverse quantum carries action weight `x_+` relative to one ternary source unit.
2. **Partition-function route:** integrate out microscopic flux/source configurations and show the effective transverse action acquires `K_T,0 = x_+`.
3. **Kinetic-matrix route:** derive a projected two-sector kinetic matrix whose physical eigenvalue is `x_+`.
4. **Measurement-normalization route:** show that the observable source current is normalized by `sqrt(x_+)` relative to native `s` units.

None of these is currently present in the action or engine.

---

## Consequence for alpha

The cleanest stiffness route does not currently derive physical alpha.

The remaining gate options are:

```text
R2. Derive e0^2 = 1/x_+ from source-current normalization.
R3. Derive a projected kinetic matrix whose physical eigenvalue is x_+.
R4. Decide x_+ is arithmetic-only, not EFT charge normalization.
```

`DERIV_PROJECTED_RESPONSE_EIGENVALUE_XPLUS_ATTEMPT.md` later tests R3 and closes it negative under the current projected action. The master quadratic naturally has an eigenvalue representation, but the current projected EFT does not derive the physical two-sector response matrix.

`DERIV_SOURCE_CURRENT_NORMALIZATION_XPLUS_ATTEMPT.md` later tests R2 and closes it negative under the current projected action. Ternary source transport fixes integer charge and current conservation, not the physical coupling magnitude.

---

## Claim impact

| Claim | Status after this attempt |
|---|---|
| Native FTD flux field has a well-defined transverse sector | Supported |
| U(1)-like redundancy emerges after transverse projection | Supported as EFT description |
| Current action gives canonical transverse stiffness | Supported |
| Current action derives `K_T,0 = x_+` | Closed negative |
| `x_+ = 1/alpha` via stiffness normalization | Not derived |
| Future projected kinetic-matrix/eigenvalue bridge | Closed negative under current action; open only with a new derived two-sector matrix |

---

## Bottom line

The current action says:

```text
J_T is canonically normalized flux.
```

It does not say:

```text
J_T has stiffness x_+.
```

So `x_+` cannot presently enter physical alpha through a free transverse stiffness coefficient. After the response-eigenvalue and source-current audits, no current-action physical insertion route remains; `x_+` is arithmetic-only unless a new normalization theorem is supplied.

# Source-Current Normalization and x_+ Attempt

**Date:** 2026-04-22
**Status:** [CLOSED NEGATIVE under current projected action] / [OPEN] only with a new normalization theorem
**Purpose:** Test whether the projected FTD-to-EFT bridge derives `e0^2 = 1/x_+` from ternary source and current normalization.

---

## Executive result

The current FTD bridge does **not** derive:

```text
e0^2 = 1/x_+.
```

It derives or supports these ingredients:

```text
s in {-1, 0, +1}        signed source alphabet
q in Z                  relative charge units
j_i                     signed transport current
Delta_t rho + div j = 0 continuity, given charge-conserving updates
J_T = P_T A             projected transverse field description
```

But none of those fixes the dimensionless magnitude `e0`.

The source-current normalization route was the last current-action physical route after:

```text
R1. K_T,0 = x_+ stiffness route          closed negative
R3. x_+ response eigenvalue route        closed negative
```

This document tests:

```text
R2. Derive e0^2 = 1/x_+ from source-current normalization.
```

and closes it negative under the current projected action.

Therefore, under the current bridge, the honest endpoint is:

```text
x_+ is an arithmetic root with a strong empirical match to 1/alpha,
not yet a derived EFT charge normalization.
```

---

## What source-current normalization does fix

The ternary state fixes a signed source unit:

```text
s = +1, 0, -1.
```

For charge-conserving motion, the transport current can be defined by signed state crossing:

```text
s(x,t) -> s(x + e_i, t + 1)

j_i(x + e_i/2, t + 1/2) += s(x,t).
```

This gives a lattice continuity equation:

```text
Delta_t rho + div j = 0
```

when state updates preserve total signed charge, with balanced pair events contributing zero net charge.

This is enough to define:

```text
relative charge signs
integer charge units
current conservation
```

It is not enough to define:

```text
physical electron charge magnitude
alpha = e_R^2 / (4 pi)
e0^2 = 1/x_+
```

The missing step is a relative normalization between the source-current unit and the canonically normalized projected field.

---

## Where the coupling lives

The projected EFT can be written schematically as:

```text
S_A   = 1/2 sum F_T,c^2
S_int = - e0 sum q j_T . A_T,c
```

or before canonical field normalization:

```text
S_A   = (K_A / 2) sum F_T^2
S_int = - sum q j_T . A_T.
```

Those descriptions are related by:

```text
A_T,c = sqrt(K_A) A_T
e0^2  = 1 / K_A
```

only after the source-current unit has been fixed relative to the field unit.

The previous stiffness audit showed that the current projected field action has:

```text
K_A = 1
```

up to conventional speed/stencil normalization. Therefore the field side does not supply `x_+`.

The source-current side supplies:

```text
q = +/-1
```

but not:

```text
e0 = 1/sqrt(x_+).
```

---

## Why the native `s div J` coupling does not close it

The historical state-flux coupling uses:

```text
L_int = -g_c s div J.
```

This term is useful as an internal source-flux coupling, but it does not derive physical QED alpha by itself.

First, it primarily couples to the longitudinal/source sector:

```text
div J = rho.
```

After Helmholtz projection, the transverse radiative coupling is:

```text
S_int,T = - e0 j_T . A_T.
```

The map:

```text
g_c  ->  e0
```

requires a derived field/current normalization and a QED observable. It is not fixed by writing `s div J`.

Second, the value used in engine and coupling documents,

```text
g_c = sqrt(alpha) = 1/sqrt(x_+),
```

imports `x_+` as the coupling value. That is a selected normalization, not a derivation from the ternary source alphabet.

Third, dimensional analysis only gives:

```text
g_c is dimensionless.
```

It does not select the dimensionless number `1/sqrt(137.036...)`.

---

## Why thresholds and mode counts do not close it

Several historical arguments tried to relate source-flux coupling to thresholds such as:

```text
K_B, K_C, G*, x_+ - x_-.
```

The problem is structural. These quantities can form many dimensionless ratios. Without a derivation of the action measure or current two-point normalization, choosing one ratio is a parametric insertion.

For example, a statement of the form:

```text
e0^2 = (some threshold factor) / (some mode count)
```

must explain:

1. why that threshold factor is the current normalization,
2. why that mode count divides the current rather than the field,
3. why the result is evaluated at the electromagnetic matching scale,
4. why no alternative equally natural ratio is allowed.

The current projected bridge does not supply those rules.

---

## What would be needed to revive R2

R2 could be revived by a theorem-level or clearly ledgered selection result that fixes the current normalization independently of the alpha target.

Acceptable future routes include:

1. **Path-integral measure route:** derive an effective action by summing microscopic ternary/source histories and show that the current-current coefficient is exactly `x_+`.
2. **Ward-normalization route:** derive a conserved Noether current whose canonically normalized unit charge satisfies `e0^-2 = x_+`.
3. **Coulomb-response route:** compute the native static source response in fixed lattice units and prove the long-distance coefficient is `1/x_+`.
4. **Arithmetic-sector route:** derive that the electromagnetic source current lives on the BCC/CM sector whose capacity is the master-quadratic root.

None of those results is currently present.

The important constraint is that the rule must be written before numerical evaluation and must not be chosen by the alpha residual.

---

## Consequence for the bridge

The current-action physical routes are now:

| Route | Claim | Status |
|---|---|---|
| R1 | `x_+` is projected transverse stiffness | Closed negative |
| R2 | `x_+` fixes source-current charge normalization | Closed negative |
| R3 | `x_+` is a projected response eigenvalue | Closed negative |
| R4 | `x_+` is arithmetic-only under current bridge | Current honest endpoint |

This does not erase the arithmetic result:

```text
x_+ = 137.036171...
```

It also does not erase Structure-1 as a selected scalar-EFT computation.

It does mean that, under the currently documented projected action, FTD has not derived physical QED alpha as an EFT charge normalization.

---

## Bottom line

Ternary source dynamics gives:

```text
charge alphabet and conserved current.
```

It does not give:

```text
the physical electric charge magnitude.
```

So the current bridge cannot honestly say:

```text
e0^2 = 1/x_+ is derived.
```

It can say:

```text
e0^2 = 1/x_+ is the historical matching selection,
while x_+ itself remains a robust arithmetic root.
```

# FTD Native Source-Flux Coupling Closure

**Date:** 2026-04-22
**Status:** [CLOSED NEGATIVE for non-unit derivation] / [DEFINITION] canonical native normalization

---

## Question

The native response tuple left one symbol open:

```text
g_sJ^FTD
```

The question is whether FTD currently derives a nontrivial dimensionless
source-flux coupling, or whether the coupling is just a choice of units between
the signed source alphabet and the flux field.

This note closes the current-action version of that question.

---

## Inputs Already Fixed

The native audits established:

```text
C_L^FTD = 1
K_T^FTD = 1
Z_j^FTD = 1
c_FTD = 1/sqrt(3)
```

in the current signed-source and canonical-flux conventions.

The state alphabet is:

```text
s in {-1, 0, +1}
```

and charge-conserving movement transports exactly one unit of signed source:

```text
j_i = +/-1
```

per elementary move in lattice units.

---

## Current Production Engine Fact

The historical production Lagrangian contains:

```text
L_int = -G_C s div J
```

with:

```text
G_C = sqrt(alpha)
```

This is real code behavior. It is not, by itself, a first-principles derivation
of a physical electric charge. It is a correspondence normalization imported
from the old alpha-facing bridge.

Classification:

```text
[IMPOSED] production `G_C = sqrt(alpha)` is a historical QED-facing
          normalization.
[CONJECTURE] interpreting production `G_C` as physical electric charge requires
             a separate matching theorem.
```

---

## Native Canonical Normalization

Once the native source and flux units are fixed by:

```text
div J = rho
rho = s
K_T^FTD = 1
Z_j^FTD = 1
```

there is no remaining independent dimensionless number in the bare linear
source-flux vertex. The canonical native coupling is therefore:

```text
g_sJ^FTD = 1
```

This is not a prediction of QED alpha. It is the normalization convention in
which one ternary source unit creates one native longitudinal flux unit.

Classification:

```text
[DEFINITION] g_sJ^FTD = 1 in canonical signed-source/native-flux units.
[THEOREM] no non-unit dimensionless coupling is selected by the current bare
          linear action once C_L, K_T, and Z_j are canonically normalized.
[CLOSED NEGATIVE] g_sJ^FTD = sqrt(alpha_QED) is not derived by the current
                  native action.
```

---

## Why This Is A Closure

A non-unit `g_sJ` would be meaningful only if FTD supplied one of the following:

1. a microscopic path-integral or counting measure that weights source-flux
   histories by a nontrivial coefficient,
2. a current-current normalization theorem,
3. a renormalization flow with a fixed point or measured running coupling,
4. a matching theorem from the CM/arithmetic sector to the source current.

The current deterministic engine and fixed native audits do not supply those
extra structures. They supply a unit source alphabet, a unit current ledger, a
unit longitudinal response, and a unit canonical transverse stiffness.

Therefore the current-action endpoint is:

```text
R_FTD,bare = (C_L, K_T, Z_j, g_sJ, c)
           = (1, 1, 1, 1, 1/sqrt(3))
```

with `W_18 ~= 1.2679` retained as a measured local Green-geometry scalar.

---

## Remaining Open Work

The live open item is no longer "derive `g_sJ` from the current bare action."
That is closed.

The next legitimate questions are:

```text
[OPEN] define a nontrivial FTD action/measure for source histories, if one is
       wanted.
[OPEN] define a fixed coarse-graining protocol and measure whether the native
       tuple flows away from the bare value.
[OPEN] decide whether the production engine should separate native canonical
       coupling from historical QED-facing `G_C`.
```

No CODATA or physical alpha target is allowed in these definitions.

# Projected Dirac Operator and Charge Normalization

**Date:** 2026-04-22
**Status:** [CURRENT-ACTION CLOSED NEGATIVE for QED alpha] / historical projected-QED candidate
**Purpose:** State the projected Dirac-QED candidate with symbolic coupling, and record why it does not set `e^2 = 1/x_+` under the current action.

---

## Executive result

The projected EFT can support a standard lattice Dirac operator, but FTD does not uniquely select a physical QED charge normalization under the current action.

The fixed candidate is:

```text
S = S_A[A_T] + S_psi[psi, A; m, e0] + S_Coulomb[rho]

S_A:
    transverse projected flux kinetic term

S_psi:
    psi_bar [D_W/A + m] psi

S_Coulomb:
    1/2 rho (-Delta)^-1 rho
```

where `D_W/A` is a central-difference lattice Dirac operator, optionally with a Wilson doubler-lifting term, coupled to the auxiliary projected U(1) potential.

The honest status is:

```text
Dirac operator form          selected by locality/symmetry/first-order demand
integer charge q             supported by ternary source alphabet
bare coupling e0             not fixed by q alone
e0^2 = 1/x_+                 closed negative under current projected action
physical alpha               superseded as primary target
```

Later audits close the stiffness, response-eigenvalue, and source-current normalization routes negative. This document is now a historical projected-QED candidate. The active replacement target is FTD-native electrodynamics.

---

## Dirac operator candidate

The least-structured QED-facing matter operator compatible with the previous bridge documents is:

```text
D_naive(p) = i sum_mu gamma_mu sin(p_mu) + m.
```

In position space:

```text
D_naive psi(n)
    = m psi(n)
    + 1/2 sum_mu gamma_mu [psi(n+mu) - psi(n-mu)].
```

Coupling to the auxiliary U(1) potential uses link phases:

```text
psi(n+mu) -> U_mu(n) psi(n+mu)
psi(n-mu) -> U_mu^*(n-mu) psi(n-mu)
U_mu(n) = exp(i q e0 A_mu(n)).
```

For doubler lifting, the Wilson candidate is:

```text
D_W(p) = i sum_mu gamma_mu sin(p_mu)
       + m
       + r sum_mu [1 - cos(p_mu)].
```

Status:

| Ingredient | Status | Reason |
|---|---|---|
| central difference derivative | [SELECTION] | matches existing lattice-gradient convention |
| first-order Dirac factorization | [SELECTION] | natural QED-facing factorization, not forced by native second-order flux equation |
| gamma matrices / Clifford algebra | [THEOREM] once first-order relativistic spinor dynamics are selected |
| Wilson term | [SELECTION] | resolves doublers but breaks chiral symmetry at finite spacing |
| overlap/domain-wall alternatives | [SELECTION] | standard alternatives, not currently selected by FTD axioms |

Therefore:

> FTD currently selects the *need* for a spinor-like projected matter sector more strongly than it selects the exact lattice fermion discretization.

---

## What FTD already supplies

FTD supplies several nontrivial ingredients before importing standard lattice QED:

1. **Signed source alphabet:** `s in {-1,0,+1}` gives integer signed source units.
2. **Charge conjugation:** `s -> -s` is exact.
3. **Void pair structure:** `0 -> (+1) + (-1)` gives particle/antiparticle pairing.
4. **Transverse complexification:** two projected flux components can be packaged as `psi = J_1 + i J_2`.
5. **Four BCC complementary pairs:** a plausible four-component spinor basis.
6. **Spin-statistics support:** existing lemniscate/topology documents support the spinor interpretation at [SELECTION] level.

These support Dirac matter as the preferred QED-facing completion. They do not by themselves fix the physical electric coupling.

---

## Why charge quantization does not fix alpha

The ternary alphabet gives:

```text
q in {-1,0,+1}
```

and, for composites or repeated transport,

```text
q in Z.
```

But the EFT coupling is:

```text
q e0 A_mu.
```

The integer `q` fixes relative charge units. It does not fix the size of `e0`.

There is also a field-normalization degeneracy:

```text
A_mu -> lambda A_mu
e0   -> e0 / lambda
```

which leaves `e0 A_mu` unchanged unless the kinetic normalization of `A_mu` is independently fixed.

So the physical coupling requires four choices or derivations:

1. **Field normalization:** what is the coefficient of the projected transverse kinetic term?
2. **Charge unit:** which integer source unit is the electron?
3. **Observable:** which measured or computed amplitude defines `alpha`?
4. **Renormalization prescription:** how does the bare coupling become the physical Thomson coupling?

FTD has strong candidates for (1) and (2), but (3) and (4) are still open.

---

## Where `x_+` can enter

There are three logically distinct ways to use `x_+`.

### Option A: kinetic stiffness

Assign:

```text
K_A = x_+
```

as the coefficient of the projected field kinetic term:

```text
S_A = (K_A / 2) sum F_T^2.
```

After canonical normalization, the effective coupling scales like:

```text
e_eff^2 ~ 1 / K_A.
```

This is the cleanest route to `e^2 = 1/x_+`, but it still requires a derivation that the projected transverse kinetic coefficient is exactly `x_+`.

Status: **[OPEN] / candidate matching rule**.

### Option B: source coupling strength

Assign:

```text
e0^2 = 1/x_+
```

directly in:

```text
D_mu = partial_mu + i q e0 A_mu.
```

This is the old direct identification. It is simple but currently only a selection.

Status: **[SELECTION] / not a derivation**.

### Option C: eigenvalue observable

Treat `x_+` as an eigenvalue of a two-sector kinetic matrix and identify the physical alpha with the response eigenvalue after projection and renormalization.

This resembles the Structure-2 route, but the natural scalar gauge completion failed. `DERIV_PROJECTED_RESPONSE_EIGENVALUE_XPLUS_ATTEMPT.md` also closes this option negative under the current projected action: the master quadratic has an algebraic `2 x 2` matrix representation, but the projected FTD action does not derive the physical two-sector response matrix.

Status: **closed negative under current action / future route only if a new two-sector response matrix is derived**.

---

## Charge-normalization gate

Before any new alpha loop computation, the project must pass this gate:

```text
Choose or derive one statement:

G1. x_+ is the projected transverse kinetic stiffness K_A.
G2. x_+ is the inverse bare charge e0^-2.
G3. x_+ is an eigenvalue of a projected kinetic-response matrix.
G4. x_+ is not the EFT charge normalization; it is only an arithmetic root.
```

The current-action audits close G1 and G3 negative. `DERIV_SOURCE_CURRENT_NORMALIZATION_XPLUS_ATTEMPT.md` then closes G2 negative under the current projected action. The current endpoint is:

```text
G4. x_+ is not the EFT charge normalization; it is only an arithmetic root.
```

Only a new normalization theorem or explicitly ledgered selection can reopen G1-G3. G4 preserves the arithmetic match but drops the physical alpha claim.

The choice must be made without looking at a residual.

---

## Minimal fixed candidate for future verification

If we need one non-search candidate to test later, the least ad hoc projected-QED candidate is:

```text
Field:
    projected auxiliary U(1) transverse potential A_T

Matter:
    Wilson-Dirac spinor from transverse complexification + BCC pairs

Charge:
    q = +/-1 from ternary source sign

Coupling:
    symbolic e0 until normalization gate is passed

Regulator:
    periodic cubic BZ with explicitly stated Wilson parameter r

Counterterms:
    Ward-compatible charge renormalization only after gauge completion
```

No numeric alpha classification should be attached to this candidate until `e0` is fixed by a matching rule.

---

## What this closes

This closes the next bridge target at the level of classification:

```text
projected Dirac operator: PARTIAL
charge normalization: OPEN
```

Resolved:

- The QED-facing matter branch should be Dirac, not scalar, if the goal is electron-like physics.
- The central-difference Dirac operator is the minimal local first-order candidate.
- Doubler lifting is a separate selection and must be stated.
- Ternary charge gives integer `q`, not the physical coupling magnitude.
- The equality `e0^2 = 1/x_+` is not derived by charge quantization alone.

Still open if the project reopens a QED-facing branch:

- derive first-order Dirac dynamics directly from FTD update rules
- derive or select Wilson/overlap/staggered fermion prescription
- derive field normalization for the projected transverse potential
- derive a new normalization theorem for `x_+`
- specify counterterms and physical alpha observable before any QED comparison

---

## Replacement target

The current target is no longer the projected Dirac-QED alpha gate. It is:

```text
derive FTD-native source/flux observables:
    C_L^FTD, K_T^FTD, Z_j^FTD, g_sJ^FTD, and native flow laws
```

See `SPEC_FTD_NATIVE_ELECTRODYNAMICS.md`.

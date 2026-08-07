# AUDIT — Matched contact-energy obstruction

**Date:** 2026-07-25  
**Identifier:** `FTD-0529`  
**Status:** `[THEOREM — GAUSS-COMPATIBLE TRANSVERSE ENERGY SPLIT]` +
`[CLOSED NEGATIVE — UNCHANGED EDGE/CORNER ELASTIC CONTACT RECIPROCITY]` +
`[RESOLVED BY FTD-0530 — AXIAL CURRENT NULL]` +
`[CONSTRUCTIVE BY FTD-0531 — SYMMETRIC ENERGY ENDPOINT]` +
`[OPEN — COMMON-ACTION VECTOR FORCE]`  
**Verdict:**
`ELASTIC_CONTACT_CANNOT_COUPLE_RECIPROCALLY_WITHOUT_FIELD_DEPENDENT_MATTER_OR_DRESSING`  
**Pre-registration:**
[`PREREG_MATCHED_CONTACT_ENERGY_OBSTRUCTION_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_MATCHED_CONTACT_ENERGY_OBSTRUCTION_v1.md)  
**Run of record:** `engine/results/ftd_0529/windows_msvc_cpu.json`

## 1. Exact obstruction

Let `K` be the complete FTD-0527 contact current and let `beta` be the frozen
FTD-0478 field-energy/work coefficient. The matched current substep is

```text
E_1=E_*-K,
Delta H_field=-beta <K,E_*-K/2>.
```

The unchanged identical-carrier rebase preserves the complete relativistic
matter-energy multiset, so `Delta H_matter=0`. It can close total energy only
for fields on the hyperplane `<K,E_*-K/2>=0`.

Now define the source-free challenge

```text
F=C C^T K.
```

The matched complex gives the exact identities

```text
D F=0,
<K,F>=||C^T K||^2.
```

Therefore `E_*` and `E_*+aF` satisfy the same Gauss law, while their exact
field-energy changes differ by

```text
-beta a ||C^T K||^2.
```

No field-independent zero-work matter update can cancel both values when the
norm is nonzero. This is independent of the compatible baseline field used to
witness existence.

## 2. Full staggered-step embedding

The test does not disable propagation. For each arbitrary deposition field it
sets

```text
B_before=lambda C^T E_*,  lambda=C_SPEED.
```

The Faraday substep then lands on `B_half=0`, the Ampere substep returns the
same `E_*`, and the source-free modified energy is preserved exactly. Thus the
counterfamily is a legal input pair of the selected staggered dynamics, not a
`wave_speed=0` artifact.

A uniform stationary background neutralizes the periodic fractional charge.
A deterministic routed face field realizes absolute Gauss before the event;
the same exact current realizes absolute Gauss afterward. The routing choice
does not enter the challenged-minus-baseline theorem.

## 3. Measured scope

All 312 polarity/direction/speed/translation arms pass the history, continuity,
Gauss, adjoint, embedding, and energy identities. The geometric split is:

```text
edge/corner arms with ||C^T K||^2>0       240
axial arms with C^T K=0                    72
minimum positive ||C^T K||^2                0.054925879693913687
maximum ||C^T K||^2                         0.22807936272011650
minimum predicted compatible-field split   0.00015030506588720264
maximum predicted compatible-field split   0.00062414082090626974
```

The axial exception is structural, not a failed gate: these registered axial
contact currents are curl-free, so divergence-free field changes cannot alter
their work. FTD-0530 subsequently resolves the remaining fixed-path question
more strongly: the summed axial current and endpoint-density change vanish
pointwise, so the field performs no longitudinal work and no impulse is needed
on that symmetric identical quotient.

Registered worst residuals are:

```text
continuity                              2.9143354396410359e-15
absolute Gauss                          6.1709209790705710e-14
challenge divergence                    2.2204460492503131e-16
adjoint identity                        1.3877787807814457e-16
staggered embedding                     0
field midpoint-energy identity          7.6605388699135801e-15
energy-split formula                    2.3890394870718090e-15
matter-energy change                    0
cubic-orbit magnitude                   2.6090241078691179e-15
```

## 4. Consequence

FTD-0527 remains an exact kinematic quotient repair. It is not a reciprocal
collision law in a general matched field. For edge/corner contact, the next
construction must solve the outgoing momenta, exact complete current, and
field update together, or assign the exact work to an explicit dressing/history
degree of freedom. Appending the unchanged elastic rebase after the field
update is mathematically excluded.

FTD-0531 supplies a constructive existence result for the first option on a
selected symmetric field family: one scalar outgoing momentum magnitude can be
solved with endpoint, current, field work, and reversal. The remaining defect
is the common-action origin and general three-vector/arbitrary-field extension,
not local energy capacity.

This does not license a new production force or prove that an additional
primitive variable is necessary. A simultaneous field-dependent discrete
collision solve may still close with existing momentum. No production code,
default, toggle, scenario, normalization, phase order, or tolerance changed.

## 5. Reproducibility

- checks: `9/9 PASS` over `312` arms;
- test SHA256:
  `FE334128B422958BD3AD4B32165FA26B6270F1E3074DDE4559421692F5446287`;
- header SHA256:
  `A9CDB6F8E3728A897CF5E12587679B7FDAF34E4EC181EB2F31530B41205D18ED`;
- implementation SHA256:
  `E104B1685F7CB866A7411D9B02D7B1BC039DC8DD3735281E16C328216A86E176`;
- locked preregistration SHA256:
  `6821AE535D777495621145CE9D8E5D33E34B0AB07C005B59F23F5F5AE2A24AFD`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.

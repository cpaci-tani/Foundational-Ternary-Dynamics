# AUDIT — Contact quotient coupling scope

**Date:** 2026-07-25  
**Identifier:** `FTD-0528`  
**Status:** `[PRE-REGISTERED AXIAL-FACTORIZATION GATE FAILED]` +
`[MEASURED + FORMULA-EXACT — NATIVE SNAPSHOT NON-FACTORIZATION]` +
`[THEOREM — MATCHED HISTORY FACTORIZATION]` +
`[OPEN — RECIPROCAL MATCHED FIELD-TO-MATTER LAW]`  
**Locked verdict:** `CONTACT_QUOTIENT_COUPLING_SCOPE_UNRESOLVED`  
**Mechanistic result:**
`NATIVE_COUPLING_BREAKS_CONTACT_QUOTIENT_ALL_DIRECTIONS_MATCHED_HISTORY_FACTORS`  
**Pre-registration:**
[`PREREG_CONTACT_QUOTIENT_COUPLING_SCOPE_v1.md`](../10_eft_program/preregistrations/PREREG_CONTACT_QUOTIENT_COUPLING_SCOPE_v1.md)  
**Run of record:** `engine/results/ftd_0528/windows_msvc_cpu.json`

## 1. The preregistered axial prediction failed

The locked protocol predicted that the two FTD-0527 representatives would
factor through native coupling for face-normal contact and differ only for
edge/corner directions. The axial gate failed in every registered axial arm:

```text
axial arms                                  72
minimum axial response difference           0.010678067887856796
maximum axial response difference           0.021356135775713592
required axial difference                   < 1e-12
```

No locked pass verdict described the observed all-direction pattern. The
pre-registered verdict is therefore `CONTACT_QUOTIENT_COUPLING_SCOPE_UNRESOLVED`.
The failed prediction is retained; the gate was not relaxed.

## 2. Exact source decomposition

For both contact representatives the primitive ternary arrays are identical,
so the electric-gradient source agrees exactly:

```text
-G_C grad(s)_crossing = -G_C grad(s)_bounce.
```

The velocity assignments to those sites differ. The actual CPU source reads

```text
Delta wave_vel = -G_C grad(s) + G_C curl(s v).
```

Across all 312 arms, the executed wave response matched this formula exactly,
the gradient difference was zero, and the entire response difference was
exactly the difference of `G_C curl(sv)`:

```text
worst production-formula residual           0
worst gradient-source difference             0
worst curl-explanation residual              0
minimum edge/corner response difference      0.0061649853694792211
maximum edge/corner response difference      0.015101068426947709
```

The axial intuition failed because the carrier current is not a smooth vector
depending only on the normal coordinate. It is a site-supported vector with
transverse delta-like localization. Even when its nonzero component is
parallel to the chart separation, transverse central differences of that
localized site field give a nonzero curl.

Thus the native snapshot source observes which velocity is attached to which
raw anchor. It does not factor through the identical-carrier phase quotient.

## 3. Complete history current does factor

FTD-0527's crossing and already-bounced representatives trace the same
unlabelled worldline 1-chain over the completed tick. Their compact endpoint
density and exact oriented current are therefore identical. Applying either
current to the same matched face field also gives the same field response:

```text
worst exact density residual                 0
worst exact face-current residual            0
worst matched field-response residual        0
worst continuity residual                    2.9143354396410359e-15
```

After the FTD-0527 paired rebase, both representatives also have the same raw
position/velocity/polarity output, so native coupling agrees from that point
forward. The noncongruence occurs specifically because the current production
tick samples `curl(sv)` before the movement/contact transaction is complete.

## 4. Consequence for the mobile-matter branch

The FTD-0527 transaction cannot be inserted only at the existing late
occupied-target branch while retaining native coupling earlier in the tick.
The field has already recorded a chart-dependent source. Repairing that path
would require at least one of:

1. replace the snapshot `curl(sv)` source with a complete event-history
   current;
2. move the atomic matter transaction before every source evaluation; or
3. retain raw chart history as physical and abandon the quotient reading.

Options 1 and 2 change the production source contract or phase ordering.
Neither is introduced here.

The selected matched-face sector has the right ordering for the quotient: it
forbids legacy coupling and updates its field from the complete conservative
movement history after the atomic transaction. FTD-0528 therefore narrows the
admissible research path to the isolated matched-history branch. It does not
supply the missing reciprocal field-to-matter force or license FTD-0481.

FTD-0529 supplies the next scope result: retaining the unchanged elastic
FTD-0527 output fails exact matched-field energy reciprocity on every
edge/corner arm. FTD-0530 then proves that the 72 symmetric axial aggregate
currents vanish pointwise, so that fixed path needs no longitudinal impulse.

No production code, default, toggle, scenario, force, collision rule, phase
order, field ontology, normalization, or tolerance changed.

- classification checks: `9/9 PASS`, including explicit detection of the
  failed locked axial hypothesis;
- test SHA256:
  `32F2850C32689B1504682E9B44FF8867C3620508CE879883014C6D8BD3E63E23`;
- header SHA256:
  `220C553065A7B3EA979537D57DBA45E6E001554DD0F7B53A4413E393A191BCFC`;
- implementation SHA256:
  `F62C7FF9F49A77C1B0535FF33FE284D27842FE4EA054F88FFD1D4D0399BDCD5E`;
- locked preregistration SHA256:
  `5492DCA256393375B5C60FA3D7CD994455BE5EC04F69C84AC8B64B41998DA531`;
- toolchain: pinned MSVC `14.44.35207`, Release, actual CPU coupling plus
  observer-only matched history;
- production state and defaults: unchanged.

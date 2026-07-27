# AUDIT — Single-slab connection compatibility

**Date:** 2026-07-25  
**Identifier:** `FTD-0534`  
**Status:** `[THEOREM — DIAGONAL SINGLE-SLAB INCOMPATIBILITY]` +
`[THEOREM + MEASURED — AXIAL NULL COMPATIBILITY]` +
`[SHARPENED BY FTD-0535 — EXACT START-SPLIT OBSTRUCTION]` +
`[RESOLVED BY FTD-0536 — IMPLICIT ATOMIC ACTION CONSTRUCTIVE]` +
`[OPEN — NEW NONLINEAR ROOT/ENERGY/REVERSAL]`  
**Verdict:**
`MIDPOINT_WORK_AND_STAGGERED_MAGNETIC_HISTORY_REQUIRE_MULTISTAGE_CONNECTION`  
**Pre-registration:**
[`PREREG_SINGLE_SLAB_CONNECTION_COMPATIBILITY_v1.md`](../10_eft_program/preregistrations/PREREG_SINGLE_SLAB_CONNECTION_COMPATIBILITY_v1.md)  
**Run of record:** `engine/results/ftd_0534/windows_msvc_cpu.json`

## 1. Exact obstruction

For one FTD-0484 connection slab,

```text
E=-(A_1-A_0)/lambda-G Phi,
B_n=C^T A_n.
```

The cochain identity `C^T G=0` forces discrete Faraday:

```text
B_1-B_0=-lambda C^T E.
```

This is gauge independent and holds before choosing a particle force.

The FTD-0531 energy construction uses

```text
B_0=lambda C^T E_0,
B_1=0,
E_1=E_0-K,
E_work=(E_0+E_1)/2=E_0-K/2.
```

If the field doing the exact particle work is also the electric representative
of that one connection slab, its Faraday residual is

```text
R_F=(B_1-B_0)+lambda C^T E_work
   =-(lambda/2) C^T K.
```

Therefore

```text
||R_F||^2=(lambda^2/4)||C^T K||^2.
```

No gauge choice, scalar potential, endpoint interpolation, or incident-cell
selection can change this identity.

## 2. Registered result

All 240 FTD-0531 edge/corner endpoints retain their exact current and energy
identities. Their final currents satisfy

```text
0.05543427669378428 <= ||C^T K||^2
                    <= 0.22913232901251784.
```

Consequently every diagonal one-slab mismatch is nonzero:

```text
0.06796707333566275 <= ||R_F||
                    <= 0.1381823460180176.
```

The component identity closes to `2.7755575615628914e-17`; the squared-norm
identity closes to `1.7347234759768071e-17`. Translation, polarity mirror, and
signed-cubic residuals remain below `1.01e-16`.

All 72 symmetric axial controls reproduce FTD-0530: the complete pair current,
its curl, and `R_F` vanish. A zero connection is then an explicit compatible
single-slab representative.

## 3. Correct consequence

FTD-0534 does not invalidate the scalar FTD-0531 energy root or the FTD-0533
global internal-knot derivative. It proves they cannot be combined by assigning
the midpoint work field and the staggered magnetic endpoints to one FTD-0484
slab.

The next admissible construction must be genuinely multistage or phase-space:
the Faraday substep, current deposition, field momentum, and particle
worldline coupling must arise from one composed discrete action. The existing
face electric and edge magnetic fields may be sufficient variables, but their
stage placement must be derived. Simply averaging `E_0` with `E_work`, changing
the magnetic endpoint, or selecting an incident-cell force would change a
locked identity and is not a repair.

FTD-0535 supplies the endpoint-resolved version: the exact FTD-0484 start
deposit has nonzero curl on every diagonal root, so the frozen Faraday stage
cannot remain current-free. The required multistage construction is therefore
an implicit atomic solve, not a sequential reuse of the frozen phases.

FTD-0536 constructs the minimal atomic action that includes this start deposit.
Its field equations close, resolving action existence, but the old FTD-0531
scalar root is not stationary. A new simultaneous nonlinear solve remains.

No production code, default, toggle, scenario, force, collision law, phase
order, field ontology, normalization, or tolerance changed.

## 4. Reproducibility

- checks: `7/7 PASS` over `312` Moore-direction arms;
- test SHA256:
  `4B1A9B7B85F0F3B99B1054307E5957A84FA559214F8AE0821649B4A59186CED3`;
- header SHA256:
  `376ACA63AC322DC0000F44FBF977FF675C2D0515A5C56AE5BB0E96663B1CA1D7`;
- implementation SHA256:
  `1FED7D9C22FA0048F7FA864E4104B07236541EE41E5A51B902BB1FDDC8DEEFAE`;
- locked preregistration SHA256:
  `D5E822A46CEFE602113BADC6FF417E56348EF3E27856BAABC8B05BF86EE65B06`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.

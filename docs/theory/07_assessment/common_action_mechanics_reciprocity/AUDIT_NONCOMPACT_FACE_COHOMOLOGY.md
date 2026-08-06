# Audit — Noncompact Matched-Face Cohomology (FTD-0583)

**Date:** 2026-07-26  
**Verdict:**
`MATCHED_NONCOMPACT_COHOMOLOGY_GLOBAL_ONLY_LOCAL_PROTECTED_DEFECT_CLOSED`

## Findings

1. **The periodic real matched complex has only zero-mode cohomology.** Direct
   row reduction found ranks `(0,0,0)` at the four registered zero modes and
   `(1,2,1)` at all 724 nonzero modes. No rank mismatch occurred.

2. **The face cohomology is `R^3`, not a localized charge sector.** The three
   generators are constant fluxes through the noncontractible coordinate
   planes. They are delocalized and continuously valued.

3. **All tested localized zero-harmonic fields contract to vacuum.** The 24
   local curl fixtures retained exact zero divergence and harmonic flux under
   120 contraction samples. Their energies followed `U(tE)=t^2U(E)` exactly.

4. **Gauss sourcing does not quantize the real field.** All 120 periodic
   source/sink arms had zero total charge and exact linear scaling to the
   vacuum. A ternary source value and a topological field class are distinct.

5. **Cubic covariance holds after correcting the observer's reflected-cell
   indexing.** All 24 proper rotations preserved curl, divergence, energy,
   and the harmonic flux vector exactly. The correction changed observer
   geometry only; it did not alter the matched operator or production.

6. **The protected-carrier escape is closed only for the current variables.**
   Compact links, branch integers, singular defects, nonlinear cores, and
   boundary sectors were excluded from the campaign and remain unproved.

## Registered execution

| Gate | Registered count | Result |
|---|---:|---:|
| Fourier symbol arms | 728 | PASS |
| nonzero modes | 724 | ranks `(1,2,1)` throughout |
| zero modes | 4 | ranks `(0,0,0)` throughout |
| volume Betti checks | 4 | `(1,3,3,1)` throughout |
| harmonic arms | 48 | PASS |
| localized curl arms | 24 | PASS |
| contraction samples | 120 | PASS |
| Gauss charge-scaling arms | 120 | PASS |
| proper cubic rotations | 24 | PASS |

The maximum Fourier chain residual was `8.881784197001252e-16`. All measured
divergence, plane-flux, contraction-energy, charge-scaling, surface-telescope,
and cubic-covariance residuals were exactly zero at binary64 output precision.
The localized fixtures had support four and minimum nonzero quadratic energy
`0.095703125` before contraction.

## Provenance

The preregistration was locked before implementation at SHA-256
`755D703FB3E9DA9CA7F2EB46B1FE399D704F739AD08050D39242D1EB0B2BB922`.

| Artifact | SHA-256 |
|---|---|
| observer header | `B3C6668E4E492A14060F993F79EF313D1FD5C9383C249884F031D4C33A56E490` |
| observer source | `46F380DE557F124EEEF684238B141A23E9BC63ADD5A118EDD373B1CCD225C0AC` |
| native test | `1A65D3F79B524F0C2798A15542A723FE80AB6B002027AE4B49DCCB4E59D19BBA` |
| independent proof | `AF5B9BBC80B47BFBE09451DBFA6451F620248AD8F72832201E05AAD590BA31C0` |
| MSVC Release binary | `4D77C6BD361E1C42BC6F9DFF5CE40122C6410A3FD9D8C4AE7F038A9FED0364C8` |
| matched operator header | `1E07F87A0EBD0D1830D0632B82C2BD65497EBEAE7BB152EA02C5AAE19328B033` |
| matched operator source | `12BF98040BB45AD6CD9A409A93C842101C400CEEE6242E9B9352158A33A9D028` |
| primitive voxel state | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| production `phase_read` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| production `phase_write` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |

The versioned run of record is
`engine/results/ftd_0583/windows_msvc_cpu.json`. Production defaults, state,
tick ordering, toggles, scenarios, and rendering are unchanged.

## Build note

The focused target and test pass. Two separate all-target attempts hit
empty-output linker failures in different unrelated targets
(`test_sim_parity`, then `test_variational_coulomb`) under the 32-way Windows
build. The canonical batch wrapper printed its generic success footer despite
Ninja stopping in both cases. This appears independent of FTD-0583, but those
runs are not counted as successful full builds. Both failed targets rebuilt
successfully together at parallelism eight, supporting a high-concurrency
Windows link/resource diagnosis rather than an FTD-0583 source failure.

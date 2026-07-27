# PRE-REGISTRATION — Contact quotient coupling scope

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0528`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; AXIAL GATE FAILED]`  
**Scope:** observer-only comparison of the FTD-0527 identical-contact quotient
against the actual native state-flux coupling source and the exact matched
face-current response. No production state, phase order, default, toggle,
scenario, force, collision law, field ontology, or tolerance change.

## 1. Discriminator

One tick before the FTD-0526 chart horizon, crossing and already-bounced
representatives have the same ternary site occupancy and the same unlabelled
phase-space multiset. Their velocities are assigned oppositely to the two raw
anchors.

The actual native coupling source is

```text
Delta wave_vel = -G_C grad(s) + G_C curl(s v).
```

The first term must agree because the primitive `s` arrays agree. The second
need not agree because the site-velocity arrays are different chart
representatives. In contrast, the FTD-0478/0484 exact face current depends on
the physical worldline 1-chain and must agree by FTD-0527.

## 2. Registered arms and gates

Use both polarities, three translations, every nonzero Moore direction, and
speeds `1/8` and `1/4` (`312` arms). For each arm:

1. reproduce both actual CPU coupling updates with formula residual below
   `1e-12`;
2. require the gradient-source difference below `1e-12`;
3. require the native wave-response difference to equal the `G_C curl(sv)`
   difference below `1e-12`;
4. require all 72 axial arms to factor below `1e-12`;
5. require all 240 edge/corner arms to differ by more than `1e-6`;
6. require the FTD-0527 exact density/current response to agree below
   `1e-12` on every arm;
7. apply both exact currents to identical matched face fields and require the
   resulting field residual below `1e-12`;
8. after the FTD-0527 common raw output, require native coupling to agree
   below `1e-12`;
9. require translation, polarity-mirror magnitude, and cubic-orbit magnitude
   residuals below `1e-12`;
10. invalid inputs fail closed.

## 3. Locked verdicts

- If axial native coupling factors, every diagonal native arm differs exactly
  through `curl(sv)`, and exact matched current factors:
  `NATIVE_COUPLING_BREAKS_CONTACT_QUOTIENT_MATCHED_HISTORY_FACTORS`.
- If all native arms factor:
  `CONTACT_QUOTIENT_FACTORS_NATIVE_COUPLING`.
- If exact matched face current or its field response differs:
  `FTD0527_PHYSICAL_QUOTIENT_REJECTED`.
- If the response is not explained by the documented source formula:
  `CONTACT_COUPLING_SCOPE_UNRESOLVED`.

The first verdict would mean the FTD-0527 transaction cannot simply replace
the late movement branch while leaving native coupling earlier in the tick:
the pre-movement source has already observed the raw representative. It would
remain composable with the isolated matched-face sector, where legacy coupling
is forbidden and field response is deposited from the complete movement
history after the atomic transaction.

## 4. Execution record

Executed 2026-07-25 with pinned MSVC `14.44.35207`, Release, actual CPU
coupling plus observer-only matched history. The locked preregistration SHA256
before execution/status annotation was
`5492DCA256393375B5C60FA3D7CD994455BE5EC04F69C84AC8B64B41998DA531`.

The preregistered axial-factorization gate failed: all 72 axial arms had a
positive native response difference of `0.0106781..0.0213561`. All 240
edge/corner arms also differed. The executed response matched the documented
source formula exactly, and `curl(sv)` explained the complete difference.
Exact density, face current, matched field response, and the common post-rebase
native response still agreed. Because no locked pass verdict covered this
all-direction pattern, the locked verdict is

```text
CONTACT_QUOTIENT_COUPLING_SCOPE_UNRESOLVED
```

The separately recorded mechanistic result is

```text
NATIVE_COUPLING_BREAKS_CONTACT_QUOTIENT_ALL_DIRECTIONS_MATCHED_HISTORY_FACTORS
```

Canonical result:
[`AUDIT_CONTACT_QUOTIENT_COUPLING_SCOPE.md`](../../07_assessment/AUDIT_CONTACT_QUOTIENT_COUPLING_SCOPE.md).

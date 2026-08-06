# FTD-0761 — M4 boosted relational transport discovery result v1

**Status:** `[HISTORICAL FROZEN VERDICT; CORRECTED BY FTD-0762 — MOVING-CORE WITNESS, COMPLETE DRESSING OPEN]`  
**Date:** 2026-07-31  
**Protocol:** `PREREG_M4_BOOSTED_RELATIONAL_TRANSPORT_DISCOVERY_v1.md`  
**Protocol SHA-256:**
`AD6368C6793374771703A1506FA60C06E1D11C0649227F315DD1A79A0F3BDA5C`

## Result

The final `L=33`, `<100>`, two-tick WSL2 CUDA qualification passed. The
registered `<100>`, `<110>`, and `<111>` modes then ran once with the frozen
`L=321`, `q=0.015`, 256-tick protocol. Each direction produced 771 rows: the
rest, plus, and minus arms at ticks 160--416. The body mode wrote the aggregate.

The frozen independent certificate passes:

```text
FTD-0761 artifact certificate: 247/247 checks
verdict=M4_BOOSTED_RELATIONAL_COHERENCE_CLOSED_AT_REGISTERED_SCALE
```

Production, the common action, predicate, ontology, defaults, scenarios, and
`RenderBridge` are unchanged.

> **FTD-0762 correction.** The 0/24 observer and ladder failures below are an
> integer-center observer-chart obstruction, not measurements of physical
> field incoherence. All moved checkpoints have fractional centroids and are
> rejected before their fields are compared. The frozen FTD-0761 artifact is
> preserved, but its coherence-closure interpretation is superseded by
> `AUDIT_M4_MOVING_DRESSING_OBSERVER_FORENSICS_RESULT_v1.md`.

The ignored run-of-record directory is `engine/results/ftd_0761/`. Its frozen
artifact hashes are:

| artifact | SHA-256 |
|---|---|
| `..._face.csv` | `EC1081121A16709B419067DA6BF3266476653A49B907536185C6C8D2A20787FA` |
| `..._face.json` | `C6DF277E213161627DCA1499861B0DBDB303B3B70FFDCF0B3C15C1945F1814C1` |
| `..._edge.csv` | `715C418FE7AED2C02091C1847AAA3CE9FBAD36E8F4BDAE46BC0D213EC065FE51` |
| `..._edge.json` | `06A8A3057FC189D0E74C203BC4301DE675369B8B4BA7D270AB90B0F384F405E3` |
| `..._body.csv` | `7274B0FD3105315F195B5396681849F8C819B8BF1B5811C420E3675564ADC097` |
| `..._body.json` | `22A3D39B0C7FF9D121007BCEE4894A29C618397AB270196A35838DFF2AE861C7` |
| aggregate JSON | `A3247233851C757B9B9372380C33625F153076B2340941C803DDA3F7D9307733` |

## What passed

All three unboosted controls are exact finite-time members and remain
stationary. All six boosted arms initialize, execute all 256 transactions,
remain inside the support-independent core predicate, and pass the local
common-action, energy, causality, root-regularity, and state-only inversion
gates.

| quantity | registered extremum over boosted arms | gate |
|---|---:|---:|
| minimum graph margin | `0.10414550075514262` | `>=1e-6` |
| minimum energy margin | `0.00035652881433715505` | `>=1e-6` |
| maximum common-action residual | `5.46229728115577e-14` | `<=1e-10` |
| maximum transaction energy residual | `6.330439644708363e-15` | `<=1e-8` |
| maximum cumulative energy drift | `3.164135620181696e-14` | recorded |
| maximum causal-speed excess | `0` | `<=1e-12` |
| minimum root singular value | `0.9843660625027102` | `>=1e-3` |
| maximum root condition number | `1.087077242517246` | `<=1e4` |
| maximum one-step reverse recovery | `2.842170943040401e-14` | `<=1e-10` |

The centers also satisfy every frozen *kinematic* transport discriminator when
reconstructed directly from the rows, although transport cannot be credited
after coherence fails:

| ray | signed final displacement, plus/minus | four positive 64-tick increments, plus | maximum transverse displacement | mirror residual | plus/minus hops |
|---|---:|---|---:|---:|---:|
| `<100>` | `1.5575532924218294 / 1.5575532924216873` | `0.4042465806223561, 0.35252328040203906, 0.43829706334668117, 0.36248636805075307` | `6.561661155396448e-13` | `9.211520435314924e-12` | `7 / 7` |
| `<110>` | `1.2271455627876124 / 1.2271455627874515` | `0.40005326208963665, 0.2600793605958023, 0.229955236255171, 0.3370577038470025` | `3.6983186415103065e-12` | `8.08642042215979e-12` | `5 / 5` |
| `<111>` | `1.1405666028003323 / 1.1405666027999222` | `0.41567332180485217, 0.2829980298954645, 0.21088915288890364, 0.23100609821111195` | `3.3118304908175027e-12` | `1.3450573987938697e-11` | `3 / 3` |

Thus the exact common action supports secular, mirrored center translation of
the two-constituent core at this scale. This fact alone is not a certified
moving matter state because the complete matter/field classifier fails.

## Exact failure localization

The coherence closure is caused by one repeated failure class only:

```text
rest observer/support checkpoints:       15/15 pass
boosted initial checkpoints (tick 160):    6/6 pass
boosted post-initial observer checks:      0/24 pass
boosted post-initial support-ladder checks:0/24 pass
all other registered boosted gates:       pass
```

For both boost signs and every ray, `observer_valid=0` and `ladder_valid=0` at
ticks 224, 288, 352, and 416. No core, action, energy, causal, regularity, or
inverse threshold fails on those rows. The frozen CSV schema records the two
observer verdict bits but not their internal residual vectors, so FTD-0761
cannot determine whether the common cause is:

1. a genuine non-comoving field dressing produced by adding matter momentum
   while leaving the initial face/edge field unchanged; or
2. a fractional-center or numerical covariance defect in the state-only
   observer/support construction.

That distinction is open and cannot be repaired or inferred inside FTD-0761.

## Momentum classification

No boosted direction is `FIELD_BALANCED`. The maximum per-step and cumulative
spline matter--field momentum defects are:

| ray | maximum step defect | maximum cumulative defect |
|---|---:|---:|
| `<100>` | `0.0001786902711804783` | `0.007866190143622602` |
| `<110>` | `0.00023242918146216355` | `0.015625943713044266` |
| `<111>` | `0.00019355636584050728` | `0.016598741284559227` |

These exceed the registered `1e-9` and `1e-8` classification thresholds by
large margins. The aggregate field-balance bit is vacuously true because zero
directions pass transport; it is not a physical balance result.

## Correct statement

FTD-0761 does not certify the **matter-only collective boost with unchanged
initial field dressing** as a complete moving-member construction. FTD-0762
shows that its observer closure was outside the observer's domain, so it also
does not physically falsify that dressing. The record contains a strong but
uncertified moving-core witness: mirrored secular center translation, exact
energy exchange, causal evolution, well-conditioned roots, and state-only
inversion all survive. Complete dressing and momentum balance remain open.

The immediate bottleneck is no longer the atomic transaction. It is the
relation between motion and dressing. A physical moving object cannot yet be
claimed until a co-moving state-only field decomposition passes and the large
momentum defect is either balanced by a correctly prepared field or classified
as a genuine substrate reaction.

## Next admissible gate

Before applying an incoming packet or changing ontology, freeze a separate
observer-covariance forensic campaign that records every internal observer and
support-ladder residual on translated fractional-center states and on the six
FTD-0761 boosted histories. It must distinguish a numerical/readout failure
from a physical stationary-dressing mismatch without changing the dynamics or
tolerances.

If the observer is covariant and the dressing mismatch is physical, the next
constructive candidate must prepare matter and face/edge fields jointly from
the common action; adding momentum to constituents alone is closed. If no
local joint preparation exists, escalation to a richer constituent phase
space and then a connection-based electromagnetic primitive is justified.

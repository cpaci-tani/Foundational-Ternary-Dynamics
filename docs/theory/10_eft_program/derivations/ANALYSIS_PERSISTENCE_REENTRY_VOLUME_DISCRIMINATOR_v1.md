# FTD-0730 — Persistence/re-entry volume discriminator v1

**Status:** `[SELECTED DYNAMICS + MEASURED — TWO-VOLUME LOCAL RECURRENCE]`  
**Verdict:** `P012_REENTRY_LOCAL_DYNAMICS_VOLUME_STABLE`  
**Production status:** unchanged

## Result

All 88 registered `L=33/65` histories pass common-action, energy, recoil,
state-only inverse, and bound-control gates. The lower-energy core and field
morphology reproduce exactly across the two volumes, and the ambiguous
`p=0.0120` third transition is identical in every matched arm.

```text
matched p=0.0120 arms                    26
re-entered on L=33 / L=65                26 / 26
maximum matched third-tick difference     0
matched lower-energy parent arms         12
persistent on both volumes               12
maximum matched tick-96 radius difference 0
bound controls on L=33 / L=65             6 / 6
```

The `p=0.0120` event times resolve into the three cubic ray classes:

| ray class | directions | entry | exit | re-entry | tick-96 radius | final-eight negative |
|---|---:|---:|---:|---:|---:|---:|
| face | 3 | 7 | 26 | 63 | 3 | 6/6 arms |
| edge | 6 | 7 | 26 | 79 | 6 | 0/12 arms |
| body diagonal | 4 | 7 | 26 | 96 | 12 | 0/8 arms |

Both polarity orders are identical. Every transition tick, final sign class,
and radius class is the same on `L=33` and `L=65`.

## Interpretation

The third transition is not caused by the `L=33` boundary in this two-volume
comparison. It is a local recurrence of the selected coupled matter-field
dynamics. The pair undergoes an initial encounter, exits, and then returns on
a direction-dependent clock. The clock follows face/edge/body ray class, so
the recurrence is microscopically cubic rather than rotationally continuous.

This changes the formation question. The correct candidate is no longer only
single-pass capture. An initially unbound pair can export energy to the field,
leave the interaction graph, and later re-enter. Face-direction arms already
remain negative over the final-eight-tick window after re-entry. That is a
multi-pass formation candidate, not yet qualified capture, because the locked
single-pass classifier rejected any prior exit and the 96-tick record does not
establish persistence over another recurrence cycle.

## Ontological consequence

The data support a more dynamic minimal picture:

> matter formation may be a recurrent matter-field transaction in which a
> relational core revisits the interaction region while an extended field
> carries and returns part of the energy exchange.

This is neither a rigid two-voxel bead nor a permanently compact aura. It is a
reversible core-field orbit with discrete directional phase. No new primitive
is indicated: the present constituent and face/edge state predicts every
return time and inverts exactly.

The next gate is a freshly preregistered multi-pass persistence campaign over
at least one additional recurrence cycle. It must distinguish durable
negative capture, recurrent scattering, and later release without changing
the action or retroactively rewriting the FTD-0722/0726 classifiers.

## Verification anchors

- protocol `50582DF6…EB83`;
- runner `6EBAC4DA…D210`;
- JSON `ADA8931C…6DB4`;
- CSV `2C40C6B3…FCCA`;
- independent certificate `FF8E9EF3…D71C`, `387/387 PASS`;
- focused CTest `1/1 PASS` in `1826.78 s`.


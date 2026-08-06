# AUDIT — Localized-basin relaxation v2

**Date:** 2026-07-28  
**Identifier:** `FTD-0679`  
**Status:** `[EXECUTION INVALID — OUTPUT CONTRACT]`  
**Canonical verdict:** `LOCALIZED_BASIN_RELAXATION_V2_EXECUTION_INVALID`  
**Raw runner verdict:** `LOCALIZED_BASIN_V2_REMOTE_FIELD_NOT_DOMINANT`

## Result

The full `L=97`, 80-tick forward/reverse history completed with exact dynamics
gates, but the runner violated its registered output schema:

- `observe_donor().target` is already `E_target(t)/E_target(0)`;
- the CSV column labelled `target` stored that ratio, not energy;
- `target_ratio` divided that ratio by `E_target(0)` a second time.

The target columns are therefore mislabeled and the run is not the registered
run of record.  The JSON's `exact_execution_pass=true` does not override this
audit-level conformance failure.

## Non-promotable raw behavior

The independent core and shell classifier did not use either defective target
column.  Its raw values were:

```text
Gamma_core                    0.0065371211 / 0.0065371283
DeltaBIC                      412.6953 / 412.6955
R^2                           0.999331984 / 0.999331987
core decline, tick 8 to 64   0.312296700 / 0.312296913
far fraction at tick 80      0.129272228 / 0.129272254
near fraction at tick 80     0.248317540 / corresponding sign mirror
inverse residual             1.11e-12 / 6.96e-13
```

These values may set expectations for an explicitly registered replication;
they are not a held-out result.  The corrected FTD-0680 storage decoder does
not change these shell values because the campaign origin was `(48,48,48)` and
the shell norm is invariant under the historical x/z exchange.

## Reproducibility

- protocol SHA256:
  `697FC9058FA9AD3A48F10833CAA744C9260570DB3A5AF8F2F8CE97B32C65DF95`;
- runner SHA256:
  `26B45994628350BD979EE1C4CF9B8A6520B7A023D2D8FDB7696C6BDBC57E83D2`;
- executed binary SHA256:
  `BBC2156DA1CFA5DB3A6965589E7AEAC30FA215E8B992FFE9EF57061C1A2D1C16`;
- raw JSON SHA256:
  `0C97C77BF036AB742195684B2B059D3C1BB89BB0B24AE9D9BEDFCCC17BA36AD9`;
- raw CSV SHA256:
  `5B27C13E2C3CBE77BEEC0350FAE5E6B4BA1264CB4BF29F2A196B9471FEA477BA`.

No production state or behavior changed.

# FTD-0728 — Persistence covariance convergence v1

**Status:** `[SELECTED NUMERICAL REALIZATION + MEASURED — ABSOLUTE
COVARIANCE PASS / CONVERGENCE INCOMPLETE]`  
**Verdict:** `PERSISTENCE_COVARIANCE_PASSES_WITH_INCOMPLETE_CONVERGENCE`  
**Production status:** unchanged

## Result

A fresh full replay of the FTD-0727 208-history matrix at root termination
`2e-13` passes the absolute scalar-history covariance gate while preserving
every physical classification:

```text
parent spread                   1.1065308669344631e-9
tighter-root spread             5.6798055148021831e-10
tight/parent ratio              0.51329842524298808
absolute gate                   1e-9                    PASS
registered fivefold ratio       <=0.2                   FAIL
```

All 104 `p=0.0060/0.0095` arms remain negative and graph-connected for ticks
49--96. Their dynamic field again expands from radius three at tick 48 to
radius five or six at tick 96. All 52 `p=0.0120` histories again undergo three
graph transitions and fail the clean escape control; 12/52 finish negative.
All 52 pre-bound controls persist and remain radius two.

## Localization

The worst tight-root covariance difference is constituent separation in the
unbound `p=0.0120`, direction `0_1_-1`, plus-minus, shifted arm at tick 92.
It is not field energy. This localizes the remaining numerical sensitivity to
late matter separation near the re-entry episode.

## Interpretation

The classification-level physics is insensitive to the ten-times tighter
root: persistence, field extension, re-entry, and final sign counts are
identical. The absolute covariance criterion is now satisfied. However, the
preregistered fivefold convergence condition is not. The data therefore do
not yet establish the expected solver-error scaling or fully attribute the
parent miss to ordinary root termination.

The correct next check is targeted convergence of the worst late re-entry
history across one additional decade of tolerance, recording complete matter
and field differences. It is not another full momentum or morphology search.

## Ontological consequence

No new state variable is indicated. The complete state continues to generate
and invert the same class of histories as numerical accuracy increases. The
live ontological uncertainty is environmental: does the extended field and
late re-entry persist with increasing volume, or is it a periodic-volume
recurrence? That volume inference remains gated on the targeted convergence
check.

## Verification anchors

- protocol `F2C1D17A…6412`;
- runner `F2294329…ABB5`;
- JSON `3E9723FE…0F7D`;
- CSV `72621EA4…6F42`;
- independent certificate `4E83074F…809F`, `103/103 PASS`;
- focused CTest `1/1 PASS` in `930.77 s`.


# Audit — FTD-0724 lower-energy formation crossover v1

**Status:** `[AUDIT PASS — EXECUTION UNRESOLVED / RAW CROSSOVER
NON-PROMOTABLE]`  
**Date:** 2026-07-29

## Findings

1. **The registered verdict is unresolved.** Maximum scalar-history spread is
   `1.0681e-8`, exceeding the locked translation/polarity gate `1e-9`.

2. **The failure is global rather than rowwise.** All 312 arms individually
   pass common-action, energy, recoil, and inverse gates; all 52 bound controls
   survive. Those facts do not waive the failed covariance comparison.

3. **Raw energetic trapping is present but non-promotable.** Every arm at the
   four lowest momenta enters once and finishes negative; all `p=0.0120` arms
   enter and exit positive. Raw negative-sector count is `208/260`.

4. **The preregistered extrapolation is not confirmed.** The held-out
   `p=0.0095` family was predicted to escape but is raw-negative in 52/52 arms.
   The apparent transition lies above the fitted envelope.

5. **The FTD-0723 linear descriptor is horizon-dependent.** Field export at
   48 ticks is several times the extrapolated value in raw trapped arms.
   Interaction time and field feedback cannot be omitted from a formation
   model.

6. **No qualified detached-field capture occurs.** Every raw trapped family
   fails the unchanged radius gate with median doubled radius three. The only
   radius-five family escapes.

7. **Threshold chasing is not the next valid action.** The covariance defect
   must first be localized and shown to converge with tighter numerical root
   tolerance under the same action. Refining momenta before that would build on
   an unresolved record.

## Correct statement

FTD-0724 produces a reproducible raw energy-sign transition between
`p=0.0095` and `p=0.0120` in individually exact and invertible histories, but
the campaign fails its locked global covariance gate and produces zero
qualified captures. The energetic pattern is diagnostic evidence for a
conditioning test, not validated matter formation.

## Verification

- preregistration SHA-256:
  `BCCCE4691FA5EBA22C61CB88554DC4972E07B0178C55396CA9A79ED98D3F4762`;
- runner SHA-256:
  `98C3D1B572B695B901C11555765CD4B5BC33F7FFCAEB5C6F638DB96113801208`;
- result JSON SHA-256:
  `068AAFAE5029C3993BF0D7ECA3A9F681A3830B0A6B9E1191011EE6C0B53CD91F`;
- result CSV SHA-256:
  `3D8BFFC1E03559CA7B14E9FA79B1EB8542B117484D0AD6C7E9BA4AC4ACB06D20`;
- independent certificate:
  `FE80A399DDC96FA38A204C1944D084998F4EAC1619C7F3631B4D677CB33899C4`,
  `139/139 PASS`;
- focused CTest: `1/1 PASS` (execution, not scientific promotion);
- production defaults, tick, toggles, and scenarios: unchanged.

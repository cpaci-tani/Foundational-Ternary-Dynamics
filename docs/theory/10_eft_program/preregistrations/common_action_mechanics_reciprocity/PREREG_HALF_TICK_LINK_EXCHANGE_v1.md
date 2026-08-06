# PRE-REGISTRATION — Half-tick link exchange v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0451`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0447` through `FTD-0450`  
**Engine artifact:** `engine/tests/campaign_half_tick_link_exchange.cpp`  
**Campaign SHA256:** `efa7f7e897765e039e518da439d0013c85ecce1e7a24c5dcca107f6a904a5d51`  
**Helper SHA256:** `eeebe9622acb6a23a87e78ecf68884a58fc3b655000615da5e531c07fef5baf8`

## 1. Question

Can the corrected production-compatible selected momentum map be packaged as a
local, oriented, half-tick exchange record that closes momentum and energy,
cancels under an independently recomputed reverse transaction, and transforms
covariantly under the cubic group?

## 2. Frozen selected object

For each oriented Moore displacement `d` at tick `n`, store a record at
`2t=2n+1` containing:

- one of 13 unoriented Moore-channel indices plus orientation sign;
- particle momentum before/after from FTD-0450's corrected selected map;
- field momentum exchange `p_before-p_after`;
- particle work `W` and field energy exchange `-W`.

The initial production momentum comes from velocity
`0.15 d_hat + 0.03 transverse_hat`. Registered work is `1e-4`.

## 3. Frozen gates

Across all 26 directions:

- channel index/orientation reconstructs `d` exactly;
- reverse displacement uses the same channel and opposite orientation;
- per-record particle-plus-field momentum and energy residuals `<=1e-12`;
- independently recomputed reverse work `-W` restores particle momentum to
  `1e-12`;
- forward/reverse field momentum and energy records cancel to `1e-12`;
- transformed momentum/recoil is covariant under all 48 signed coordinate
  permutations, `1248` cases, residual `<=1e-12`;
- transformed channel record reconstructs the transformed displacement exactly.

## 4. Locked outcomes

- `REVERSIBLE_HALF_TICK_LINK_LEDGER_CONSTRUCTED_NOT_DYNAMICS`: all gates pass.
- `PROTOCOL_INVALID`: any gate fails.

## 5. Interpretation boundary

The record is a sufficient reversible exchange ledger, not a field theory.
It assigns recoil momentum and energy numbers but does not realize them in
production `J/W`, define a 13-channel Hamiltonian, or propagate the link state.

The primitive-channel representation and the finite momentum branch are
selected. FTD-0447 derives only the isolated response direction. Passing this
campaign licenses the statement “a reversible local record can be built,” not
“native matter mechanics has emerged.”

## 6. Banned moves

- No channel basis, velocity, work, half-tick convention, map, group, tolerance,
  or outcome label may change after first execution.
- No production tick or field mutation.
- No promotion from ledger closure to physical energy conservation.

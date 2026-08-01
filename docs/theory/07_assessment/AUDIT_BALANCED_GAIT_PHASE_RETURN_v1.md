# Audit — FTD-0620 balanced-gait phase return

**Status:** `[AUDIT — RECURRENCE CLOSED IN LOCKED WINDOW; INTERMITTENT
MULTIMODE TRANSPORT MEASURED]`
**Verdict:** `BALANCED_GAIT_PHASE_BEHAVIOR_MIXED`

- protocol SHA-256: `A5B97A92...B78C`;
- FTD-0618 parent SHA-256: `5F04E64D...54D3`;
- FTD-0619 parent SHA-256: `0FEE2158...669D`;
- runner SHA-256: `65EF959C...576C`;
- result JSON SHA-256: `0D66A13C...08DF`;
- certificate SHA-256: `AFBA2942...B2297`;
- independent certificate: 29/29 checks pass;
- run of record: `engine/results/ftd_0620/`;
- production dynamics: unchanged.

The rest arm and both active signs complete 2,304 forward/reverse transactions.
Action, energy, geometry, sign mirror, and state-only inverse gates pass.  No
active arm reaches the registered matter-internal return neighborhood in 512
ticks; the best normalized distance is `5.22037` against `0.05`.

The gait is not classified as a one-time relaxation.  It retains `63.03%` of
its initial internal-momentum norm, reaches `3.36880` cells total displacement,
and recovers from a `0.210845`-cell third window to a `1.10214`-cell fourth
window.  The correct finite-window statement is intermittent multimode
transport with no observed recurrence.  Long-period recurrence remains open.

The phase angle is only a two-mode projection.  Its winding does not license a
clock or cyclic-particle claim because the remaining internal coordinates do
not return.

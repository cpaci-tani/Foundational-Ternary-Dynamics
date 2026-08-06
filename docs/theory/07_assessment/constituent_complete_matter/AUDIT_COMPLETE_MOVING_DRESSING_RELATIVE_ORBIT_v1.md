# Audit — FTD-0706 complete moving-dressing relative orbit

**Verdict:** `[EXECUTION INVALID — FAILED LOCKED REST CONTROL]`

The independent certificate pins the preregistration, runner, JSON, and CSV;
checks the exact two-tick, one-cell target; confirms common-action, energy,
state-only inverse, and integer-translation covariance gates; and independently
enforces the failed rest fixed-point gate. It passes.

The run cannot be classified as either a complete moving-dressing success or a
moving-dressing no-go. The `L=33` state used as the static control changes by
`1.88e-5` in two ticks, while the protocol required at most `1e-9`. Under the
locked decision rule, that defect invalidates the execution.

The small position/momentum translation residuals and large electric/magnetic
translation residuals are diagnostic evidence that a static field was boosted
inconsistently. They do not override the failed control. No post-hoc redressing,
tolerance relaxation, or verdict substitution is admissible on this run.

The correct next test is a new preregistered preparation: qualify an exact
finite-volume rest fixed point first, then solve or falsify the complete-state
relative-orbit equation without replacing the field after motion.

See
[`ANALYSIS_COMPLETE_MOVING_DRESSING_RELATIVE_ORBIT_v1.md`](../10_eft_program/derivations/ANALYSIS_COMPLETE_MOVING_DRESSING_RELATIVE_ORBIT_v1.md).


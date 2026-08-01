# Audit — FTD-0615 zero-momentum internal-mode mobility

**Status:** `[AUDIT — CONSTRUCTIVE INTERNAL WALKER; DIRECTION/RECURRENCE OPEN]`
**Verdict:** `ZERO_MOMENTUM_INTERNAL_WALKER_CONSTRUCTIVE`

- protocol prefix SHA-256: `1F8B86C2...7104B`;
- locked FTD-0614 parent SHA-256: `8A286636...BA45`;
- runner: `engine/tests/test_zero_momentum_internal_mode_mobility.cpp`;
- certificate: `scripts/proofs/proof_zero_momentum_internal_mode_mobility.py`;
- independent checks: 23/23 pass;
- run of record: `engine/results/ftd_0615/`.

All 24 arms complete 128 forward and 128 state-only inverse ticks.  Initial
matter-centre momentum is zero to `3.30e-18`; common-action residuals stay
below `1.98e-13`, energy drift below `3.24e-14`, and recovery below
`1.38e-11`.  Geometry remains intact.

Four high-energy rotational arms meet the locked walker criterion, translating
`1.45360` cells.  All three strain families remain bounded below `1.97e-4`
cell centre excursion, and the third rotation axis is intermediate at high
energy.  Thus the measured effect is mode-selective rather than an arbitrary
conversion of internal energy into centre motion.

The runner's field `parent_hash_pass` is a stored-record fingerprint check;
the independent certificate separately computes and verifies the complete
cryptographic parent hash.  The record does not store displacement direction
or internal phase recurrence.  Its nonzero pseudomomentum defect and external
uniform neutralizer prevent an isolated-momentum or self-propulsion claim.
No production behavior, toggle, scenario, particle, pole, Lorentz, or
electromagnetic claim changes.


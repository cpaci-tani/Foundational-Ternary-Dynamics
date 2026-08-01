# Audit — FTD-0607 site-admissible compact matter motion v1

**Status:** `[AUDIT — STATIC EXISTENCE CONSTRUCTIVE AT FIVE PHASES; MOTION
UNRESOLVED]`
**Verdict:** `SITE_ADMISSIBLE_COMPACT_MATTER_NUMERICALLY_UNRESOLVED`

## Reproducibility record

- locked protocol prefix SHA-256:
  `CA37FB9700A2416FE293B26A903A9DCA5233091C215E0AEB83D92BA802D871E9`;
- observer runner:
  `engine/tests/test_site_admissible_compact_matter_motion.cpp`;
- independent certificate:
  `scripts/proofs/proof_site_admissible_compact_matter_motion.py`;
- records:
  `engine/results/ftd_0607/ftd_0607_site_admissible_motion_v1.json`,
  `ftd_0607_site_admissible_static_samples_v1.csv`, and
  `ftd_0607_motion_ticks_v1.csv`;
- focused CTest: `site_admissible_compact_matter_motion`, pass;
- independent certificate: pass.

## Gate disposition

| gate | result |
|---|---|
| periodic Green/direct-field baseline | pass; `5.43e-16` |
| admissible registered starts | pass; 24/24 at every phase |
| optimizer termination | pass at every phase |
| best-energy repeatability cluster | fail at 22/32 phases |
| all static gates | pass at phases `14,15,16,17,26` |
| qualified chart margin | minimum `5.81e-3` |
| qualified stationarity | worst gradient `1.07e-7` |
| qualified six-mode stability | minimum eigenvalue `6.31e-4` |
| qualified direct field | worst gate `8.79e-16` |
| phase-zero selection | fail; chart margin `2.22e-14` |
| autonomous motion | not executed by the lock |
| state-only inverse | not executed by the lock |
| integer translation covariance | not executed by the lock |

## Audit conclusion

The campaign establishes a narrower positive result than its title might
suggest: the selected compact family contains stable, field-clean states that
respect finite ternary-site capacity. It does not establish their mobility.

The numerical verdict is unresolved because the preregistered coverage and
phase-zero gates fail before motion. Treating the empty motion record as a
negative dynamics result would be false. Treating the five qualified phase
slices as a phase-robust material ground state would also be false.

The phase-zero requirement is now seen to be a poor dynamical launch condition
for this family: it selects a chart-boundary state even though independently
qualified interior states exist nearby. A new preregistration may choose one
qualified phase before execution and ask the actual transport question. That
is a new experiment, not a reinterpretation of FTD-0607.

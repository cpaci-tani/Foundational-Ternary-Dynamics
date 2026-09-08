# Registered hydrodynamic response campaign: independent audit

Date: 2026-09-08. **Disposition: [OBSTRUCTION RETAINED] registered acceptance
not met (1/42 cells); per the preregistration the failing cells are the
retained obstruction, not a corrected result.**

An independent, read-only pass reproduces the retained campaign report from
`engine/build_strict_hydro/campaign_v1`, replays two of the 336 manifest
cases against an out-of-campaign CUDA invocation and a pure-Python reference,
and computes a post hoc diagnostic on the persisted per-cell report. No
registered quantity, acceptance criterion, source file, or number was
changed by this audit.

## Protocol and evidence

The [preregistration](PREREG_STRICT_HYDRODYNAMIC_RESPONSE.md) fixes the law
`phi-v2-staged-candidate-1`, a periodic `L = 32` lattice, `p = 1/96`,
`epsilon = 1/4`, 23 stroboscopes of 48 microticks each, and the matrix
3 directions x 2 wavenumbers x 7 stage-0 conserved modes x 8 seeds = 336
cases (370,944 physical microticks total). The fixed acceptance, per (k,
mode) cell: relative RMS deviation of the seed-mean trajectory from the
locked linear-Boltzmann prediction at most 1/10, and the declared rate
estimator (least-squares slope of ln|m(n)| against n) within a 2-standard-error
band of the predicted rate, both jointly. All 42 cells must pass for closure
to be verified at this scope; otherwise the failing cells are the retained
obstruction, with no retuning.

The persistent [campaign report](evidence/strict-hydro-response-v1.json) is
a copy of `engine/build_strict_hydro/campaign_v1/report.json`; independently
rehashing both files gives an identical SHA256
(`092d36821051b9f234ea24566d6b939eceaa5b1fba24f0ef8c41caa40b5dc4e6`), so the
committed evidence is byte-for-byte the retained report. The local replay
directory `engine/build_strict_hydro/campaign_v1/` retains the registration
lock, all 336 locked preparations and 336 final checkpoints, the locked
linear-Boltzmann predictions, the compact stroboscope trace, and the
preflight/execution receipts. It is a local build output (4.3 GB) and was
not modified by this audit.

| Frozen evidence | SHA256 |
|---|---|
| Scientific registration | `11af36d30bcf10acb43cd55bf3406a898e027ae15dd5cd59ffa399b494d45620` |
| Case manifest | `3cf1dd41e9054767180293c51543ab2800ffb387f6e75a8e6af7a53ccb6bb204` |
| Locked linear-Boltzmann predictions | `06d53ea3e26365cd41f6476f8c4f61477766230286913336bb9ec7144b3f5498` |
| Compact stroboscope trace | `1d7d1fec8971bd17042ddb4bd00eb71c24b1f8aac11078e55546530a10ca895c` |
| CUDA campaign runner (`ftd_strict_hydro_campaign`) | `b2192d2a77287dd7a7ca754dfc90d1975ff43231247de2b60b2fe890507d7f35` |
| Weights table | `b480a78f53ede91c4020acf9c17d8aaafee12536ba335fb39d2558e01f1a9f94` |

The accepted launch used the WSL2 Ubuntu-22.04 CUDA backend
(`cuda_device_kernels`, NVIDIA GeForce RTX 5090, compute capability 12.0).
The retained execution receipt (`execution.json`) reports 370,944 physical
microticks in 3388.7405394000234 seconds (~56 min 29 s), binds its trace
hash to the preflight receipt (which binds to the lock), and reports
`postflight: complete frozen-source/input validation passed`. This is a
correctness-campaign timing, not a browser or simulation-throughput claim.

## Reproduction

Running `recovery_hydro_campaign.validate_lock` then `summarize_campaign`
against the retained `campaign_v1` directory (no report writes suppressed;
`summarize_campaign` rewrites `report.json` to its own retained content)
reproduces the following, checked against the committed evidence file:

| Key | Reproduced | Committed evidence |
|---|---|---|
| `groups` | 42 | 42 |
| `groups_passing` | 1 | 1 |
| `boltzmann_closure_verified_at_registered_scope` | `false` | `false` |
| `registration_sha256` | `11af36d3...45620` | `11af36d3...45620` |
| `manifest_sha256` | `3cf1dd41...6bb204` | `3cf1dd41...6bb204` |
| `predictions_sha256` | `06d53ea3...44b3f5498` | `06d53ea3...44b3f5498` |
| `trace_sha256` | `1d7d1fec...ca895c` | `1d7d1fec...ca895c` |

All seven keys match exactly. As a second, independent check, an
out-of-pipeline recomputation of `predict()` for the first manifest case
(`h_d100_m1_mode0_s0`) reproduces the locked `predictions.json` values with a
maximum absolute difference of `0.0` across all 24 stroboscope points and 7
modes (exact double-precision agreement).

## Independent replay

`scripts/phi_v2_lattice/experiments/replay_hydro_case.py` replays the first
two manifest cases (`h_d100_m1_mode0_s0`, `h_d100_m1_mode0_s1`) against two
independent paths. It never writes into `campaign_v1/`; scratch files go
under `engine/build_strict_hydro/replay/`.

1. **8-microtick Python-vs-CUDA.** The locked preparation is advanced 8
   microticks with the pure-Python `staged.step` reference (called 8 times)
   and, separately, with an 8-tick invocation of the accepted CUDA CLI
   `ftd_strict_cuda_cli` on the same preparation file. The pure-Python
   reference is too slow to replay a full 48-microtick stroboscope at
   `L = 32`, so only the first 8 of the 48 microticks in a stroboscope are
   compared. The resulting complete-state SHA256 hashes are compared.
2. **48-microtick CUDA-vs-trace.** The same locked preparation is advanced
   48 microticks (one full stroboscope) with an independent, out-of-campaign
   invocation of the same accepted CUDA CLI (not the campaign runner
   `ftd_strict_hydro_campaign`), and the resulting state hash is compared
   against the `stroboscopes[1].sha256` value already recorded in the
   campaign's `trace.jsonl`. This ties the campaign trace to a separate CUDA
   run of the accepted backend, not merely to itself.

| Case | 8-tick Python == CUDA | 48-tick CUDA == trace |
|---|---|---|
| `h_d100_m1_mode0_s0` | `True` | `True` |
| `h_d100_m1_mode0_s1` | `True` | `True` |

Scope: 2 of 336 cases (0.6%); 8 of 1,104 microticks per case for the
Python-vs-CUDA comparison, the full 48-microtick stroboscope for the
CUDA-vs-trace comparison. This establishes agreement between the pure-Python
reference and the accepted CUDA backend on a short horizon for two cases,
and ties the trace to an independent CUDA invocation on one full period for
those same two cases; it does not replay the other 334 cases, the full
1,104-microtick horizon in Python, or the remaining 22 stroboscopes of these
two cases.

## Post hoc diagnostic (not the acceptance of record)

For every one of the 42 registered (k, mode) cells, using only the retained
`report.json` / committed evidence and the locked `predictions.json` (no new
campaign quantity, no change to the registered acceptance rule), this audit
computes, over the 24 stroboscope points n = 0..23 of the seed-mean
trajectory `mean(n)` and its standard error `stderr(n)`, against the locked
prediction `predicted(n)` for the same (k, mode):

- `D = max_n |mean(n) - predicted(n)| / max_n |stderr(n)|` (the largest
  trajectory deviation from prediction, relative to the largest seed
  standard error; both maximized separately over n);
- `F = rms(stderr over n) / rms(|predicted| over n)` (a ratio of the seeds'
  own reporting noise to the size of the predicted signal — a noise-floor
  estimate);
- the fraction of the 24 stroboscope points at which `|mean(n)| < 2 |stderr(n)|`
  (the seed-mean signal is not resolved from zero at 2 standard errors).

| k | mode | relative_rms | F | D | D<=2 | frac signal-in-noise |
|---|---|---:|---:|---:|---|---:|
| [1,0,0] | 0 | 0.714965 | 0.111661 | 8.819551 | False | 0.2500 |
| [1,0,0] | 1 | 0.282936 | 0.063252 | 6.973849 | False | 0.0000 |
| [1,0,0] | 2 | 0.223409 | 0.071859 | 5.196998 | False | 0.0000 |
| [1,0,0] | 3 | 0.268363 | 0.065565 | 6.777097 | False | 0.0000 |
| [1,0,0] | 4 | 0.077985 | 0.064531 | 1.930732 | True | 0.0000 |
| [1,0,0] | 5 | 0.365181 | 0.111005 | 4.910281 | False | 0.0417 |
| [1,0,0] | 6 | 0.186170 | 0.120286 | 2.015353 | False | 0.2500 |
| [1,1,0] | 0 | 0.800316 | 0.103465 | 13.067427 | False | 0.0833 |
| [1,1,0] | 1 | 0.880760 | 0.109617 | 11.547964 | False | 0.2500 |
| [1,1,0] | 2 | 0.321561 | 0.099582 | 5.751221 | False | 0.2500 |
| [1,1,0] | 3 | 0.327619 | 0.100640 | 6.073837 | False | 0.2500 |
| [1,1,0] | 4 | 0.402470 | 0.113785 | 6.882553 | False | 0.2500 |
| [1,1,0] | 5 | 0.764052 | 0.170416 | 7.498926 | False | 0.2500 |
| [1,1,0] | 6 | 0.737874 | 0.152088 | 8.237920 | False | 0.1250 |
| [1,1,1] | 0 | 0.728342 | 0.153639 | 8.070061 | False | 0.2500 |
| [1,1,1] | 1 | 0.699046 | 0.144120 | 7.950756 | False | 0.1667 |
| [1,1,1] | 2 | 0.738550 | 0.126246 | 9.241514 | False | 0.2500 |
| [1,1,1] | 3 | 0.664510 | 0.121674 | 8.510011 | False | 0.2083 |
| [1,1,1] | 4 | 0.745346 | 0.158902 | 8.393199 | False | 0.1667 |
| [1,1,1] | 5 | 0.704893 | 0.231976 | 4.577935 | False | 0.2500 |
| [1,1,1] | 6 | 0.746145 | 0.230160 | 5.349410 | False | 0.2500 |
| [2,0,0] | 0 | 0.814610 | 0.129551 | 10.018436 | False | 0.5000 |
| [2,0,0] | 1 | 0.206520 | 0.073863 | 4.800416 | False | 0.0000 |
| [2,0,0] | 2 | 0.214764 | 0.074991 | 5.176339 | False | 0.0000 |
| [2,0,0] | 3 | 0.277250 | 0.077864 | 6.456801 | False | 0.0000 |
| [2,0,0] | 4 | 0.083288 | 0.059433 | 2.293924 | False | 0.0000 |
| [2,0,0] | 5 | 0.316860 | 0.121299 | 4.765892 | False | 0.4167 |
| [2,0,0] | 6 | 0.214168 | 0.116755 | 2.637622 | False | 0.3750 |
| [2,2,0] | 0 | 0.777174 | 0.092407 | 11.820898 | False | 0.1667 |
| [2,2,0] | 1 | 0.780790 | 0.088753 | 13.249062 | False | 0.5000 |
| [2,2,0] | 2 | 0.274104 | 0.084808 | 5.045623 | False | 0.5000 |
| [2,2,0] | 3 | 0.304800 | 0.082686 | 5.402523 | False | 0.5000 |
| [2,2,0] | 4 | 0.396251 | 0.105056 | 6.222261 | False | 0.4583 |
| [2,2,0] | 5 | 0.662895 | 0.125602 | 7.261733 | False | 0.4583 |
| [2,2,0] | 6 | 0.744355 | 0.126237 | 7.894607 | False | 0.5000 |
| [2,2,2] | 0 | 0.667100 | 0.106035 | 8.969225 | False | 0.5000 |
| [2,2,2] | 1 | 0.767362 | 0.115753 | 8.451610 | False | 0.5000 |
| [2,2,2] | 2 | 0.721501 | 0.108846 | 9.062829 | False | 0.5000 |
| [2,2,2] | 3 | 0.713720 | 0.109168 | 9.679849 | False | 0.5000 |
| [2,2,2] | 4 | 0.762735 | 0.096983 | 9.533405 | False | 0.3750 |
| [2,2,2] | 5 | 0.763485 | 0.169938 | 6.788463 | False | 0.3750 |
| [2,2,2] | 6 | 0.697999 | 0.177372 | 4.973910 | False | 0.5000 |

28 of 42 cells have `F >= 0.1`, the registered relative-RMS tolerance: for
those cells the seed-to-seed reporting noise alone, by this ratio, is at or
above the size of the tolerance the acceptance rule allows for the
trajectory-shape error. 1 of 42 cells has `D <= 2`
(`[1,0,0]` mode 4, `D = 1.930732`); this is not the same cell that passed
the registered joint criterion (`[2,0,0]` mode 4, whose `D = 2.293924`
here) — `D` and the registered rate-band criterion are different statistics
(a single worst-point ratio against a single worst standard error, versus a
least-squares rate fit against the standard error of seed-level rate
estimates) and are not expected to agree cell-for-cell. The fraction of
stroboscope points at which the seed-mean signal is not resolved from zero
at 2 standard errors ranges from 0.0 (several axis-aligned cells) to 0.5
(several diagonal-direction cells).

A noise floor at or above the registered tolerance means the registered
criteria had low statistical power at this signal-to-noise ratio for the
affected cells; it does not verify the Boltzmann closure, and it does not
retroactively pass or fail any cell under the registered acceptance rule.
This diagnostic is post hoc, is not part of the preregistered acceptance,
and changes no registered quantity.

The full diagnostic, as JSON, is appended to the task report
(`.superpowers/sdd/2026-09-07-phi-hydrodynamics-phase1/task-11-report.md`).

## Defects

No defect was found in the retained campaign artifacts, the
lock/preflight/execution/report chain, or the `recovery_hydro_campaign`
module within the scope of this audit: the reproduced summary matches the
committed evidence exactly on every checked key; `report.json` and the
committed evidence copy are byte-identical; an independent recomputation of
`predict()` for one case matches the locked `predictions.json` value exactly;
and both replay comparisons (8-tick Python-vs-CUDA, 48-tick CUDA-vs-trace)
matched for both replayed cases. No repair is indicated by this audit's
scope; nothing was returned to the implementer.

## Disposition

The registered acceptance — all 42 cells jointly passing the relative-RMS
and rate-band criteria — was not met: 1 of 42 cells passed
(`boltzmann_closure_verified_at_registered_scope = false`). Per the
preregistration ("Failure is retained as 'Boltzmann closure fails for the
staged law at this preparation', not corrected"), the 41 failing cells are
retained as the obstruction at this registered scope: `L = 32`, `p = 1/96`,
`epsilon = 1/4`, the seven stage-0 conserved modes, and the six registered
(direction, wavenumber) pairs. This audit changed no registered quantity,
source, preparation, or acceptance criterion. The post hoc diagnostic above
characterizes the measurement's signal-to-noise ratio at the registered
scope; it is not a substitute for, and does not reinterpret, the registered
acceptance rule or its outcome.

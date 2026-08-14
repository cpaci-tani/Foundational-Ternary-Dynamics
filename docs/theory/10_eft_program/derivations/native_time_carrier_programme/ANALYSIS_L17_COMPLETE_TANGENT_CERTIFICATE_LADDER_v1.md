# FTD-0829--0834 — L=17 complete-tangent certificate ladder

**Status:** `[MEASURED — FOUR EXECUTION-INVALID SUCCESSORS PLUS TWO REPLAY REPAIRS; NO PHYSICS VERDICT]`  
**Scope:** isolated reconstruction and target-blind numerical certification of the locked FTD-0774 complete tangent candidate  
**Production impact:** none  
**Date:** 2026-08-10

## 1. Result in one sentence

The FTD-0774 candidate still has no tangent-clock verdict: FTD-0832's
preregistered nonsingular direct-product chart finally passed preflight and
completed Krylov, but the independent replay failed one ill-conditioned
principal-angle serialization cross-check; producer and replay agree on one
eligible, zero qualified, unresolved candidate, yet the locked `94/95`
certificate cannot be promoted.

## 2. Provenance and execution isolation

All successor executables were built in the isolated worktree

```text
C:\Users\cpaci\AppData\Local\Temp\ftd-0774-clean-run-20260810
```

at production source commit
`93748ac2021e4db5a9b8583cc28493332c716ac0`. The exact FTD-0638/0639/0640/0641
parents and compiled closure matched the FTD-0774 hashes. The production
source gate

```text
git diff --quiet 93748ac... -- engine/include/ftd engine/src/eft
```

passed before each execution. MSVC `14.44.35207` built the test-only wrappers.
No production physics file changed and no numerical target search was run.

The `74/74` FTD-0831 replay and `94/95` FTD-0832 replay quoted below are from that
source-pinned worktree. Running the same verifier from the dirty main checkout
fails closed before the runtime audit because unrelated current production
files differ from the locked source commit. The clean worktree source gate
returns zero. This is intended provenance behavior; the verifier was not
weakened to make a dirty-checkout replay pass.

## 3. Successor ledger

| ID | protocol SHA-256 | registered repair | measured terminal |
|---|---|---|---|
| FTD-0829 | `04C771A53E0A749492359255C613BD72A693A399920C0F3CA0FAE757931F361F` | periodic-divergence Hodge mean normalized by parent residue; semantic 98-group order | execution-invalid; old Hodge singularity removed, but 56/64 primary probes failed only the near-zero face-harmonic relative reconstruction and the first verifier hit a row-label bug |
| FTD-0830 | `A0D660F846D6C9AF43D94D475F1890E3D90D3967C6218B065ABDD9AA3BBFA5EC` | four stable post-reinsertion mean-correction passes; fail-closed verifier label | execution-invalid; 49/64 primary probes still failed only the face-harmonic pure-relative gate because the required change lay below a field-entry ULP |
| FTD-0831 | `A7BA4CEE3CC57AEC23CA9B9F60B0330C1E5B09EDBFC07FD5CC3E441AC736B3A1` | mixed `1e-12` relative plus `gamma_(L^3+32)` binary64 backward-error floor | execution-invalid; all 64 primary probes and 24/32 compositions passed; exactly eight compositions failed Hodge-correction/reconstruction component-relative gates |
| FTD-0832 | `2CE5516F7C0D4AF06649D54DD50C1E680C5BEE7CBEDFA10BDB09C2669FA81805` | explicit electric harmonic coordinate plus complete direct-product codec norm | execution-invalid at replay; preflight and Krylov complete, one eligible primary cluster, zero qualified; producer/replay verdict scalars agree but candidate metric-row replay is `94/95` |
| FTD-0833 | pre-run `637712E215E5B1C1D75267310BE21005CCA15F3D33D587059CA7E15A0503AD0C` | inject the unchanged `scalar_close` helper into the unchanged verifier | replay repair invalid; exposed the second undefined v5 helper `required` before any artifact verdict |
| FTD-0834 | `791AE3BB024ED245C76AF6E28ACA2FEDC60891349529FF29E79ECACBCD5C09BC` | inject both unchanged helper semantics into the unchanged verifier | certificate invalid; terminal replay reaches `94/95` and isolates the `sign_angle` scalar disagreement |

The FTD-0829 and FTD-0830 invalid corpora are retained under
`engine/results/ftd_0829/` and `engine/results/ftd_0830/`. The complete
FTD-0831 and FTD-0832 corpora are under `engine/results/ftd_0831/` and
`engine/results/ftd_0832/` respectively.

## 4. What FTD-0831 established

The independent replay reports:

```text
FTD-0831 independent tangent certificate: 74/74 checks PASS
execution_valid=false
solve_resolved=false
eligible_candidate_count=0
qualified_candidate_count=0
verdict=L17_COMPLETE_TANGENT_EXECUTION_INVALID
```

The producer and verifier agree. All provenance, source, representative,
option, Hessian, gradient, energy-form, seed-metric, root-regularity, cache,
field-control, artifact-schema, and hash gates pass. Useful measured maxima are:

| quantity | measured | locked gate |
|---|---:|---:|
| endpoint common residual | `1.9893185348019329e-13` | `<=1e-10` |
| complete-energy drift | `1.7763568394002505e-15` | `<=1e-12` |
| forward/reverse recovery | `2.2705874572714566e-14` | `<=1e-10` |
| two-scale derivative residual | `2.1921283586046409e-08` | `<=1e-3` |
| full composition residual | `1.2188019066854827e-08` | `<=1e-4` |
| adjoint residual | `1.2151477601918292e-09` | `<=1e-4` |
| minimum root `sigma_min` | `1.002606991238185` | `>=1e-3` |
| maximum condition number | `17.906141970319151` | `<=1e4` |
| maximum repaired harmonic residual | `3.6322540596672727e-16` | `<=1e-12` |

Every one of the 64 direct probe codecs passes. Of 32 composition codecs, 24
pass and eight fail both `hodge_correction <=2e-4` and
`reconstruction <=2e-4`:

| input column | semantic input | two orders | correction range |
|---:|---|---:|---:|
| 2 | `p6` | 2 | `0.5140636--0.5152344` |
| 3 | `p7` | 2 | `0.5254754--0.5281639` |
| 9 | `f_b` | 2 | `0.3275992--0.3289408` |
| 15 | `p6+f_b` | 2 | `0.4360338--0.4362892` |

These are precisely the registered directions whose input electric component
is zero. Their pre-clean divergences remain only
`4.29e-10--8.01e-10`, and cleaned divergences are `O(1e-23)`. The large
reported fractions arise because the FTD-0774 codec divides the small Hodge
correction by the equally small centered electric output component. The full
composition residual is small and passes.

## 5. Why there is no fourth repair in this analysis

The prior repairs were forced by exact identities or declared arithmetic:

- a periodic divergence has exactly zero total source;
- serialization must retain its registered semantic order;
- a binary64 mean has a calculable representability floor.

The remaining normalization has no unique forced replacement. Plausible
denominators include the electric component, the complete chart norm, the
input `K` norm, an endpoint-error-over-`h` floor, or a mixed block norm. They
answer different questions. Choosing one after observing which directions
fail would be a post-hoc model selection, even if the full composition
residual looks favorable.

Therefore:

> **[OPEN]** A fresh chart protocol must declare, before execution, whether
> constraint-cleaning error is component-relative or full-tangent-relative,
> and it must prove that the norm remains nonsingular on legitimate zero
> components.

Until that protocol exists, the eight rows are an execution/certificate
boundary, not a clock no-go and not evidence for a clock.

## 6. FTD-0832 complete-chart result and replay boundary

FTD-0832 declared the complete direct-product chart norm before execution.
The clean-source producer then passed every front and preflight gate. Measured
maxima improved to:

| quantity | measured | locked gate |
|---|---:|---:|
| complete-chart Hodge correction | `1.6988499127989015e-09` | `<=2e-4` |
| complete-chart reconstruction | `1.6988499136610722e-09` | `<=2e-4` |
| full composition residual | `1.2188019066854769e-08` | `<=1e-4` |
| adjoint residual | `1.2151477601918292e-09` | `<=1e-4` |

Krylov executed `1920` derivative evaluations. Each construction reached a
64-dimensional final space. The primary final spectrum contained one
eligible rank-four cluster near the internal phase, and the producer built
the matching `h1`, sign, and rotation candidates. All four failed the core
gate; the primary Ritz residual was `0.01359555`, far above `2e-4`, with
invariance residuals about `0.029`. The producer therefore reported one
eligible, zero qualified, and an unresolved solve.

That is descriptive, not a certified physics verdict. The original replay
had two v5 module/local-scope `NameError`s. FTD-0833 and FTD-0834 repaired
only launch-time name resolution while preserving the verifier and corpus
hashes. The resulting replay reached `94/95`; its sole failure was:

```text
producer sign_angle = 7.300048299977713e-08
replay sign_angle   = 0.0
locked row tolerance = 2e-8
physical sign-angle gate = 1e-6
```

The serialized primary-to-sign cross-Gram is `-I` to roughly `1e-15`.
Consequently `sqrt(max(0,1-sigma_min^2))` is ill-conditioned at zero: the
producer retains a sub-unit singular value and the replay rounds its
recomputed value to at least one. This explains the disagreement but does not
erase it. A stable squared-angle or cross-Gram certificate would be a new
protocol selected after seeing this boundary. It is not run here.

FTD-0832 therefore takes registered Outcome A: execution invalid at
independent replay, with no tangent or clock verdict.

## 7. Consequence for the substrate-native clock programme

FTD-0826 still establishes a substrate-native oriented **modal** clock. This
campaign does not promote it to a bounded autonomous local body. FTD-0832
descriptively supplies one eligible but core-failing tangent cluster; because
the independent certificate fails, it supplies no certified tangent, no
localization, no nonlinear continuation, no finite-period return, and no
gate/energy ledger.

The ordinary local-clock front now forks:

1. **Fresh stable-certificate route:** retain the now-nonsingular complete
   chart, but preregister a numerically stable invariant for coincident
   subspaces before any replay. The observed principal-angle scalar may not
   set that new arithmetic rule. Prefer an analytic/automatic tangent action
   over a solve-tolerance-divided finite difference.
2. **Independent localized-carrier route:** construct a bounded native
   wavepacket/body with the FTD-0828 phase-current, recurrence, support, and
   energy/work gates without depending on the FTD-0774 tangent chart.

The native `G*` front remains separate: first prove exact critical quarticity
for the registered unit-strut-tensegrity class, then apply the FTD-0827 CM
gearbox. None of FTD-0829--0834 changes that ordering.

## 8. Epistemic verdict

- **[MEASURED]** FTD-0831 artifact and independent replay are valid and agree
  `74/74` on an execution-invalid verdict.
- **[MEASURED]** FTD-0832 passes its nonsingular product chart and completes
  Krylov, but its independent replay is `94/95`; the producer's unresolved
  result is descriptive only.
- **[CORRECTION]** The previous “FTD-0774 corpus incomplete” status is
  superseded by a complete successor corpus, but not by a physics verdict.
- **[CORRECTION]** The chart-norm singularity is repaired. The live certificate
  boundary is now the ill-conditioned principal-angle metric-row replay, not
  a zero-component codec denominator.
- **[OPEN]** The selected complete endpoint's tangent-clock candidate remains
  unadjudicated because the locked independent certificate does not pass.
- **[OPEN]** Bounded autonomous local clock hardware remains unestablished.
- **[OPEN]** Native critical-quartic `G*` hardware remains unestablished.

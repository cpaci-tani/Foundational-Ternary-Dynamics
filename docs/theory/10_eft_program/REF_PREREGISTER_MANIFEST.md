# Pre-Registration Manifest

**Purpose:** single authoritative table mapping every pre-registered
FTD measurement to (a) the git tag committed BEFORE the run, (b) the
commit SHA the tag points at, (c) the script and any flags used, (d)
the output directory the campaign emits to, and (e) the analysis
document that interprets the result.

**Why it lives here:** the `engine/results/` gitignore default makes
new campaign outputs **local-only** by default — analysis docs cite
result paths that won't exist in a fresh clone. This manifest gives
posterity a recipe for reproducing each campaign from a tagged
commit.

**Discipline:** SHA256 of every pre-registered measurement script is
recorded in the corresponding analysis document (e.g.
`AUDIT_LOOK_ELSEWHERE_RESULTS.md`). The git tag locks the SHA at
pre-registration time. To verify a tag's commit hasn't drifted, run:

```sh
git rev-list -n1 <tag-name>     # commit SHA
git tag -l <tag-name>            # tag listing
```

---

## Pre-registered campaigns (2026-04-27 / 2026-04-28 cycle)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0097** look-elsewhere scan | `preregister-look-elsewhere-scan-v1` | `f11dcaa` | `tools/scan_look_elsewhere.py` | `--epsilon 1e-3,1e-4` | `engine/results/look_elsewhere_2026-04-27/` | [`AUDIT_LOOK_ELSEWHERE_RESULTS.md`](../07_assessment/AUDIT_LOOK_ELSEWHERE_RESULTS.md) |
| **FTD-0105** lemniscatic 2-sphere test | `preregister-lemniscatic-v1` | `7bc2185` | `engine/build_wsl/benchmark_black_hole_thermo` | `--lemniscatic-mode` | `engine/results/lemniscatic_*` | LEDGER row FTD-0105 |
| **FTD-0106** G\*/π asymmetry scan | `preregister-gstar-asymmetry-v1` | `edd1349` | (theory-only catalog committed; engine measurements deferred) | n/a | n/a yet | LEDGER row FTD-0106 |
| **FTD-0107** emergent-spectrum L=64 G1 | `preregister-emergent-spectrum-g1` | `37ea371` | `engine/build/campaign_emergent_spectrum_2026-04-27` | `--L 64 --output-dir=engine/results/emergent_spectrum_2026-04-27_L64 --N-samples 5 --N-seeds 5` | `engine/results/emergent_spectrum_2026-04-27_L64/` | [`ANALYSIS_EMERGENT_SPECTRUM_G1.md`](archive/campaign_complete/ANALYSIS_EMERGENT_SPECTRUM_G1.md) |
| **FTD-0107** emergent-spectrum L=128 G2 | `preregister-emergent-spectrum-g2` | (this commit) | `engine/build_wsl/campaign_emergent_spectrum_2026-04-27` | `--L=128 --seeds=5 --samples=50 --burn=200 --stride=50 --output-dir=engine/results/emergent_spectrum_2026-04-28_L128/` | `engine/results/emergent_spectrum_2026-04-28_L128/` | [`PROTOCOL_EMERGENT_SPECTRUM_G2.md`](archive/campaign_complete/PROTOCOL_EMERGENT_SPECTRUM_G2.md) (analysis pending) |

The launcher script `engine/tools/run_emergent_spectrum_g1.sh` wraps
the FTD-0107 invocation; see `commit a0983ca` for the script body.

## Campaign 2 — dynamical time dilation

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0252** dynamical time dilation (v1) | `preregister-dynamical-time-dilation-v1` (owner-deferred — see pre-reg header) | (pending owner commit) | runner `engine/tests/campaign_time_dilation.cpp` SHA256 `ea29260b20dcbcadbaeec5e79125d099bad77313f346fe28df74454f07fff331`; analysis `scripts/exploration/analyze_time_dilation.py` SHA256 `323b2e7d4dce9a2a0211007a0bc39492f48acdba43813bdd93e99a655a513bad` | `--Llist=33,49,65,97,129` | `engine/results/time_dilation_2026-06-07/` | [`PREREG_DYNAMICAL_TIME_DILATION_v1.md`](../03_derivations/foundational_mechanics/PREREG_DYNAMICAL_TIME_DILATION_v1.md) → `ANALYSIS_DYNAMICAL_TIME_DILATION.md` (verdict OTHER) |
| **FTD-0252** v2 (IR limit) | `preregister-dynamical-time-dilation-v2` (owner-deferred) | (pending) | runner SHA256 `28c99f87f82b82bb25eea14be7e72ae4c422307e955840ac92c7dbd75b3b1140` (mode `--nperp-fixed=3`); analysis `scripts/exploration/analyze_time_dilation_v2.py` SHA256 `9a7559046f8bac01f5644a4f908f080ff220d6d7492a85cd90e90a72c5d9046c` | `--nperp-fixed=3 --Llist=33,65,97,129,193` | `engine/results/time_dilation_v2_2026-06-07/` | [`PREREG_DYNAMICAL_TIME_DILATION_v2.md`](../03_derivations/foundational_mechanics/PREREG_DYNAMICAL_TIME_DILATION_v2.md) → `ANALYSIS_DYNAMICAL_TIME_DILATION.md` §v2 (IR_CONFIRMED, scoped) |
| **FTD-0268** blind L=257 extension | `preregister-time-dilation-L257-blind-v1` | `ee8976b6` | runner unchanged from v2 (SHA256 `28c99f87…b1140`); predictor/scorer `scripts/exploration/predict_time_dilation_L257.py` SHA256 `d6d8799ff0981c2f5b49bcf29e63806e6d6e0d2209547d832f2adf7ec36b0816` | `--L=257 --nperp-fixed=3` | `engine/results/time_dilation_L257_blind_2026-06-11/` | [`PREREG_TIME_DILATION_L257_BLIND_v1.md`](../03_derivations/foundational_mechanics/PREREG_TIME_DILATION_L257_BLIND_v1.md) → [`ANALYSIS_TIME_DILATION_L257_BLIND_v1.md`](../03_derivations/foundational_mechanics/ANALYSIS_TIME_DILATION_L257_BLIND_v1.md) |

## FTD-0110 nonlinear bridge — quantitative N(A) law

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0110** N(A) law from substrate params | `preregister-ftd0110-na-law-v1` | (lock commit) | model `scripts/exploration/genesis_na_law_forward.py` SHA256 `ea17ccc294e87eac1fcd7d8b6ae9e7c6525b18167fbf0fa27ebd366850ff59b1`; adjudicator `scripts/exploration/analyze_na_law.py` SHA256 `867d99dfbef6187f945c7953b111e3c0f0d9b14dec1bdfbcfa39d369dadcd6c2`; engine instrument `engine/tests/campaign_genesis_geometry.cpp` SHA256 `7bda40a6e57c63e926e3b9183f3565093b96b1b570f32600b7103949b6b2cc36` | model `--sweep --L=32 --seeds=8 --gauss on --coupling on`; engine `--L=32 --A=14,30 --cpu` | `engine/results/genesis_geometry_2026-06-11/` + `scripts/exploration/results/na_law_2026-06-11/` | [`PREREG_FTD0110_NA_LAW_v1.md`](../03_derivations/foundational_mechanics/PREREG_FTD0110_NA_LAW_v1.md) → `ANALYSIS_FTD0110_NA_LAW.md` (post-run) |
| **FTD-0277** collective-coordinate genesis counting v1 | (local hash-lock; no clean git tag) | (pending owner commit) | model `scripts/exploration/genesis_counting_model.py` SHA256 `4fdaa1f9e9e32735fbab9d0ed9752b09bc6610a19e637c778595b397fc1d617b`; adjudicator `scripts/exploration/analyze_genesis_counting.py` SHA256 `7a4506022cf6927062b3d587a3c4082a5cda076ad3ea8c36bdf80ade96fd9a1b` | `python scripts/exploration/analyze_genesis_counting.py --out scripts/exploration/results/genesis_counting_v1/analysis.txt --json-out scripts/exploration/results/genesis_counting_v1/analysis.json` | `scripts/exploration/results/genesis_counting_v1/` | [`PREREG_GENESIS_COUNTING_v1.md`](preregistrations/PREREG_GENESIS_COUNTING_v1.md) → [`ANALYSIS_GENESIS_COUNTING_v1.md`](../03_derivations/archive/closed_negative/ANALYSIS_GENESIS_COUNTING_v1.md): `COUNTING_MODEL_V1_CLOSED_NEGATIVE` |

Platform: canonical post-optimization stack atop HEAD `761daa75` + uncommitted 8-color-SOR Gauss optimization (owner-declared canonical); physics-diff fingerprint SHA256 `961916b56569d1409984994121f51f3b897c02fe993ebf2ce0e2b03b3d07e381`. Genesis/flux field verified bit-reproducible (identical-seed firing geometry); golden-hash energy-audit non-determinism is a separate flagged regression that does not touch the genesis field. Three-outcome scheme: PROMOTE (framework-only model hits knee∈[14,18] + p_lo∈[3.3,4.1] + p_hi∈[1.6,2.1] + curve-RMS≤0.10 + shell-L1≤0.30) / BOUNDARY (drain or √α-coupling load-bearing) / FALSIFY. Priors PROMOTE 35 / BOUNDARY 45 / FALSIFY-UNDET 20. Supersedes the over-tagged `DERIV_FTD0110_GENESIS_THROTTLE.md`.

## Atomic sector hardening — replay/manifest package (FTD-0280; locked/run)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0280** atomic sector hardening | `preregister-atomic-sector-hardening-v1` | `fe55b42f` | `scripts/exploration/atomic_sector_hardening.py` SHA256 `0575bc9154f3760f1cd6049f24f3ce5bf18bed6eeb5d3a0bd0c9b04585fdc83d` | `--verify-locks --manifest`; locked replay `--replay-records --out-dir <dir>` | `scripts/exploration/results/atomic_sector_hardening_2026-06-13/` | [`ANALYSIS_ATOMIC_PARADIGM_CAMPAIGN_v1.md`](ANALYSIS_ATOMIC_PARADIGM_CAMPAIGN_v1.md): `ATOMIC-SECTOR-REPLAY-CONFIRMED` |

This campaign adds no physics claim. It verifies the FTD-0278/0279 script hashes,
prereg tags, manifest provenance, and replay verdicts. The underlying result class
remains `[CONDITIONAL -- DERIVED-GIVEN-IMPOSED-INPUT]`; FTD-0270, FC-1, FTD-0013,
and MC-T4.3 remain unchanged.

## Atomic next-three campaigns (FTD-0281/0282/0283; locked/run)

| FTD ID | Pre-reg tag | Commit | Script / engine artifact | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0281** DB-clock Coulomb spectroscopy | `preregister-db-clock-coulomb-spectroscopy-v1` | `fe55b42f` | engine hook: `db_clock_coulomb`; smoke test SHA256 `1be528966c02d21b739677e46735d39ad22e3c636d709b4254c49f5017b8fd28` | `ctest -R "db_clock_coulomb\|render_bridge_golden"` | `scripts/exploration/results/atomic_next_three_2026-06-13/` | [`ANALYSIS_ATOMIC_PARADIGM_CAMPAIGN_v1.md`](ANALYSIS_ATOMIC_PARADIGM_CAMPAIGN_v1.md): `DB-CLOCK-COULOMB-HOOK-CONFIRMED`; FFT verdict still downstream |
| **FTD-0282** exchange/correlation wall | `preregister-atomic-exchange-correlation-wall-v1` | `fe55b42f` | `scripts/exploration/atomic_next_three_campaigns.py` SHA256 `f7ef3f73427a90674d70695bbc875fb2de9984e77e691ea7160fd27c31925df8` | `--ftd-0282-wall-record` | `scripts/exploration/results/atomic_next_three_2026-06-13/` | [`ANALYSIS_ATOMIC_PARADIGM_CAMPAIGN_v1.md`](ANALYSIS_ATOMIC_PARADIGM_CAMPAIGN_v1.md): `EXCHANGE-CORRELATION-WALL-CONFIRMED` |
| **FTD-0283** no-new-knob ladder | `preregister-atomic-no-new-knob-ladder-v1` | `fe55b42f` | `scripts/exploration/atomic_next_three_campaigns.py` SHA256 `f7ef3f73427a90674d70695bbc875fb2de9984e77e691ea7160fd27c31925df8` | `--ftd-0283-ladder-record` | `scripts/exploration/results/atomic_next_three_2026-06-13/` | [`ANALYSIS_ATOMIC_PARADIGM_CAMPAIGN_v1.md`](ANALYSIS_ATOMIC_PARADIGM_CAMPAIGN_v1.md): `NO-NEW-KNOB-LADDER-NOT-CONFIRMED / Z2-SCALING-FAIL` |

These campaigns are deliberately split. FTD-0281 verifies live-engine semantics
before any FFT-spectrum claim. FTD-0282 is a negative-boundary test, not a fit.
FTD-0283 uses a fixed ion set and dimensionless scaling gates only.

## Alpha dynamical readout discriminator (locked; ⚠ id collision — booked under FTD-0384)

> **Id-collision note (2026-07-12 census, FTD-0384):** the LEDGER's FTD-0284 row is the **D=3
> forced-escape** (`preregister-alpha-d3-forced-escape-v1`), not this discriminator — so this
> discriminator has **no LEDGER row of its own** (the same disease as the known FTD-0243↔0189
> double-booking). The lock below is genuine (tag `preregister-alpha-dynamical-readout-v1` resolves);
> the discriminator is tracked under the FTD-0384 reconciliation and receives its own fresh id at
> next execution. Do not cite "FTD-0284" for this object.

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0284** alpha dynamical readout | `preregister-alpha-dynamical-readout-v1` | `1584fe95` | `scripts/proofs/proof_alpha_dynamical_readout_contract.py` SHA256 `5a4509ad24dc9be1354b31dfa5336eb573e0eaaafa2e8607fda489748f3af390` | `--verify-static --manifest`; future engine run must freeze its own artifact first | n/a for v1 static contract; future run output TBD | [`PREREG_ALPHA_DYNAMICAL_READOUT_v1.md`](preregistrations/PREREG_ALPHA_DYNAMICAL_READOUT_v1.md) (pre-reg only; no measurement result yet) |

This discriminator continues the alpha program after FTD-0242 and FTD-0244.
It does not search couplings. It freezes three distinct outcomes before any
future no-alpha-input engine measurement: native unit response (`NATIVE-NULL`),
external matching/Postulate-W, and a much stricter `DYNAMICAL-FOUND` branch.

## Alpha no-alpha engine probe (FTD-0285; locked/run invalidated)

| FTD ID | Pre-reg tag | Commit | Script / engine artifact | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0285** alpha no-alpha engine probe | `preregister-alpha-no-alpha-engine-probe-v1` | `cce615b0` | engine artifact `engine/tests/campaign_alpha_no_alpha_probe.cpp` SHA256 `883a917358077f626e90f5affacabd8e565f48fd1cc00aa775ac6e4c7ffbdade` | build target `campaign_alpha_no_alpha_probe`; run `ctest --test-dir engine/build -C Release -R "^alpha_no_alpha_probe$" --output-on-failure` | console/CTest output | [`ANALYSIS_ALPHA_NO_ALPHA_ENGINE_PROBE_v1.md`](ANALYSIS_ALPHA_NO_ALPHA_ENGINE_PROBE_v1.md): `INVALIDATED_PROTOCOL_OR_ENGINE_DRIFT` |

This probe is the first FTD-0284 engine artifact. Its native arm disables
known alpha leak paths (`coupling`, damping, force/Poisson/Lorentz hooks) and
tests whether the no-alpha Gauss projection yields unit geometric Coulomb or
the master-quadratic normalization. Its Postulate-W arm is a positive control,
not a derivation. The v1 run invalidated the absolute Phase-G gate for this
finite live-engine protocol; it did not produce a dynamical alpha result.

## Alpha estimator validation (FTD-0286; locked/run)

| FTD ID | Pre-reg tag | Commit | Script / engine artifact | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0286** alpha estimator validation | `preregister-alpha-estimator-validation-v1` | `7cf8bd5c` | engine artifact `engine/tests/campaign_alpha_estimator_validation.cpp` SHA256 `dce6018d4ccc7565c1bab6870c9a90647f1bc4290c0fed600cac0fd3883ee570` | build target `campaign_alpha_estimator_validation`; run `ctest --test-dir engine/build -C Release -R "^alpha_estimator_validation$" --output-on-failure` | console/CTest output | [`ANALYSIS_ALPHA_ESTIMATOR_VALIDATION_v1.md`](ANALYSIS_ALPHA_ESTIMATOR_VALIDATION_v1.md): `ENERGY_FUNCTIONAL_MISMATCH` |

This campaign is an instrument validation after FTD-0285. It compares the
production live-tick Gauss estimator with the existing matched-stencil EFT
projector on the same `L=32`, `r={5,7,9}` absolute Phase-G gate. The run
returned `ENERGY_FUNCTIONAL_MISMATCH`: matched projection converged, but the
field-energy observable still missed the analytic normalization. It does not
include a Postulate-W arm and cannot promote `x_+ = 1/alpha`.

## Alpha estimator validation v2 (FTD-0286 v2)

| FTD ID | Pre-reg tag | Commit | Script / engine artifact | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0286 v2** half-energy gate pairing | *(documentation lock — no git tag yet)* | *(uncommitted)* | engine artifact `engine/tests/campaign_alpha_estimator_validation_v2.cpp` SHA256 `9b6431c1f37f835969e38bf1de0f79d75625a75c86db0de6921a68323e6bdc74`; helper `engine/include/ftd/eft/lattice_coulomb_gate.h` | build target `campaign_alpha_estimator_validation_v2`; run `ctest --test-dir engine/build -C Release -R "^alpha_estimator_validation_v2$" --output-on-failure` | console/CTest output | [`ANALYSIS_ALPHA_ESTIMATOR_VALIDATION_v2.md`](ANALYSIS_ALPHA_ESTIMATOR_VALIDATION_v2.md): `HALF_ENERGY_GATE_CONFIRMED_MATCHED` |

v2 re-pairs `energy_audit().field_energy = ½Σ|J|²` with gate `α_r = r G_L(r)`.
Matched static projector passes (max rel err 0.26%); production live-tick still
fails (~12% systematic stencil drift). Resolves the v1 pairing error; does not
promote `x_+ = 1/alpha`.

## Thomson unlocked recoil (FTD-0288; locked/run)

| FTD ID | Pre-reg tag | Commit | Script / engine artifact | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0288** Thomson unlocked recoil | `preregister-thomson-unlocked-recoil-v1` | `7260b274` | engine artifact `engine/tests/campaign_thomson_unlocked_recoil.cpp` SHA256 `f43194598188bab303eecbdebcf99655118f90d2024279ed3a8a56607d864acc` | build target `campaign_thomson_unlocked_recoil`; run `ctest --test-dir engine/build -C Release -R "^thomson_unlocked_recoil$" --output-on-failure` | console/CTest output | [`ANALYSIS_THOMSON_UNLOCKED_RECOIL_v1.md`](ANALYSIS_THOMSON_UNLOCKED_RECOIL_v1.md): `NATIVE_EMERGENT_FLUX_GRADIENT_RECOIL_DETECTED` |

This campaign follows FTD-0287. It unlocks the charge and separates native
legacy force, native emergent flux-gradient force, and an explicitly imposed
diagnostic qE hook. The native emergent arm recoils deterministically; legacy
does not. The diagnostic qE arm responds transversely but remains imposed. No
alpha, Thomson cross-section, or QED amplitude claim is promoted.

## Thomson flux-excess discriminator (FTD-0289; locked/run)
| FTD ID | Pre-reg tag | Commit | Script / engine artifact | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0289** Thomson flux-excess discriminator | `preregister-thomson-flux-excess-v1` | `acb4005a` | engine artifact `engine/tests/campaign_thomson_flux_excess.cpp` SHA256 `1f562ac9e9e0f3fdeb72bce00fda2c00f70117271439ef27d418d05c29ec7589` | build target `campaign_thomson_flux_excess`; run `ctest --test-dir engine/build -C Release -R "^thomson_flux_excess$" --output-on-failure` | console/CTest output | [`ANALYSIS_THOMSON_FLUX_EXCESS_v1.md`](ANALYSIS_THOMSON_FLUX_EXCESS_v1.md): `NATIVE_EMERGENT_EXCESS_FLUX_DEFLECTION_DETECTED` |
This campaign follows FTD-0288. It subtracts the free propagating wave and the
charge-only field from the charge-plus-beam run. Locked and legacy residuals
stay at machine noise; the native emergent flux-gradient path leaves an
above-gate residual. The frozen transverse-centroid subtype does not fire. No
alpha, Thomson cross-section, or QED amplitude claim is promoted.
## Thomson radiation shell meter (FTD-0290; locked/run)
| **FTD-0290** Thomson radiation shell meter | `preregister-thomson-radiation-shells-v1` | `8ccfee7b` | engine artifact `engine/tests/campaign_thomson_radiation_shells.cpp` SHA256 `a47de9c1bb52f92a6dc35471f4eba516fb76acaf9b66abd3de44dd6431d67edf` | build target `campaign_thomson_radiation_shells`; run `ctest --test-dir engine/build -C Release -R "^thomson_radiation_shells$" --output-on-failure` | console/CTest output | [`ANALYSIS_THOMSON_RADIATION_SHELLS_v1.md`](ANALYSIS_THOMSON_RADIATION_SHELLS_v1.md): `NO_BASELINE_SUBTRACTED_OUTWARD_POWER` |
This campaign follows FTD-0289. It computes `S_res = (-W_res) × curl(J_res)`
from the baseline-subtracted residual field and sums outward radial Poynting
power on fixed shells `{5,7,9,11,13,15}`. Locked and legacy shell powers are
machine-zero; the native emergent trace remains below the frozen gate. No
radiation, Thomson cross-section, QED amplitude, or alpha claim is promoted.
## Thomson native finite-volume continuity meter (FTD-0291; locked/run)
| **FTD-0291** Thomson native finite-volume continuity meter | `preregister-thomson-native-continuity-v1` | `47ccbee4` | engine artifact `engine/tests/campaign_thomson_native_continuity.cpp` SHA256 `357a2a2b4bd7fb8d8604a4c30490f68ab9a404e8574ed6e55b034056a5b3f3e8` | build target `campaign_thomson_native_continuity`; run `ctest --test-dir engine/build -C Release -R "^thomson_native_continuity$" --output-on-failure` | console/CTest output | [`ANALYSIS_THOMSON_NATIVE_CONTINUITY_v1.md`](ANALYSIS_THOMSON_NATIVE_CONTINUITY_v1.md): `NATIVE_GRAPH_CONTINUITY_CANDIDATE_INVALIDATED` |
This campaign follows FTD-0290. It replaces the borrowed Poynting shell meter
with an 18-neighbor graph-energy finite-volume balance candidate. Repeat and
locked-linear residual controls pass, but the candidate current fails the
free-wave balance gate, so no native radiation, source, Thomson cross-section,
QED amplitude, or alpha claim is promoted.
## Source-free discrete tick energy invariant v1 (FTD-0292; locked/run)
| **FTD-0292** Source-free discrete tick energy invariant v1 | `preregister-thomson-tick-invariant-v1` | `87f0cda2` | engine artifact `engine/tests/campaign_thomson_tick_invariant.cpp` SHA256 `5e6e2b77796d8a91f02bc7b2a85c9c862dd1f4e91b832be19ae5d5b41c455e16` | build target `campaign_thomson_tick_invariant`; run `ctest --test-dir engine/build -C Release -R "^thomson_tick_invariant$" --output-on-failure` | console/CTest output | [`ANALYSIS_THOMSON_TICK_INVARIANT_v1.md`](ANALYSIS_THOMSON_TICK_INVARIANT_v1.md): `DISCRETE_TICK_INVARIANT_INVALIDATED` |
This campaign follows FTD-0291. It tests the modified energy of the source-free
tick with ordinary double accumulation. The modified energy stayed much tighter
than naive energy but missed the frozen relative gate; v1 is a numeric-gate
invalidation, not a promoted theorem.
## Source-free discrete tick energy invariant v2 (FTD-0293; locked/run)
| **FTD-0293** Source-free discrete tick energy invariant v2 | `preregister-thomson-tick-invariant-v2` | `83863d5e` | engine artifact `engine/tests/campaign_thomson_tick_invariant_v2.cpp` SHA256 `c362d35e1a2c61216982bb7ae2c8cf4ee916e59f1e3bcc77a62cee993caa8b5f` | build target `campaign_thomson_tick_invariant_v2`; run `ctest --test-dir engine/build -C Release -R "^thomson_tick_invariant_v2$" --output-on-failure` | console/CTest output | [`ANALYSIS_THOMSON_TICK_INVARIANT_v2.md`](ANALYSIS_THOMSON_TICK_INVARIANT_v2.md): `DISCRETE_TICK_MODIFIED_ENERGY_CONFIRMED` |
This campaign keeps the v1 update, initial condition, invariant formula, and
gates, but measures with long-double Kahan accumulation. It confirms the
source-free modified tick energy while the naive continuum energy visibly
drifts. No radiation, Thomson cross-section, QED amplitude, or alpha claim is
promoted.
## Source-free discrete tick local continuity v1 (FTD-0294; locked/run)
| **FTD-0294** Source-free discrete tick local continuity v1 | `preregister-thomson-tick-local-continuity-v1` | `7ebc236e` | engine artifact `engine/tests/campaign_thomson_tick_local_continuity.cpp` SHA256 `6b137c83016b9aefb10d47d22df0094487ab761c06e167870a209004ada99aa3` | build target `campaign_thomson_tick_local_continuity`; run `ctest --test-dir engine/build -C Release -R "^thomson_tick_local_continuity$" --output-on-failure` | console/CTest output | [`ANALYSIS_THOMSON_TICK_LOCAL_CONTINUITY_v1.md`](ANALYSIS_THOMSON_TICK_LOCAL_CONTINUITY_v1.md): `SOURCE_FREE_LOCAL_TICK_CONTINUITY_INVALIDATED` |
This campaign tests the exact source-free local tick density/current. Absolute
balance closes at roundoff, but the exchange-relative denominator is
degenerate on quiet exchanges, so v1 is not promoted.
## Source-free discrete tick local continuity v2 (FTD-0295; locked/run)
| **FTD-0295** Source-free discrete tick local continuity v2 | `preregister-thomson-tick-local-continuity-v2` | `1d4a29a5` | engine artifact `engine/tests/campaign_thomson_tick_local_continuity_v2.cpp` SHA256 `9b48ca418e784ba98e35708563214b22c78cf2580f880fda9fa923cef4c7a804` | build target `campaign_thomson_tick_local_continuity_v2`; run `ctest --test-dir engine/build -C Release -R "^thomson_tick_local_continuity_v2$" --output-on-failure` | console/CTest output | [`ANALYSIS_THOMSON_TICK_LOCAL_CONTINUITY_v2.md`](ANALYSIS_THOMSON_TICK_LOCAL_CONTINUITY_v2.md): `SOURCE_FREE_LOCAL_TICK_CONTINUITY_CONFIRMED` |
This campaign keeps the v1 density/current unchanged and gates the relative
residual against finite-volume energy scale. It confirms source-free local
tick continuity. Coupled source/work terms remain the next open target.
## Fixed-charge coupled tick source/work continuity (FTD-0296; locked/run)
| **FTD-0296** Fixed-charge coupled tick source/work continuity | `preregister-thomson-coupled-source-work-v1` | `5d88062e` | engine artifact `engine/tests/campaign_thomson_coupled_source_work.cpp` SHA256 `95747a57895973577e0054d075752b79e74173507097652e31498b125d7ec88e` | build target `campaign_thomson_coupled_source_work`; run `ctest --test-dir engine/build -C Release -R "^thomson_coupled_source_work$" --output-on-failure` | console/CTest output | [`ANALYSIS_THOMSON_COUPLED_SOURCE_WORK_v1.md`](ANALYSIS_THOMSON_COUPLED_SOURCE_WORK_v1.md): `FIXED_CHARGE_SOURCE_WORK_CONTINUITY_CONFIRMED` |
This campaign adds the engine's additive state-flux source term with one
locked charge and movement off. The native source/work term closes the
finite-volume balance at roundoff. Moving-source recoil accounting remains the
next open target.
## Thomson moving-recoil source/work accounting (FTD-0297; locked/run)
| **FTD-0297** Thomson moving-recoil source/work accounting | `preregister-thomson-moving-recoil-accounting-v1` | `0ba544f5` | engine artifact `engine/tests/campaign_thomson_moving_recoil_accounting.cpp` SHA256 `aae604ea897943102273f89b819735283804474d26a2a531f769835dc46f5c89` | build target `campaign_thomson_moving_recoil_accounting`; run `ctest --test-dir engine/build -C Release -R "^thomson_moving_recoil_accounting$" --output-on-failure` | console/CTest output | [`ANALYSIS_THOMSON_MOVING_RECOIL_ACCOUNTING_v1.md`](ANALYSIS_THOMSON_MOVING_RECOIL_ACCOUNTING_v1.md): `SUBVOXEL_RECOIL_ACCOUNTED_BY_ADDITIVE_SOURCE_WORK` |
This campaign unlocks the charge in native legacy and native emergent modes.
The emergent mode recoils deterministically, but the fixed 200-tick protocol
has zero integer transport events, so the additive source/work balance still
closes at roundoff. Integer transport work remains open.
## Halo-exponent forcedness audit (FTD-0300; locked/run)
| **FTD-0300** halo-exponent forcedness | `preregister-halo-forcedness-v1` | `168148e0` | engine `engine/tests/campaign_halo_forcedness.cpp` SHA256 `84f7c407bbdc3bd8e9530235f828dec68c90a9b48f3a39896635609ae92b188e`; analyzer `scripts/exploration/analyze_halo_forcedness.py` SHA256 `44f09ac4d01b3be40359266bf56bb77526524499e85d165316d60762c8c5ad76`; wrapper `scripts/exploration/run_halo_constant_sweeps.py` SHA256 `384bd0481dd332c3e339f4eed8dcbba1be392b85e7d8076a899d8f89abeee896` | GPU `engine/build_wsl`; `--arm=det --Ls=64,96,128,160 --selective=on,off --toggles=minimal --ticks=1500` | `engine/results/halo_forcedness/halo_forcedness_v1.csv` | [`ANALYSIS_HALO_FORCEDNESS_v1.md`](ANALYSIS_HALO_FORCEDNESS_v1.md): `INDETERMINATE (frozen); SPARC boundary` |
Gate (Step 1) of the dark-matter / SPARC rotation-curve program. The lossless dark-matter
halo (`selective_damping = ON`, §4.2) **box-fills** the periodic lattice (`r_eff ≈ L/2`)
yet its windowed exponent **converges to −1.25** (the doc's −0.69 is an L=64 transient,
**falsified**) → R1 **INDETERMINATE** (box-fill ⇒ not localized-forced; convergent ⇒ not
drift-tuned). The damped Coulomb near-field (`selective OFF`) is the only forced, localized
self-field (−2.15, localized, R0 PASS). **SPARC not founded** (box-fill ⇒ no localized
scale). Golden `0x56fa28acb5b9fe88` green; observation-only; zero promotions.
## Proton-stability forcedness audit (FTD-0301; locked/run)
| **FTD-0301** proton-stability forcedness | `preregister-proton-stability-v1` | `bb99a20d` | engine `engine/tests/campaign_proton_stability.cpp` SHA256 `56fe09548e98787e66b988161b4e57e66f42aa235c8ac4ea7930c8009058bd48`; analyzer `scripts/exploration/analyze_proton_stability.py` SHA256 `eb076a16c4033cd869ac4bdfd08e49862d3a3a01a4b13e89e41229529786a8e0` | CPU `engine/build/Release`; cold `--heat=none --radius=1,2 --genesis=on --seeds=16 --ticks=2000`; heated `--heat=inject --genesis=off` and `--heat=langevin --dual=off --heat-T=0.3,0.8` | `engine/results/proton_stability/proton_stability_ror_*.csv` | [`ANALYSIS_PROTON_STABILITY_v1.md`](ANALYSIS_PROTON_STABILITY_v1.md): `UNFORCED-METASTABLE [BOUNDARY] (frozen)` |
The "micro" pier candidate after FTD-0300. `proof_complete_sm.py:460–471` tags `τ_p = ∞`
`[THEOREM]`; this audit finds it **unforced**. The triad lock locks only same-sign triples
(`transmutation_phases.cpp:148,153`) ⇒ cannot fire on the mixed-sign proton (`uud`); the
proton is metastable, never lock-protected (`max locked = 0`), evaporates spontaneously
(12.5% r1 / 43.75% r2), and FTD's own weak channel transmutes it (`uud→uuu`, **46/48**
heated weak=on, control 0/48) while violating its only exact charge Σs (**47/64** decays).
Frozen verdict **UNFORCED-METASTABLE [BOUNDARY]**; corrects the `proof_complete_sm.py`
`τ_p = ∞` tag `[THEOREM]` → `[SELECTION]`. Golden `0x56fa28acb5b9fe88` green;
observation-only; zero other promotions.

## Lattice wave sectors — dispersion atlas + condensate-compression probe (FTD-0299; locked/run)

| FTD ID | Pre-reg tag | Commit | Script / engine artifact | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0299** wave sectors | `preregister-wave-sectors-v1` | `8fff0187` | engine artifact `engine/tests/campaign_wave_sectors.cpp` SHA256 `e25396b8c6552d4bf7e03436b169d40e991de908a26330cca046eb3f5e92dd30`; analyzer `scripts/exploration/analyze_wave_sectors.py` SHA256 `b76869fee3046aa134221abd0972da5ba339b8a2f7c059d0209801696ee75936` | build target `campaign_wave_sectors`; light `--arm=light --L={24,32,48}`; sound `OMP_NUM_THREADS=1 --arm=sound --L=24 --seeds=4 --nmodes=5 --ticks=256 --equil=200 --kick=0.05 --Tcond=0.5` | `engine/results/wave_sectors/` (local) | [`ANALYSIS_WAVE_SECTORS_v1.md`](../03_derivations/foundational_mechanics/ANALYSIS_WAVE_SECTORS_v1.md): `LIGHT=LIGHT-CONFIRMED  SOUND=NULL` |

Executes the FTD-0298-SOUND `[OPEN]`. Q1 LIGHT-CONFIRMED (ω matches the 18-pt stencil to machine zero across ⟨100⟩/⟨110⟩/⟨111⟩; isotropic c=1/√3). Q2 NULL (no propagating compression branch — FTD-0298 boundary engine-confirmed). Hardened pre-lock by a 24-blocker adversarial review. Golden-neutral; no promotions.

## FQCR (Finite Quarter-Conjugacy Recurrence) Model IV uniqueness scan (scan queued)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0143** FQCR (4,6;3,2) uniqueness scan | `preregister-fqcr-quotient-uniqueness-v1` | `557593e` | `tools/scan_fqcr_quotient_uniqueness.py` — SHA256 `719015e253037a8d699d4aa95d524625f5d6fd08a5ec04ff25715919b28903da` (registered at runtime 2026-07-12 per FTD-0097 precedent; imports TARGETS/TOLERANCES from `tools/scan_look_elsewhere.py` to preclude copy-drift) | (k, d, ℓ, m) ∈ {2,...,8}^4; tolerances {1e-3, 1e-4, 1e-5, 1e-6}; targets = 20 FTD-0097 spine targets | `engine/results/fqcr_quotient_uniqueness_2026-05-06_l_scan/` | `PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md` (pre-reg) → **EXECUTED 2026-07-12, verdict Outcome B [CLOSED NEGATIVE — uniqueness rejected]** (`reports_and_audits/ANALYSIS_FQCR_QUOTIENT_UNIQUENESS.md`; mechanical criterion-split C disclosed and adjudicated therein) |

Pre-reg SHA256: `94bc4cd74cbf90017996bf90a19f0bbeaae7937f8c47a6317b3409f58c268a1f`.

Backend: pure Python via mpmath (no engine GPU required). Scan execution ~1-2 hours wall on a single CPU core.

When launching: confirm `git rev-list -n1 preregister-fqcr-quotient-uniqueness-v1` resolves to `557593e` and that the scan-runner's content hash is recorded against this anchor at runtime per FTD-0097's precedent.

## Alpha arithmetic generativity Test 4 (candidate inventory queued)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0185** alpha arithmetic generativity | `preregister-alpha-arithmetic-generativity-v1` | (pending commit/tag) | none; desk-audit target declaration gate | No numerical search. Candidate must publish target declaration before comparison; `x_- ≈ N_c` excluded as the prize | n/a until a candidate declaration exists | `PREREG_ALPHA_ARITHMETIC_GENERATIVITY_v1.md` → candidate declaration or no-candidate report |

Pre-reg SHA256: `b222c2a0873fa21dcf28b87111ecab5de8753ec3a4a38e3074d038b6f3d06a27`. This pre-registration locks the rules for Test 4, not a measurement script.

## Derive-QM / epistemic arc — desk pre-regs (closure attempts complete)

Desk pre-registrations (in-session SHA256 lock recorded **before** each analysis; no engine GPU; commit deferred per owner, integrated this commit). Per the FTD-0224 alpha-readout precedent, the lock is the pre-reg file's SHA256 recorded in-session, not a `preregister-*` git tag anchored before a separate engine run.

| FTD ID | Pre-reg doc (`10_eft_program/`) | In-session SHA256 | Verifier (passes) | Verdict |
|---|---|---|---|---|
| **FTD-0225** Route B modular-time algebra type (B1) | `PREREG_MODULAR_TIME_ALGEBRA_TYPE_v1.md` | `f8a3e960c400863677e631abba898e13d73ef64023e9da9ea51fe088b63606e5` | `scripts/proofs/proof_modular_time_algebra_type.py` (4/4) | CLOSED-NEGATIVE (type I) |
| **FTD-0226** manifestation non-commutativity (B-QM-1) | `PREREG_MANIFESTATION_NONCOMMUTATIVITY_v1.md` | `fefcd6ad26320ed4f2b3e8a46144080894c3eceb07bf90378295cd3a3386d91b` | `scripts/proofs/proof_manifestation_noncommutativity.py` (5/5) | CLOSED-NEGATIVE (Boolean) |
| **FTD-0227** Spekkens knowledge-balance (B-QM-1′) | `PREREG_SPEKKENS_KNOWLEDGE_BALANCE_v1.md` | `79e3b7f8c4a7e4aff5887c0cd130c45f5477778400c1da4db1cd51fcdc49f2dc` | `scripts/proofs/proof_spekkens_knowledge_balance.py` (10/10) | PARTIAL (binding derived) |
| **FTD-0228** symplectic budget symmetry (B-QM-1″) | `PREREG_SYMPLECTIC_BUDGET_SYMMETRY_v1.md` | `dd8a8fa065ae2800d7554a2c82938137d340e0825e37a3362ffc1f22951a0f20` | `scripts/proofs/proof_symplectic_budget_symmetry.py` (5/5) | CLOSED-NEGATIVE (apophenia) |

Companion scopes: `SCOPE_ROUTE_B_MODULAR_TIME.md`, `SCOPE_DERIVE_QM_GAP.md`. Verdict docs: the matching `AUDIT_*` files. No spine claim promoted or demoted (`x₊=1/α` FTD-0013 unchanged).

## R3a operator-mixing L-scan (campaign queued)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0140** R3a operator-mixing L-scan | `preregister-operator-mixing-l-scan-v1` | `f3fa700` | `engine/build_wsl/campaign_operator_mixing_2026-04-26` | `--L <64\|96\|128> --b <2\|4> --inj-mult 1.0` (6 configs total) | `engine/results/operator_mixing_2026-05-05_l_scan/L<L>_b<b>/` | [`PREREG_OPERATOR_MIXING_L_SCAN_v1.md`](archive/campaign_complete/PREREG_OPERATOR_MIXING_L_SCAN_v1.md) (pre-reg) → `ANALYSIS_OPERATOR_MIXING_L_SCAN.md` (post-launch) |

Pre-reg SHA256: `290005066803b2cada8be9820c50f35ef3f810ae61fba53d436d9a393a5c2f0d`.

Backend anchor: HEAD `00f41fe` post BH-F5/F8/F9 RNG portability closure (commits `c1a4f88` + `c8e03a5`). Per-voxel CPUGPU bit-exact at unit mass under stochastic toggles. The campaign is pre-registered now and launches when GPU is clear.

When launching: confirm `git rev-list -n1 preregister-operator-mixing-l-scan-v1` resolves to `f3fa700` and that the campaign binary's commit-sha matches that anchor.

## Earlier campaigns (pre-2026-04-27, no pre-reg tag yet)

These campaigns precede the pre-registration discipline (introduced
2026-04-27) and don't have `preregister-*` tags. Their analysis
documents still cite specific commit ranges + result directories;
manually trace via `git log --follow` if reproducing.

| FTD ID | Date | Output dir | Analysis doc |
|---|---|---|---|
| FTD-0098–0102 operator-mixing baseline | 2026-04-26 | `engine/results/operator_mixing_2026-04-26/` | LEDGER rows |
| FTD-0103 continuum-limit | 2026-04-26 | `engine/results/baseline_2026-04-26/` (campaign_continuum subset) | LEDGER row FTD-0103 |
| FTD-0104 topology atlas | 2026-04-26 | `engine/results/baseline_2026-04-26/` (campaign_topology subset) | LEDGER row FTD-0104 |
| FTD-0093 Mechanism C closure | 2026-04-27 | `engine/results/baseline_2026-04-26/bcc_band_spectrum/` | [`AUDIT_LINK8_CLOSURE.md`](archive/closed_negative/AUDIT_LINK8_CLOSURE.md) cross-ref |

---

## Structural / dynamical discriminator -- boundary theorem Stage 1 (v1, v2 close-positive)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0186** structural/dynamical discriminator (v1, historical — archived to `archive/superseded/`, preserved as provenance) | `preregister-structural-dynamical-discriminator-v1` | `75ebe56` | `scripts/proofs/proof_structural_dynamical_partition.py` | desk classification of the LEDGER record; no numerical search | n/a (classification is a theory doc) | `PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v1.md` (pre-reg) -> `FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md` (Stage-1 result; v1 falsifier A1 fired -- see §5) |
| **FTD-0186** structural/dynamical discriminator (v2, current) | `preregister-structural-dynamical-discriminator-v2` | `d550bca` | `scripts/proofs/proof_structural_dynamical_partition.py` (script encodes v2-style expectations per its header; same code as v1, re-applied against v2 wording -- no script edit required) | desk classification of the decisive set; no numerical search | n/a (classification is a theory doc) | `PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v2.md` (pre-reg, supersedes v1's falsifier wording) -> `FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md` §5.2 (v2 result: Outcome A -- clean partition, A1 v2 PASS / A2 PASS / A3 PASS) |

Pre-reg v1 SHA256: `a6562dca56154401e7a2cfb8785266cef0d5b4ee70d3755797762ddffa3e538d`.
Pre-reg v2 SHA256: `a233fa28be54c63c6a7ebae26c6b54e129c9f2120e535f92d85999ac84d9068a`.

When auditing: confirm `git rev-list -n1 preregister-structural-dynamical-discriminator-v1` resolves to `75ebe56` and `git rev-list -n1 preregister-structural-dynamical-discriminator-v2` resolves to `d550bca`. The discriminator definition (pre-reg §2) was locked under v1 and **carried over verbatim** into v2; the v1 falsifier (§4) fired on its own pre-registered wording -- v2 sharpens A1 to "failed attempt to derive a non-universal *dynamical value*" (rather than v1's broader "failed derivation attempt") and adds A3 to record structural-provenance closed-negatives as a separate honest category. The v2 re-run (`python scripts/proofs/proof_structural_dynamical_partition.py`) returns clean partition: 12 spine theorems all STRUCTURAL; 13 type-i closed-negatives all NON-UNIVERSAL DYNAMICAL / CALIBRATION-CONDITIONAL; 3 type-ii closed-negatives all STRUCTURAL targets (structural-provenance, outside the boundary-theorem axis). LEDGER FTD-0186 status updated from `[DEFINITION] + [OPEN]` to `[DEFINITION] + [STAGE 1 CLOSED POSITIVE per v2]`. **Honest framing per v2 §1:** v2 is a scope clarification, not a "win"; v2's falsifier is partly engineered to produce Outcome A; the discipline-bearing test is whether Stage 2 produces a provable proposition with stated axioms, independently of v2's outcome. **No FTD claim promoted or demoted.** Both v1 and v2 rows are preserved -- v1 as historical provenance, v2 as the current locked falsifier.

---

## Finite neutral lock -- finite-closure SM-shadow audit (Q10)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0190** finite neutral lock (Q10) | `preregister-finite-neutral-lock-v1` | tag `preregister-finite-neutral-lock-v1` | [`audit_finite_neutral_lock.py`](../../../scripts/proofs/audit_finite_neutral_lock.py) -- frozen-catalog enumeration (pre-reg §4); no numerical search, no near-miss scan | n/a | n/a (desk audit) | [`PREREG_FINITE_NEUTRAL_LOCK_v1.md`](../08_structural/PREREG_FINITE_NEUTRAL_LOCK_v1.md) (pre-reg) -> [`AUDIT_FINITE_NEUTRAL_LOCK.md`](../08_structural/AUDIT_FINITE_NEUTRAL_LOCK.md) (result: UNDERDETERMINED) |

Pre-reg SHA256: `41c3f86584270d59fd25736bfec3cee3efb6a656d34f12be44b93272e57ae346`.

When auditing: confirm `git rev-list -n1 preregister-finite-neutral-lock-v1` resolves to the commit that introduced `PREREG_FINITE_NEUTRAL_LOCK_v1.md`, and that the file's SHA256 still matches the value above (`sha256sum docs/theory/08_structural/PREREG_FINITE_NEUTRAL_LOCK_v1.md`). The question Q10, definitions D1-D6, the FROZEN admissible search space (pre-reg §4), the (1,2)_{1/2} benchmark (§5), the three pre-blessed outcomes (§6), and the falsifier F-a..F-e (§7) were all locked before the audit was run. The pre-reg doc lives in `08_structural/` (the structural cluster), not in `10_eft_program/`.

---

## Colour-singlet rank -- electroweak-rank audit (Q11)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0191** colour-singlet rank (Q11) | `preregister-colour-singlet-rank-v1` | tag `preregister-colour-singlet-rank-v1` | [`audit_colour_singlet_rank.py`](../../../scripts/proofs/audit_colour_singlet_rank.py) -- frozen-catalog enumeration (pre-reg §4 = Q10 §4); no numerical search | n/a | n/a (desk audit) | [`PREREG_COLOUR_SINGLET_RANK_v1.md`](../08_structural/PREREG_COLOUR_SINGLET_RANK_v1.md) (pre-reg) -> [`AUDIT_COLOUR_SINGLET_RANK.md`](../08_structural/AUDIT_COLOUR_SINGLET_RANK.md) (result: UNDERDETERMINED) |

Pre-reg SHA256: `08c55b8e060332a2311be7ae6dedf5d48cbf1af861db627195d1dd2f8a886dbe`.

When auditing: confirm `git rev-list -n1 preregister-colour-singlet-rank-v1` resolves to the commit that introduced `PREREG_COLOUR_SINGLET_RANK_v1.md`, and that the file's SHA256 still matches the value above. Q11 is the successor to Q10 (FTD-0190): its verdict decides whether FTD-0190 lifts to FOUND, stays UNDERDETERMINED, or closes negative. The question, definitions D1-D6, the frozen catalog (§4), the benchmark (§5), the three outcomes (§6), and the falsifier F-a..F-f (§7) were all locked before the audit was run.

---

## Weak-SU(2) provenance audit (Q12)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0192** weak-SU(2) provenance (Q12) | `preregister-weak-su2-provenance-v1` | tag `preregister-weak-su2-provenance-v1` | desk provenance audit of an existing derivation (`DERIV_LATTICE_SU2_WEAK.md`); step-by-step epistemic classification, no numerical search | n/a | n/a (desk audit) | [`PREREG_WEAK_SU2_PROVENANCE_v1.md`](../08_structural/PREREG_WEAK_SU2_PROVENANCE_v1.md) (pre-reg) -> [`AUDIT_WEAK_SU2_PROVENANCE.md`](../08_structural/AUDIT_WEAK_SU2_PROVENANCE.md) (result: COUNT-MATCH) |

Pre-reg SHA256: `25ee75f4cf472841bf79a2c14495728731b2b2c27f5395ab28f3b30ea2c61784`.

When auditing: confirm `git rev-list -n1 preregister-weak-su2-provenance-v1` resolves to the commit that introduced `PREREG_WEAK_SU2_PROVENANCE_v1.md`, and that the file's SHA256 still matches the value above. Q12 is the terminating step of the Q10 -> Q11 -> Q12 chain: its verdict decides whether FTD-0190 and FTD-0191 lift to FOUND (GENUINE), close negative (COUNT-MATCH), or stay UNDERDETERMINED with the gap pinned to one step (PARTIAL). The audit reads the frozen target documents as they exist at the lock commit. The question, definitions D1-D5, the genuine-derivation benchmark (§4), the three outcomes (§6), and the falsifier F-a..F-e (§7) were all locked before the audit was run.

---

## Alpha-readout ARC-B1 observable-selection -- MC-T4.3 closure attempt design

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0198** alpha-readout ARC-B1 observable-selection | `preregister-alpha-readout-observable-selection-v1` | `0e79820` | desk derivation (no script in this commit); engine measurements where finite-L stability or transfer-operator spectra need numerical confirmation will be instrumented once a candidate ARC tuple identifies the measurement need | n/a (desk) until engine measurement specified | `engine/results/alpha_readout_observable_selection_YYYY-MM-DD/` once instrumented | [`PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md`](preregistrations/PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md) (pre-reg, design only) -> `FOUND_ALPHA_READOUT_OBSERVABLE_SELECTION.md` / `AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION.md` / `AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION_CLOSED_NEGATIVE.md` (post-attempt, per §6 verdict) |

Pre-reg SHA256: `e273ca85234c04406c14b0b0bb01bb2ea760367ca7286c2b35649b80563b582a`.

When auditing: confirm `git rev-list -n1 preregister-alpha-readout-observable-selection-v1` resolves to commit `0e79820` (the commit that introduced `PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md`), and that the file's SHA256 still matches the value above. This pre-registration locks the design of the first attempt against MC-T4.3 (the Priority-0 central foundational obstruction per `SPEC_DOCTRINE_LEDGER.md` v1.4 §14 Phase 2). The closure attempt itself is a downstream multi-session arc; this manifest entry records the design lock, not a measurement. The question (§2), definitions D1-D6 (§3), the FROZEN admissible observable catalog (§4 -- non-site-local FTD-native observables only: state field, flux field + dual substrate, bilinear link observables, plaquette bivectors, Wilson-loop traces, boundary-to-boundary transfer observables, reference frame projections, with the FQCR Model V `T_O` and master quadratic + coefficient 16 as targets-not-inputs), the MC-T4.3 contract benchmark (§5 = `SPEC_ALPHA_READOUT_CONTRACT.md` §1 verbatim) and ARC-0..ARC-3 status levels, the three pre-blessed outcomes (§6 -- FOUND / UNDERDETERMINED / CLOSED-NEGATIVE), the falsifier F-a..F-j (§7), the banned moves (§8), and the locked 11-step method (§9) with numerical comparison only at step 10 after admissibility gate + falsifier checklist + banned-moves checklist were all locked before the closure attempt was run. **Prior-favoured outcome: CLOSED-NEGATIVE** (11 closed-negative alpha-derivation routes precede; the value of the pre-reg is in making whichever verdict lands rigorous and providing load-bearing input to Path II FTD-0186 v2 boundary theorem if it closes negative). No closure attempt in this commit -- design lock only. Companion docs cross-referenced in pre-reg header. **Closure attempts have now landed** (2026-05-23 Sessions C1 + C3 + C4): plaquette bivectors (catalog item 4, FTD-0204, commit `01d171d`, [CLOSED NEGATIVE] per §6 (c)); boundary-to-boundary transfer + reference frame projections + synthesis across {4, 6, 7} (FTD-0205, commit `6e7b77a`, [CLOSED NEGATIVE -- ARC-B1 primary catalog items] per §6 (c)). The v1 pre-reg wording proved correct (no v2 required); no falsifier fires; no banned move invoked; all three primary routes close negative at §9 step 5 by the same categorical structural mismatch (FTD-native lattice substrate arithmetic vs lemniscatic-curve / ℤ[i]-module / Chowla-Selberg arithmetic). Catalog-item variants and ARC-A / ARC-C / ARC-D remain open.

---

## Catalan algebraic-independence frontier-documentation (Conjecture 19.2)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0206** Catalan algebraic-independence | `preregister-catalan-independence-v1` | `e861198` | (no closure attempt run; this PREREG is frontier-documentation, not a measurement script. The PSLQ baseline at 80 digits is documented in `scripts/verification/verify_gstar_paper.py` + `scripts/verification/investigate_p2_cubic_agm.py`) | n/a (desk; PSLQ-baseline already in the committed verify_gstar_paper.py corpus at commit `e6a6553`) | n/a (no campaign output) | [`PREREG_CATALAN_INDEPENDENCE_v1.md`](../09_mathematical/general_math/PREREG_CATALAN_INDEPENDENCE_v1.md) (PREREG, frontier-documentation) -> successor docs `FOUND_CATALAN_INDEPENDENCE.md` (if positive closure) / `AUDIT_CATALAN_INDEPENDENCE_CLOSED_NEGATIVE.md` (if falsified) -- default expectation: none of these landing in FTD's reach |

Pre-reg SHA256: `e5415458ac4002430576615a41b16f4b71d6cbd42ae647b5c67989c847ce5dd1`.

When auditing: confirm `git rev-list -n1 preregister-catalan-independence-v1` resolves to commit `e861198` (the commit that introduced `PREREG_CATALAN_INDEPENDENCE_v1.md`), and that the file's SHA256 still matches the value above. This pre-registration is **frontier documentation**, not a closure attempt. It locks (a) the conjecture statement (Catalan G algebraically independent of {G\*, π} over Q-bar, three equivalent formulations), (b) the current PSLQ-baseline evidence (80 digits, basis `{1, G_Catalan, G_G^k π^ℓ, G_Catalan · G_G^k π^ℓ}` for |k|, |ℓ| ≤ 8, no integer relation at coefficient bound 10^12; reproducible from `scripts/verification/verify_gstar_paper.py`), (c) the Beilinson-Deligne structural motivation (non-critical L-values conjecturally outside the period ring of ℚ(i)), (d) the falsification criterion F-CAT-1/2/3 (integer relation at any precision, polynomial identity derived analytically, or proof that the period-ring statement is false), (e) the evidence-strengthening criteria S-CAT-1/2/3/4 (extended-precision PSLQ at 200 / 500 digits; extended basis adding Γ(1/3) / Γ(1/5) / W^(4)_BCC; direct Deligne regulator computation -- none of which close the conjecture), and (f) the closure criteria CLOSE-CAT-1/2/3 (Baker / Deligne / Eisenstein-series transcendence routes, all FO-difficulty and beyond current scope; CLOSE-CAT-4 is the negative closure per S4). **The default expectation is that none of S5 strengthenings or S6 closures will be achieved within FTD's reach.** The PREREG documents the boundary so that future work does not inadvertently mis-cite "the Catalan conjecture is proven in FTD" -- it is not. **No spine tag move; no FTD claim promoted or demoted.** Companion docs: `docs/papers/PAPER_GSTAR_INTRODUCTION.tex` §19 / Conjecture 19.2 (the paper-side statement), `docs/theory/09_mathematical/REF_GUILLERA_CORPUS_MAP.md` (surrounding AGM/period framework), `docs/theory/09_mathematical/ROADMAP_IDENTITY_PRIORITIES.md` (Bundle 1 -- Catalan  {G_G, π, x_+, x_-} -- the synonymy-graph roadmap entry that flags this conjecture as the FO-blocked frontier).

---

## Clock-hypothesis substrate-derivation -- Arc B P2 closure attempt design

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0208** clock-hypothesis substrate-derivation (Arc B P2, v1) | `preregister-clock-hypothesis-derivation-v1` | `4c15ba1` | desk derivation; closure attempt executed 2026-05-25; quick-check companion `scripts/proofs/proof_newton_from_substrate.py` (STEP 3 comment references pre-reg) | n/a (desk) | n/a (desk attempt) | [`PREREG_CLOCK_HYPOTHESIS_DERIVATION_v1.md`](../03_derivations/archive/superseded/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v1.md) (pre-reg, archived 2026-06-15) → [`AUDIT_CLOCK_HYPOTHESIS_v1_UNDERDETERMINED.md`](../07_assessment/audits/AUDIT_CLOCK_HYPOTHESIS_v1_UNDERDETERMINED.md) (Outcome B UNDERDETERMINED, 2026-05-25; adversarial review FAIL → UNDERDETERMINED on executor's provisional CLOSED-NEGATIVE; superseded by v2 invalidation + v3 closed-negative result) |
| **FTD-0208** clock-hypothesis substrate-derivation (Arc B P2, v2 INVALIDATED) | (no tag; pre-reg was never committed or git-tagged) | (no commit; archived) | desk derivation; closure attempt drafted 2026-05-25 19:47 but invalidated by post-hoc audit same day | n/a | n/a | [`archive/retracted/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v2.md`](../03_derivations/archive/retracted/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v2.md) (archived; never hash-locked) + [`archive/retracted/FOUND_CLOCK_HYPOTHESIS.md`](../03_derivations/archive/retracted/FOUND_CLOCK_HYPOTHESIS.md) (archived; claimed FOUND verdict invalidated) → [`AUDIT_CLOCK_HYPOTHESIS_v2_UNDERDETERMINED.md`](../07_assessment/audits/AUDIT_CLOCK_HYPOTHESIS_v2_UNDERDETERMINED.md) — verdict: INVALIDATED on **two independent axes**: (a) **process** — pre-reg and FOUND result authored within the same minute (mtime 2026-05-25 19:47); the v2 pre-reg's own §1 line-16 protocol requires commit-and-tag of §§2-9 BEFORE the closure attempt is run, and that was bypassed (`git tag --list` shows only v1, both files were untracked, FOUND doc claims a tag/SHA256 that does not exist in git); (b) **substance** — v2 §4 catalog item 7 introduces a quadratic `(dτ/dt_local)² + v_local² = 1` "Bandwidth-Internal-Time budget-conservation primitive" that is QM/SR-borrowed Pythagorean L²-norm structure with no derivation from FTD Postulates 1–5 (ternary state space `{-1,0,+1}^Λ` has no native L² norm); per v2's own Outcome B this primitive is "an intermediate principle outside the §4 catalog that has not been independently substrate-derived" → honest verdict is UNDERDETERMINED, not FOUND. v3 later executed and closed negative. |
| **FTD-0208** clock-hypothesis substrate-derivation (Arc B P2, v3) | `preregister-clock-hypothesis-derivation-v3` | `0dbc5aa` | desk derivation; closure attempt executed 2026-05-27 after pre-reg commit/tag | n/a | n/a | [`PREREG_CLOCK_HYPOTHESIS_DERIVATION_v3.md`](../03_derivations/foundational_mechanics/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v3.md) (pre-reg) → [`AUDIT_CLOCK_HYPOTHESIS_v3_CLOSED_NEGATIVE.md`](../03_derivations/archive/AUDIT_CLOCK_HYPOTHESIS_v3_CLOSED_NEGATIVE.md) (Outcome C CLOSED-NEGATIVE, AXIOM-LEVEL; adversarial review PASS, 9.8/10) |

Pre-reg v1 SHA256: `9feb9d57ee53709ca419a6d068ed183b4b1426186bdaf662fad84061438ee4a5`.
Pre-reg v3 SHA256: `646cca3ac8b37502df2ef190afea6fff02338b6b73440b0b0065120780c00a78`.

When auditing: confirm `git rev-list -n1 preregister-clock-hypothesis-derivation-v1` resolves to commit `4c15ba1` (the commit that introduced `PREREG_CLOCK_HYPOTHESIS_DERIVATION_v1.md`), and that the file's SHA256 still matches the value above (`sha256sum docs/theory/03_derivations/archive/superseded/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v1.md`). This pre-registration locks the design of the Arc B P2 closure attempt of the Wilsonian-reframe plan v2. The Arc B P0 reconciliation audit (`AUDIT_NEWTON_POSTULATES_RECONCILIATION.md` §2, commit `a7d8b8f`) found that SPEC_FTD_LAGRANGIAN.md §4.3 [THEOREM] subsumes DERIV_NEWTON_FROM_SUBSTRATE.md §1.4's [POSTULATE 2] modulo the clock hypothesis (the identification "Born-Infeld action measure IS proper time"). A grep across `docs/` returns the clock hypothesis only in SPEC §4.3 and the AUDIT — not formally tagged anywhere. This pre-reg locks the question (§2 Q-CH-1), definitions D1-D6 (§3), the FROZEN admissible search space (§4 = SPEC §3.7 bandwidth constraint + substrate manifestation rate + Born-Infeld action measure; explicitly excludes GR's empirical clock postulate + standard relativistic-particle-theory moves + Schwarzschild form insertion), the benchmark (§5 = `dτ/dt = √(f - v²/f)` SPEC §4.3 form), the three pre-blessed outcomes (§6 = FOUND / UNDERDETERMINED / CLOSED-NEGATIVE), the falsifier F-a..F-j (§7), the banned moves B-1..B-8 (§8), and the locked 11-step method (§9) with mandatory adversarial review checkpoint at step 9 BEFORE the numerical comparison at step 10. **Prior-favoured outcome: UNDERDETERMINED** — the clock hypothesis is a standard interpretive step in relativistic-particle theory; a substrate-physics derivation via the bandwidth-constraint route (SPEC §3.7's "v and ℒ draw from same bandwidth budget") is plausible but unattempted; the likely failure mode is requiring an intermediate principle outside the §4 catalog. **F9 collusion-bias risk HIGH** (target value `√(f - v²/f)` is canonical GR proper-time formula known to any physics-trained agent or reviewer); §7 + §8 + §9 step 9 calibrated specifically to catch reverse-engineering toward the target. **No FTD claim is promoted or demoted by this pre-reg** — tag changes happen only at result-doc landing per §6 verdict, never in this pre-reg or in this manifest entry.

When auditing v3: confirm `git rev-list -n1 preregister-clock-hypothesis-derivation-v3` resolves to commit `0dbc5aa` (the commit that introduced `PREREG_CLOCK_HYPOTHESIS_DERIVATION_v3.md`), and that the file's SHA256 still matches the value above (`sha256sum docs/theory/03_derivations/foundational_mechanics/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v3.md`). This pre-registration locked the design of the Arc B P2 v3 closure attempt. The v2 attempt's process and substance failure highlighted that the budget-conservation primitive itself must be derived from FTD axioms, not imported. v3 focused on whether the quadratic relation $(d\tau/dt_{\text{local}})^2 + v_{\text{local}}^2 = 1$ is forced by the discrete FTD substrate. **Result:** CLOSED-NEGATIVE, AXIOM-LEVEL; B-9 and B-10 complied, and independent adversarial review passed.

**v2 attempt INVALIDATED** per `AUDIT_CLOCK_HYPOTHESIS_v2_UNDERDETERMINED.md` — the v2 pre-reg and result document were authored within the same minute with no intervening commit-and-tag step (`git tag --list` confirmed no v2 tag exists; both files were untracked at the time of FOUND-verdict claim). Per v2's own §1 line 16 anti-laundering clause this is determinative of process failure; per v2's own Outcome B the substantive verdict is UNDERDETERMINED because the v2 §4 catalog smuggled in the Pythagorean budget-conservation primitive as a derivation input rather than deriving it from FTD axioms. v3 then executed with sharpened admissibility (target = the budget-conservation primitive itself), new falsifiers F-k/F-l, and new banned moves B-9 (no same-minute mtime) / B-10 (independent-agent adversarial review mandatory), landing CLOSED-NEGATIVE.

---

## Spin-2 boundary theorem -- Arc C2 P3 closure attempt design

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0209** spin-2 boundary theorem (Arc C2 P3) | `preregister-spin2-boundary-theorem-v1` | `d8e016b` | desk derivation; closure attempt executed 2026-05-25 against substantive proof scaffold in `DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY.md` + `DERIV_J_BILINEAR_NO_SPIN2_POLE.md` (commit `d2ec208`) | n/a (desk) | n/a (desk attempt) | [`PREREG_SPIN2_BOUNDARY_THEOREM_v1.md`](preregistrations/PREREG_SPIN2_BOUNDARY_THEOREM_v1.md) (pre-reg) → [`FOUND_SPIN2_BOUNDARY_THEOREM.md`](derivations/FOUND_SPIN2_BOUNDARY_THEOREM.md) (Outcome A FOUND, 2026-05-25; adversarial review PASS-WITH-CAVEATS; all 4 caveats incorporated inline: §5.1 uniqueness sub-case walk, finite-L caveat, L=128 deferral framing, Conjecture 10.1 scope-bounding) |

Pre-reg SHA256: `c6bd0e182d85cf9027c4a1d54d0c16b83724c6a2bbd12a3b0b8391b0036440db`.

When auditing: confirm `git rev-list -n1 preregister-spin2-boundary-theorem-v1` resolves to commit `d8e016b` (the commit that introduced `PREREG_SPIN2_BOUNDARY_THEOREM_v1.md`), and that the file's SHA256 still matches the value above (`sha256sum docs/theory/10_eft_program/preregistrations/PREREG_SPIN2_BOUNDARY_THEOREM_v1.md` — path corrected 2026-07-12; the file lives in `preregistrations/`). **Census note (FTD-0384): executed 2026-05-25, Outcome A FOUND — not standing debt; the §13 L=128 engine deferral is the attached-unrun residue, disposition scheduled Arc 2 (run or retire explicitly).** This pre-registration locks the design of the Arc C2 P4 closure attempt of the Wilsonian-reframe plan v2 (Arc C2: spin-2 boundary theorem, caps the upper end of substrate-derived gravity scaling per the Wilsonian reframe). The substantive proof scaffold is already authored in `DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY.md` (4-clause consolidated derivation with dual tag structure) + `DERIV_J_BILINEAR_NO_SPIN2_POLE.md` (load-bearing C2-2 bubble-integral analysis, [THEOREM] free-theory + [SMC] canonical-toggle with FTD-0193 11/12 k-point empirical floor); this pre-reg's function is verdict-discipline lockdown, not new derivation. The pre-reg locks the four-clause theorem statement (§2 Q-SPIN2-BOUNDARY-v1, verbatim D1 from DERIV doc §1), definitions D1-D7 (§3, including D7 Arc B P2 verdict-branch handling that accommodates FOUND / CLOSED-NEGATIVE / pending without blocking C2 closure), the FROZEN admissible search space (§4: 14 inclusions including FTD axioms 1-5 + calibration + §4 frozen catalog + DERIV docs + FTD-0193 + Peskin-Schroeder §10.2 + Montvay-Münster §3 lattice analog; 6 exclusions including h_μν import as derivation input + Deser-bootstrap as substrate-emergence evidence + Lovelock-implies-substrate-GR + Doctrine §12 candidate principles + LIGO-as-evidence + closed-negative routes FTD-0073/FTD-0184/FTD-0050), the benchmark (§5 = four-clause theorem statement at dual-tag scope), the three pre-blessed outcomes (§6 = FOUND / CLOSED-NEGATIVE / UNDERDETERMINED), the falsifier F-a..F-j (§7) with F-h critically distinguishing structural [THEOREM]-grade argument from FTD-0193 empirical floor (catches F9 risk), the banned moves B-1..B-8 (§8) with B-5 enforcing dual-tag preservation in result-doc + B-3/B-4 preventing metaphysical priors and LIGO-as-substrate-spin2-evidence framings, and the locked 11-step method (§9) with mandatory adversarial review checkpoint at step 10 (separate reviewer; executor cannot self-review). **Prior-favoured outcome: FOUND** — the DERIV docs already establish the chain at [THEOREM] free-theory + Gauss-only + [SMC] canonical-toggle level; the closure attempt is mechanical F-/B-checklist verification + adversarial review, not new derivation work. **F9 risk HIGH** ("easy theorem hides assumptions"); the §7/§8/§9 step 10 discipline is calibrated specifically to catch this. §1 honest framing per FTD-0186 v2 §1 precedent: this is scope clarification, not "we proved no graviton." **Sibling to FTD-0186 Stage 1** (structural/dynamical-value discriminator [STAGE 1 CLOSED POSITIVE per v2]): both are boundary theorems on independent axes; methodologically parallel. **No FTD claim is promoted or demoted by this pre-reg** — tag changes happen only at result-doc landing per §6 verdict.


## x_- physical-identification search -- Arc B P1 closure attempt design

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0210** x_- physical identification | `preregister-x-minus-physical-identification-v1` | `6a0392e` | [`search_x_minus_candidates.py`](../../../scripts/exploration/search_x_minus_candidates.py) -- frozen-catalog search (pre-reg §4); no numerical search, no near-miss scan | n/a | n/a (desk audit) | [`AUDIT_X_MINUS_CLOSED_NEGATIVE.md`](archive/closed_negative/AUDIT_X_MINUS_CLOSED_NEGATIVE.md) (verdict: CLOSED-NEGATIVE) |

Pre-reg SHA256: `06c1cd0f0c82f331292d51620077d6eec99424af8a728de4fc24a3cfbe619f08`.

When auditing: confirm `git rev-list -n1 preregister-x-minus-physical-identification-v1` resolves to the commit that introduced `PREREG_X_MINUS_PHYSICAL_IDENTIFICATION_v1.md`, and that the file's SHA256 still matches the value above. The question, definitions D1-D6, the FROZEN admissible search space (§4), the three pre-blessed outcomes (§5), the measurement procedure (§6), the falsifier F-a..F-j (§7), and the banned moves B-1..B-10 (§8) were all locked before the search was run.

---

## W5 Moore-shell DM weighting independent confirmation -- Arc B P1 closure attempt design

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0211** W5 DM weighting confirmation | `preregister-w5-confirmation-v1` | `ae9996e` | [`verify_w5_cosmology.py`](../../../scripts/exploration/verify_w5_cosmology.py) | n/a | n/a (desk/numerical) | [`PREREG_DM_BARYON_W5_INDEPENDENT_CONFIRMATION_v1.md`](preregistrations/PREREG_DM_BARYON_W5_INDEPENDENT_CONFIRMATION_v1.md) (pre-reg) → [`FOUND_DM_BARYON_W5_CONFIRMATION.md`](archive/resolved/FOUND_DM_BARYON_W5_CONFIRMATION.md) (Outcome B UNDERDETERMINED) |

Pre-reg SHA256: `a771b279327b0e82d409b645416ca9b1a68633b129e0852e875790150dbaa2ee`.

When auditing: confirm `git rev-list -n1 preregister-w5-confirmation-v1` resolves to the commit that introduced `PREREG_DM_BARYON_W5_INDEPENDENT_CONFIRMATION_v1.md`, and that the file's SHA256 matches the value above. The campaign design, question, independent observables, and three pre-blessed outcomes were locked before the verification was run.

---

## Lemniscatic K_2-regulator closed-form derivation -- Arc B P1 Path A design

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0212** Lemniscatic K_2-regulator derivation | `preregister-lemniscatic-k2-regulator-v1` | `ae9996e` | [`proof_lemniscatic_k2_regulator.py`](../../../scripts/proofs/proof_lemniscatic_k2_regulator.py) | n/a | n/a (numerical proof) | [`PREREG_LEMNISCATIC_K2_REGULATOR_v1.md`](preregistrations/PREREG_LEMNISCATIC_K2_REGULATOR_v1.md) (pre-reg) → [`FOUND_LEMNISCATIC_K2_REGULATOR.md`](archive/closed_negative/FOUND_LEMNISCATIC_K2_REGULATOR.md) (Outcome C CLOSED-NEGATIVE) |

Pre-reg SHA256: `c514f20593bde5fb6e0638367420499e778dbfd0ff00b0e24e84fdbaffa9f797`.

When auditing: confirm `git rev-list -n1 preregister-lemniscatic-k2-regulator-v1` resolves to the commit that introduced `PREREG_LEMNISCATIC_K2_REGULATOR_v1.md`, and that the file's SHA256 matches the value above. The campaign design, functional equation accelerated series, and PSLQ period basis were locked before the verification was run.

---

## FTD Native strong-field gravity signature campaign -- FTD emergent gravity audit (FTD-0213)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0213** FTD native strong-field gravity signature | `preregister-strong-field-gravity-v1` | tag `preregister-strong-field-gravity-v1` | [`verify_strong_field_gravity.py`](../../../scripts/exploration/verify_strong_field_gravity.py) | n/a | n/a (numerical simulation) | [`PREREG_STRONG_FIELD_GRAVITY_v1.md`](preregistrations/PREREG_STRONG_FIELD_GRAVITY_v1.md) (pre-reg) → [`FOUND_STRONG_FIELD_GRAVITY_SIGNATURE.md`](../03_derivations/archive/closed_negative/FOUND_STRONG_FIELD_GRAVITY_SIGNATURE.md) (post-attempt) |

Pre-reg SHA256: `9c624520b99ed40a2ac0dc43bb7d70a2a8572b98129eded3479bc23496701bf8`.

When auditing: confirm `git rev-list -n1 preregister-strong-field-gravity-v1` resolves to the commit that introduced `PREREG_STRONG_FIELD_GRAVITY_v1.md`, and that the file's SHA256 matches the value above. The campaign design, physical observables (ISCO, precession, decay), and pre-blessed outcomes were locked before the verification was run.

---

## No 4th Generation Fermions No-Go Formalization Campaign -- Moore Layer Theorem (FTD-0220)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0220** No 4th generation fermions no-go | `preregister-no-4th-generation-no-go-v1` | tag `preregister-no-4th-generation-no-go-v1` | [`verify_no_4th_generation.py`](../../../scripts/exploration/verify_no_4th_generation.py) | n/a | n/a (combinatorial proof) | [`PREREG_NO_4TH_GENERATION_NO_GO_v1.md`](preregistrations/PREREG_NO_4TH_GENERATION_NO_GO_v1.md) (pre-reg) → [`FOUND_NO_4TH_GENERATION_NO_GO.md`](derivations/FOUND_NO_4TH_GENERATION_NO_GO.md) (post-attempt) |

Pre-reg SHA256: `6d53d163f26ce47641c51a8612afe2b106bda3fe13e3b37db9bb3b75f8820435`.

When auditing: confirm `git rev-list -n1 preregister-no-4th-generation-no-go-v1` resolves to the commit that introduced `PREREG_NO_4TH_GENERATION_NO_GO_v1.md`, and that the file's SHA256 matches the value above. The campaign design, polyhedral decomposition representation counts, and pre-blessed outcomes were locked before the verification was run.

---

## δ-IND closure definition — the δ-independence program Stage S2 (FTD-0368)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0368 S2** δ-IND native-closure definition v1 (frozen before δ is tested) | `preregister-delta-ind-closure-v1` | `63e9c506` | [`proof_s2_adequacy_anchors.py`](../../../scripts/proofs/proof_s2_adequacy_anchors.py) | n/a | n/a (exact-arithmetic adequacy anchors; stdout) | [`PREREG_DELTA_IND_CLOSURE_DEFINITION_v1.md`](../02_foundations/PREREG_DELTA_IND_CLOSURE_DEFINITION_v1.md) (pre-reg) → [`ANALYSIS_DELTA_IND_CLOSURE_v1.md`](../02_foundations/ANALYSIS_DELTA_IND_CLOSURE_v1.md) (post-lock verdict, FTD-0369: **PROVEN-CONDITIONAL**, 2026-07-05) |

Instrument SHA256: `452038d1164f04524dc7345627dcd13e7e3c67fd088923acf7678ab603073394`.

When auditing: confirm `git rev-list -n1 preregister-delta-ind-closure-v1` = `63e9c506` (the commit that introduced the prereg + instrument), that the instrument's SHA256 matches, and that **the instrument contains no δ-computation** (ban B1 — the definition of N, its D1–D4 clauses, the adequacy anchors, and the complete S3 verdict map were locked before any δ-membership question was evaluated).

---

## Vertex program v1 — DK evolution + noise-controlled bivector closure (FTD-0379/0380; locked)

| FTD ID | Pre-reg tag | Commit | Script / engine artifact | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0379** DK evolution test (M1) | `preregister-vertex-dk-closure-v1` | (lock commit) | engine artifact `engine/tests/test_dk_evolution.cpp` SHA256 `cb3083d4180127d76b38cd504a8107aea44c874b04592588a0e5fbac81696f96` | build target `test_dk_evolution`; run `ctest --test-dir engine/build -C Release -R "^dk_evolution$" --output-on-failure` | `engine/results/vertex_dk_closure_2026-07-10/` | [`PREREG_VERTEX_DK_CLOSURE_v1.md`](preregistrations/PREREG_VERTEX_DK_CLOSURE_v1.md) §2 → analysis doc post-run |
| **FTD-0380** noise-controlled bivector closure (M2) | `preregister-vertex-dk-closure-v1` | (lock commit) | engine artifact `engine/tests/test_bivector_closure_v2.cpp` SHA256 `c6c74322dbf2f46fdfcefe9252391feb13364dc37e2dc1bb0ae028c0f9f8637a` | build target `test_bivector_closure_v2`; run `ctest --test-dir engine/build -C Release -R "^bivector_closure_v2$" --output-on-failure` | `engine/results/vertex_dk_closure_2026-07-10/` | [`PREREG_VERTEX_DK_CLOSURE_v1.md`](preregistrations/PREREG_VERTEX_DK_CLOSURE_v1.md) §3 → analysis doc post-run |
| **FTD-0379 v1.1** corrected-operator, free-scale DK re-test | `preregister-vertex-dk-closure-v1-1` | `07a03489` (+ pre-measurement gate-sign amendment `280e5d86`; the first-lock runner failed its own D² gate, exit 1, no dynamics output observed) | engine artifact `engine/tests/test_dk_evolution_v11.cpp` SHA256 `cfe38b5729fce787cc5db74315d5aa0679560c7c87c0a77a436cbd8141710247` | build target `test_dk_evolution_v11`; run `ctest --test-dir engine/build -C Release -R "^dk_evolution_v11$" --output-on-failure` | `engine/results/vertex_dk_closure_2026-07-10/` | [`PREREG_VERTEX_DK_CLOSURE_v1_1.md`](preregistrations/PREREG_VERTEX_DK_CLOSURE_v1_1.md) → [`ANALYSIS_VERTEX_DK_CLOSURE_v1.md`](../09_mathematical/algebra/ANALYSIS_VERTEX_DK_CLOSURE_v1.md) §1.5: **DK-STATIC-ONLY, fitted operator speed a\* ≈ 0** |

Lock/run provenance for the v1 pair: lock commit `b46fdfe0`, results same-day
(M1 DK-STATIC-ONLY; M2 CLOSURE-ROBUST-FAIL). The v1.1 row is the adversarial
math review's prescribed decisive follow-up (the v1 harness executed FTD-0089
§A1.3 literally, whose δ-convention is provably not the DK operator — see the
analysis §1.3c); v1.1's true-adjoint, free-scale instrument returned the same
verdict with the operator scale fitted to zero, closing both instrument
loopholes.

The two decisive, previously-scoped-but-never-executed measurements of the
vertex program (§7-bivector → §7-dirac critical path). M1 executes
`DERIV_DIRAC_KAHLER_IDENTIFICATION.md` §A1.5 (does engine evolution satisfy
the discrete Dirac-Kähler equation, vs a Klein-Gordon comparator?). M2
executes the noise-controlled re-test named in
`DERIV_BIVECTOR_ALGEBRA_PROGRAM_F.md` §3.5.3 (does the FTD-0087 4-injection
closure failure recover under time-averaged readouts / larger L / lower A?).
Four pre-blessed outcomes each, priors stated in the pre-reg, ramification
grade 0 throughout (no α/δ content anywhere). **No FTD claim is promoted or
demoted by this pre-reg** — tag changes happen only at analysis-doc landing
per the pre-reg's tag-impact table (§5), never in this manifest entry.

---


## How to add a new pre-registration row

1. **Pre-register** before measurement:
   - Decide the script + flags + expected outcome.
   - Commit the script (and any pre-registration prose). Compute its
     SHA256 (`sha256sum tools/<script>.py` or equivalent for C++
     campaigns) and record it in the campaign's pre-reg analysis doc
     stub.
   - Create a lightweight git tag pointing at the pre-reg commit:
     ```sh
     git tag preregister-<name>-v1 -m "Pre-reg for FTD-NNNN: <description>"
     git push origin preregister-<name>-v1
     ```

2. **Run** the measurement against the tagged commit. Save output to
   `engine/results/<campaign_name>_YYYY-MM-DD/`. The directory is
   gitignored by default; track only the analysis-doc-cited subset
   with `git add -f <path>`.

3. **Add a row to this manifest** populating all six columns. Cite
   the analysis doc and the LEDGER row.

4. **Don't retroactively pre-register**. If a measurement was run
   before the tag, don't backfill — record it in the "earlier
   campaigns" table above instead. The discipline only works if
   pre-registration genuinely precedes measurement.

---

## Verification recipe (reproducing a tagged campaign from scratch)

```sh
# 1. Check out the pre-registration commit (read-only inspection).
git checkout <pre-reg tag or commit SHA>

# 2. Verify script SHA matches what the analysis doc recorded.
sha256sum <script>      # compare against analysis doc

# 3. Build and run.
#    (Native CTest build / WSL2 build / WASM build — per CLAUDE.md.)

# 4. Compare output to analysis doc's reported numbers.
#    Bit-for-bit reproducibility is not guaranteed across machines
#    (RNG seeding modulo platform), but statistical equivalence of
#    the reported summary statistics is.

# 5. Return to main:
git checkout main
```

---

## Cross-references

- [`CLAUDE.md`](../../../CLAUDE.md) §"NEW INFRASTRUCTURE 2026-04-27" —
  introduces the pre-registration discipline.
- [`docs/WHERE_WE_LEFT_OFF.md`](../../WHERE_WE_LEFT_OFF.md) §10 —
  bird's-eye assessment, includes the structural-bridge gap that
  motivates further pre-registered campaigns.
- [`07_assessment/core_ledgers/LEDGER.md`](../07_assessment/core_ledgers/LEDGER.md) — single
  source of truth for claim status; each FTD-NNNN row cross-references
  its pre-reg tag (when present) and analysis doc.
- [`CHANGELOG.md`](../../../CHANGELOG.md) "Measurement output → pre-
  registration tag mapping" — short summary table mirroring this
  manifest's rows for the 2026-04-27 cycle.


---

## Consumption Program governance lock (Arc-6 review; locked 2026-07-12)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| (governance — under FTD-0383/0384) program review at Arc-6 close | `preregister-program-review-arc6-v1` | `5ec5e992` | n/a (desk review; executor = owner + six-chair AI panel) | frozen outcome map RE-SCOPE / ARCHIVE-AS-MAPPED-BOUNDARY / CONTINUE(re-chartered); window = Arc-6 close or 6 months, whichever first; unexecuted-past-window blocks all lock-cutting | n/a | [`PREREG_PROGRAM_REVIEW_ARC6_v1.md`](preregistrations/PREREG_PROGRAM_REVIEW_ARC6_v1.md) (SHA256 `af6ad402095feeb0ab4cc37f9e25a6f22fb8c1e41876f8dd40d9f0e9959a00c7`) |

## Self-energy pinning lock (K_MANIFEST substrate-origin conjecture; locked 2026-07-17)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| (unminted — conjecture adjudication; LEDGER row only on owner adoption per outcome map) | `preregister-selfenergy-pinning-v1` | (tag commit) | `scripts/proofs/prereg_selfenergy_pinning_predictions.py` (predictions, SHA256 `b83eed3d8475e6c024b71c1863301f7d956c1ebcac9b6b21d75c8ddaf50fe02a`); measurement = standalone driver over `engine/src/poisson_solvers.cpp` `gauss_project_cpu` (engine operator of record, no engine-tree edits) | projector-only GF-A: single s=+1 at center, all dynamics off, charge_coupling=1.0, exact_dual_gauss=false (engine default), SOR 6 iters/application, converge max residual < 1e-8 or 10^4 applications; L ∈ {17,33,65}; adjudication = tracker convention E_half within 0.5% of a family's exact finite-L values at ALL three L | (driver output recorded in prereg §9 execution record) | [`PREREG_SELFENERGY_PINNING_v1.md`](preregistrations/PREREG_SELFENERGY_PINNING_v1.md) (SHA256 at lock `a37e739304b9cfcc3da06f64356c05acf2e86922e600a9ad1791c4bf983858fd`; §9 execution record appended post-run by design) |

## Census addendum — closure over the tag namespace (2026-07-12, FTD-0384)

This manifest's per-campaign tables above are **not exhaustive** over the `preregister-*` git-tag
namespace. As of the Arc-1 "Honest Mint" census (`tools/preregister_census.py`, standing arc gate per
the Consumption Program charter AM-4), the **authoritative closure** of the namespace is:

- the mechanical census itself (run it: `python tools/preregister_census.py`; exit 0 = GREEN), and
- the dispositions register `preregister_census_dispositions.json` (same directory) — one entry per
  tag/citation not fully reconciled by a manifest row, each with status
  (`executed-verdict-booked` / `anchored-late` / `arc2-disposition-pending` / `historical-superseded`
  / `retracted`), note, and date.

First-census results (full narrative: `../07_assessment/AUDIT_PREREGISTER_CENSUS_2026-07.md`):
70 undispositioned failures → GREEN; 2 counterfeit FOUND verdicts **retracted** (color-confinement,
stochastic-effective-action); 4 tag-claim defects repaired by late anchor after SHA recovery at
historical commits; 24 late-anchor tags cut at registration commits; 11 items pending Arc-2
disposition (grandfather clause). Rows for the 24 `executed-verdict-booked` items are owed to the
tables above; until written, the dispositions JSON is authoritative for them.

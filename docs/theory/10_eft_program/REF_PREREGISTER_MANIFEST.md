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

For the later self-identifying Markdown protocols, `protocol_sha256` is the
SHA-256 of the exact byte prefix preceding the backticked
`protocol_sha256=` field. Hashing the complete file is not the registered
operation because the complete file contains the hash itself. The associated
independent certificate implements this byte-prefix check.

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
| (unminted — conjecture adjudication; LEDGER row only on owner adoption per outcome map) | `preregister-selfenergy-pinning-v1` + `-v1-1` (procedural amendment, prereg §10; L=65 cap 2.5×10⁴). **EXECUTED 2026-07-17 — OUTCOME-P1** (prereg §11): all three L valid, P1 matched at ≤0.00084%, P2 excluded >215%; K_MANIFEST := W_SC advances to owner adjudication as [DERIVED — substrate geometry, CANDIDATE] | (tag commit) | `scripts/proofs/prereg_selfenergy_pinning_predictions.py` (predictions, SHA256 `b83eed3d8475e6c024b71c1863301f7d956c1ebcac9b6b21d75c8ddaf50fe02a`); measurement = standalone driver over `engine/src/poisson_solvers.cpp` `gauss_project_cpu` (engine operator of record, no engine-tree edits) | projector-only GF-A: single s=+1 at center, all dynamics off, charge_coupling=1.0, exact_dual_gauss=false (engine default), SOR 6 iters/application, converge max residual < 1e-8 or 10^4 applications; L ∈ {17,33,65}; adjudication = tracker convention E_half within 0.5% of a family's exact finite-L values at ALL three L | (driver output recorded in prereg §9 execution record) | [`PREREG_SELFENERGY_PINNING_v1.md`](preregistrations/PREREG_SELFENERGY_PINNING_v1.md) (SHA256 at lock `a37e739304b9cfcc3da06f64356c05acf2e86922e600a9ad1791c4bf983858fd`; §9 execution record appended post-run by design) |

## Invariant/quotient roadmap locks (2026-07-20 onward)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0395** full-state irreversibility — **FULL-NONINJECTIVE** | `preregister-full-state-irreversibility-v1` | `30bf2216` | `engine/tests/test_full_state_irreversibility.cpp` SHA256 `45ec650a3a7ef62b1ba4fa9fd6833b6102ddc81f364d10e0c8f23cbdb9445674`; binary SHA256 `bf83a1d7c12333b54b03ffda811bd46b54b16cd937c9b578d4988e7a4dc31301` | WSL2 `engine/build_wsl`; CPU-forced; `FTD_FORCE_GPU` unset; only `evaporation` ON; seed `20260422`; duplicate execution; FTD-0394 target as separate gate; all gates PASS | `engine/results/full_state_irreversibility_2026-07-20/` | [`ANALYSIS_FULL_STATE_IRREVERSIBILITY_v1.md`](../02_foundations/ANALYSIS_FULL_STATE_IRREVERSIBILITY_v1.md); prereg SHA256 `cad088896cdc6854ffa6e9fdc70d28b5c5a4bd7424006d5ab4f8d03a64ec82e4` |
| **FTD-0396** nonlinear delta-IND v2 — **bounded BLOCKED-ESCAPE; unrestricted BLOCKED-ESCAPE** | `preregister-nonlinear-delta-ind-v2` | `8b6003d3` | `scripts/proofs/proof_nonlinear_delta_ind_v2.py` SHA256 `f4624e066131fe43f775b8d334d06893d7af0d978e5e07b8b0673215c08c94f5` | spec-level `Fraction` arithmetic; both exact anchors PASS; duplicate output identical; no engine/alpha/mass evidence; delta valuation NOT RUN because properness did not succeed | `engine/results/nonlinear_delta_ind_v2_2026-07-20/` | [`ANALYSIS_NONLINEAR_DELTA_IND_v2.md`](../02_foundations/ANALYSIS_NONLINEAR_DELTA_IND_v2.md); prereg SHA256 `5179b24c480cf89be6fb4b0e4fc6df2a72fdb995c2180e94f697244532394385` |
| **FTD-0397** `n=11` order-type no-go — **PROVEN-SCOPED** | `preregister-n11-order-type-no-go-v1` | `7012c12b` | `scripts/proofs/proof_n11_order_type_no_go.py` SHA256 `49b8e5b0939547e3cb73635e79e190cf34ba9aea3ecb312f3a52ffcd1cfdb707` | exact integer group action; 12 orderings form one `S4` orbit; positions `{10:2,11:4,13:4,14:2}`; invariant selectors choose 0 or 12; duplicate output identical; no empirical target | `engine/results/n11_order_type_no_go_2026-07-20/` | [`THEOREM_N11_ORDER_TYPE_NO_GO.md`](../05_particles/THEOREM_N11_ORDER_TYPE_NO_GO.md); prereg SHA256 `c2f33663b583f6012ab3e4a8029904493850b5c8f4e447600f1301e0ef713c1d` |
| **FTD-0398** terminal topological-charge transport — **UNDERDETERMINED** | `preregister-topological-charge-transport-v1` | `993d78c5` | `engine/tests/campaign_topological_charge_transport.cpp` SHA256 `44f8965d167231bad7019b2bbf79fc8f23356dc26a87f6f29d4db3bb11cae12c`; verifier SHA256 `a73e9036f1946039e9eb0496bb7f0719669d020cec264f552e9bb2e2550906b2`; binary SHA256 `4e8b0b3b4e24d277c4fb47052d0e75ede2aebb9cdcd44ddbe24a04a31ec5b649` | WSL2 Ubuntu 22.04.5 `engine/build_wsl`; CPU-forced after constructor; `FTD_FORCE_GPU` unset; A/C/E; `L=17`; `t=0..8`; scaled octahedra `R=1..6`; all gates PASS; duplicates byte-identical; verifier UNDERDETERMINED | `engine/results/topological_charge_transport_2026-07-20/` (CSV SHA256 `5338d373b80b9eae37bd3c0b23a563d3ea7523ed7920d75693e5cf31eb1ac4fc`) | [`ANALYSIS_TOPOLOGICAL_CHARGE_TRANSPORT_v1.md`](../03_derivations/foundational_mechanics/ANALYSIS_TOPOLOGICAL_CHARGE_TRANSPORT_v1.md); prereg SHA256 `50bee4c449b6e66f3cbb4e04717b7c69f86093843991cab9c637476f4d9dce80` |
| **FTD-0399** target-blind particlehood — **INVALID (G2)** | `preregister-target-blind-particlehood-v1` | `c4f7af98` | `engine/tests/campaign_target_blind_particlehood.cpp` SHA256 `14833be2d81d31b682af73b51618126fa9a6c1991b2d965d0538a907c367b501`; verifier SHA256 `a35a7c1b2a4b50818678cc59fcf4343d6d818bde61171813fd5bfa370d1e75d0`; binary SHA256 `738898b931001bb8895ae425c8c477bdb3a5a61d28401999d407d0c7ffd56cb7` | WSL2 Ubuntu 22.04.5 `engine/build_wsl`; CPU-forced; `FTD_FORCE_GPU` unset; A/C/E; dissipative+undamped; `L=33,65`; A/E fail to manifest by 200 in all four protocol/size arms, C manifests at 2; all internal duplicates and two external stderr records identical; profile verifier N/A after G2 | `engine/results/target_blind_particlehood_2026-07-20/` (stderr SHA256 `a8edc12dbfd73a77f91bb39e4873fa56039aab53ce6e6e722bb22fc3c4d39972`) | [`ANALYSIS_TARGET_BLIND_PARTICLEHOOD_v1.md`](../05_particles/ANALYSIS_TARGET_BLIND_PARTICLEHOOD_v1.md); prereg SHA256 `dc9c78f1b34d8de13853a8ec2468a0f85d45ff7907b3ec01946dbad8c4c1eac6` |

## Causal normalization and mass-role reconciliation lock (2026-07-21)

| FTD ID | Pre-reg tag | Commit | Script | Flags | Output dir | Analysis doc |
|---|---|---|---|---|---|---|
| **FTD-0402** causal normalization and mass-role reconciliation — **PARTIAL** | `preregister-causal-normalization-mass-roles-v1` | lock `41e06051`; implementation `6526fefa`; result `e2583b25` | `engine/tests/test_causal_normalization.cpp` SHA256 `a164eed0406b110cdf62b09985ba64ab6fafd330d82c7e9319809d984d52dbdc`; `scripts/proofs/verify_causal_normalization_mass_roles.py` SHA256 `3bb62704d5f77c1ea4f1488129e8cc4bef3e10fb93610e2c5b0bfb849ee7f2ad`; binary SHA256 `4ba9b21b32795b403f56ad48e6dd6a6115240b5dd0f1d7a37f9893727648046f` | exact/targeted gates PASS: raw `B=u²/C²+L²`, explicit mass roles, force/movement/tau parity, golden 7/7 twice, targeted native/GPU twice, WASM/web/verifier; aggregate G9 incomplete after wrapper detachment and owner-directed stop, hence frozen PARTIAL; at this verdict §12-cnorm remained open, with successor FTD-0403 v2 later supplying targeted closure | `engine/results/causal_normalization_2026-07-21/verification.txt` | [`RESULT_CAUSAL_NORMALIZATION_MASS_ROLES.md`](../07_assessment/RESULT_CAUSAL_NORMALIZATION_MASS_ROLES.md) SHA256 `14094e423fb1661f39df5b4456add40be00ecd7423cab3b819c466c32665677d`; prereg SHA256 `1ebbd3359b8766120feb8cd0bbf372786246a2d2adecc1dc9b240b2ffdbc2ee0` |
| **FTD-0403 v1** targeted causal-normalization closure — **INVALID (instrument-domain mismatch)** | `preregister-causal-normalization-targeted-closure-v1` | lock `39f668e4`; result `3a11a6dd`; fixture repair `4325b36a` is outside v1 | exact verifier PASS; changed-surface native CTests 13/14; `boundary_movement` SHA256 after repair `aa3538c47d048e78f8ddcd6ae753a0924a7cc8679cca6a6ccd802f86ae4d0d06` | historical fixture injected raw `velocity.x=-1`, outside the selected `C_SPEED=1/sqrt(3)` domain, so movement correctly projected it before the expected one-tick crossing; v1 precedence fixes INVALID and T3–T6 were not run | `engine/results/causal_normalization_targeted_closure_2026-07-21/v1_verification.txt` | [`RESULT_CAUSAL_NORMALIZATION_TARGETED_CLOSURE_v1_INVALID.md`](../07_assessment/RESULT_CAUSAL_NORMALIZATION_TARGETED_CLOSURE_v1_INVALID.md) SHA256 `d2066f48d38a108a2fa34e4a043d36f648991aab6bde998b4a7e335a4a3d0e35`; prereg SHA256 `09a73ab988a8d54c3bca283c1500406cca4e9e98aa8ac93a05efe375a29bb61a` |
| **FTD-0403 v2** targeted causal-normalization closure — **TARGETED-CLOSURE** | `preregister-causal-normalization-targeted-closure-v2` | lock `6ceaa76e`; result `c574f9e1` | repaired `boundary_movement` source SHA256 `aa3538c47d048e78f8ddcd6ae753a0924a7cc8679cca6a6ccd802f86ae4d0d06`, binary SHA256 `c2d3387816cbb5b89742ca9a284e4dda8a2a08564ae2ebdf89df8c71ece74c8c`; exact verifier SHA256 `3bb62704d5f77c1ea4f1488129e8cc4bef3e10fb93610e2c5b0bfb849ee7f2ad`; WASM SHA256 `0b9c04210a09b92fb638cfae7884280e870941f57bd9d86b9a115abe317ef011` | exact A1–A7/S1–S9 PASS; native 14/14; CUDA 6/6; golden 7/7; WASM build; direct web 3/3; static contracts PASS; no full CTest or unrelated campaign run; closes `§12-cnorm`, makes separately locked NCEMC admissible | `engine/results/causal_normalization_targeted_closure_2026-07-21/v2_verification.txt` | [`RESULT_CAUSAL_NORMALIZATION_TARGETED_CLOSURE_v2.md`](../07_assessment/RESULT_CAUSAL_NORMALIZATION_TARGETED_CLOSURE_v2.md) SHA256 `062f4112a1d4e2376e2266ea59a5dc4aa765258b2bda1638dfe138a6829ed07c`; prereg SHA256 `efe5533f7276870ad4a276e317d61933a56535371a8aba45417c063372c37b2a` |
| **FTD-0404** volumetric measure reconciliation — **VOLUMETRIC-NEUTRAL** | `preregister-volumetric-measure-reconciliation-v1` | lock `adb35cdb`; implementation `92535fe7`; result `d5c4f138` | `engine/tests/test_volumetric_measure.cpp` SHA256 `f51899ee71622b9c1c5ee31a0143e900e940d6d138d2aaf4f5a6bf33c1254e1c`, binary SHA256 `9d5138c239a2968d7a1abbdf87ff32b7efa72a5c588dcc8b6c1893df100876b7`; exact verifier SHA256 `bc9a674f2bbec63b150f44b47130744f6e9b5f3c6e546a06179c0cd892912de9`; WASM SHA256 `4d4e46524616d9da95be43cc66f4a2e9cf17c6c0b6863da2633c548a28615508` | exact 17/17 PASS; changed native/CUDA 7/7; golden 7/7; WASM build; direct browser 3/3; `V_cell=a_lat³` explicit with unit-lattice numerical neutrality; no full CTest or unrelated campaign run; NCEMC unchanged and open | `engine/results/volumetric_measure_reconciliation_2026-07-21/verification.txt` | [`RESULT_VOLUMETRIC_MEASURE_RECONCILIATION.md`](../07_assessment/RESULT_VOLUMETRIC_MEASURE_RECONCILIATION.md) SHA256 `dd28c60b07a2a7ff5655137817bc9e964419927fb3456344d74399d9672f8962`; prereg SHA256 `270ad712890e90d932e07aa62cf2572353c54867ac60cf4a80902dee25f06b36` |
| **FTD-0405** NCEMC feasibility — **DOUBLE-OBSTRUCTION** | `preregister-native-confinement-energy-momentum-contract-v1` | lock `2d74956a`; instrument `726c2daf`; result `f4687711` | `engine/tests/test_ncemc_feasibility.cpp` SHA256 `9614087978b3a541f1cc89f3065b372a103986fe0b2548b358c94d95800909d7`, binary SHA256 `bdb49a8865124388e2eb9592803f1c293e203b6c4f1eb71a709e2d75a3115b16`; exact verifier SHA256 `41c7961527027420cf2b65c7734ad9f3dd0cf3c1ae0e92d05c06f6f81e649ff0` | exact 16/16 PASS; instrument 24/24 twice with identical observations; neighboring native/CUDA 5/5; work residual `-0.076410377105056576`; additive gravitational zero remains underdetermined; no production source, broad suite, or mass campaign | `engine/results/ncemc_feasibility_2026-07-21/verification.txt` | [`RESULT_NCEMC_FEASIBILITY.md`](../07_assessment/RESULT_NCEMC_FEASIBILITY.md) SHA256 `d2043aa299bbbfb6c2bc05841cd2a833a05eece7f0439616bc2447b280af020d`; prereg SHA256 `86c2418062711d3e9e308533c29ae87530e1f35c71a049ad58bf75f6d1ccb849` |
| **FTD-0406** strong stress–energy contract v1 — **CPU-SCOPED-CONTRACT** | `preregister-strong-stress-energy-contract-v1` | lock `2692405f`; implementation `808bf272`; result `d77a644e` | header SHA256 `32fa1fef11fee23138e8da41e5b950810ebcdd0e3ec058fb8b8ea9f06b16d5a5`; source SHA256 `022614da328b632866650a7efa3665a03183b2c0ab17144f7844d0beb1204760`; test SHA256 `37b27312000b4c2ebed2966837b19fbcf1da8e422c9b2f41a8015681779ac57a`; verifier SHA256 `98750acf07c709fecad7b899678953ee355c0b454e23459ba3509c92f5c74d3e`; binary SHA256 `14030f3ac2c34e349837cfca1d32854ee5d82b316927b884a1b1816630c7fb14` | exact 21/21; native 35/35 twice with identical observations; selected two-body residual zero at printed precision; three-body residual `-8.8817841970012523e-16`; local T00 sum and `T00/C_SPEED²` latency source pass; neighboring tests 10/10; golden 7/7; no full CTest, GPU contract, WASM/web or mass campaign | `engine/results/strong_stress_energy_contract_2026-07-21/verification.txt` | [`RESULT_STRONG_STRESS_ENERGY_CONTRACT_v1.md`](../07_assessment/RESULT_STRONG_STRESS_ENERGY_CONTRACT_v1.md) SHA256 `8f8866f81f8af0389ad0d65c72a66f0c2dd7ad753cc8c8ded3758a117a470e25`; prereg SHA256 `10725372b020756d82f823c909384a45cc942f539c5c0a2e329311ee90faf2d7` |

## Moving-source dressing lock (2026-07-26)

| FTD ID | Lock | Implementation | Registered gates | Output | Verdict |
|---|---|---|---|---|---|
| **FTD-0562** full-surface finite-source obstruction | [`PREREG_FULL_SURFACE_SOURCE_OBSTRUCTION_v1.md`](preregistrations/PREREG_FULL_SURFACE_SOURCE_OBSTRUCTION_v1.md), pre-execution SHA256 `D9F9B23232AB1A67A1829090C216207BAF58873E3EC9CE75CC809E395E0531D5` | header `0DA5E527…C3994`; source `44F1B726…FA1E`; test `9F0B9483…4043`; independent proof `FB6CBCA1…72E8` | 4 profiles × 4 periods × 8 directions × 3 axes × 2 mirrors = 768 arms; pole/regularity/mirror/covariance/radius/forcing gates frozen before implementation | `engine/results/ftd_0562/windows_msvc_cpu.json` | 768/768 arms and 96/96 witness groups PASS; `FINITE_RIGID_FULL_SURFACE_CANCELLATION_OBSTRUCTED` |
| **FTD-0563** Gauss-monopole/mobile-dressing dichotomy | [`PREREG_GAUSS_MONOPOLE_MOBILE_DICHOTOMY_v1.md`](preregistrations/PREREG_GAUSS_MONOPOLE_MOBILE_DICHOTOMY_v1.md), pre-execution SHA256 `7629151DEA58E98F44A7FF37271BA591054D67C158C6600C2581523F4E6CFC6C` | header `40F31DA0…D66`; source `ADA07C2B…9018`; test `AFAED106…E70C`; independent proof `2E90A590…DA18` | 4 profiles × 4 volumes × 4 directions × 3 axes × 2 mirrors = 384 arms; exact zero-mode, matched solver/curl, monopole/multipole, mirror, and covariance gates | `engine/results/ftd_0563/windows_msvc_cpu.json` | 384/384 arms, 96/96 witness groups, and 54/54 neutral monotonicity witnesses PASS; `GAUSS_MONOPOLE_MOBILE_DRESSING_DICHOTOMY_PROVED` |
| **FTD-0564** orientation-degree/Gauss-flux independence | [`PREREG_ORIENTATION_GAUSS_INDEPENDENCE_v1.md`](preregistrations/PREREG_ORIENTATION_GAUSS_INDEPENDENCE_v1.md), pre-execution SHA256 `25DB8EA8343E165FE4EFC3FB2D83C4520BEC76CC97A05F907412A7E029C58663` | header `908E8616…D116`; source `727D0A2F…2EE8`; test `829EAD13…D46B`; independent proof `6E1DCCF4…922A` | 2 exact field families × 5 amplitudes × 2 polarities × 3 rotations = 60 arms; affine flux, Berg–Lüscher degree, rank/routing, and source-provenance gates | `engine/results/ftd_0564/windows_msvc_cpu.json` | 60/60 arms and 2/2 exact rank witnesses PASS; `ORIENTATION_GAUSS_INDEPENDENT` |
| **FTD-0567** genesis amplitude/common-action obstruction *(immutable preregistration mislabels FTD-0565)* | [`PREREG_GENESIS_AMPLITUDE_ACTION_OBSTRUCTION_v1.md`](preregistrations/PREREG_GENESIS_AMPLITUDE_ACTION_OBSTRUCTION_v1.md), pre-execution SHA256 `C8DC397572217AF20CD69E01BB398CBB66C3E58A49C309A07A5B8F8F974925C8` | header `07DF98C8…814B`; source `5524C4E4…92A`; test `1F1F5F8B…81EA`; independent proof `9206B8E6…6818` | 32 single + 16 dual exact arms; overshoot, energy, action-threshold, evaporation-preimage, and source-provenance gates | `engine/results/ftd_0567/windows_msvc_cpu.json` | 48/48 arms, 4/4 distinct post amplitudes, and exact action/injectivity witnesses PASS; `GENESIS_ACTION_OBSTRUCTION` |
| **FTD-0569** genesis reservoir dilation | [`PREREG_GENESIS_RESERVOIR_DILATION_v1.md`](preregistrations/PREREG_GENESIS_RESERVOIR_DILATION_v1.md), pre-execution SHA256 `F0E03DBA0FCB2D757881DDF10AFC115E9A89647B056AE734CD90D07B442C0A66` | header `377472A1…4C67`; source `DE56A0EE…A176`; test `3D7D43D0…FB58`; independent proof `60AA92CE…32E4` | 540 accepted-genesis inverse arms; 16 Bernoulli arms; 20-step history, evaporation composition, energy-payload, detailed-balance, and source-provenance gates | `engine/results/ftd_0569/windows_msvc_cpu.json` | inverse `1.11e-16`; 1,048,576 erased-history preimages; `ONE_EVENT_DILATION_OPEN_SYSTEM_ONLY` |
| **FTD-0570** genesis exact-real natural extension/symplectic boundary | [`PREREG_GENESIS_NATURAL_EXTENSION_v1.md`](preregistrations/PREREG_GENESIS_NATURAL_EXTENSION_v1.md), pre-execution SHA256 `1C5EB97350D49AC03F63CD5BF995BDB31E9D300CFF71E4180339AE4D5CD3E0D8` | header `07FE4D2F…21F1`; source `95721063…24BD`; test `F1B7381F…D155`; independent proof `01E6208C…7EB` | 48 baker arms; exact rational depth 100; 4,320 branchwise lift arms; raw symplectic, binary64 collision, energy, generator, and absolute-irreversibility gates | `engine/results/ftd_0570/windows_msvc_cpu.json` | lift inverse `1.89e-15`; energy `8.89e-16`; raw Jacobian max `0.308642`; `EXACT_REAL_NATURAL_EXTENSION_ADDITIONAL_PRIMITIVES_REQUIRED` |
| **FTD-0571** genesis environment-feedback necessity | [`PREREG_GENESIS_ENVIRONMENT_FEEDBACK_v1.md`](preregistrations/PREREG_GENESIS_ENVIRONMENT_FEEDBACK_v1.md), pre-execution SHA256 `BC31C67CF64B70D742525B2D07DB3E387A7A18955EA5F16B5EDC65464A1EBEE4` | header `2F8B7A76…76AB`; source `4DE62DC5…CCB`; test `FF95D99D…99C`; independent proof `EFA4E199…AF6` | block-triangular symplectic theorem; 90 rank arms; exact source audit of 34 continuous spectators; prepared-bath/reset discriminator | `engine/results/ftd_0571/windows_msvc_cpu.json` | 30 rank-4 + 60 rank-6 arms PASS; min defect `0.444444`; no spectator writes; `ENVIRONMENT_FEEDBACK_OR_RESET_REQUIRED` |
| **FTD-0572** genesis minimum symplectic bath | [`PREREG_GENESIS_MINIMAL_BATH_v1.md`](preregistrations/PREREG_GENESIS_MINIMAL_BATH_v1.md), pre-execution SHA256 `26C87DB4BFF2800D07C687031A606728F2982933ABBAD55A73E0BF010DEB4B1C` | header `CCD7B099…1B42`; source `47664A3A…3223`; test `203AB4AC…9891`; independent proof `2D10C82E…D3D9` | 120 defect-rank arms; 360 pair arms; 330 defective-pair symplectic dilations; two-step feedback and passive-energy gates | `engine/results/ftd_0572/windows_msvc_cpu.json` | pair residual `2.22e-16`; second-step formula `1.78e-15`; minimum bath pairs `2/3`; `MINIMAL_FEEDBACK_DILATION_REQUIRES_RESET_OR_ACTIVE_ENERGY_RESERVOIR` |
| **FTD-0573** cubic canonical-form uniqueness and bath price | [`PREREG_GENESIS_CUBIC_CANONICAL_FORM_v1.md`](preregistrations/PREREG_GENESIS_CUBIC_CANONICAL_FORM_v1.md), pre-execution SHA256 `0EABA25DFCE05351FE361AE69920AAA3CD37F79B18A4C2028BB1BCEC7DDE3438` | header `7C3ECEEC…B54`; source `BB11E6F3…5A9B`; test `9C1F5B68…81D1`; independent proof `BC419EE1…AB69` | 48/24 cubic groups; rank-14 constraint; 120 production arms; 30 `a=t` controls; branchwise rank minima and symmetry-price gates | `engine/results/ftd_0573/windows_msvc_cpu.json` | invariant residual `0`; determinant residual `1.11e-16`; symmetry price `1` bath pair; `CUBIC_COVARIANCE_SELECTS_STANDARD_PAIRING_AND_PRICES_ONE_BATH_PAIR` |
| **FTD-0574** native field discrete action and source-operator boundary | [`PREREG_NATIVE_FIELD_DISCRETE_ACTION_v1.md`](preregistrations/PREREG_NATIVE_FIELD_DISCRETE_ACTION_v1.md), pre-execution SHA256 `09970E8A18974B56F399DC68023BD7527FDCED50A937054413C3FC53B7F1AFEB` | header `85B8BD24…81DF`; source `EBDB91ED…0077`; test `940A0D58…49E8`; independent proof `2E4B98A1…E12` | 36 modal arms; 4 lattice-action arms; 4 source-adjoint arms; 96 proper-cubic covariance arms; 8 uniform operator counterexamples | `engine/results/ftd_0574/windows_msvc_cpu.json` | native max residual `2.08e-15`; uniform coded source `0`; minimum documented mismatch `0.08542454310285437`; `NATIVE_FIELD_DISCRETE_ACTION_DERIVED_MAGNETIC_SOURCE_ACTION_MISMATCH` |
| **FTD-0575** native Hodge reciprocity and static-pole cancellation | [`PREREG_NATIVE_HODGE_RECIPROCITY_STATIC_POLE_v1.md`](preregistrations/PREREG_NATIVE_HODGE_RECIPROCITY_STATIC_POLE_v1.md), pre-execution SHA256 `BE33049A5C93E887574BDE5509E93F666150A5CAF02E2B93989D96980D1788F6` | header `AC36A734…29D5`; source `3E7BFD75…F941`; test `AF9EE5E9…2D67`; independent proof `1297BC71…24FC` | 27 infrared arms; 24 proper-cubic rotations; 12 charge + 12 transverse-current arms; 4 corners; 4 operator identities; 8 path variations | `engine/results/ftd_0575/windows_msvc_cpu.json` | max native residual `1.15e-17`; `0<=R<=3`; same/opposite cross energies `-/+0.021058840860053937`; `NATIVE_HODGE_FORCE_DERIVED_STATIC_POLE_CANCELED_SAME_SIGN_ATTRACTIVE` |
| **FTD-0576** native Hodge energy and central-continuity obstruction | [`PREREG_NATIVE_HODGE_ENERGY_CONTINUITY_v1.md`](preregistrations/PREREG_NATIVE_HODGE_ENERGY_CONTINUITY_v1.md), pre-execution SHA256 `98B3F8D13E6FBAAD26931C6DD7EC37C9377BD054899012B109C63A0512C26E78` | header `A7C0FD5F…6691`; source `304CFB0D…D3D6`; test `60B8E77D…22E8`; independent proof `C712D94F…9860` | 36 mode-work arms; 4 full-field + 4 conditional-energy arms; 18 axial hops; 36 polarity checks; 24 rotations | `engine/results/ftd_0576/windows_msvc_cpu.json` | max native energy residual `1.36e-15`; even witness `2`; odd support `17..65`; `NATIVE_HODGE_ENERGY_IDENTITY_CENTRAL_LOCAL_MOBILE_CURRENT_OBSTRUCTED` |
| **FTD-0577** minimal Moore compatibility coat | [`PREREG_MINIMAL_MOORE_COMPATIBILITY_COAT_v1.md`](preregistrations/PREREG_MINIMAL_MOORE_COMPATIBILITY_COAT_v1.md), pre-execution SHA256 `94C706936189B077A144ACA7B64D4FEBE93DCDB93AEA36BA604C466480C80F8D` | header `10FBBEA5…7134`; source `4D8C2405…BD7E`; test `0E260F34…B544`; independent proof `060E5CD9…0D0F` | exact scoped uniqueness/27 weights; 36 path arms; 3 translations; 24 rotations; 4 conditional-energy fixtures | `engine/results/ftd_0577/windows_msvc_cpu.json` | max continuity `9.02e-17`; supports density `27..64`, current `18..56`; cardinality defect `7/8`; `MINIMAL_MOORE_COAT_RESTORES_LOCAL_CENTRAL_CONTINUITY_NONCARDINAL_SELECTED` |
| **FTD-0578** common Moore worldline action and point-carrier obstructions | [`PREREG_COMMON_MOORE_WORLDLINE_ACTION_v1.md`](preregistrations/PREREG_COMMON_MOORE_WORLDLINE_ACTION_v1.md), pre-execution SHA256 `DE4F20274E679F0C0E39967B985025F85D5D6F56A1D142B86CE6DE603A62019B` | header `49880E67…E2C7`; source `D90D5143…867E`; test `969FA187…028F`; independent proof `B839A88E…FB58` | 104 aggregate/split-current arms; 4 independent deposit/orbit action fixtures; 3 translations; 24 rotations; exact centering norms; 108 Peierls arms | `engine/results/ftd_0578/windows_msvc_cpu.json` | max split continuity `1.39e-17`; deposit/orbit `4.32e-18`; centering `0,1/1536,5/3072`; minimum barrier `6.740476153376211e-5`; `COMMON_MOORE_WORLDLINE_ACTION_DERIVED_ENERGY_CENTERING_MISMATCH_PEIERLS_PINNED` |
| **FTD-0579** finite rigid Moore-carrier obstruction | [`PREREG_FINITE_RIGID_MOORE_CARRIER_OBSTRUCTION_v1.md`](preregistrations/PREREG_FINITE_RIGID_MOORE_CARRIER_OBSTRUCTION_v1.md), pre-execution SHA256 `7E9C64012B5595969CBE645302450F234387747138A420D371E834FAB705914A` | header `B8B84173…2347C`; source `79CA3739…8964`; test `0060A49F…7919`; independent proof `B12BFA62…F48C` | 5 profiles; 520 centering arms; 60 Peierls coefficients; 540 potential samples; 12 smooth-binomial scaling arms | `engine/results/ftd_0579/windows_msvc_cpu.json` | direct/Fourier `6.90e-17`; minimum diagonal norm squared `4.3402777777777775e-4`; minimum barrier `2.9465935204693173e-5`; `N Pi_i=0.47975..0.48149` at `m=32`; `FINITE_RIGID_MOORE_CARRIER_CANNOT_REMOVE_CENTERING_OR_PEIERLS_EXTENSION_SUPPRESSES_ONLY` |
| **FTD-0580** symmetric chord Moore action | [`PREREG_SYMMETRIC_CHORD_MOORE_ACTION_v1.md`](preregistrations/PREREG_SYMMETRIC_CHORD_MOORE_ACTION_v1.md), pre-execution SHA256 `E3B651CA2E4D05395DA876DA61B873A11E6E5BD17220CDC70EB055F944527DF3` | header `7297E330…B12`; source `FA0344EA…EC99`; test `CE5F4C97…C6EB5`; independent proof `79DC35D6…D5447` | 936 shape samples; 104 path/centering arms; 104 Peierls coefficients; 936 potential samples; 24 proper rotations | `engine/results/ftd_0580/windows_msvc_cpu.json` | raw continuity `0`; central `1.39e-17`; centering `0`; split `6.94e-18`; minimum barrier `6.740476153376211e-5`; `SYMMETRIC_CHORD_CLOSES_MOORE_CENTERING_PEIERLS_PINNING_REMAINS` |
| **FTD-0581** passive dressing depinning obstruction | [`PREREG_PASSIVE_DRESSING_DEPINNING_OBSTRUCTION_v1.md`](preregistrations/PREREG_PASSIVE_DRESSING_DEPINNING_OBSTRUCTION_v1.md), pre-execution SHA256 `CB525DEF5A5E6B92127C4DFD9C72DCF1F7799E7D97113519EDF2C732E56B0DDC` | header `069BD0FA…24D9`; source `038B2F72…27AB`; test `13354BB8…449E`; independent proof `A23AF414…E928` | 104 threshold arms; 416 passive fixtures; 3,744 passive samples; 2,808 active-budget samples; 24 proper rotations | `engine/results/ftd_0581/windows_msvc_cpu.json` | max threshold residual `1.16e-17`; inverse `7.26e-16`; `v_dep/C_SPEED=0.0281..0.0411`; `PASSIVE_DRESSING_CANNOT_DEPIN_ACTIVE_TRAVERSAL_COSTS_FINITE_EXCITATION` |
| **FTD-0582** native active-mode backreaction | [`PREREG_NATIVE_ACTIVE_MODE_BACKREACTION_v1.md`](preregistrations/PREREG_NATIVE_ACTIVE_MODE_BACKREACTION_v1.md), pre-execution SHA256 `5A488BB1E9B9B25DA4363B0C8B27CDA9EA48B7FD6822124666179A3B5D948BEE` | header `5B286271…BB34`; source `BA3272FE…94AE`; test `A9AD0FC4…7099`; independent proof `8F1A900B…D91B` | 144 active arms; 18,432 field ticks; 12 ballistic arms; four selected-force controls; six coupling pairs | `engine/results/ftd_0582/windows_msvc_cpu.json` | all fields changed; velocity/remainder/anchor/movements exact zero; min ballistic movements `4`; selected response `0.04168`; `FROZEN_NATIVE_FIELD_IS_ONE_WAY_TO_MATTER_ACTIVE_TRAVERSAL_CLOSED` |
| **FTD-0583** noncompact matched-face cohomology | [`PREREG_NONCOMPACT_FACE_COHOMOLOGY_v1.md`](preregistrations/PREREG_NONCOMPACT_FACE_COHOMOLOGY_v1.md), pre-execution SHA256 `755D703FB3E9DA9CA7F2EB46B1FE399D704F739AD08050D39242D1EB0B2BB922` | header `B3C6668E…E490`; source `46F380DE…C0AC`; test `1A65D3F7…9BBA`; independent proof `AF5B9BBC…1C0` | 728 Fourier ranks; 48 harmonics; 24 localized curls; 120 contractions; 120 charge scalings; 24 proper rotations | `engine/results/ftd_0583/windows_msvc_cpu.json` | Betti `(1,3,3,1)`; max residual `8.89e-16`; all local/charge/covariance residuals `0`; `MATCHED_NONCOMPACT_COHOMOLOGY_GLOBAL_ONLY_LOCAL_PROTECTED_DEFECT_CLOSED` |
| **FTD-0584** configuration-space carrier necessity | [`PREREG_CONFIGURATION_SPACE_CARRIER_NECESSITY_v1.md`](preregistrations/PREREG_CONFIGURATION_SPACE_CARRIER_NECESSITY_v1.md), pre-execution SHA256 `06CBA799BE32910C8042C270250EDBA90AAE2A77ABC832492705BC66769F9503` | header `AC8032AA…C69D`; source `83AD9F52…084D`; test `ADEAA5C3…613C`; independent proof `A309DCFD…87CE` | 192 fixed-source/fixed-harmonic fixtures; 960 homotopy samples; uncontained support, transition-rank, zero-vacuum, Derrick, and compact-branch gates | `engine/results/ftd_0584/windows_msvc_cpu.json` | all observer residuals `0`; exact transition rank/nullity `4/0`; proof `38/38`; `CURRENT_FIXED_SOURCE_FIBRES_CONTRACTIBLE_CURRENT_VACUUM_HAS_NO_DEFECT_HOMOTOPY_STATIC_TWO_DERIVATIVE_CORE_UNSTABLE_MINIMUM_ENLARGEMENT_CLASSIFIED_NOT_DERIVED` |
| **FTD-0585** native motion/reaction-front trichotomy | [`PREREG_NATIVE_MOTION_REACTION_FRONT_TRICHOTOMY_v1.md`](preregistrations/PREREG_NATIVE_MOTION_REACTION_FRONT_TRICHOTOMY_v1.md), protocol-lock SHA256 `972F221AAE2BA9CBE1C95C9E71CA9789D3082A1DD5695B56F6996ACD29ABFC1B` | header `4192A26F…1175`; source `36C04AC5…EC95`; test `348EA25A…A8296`; independent proof `706425EB…827` | 12 rest arms/384 ticks; 12 ballistic arms; 36 transport + 36 reaction histories; 12 live stale-kinematics cycles | `engine/results/ftd_0585/windows_msvc_cpu.json` | all residuals `0`; max evaporation wait `16`; proof `58/58`; `TRANSPORT_REACTION_FRONT_AND_STALE_MEMORY_DISTINGUISHED_RECIPROCAL_NATIVE_PARTICLE_MOTION_STILL_CLOSED` |
| **FTD-0586** endogenous reaction-carrier bound | [`PREREG_ENDOGENOUS_REACTION_CARRIER_BOUND_v1.md`](preregistrations/PREREG_ENDOGENOUS_REACTION_CARRIER_BOUND_v1.md), pre-execution SHA256 `2AB91067BD68FC995BDF0318843E074ADF027ADE899F9F2DF1688C0D07F64251` | header `DF970FDA…EE41`; source `C62BD6E5…CFB2`; test `7254CB85…C3F`; independent proof `D337FB8D…BB69` | four spectral volumes; 96 sanitized live arms/12,288 ticks; four external controls; exact first-event bound | `engine/results/ftd_0586/windows_msvc_cpu.json` | worst `3B=1.1598848941400712`; zero endogenous genesis; proof `72/72`; `ENDOGENOUS_N_LE_3_AUTOCATALYSIS_CLOSED_BOUND_INCONCLUSIVE_AT_N_GE_4` |
| **FTD-0587** ignition-cut support ablation | [`PREREG_IGNITION_CUT_SUPPORT_ABLATION_v1.md`](preregistrations/PREREG_IGNITION_CUT_SUPPORT_ABLATION_v1.md), pre-execution SHA256 `C2417CD829E665C6A4936D37DFA7C83F790925E5395FA387C34C03F27C857B2B` | header `08A03E38…71A4`; source `FBCEB5B1…D868F`; test `5B4F15A8…B9D9`; independent proof `E34AFA42…4D2C` | 24 FTD-0474 prefix cells × six continuations = 144 runs/43,200 registered ticks; unchanged stability and mechanism gates | `engine/results/ftd_0587/windows_msvc_cpu.json` + CSV | intact Gauss `20/24,5/6`; cleared Gauss `18/24,4/6`; all causal/reservoir isolated arms `0/24`; proof `39/39`; `MIXED_OR_UNRESOLVED` |
| **FTD-0588** collective source-history bound | [`PREREG_COLLECTIVE_SOURCE_HISTORY_BOUND_v1.md`](preregistrations/PREREG_COLLECTIVE_SOURCE_HISTORY_BOUND_v1.md), pre-execution SHA256 `06DE9E8B896272044D847FF5BEC53A342928E3B210B61AC4D3AD605D9D36692E` | header `28B9CF91…A569`; source `D386A8A1…8A78`; test `8A7E6DBD…5473`; independent proof `FA27492D…8E9F` | four spectral volumes; exact stencil/Parseval identities; 64 common-history + 64 native-unlocked arms/16,384 ticks | `engine/results/ftd_0588/windows_msvc_cpu.json` + CSV | common `N<=5` and asynchronous `N<=4` closed; all 64 unlocked arms completely evaporated; zero genesis; proof `127/127`; `COMMON_N_LE_5_ASYNC_N_LE_4_CLOSED_N5_RESIDUAL_TAIL_UNRESOLVED` |
| **FTD-0589** removal-time pulse bound | [`PREREG_REMOVAL_TIME_PULSE_BOUND_v1.md`](preregistrations/PREREG_REMOVAL_TIME_PULSE_BOUND_v1.md), pre-execution SHA256 `F438DBB1950E009641B1332D57B23B2EDFC23CD522A4E23C17E5FCC967AF5A33` | header `4D52E903…BD71`; source `F3C71E28…A3DA`; test `2FE3C499…0FA0`; independent proof `3C2EE9CA…6AED` | four spectral volumes; 8,736 pulse identities; 48 Gram checks; 24 rotations; 64 prescribed + 32 native-unlocked arms/12,288 ticks | `engine/results/ftd_0589/windows_msvc_cpu.json` + CSV | arbitrary one-time removals `N<=6` closed; 176 evaporations; zero genesis; proof `120/120`; `ARBITRARY_REMOVAL_N_LE_6_CLOSED_NEXT_COUNT_7_UNRESOLVED` |
| **FTD-0590** removal-time cubic-orbit coherence | [`PREREG_REMOVAL_TIME_ORBIT_COHERENCE_v1.md`](preregistrations/PREREG_REMOVAL_TIME_ORBIT_COHERENCE_v1.md), pre-execution SHA256 `E7C766CB3AD7062452F6AC1DDD9B3DC854F0DF6BCC6B2D32B1DC402281BD7721` | header `647381D6…B7DD`; source `B52DAC00…155B`; test `2C3828C8…6D8F`; independent proof `F987A972…B496` | four registered odd volumes; exact mode-orbit coverage; every nonzero displacement orbit; direct character and cross-language gates; no geometry/schedule campaign | `engine/results/ftd_0590/windows_msvc_cpu.json` + CSV | `mu_L=0.3610...0.36274`; arbitrary one-time removals `N<=7` closed with minimum margin `0.3021096767`; proof `72/72`; `ARBITRARY_REMOVAL_N_LE_7_CLOSED_BY_ORBIT_COHERENCE` |
| **FTD-0591** eight-source orbit coherence | [`PREREG_EIGHT_SOURCE_ORBIT_COHERENCE_v1.md`](preregistrations/PREREG_EIGHT_SOURCE_ORBIT_COHERENCE_v1.md), pre-evaluation SHA256 `F6ED8183765BCCC29427DFFBCA6074D916FEDBF7D97B557F38DD3405721D4F70` | header `9981BA1F…C707`; source `1645C2FA…E0E`; test `DC047506…3419`; independent proof `2580C491…1062` | same four odd volumes; all 36 `(L,r)` partitions for `N=8`; parent orbit quantities recomputed; no stronger relaxation or geometry/schedule search | `engine/results/ftd_0591/windows_msvc_cpu.json` + CSV | maximum `1.3473027424` at `L=65,r=7`, margin `0.1690833168`; arbitrary one-time removals `N<=8` closed; proof `122/122`; `ARBITRARY_REMOVAL_N_LE_8_CLOSED_BY_ORBIT_COHERENCE` |
| **FTD-0592** nine-source orbit coherence | [`PREREG_NINE_SOURCE_ORBIT_COHERENCE_v1.md`](preregistrations/PREREG_NINE_SOURCE_ORBIT_COHERENCE_v1.md), pre-evaluation SHA256 `DDAA7FC084C3F8F146E722F15E1089FDDA83D095EB5C55D2B31823A20BD41DE8` | header `E3674F55…B3CC`; source `15C2E4F9…B5DF`; test `D6DA04D8…AD2E`; independent proof `93D67C5D…A2AB` | same four odd volumes; all 40 `(L,r)` partitions for `N=9`; parent orbit quantities recomputed; no stronger relaxation or geometry/schedule search | `engine/results/ftd_0592/windows_msvc_cpu.json` + CSV | maximum `1.4801131738` at `L=65,r=8`, margin `0.0362728854`; arbitrary one-time removals `N<=9` closed; proof `126/126`; `ARBITRARY_REMOVAL_N_LE_9_CLOSED_BY_ORBIT_COHERENCE` |
| **FTD-0593** ten-source orbit-coherence boundary | [`PREREG_TEN_SOURCE_ORBIT_COHERENCE_v1.md`](preregistrations/PREREG_TEN_SOURCE_ORBIT_COHERENCE_v1.md), pre-evaluation SHA256 `10EBAFCC24B0589B975BD14E3CD4FD4508942830EA7A4FB541378655F25DC348` | header `1B540B1E…53FD`; source `FCEDDD66…DB76`; test `09089D16…551F`; independent proof `4343D44A…2DEC` | same four odd volumes; all 44 `(L,r)` partitions for `N=10`; unchanged parent inequality; no stronger relaxation or geometry/schedule search | `engine/results/ftd_0593/windows_msvc_cpu.json` + CSV | all volumes maximize at `r=9`; maximum `1.6127738812` at `L=65`, threshold excess `0.0963878221`; proof `130/130`; `TEN_SOURCE_ORBIT_BOUND_INCONCLUSIVE` |
| **FTD-0594** ten-source exact shared-`M` coherence | [`PREREG_TEN_SOURCE_SHARED_M_COHERENCE_v1.md`](preregistrations/PREREG_TEN_SOURCE_SHARED_M_COHERENCE_v1.md), pre-evaluation SHA256 `F7E04AA0E1B417CC856C58C2B60A4AEABF8D81CA0B766DF5756AC4CEF8A83E25` | header `61E5D02B…E69C`; source `891006A8…1FF4`; test `902E81F1…539D`; independent proof `75DF5129…2542` | exact cyclotomic `6M` keys; full shell census and key/multiplicity comparison; all 44 shared-shell bounds; no approximate clustering or history search | `engine/results/ftd_0594/windows_msvc_cpu.json` + CSV | `L=65` has 6,544 singleton shells, so the decisive bound is unchanged; proof `172/172`; `TEN_SOURCE_SHARED_M_BOUND_INCONCLUSIVE` |
| **FTD-0595** ten-source pair-distance capacity | [`PREREG_TEN_SOURCE_PAIR_DISTANCE_CAPACITY_v1.md`](preregistrations/PREREG_TEN_SOURCE_PAIR_DISTANCE_CAPACITY_v1.md), pre-evaluation SHA256 `3652D216C915389CD1838CA453C6B0A42F47D748771A9C5D3A1AF23BEEA5AB96` | header `7F6A7DB8…5B38`; source `B565914E…910A`; test `366EA4DE…59EF`; independent proof `2CC9BA4E…6373` | exact shared-shell axial/nonaxial kernels; periodic/free cubic animals through size nine; all 44 pair-capacity partitions; no third class or history search | `engine/results/ftd_0595/windows_msvc_cpu.json` + CSV | 25,413 size-nine animals and exact 13-edge cap; worst bound `1.6115888534`; proof `258/258`; `TEN_SOURCE_PAIR_DISTANCE_BOUND_INCONCLUSIVE` |
| **FTD-0596** ten-source distance-distribution LP | [`PREREG_TEN_SOURCE_DISTANCE_DISTRIBUTION_LP_v1.md`](preregistrations/PREREG_TEN_SOURCE_DISTANCE_DISTRIBUTION_LP_v1.md), pre-evaluation SHA256 `D69E9AFE8FCB2ECA487D285AC0B4A85D57FF1182B68FE613E32B0CADE7D3F2FA` | header `4C90DAE5…758D`; source `247B915E…3250`; test `9DBF373F…B1D5`; generator `54504BE1…9D51`; independent proof `2527315C…CA9B` | complete cubic displacement/momentum association scheme; every Fourier-positivity constraint; 32 padded sparse dual certificates; no configuration/history search or extra cut | `engine/results/ftd_0596/windows_msvc_cpu.json` + summary/certificate CSV | all volumes maximize at `r=8`; bounds `1.5218539833...1.5932999259`; proof `396/396`; `TEN_SOURCE_DISTANCE_DISTRIBUTION_LP_INCONCLUSIVE` |
| **FTD-0597** ten-source temporal product capacity | [`PREREG_TEN_SOURCE_TEMPORAL_PRODUCT_CAPACITY_v1.md`](preregistrations/PREREG_TEN_SOURCE_TEMPORAL_PRODUCT_CAPACITY_v1.md), pre-evaluation SHA256 `7FF1D85959CE80932C3F60FBC0E39BEBC09E7567EF39724B166879F41843801D` | header `04D5D343…0CA4`; source `DFD60F38…FC1F`; test `B2C2B81A…F269`; generator `F98C5ABF…BC95`; independent proof `C50803DF…A876` | exact `[-1/4,1]` pulse-product lemma; complete signed exact-shell kernels; unchanged FTD-0596 polytope; 32 padded dual certificates; no configuration/polarity/history/time search | `engine/results/ftd_0597/windows_msvc_cpu.json` + summary/certificate CSV | all volumes maximize at `r=8`; worst bound `1.4577559408`, minimum margin `0.0586301184`; proof `413/413`; `ARBITRARY_REMOVAL_N_LE_10_CLOSED_BY_TEMPORAL_PRODUCT_CAPACITY` |
| **FTD-0599** site-ontic atomic reciprocal hop | [`PREREG_SITE_ONTIC_ATOMIC_RECIPROCAL_HOP_v1.md`](preregistrations/PREREG_SITE_ONTIC_ATOMIC_RECIPROCAL_HOP_v1.md), protocol SHA256 `DDD146E19C06E488C584AFBAB4092FB802E72F4DFC13F12407A5A914704E8886` | header `C7996925…BA7C`; source `FAF52E1E…3BF3`; test `04A92D15…0759`; independent proof `6EC88044…18AF` | locked half-open chart, exact FTD-0577 current, native kick--drift, vector recoil root, independent native energy kill test; first counterexample stops downstream arms | `engine/results/ftd_0599/ftd_0599_one_event_v1.json` + CSV | stationary control passes; unique Arb-certified body hop closes continuity/recoil but energy/work both miss by `6.3504e-6` against `1e-12`; `SITE_ONTIC_NATIVE_RECOIL_MAP_FAILS_ATOMIC_COMPATIBILITY` |
| **FTD-0600** constituent-complete charged-trimer transaction | [`PREREG_CONSTITUENT_COMPLETE_CHARGED_TRIMER_TRANSACTION_v1.md`](preregistrations/PREREG_CONSTITUENT_COMPLETE_CHARGED_TRIMER_TRANSACTION_v1.md), protocol SHA256 `F24CC0BFBF0741B0F1A07DCE3B719EA6452E3DC81BB0E9F76013F211D25F6328` | header `044688FB…5034`; source `742F8E4D…7E5E`; test `40DE5A69…4CE6`; independent proof `E6BDB5FF…5724` | explicit charge-conjugate trimers; exact binding/current/work identities; 32 forward + 32 state-only inverse arms; translation/rotation/permutation gates; 64-step forward/reverse campaign | `engine/results/ftd_0600/ftd_0600_charged_trimer_v1.json` + CSV | one-step gate `1.98e-13`; recovery `2.57e-12`; drift `1.67e-15`; 24 hops; pseudomomentum defect `5.69e-3`; `CHARGED_TRIMER_COMMON_ACTION_CONSTRUCTIVE` with isolated recoil unlicensed |
| **FTD-0601** closed neutral trimer pair | [`PREREG_CLOSED_NEUTRAL_TRIMER_PAIR_DYNAMICS_v1.md`](preregistrations/PREREG_CLOSED_NEUTRAL_TRIMER_PAIR_DYNAMICS_v1.md), protocol SHA256 `89979BF190B8A5FD36DF6642356E455F13ED01C9A2C42E20777B150996C1C1F3` | header `E78AD9F3…FA7C`; source `11F82836…8A41`; test `23064B33…ABAC`; independent proof `F803EC96…858A` | six dynamical constituents; no fixed compensator; 20 forward/reverse one-step arms; 48-step reversible campaign | `engine/results/ftd_0601/ftd_0601_closed_neutral_pair_v1.json` + CSV | 9 hops; recovery `8.51e-15`; drift `1.33e-15`; pseudomomentum defect `1.03e-2`; non-minimal field sign deferred to FTD-0602 |
| **FTD-0602** minimum-energy neutral-pair force sign | [`PREREG_MINIMUM_ENERGY_NEUTRAL_PAIR_FORCE_SIGN_v1.md`](preregistrations/PREREG_MINIMUM_ENERGY_NEUTRAL_PAIR_FORCE_SIGN_v1.md), protocol SHA256 `1ECB8957CCBA4AE5770FDB310E883357F745418DD36AD30CD5C7E7D35366F341` | test `BB8A4D05…91B6`; independent proof `97A11680…BF77` | exact periodic longitudinal minimum; transverse energy challenge; 12 forward/reverse arms; 16-step reversible rest campaign | `engine/results/ftd_0602/ftd_0602_minimum_energy_force_sign_v1.json` + CSV | attraction restored at registered phase; defect `1.91e-4`; `MINIMUM_ENERGY_ATTRACTION_RESTORED_MOMENTUM_CHANNEL_MISSING` |
| **FTD-0603** neutral-pair translation-phase balance | [`PREREG_NEUTRAL_PAIR_TRANSLATION_PHASE_BALANCE_v1.md`](preregistrations/PREREG_NEUTRAL_PAIR_TRANSLATION_PHASE_BALANCE_v1.md), protocol SHA256 `9C88B2B593C2E31EA08999010E71EF85204ECB3F8C63AA248B7A86A937E16595` | test `67179D96…771A`; independent proof `8F308D0B…793E` | 168 minimum-field phase arms at `N=8,16,32`; three integer-period controls; per-arm record | `engine/results/ftd_0603/ftd_0603_translation_phase_balance_v1.json` + summary/sample CSV | 14/32 non-attractive principal-axis phases; phase means unresolved at `2.57e-8`; `TRANSLATION_PHASE_ATTRACTION_NOT_ROBUST` |
| **FTD-0604** symmetric breathing matter core | [`PREREG_SYMMETRIC_BREATHING_MATTER_CORE_v1.md`](preregistrations/PREREG_SYMMETRIC_BREATHING_MATTER_CORE_v1.md), protocol SHA256 `CD8DB5F38A6E9F01BB8EDFAF63664EF940BF0D1F87C1CE8BF5B17789616FDACE` | test `A0A2D29D…B930`; independent proof `4D2BF857…AC66` | 32 static scale minimizations, common-action forward/inverse arms, force-sign and integer-period controls | `engine/results/ftd_0604/ftd_0604_symmetric_breathing_core_v1.json` + sample CSV | exact curvature `48`; scale response about `4.5e-5`; barrier reduction `5.07e-5`; 14/32 non-attractive; locked stationarity gate fails; `SYMMETRIC_BREATHING_CORE_STATIC_BRANCH_CLOSED_NEGATIVE` |
| **FTD-0605** full mirrored internal-shape core | [`PREREG_FULL_MIRRORED_INTERNAL_SHAPE_CORE_v1.md`](preregistrations/PREREG_FULL_MIRRORED_INTERNAL_SHAPE_CORE_v1.md), protocol SHA256 `388926B3947F0C0A378FC3B52BD99E3C94D8F9BBB0A4D325E26CE1252B79C70F` | test `B46CB59C…7F10A`; independent proof `B40585DF…1CF22` | 32 independent six-coordinate optimizations; exact binding Hessian; fast/direct field cross-check; common-action/inverse on returned states | `engine/results/ftd_0605/ftd_0605_full_mirrored_shape_core_v1.json` + sample CSV | exact binding rank `3`; 29 optimizer exhaustions; three nonstationary boundary returns; `FULL_MIRRORED_SHAPE_STATIC_BRANCH_CLOSED_NEGATIVE` |

## Total momentum stress ledger lock (Arc 2, 2026-08-02)

| FTD ID | Lock | Implementation | Registered gates | Output | Verdict |
|---|---|---|---|---|---|
| **FTD-0769** total momentum stress ledger v1 | [`PREREG_TOTAL_MOMENTUM_STRESS_LEDGER_v1.md`](preregistrations/PREREG_TOTAL_MOMENTUM_STRESS_LEDGER_v1.md), protocol SHA256 `215B03A85A76B706E91099CA24E276FAC3B57DE3852353981456F79F411D8A13` | header `77318892…1AC4F`; source `F230ADDC…A5957`; CUDA header `617F3055…3398A`; CUDA source `14E6E209…06E706B3A`; test `28BFD081…9BA98`; independent proof `FCE47F05…ABCFD85`; independent result certificate `scripts/proofs/proof_total_momentum_stress_ledger_result.py` | frozen operators §0; regional transport identity M1/M2; η−τ≡1 corollary; G0–G8 gates (L=11 exactness, L=17 firewall, host/device parity, ordering, region-mask, discriminator, EXCHANGE_SIGN_INVERTED pin); B1–B10 banned-move firewall | `engine/results/ftd_0769/ftd_0769_total_momentum_stress_ledger_v1.json`, SHA256 `544E6A9A9273438A212DC9B61D2BF5C47C11DEF3611853754DCC19950735D24F` | **LOCKED/RUN; EXECUTION INVALID.** WSL2/RTX 5090 `--run` completed 2026-08-02. Forward history and all 13 checkpoints valid, discrete reverse recovery exact, but continuous `reverse_recovery = 3.8786822642578045e-9 > 1e-10` (§6.3 G2, frozen unchanged from FTD-0768, not loosened) — bit-identical to the FTD-0768 parent artifact on every shared field, so an inherited reproduction, not a new defect. Verdict `MOMENTUM_LEDGER_BASELINE_INVALID` on all three axes; §7 items 3–11 (localization, discriminator, closure buckets) never reached (`qualifying_checkpoints: 0` everywhere). Independent result certificate `29/29`. See [`AUDIT_TOTAL_MOMENTUM_STRESS_LEDGER_v1.md`](../07_assessment/AUDIT_TOTAL_MOMENTUM_STRESS_LEDGER_v1.md). Per §8, not tuned or rerun under v1; a fresh v2 would be required. |

## Coupled Quartic Clock Field v1 lock (2026-08-02)

| FTD ID | Lock | Implementation | Registered gates | Output | Verdict |
|---|---|---|---|---|---|
| **FTD-0770** coupled quartic clock field v1 | [`PREREG_COUPLED_QUARTIC_CLOCK_FIELD_v1.md`](preregistrations/PREREG_COUPLED_QUARTIC_CLOCK_FIELD_v1.md), protocol SHA256 `384C67CF1D6B96829C46C144414B1B5F43E8AE1FCD4FB4D83AA132EFB6616AB4` | header `52F3E1FB…018A`; source `49EAAD73…8960`; test `F815C230…EE92`; independent proof `B5ED4A22…F3E2` | selected-model firewall; `m={2,4,6}` period/energy controls; quartic action and shell normalization; axial/Moore factors; dimensionless wave-cycle cancellation; six chain-dispersion arms; compliance; gauge/holonomy; positive-action rollback | focused CTest output plus [`ANALYSIS_COUPLED_QUARTIC_CLOCK_FIELD_v1.md`](derivations/ANALYSIS_COUPLED_QUARTIC_CLOCK_FIELD_v1.md) | **LOCKED/RUN; PASS.** Exact certificate `15/15`; focused CTest `1/1`; max period error `1.53e-9`, max dispersion error `2.16e-8`, action drift `4.05e-13`; verdicts `COUPLED_QUARTIC_CLOCK_FIELD_V1_CONDITIONAL_THEOREMS_PASS`, `GSTAR_LINEAR_SIGNATURE_ABSENT`, `FIXED_BACKGROUND_HOLONOMY_KINEMATIC_ONLY`. |

## Quartic Clock--Rod Synchronization v1 lock (2026-08-02)

| FTD ID | Lock | Implementation | Registered gates | Output | Verdict |
|---|---|---|---|---|---|
| **FTD-0771** quartic clock--rod synchronization boundary v1 | [`PREREG_QUARTIC_CLOCK_ROD_SYNCHRONIZATION_v1.md`](preregistrations/PREREG_QUARTIC_CLOCK_ROD_SYNCHRONIZATION_v1.md), protocol SHA256 `360BAC51AC50F525DD4AF6DCD588F61831F13778C38FF5644854E2D35817FE16` | exact consistency proof `EA03C851…F6E54` | imposed-clock firewall; transformed quartic period; explicit `rho`, shell, and speed role; common coordinate rescaling; abstract-edge calibration cancellation; two-rate conservative-extension discriminator; FTD-0770 selected linear common-cone control | [`ANALYSIS_QUARTIC_CLOCK_ROD_SYNCHRONIZATION_v1.md`](derivations/ANALYSIS_QUARTIC_CLOCK_ROD_SYNCHRONIZATION_v1.md) | **LOCKED/RUN; EXACT CHECKS PASS.** Consistency certificate `20/20`; verdicts `CLOCK_ROD_RATIO_CONDITIONAL_GSTAR_PRESENT`, `P1_P5_SYNCHRONIZATION_UNDERDETERMINED`, `COMMON_CONE_GSTAR_CANCELLATION`. No operational material rod or native clock is established. |

## Native Temporal Occupancy v1 lock (2026-08-02)

| FTD ID | Lock | Implementation | Registered gates | Output | Verdict |
|---|---|---|---|---|---|
| **FTD-0772** native temporal occupancy v1 | [`PREREG_NATIVE_TEMPORAL_OCCUPANCY_v1.md`](preregistrations/PREREG_NATIVE_TEMPORAL_OCCUPANCY_v1.md), protocol SHA256 `3E779CDFFDE5D17299921750A06E26B075000572CAB60E9DA8FBF154239CC41C` | analyzer `01CFDA65…4C90`; exact proof `98F2D9EC…A4B1`; independent result certificate `8757D095…7739` | immutable FTD-0659 hashes; signed basis-covariant fixed-ray observer; parent modal amplitude; all ticks with fixed three-window split; finite-atomic-measure firewall; full CDF and `mu_1,mu_2,mu_4`; `G_rms/G_abs`; `m={2,4,6}` controls; fixed-ray, recurrence, quadrature, amplitude, and covariance gates; no fitted power or nonlinear remap | ignored result JSON `FAD820D5…AD9B`; cell CSV `600A0061…3A3`; [`ANALYSIS_NATIVE_TEMPORAL_OCCUPANCY_v1.md`](derivations/ANALYSIS_NATIVE_TEMPORAL_OCCUPANCY_v1.md) | **LOCKED/RUN; RECURRENCE UNQUALIFIED.** Exact certificate `45/45`; independent result certificate `49/49`; valid parent execution, but all `18` cells fail window return/CDF/moment stationarity and `0/18` pass quartic. Verdict `NATIVE_TEMPORAL_OCCUPANCY_RECURRENCE_UNQUALIFIED`; no native potential, `G*`, phase response, or coupling is derived. |

## Quartic Waveform Nonlinear Edge Signature v1 lock (2026-08-02)

| FTD ID | Lock | Implementation | Registered gates | Output | Verdict |
|---|---|---|---|---|---|
| **FTD-0773** quartic waveform nonlinear edge signature v1 | [`PREREG_QUARTIC_WAVEFORM_NONLINEAR_EDGE_SIGNATURE_v1.md`](preregistrations/PREREG_QUARTIC_WAVEFORM_NONLINEAR_EDGE_SIGNATURE_v1.md), protocol SHA256 `33E126673B8F072CAEBAD490B74F810818373D8014CC2D6F73CEF9592ED88DAA` | exact symbolic proof `scripts/proofs/proof_quartic_waveform_nonlinear_edge_signature.py`, SHA256 `CA8876D7DCF8370C313C96C9016A81A15E7E183D8E8E9FC9F630658DF943CF7E`; no engine implementation | continuous/discrete and selected/native firewalls; equal-branch versus harmonic-mean inversion; conservative inverse potential; quartic period/action/signed phase; correlated moments and exponent limits; general even-power quadratic-edge barrier/curvature ratio; exact `m={2,4,6}` controls; scale cancellation, nonlinear-coordinate dependence, and finite-atomic-measure boundary | [`ANALYSIS_QUARTIC_WAVEFORM_NONLINEAR_EDGE_SIGNATURE_v1.md`](derivations/ANALYSIS_QUARTIC_WAVEFORM_NONLINEAR_EDGE_SIGNATURE_v1.md) | **LOCKED/RUN; EXACT-ONLY CONDITIONAL THEOREMS PASS; NATIVE TEST BLOCKED.** Exact certificate `95/95`; `B_4=48pi/G*^4=1.967895315142656...` and `H_0''K_4=epsilon/3` for the imposed quadratic coordinate edge. Verdicts `QUARTIC_CONTINUOUS_INVERSE_CHAIN_CONDITIONAL_THEOREMS_PASS`, `QUARTIC_NONLINEAR_EDGE_SHAPE_FUNCTIONAL_GSTAR_PRESENT`, `NATIVE_QUARTIC_TIME_DERIVATION_NOT_ESTABLISHED`, and `NATIVE_NONLINEAR_EDGE_TEST_BLOCKED`. FTD-0772 recurrence failure and the absent native edge prohibit an engine run. |

## L=17 Complete Tangent Candidate v1 lock (2026-08-02)

| FTD ID | Lock | Implementation | Registered gates | Output | Verdict |
|---|---|---|---|---|---|
| **FTD-0774** L=17 complete tangent candidate v1 | [`PREREG_L17_COMPLETE_TANGENT_CANDIDATE_v1.md`](preregistrations/PREREG_L17_COMPLETE_TANGENT_CANDIDATE_v1.md), pre-execution SHA256 `0604AF560EA193BDE9E339ADB3FB28C0631B43D204186BEDA977EB700DD7F27E` | pending locked C++ matrix-free runner, test-only tangent codec, independent numerical certificate, and symbolic certificate | exact orientation-0 FTD-0638/0639 representative and FTD-0640 modes 6/7; complete Gauss-chart codec with electric/magnetic harmonics; intrinsic energy Hessian; `h_0=2e-6`, `h_1=1e-6`; root regularity; at-most-64-dimensional filtered block Krylov; isolated rank-four spectral clusters; full-state forward/reverse, scale, covariance, positivity, and isolated-field controls | `engine/results/ftd_0774/` (not yet created) | **LOCKED; NOT RUN.** The only constructive outcome is an approximate finite-volume selected-map tangent candidate. No localization, recurrence, quartic occupancy, native clock, or minimum dimensionless `dt` is licensed by the lock alone. |

## Native q_active Temporal Pilot v1 lock (2026-08-02)

| FTD ID | Lock | Implementation | Registered gates | Output | Verdict |
|---|---|---|---|---|---|
| **FTD-0776** native `q_active` temporal pilot v1 | [`PREREG_NATIVE_QACTIVE_TEMPORAL_PILOT_v1.md`](preregistrations/PREREG_NATIVE_QACTIVE_TEMPORAL_PILOT_v1.md), immutable run-lock SHA256 `3FECCBCC92452DC7C066C6B7A594F65D9358A9E23464D667C7BCC77AD072662E`; erratum `D7F1C0EB...113BD7` | observation-only dumper `90468572...6FDE4`; exact transfer analyzer `2CAFF9E8...F28B0`; independent verifier and raw crossing reconstruction; no production physics diff | exact raw schema/coverage; primary `q_active`; four `L=32`, seed-1 amplitudes; recurrence before occupancy/speed/moment/correlated-`G`/waveform gates; controls cannot substitute; scoped interpretation firewall | ignored artifact root `engine/results/gstar_qactive_pilot_20260802/`; manifest CSV SHA256 `21DC6548...8343F6`; execution-profile audit `8DB630D6...63DA06`; [`ANALYSIS_NATIVE_QACTIVE_TEMPORAL_PILOT_v1.md`](derivations/ANALYSIS_NATIVE_QACTIVE_TEMPORAL_PILOT_v1.md) | **LOCKED/RUN; OBSERVABLE- AND CONFIGURATION-SCOPED NEGATIVE.** Four raw-valid arms, crossings `1,1,0,0`, zero complete primary cycles. Verdict `Q_ACTIVE_RECURRENCE_UNQUALIFIED_IN_LOCKED_L32_SEED1_CPU_SOR_PROFILE`; all downstream quartic diagnostics N/A. No scale/body/minimum-`dt` work. |

## Finite Dyadic Monodromy Clock--Memory Boundary v1 lock (2026-08-02)

| FTD ID | Lock | Implementation | Registered gates | Output | Verdict |
|---|---|---|---|---|---|
| **FTD-0777** finite dyadic monodromy clock--memory boundary v1 | [`PREREG_DYADIC_MONODROMY_CLOCK_MEMORY_v1.md`](preregistrations/PREREG_DYADIC_MONODROMY_CLOCK_MEMORY_v1.md), protocol SHA256 `D1AA721CCB4B6A3D6C6AD657477DC3EE62B865E2821581F18BC4727D641AF32A` | exact verifier `scripts/proofs/proof_dyadic_monodromy_clock_memory.py`, SHA256 `8907670CAF40D165AFB077639FBAF482FD985AD62C6F3145E92D6B9753DD9685`; no engine implementation | sixteen exact/conditional construction gates: forward-power capacity, root sheets/monodromy/return, prefix and ternary counts, append-only recurrence obstruction, single-tower payload no-go, selected two-tower relational repair, quartic composition/occupancy boundary, Floquet orders, commensurability, and causal ceiling | ignored artifact root `engine/results/dyadic_monodromy_clock_memory_20260802/`; gate table `B648F116...D077E`; summary `0B533885...27D65`; [`ANALYSIS_DYADIC_MONODROMY_CLOCK_MEMORY_v1.md`](derivations/ANALYSIS_DYADIC_MONODROMY_CLOCK_MEMORY_v1.md) | **LOCKED/RUN; EXACT/SYNTHETIC CERTIFICATE `16/16` PASS; NATIVE NOT TESTED.** A finite root tower is a recurrence/epoch hierarchy. One tower cannot hold a nonconstant cycle-invariant payload; the relational repair requires a separately selected reference lift. Native birth, reference, persistence, decoding, and source/work closure remain open. |

## Maxwell C3 screen and triangulated-sheet follow-up locks (2026-08-04)

| FTD ID | Lock | Implementation | Registered scope | Output | Verdict |
|---|---|---|---|---|---|
| **FTD-0800** Maxwell-criterion C3 screen | [`PREREG_MAXWELL_C3_SCREEN_v1.md`](preregistrations/PREREG_MAXWELL_C3_SCREEN_v1.md), lock commit `38292bf1`, SHA256 `D2B73573B097BF8BE346B7B17AFF7C8FBC84DC9FEFB8DCA98F25024307556B05` | `scripts/experiments/maxwell_c3_screen.py`; `verify_sc_shear_quartic.py`; post-hoc graph-class follow-up | relaxed-null-direction guard; `n={2,4,infinity}` classification; Tier B registered as exhaustive `N=3..7`; SC through `L=4` | [`ANALYSIS_MAXWELL_C3_SCREEN_v1.md`](derivations/ANALYSIS_MAXWELL_C3_SCREEN_v1.md) | **LOCKED/RUN; CLOSED NEGATIVE — SCOPED.** Actual Tier B is 38 sampled `N=3..6` equilibria. The corrected post-hoc `N=6` pass is exploratory only: 51 accepted sampled embeddings, none stress-plus-flex; 11 classes and realization strata unresolved. No `N<=6` no-go. |
| **FTD-0801** periodic triangulated-sheet C3 follow-up | [`PREREG_TRIANGULATED_SHEET_N4_v1.md`](preregistrations/PREREG_TRIANGULATED_SHEET_N4_v1.md), lock commit `0abf097b`, SHA256 `D009E83E2A039A3FBCD9655046AF1DCFA8BA35300A0654C73C2E604858933E5F` | `scripts/experiments/triangulated_sheet_n4.py` | fixed-cell/free-cell relaxation; `{4x4,6x6,8x8}` plan; positive, negative, and self-stress controls | [`ANALYSIS_TRIANGULATED_SHEET_N4_v1.md`](derivations/ANALYSIS_TRIANGULATED_SHEET_N4_v1.md) | **LOCKED/RUN; `N4_CLAMPED_ONLY`; CLOSED NEGATIVE — SCOPED.** Analysis reports `4x4` and `6x6`: the decisive `cos(2q.x)` free-cell witness reaches exact zero and other resolved witnesses collapse to the numerical floor. This supports only the clamped status of resolved quartic witnesses found so far in the scoped law; the `6x6` random residual is unresolved. |

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

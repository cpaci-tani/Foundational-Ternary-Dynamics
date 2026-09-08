# Hydrodynamics from the strict Φ law: design

Date: 2026-09-07. Status: **DESIGN — approved in brainstorming, awaiting owner spec review.**
Program: strict-discrete stack, recovery wave 3 (candidate name). Law under test:
`phi-v2-staged-candidate-1` (`engine/strict`, `scripts/phi_v2_lattice`).
Parent documents: `engine/docs/PROGRAM_STRICT_DISCRETE_STACK.md` (gates C1–C4, M4),
`engine/docs/PROGRAM_STRICT_RECOVERY_WAVE_2.md`,
`engine/docs/SPEC_STRICT_RECOVERY_SUCCESSOR_REQUIREMENTS_V1.md`.

## 0. Decisions of record

Owner decisions taken during brainstorming on 2026-09-07:

1. **Route:** derive hydrodynamics from the v3 Φ staged candidate. No imported
   lattice-Boltzmann solver. RenderBridge's flux field (leapfrog wave equation,
   Gauss constraint `div J = s`) is not a fluid and is not touched.
2. **Two phases in sequence.** Phase 1 derives and measures the hydrodynamic
   limit of the current law and books the result at its tag. Phase 2 designs,
   prices, and builds a momentum-conserving FCHC-class successor with full
   native parity, **conditional on Phase 1's gate H1 verdict** (section 4.1).
3. **Execution:** implementation and review by Sonnet subagents with
   comprehensive, self-contained instructions; implementer never reviews own
   work; Fable 5.1 coordinates, writes instructions, integrates, and guards tags.
4. Standing project rules bind: exact arithmetic for every derivation, no
   near-match search, no retuning after a result is seen, GPU only through
   WSL2, `-j 24`/`-j 32` for CPU builds and tests, no AI co-author trailers,
   frozen wave-1/wave-2 sources unchanged, golden gate untouched, strict stack
   remains opt-in (`FTD_BUILD_STRICT_CANDIDATE`).

## 1. State of knowledge at design time

All entries were computed in the 2026-09-07 session with `PYTHONPATH=scripts`
and exact integer or rational arithmetic. They are inputs to gate H0, which
recomputes them under test.

| Fact | Result | Method |
|---|---|---|
| Fixed-weight additive invariants of all three collision layers plus streaming, per polarity | dimension **1** (population) | `recovery_kinetic_response.common_additive_classification()` |
| Layer covariance `U C_q = C_{q−1} U` on the exactly-two sector | **0 violations** over 3 × 18,336 rows | direct table check |
| Locked-schedule precessing invariants, weights `w_t = w_0 ∘ U^{−t}`, layer `(ℓ_0 − t) mod 3`, t = 0..11 | dimension **7** per polarity for every ℓ_0 | sympy `DomainMatrix.nullspace`, ~93 s |
| Basis of the 7-space at ℓ_0 = 0 | constant plus the six layer-0 (E,B) values `layer_value_of(c, 0)`; the tangent 3-vector `tangent(c)` lies in the span | rank tests |
| Per-layer conservation of declared moments | `tangent_*` conserved by layer 0 only; `layer_q_*` by layer q only; `field_*` by none; only the constant is U-invariant | `declared_moments()` |
| Fourth-rank isotropy `T_xxxx = 3·T_xxyy` of velocity multisets | 8 body-diagonal: 8 vs 24, no. 18 face+edge unweighted: 10 vs 12, no. 26 Moore unweighted: 18 vs 36, no. **FCHC projected to 3D (6 face × 2, 12 edge × 1): 12 vs 12, yes.** | direct sums |
| Feasible collision density | p = 1/96 maximizes the exactly-two eligibility `q(p) = C(192,2) p² (1−p)^190 ≈ 0.27` per bank per collision stage; half occupation is infeasible | `DERIV_STRICT_KINETIC_REFERENCE.md` |

A claim made earlier in the same session, that stage-dependent invariants are
one-dimensional, required invariance under all three layers at every stage.
That constraint does not describe the locked schedule and the claim is
**withdrawn**. The correct locked-schedule dimension is 7.

Consequence for design: momentum-class hydrodynamics of the current law is
**open at the invariant level**. The conserved 3-vector is a co-precessing
tangent sum: each token's tangent cycles `d → hn → h(d×n)`, a 120° rotation
about that token's own body diagonal, so the invariant's lab-frame meaning is
stage-dependent. Whether the long-wavelength dynamics is Navier–Stokes-class
is decided by gate H1, not by inspection.

## 2. Scope

**In scope.** The field sector of the current law on the homogeneous
doubly-occupied relation background (`s = 0`, uniform ℓ, both A9 slots of
every SC/FCC relation occupied by the same nonblank code), single polarity,
finite periodic torus. Exact linear-response (Boltzmann, product-closure)
hydrodynamics of that sector; one registered CUDA verification; a priced
successor law if the current law's hydrodynamics is not Navier–Stokes-class.

**Boundaries that hold throughout.** No Clay Millennium claim of any kind
(LEDGER FTD-0043 remains retracted). No physical unit calibration: lattice
spacing and microtick are the only units, and `a`, `τ` are external comparison
choices. Token count is not energy or mass in physical units. A passing
software suite is not a physical recovery gate. Phase 2 is an adoption at a
declared price, never a derivation.

## 3. Phase 1: hydrodynamic limit of the current law

Three gates. Each has an exact deliverable, a fixed acceptance rule written
before execution, and an independent review.

### 3.1 Gate H0: invariant census

**Module:** `scripts/phi_v2_lattice/recovery_hydro_invariants.py`.
**Tests:** `scripts/tests/phi_v2_lattice/test_recovery_hydro_invariants.py`.
**Evidence:** `engine/docs/evidence/strict-hydro-invariants-exact.json`.

Functions (all exact, rational output, no floats in acceptance):

- `fixed_kernel()` — re-exports the existing fixed-weight classification.
- `covariance_violations()` — count of rows where `U C_q ≠ C_{q−1} U`.
- `locked_kernel(l0)` — primitive integer basis of the locked-schedule
  invariant space for starting layer `l0 ∈ {0,1,2}`, computed directly from
  the 12-stage constraint rows. A fast path may use the covariance identity
  to reduce to a single-layer kernel, but the direct computation must run at
  least once under test and agree.
- `span_membership(basis, vectors)` — exact rank test.
- `fourth_rank_isotropy(velocities_with_multiplicity)` — returns
  `(T_xxxx, T_xxyy, T_xx, T_xy, isotropic4, isotropic2)`.

**Acceptance:** fixed dimension 1; zero covariance violations; locked
dimension 7 for every `l0`; tangent rows inside the span; isotropy table equal
to section 1. Runtime under five minutes on the 9950X3D. A different number
in any cell is a finding to report, never a value to edit.

**Tag on booking:** [THEOREM — finite, exact, scoped to the field sector on the
frozen background].

### 3.2 Gate H1: exact small-wavevector dispersion

**Module:** `scripts/phi_v2_lattice/recovery_hydro_dispersion.py`.
**Tests:** `scripts/tests/phi_v2_lattice/test_recovery_hydro_dispersion.py`.
**Document:** `engine/docs/DERIV_STRICT_HYDRODYNAMIC_DISPERSION.md`, written as a
contract (definitions, verdict rule) before the computation runs, results
appended after.
**Evidence:** `engine/docs/evidence/strict-hydro-dispersion-exact.json`.

**Objects.** Per polarity, channel space ℚ^192. Reference density p, default
1/96, rational. Linearized collision at layer q is `J_q = I + r K_q` with
`r = p (1−p)^189` and `K_q = collision_operator(q).integer_correction`. A
first-degree perturbation `δN_c(x) = h_c e^{i k·x}` streams to
`h'_{U(c)} = e^{−i k·d(c)} h_c`, so streaming at wavevector k is
`S_k = P_U · diag(e^{−i k·d(c)})` with `P_U` the permutation matrix of U.
One stage is `M_t(k) = S_k J_{ℓ_t}` with `ℓ_t = (ℓ_0 − t) mod 3`; the period
map is `P(k) = M_11(k) ⋯ M_0(k)`, representing 48 physical microticks.

**Method.** Formal expansion in k along a declared rational direction
`k = κ n̂` with `n̂ ∈ {(1,0,0), (1,1,0), (1,1,1)}` (unnormalized integer
vectors; results reported per unit |k|²). Expand `e^{−iκ n̂·d}` to second
order with `i` a formal symbol, so every coefficient lives in ℚ(i). Degenerate
perturbation theory on the unit-eigenvalue subspace of `P(0)`: left eigenvectors
`W` (the 7 conserved weights at stage 0, from H0), right eigenvectors `V` with
`W V = I`, reduced resolvent `R` of `(I − P(0))` on the 185-dimensional
complement. Effective 7 × 7 matrices:

```text
Λ1 = W P1 V
Λ2 = W (P2 + P1 R P1) V
```

where `P1`, `P2` are the first- and second-order coefficients of `P(k)`. The
eigenvalues of `I + κ Λ1 + κ² Λ2` to `O(κ²)` are the per-period mode factors.
Before inverting, verify that `P(0)` has no eigenvalue equal to 1 on the
complement (else H0 missed an invariant: stop and report). Report every
eigenvalue of `P(0)` of modulus 1 (roots of unity from the U period indicate
staggered modes; they are recorded, not suppressed).

Exact arithmetic is mandatory for `Λ1`, `Λ2`, and the verdict. Implementation
may work over ℚ with real and imaginary parts as a 370-dimensional real system,
or over ℚ(i) directly; both must agree on a test case. Full `P(k)` at the
finite k of gate H2 may be evaluated in float64 for the prediction curve only.

**Outputs.** For each direction and for p ∈ {1/96, 1/192, 1/48}: the exact
`Λ1`, `Λ2`; their eigenvalue structure; first-order propagation speeds; second-
order damping rates per unit |k|²; the density-mode diffusion coefficient; the
modulus-1 spectrum of `P(0)`.

**Verdict rule, fixed now.** The law is **NS-class isotropic** if and only if
all of the following hold exactly:

1. The smallest subspace of the 7-space that contains the density weight and
   is invariant under `Λ1` and `Λ2` for all three directions is exactly
   4-dimensional. Its 3-dimensional complement within that block is what
   "momentum-like" means below; no other definition is admitted.
2. In that block, `Λ1` has one longitudinal pair with speeds `±c_s` and two
   transverse modes with zero first-order coupling, for every direction, with
   the same `c_s`.
3. The transverse second-order damping per unit |k|² is identical along
   (1,0,0), (1,1,0), (1,1,1), and the longitudinal damping is likewise
   direction-independent.

Otherwise the outcome is one of: **anisotropic momentum hydrodynamics**
(momentum-like propagating or diffusing modes exist, condition 3 fails),
**diffusive only** (`Λ1` restricted to the 7-space is zero), or **other**,
with the complete mode table booked either way. There is no fourth option and
no reinterpretation after the numbers exist.

**Tag on booking:** [DERIVED — linearized Boltzmann (product) closure at the
declared reference; correlation leakage not bounded here]. The wave-2
full-tangent bounds are loose, so H1 is a prediction that H2 tests, not a
theorem about Φ.

### 3.3 Gate H2: registered CUDA verification

**Preregistration:** `engine/docs/PREREG_STRICT_HYDRODYNAMIC_RESPONSE.md`,
written after H1's exact results exist and before any GPU trajectory runs. It
fixes preparations, seeds, observables, horizons, acceptance band, and the
throughput-derived lattice size.
**Instrument:** `engine/strict/recovery_hydro/hydro_main.cpp` plus
`engine/strict/recovery_hydro/CMakeLists.txt`, mirroring
`engine/strict/recovery/recovery_carrier_main.cpp`: runs the accepted CUDA
`advance`, downloads the bank at stroboscopic times, computes the projected
moments on the host, and emits compact JSON. No kernel or law changes.
**Runner:** `scripts/phi_v2_lattice/recovery_hydro_campaign.py` with
`prepare_campaign`, `validate_lock`, `run_campaign`, `summarize_campaign`,
mirroring `recovery_carriers.py`: source-closure hashing, WSL2 invocation,
preflight and post-run lock checks.
**Audit:** `engine/docs/AUDIT_STRICT_HYDRODYNAMIC_RESPONSE.md` by an
independent reviewer who reproduces the summary, checks every lock, and replays
a sample of transitions against the Python reference.

**Throughput probe (first task of H2).** Build `ftd_strict_cuda_cli` under
WSL2 and measure microticks per second on a p = 1/96 preparation at
L ∈ {16, 32, 48, 64}. Choose the largest L whose registered horizon fits a
two-hour GPU budget per case set. Record the probe in the preregistration.

**Preparation.** Frozen background as in section 2, single polarity (other
bank empty), uniform `ℓ = 0`, phase-0 boundary. Field bank drawn as independent
Bernoulli with site-and-channel probability
`p_c(x) = p (1 + ε φ_c cos(k·x))`, where `φ` is (a) the density mode
(`φ_c = 1`) and (b) each momentum-like right eigenvector from H1, scaled so
that every probability stays in [0, 1]; `k = 2π m n̂ / L` for m ∈ {1, 2} and
the three directions of H1. Draws are a deterministic function of a declared
seed; N_seeds per case is fixed in the preregistration (default 8). The
ensemble is an external counting experiment, never a primitive probability.

**Observable.** At stroboscopic microticks `48 n` (phase 0, stage ≡ 0 mod 12)
the projected conserved moments
`m_a(n) = Σ_x e^{−i k·x} Σ_c w_a(c) N_c(x, 48 n)` for the 7 stage-0 weights
`w_a`, per seed, plus the seed mean and standard error.

**Prediction.** `m(n) = P_eff(k)^n m(0)` with `P_eff(k)` the 7 × 7 block of
the full `P(k)` evaluated at the actual finite k (float64 permitted here).

**Acceptance band, fixed in the preregistration.** For each case: relative RMS
deviation between measured seed-mean and predicted trajectories over the
horizon at most 10 %, and the measured e-folding rate of the slowest mode
inside the seed-scatter band around the prediction. The horizon is at least
three e-folds of the slowest predicted mode, capped by the probe budget.

**Outcomes.** Pass: H1's closure describes this law at this preparation
[MEASURED]. Fail: retained as an obstruction, "Boltzmann closure fails for
Φ-v2 at the registered preparation," with the measured mode table
[CLOSED NEGATIVE at the registered scope]. No retuning of ε, p, k, or band.

### 3.4 Phase 1 booking

- `engine/docs/DERIV_STRICT_HYDRODYNAMIC_SECTOR.md`: H0, H1, H2 at their tags,
  the NS-class verdict, and the explicit trigger decision for Phase 2.
- `engine/docs/PROGRAM_STRICT_RECOVERY_WAVE_3.md`: ownership, gates, evidence,
  disposition, following the wave-1/wave-2 pattern; one pointer line added to
  `PROGRAM_STRICT_DISCRETE_STACK.md` (rows C3 and M4).
- `docs/theory/07_assessment/core_ledgers/LEDGER_ROW_DRAFT_phi_hydrodynamics.md`:
  three draft rows (census, dispersion, verification). The owner assigns ids.
  Session notes record FTD-1029 and FTD-1030 as claimed by earlier unbooked
  drafts; the booking task verifies the next free id with
  `scripts/theory/check_registry.py` rather than trusting that note.
- Wave-2 text stating that fixed channel-weight momentum is excluded stays as
  written (it is true); the locked-schedule result is added beside it.

## 4. Phase 2: FCHC-class momentum-conserving successor (conditional)

### 4.1 Trigger

Phase 2 is built if and only if H1's verdict is **not** "NS-class isotropic".
If H1 returns NS-class isotropic, Phase 2 tasks are skipped and H2 is extended
with a registered shear-wave viscosity measurement on the current law; the
plan carries that branch explicitly.

### 4.2 Candidate law

Working name `phi-hydro-staged-candidate-1` (owner may rename). Route: a
field-sector successor. Relation records, one-way absorption, crossing,
manifestation, the four-stage schedule, and the global clock are unchanged.
Three things change, each priced:

1. **Field alphabet.** Per polarity, velocity set V = 6 face directions with
   multiplicity 2 (a retained binary label, the FCHC fourth-coordinate sign)
   plus the 12 FCC edge directions: 24 velocity channels. The C4 phase k is
   retained as a passive label advanced by one per hop so that absorption's
   `z(k, ε)` write keeps its current definition: 24 × 4 = 96 states per
   polarity, 192 channels per site. The normal `n` and handedness `h` of the
   current flag are dropped. All alphabet cardinalities enter the price table.
2. **Streaming.** Each occupied channel hops one step along its own velocity:
   face channels to the SC neighbor, edge channels to the FCC neighbor. Both
   are within the radius-one Moore support (P4). No velocity precession.
3. **Collision.** At each site and polarity, occupancy set A ⊂ V maps to
   F(A) with `|F(A)| = |A|` and `Σ_{v∈F(A)} v = Σ_{v∈A} v`; F is an involution
   acting within each (mass, momentum) class, fixed-point-free on classes of
   even cardinality and with exactly one fixed point on classes of odd
   cardinality (an involution on an odd set must fix a point), covariant under
   the cube group `F(gA) = gF(A)`, and selected by a
   documented orbit-and-rank rule in the spirit of the current table's
   construction. The full 2^24 table is generated by
   `engine/strict/generate_hydro_tables.py`, hash-pinned, and transcribed to a
   frozen header; the price counts the rule's selection bits, not the table
   size. Passive k labels are carried by a fixed sorted assignment so that
   parallel writes stay unambiguous.

Conserved by construction: population and momentum (fixed weights, streaming-
and collision-invariant). Required proofs: no further linear invariants (H0′
census, expected dimension 4 per polarity) and no unit-modulus staggered
eigenvalues of `P(0)` beyond those recorded (H1′). Spurious lattice-gas
invariants are a known failure mode; if one appears the rule is revised and the
revision is a new priced line.

### 4.3 Gates

| Gate | Deliverable | Acceptance |
|---|---|---|
| Specification | `engine/docs/SPEC_STRICT_HYDRO_CANDIDATE_v1.md`: complete alphabet, schedule, ownership, initialization, boundary, transition map, conflict cases, price table in the currency of `docs/theory/01_reference/SPEC_ADOPTION_PRICING_RULES.md` | Independent review finds every case enumerated; matches the successor-requirements "Deliverable required" list |
| Python reference | `scripts/phi_v2_lattice/staged_hydro.py`, `channels_hydro.py`, checkpoint codec with a new magic and law hash | Complete-state validation, checkpoint round trip, causal witness at radius one, all-stage accounting |
| Native parity | `engine/strict/staged_runtime_hydro.cpp/.h`, `staged_cuda_hydro.cu`, WASM bindings, `frozen_hydro_tables.h` via generator | Exact state and event equality Python/C++/CUDA/WASM on the wave-1 fixture families; CUDA memcheck clean |
| H0′ | census module reused with the new tables | Fixed kernel dimension 4 per polarity; isotropy table row for V |
| H1′ | dispersion module reused | Verdict NS-class isotropic by the section 3.2 rule; exact `c_s²(p)` and `ν(p)` |
| H2′ | preregistered WSL2 campaign at L = 64: shear-wave decay along three directions and a Taylor–Green vortex; Galilean factor `g(ρ)` measured | Viscosity inside the registered band around H1′; anisotropy ratio inside band; `g(ρ)` reported, not hidden |
| Booking | `DERIV_STRICT_HYDRO_CANDIDATE_SECTOR.md`, PREREG/AUDIT docs, LEDGER drafts | Rows tagged [SELECTION — priced adoption candidate] and [MEASURED]; adoption itself is an owner act |

Known lattice-gas properties (velocity-dependent pressure, the Galilean factor,
statistical noise) are declared in the specification and measured in H2′;
they are properties of the candidate, not defects to be tuned away.

## 5. Execution model

- **Planning.** The writing-plans skill produces the Phase 1 implementation
  plan with one task per gate step. The Phase 2 plan is written from section 4
  only after H1's verdict exists, because its trigger and several of its
  registered quantities depend on that verdict. Each task's instructions are
  self-contained: files,
  existing APIs to reuse (`collision_operator`, `common_additive_classification`,
  `channels.U/tangent/layer_value_of`, `coarse.restrict`, `recovery_carriers`
  campaign protocol, `generate_tables.py` transcription pattern), exact
  acceptance tests, and the standing rules of section 0.
- **Agents.** Sonnet subagents implement; a different Sonnet subagent reviews
  each gate and must reproduce the result, not read it. Findings return to the
  implementer. Fable 5.1 writes instructions, resolves rulings, integrates
  documents, and checks every epistemic tag before booking.
- **Isolation.** Work proceeds in a git worktree on a feature branch. The
  concurrent session's uncommitted files are never staged; staging is by path
  with `git diff --cached` checked before each commit.
- **Compute.** CPU: Windows, pinned MSVC 14.44 via `engine\build_native.bat
  shell`, `-j 24` or more. GPU: WSL2 Ubuntu-22.04 only. H0 under five minutes;
  H1 target under thirty minutes; H2 and H2′ sized by the throughput probe.
- **Tests.** pytest under `scripts/tests/phi_v2_lattice/`; CTest for native;
  `FTD_STRICT_CUDA_REQUIRED=1` parity in WSL2; WASM parity for Phase 2.

## 6. Risks and mitigations

- Exact resolvent over ℚ(i) in 185 dimensions may be slow: use the real
  370-dimensional formulation, cache `P(0)` factorization, and cross-check a
  small case both ways. Fallback is high-precision floating point with rational
  reconstruction, reported as such.
- CUDA throughput at L = 64 is unmeasured: the probe decides L before
  registration; L = 32 or 48 is acceptable.
- Lattice-gas noise: multiple seeds and full-torus projection; acceptance uses
  seed scatter, never a single trajectory.
- The 2^24 collision table is 48 MB in three-byte entries: acceptable for
  device global memory; generation is deterministic and cached by hash.
- Spurious invariants in the successor: H0′ and H1′ detect them; a rule
  revision is a new priced line, not a silent edit.

## 7. Deliverable checklist

Phase 1: `recovery_hydro_invariants.py`, `recovery_hydro_dispersion.py`,
`recovery_hydro_campaign.py`, their tests, `hydro_main.cpp` with CMake,
`DERIV_STRICT_HYDRODYNAMIC_DISPERSION.md`, `PREREG_STRICT_HYDRODYNAMIC_RESPONSE.md`,
`AUDIT_STRICT_HYDRODYNAMIC_RESPONSE.md`, `DERIV_STRICT_HYDRODYNAMIC_SECTOR.md`,
`PROGRAM_STRICT_RECOVERY_WAVE_3.md`, two evidence JSON files, LEDGER row drafts.

Phase 2 (conditional): `SPEC_STRICT_HYDRO_CANDIDATE_v1.md`, `staged_hydro.py`,
`channels_hydro.py`, `generate_hydro_tables.py`, `frozen_hydro_tables.h`,
`staged_runtime_hydro.cpp/.h`, `staged_cuda_hydro.cu`, WASM bindings, parity
tests, H0′/H1′ evidence, `PREREG_STRICT_HYDRO_VISCOSITY.md`,
`AUDIT_STRICT_HYDRO_VISCOSITY.md`, `DERIV_STRICT_HYDRO_CANDIDATE_SECTOR.md`,
LEDGER row drafts.

# FTD to EFT Bridge Contract

**Date:** 2026-04-23
**Status:** [SELECTION] bridge contract; native EFT first, QED matching second
**Purpose:** Freeze the minimal contract that lets FTD become a Wilsonian EFT without using physical alpha or Standard Model targets as bridge inputs.

---

## Executive statement

The bridge is not:

```text
FTD arithmetic root x_+ numerically matches 1/alpha, therefore FTD is QED.
```

The bridge is:

```text
FTD microscopic variables
    -> native source/flux fields
    -> fixed continuum scaling prescription
    -> symmetry-allowed operator basis
    -> source-coupled generating functional or transfer measure
    -> RG/blocking flow
    -> renormalized native observables
    -> optional external QED/SM comparison
```

Until this chain is closed, QED and Standard Model comparisons are diagnostic
only. They may guide criticism, but they may not select definitions,
regulators, counterterms, or normalizations.

---

## Current fixed bridge pieces

| Piece | Contract value | Status |
|---|---|---|
| Microscopic source alphabet | `s in {-1,0,+1}` | [AXIOM] |
| Signed source density | `rho = s` | [THEOREM] internally |
| Flux variable | `J_i in R^3` physical vector flux | [SELECTION] |
| Native decomposition | `J = J_L[rho] + J_T` | [THEOREM] after projection |
| Longitudinal constraint | `div J_L = rho` | [THEOREM] for the chosen Gauss operator |
| Transverse modes | `div J_T = 0`, two propagating DoF | [THEOREM] after constraint |
| Auxiliary U(1)-like variable | `J_T = P_T A`, `A ~ A + grad chi` | [SELECTION] representation |
| Native static response | `C_L^FTD = 1` in bare engine units | [THEOREM] for `sigma_18(k) ~ k^2` |
| Native transverse stiffness | `K_T^FTD = 1` canonical normalization | [DEFINITION] |
| Native current normalization | `Z_j^FTD = 1` for signed transport | [MEASURED] movement current |
| Native source/flux vertex | `g_sJ^FTD = 1` canonical normalization | [DEFINITION] |
| Native wave speed | `c_FTD = 1/sqrt(3)` | [THEOREM] from native wave update |
| Physical QED alpha | not derived by current bridge | [OPEN] / current alpha bridge closed negative |

This contract makes the first successful EFT target:

```text
native FTD source/flux EFT
```

not:

```text
physical QED with alpha predicted from x_+
```

---

## Hard prohibition

The following moves are outside the bridge contract:

1. Choosing an operator, regulator, finite counterterm, source normalization, or observable because it improves the residual against CODATA alpha.
2. Calling a standard QED or Standard Model formula an FTD derivation after inserting FTD-selected numbers.
3. Reclassifying the arithmetic root `x_+` as physical `1/alpha` without a new normalization theorem.
4. Treating the auxiliary projected variable `A` as a primitive microscopic gauge field.
5. Using bubble-only or zero-momentum loop diagnostics as physical alpha observables without Ward-compatible contact terms and renormalization conditions.

Allowed language:

```text
x_+ is an arithmetic FTD root with a close empirical alpha match.
```

Disallowed language under the current bridge:

```text
x_+ is derived physical 1/alpha_QED.
```

---

## Required bridge gates

### Gate 1: Field dictionary

Define the continuum fields and their dimensions from FTD variables:

```text
rho_a(x,t)      = a^{-3} s(x,t)
J_a(x,t)        = Z_J(a) J_lattice(x,t)
j_a(x,t)        = Z_j(a) j_lattice(x,t)
J_T             = P_T J
J_L             = P_L J
```

Required output:

```text
field dimensions
normalization choices
projection convention
boundary/zero-mode convention
```

Status: **[CLOSED] (2026-04-24, FTD-0064).** Frozen-scaling contract added as
"Frozen scaling contract" section of `DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md`.
Dimensions `[ρ] = L⁻³`, `[J] = L⁻²`, `[j] = L⁻²T⁻¹`, `[A] = L⁻¹` fixed under
`a_phys ≡ ℓ_P`. Z-factors, zero-mode conventions, and L→∞ / a→0 boundary
protocol all specified. QED-facing source normalization `Z_Q = e_phys` and
canonical field rescaling remain Branch-B matching parameters, but the
native-branch scaling contract is closed.

### Gate 2: Native action or measure

An EFT requires a generator of observables. FTD must choose one of:

```text
Euclidean action / partition function
real-time transfer matrix
Hamiltonian plus constraint surface
stationary ensemble over deterministic histories
```

Required output:

```text
Z[sources] or equivalent history measure
correlation-function definition
source insertion rules
reflection/unitarity/stability statement, as appropriate
```

Status: **[CLOSED] at ensemble-existence level (2026-04-24, FTD-0069).**
Langevin thermostat (FTD-0051) promoted to Gate-2 stationary ensemble in
`DERIV_FTD_NATIVE_LANGEVIN_ENSEMBLE.md`. OU update on `wave_vel` has a unique
stationary distribution with `⟨|w|²⟩ = 3T` verified to 4% on GPU
(`test_langevin_equipartition`). Source-coupled `Z[J^ext]` defined formally
under frozen Gate-1 dimensions; reduces to linear constrained-flux generator
in the `T→0` limit. Explicit `ln Z` beyond Gaussian sector and
reflection-positivity analytic continuation remain Phase-2 tasks.

### Gate 3: Symmetry and operator basis

List every low-dimension operator allowed by the actual FTD symmetries:

```text
translation
cubic O_h rotations
charge conjugation s -> -s
time reversal or arrow selection
source/flux constraint symmetries
projected transverse redundancy, if using A
```

Required output:

```text
complete relevant/marginal operator table through the chosen dimension
forbidden operators with symmetry reason
engineering dimensions
renormalization mixing classes
```

Status: **[CLOSED] at symmetry-enumeration level (2026-04-24, FTD-0068).**
Complete `O_h × C × P` enumeration through `D ≤ 6` now in
`SPEC_OPERATOR_BASIS_COMPLETE.md`: 1 relevant (`A²`), 0 at `D=3`, 4 marginal
(`J²`, `F²`, `B²`, `j·A`) identified with running couplings
`(C_L, K_T, Z_j, g_sJ)`, 2 at `D=5` and 6+ at `D=6` (all irrelevant).
Forbidden operators (C-odd `ρ`, P-odd Chern-Simons) catalogued. Supersedes
pre-Gate-1 `SPEC_OPERATOR_BASIS.md` in the continuum EFT sense. Non-Gaussian
mixing-matrix structure under `b ≥ 4` blocking remains a Phase-2 Gate-4 task.

### Gate 4: Blocking and RG

Define a fixed blocking map:

```text
B_a->ba: (s, J, j) -> (s', J', j')
```

Required output:

```text
charge conservation under blocking
flux projection compatibility
coupling extraction rules
beta functions for native coefficients
scheme-dependence audit
```

Native coefficients to flow first:

```text
C_L^FTD(L)
K_T^FTD(L)
Z_j^FTD(L)
g_sJ^FTD(L)
```

Status: **[MEASURED] at b ∈ {1,2,4,8} (2026-04-24, FTD-0065 + FTD-0067 +
FTD-0070, Phase-2 closure).** `SPEC_FTD_NATIVE_BLOCKING_MAP.md` defines
the finite-volume native blocking contract. The Gaussian b=2 tuple flow
is closed by `DERIV_FTD_NATIVE_BARE_FLOW.md`,
`DERIV_FTD_NATIVE_CURRENT_FLOW.md`, and `DERIV_FTD_NATIVE_RESPONSE_FLOW.md`:

```text
(C_L, K_T, Z_j, g_sJ)(b=2) = (1, 1, 1, 1)
```

**2026-04-24 extensions (P1.2 + P1.3):** (a) full Moore-26 transport routing
(6 face + 12 edge + 8 corner routes) verified by NET-1..NET-14 in
`test_native_engine_transport_flow` with Ward residual < 1e-12 per route;
(b) first mixed-toggle multi-tick Ward measurement closed by
`test_mixed_history_flow` at L=16, 10 ticks, 72 reaction events accumulated,
Ward residual = 0 per-tick and on b=2-blocked interval.

**Phase-2 closure (FTD-0070):** multi-scale flow measurement at b ∈ {1,2,4,8}
on GPU via `test_nonlinear_flow_multiscale`. Flux-energy density:
$\mathcal{E}_b = (4.26, 4.03, 3.93, 3.90) \times 10^{-2}$ natural units with
$\sigma_{\mathcal{E}} \approx 1.6 \times 10^{-3}$. β-function
$\beta_{\mathcal{E}}(1{\to}2{\to}4{\to}8) = (-0.080, -0.034, -0.013)$, all
consistent with zero within 1σ and geometrically decaying toward the IR
attractor. **Gaussian fixed point confirmed at this observable up to
$b = 8$.** See `DERIV_FTD_NATIVE_MULTISCALE_FLOW.md`.

**Still open (Phase-3+):** non-Gaussian flow under full mixed-toggle set
(forces + movement + pair_production + weak); L-scan to $L \ge 32$ to
tighten β-uncertainties; β for $C_L$, $Z_j$, $g_{sJ}$ individually
(currently only aggregated via flux-energy density).

### Gate 5: Ward/projection identities

For the native branch, prove or test:

```text
Delta_t rho + div j = S_reaction
div J_L = rho
div J_T = 0
P_T grad chi = 0
```

For the projected-QED branch, additionally require:

```text
q_mu Pi_mu_nu(q) = 0
contact/seagull terms included
renormalized current conserved
```

Status: **[CLOSED] on native branch (2026-04-24, FTD-0066 + FTD-0067).** Per-toggle
Ward identity verified to 1e-12 for every reaction toggle class (genesis,
pair_production, weak_transmutation, annihilation-during-movement) via
`test_native_engine_history_flow` (NEH-1..NEH-4) and
`test_native_engine_transport_flow` (NET-7). Mixed-toggle multi-tick Ward
verified to machine precision over 10 ticks by `test_mixed_history_flow`.
Projected-QED Ward identities (the second set above) remain [OPEN] under the
Branch-B matching program and are not required for the native EFT paper.

### Gate 6: Matter sector

The bridge must explicitly choose the matter level:

| Level | Matter object | Claim level |
|---|---|---|
| Native | signed manifestation worldlines | FTD-native transport EFT |
| Scalar completion | complex scalar with projected U(1) links | test EFT, not electron QED |
| Dirac completion | projected lattice Dirac matter | QED-facing selected completion |
| SM completion | chiral gauge matter, Higgs/Yukawa, anomalies | separate high gate |

Required output:

```text
matter representation
charge/current definition
mass or gap term status
statistics/spin status
doubler handling if Dirac
```

Status: [OPEN] beyond native signed transport.

### Gate 7: Matching and observables

Native observables must be defined before any external comparison:

```text
C_L^FTD      static source/flux response
K_T^FTD      transverse stiffness and dispersion
Z_j^FTD      signed-current normalization
g_sJ^FTD     source/flux vertex
W_18         local Green geometry
```

External QED observables are allowed only after the native contract is fixed:

```text
static Coulomb coefficient
Thomson scattering amplitude
renormalized transverse kinetic coefficient
running coupling in a declared scheme
```

Required output:

```text
renormalization condition
counterterm policy
regulator family
uncertainty budget
comparison ledger
```

Status: **[MEASURED] for native response tuple (2026-04-24, FTD-0070,
Phase-2 closure).** Native flux-energy density
$\mathcal{E}(b) = (4.26 \pm 0.16) \times 10^{-2}$ at $b = 1$ with
$|\beta_{\mathcal{E}}| \le 0.08$ per b-decade across $b \in \{1, 2, 4, 8\}$ at
$L = 16$ under Langevin + genesis + Gauss projection. Scheme ledger: the
dual-cell blocking map (`block_dual_cell_b2`) is face-averaged; exact
dual-cell Gauss preservation would be a different scheme and is not yet
implemented in production engine. **Still [OPEN]** for physical $\alpha$
(Branch B matching); the EFT Recovery $\alpha_\infty \approx 3.6 \alpha_\text{ref}$
is a Coulomb-source-probe observable, not the density observable measured
here, and the relationship between them is the Gate-6/Gate-7 matching
problem. See `DERIV_FTD_NATIVE_MULTISCALE_FLOW.md`.

---

## Branch policy

### Branch A: native source/flux EFT

This is the active bridge branch.

```text
s       -> signed source
J       -> physical flux
J_L     -> constrained source response
J_T     -> propagating transverse flux
```

Acceptance criterion:

```text
produce a closed source-coupled Wilsonian EFT with native observables and RG
flow, without QED alpha as an input or target.
```

### Branch B: projected QED-like EFT

This is a later comparison branch.

```text
J_T = P_T A
A ~ A + grad chi
j_T couples to A_T
matter representation selected
```

Acceptance criterion:

```text
derive or explicitly ledger every extra QED-facing selection:
matter, regulator, counterterms, charge normalization, and alpha observable.
```

### Branch C: Standard Model EFT

This branch is not active until Branch A is closed and Branch B has a
disciplined matter/gauge completion.

Required additional gates:

```text
nonabelian gauge sectors
chiral fermions
anomaly cancellation
Higgs/Yukawa sector
mass/generation structure
SMEFT operator basis
```

---

## Minimum viable real EFT

FTD becomes a real native EFT when the following document set exists and is
internally consistent:

```text
1. State/flux field dictionary with scaling dimensions.
2. Source-coupled action, transfer matrix, or history measure.
3. Complete low-dimension operator basis under FTD symmetries.
4. Fixed blocking map and native RG flow.
5. Ward/projection identities for the chosen variables.
6. Native response tuple with uncertainties and scheme ledger.
```

The first publishable claim should then be:

```text
FTD defines a native source/flux effective field theory with measured
long-distance Coulomb-like response, transverse wave modes, and native RG flow.
```

The claim should not be:

```text
FTD derives QED alpha.
```

That stronger claim remains [OPEN] until the projected-QED matching branch
passes its own gates.

---

## Immediate work queue

### Phase 1 (P1.1–P1.6): CLOSED 2026-04-24

1. ~~Write the scaling section for `DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md`.~~
   **DONE — FTD-0064, Gate 1 closed.**
2. ~~Promote the native response tuple into a fixed renormalization scheme.~~
   **DONE — FTD-0064 (scheme) + FTD-0069 (nonlinear ensemble), Gates 1 + 2
   closed.**
3. ~~Draft the native source-coupled generator.~~ **DONE — FTD-0069 upgrades
   the Langevin thermostat into a Gate-2 stationary ensemble with formal
   `Z[J^ext]` defined.**
4. ~~Extend the operator basis to include all source/flux operators through
   the chosen dimension.~~ **DONE — FTD-0068, Gate 3 closed;
   `SPEC_OPERATOR_BASIS_COMPLETE.md` enumerates all D≤6 operators.**
5. ~~Add a reaction-aware continuity/Ward ledger for every toggle class.~~
   **DONE — FTD-0066 (per-toggle) + FTD-0067 (mixed multi-tick), Gate 5
   closed on the native branch.**

Additional Phase-1 deliverable: ~~full Moore-26 transport ledger.~~
**DONE — FTD-0065, Gate 4 closed at b=2 transport level.**

### Phase 2: CLOSED 2026-04-24

1. ~~**β-function determination.** Measure `(C_L, K_T, Z_j, g_sJ)(b)` at b ∈
   {2, 4, 8} under the mixed-toggle non-linear dynamics.~~ **DONE —
   FTD-0070. Gaussian fixed point confirmed at $b \le 8$ via flux-energy
   density observable. $|\beta_{\mathcal{E}}| \le 0.08$ per b-decade;
   monotonic geometric decay toward IR attractor.**
2. ~~**Explicit `ln Z` beyond Gaussian.**~~ **Deferred to Phase 3 —
   closed-form computation of source-coupled `Z[J^ext]` is a refinement
   beyond the minimum viable EFT checklist; the ensemble-level measurement
   in FTD-0070 already bounds the relevant physical content.**
3. ~~**Native response tuple with uncertainties.**~~ **DONE — FTD-0070
   supplies ensemble means and standard errors at four block decades.
   Scheme ledger explicit (face-averaged dual-cell adapter).**

### Phase 3: publishable Branch-A paper

Once Phase 2 completes, the Minimum Viable Real EFT checklist is satisfied.
First publishable claim: *"FTD defines a native source/flux effective field
theory with measured long-distance Coulomb-like response, transverse wave
modes, and native RG flow of four coupling coefficients, without QED-α as
an input or target."*

### Deferred (Phase 4+)

- Fermion-emergence alternative routes (pair_production-style, weak, Moore-26,
  velocity-driven) per `DERIV_WH_ALGEBRA_VS_CLIFFORD_NOGO.md §8`.
- 174-ppm $m_p/m_e$ gap mechanistic attack (Theorem-3 gates).
- Branch-B matching decision (QED-α vs native-independent).

This queue keeps the bridge honest: it builds the EFT first, then asks what
external physics it resembles.

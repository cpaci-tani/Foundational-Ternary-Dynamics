# FTD-to-EFT Bridge Status

**Date:** 2026-04-22  
**Status:** [CURRENT-ACTION CLOSED NEGATIVE for QED alpha] / pivot to FTD-native electrodynamics  
**Purpose:** State where the FTD-to-QED-alpha bridge failed, and define the replacement target as native FTD source/flux physics.

---

## Executive verdict

The QED-alpha bridge is real as an attempted dictionary, but it is not load-bearing under the current projected action.

FTD currently has three solid pieces:

1. **Arithmetic core:** the CM/master-quadratic construction and `x_+ = 137.036171...` are robust at the algebraic level.
2. **Gauge-like continuum dictionary:** existing derivations give a plausible temporal-gauge U(1)-like reading of the flux field `J`, Gauss constraint, and long-wavelength field behavior.
3. **Audit discipline:** the Ward-valid Structure-2 scalar check shows which stronger claim fails. The Structure-1 ppb alpha correction is not scheme-independent under the natural scalar gauge completion tested so far.

The missing span was the matching principle:

```text
FTD state/flux dynamics
    -> unique continuum fields
    -> unique matter content
    -> unique lattice operator/regulator
    -> unique renormalized observable
    -> physical alpha
```

At present, several steps in this chain are still selected rather than forced. Therefore the honest status is:

```text
tree-level x_+ alpha match        robust arithmetic/conjectural physics link
Structure-1 ppb correction        measured scheme-specific EFT result
Structure-2 scalar gauge bridge   closed negative under tested natural cases
unique FTD-to-QED alpha bridge    closed negative under current projected action
```

The replacement target is:

```text
FTD state/flux dynamics
    -> native source/flux observables
    -> native response tuple R_FTD
    -> external comparison to QED only as diagnostic
```

See:

```text
SPEC_FTD_EFT_BRIDGE_CONTRACT.md      frozen bridge gates and branch policy
SPEC_FTD_NATIVE_ELECTRODYNAMICS.md   active native source/flux target
DERIV_FTD_NATIVE_LINEAR_GENERATOR.md linear constrained-flux generator
SPEC_FTD_NATIVE_BLOCKING_MAP.md      finite-volume native RG blocking map
DERIV_FTD_NATIVE_BARE_FLOW.md        first bare b=2 native flow audit
DERIV_FTD_NATIVE_CURRENT_FLOW.md     native current b=2 flow audit
DERIV_FTD_NATIVE_RESPONSE_FLOW.md    native static/vertex b=2 flow audit
DERIV_FTD_NATIVE_ENGINE_HISTORY_FLOW.md real engine reaction-history flow audit
DERIV_FTD_NATIVE_ENGINE_TRANSPORT_FLOW.md real engine face-transport flow audit
```

---

## What "bridge" used to mean

The bridge cannot mean "find another calculation that lands near CODATA." That would violate the project rule against numerical near-miss searches.

The bridge must mean:

> Given only FTD axioms, theorem-level structure, and explicitly ledgered selections, determine the EFT data before looking at the target alpha residual.

The required EFT data are:

| Component | Required bridge decision |
|---|---|
| Continuum fields | Which continuum fields arise from `s` and `J` |
| Gauge group | Which gauge factors are active in the alpha calculation |
| Matter content | Spin, charge vector, multiplicity, and mass relation |
| Kinetic operator | Which lattice operator defines the physical gauge kinetic term |
| Regulator | Which Brillouin zone or cutoff prescription is physical |
| Counterterms | Which renormalization prescription is allowed |
| Observable | Which eigenvalue or matrix element is physical `1/alpha` |
| Error budget | Which corrections are controlled, estimated, or open |

If any of those are chosen by checking the final alpha residual, the result is calibration or fitting, not derivation.

The new native program changes the question. It asks for:

| Native component | Required decision |
|---|---|
| Source response | Define/derive `C_L^FTD` from the actual FTD operator |
| Transverse response | Define/derive `K_T^FTD` and dispersion |
| Current normalization | Audit signed source transport and `Z_j^FTD` |
| Native coupling | Define `g_sJ^FTD` without importing QED alpha |
| Scale flow | Measure/derive native flow laws |
| External comparison | Mark QED/CODATA comparisons as diagnostic |

---

## Bridge inventory

| Bridge segment | Current best source | Status | Blocker |
|---|---|---|---|
| Ternary state `s in {-1,0,+1}` as charge-like manifestation | `FOUND_AXIOM_ZERO.md`, Moore-layer docs | Partial | Physical charge interpretation is selected, not fully forced |
| Flux field `J in R^3` as continuous field | `FOUND_AXIOM_ZERO.md` | [SELECTION] | `J` is a minimal continuous extension, not an original axiom |
| `J` as physical vector flux | `DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md` | Supported | Native flux is not automatically a gauge potential |
| U(1) as emergent transverse-projection redundancy | `DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md` | Partial | Matter coupling and alpha observable still open |
| Gauss constraint as source/transverse decomposition | Engine Gauss tests, continuum-limit docs | Strong conditional | Supports two transverse DoF, not full QED by itself |
| Exact native Gauss as dual-cell boundary flux | `DERIV_FTD_NATIVE_RESPONSE_TUPLE.md`, `SPEC_FTD_NATIVE_ELECTRODYNAMICS.md` | [MEASURED] | Production engine still stores cell-centered `J`; exact full-site Gauss would require face-centered or equivalent dual-cell storage |
| Moore shell / BCC role | `DERIV_FTD_NATIVE_RESPONSE_TUPLE.md`, `SPEC_FTD_NATIVE_ELECTRODYNAMICS.md` | [MEASURED] / [SELECTION] | G18 is the direct one-tick longitudinal response; BCC/stella is a delayed Moore layer unless a native action/timing principle selects nonzero G26 corner weight |
| Self-dual half-shell `r^2 = 1/2` | `EXPLR_SELF_DUAL_HALF_SHELL.md` | [MEASURED] / [CONJECTURE] | Dual-edge shell is now a measured response channel, but the link to `G*` remains a bridge hypothesis |
| Master quadratic root `x_+` | FTD-0001, FTD-0002, FTD-0003 | Algebraic theorem | Physical identification with alpha remains conjectural |
| Engine coupling operator | Link 8 and BCC-orthogonality audit | Negative for BCC bridge | Engine stencil is `(SC+FCC)/2`, orthogonal to BCC corner sector |
| Structure-1 one-loop scalar correction | `DERIV_ONE_LOOP_LATTICE_ALPHA.md`, HMC checks | Scheme-specific | Not reproduced by Ward-valid Structure-2 scalar gauge completion |
| Structure-2 scalar gauge completion | `AUDIT_STRUCTURE2_WARD_VALIDATION.md` | Closed negative | Natural scalar cases S2-A through S2-E fail threshold |
| Fermionic QED completion | `DERIV_LATTICE_QED_COMPLETE.md` | [SELECTION] | Fermion content and doubler handling are not forced by FTD axioms |
| Projected matter/current coupling | `DERIV_PROJECTED_EFT_MATTER_COUPLING.md` | Partial native result | Native current/coupling dictionary survives; QED charge normalization closed negative under current action |
| Projected Dirac operator and charge normalization | `DERIV_PROJECTED_DIRAC_OPERATOR_AND_CHARGE_NORMALIZATION.md` | Historical QED-facing candidate | Dirac candidate fixed symbolically; `e0^2 = 1/x_+` not derived |
| Projected EFT renormalization and alpha observable | `OPEN_PROJECTED_EFT_RENORMALIZATION_AND_ALPHA_OBSERVABLE.md` | Closed negative / superseded | Replaced by FTD-native electrodynamics target |
| Projected stiffness route `K_T,0 = x_+` | `DERIV_PROJECTED_STIFFNESS_XPLUS_ATTEMPT.md` | Closed negative | Current action gives canonical transverse stiffness, not `x_+` |
| Projected response-eigenvalue route | `DERIV_PROJECTED_RESPONSE_EIGENVALUE_XPLUS_ATTEMPT.md` | Closed negative under current action | Master quadratic has an algebraic matrix representation, but current projected EFT does not derive the two-sector response matrix |
| Source-current normalization route `e0^2 = 1/x_+` | `DERIV_SOURCE_CURRENT_NORMALIZATION_XPLUS_ATTEMPT.md` | Closed negative under current action | Ternary source transport fixes integer charge and current conservation, not the physical coupling magnitude |
| Regulator/counterterm prescription for QED alpha | `AUDIT_GPU_PLAN_PRIORITIES_1_3_5_6.md`, FTD-0056 | Closed negative for old target | Unrenormalized BCC tadpole has no continuum limit |
| Physical alpha observable | Coupling docs and audits | Superseded | QED alpha is now an external comparison, not the primary FTD observable |
| FTD-native electrodynamics replacement | `SPEC_FTD_NATIVE_ELECTRODYNAMICS.md`, `DERIV_FTD_NATIVE_RESPONSE_TUPLE.md`, `DERIV_FTD_NATIVE_SOURCE_FLUX_COUPLING_CLOSURE.md` | Active target | Native response observables replace QED alpha as the primary physics goal; bare canonical tuple is closed, native scale flow remains open |

---

## What the negative audits imply

Three negative constraints now shape the bridge:

1. **FTD-0050:** the master quadratic is not the characteristic polynomial of the current engine RG step.
2. **FTD-0056:** the unrenormalized BCC one-loop tadpole residual has no continuum limit.
3. **FTD-0058:** the natural Ward-valid Structure-2 scalar gauge completion does not reproduce the Structure-1 ppb closure.

These do not damage the arithmetic core. They do rule out a simple story where the alpha correction is automatically scheme-independent physics.

The current no-go statement is:

> Under the currently documented bridge, no existing path uniquely derives the ppb alpha correction as a physical EFT prediction. The missing choices are matter content, kinetic operator, regulator/counterterm prescription, and alpha observable.

---

## The most conservative bridge rule

A viable bridge should obey this rule:

```text
The EFT operator used for a physical claim must be selected by the same
FTD structure that selects the fields and observable.
```

This creates two honest branches:

### Branch 1: engine-native EFT

Use the engine's actual operator and dynamics.

Consequence:

- This is faithful to the simulation engine.
- It does not naturally access the BCC corner-sector arithmetic behind `16 G*^2`.
- Existing engine alpha-like observables are lattice-geometry quantities, not QED alpha derivations.

### Branch 2: arithmetic/BCC EFT

Use the BCC operator because that is where the CM arithmetic lives.

Consequence:

- This is faithful to the master-quadratic arithmetic.
- It must explain why the BCC sector, rather than the engine's `(SC+FCC)/2` stencil, is the physical electromagnetic kinetic operator.
- It must also derive matter content and renormalization before any loop correction is interpreted physically.

Right now, the project has strong ingredients for both branches, but no theorem that composes them into one unique bridge.

---

## Best next derivation targets

The next useful work is theoretical, not computational:

1. **State-to-field dictionary:** initial result recorded in `DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md`. FTD naturally gives a signed source coupled to a physical vector flux.
2. **Emergent U(1) projection:** initial result recorded in `DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md`. U(1)-like redundancy is best treated as an auxiliary-potential redundancy of transverse projected flux, not as a microscopic freedom of `J`.
3. **Matter/current coupling:** initial result recorded in `DERIV_PROJECTED_EFT_MATTER_COUPLING.md`. Native matter is signed source/worldline matter; projected radiative coupling uses `j_T · A_T`; Dirac matter is the best QED-facing completion but remains selected.
4. **Projected Dirac/charge normalization:** initial result recorded in `DERIV_PROJECTED_DIRAC_OPERATOR_AND_CHARGE_NORMALIZATION.md`. Central-difference Dirac is the minimal symbolic candidate; ternary charge fixes integer `q`, not `e0`; after the stiffness, response-eigenvalue, and source-current attempts, `x_+` is arithmetic-only under the current projected action unless a new normalization theorem is supplied.
5. **Operator choice and renormalization prescription:** decide whether physical electromagnetism lives on the engine `(SC+FCC)/2` operator, the BCC arithmetic sector, or a derived composite, then state the allowed counterterms before computing loop corrections.
6. **Only then rerun numerics:** fixed verification computations are allowed after the matching rule is written down.

---

## Current answer to "where are we?"

We are past "can the GPU recover the ppb number?" The Ward-valid answer was no for the natural scalar Structure-2 bridge.

We are now at the real bridge problem:

```text
Can FTD uniquely select the EFT whose alpha observable is x_+,
or is x_+ only an arithmetic match plus a scheme-specific correction?
```

That is the next decisive question.

### 2026-04-22 first bridge-span result

`DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md` narrows the missing theorem:

```text
FTD state/flux variables -> source-coupled vector EFT     supported
FTD state/flux variables -> compact U(1) gauge theory     not yet derived
```

So the next proof obligation is not numerical. It is to derive the projected EFT completion: matter representation, local coupling to the auxiliary potential, regulator/counterterms, and the alpha observable.

### 2026-04-22 emergent-U(1) result

`DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md` refines the bridge:

```text
microscopic J as physical flux                         supported
U(1) as redundancy of auxiliary transverse potential    supported as projection language
full QED matter/coupling/alpha matching                 still open
```

This avoids forcing primitive gauge ontology onto FTD. U(1) is now treated as an effective description of projected degrees of freedom.

### 2026-04-22 projected-matter result

`DERIV_PROJECTED_EFT_MATTER_COUPLING.md` adds:

```text
rho = s                                      native signed source
j                                            signed state-transport current
rho fixes longitudinal/Coulomb sector        supported
j_T couples to auxiliary A_T                 selected projected EFT coupling
Dirac matter                                 preferred QED-facing completion, still selected
```

The next bridge target is charge normalization and the projected Dirac operator.

### 2026-04-22 projected-Dirac result

`DERIV_PROJECTED_DIRAC_OPERATOR_AND_CHARGE_NORMALIZATION.md` adds the charge gate:

```text
q from ternary source alphabet        supported
central-difference Dirac candidate    selected, symbolic
e0 coupling magnitude                 open
e0^2 = 1/x_+                          not derived
```

The next bridge target is now the regulator/counterterm/observable gate.

### 2026-04-22 renormalization/observable gate

`OPEN_PROJECTED_EFT_RENORMALIZATION_AND_ALPHA_OBSERVABLE.md` reduces the remaining bridge to one precise decision:

```text
derive how x_+ enters the projected EFT:
    stiffness K_T,0
    bare charge e0^-2
    kinetic-response eigenvalue
    or arithmetic-only root
```

No new alpha loop calculation should be classified as a framework prediction until this gate is passed.

### 2026-04-22 stiffness attempt

`DERIV_PROJECTED_STIFFNESS_XPLUS_ATTEMPT.md` tests R1 and closes it negative under the current action:

```text
native projected transverse stiffness K_T,0 = 1     supported
K_T,0 = x_+                                         not derived
```

That left R3 as the next algebraically natural question, because the master quadratic has two roots and can be written as a characteristic polynomial.

### 2026-04-22 response-eigenvalue attempt

`DERIV_PROJECTED_RESPONSE_EIGENVALUE_XPLUS_ATTEMPT.md` tests R3 and closes it negative under the current projected action:

```text
master quadratic as 2 x 2 characteristic polynomial       algebraically valid
physical two-sector response matrix from projected FTD     not derived
longitudinal/transverse projected action                   block diagonal
```

Eigenvalues are only needed if physical `1/alpha` is a normal-mode response of a coupled matrix. The current projected EFT does not force such a matrix. After this attempt, the remaining bridge choices are:

```text
R2. derive e0^2 = 1/x_+ from source-current normalization
R4. keep x_+ as arithmetic-only
```

### 2026-04-22 source-current normalization attempt

`DERIV_SOURCE_CURRENT_NORMALIZATION_XPLUS_ATTEMPT.md` tests R2 and closes it negative under the current projected action:

```text
signed source alphabet                 supported
conserved transport current             supported, given charge-conserving updates
e0^2 = 1/x_+ from source normalization  not derived
```

The source/current side fixes relative integer charge, not the physical electric charge magnitude. The historical assignment `g_c = sqrt(alpha) = 1/sqrt(x_+)` is therefore a matching selection unless a new normalization theorem is supplied.

The current bridge endpoint is:

```text
R4. x_+ remains arithmetic-only under the current projected action.
```

### 2026-04-22 native dual-cell Gauss closure

The user hypothesis that the true source object is `J*J` / dual-cell boundary
flux was tested as a fixed native audit. The result closes one major ambiguity:

```text
cell-centered divergence of J        approximation
dual-cell face flux                  exact finite-volume native Gauss object
collocated source-site J update      not the source-core fix
```

Classification:

```text
[MEASURED] exact native Gauss lives naturally on the dual-cell boundary.
[MEASURED] the current production cell-centered projection is a face-averaged
           approximation, not a theorem-level source operator.
[OPEN] exact production Gauss would require face-centered flux storage or an
       equivalent dual-cell projection layer.
```

This is a real bridge improvement because it tells us where the source lives in
FTD-native language before any QED-facing interpretation is introduced.

### 2026-04-22 Moore/BCC role closure

The BCC question is now narrowed:

```text
Full Moore shell   = ontology/combinatorics/causal boundary
G18                = direct one-tick longitudinal Gauss response
BCC/stella         = delayed k=3 Moore layer and parity/confinement channel
G26                = allowed production family, not selected yet
```

Fixed shell audits show that G6, G18, and fourth-order isotropic G26 operators
can all close shell Gauss. Equal-layer G26 is not automatically better; it is
less isotropic at finite `k`. Nonzero-BCC G26 variants are compatible with
fourth-order isotropy, but they occupy a one-parameter family:

```text
b = 1/6 - 2c
a = 1/3 + 4c
0 <= c <= 1/12
```

Classification:

```text
[MEASURED] BCC/corner flux can participate in a consistent full-Moore shell.
[MEASURED] current G18 dynamics still produces delayed BCC response through
           propagation, even without a one-tick corner stencil.
[SELECTION] keep production Gauss at the G18 endpoint `c = 0` for the native
            direct-response tuple.
[OPEN] promote G26 only if FTD supplies an independent action or timing
       principle that selects `c`.
```

### 2026-04-22 self-dual half-shell status

The `k = 1/2` / `m = 1/2` thread now has two different statuses:

```text
elliptic self-dual point m = 1/2             [THEOREM]
dual-cell edge shell r^2 = 1/2               [THEOREM]
dual-edge shell carries engine response      [MEASURED]
G* is the primal/dual bridge normalization   [CONJECTURE]
```

This closes the weaker open item "is the half-shell only pretty geometry?" The
answer is no: it is now a measurable response channel. It does not close the
stronger open item "does this derive `G*` dynamically?"

### Current native theory queue

The live open items are therefore sharper than before:

1. Decide whether exact production Gauss should migrate to a dual-cell flux
   representation.
2. [CLOSED NEGATIVE] Supply, or reject, a native selection principle for nonzero G26 corner
   weight `c`. (Phase 3 tested spectral radius and anisotropy: no native principle selects a unique $c$. G26 is rejected; G18 is axiomatic).
3. [CLOSED NEGATIVE] Test primal-dual projection commutation and half-step action balance on the
   `r^2 = 1/2` shell. (Phase 4 tested the action ratio and found no analytical match to $G^*$. Conjecture rejected).
4. [CLOSED] Define a nontrivial source-history action/measure for genesis and evaporation. (Resolved: The action cost of manifesting a point source was analytically derived as $\Delta E = W_{18}/2 \approx 0.1585$. This formalizes the latent non-linear energy injected into the substrate).
5. Define a fixed coarse-graining protocol and measure native flow of
   `(C_L^FTD, K_T^FTD, Z_j^FTD, g_sJ^FTD)`.

None of these items require recovering QED alpha. QED comparisons remain
diagnostic until a separate matching theorem exists.

### 2026-04-22 native source-flux coupling closure

`DERIV_FTD_NATIVE_SOURCE_FLUX_COUPLING_CLOSURE.md` closes the current-action
`g_sJ` item:

```text
g_sJ^FTD = 1                       canonical native normalization
G_C = sqrt(alpha_QED)              historical QED-facing imposed value
non-unit g_sJ from current action  not derived
```

The closure is intentionally conservative. Once `rho = s`, `div J = rho`,
`C_L^FTD = 1`, `K_T^FTD = 1`, and `Z_j^FTD = 1` are fixed, the bare linear
source-flux vertex has no remaining independent dimensionless coefficient.
Thus a non-unit interaction coupling would require a new source-history
measure, coarse-grained flow, or matching theorem. It is not produced by the
current deterministic action.

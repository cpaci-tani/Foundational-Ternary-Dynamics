# Specification: FTD-Native Electrodynamics

**Date:** 2026-04-22
**Status:** [SELECTION] program specification after QED-alpha bridge closure
**Purpose:** Replace the failed attempt to derive QED `alpha` with a native FTD source/flux response theory and its own observables.

---

## Executive pivot

The current projected-action audits close the attempted QED-alpha bridge:

```text
R1. x_+ as transverse stiffness          closed negative
R2. x_+ as source-current normalization  closed negative
R3. x_+ as response eigenvalue           closed negative
```

Therefore the next framework move should not be:

```text
derive QED and physical alpha from FTD.
```

It should be:

```text
define FTD-native electrodynamics,
derive its native observables,
then compare to QED only as an external correspondence problem.
```

This keeps the strong FTD content alive without forcing it through the wrong target.

---

## Native degrees of freedom

FTD's native field content is:

```text
s(x,t) in {-1,0,+1}     manifestation/source state
J_i(x,t) in R^3         physical vector flux
rho(x,t) = s(x,t)       signed source density
j_i(x,t)                signed transport current of s
```

The native decomposition is:

```text
J = J_L[rho] + J_T
div J_L = rho
div J_T = 0
```

This gives a source/flux theory directly. It does not require microscopic U(1) gauge ontology.

The auxiliary projected-potential description remains useful:

```text
J_T = P_T A
A ~ A + grad chi
```

but this is a redundancy of the projected description, not a primitive FTD gauge symmetry.

---

## Replacement dictionary

| Imported QED concept | FTD-native replacement | Status |
|---|---|---|
| electric charge | signed ternary source `s` | [THEOREM] as alphabet, [SELECTION] as EM analogy |
| electric current | signed transport current `j` | [DEFINITION], continuity requires update-rule audit |
| electric field | longitudinal flux response `J_L[rho]` | [THEOREM] given Gauss/source constraint |
| photon transverse modes | divergence-free flux modes `J_T` | [SELECTION] as photon-like modes |
| U(1) gauge potential | auxiliary representative `A` of `J_T` | [SELECTION] projection language |
| QED Ward identity | charge continuity plus transverse projection identity | [PARTIAL] |
| physical `alpha` | FTD-native source/flux response coefficients | [OPEN] native definitions |
| QED loop correction | native response renormalization under FTD dynamics | [OPEN] |
| CODATA match | external comparison only | [DIAGNOSTIC] |

The key policy change:

```text
FTD-native observables are primary.
QED observables are comparisons, not targets.
```

---

## Native observables

The native theory should define and measure these quantities.

### O1. Static source-flux response

Place two static signed sources and measure the long-distance response:

```text
rho = +delta_x - delta_y
div J = rho
```

Define the native Coulomb coefficient by the lattice Green function:

```text
V_FTD(k) ~ C_L^FTD / qhat_engine^2
```

where `qhat_engine` is the engine-native operator, not a BCC operator chosen for alpha closure.

Status:

```text
C_L^FTD is a native response coefficient.
C_L^FTD is not assumed to equal physical alpha.
```

### O2. Transverse flux stiffness and dispersion

For divergence-free modes:

```text
div J_T = 0
```

measure:

```text
omega^2(k) = c_FTD^2 qhat_engine^2 + higher lattice corrections
K_T^FTD = native transverse normalization
```

The current projected-action audit gives:

```text
K_T^FTD = 1
```

up to speed/stencil conventions.

### O3. State-current transport normalization

Define the source current by signed state transport:

```text
s(x,t) -> s(x + e_i,t+1)
j_i(x + e_i/2,t+1/2) += s(x,t)
```

Then audit all enabled update rules for:

```text
Delta_t rho + div j = 0
```

with balanced pair events treated as zero net charge.

This gives a native current normalization:

```text
Z_j^FTD = 1
```

in signed-source units unless a separate action measure changes it.

### O4. Manifestation/source-flux vertex

The historical coupling term is:

```text
L_int = -g_sJ s div J.
```

In the native program, rename it:

```text
g_sJ = FTD source-flux coupling
```

Do not call it the QED electric coupling.

Its status is:

```text
g_sJ^FTD = 1                        canonical native normalization
g_sJ = sqrt(alpha_QED)              historical matching selection / imposed
non-unit g_sJ from current action   closed negative
```

See `DERIV_FTD_NATIVE_SOURCE_FLUX_COUPLING_CLOSURE.md`.

### O5. Native scale flow

Instead of asking whether a flow lands on CODATA alpha, measure the FTD-native scale dependence:

```text
C_L^FTD(L)
K_T^FTD(L)
Z_j^FTD(L)
g_sJ^FTD(L)
```

under explicitly defined coarse-graining or dynamical ensembles.

This is allowed only as fixed verification of native definitions, not as a search for a near-miss.

---

## Role of the master quadratic

The master quadratic remains a real arithmetic result:

```text
x^2 - 16 G*^2 x + 16 G*^3 = 0
x_+ = 137.036171...
x_- = 3.024...
```

Under the native program, interpret it as:

```text
CM/arithmetic capacity structure of the framework.
```

Do not identify it with physical QED `1/alpha` unless a new matching theorem is supplied.

Allowed statements:

```text
x_+ is a robust arithmetic root.
x_+ is an FTD capacity candidate.
x_+ numerically matches 1/alpha_QED at about 1.26 ppm.
```

Disallowed statements under the current bridge:

```text
x_+ is derived physical 1/alpha_QED.
Structure-1 ppb correction is a scheme-independent QED prediction.
g_sJ = sqrt(alpha_QED) follows from ternary source normalization.
```

---

## Native acceptance tests

A native FTD electrodynamics claim should pass these tests:

1. **Engine operator fidelity:** use the actual engine stencil/operator unless a different operator is derived.
2. **Continuity audit:** show source-current conservation for the toggles used.
3. **Static response:** measure or derive `C_L^FTD` from the native source-flux response.
4. **Transverse response:** measure or derive `K_T^FTD` and dispersion from `J_T`.
5. **No CODATA targeting:** do not choose definitions by closeness to physical alpha.
6. **Comparison ledger:** any QED comparison is marked external/diagnostic.

---

## What replaces the old alpha goal

Old target:

```text
derive physical QED alpha from x_+.
```

New target:

```text
derive the native FTD response tuple

R_FTD = (C_L^FTD, K_T^FTD, Z_j^FTD, g_sJ^FTD, flow laws)

from FTD dynamics.
```

Then, separately:

```text
compare R_FTD to QED as an external correspondence problem.
```

This converts the project from a fragile alpha-matching exercise into a native theory-building program.

---

## Immediate next work

The first native computation should be fixed and engine-faithful:

```text
Compute C_L^FTD for the actual engine source/flux operator.
Compute K_T^FTD and the small-k transverse dispersion.
Audit continuity for the update toggles included.
Report all quantities in native FTD units.
Do not compare to CODATA in the classification.
```

This is the clean replacement for the failed QED-alpha bridge.

First-pass artifact:

```text
docs/theory/10_eft_program/DERIV_FTD_NATIVE_RESPONSE_AND_BLOCKING.md
scripts/exploration/ftd_native_electrodynamics.py
```

Current bare-engine result:

```text
C_L^FTD = 1
K_T^FTD = 1
Z_j^FTD = 1 for native signed movement current
c_FTD = 1/sqrt(3)
W_18 ~= 1.2679 in the engine Watson convention
g_sJ^FTD = 1 in canonical native source/flux units
```

The production engine still contains the historical QED-facing `G_C =
sqrt(alpha)` coupling in older Lagrangian/force paths. In the native program
that value is classified as an imposed correspondence normalization, not a
derived FTD coupling.

Continuity status:

```text
Delta_t rho + div j = 0
```

passes for movement transport and annihilation in
`engine/tests/test_native_continuity.cpp`. Full dynamics should be written as:

```text
Delta_t rho + div j = S_reaction
```

until genesis, evaporation, pair production, and weak transmutation have a
separate reaction/source ledger.

First reaction-ledger status:

```text
engine/tests/test_native_reaction_ledger.cpp
```

passes and classifies the current engine rules as:

| Event | `S_reaction` | Net signed charge |
|---|---:|---:|
| movement / bounce / annihilation-as-transport | 0 | conserved |
| evaporation | `-s` | changes |
| genesis | `+s` | changes |
| pair production | local `+1,-1` | conserved globally |
| weak transmutation | `-2s` | changes |

Thus signed ternary state `s` is exactly transported by movement, but is not a
globally conserved charge for all enabled reaction rules unless a larger
dual-substrate/chirality ledger is supplied.

Weak-transmutation parent ledger:

```text
engine/tests/test_native_conserved_parent.cpp
```

passes and verifies that dual-substrate weak transmutation preserves:

```text
J = J_L + J_R
|J_L|^2 + |J_R|^2
|chi|
s * chi
```

while flipping:

```text
s -> -s
chi -> -chi
J_L <-> J_R
```

So weak transmutation is not signed-`s` conservation, but it is not an arbitrary
source either. It is a parity flip in the dual-substrate state space. The
remaining unclosed reaction channels are genesis and evaporation.

Manifestation-ledger status:

```text
engine/tests/test_native_manifestation_ledger.cpp
```

passes and verifies, in the isolated phase-write rule, that genesis and
evaporation leave the local field-side quantities unchanged:

```text
J = J_L + J_R
chi
|J_L|^2 + |J_R|^2 + |V_L|^2 + |V_R|^2
```

while creating or destroying:

```text
s
s * chi
```

Thus current FTD has a genuine manifestation/demanifestation gate. Genesis and
evaporation are not hidden-conservation transformations like weak
transmutation. With Gauss projection enabled, the engine subsequently adjusts
longitudinal flux to satisfy `div J = s`; that is a projection response to the
newly manifested source, not a local conservation exchange in the genesis rule
itself.

Full-tick source-response status:

```text
engine/tests/test_native_source_response.cpp
```

passes and verifies that, for neutral source pairs, Gauss projection changes the
flux field and reduces void-site source residuals while preserving the source
pattern:

```text
fixed pair:   rms_void 0.00798633 -> 0.00725446, flux_delta 0 -> 0.578942
genesis pair: rms_void 2.3959     -> 1.5929,     flux_delta 0 -> 50.3546
```

The current engine leaves particle sites untouched during projection, so the
verified native statement is:

```text
Gauss projection dresses manifested sources with longitudinal flux on void sites.
```

It is not yet an exact global theorem that every site satisfies `div J = s`.
That stronger claim depends on whether particle-site skipping is a deliberate
native rule or an implementation compromise.

Projection-convergence status:

```text
engine/tests/test_native_projection_convergence.cpp
```

passes on the fixed SOR ladder:

```text
0, 1, 2, 5, 10, 20, 40, 80, 160
```

For a fixed neutral source pair, the projection reaches a plateau by about
`20-40` iterations:

```text
void residual:
  rms 0.00798633 -> 0.00725446
  max 0.255500   -> 0.223070

particle residual at plateau:
  rms ~= 0.582926
```

Thus the current projection is stable, but it remains a void-site projection.
The particle-site residual is not driven to zero because the implementation
skips `s != 0` sites during the flux correction step. This must be classified as
one of:

```text
[SELECTION] manifested sites are source-core boundaries;
or
[OPEN] particle-site skipping is an implementation compromise.
```

Source-core fork status:

```text
engine/tests/test_native_source_core_fork.cpp
```

passes and compares the current skip-source rule against a fixed experimental
include-source rule:

```text
base:
  rms_void=0.00798633, rms_particle=1, particle_flux_delta=0

skip_source_sites:
  rms_void=0.00725446, rms_particle=0.582926, particle_flux_delta=0

include_source_sites:
  rms_void=0.00648854, rms_particle=0.582926, particle_flux_delta=0.0996438
```

The include-source fork improves neighboring void residuals but does not improve
the particle-site residual, because the present collocated divergence operator
samples neighboring fluxes. Updating the source site's own stored flux does not
repair `div J - s` at that site.

Therefore this fork is closed for the narrow question:

```text
simple include-source correction does not produce a full-site Gauss theorem.
```

Current specification:

```text
[SELECTION] keep manifested sites as source-core boundaries in the production
projection rule.

[OPEN] if exact full-site div J = s is required, derive a different source-core
operator, probably face-centered or dual-cell, rather than updating collocated
source flux storage.
```

Dual-cell Gauss status:

```text
engine/tests/test_native_dual_cell_gauss.cpp
```

passes and implements the `J*J`/dual-cell reading directly:

```text
s lives inside a cell
J_face lives on oriented cell faces
div_face(J) is net outward boundary flux
```

For the same neutral source pair:

```text
base_face_flux:
  rms_all=0.0234954
  rms_void=0.00798633
  rms_particle=1
  max_all=1

dual_cell_projected:
  rms_all=1.50109e-17
  rms_void=1.4608e-17
  rms_particle=1.57009e-16
  max_all=2.22045e-16
  flux_delta=0.719448
```

This resolves the source-core interpretation:

```text
[MEASURED] exact native Gauss is naturally dual-cell/face-centered.
[MEASURED] the current cell-centered divergence is a face-averaged
           approximation of that dual-cell operator.
[MEASURED] changing collocated source-site J is not the source-core fix.
```

Current production specification:

```text
[SELECTION] The production engine now supports Option B natively via the `exact_dual_gauss` toggle (default: false).
When enabled, the projection acts exactly as the face-averaged representation of the true dual-cell Gauss theorem, correcting all sites uniformly without arbitrary source-skipping. 
When disabled, the engine defaults to the legacy skip-source projection to preserve behavior for Phase G/H tests.
```

Moore-shell Gauss status:

```text
engine/tests/test_native_moore_shell_gauss.cpp
```

passes and compares fixed source-boundary operators:

```text
G6                = cubic face shell
G18               = face + edge shell with current engine isotropic weights
G26_equal_layer   = full Moore shell with equal-layer BCC/corner channel
G26_iso_mid       = nonzero-BCC midpoint of the fourth-order isotropic family
G26_iso_corner    = BCC endpoint of the fourth-order isotropic family
```

Raw summary:

```text
G6:
  finite-k symbol: axis=0.987215, face_diag=0.993591, body_diag=0.995724
  projected rms_all=1.14246e-17
  flux_delta=0.719448

G18:
  finite-k symbol: axis=0.987215, face_diag=0.987248, body_diag=0.987229
  projected rms_all=1.46522e-17
  flux_delta=1.67768

G26_equal_layer:
  finite-k symbol: axis=0.987215, face_diag=0.984076, body_diag=0.983055
  projected rms_all=1.84394e-17
  flux_delta=2.22545
  corner_delta=1.30299

G26_iso_mid:
  finite-k symbol: axis=0.987215, face_diag=0.987248, body_diag=0.987266
  projected rms_all=1.96534e-17
  flux_delta=2.02379
  corner_delta=1.20469

G26_iso_corner:
  finite-k symbol: axis=0.987215, face_diag=0.987248, body_diag=0.987302
  projected rms_all=1.49297e-17
  flux_delta=1.99100
  corner_delta=1.18797
```

Interpretation:

```text
All consistent shell operators close Gauss exactly.
Equal-layer G26 is less isotropic at finite k.
Fourth-order isotropic G26 variants preserve the G18 finite-k isotropy class.
BCC is the 8-corner subboundary of the Moore shell, not the whole shell.
The G26 audits prove that this BCC/corner channel can carry real boundary flux.
```

A full isotropic Moore operator has a one-parameter weight family. With face
weight `a`, edge weight `b`, and corner weight `c`:

```text
a + 4b + 4c = 1
6b + 12c = 1
```

or:

```text
b = 1/6 - 2c
a = 1/3 + 4c
0 <= c <= 1/12
```

The current engine G18 operator is the `c=0` endpoint. The entire 1-parameter 
family is exact mathematically at $O(k^4)$ spatial isotropy.

```text
G26_iso_mid:    c = 1/24, a = 1/2, b = 1/12
G26_iso_corner: c = 1/12, a = 2/3, b = 0
```

[MEASURED] The finite-$k$ high-frequency anisotropy variance minimizes at an arbitrary irrational point ($c \approx 0.002$), and the spectral radius is flat for $c \le 1/48$. There is **no native FTD mathematical principle** that uniquely selects a nonzero corner weight $c$.
[SELECTION] By strict requirement of minimal necessary structure (Occam's razor), we reject the arbitrary G26 parameter family and canonize **G18** ($c=0$) as the unique axiomatic projection operator.
Temporal Moore-layer selection:

```text
[SELECTION] Production native Gauss keeps the direct G18 endpoint `c = 0`.
[SELECTION] The BCC/stella shell is a delayed k=3 Moore layer, not a mandatory
            instantaneous Gauss-stencil term.
[OPEN] Promote G26 to production only if an independent FTD timing/action
       principle uniquely selects `c`.
```

Reason: by the Moore Layer Theorem, at `c_FTD = 1/sqrt(3)` the layer travel
time is `t_k = sqrt(k) * sqrt(3)` ticks, so the BCC layer (`k = 3`) is reached
after three ticks. That makes BCC natural as a delayed/confining channel while
leaving G18 as the direct one-tick longitudinal response operator.

Fixed engine audit:

```text
engine/tests/test_native_moore_layer_coupling.cpp
engine/tests/test_native_moore_temporal_layers.cpp
```

checks the current G18 engine directly and treats the older broad
`test_intervoxel_coupling.cpp` as a legacy diagnostic rather than the native
status test. The native audit asserts only:

```text
SC flux > FCC flux > BCC flux > 0
BCC response is present but subdominant
divergence remains source-centered
```

Measured output:

```text
center:           mean |J| = 0.005487999777, mean |div J| = 0.09385030829
SC face shell:    mean |J| = 0.03237848757,  mean |div J| = 0.009576494073
FCC edge shell:   mean |J| = 0.008260217877, mean |div J| = 0.004644851381
BCC corner shell: mean |J| = 0.005620270007, mean |div J| = 0.001373936088
FCC/SC = 0.2551143829
BCC/SC = 0.1735803748
```

Temporal audit output for a pure-wave center impulse:

```text
tick  center          SC              FCC             BCC
0     1.00000000e+00  0.00000000e+00  0.00000000e+00  0.00000000e+00
1     3.33333333e-01  1.11111111e-01  5.55555556e-02  0.00000000e+00
2     1.11111111e+00  8.64197531e-02  5.55555556e-02  3.70370370e-02
3     3.12757202e-01  1.19341564e-01  3.34362140e-02  6.37860082e-02
```

This refines the claim: BCC is absent from the one-tick direct G18 stencil, but
not absent from dynamics. It appears at tick 2 through propagation paths. The
Moore theorem's `t_BCC = 3` statement is retained as the CFL geometric
light-time for the `k=3` shell, not reinterpreted as a first-nonzero stencil
coefficient.

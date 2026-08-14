# FTD-0894 — Bloch quasimomentum lift and local momentum-map trilemma v1

**Identifier:** `FTD-0894`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Date:** 2026-08-11  
**Production status:** unchanged

## 1. Question

FTD-0893 proved that dressed inertia requires an independently defined
additive total-momentum map. Existing native work supplies three partial
objects: exact transport of a supplied real momentum, exact local conserved
pseudomomenta, and Bloch labels for free flux. Can any one of these already be
the globally real additive momentum map required by FTD-0893?

The registered answer is a conditional theorem and a trilemma. Integer
translation gives torus-valued quasimomentum naturally. A globally real lift
requires branch/winding information or a nonlocal generator. No strictly
finite-range, translation-invariant spectral generator can equal the
unwrapped Bloch coordinate globally.

This theorem concerns the translation-spectral route only. It does not rule
out a new local substrate stress state with its own exact exchange law.

## 2. Frozen sources

| source | SHA256 |
|---|---|
| `THEOREM_DRESSED_BOOST_MOMENTUM_MAP_AND_INERTIAL_IDENTIFIABILITY_BOUNDARY_v1.md` | `378E38227422336BF9956EA6668CA7C09006B3A1D226370577126944654F833C` |
| `AUDIT_MATCHED_FACE_MOMENTUM_TRANSACTION.md` | `C4A157B2D9114EC251E60F24B93C5580222B8EB937A3958322248779D2DC6687` |
| `AUDIT_EXACT_MOMENTUM_FACE_BALANCE.md` | `72364E30BC10216661E64FAC67B13810EE1CEB2903AF7C2A408337EA16615AAF` |
| `THEOREM_CONTINUOUS_TRANSLATION_LOCALITY_TRILEMMA.md` | `527BDA49C213C1D58862A8A6254FC153416253EA3159BD7B958F8E43B69630EC` |
| `THEOREM_INTEGER_TRANSLATION_BLOCH_TRANSPORT.md` | `F472E65AFD9EB1B97B2EA4A8CC5C613960006928752F5A87F50302974DC2E6FD` |
| `ANALYSIS_SPLINE_POYNTING_NOETHER_DEFECT_v1.md` | `2D63051782D1648F51FE9EA8A7B90FE9FF38827C119D9C8033A12953F5389DF5` |
| `AUDIT_TOTAL_MOMENTUM_STRESS_LEDGER_v1.md` | `A690E90412D397B30FF899CA1568E81E0CC496A16578F87A95D00495D69C19BE` |
| `integer_bloch_transport.h` | `AC535306938C34789AC90EAA539266DA1976A954E0A19CAE71BF4798921ED615` |
| `momentum_face_balance.h` | `B9F435FF75E7EE133A9393294E45B1C316E026472A0C93FCF457077BDE6A6567` |
| `momentum_transport_current.h` | `77318892EA9BED7CECEDB7A2DD533E0B62CB217D9F2505A19F45858F5B81AC4F` |

Any source-hash mismatch invalidates the certificate.

## 3. Registered translation theorem

For the local uncontained translation algebra modeled conditionally by
`Z^3`, every unitary character has the form

```text
chi_k(n) = exp(i k dot n),
k in T^3 = R^3/(2 pi Z^3).
```

The product of characters adds labels only modulo the reciprocal lattice:

```text
chi_k chi_l = chi_[k+l],
[k]+[l] = [k+l] in T^3.
```

This is exact quasimomentum additivity. It is not yet a real-valued physical
momentum law and it does not impose a finite-torus ontology.

## 4. No global continuous additive lift

Let `q:R^3 -> T^3` be the quotient map. There is no continuous group
homomorphism `s:T^3 -> R^3` satisfying `q o s = id`.

The proof is compactness. The continuous image `s(T^3)` would be a compact
subgroup of the additive group `R^3`. The only compact additive subgroup of
`R^3` is `{0}`: if it contained nonzero `v`, the unbounded set `{n v:n in Z}`
would be contained in it. Hence `s` is zero and cannot be a section.

Therefore a global real lift must abandon at least one of continuity,
single-valued branch-free dependence on the torus label, or exact additivity.

## 5. Finite-range spectral obstruction

A real translation-invariant quadratic lattice charge with finite interaction
range has a Bloch weight that is a finite trigonometric polynomial,

```text
f_R(k) = a_0 + sum_(r=1)^R [a_r cos(r k) + b_r sin(r k)].
```

It is continuous and `2 pi` periodic. Consequently it cannot equal the
unwrapped coordinate `k` on all of `R`, because periodicity would require
`f_R(k+2 pi)=f_R(k)` while the target changes by `2 pi`.

On the principal branch `-pi < k < pi`, the exact sawtooth series is

```text
k = 2 sum_(r=1)^infinity (-1)^(r+1) sin(r k)/r.
```

Every finite truncation is local and periodic but not globally exact. The
exact principal-branch generator has infinite real-space range and a branch
discontinuity at the zone edge.

## 6. Registered three-way price

The translation-spectral route cannot retain all three:

1. a globally real exactly additive momentum;
2. strict finite-range realization;
3. branch-free periodic dependence on Bloch quasimomentum.

The exact alternatives are:

- retain torus-valued quasimomentum, with addition modulo `2 pi`;
- select a branch and use a nonlocal unwrapped generator; or
- retain an integer winding/history triplet `w in Z^3` and define
  `k_tilde = k_principal + 2 pi w`.

The third option restores a real lift only when the winding update is itself
defined by the dynamics. Its conversion to physical momentum also needs an
independently fixed scale `p_*`:

```text
P_candidate = p_* k_tilde.
```

Neither the winding state nor `p_*` is selected or derived here.

## 7. Exact reference realization

The certificate and isolated EFT witness must implement:

```text
wrap(x) in [-pi,pi),
lift(k,w) = k + 2 pi w,
f_R(k) = 2 sum_(r=1)^R (-1)^(r+1) sin(r k)/r.
```

For two lifted labels `x1`, `x2`, addition must satisfy

```text
wrap(x1+x2) = k12,
x1+x2 = k12 + 2 pi w12
```

with an exact integer carry `w12`. Principal-value addition alone must be
shown to lose this carry at a zone crossing. The implementation is an
`[IMPOSED reference realization]`; it is not production momentum.

## 8. Certificate gates

The source-locked exact certificate must test:

- all ten source hashes and their scope markers;
- character multiplication and modulo-reciprocal-lattice additivity;
- the compact-subgroup proof of no continuous homomorphic section;
- periodicity of every finite trigonometric generator;
- the exact Fourier coefficients of the principal sawtooth;
- convergence at interior rational test angles without using a fitted search;
- failure at the branch edge and nonuniform/global exactness;
- exact wrap/lift/add/carry identities in one and three dimensions;
- ambiguity of momentum scale and of local spectral weighting;
- compatibility with the FTD-0893 conditional mass theorem;
- the distinction between transported supplied momentum and an originating
  physical momentum map;
- the invalid execution status of FTD-0769;
- terminal scope markers and a fail-closed aggregate verdict.

No numerical near-miss search, target fitting, or formula-substitution claim
is permitted.

## 9. Outcome map

- **Outcome A:** all exact gates pass. Book the Bloch-lift trilemma and retain
  the physical total-momentum map as open.
- **Outcome B:** a frozen source already supplies both an exact dynamically
  updated winding/stress state and an independently normalized physical scale.
  Identify that source before any promotion.
- **Outcome C:** an algebraic or source-scope gate fails. Book no theorem.
- **Execution invalid:** any hash, protocol, or terminal-gate failure.

## 10. Post-certificate implementation

Only after a passing locked certificate, add an isolated EFT analyzer for the
reference wrap/lift/carry construction. It must fail closed on nonfinite
inputs, nonpositive scales, nonprincipal labels, invalid truncation order, or
integer overflow, and expose these negative flags explicitly:

```text
WINDING_DYNAMICS_DERIVED=FALSE
PHYSICAL_MOMENTUM_SCALE_DERIVED=FALSE
TOTAL_FIELD_MATTER_MOMENTUM_MAP_DERIVED=FALSE
ABSOLUTE_MASS_DERIVED=FALSE
PRODUCTION_COUPLING=FALSE
BORN_TARGET_USED=FALSE
NATIVE_GSTAR_SYNCHRONIZATION=FALSE
```

No production `Voxel`, tick phase, default toggle, Born selector, or clock path
may change.

## 11. Next acceptance gate

Construct one of the following without target-coded normalization:

1. a local substrate stress/momentum state with an exact update, matter-field
   exchange law, and independently fixed impulse unit; or
2. a winding/history quasimomentum state whose update is generated by the
   native hop dynamics and whose scale follows from an independent action or
   impulse relation.

Then insert its linearization `B` into the FTD-0893 tensor and require the same
inertia from constrained energy curvature, impulse/velocity, and the complete
matter-field partition. Disagreement is a stop condition.

## 12. Scope firewall

```text
Z3_CHARACTER_DUAL=T3
QUASIMOMENTUM_ADDITION=EXACT_MODULO_RECIPROCAL_LATTICE
GLOBAL_CONTINUOUS_HOMOMORPHIC_T3_TO_R3_SECTION=IMPOSSIBLE
FINITE_RANGE_GLOBAL_UNWRAPPED_GENERATOR=IMPOSSIBLE
EXACT_PRINCIPAL_BRANCH_GENERATOR=INFINITE_RANGE_AND_BRANCH_DISCONTINUOUS
FINITE_TORUS_ONTOLOGY=NOT_ASSUMED
LOCAL_STRESS_ROUTE=NOT_RULED_OUT
WINDING_HISTORY_TYPE=OPEN_CANDIDATE_NOT_SELECTED
PHYSICAL_MOMENTUM_SCALE=OPEN
TOTAL_FIELD_MATTER_MOMENTUM_MAP=OPEN
ABSOLUTE_MASS_SCALE=NOT_DERIVED
PRODUCTION_INTEGRATION=FORBIDDEN
NO_NEW_SELECTED_VECTOR_TYPE=TRUE
GSTAR_BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```

The exact SHA256 of this protocol and its certificate must be entered in the
preregistration manifest before first execution.

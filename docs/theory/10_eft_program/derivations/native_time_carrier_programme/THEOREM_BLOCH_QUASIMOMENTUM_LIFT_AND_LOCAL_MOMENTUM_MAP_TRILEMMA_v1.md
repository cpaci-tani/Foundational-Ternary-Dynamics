# FTD-0894/0896 — Bloch-quasimomentum lift and local momentum-map trilemma

**Status:** `[THEOREM — EXACT QUASIMOMENTUM ADDITION MODULO RECIPROCAL LATTICE]` +
`[THEOREM — NO GLOBAL CONTINUOUS HOMOMORPHIC TORUS-TO-REAL SECTION]` +
`[THEOREM — FINITE-RANGE UNWRAPPED-GENERATOR OBSTRUCTION]` +
`[BOUNDARY — PHYSICAL TOTAL-MOMENTUM MAP OPEN]` +
`[IMPOSED — ISOLATED WRAP/LIFT/CARRY REFERENCE]`  
**Date:** 2026-08-11  
**Production status:** unchanged

## 1. Result

Integer lattice translation already supplies a coherent momentum-like object:
Bloch quasimomentum. Its natural value space is not `R^3`, however. It is the
character torus

```text
T^3 = R^3/(2 pi Z^3).
```

Quasimomenta add exactly modulo reciprocal-lattice vectors. To turn this
cyclic label into a globally real additive quantity, a theory must add a
branch history/winding state or accept a nonlocal generator. A strictly
finite-range translation-invariant spectral charge cannot equal the globally
unwrapped Bloch coordinate.

This identifies the missing information in the translation-spectral route to
the FTD-0893 momentum map. It does not yet provide physical momentum: the
winding update and the conversion scale remain open.

## 2. The character algebra

The ontology is not assumed to be a finite periodic box. The theorem uses the
conditional local translation algebra `Z^3` for finite-support or decaying
configurations in an uncontained cubic adjacency. Its unitary characters are

```text
chi_k(n) = exp(i k dot n).
```

Because `n` has integer components,

```text
chi_(k+2 pi m)(n) = chi_k(n),       m in Z^3.
```

The character label is therefore `[k] in T^3`. Character multiplication gives

```text
chi_k chi_l = chi_(k+l),
[k]+[l] = [k+l].                    (1)
```

Equation (1) is an exact additive law on the torus. It is the correct native
notion of free Bloch quasimomentum supplied by integer translation. Calling a
chosen principal representative a globally real momentum silently discards
reciprocal-lattice carries.

## 3. No continuous additive global section

Let

```text
q:R^3 -> T^3
```

be the quotient. Suppose a continuous group homomorphism
`s:T^3 -> R^3` satisfied `q o s=id`. The image `s(T^3)` would be a compact
subgroup of the additive group `R^3`. But any additive subgroup containing a
nonzero vector `v` contains every integer multiple `n v`, which is unbounded.
The only compact additive subgroup of `R^3` is consequently `{0}`. The zero
map cannot be a section. Contradiction.

Thus there is no continuous, single-valued, exactly additive, branch-free
global lift from Bloch quasimomentum to a real vector.

This statement does not say `Hom(Z^3,R^3)=0`; it is nontrivial. Such a
homomorphism maps integer displacement into a real vector and still has an
undetermined unit. The obstruction concerns a state label on the character
torus, not the abstract displacement group.

## 4. Finite-range spectral obstruction

Consider one axis. A translation-invariant quadratic charge with a real-space
kernel of finite range `R` has a finite trigonometric Bloch weight,

```text
f_R(k) = a_0 + sum_(r=1)^R
         [a_r cos(r k) + b_r sin(r k)].            (2)
```

Every such weight is continuous and `2 pi` periodic. It cannot equal the
unwrapped coordinate globally because

```text
f_R(k+2 pi)=f_R(k),       (k+2 pi)-k=2 pi.         (3)
```

On the principal branch, the unwrapped coordinate has the exact Fourier
series

```text
k = 2 sum_(r=1)^infinity
    (-1)^(r+1) sin(r k)/r,       -pi < k < pi.     (4)
```

The sine coefficient follows exactly from

```text
(1/pi) integral_(-pi)^pi k sin(r k) dk
  = 2 (-1)^(r+1)/r.
```

Every finite truncation of (4) is a legitimate finite-range periodic
observer, but it equals zero at both zone-edge representatives while the
one-sided unwrapped values approach `+-pi`. Exact recovery of the principal
coordinate therefore uses infinitely many lattice separations and retains a
branch discontinuity.

This explains why the exact local FTD-0473 pseudomomentum can be conserved yet
remain neither uniquely normalized nor identical to the global unwrapped
generator. Local finite-difference symbols such as `sin(k)` are periodic
spectral weightings, not the coordinate `k` itself.

## 5. The trilemma

Along the translation-spectral route, the following three requirements cannot
all hold:

1. globally real, exactly additive momentum;
2. strict finite-range realization;
3. branch-free periodic dependence on Bloch quasimomentum.

The exact choices are:

1. **Torus law:** retain `[k]` and add modulo reciprocal vectors.
2. **Nonlocal branch:** choose a principal branch and use the infinite-range
   generator (4).
3. **Winding lift:** retain `k in [-pi,pi)^3` together with
   `w in Z^3`, and define

   ```text
   k_tilde = k + 2 pi w.                           (5)
   ```

For two lifted labels, the principal sum produces an integer carry `c`:

```text
k_12 = wrap(k_1+k_2),
w_12 = w_1+w_2+c,
k_tilde_12 = k_tilde_1+k_tilde_2.                 (6)
```

Equation (6) is exact. It is not yet a dynamical law. A physical theory must
specify what local event updates `w`, where that retained history resides, and
how reciprocal-lattice momentum is exchanged with the substrate.

## 6. The remaining physical scale

Even after a real lift is supplied, physical momentum needs a conversion
scale:

```text
P_candidate = p_* k_tilde.                         (7)
```

Translation algebra does not fix `p_*`. Rescaling `p_*` leaves all character
phases and wrap/carry identities unchanged. In the FTD-0893 conditional mass
tensor, the same ambiguity appears as

```text
B -> s B,
M=B A^-1 B^T -> s^2 M.                            (8)
```

Consequently Bloch transport alone cannot derive absolute inertial mass.
An independent action, impulse, or exact matter-field exchange relation must
fix the unit.

## 7. Reconciliation with the existing momentum corpus

- FTD-0473 supplies an exact selected local staggered pseudomomentum, but its
  normalization and spectral weighting are not unique; its quiet
  electrostatic hop has no recoil in that channel.
- FTD-0514 supplies exact face continuity and kinetic stress for an already
  known input momentum. Exact balance after an impulse is not the origin of
  the impulse.
- FTD-0554 proves that exact continuous fractional translation has a nonlocal
  support cost.
- FTD-0556 supplies the free-flux Bloch band and continuous packet centroid,
  but no stable matter pole or total matter-field charge.
- FTD-0619 closes the natural spline-Poynting coupled-recoil candidate
  negative.
- FTD-0769 is execution-invalid and establishes neither closure nor
  non-closure of its moving-core total-momentum ledger.
- FTD-0893 remains conditional on an independently closed physical `B`.

These results are mutually consistent. They describe transport observers,
free spectral labels, or failed candidates—not the missing complete physical
momentum map.

## 8. What this theorem does not exclude

The finite-range obstruction applies to a momentum map obtained as a
translation-spectral weighting of existing fields. It does **not** prove that
all local momentum mechanics is impossible. A new local substrate stress,
bond-impulse state, or reaction reservoir could carry exact real momentum by
having its own update and exchange law. Such a state would be additional
dynamical content rather than a function of the Bloch label alone.

This distinction matters. Winding/history is the minimum missing type for the
spectral route; it is not proven to be the unique possible physical route.

## 9. Certificate and repair provenance

The locked FTD-0894 certificate first returned `75/81`. All substantive gates
passed; six expression/source-marker comparisons failed. The locked FTD-0895
repair returned `80/81`; its remaining C73 mismatch retained a Doxygen line
asterisk. The one-marker FTD-0896 repair then returned `81/81` without changing
the parent protocol, certificate, sources, equations, or scope.

The passing command is

```text
python scripts/proofs/proof_bloch_quasimomentum_lift_local_momentum_map_trilemma_v3.py
```

The verdict is

```text
BLOCH_QUASIMOMENTUM_LIFT_TRILEMMA_EXACT_PHYSICAL_MOMENTUM_MAP_OPEN=TRUE
```

## 10. Next acceptance gate

Choose one branch of work without target-coded normalization:

1. derive a local stress/bond-impulse state with exact update, matter-field
   exchange, and an independently fixed impulse unit; or
2. derive a winding update from native hop dynamics and fix `p_*` from an
   independent action or impulse relation.

Then linearize the resulting total charge to obtain `B` and demand one common
inertial tensor from constrained energy curvature, impulse divided by center
velocity, and the complete matter-field momentum partition. Disagreement is a
stop condition.

## 11. Scope firewall

```text
QUASIMOMENTUM_ADDITION=EXACT_MODULO_RECIPROCAL_LATTICE
GLOBAL_CONTINUOUS_HOMOMORPHIC_T3_TO_R3_SECTION=IMPOSSIBLE
FINITE_RANGE_GLOBAL_UNWRAPPED_GENERATOR=IMPOSSIBLE
EXACT_PRINCIPAL_BRANCH_GENERATOR=INFINITE_RANGE_AND_BRANCH_DISCONTINUOUS
FINITE_TORUS_ONTOLOGY=NOT_ASSUMED
LOCAL_STRESS_ROUTE=NOT_RULED_OUT
WINDING_HISTORY_TYPE=OPEN_CANDIDATE_NOT_SELECTED
WINDING_DYNAMICS=OPEN
PHYSICAL_MOMENTUM_SCALE=OPEN
TOTAL_FIELD_MATTER_MOMENTUM_MAP=OPEN
ABSOLUTE_MASS_SCALE=NOT_DERIVED
STABLE_MATTER_POLE=OPEN
PRODUCTION_INTEGRATION=FORBIDDEN
NO_NEW_SELECTED_VECTOR_TYPE=TRUE
GSTAR_BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```

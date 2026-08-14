# FTD-0935 — Preregistration: native bilateral C4 translation character and Moore-shell parity boundary v1

**Identifier:** `FTD-0935`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** exact representation-level coupling of the FTD-0907 native ternary
dipole/wedge data to the compact translation character required by FTD-0934;
classification of the minimum signed-cubic-equivariant integer-linear `C4`
gearbox; exact face/edge/corner Moore-shell parity; no numerical search, fit,
production-memory promotion, new ontology type, unwrapped momentum, physical
scale, reciprocal recoil, `G*` cadence, Born, Bell, context, outcome, or hiding
read

## 1. Question

FTD-0934 proves that the positive dressing square loses the conjugation sign
that distinguishes a translation from its reverse. It leaves open whether an
existing substrate observable can realize the missing character

\[
 \chi_k(d)=e^{ik\cdot d}.
\]

FTD-0907 already identifies two representation-level observables in existing
native types: an integer neutral-dipole displacement and a time-odd bilateral
phase-wedge sign. The present exact discriminator asks:

1. does their product form a presentation-independent time-odd polar lattice
   vector;
2. can that vector define a signed-cubic-covariant character of `Z^3` valued
   in the internal quarter-turn group `C4={1,i,-1,-i}`;
3. is the resulting gearbox unique within the minimum integer-linear
   signed-cubic-equivariant class; and
4. which Moore shells retain clockwise/counterclockwise information under the
   natural self-translation probe?

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `THEOREM_NATIVE_TERNARY_DIPOLE_AXIS_AND_BILATERAL_PHASE_WEDGE_MEMORY_BOUNDARY_v1.md` | `8B07C26475A76E79C37B825B91EA174C0D1D8C13F06422483EE60B236DC14340` |
| `THEOREM_BLOCH_QUASIMOMENTUM_LIFT_AND_LOCAL_MOMENTUM_MAP_TRILEMMA_v1.md` | `0C2F0C289C82D45457B5DF330F767C10AD5CA3966FB667B329391C283FD47973` |
| `THEOREM_C4_DRESSING_TRANSLATION_COCYCLE_AND_DIRECTED_RECOIL_STATE_NECESSITY_v1.md` | `4247301642D82587066F2294D7DA5ABF7699CC0DB06E43AA4E3733844E6312B9` |
| `proof_c4_dressing_translation_cocycle_directed_recoil_state_necessity.py` | `52C776DE265D8535C7CF0ABF531EC468802CA06FE71B40BC3D61EC963CAD3DD3` |

The certificate fails closed on source drift.

## 3. Frozen native directed lattice datum

For a distinct neutral ternary pair, let

\[
 a=x_+-x_-\in\mathbb Z^3\setminus\{0\},
 \qquad e={a\over |a|}.
\]

With the FTD-0907 endpoint projections

\[
 q_\pm=e\cdot J_\pm,
 \qquad p_\pm=e\cdot W_\pm,
\]

define

\[
 \ell=q_+p_--q_-p_+,
 \qquad \chi=\operatorname{sgn}(\ell),
 \qquad \boxed{p_4=\chi a\in\mathbb Z^3}.             \tag{1}
\]

The certificate must verify three exact transformations.

1. Under a signed cubic transformation `Q`, the projected endpoint values
   are scalars, so `ell` and `chi` are unchanged while
   `a -> Qa`; hence `p_4 -> Qp_4`.
2. Under canonical time reversal, `a` and the projected fluxes are unchanged,
   the projected wave velocities reverse, and therefore
   `ell -> -ell`, `chi -> -chi`, and `p_4 -> -p_4`.
3. Under reversal of the ordered endpoint presentation, recompute the axis:

   \[
    a\mapsto-a,
    \quad(q_+,p_+;q_-,p_-)
    \mapsto(-q_-,-p_-;-q_+,-p_+).
   \]

   Then `ell -> -ell`, `chi -> -chi`, and the product `p_4=chi a`
   is invariant.

Thus equation (1) is permitted to be called a presentation-independent,
time-odd polar integer vector on the nonzero-wedge domain. It is not permitted
to be called a protected production memory: FTD-0911/0913 already closed that
claim negative for the tested two-endpoint production class.

## 4. Frozen C4 translation character

Use the native integer pairing to define

\[
 \boxed{
 \Xi_{p_4}(d)=i^{p_4\cdot d}
 =\exp\!\left({i\pi\over2}p_4\cdot d\right),
 \qquad d\in\mathbb Z^3.}                            \tag{2}
\]

The certificate must prove

\[
 \Xi_{p_4}(d+f)=\Xi_{p_4}(d)\Xi_{p_4}(f),
 \qquad
 \Xi_{-p_4}(d)=\overline{\Xi_{p_4}(d)},              \tag{3}
\]

and signed-cubic covariance

\[
 \Xi_{Qp_4}(Qd)=\Xi_{p_4}(d).                        \tag{4}
\]

Equation (2) is a genuine compact translation character with Bloch label

\[
 [k]={\pi\over2}p_4\pmod{2\pi\mathbb Z^3}.           \tag{5}
\]

It uses no fitted continuous scale. It does not define a real unwrapped
momentum: only `p_4 mod 4` is visible to equation (2), and FTD-0896's lift,
carry, and physical conversion-scale boundary remains unchanged.

## 5. Frozen minimum-class classifier

Register the minimum gearbox class as integer-linear maps

\[
 F:\mathbb Z^3\to\mathbb Z^3
\]

that commute with every signed permutation matrix `Q` of the cubic group.
Commutation with the three coordinate sign flips must kill every off-diagonal
entry of `F`; commutation with coordinate permutations must make the three
diagonal entries equal. Therefore

\[
 \boxed{F=mI_3,\qquad m\in\mathbb Z.}                 \tag{6}
\]

Modulo four:

- `m=0` gives the trivial character;
- `m=2` is real-valued and satisfies `Xi_p=Xi_-p`, so it loses orientation;
- `m=1` and `m=3` are complex-conjugate conventions and are the only
  chirality-sensitive members.

Consequently equation (2), corresponding to `m=1`, is the unique minimum
chirality-sensitive gearbox in the registered class up to global complex
conjugation. This is a conditional classification, not a theorem that the
production tick dynamically chooses the class or maintains its input.

## 6. Frozen Moore-shell parity

For one Moore-neighbour dipole vector

\[
 a\in\{-1,0,+1\}^3\setminus\{0\},
 \qquad w=|a|^2\in\{1,2,3\},                          \tag{7}
\]

the natural self-translation probe is

\[
 \boxed{\Xi_{\chi a}(a)=i^{\chi w}.}                 \tag{8}
\]

The exact shell table is frozen as

| Moore shell | count | `w mod 4` | `Xi_(chi a)(a)` | chirality visible? |
|---|---:|---:|---|---|
| face / SC | 6 | 1 | `i^chi` | yes |
| edge / FCC | 12 | 2 | `-1` | no |
| corner / BCC | 8 | 3 | `-i^chi` | yes |

Thus the even-coordinate edge shell is self-conjugate under this `C4`
self-probe, whereas the odd-norm face and BCC-corner shells retain the
clockwise/counterclockwise sign. Equivalently, the self-probe distinguishes
`chi` exactly when `|a|^2` is odd.

This is a mod-four lattice-pairing theorem. It is not permission to identify
Moore shells with split or inert Gaussian primes, nor does it derive the CM
Euler product or `G*` from the shell table.

## 7. Frozen symmetric-square reconciliation

For every `C4` character value,

\[
 |1-\Xi_{p_4}(d)|^2
 =2[1-\operatorname{Re}\Xi_{p_4}(d)]                 \tag{9}
\]

is invariant under `p_4 -> -p_4`, while
`Im Xi_(p_4)(d)` reverses. Equation (9) is the finite `C4` version of the
FTD-0934 dressing-square information loss. The bilateral wedge supplies the
missing conjugation sheet only on snapshots where `ell != 0`.

## 8. Registered outcomes

- **Outcome A — exact native-data compact-character gearbox:** equations
  (1)--(9) pass. Existing ternary position plus flux/wave-velocity data can
  represent the missing compact `C4` translation character without a new
  selected type. The minimum integer-linear cubic-equivariant gearbox is
  unique up to conjugation. Face and BCC-corner self-probes retain chirality;
  the FCC-edge self-probe is real and orientation blind. Production
  protection, physical momentum, reciprocal recoil, and the `G*` calendar
  coupling remain open.
- **Outcome B — representation without minimum classification:** equations
  (1)--(5) pass but equation (6) or the mod-four classifier fails. No
  uniqueness or minimum claim is licensed.
- **Outcome C — no native-data character realization:** the presentation,
  parity, homomorphism, or covariance tests fail.
- **Invalid:** source drift, post-lock formula change, numerical search, fit,
  production-memory revival, physical momentum or recoil promotion, new type
  adoption, engine/CMake mutation, Gaussian-prime/CM promotion, context/Born
  read, or completed-infinity rhetoric.

## 9. Firewalls and next gate

No engine source, CMake target, `Voxel` field, toggle, default, production
law, ontology type, import, physical constant, phenomenological formula, Born
weight, Bell correlation, measurement context, or `G*` cadence is changed.

Even Outcome A does not derive formation or protection of `a` and `ell`, an
unwrapped momentum, reciprocal-lattice winding, `p_*`, `gamma`, a common
source-field action, vector recoil, autonomous hopping, source formation,
recovery, Lorentz hiding, or framework completeness.

The next admissible gate is dynamic: either derive a protected version of
`p_4` from the later compact `C4` body/current construction, or couple the
compact character to a common local action whose source and field impulses
close equal-and-oppositely without reading a target phase or physical scale.

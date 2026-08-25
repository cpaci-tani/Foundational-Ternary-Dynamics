# C18 equivariant single-record collision no-go v1

**Date:** 2026-08-23  
**Status:** **[THEOREM — EXACT CUBIC CENTRALIZER]** +
**[CLOSED NEGATIVE — FIXED ONE-RECORD INTERACTION]** +
**[OPEN — STATE-DEPENDENT MULTI-RECORD COLLISION OR DYNAMICAL CONTROLLER]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_c18_equivariant_single_record_collision_no_go.py](../../../../../scripts/proofs/proof_c18_equivariant_single_record_collision_no_go.py)
enumerates the signed-permutation realization of (O_h), constructs every
equivariant permutation on the directed SC and FCC shells, and verifies the
streamed two-tick dynamics. The certificate performs no fit and imports no
physical constant.

---

## 1. Question

The
[uniform-token bare-blocking theorem](THEOREM_C18_UNIFORM_TOKEN_BARE_BLOCKING_v1.md)
supplies a finite carrier and a target-free real quadratic block response, but
its microscopic dynamics is only one-hop streaming. The smallest possible
interacting extension would be a fixed local collision acting independently
on each occupied directed record.

Let the directed C18 carrier be the disjoint union

\[
 D_{18}=D_{\rm SC}\sqcup D_{\rm FCC},               \tag{1}
\]

where

\[
 D_{\rm SC}=\{\pm e_x,\pm e_y,\pm e_z\},            \tag{2}
\]

and (D_{\rm FCC}) consists of the twelve signed face-diagonal directions
with exactly two nonzero coordinates. Ask whether a fixed permutation

\[
 C:D_{18}\longrightarrow D_{18}                    \tag{3}
\]

can be nontrivial while commuting with the full cubic action:

\[
 C(gd)=gC(d)
 \quad\text{for every }g\in O_h, d\in D_{18}.       \tag{4}
\]

This is deliberately the weakest collision ansatz: (C) sees the direction
of one record but no neighboring occupancy, charge relation, phase relation,
capacity, or local controller state.

---

## 2. Exact centralizer

The SC and FCC direction sets are inequivalent (O_h) orbits of cardinality
six and twelve. An equivariant bijection preserves each orbit. On either
orbit, an equivariant map is fixed by the image of one representative, and
that image must be fixed by the representative's stabilizer.

The exact stabilizer test leaves only two images on each shell:

\[
 d\longmapsto d,
 \qquad
 d\longmapsto-d.                                   \tag{5}
\]

Consequently the shell-preserving centralizer is exactly

\[
 \boxed{
 \operatorname{Cent}_{\operatorname{Sym}(D_{18})}(O_h)
 \cong C_2^{\rm SC}\times C_2^{\rm FCC}.}           \tag{6}
\]

There are four full C18 maps: identity or antipodal reversal may be selected
independently on the SC and FCC shells. The certificate checks equation (4)
for all 48 signed permutation matrices and all constructed maps.

---

## 3. Streaming classification

Compose (C) with one-hop streaming. For a record at position (x) with
direction (d), one tick is

\[
 (x,d)\longmapsto(x+C(d),C(d)).                     \tag{7}
\]

If (C(d)=d), then

\[
 (x,d)\longmapsto(x+d,d)\longmapsto(x+2d,d),        \tag{8}
\]

so every record remains an independent ballistic ray.

If (C(d)=-d), then

\[
 (x,d)\longmapsto(x-d,-d)\longmapsto(x,d),          \tag{9}
\]

so the record executes an exact two-tick spatial bounce.

A fixed shellwise C4 phase translation may be appended, but it does not
change equations (8)--(9); its phase closes after four applications. Thus a
fixed phase advance alone cannot turn the one-record rule into scattering.

---

## 4. No-go statement

**Theorem.** A fixed, reversible, cubic-equivariant collision permutation on
one directed C18 record has only ballistic or two-tick-bounce spatial
dynamics. It cannot mix rays, redistribute momentum among records, generate
an occupancy-dependent current, bind a localized object, or create a
derivative tensor response.

This is a no-go for the ansatz, not for the C18 carrier. It does not exclude:

- collisions conditional on two or more simultaneously present records;
- reversible gates controlled by relative C4 phase, ternary endpoint charge,
  capacity, or backpressure;
- a local frame carried by matter rather than supplied externally; or
- longer-range finite collisions assembled from local reversible gates.

Those alternatives act on a larger state space than equation (3).

---

## 5. Consequence for the one-action program

The interacting microscopic action cannot be obtained by assigning each
token a context-free turn rule. The minimum next search space is a
payload-complete local permutation on a **joint occupancy sector**. Such a
collision must expose its conserved quantities and inverse before its blocked
kernel is inspected.

The next exact gate is therefore:

1. classify two-record C18 sectors by shell content, total directed momentum,
   C4 phase relation, ternary charge, and capacity;
2. find which sectors contain more than one cubic-related microstate and can
   therefore scatter reversibly;
3. determine whether a nontrivial deterministic selection is itself
   (O_h\)-equivariant or requires a dynamical local controller; and
4. only then derive the linearized two-point block kernel.

The first two-record census is now available in the
[phase-complete scattering theorem and axial-routing boundary](THEOREM_C18_TWO_RECORD_PHASE_COMPLETE_SCATTERING_AND_AXIAL_ROUTING_BOUNDARY_v1.md).
It finds eighteen FCC momentum doubletons. Twelve admit a unique
maximum-dot phase-payload route, while six axial doubletons obstruct routing
of unequal spatial-scalar phases. The resulting minimum reference collision is
a finite reversible interacting permutation; its blocked physical kernel
remains open.

No Maxwell, gravity, Born, matter, or native-(\alpha) claim is promoted by
this theorem. It removes the smallest noninteracting ansatz and fixes the
minimum structural price of the next one.

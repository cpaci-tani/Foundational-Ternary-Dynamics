# Preregistration — C18 charge-even constraint bundle and axial-context price v1

**Date frozen:** 2026-08-24  
**Campaign status:** pre-execution lock  
**Ledger status:** no FTD identifier or claim row reserved  
**Production status:** no engine mutation authorized

## 1. Question

Can the spare C18 \(T_{1u}\) vector identified by the blocked constraint-seam
theorem be loaded by an explicit phase-complete finite ownership permutation
that:

1. leaves the electromagnetic vector moment unchanged;
2. is charge even;
3. retains every record and its C4 payload;
4. is signed-cubic covariant; and
5. realizes the local divergence of the manifestation STF source?

The discriminator must separately decide transverse incidence
\(q\perp r\) and axial incidence \(q\parallel r\). It must not add a hidden
transverse plane, duplicate a line record, or average over polarizations
without pricing the required ownership.

## 2. Frozen source chain

The exact certificate may import or compare only:

1. proof_c18_two_record_momentum_sector_census.py, SHA-256
   1EC3D1E23B5264B8CF422C376F222B39166B86CF07F473306CF4D8F2199A0A82;
2. proof_c18_equivariant_single_record_collision_no_go.py, SHA-256
   F759EE742070EF3EBF31E0268D89DE2E1B6A9C5877A1977A4947BDE0BEEC76E3;
3. proof_c18_scalar_stf_vector_constraint_absorption_seam.py, SHA-256
   29C6BC475F6DDFCC3FC73DA5683D0F14ECAC96233924084920F682389D1B1F6E;
4. proof_c18_phase_neutral_shared_charge_stress_vertex.py, SHA-256
   13334840F23DBB1D70EFD59B805D97E462EDFC5B4EEC00D5C9FFF784ECEAAF35;
5. proof_c4_phase_parity_half_admitted_two_polarization_carrier.py, SHA-256
   743CE826C259905DEF31CE1F2324EE8C5DB6EF8E04A8AE9B94227833D31F6000;
   and
6. this preregistration's pre-execution SHA-256.

No numerical search, tolerance, target coupling, deflection angle, master
root, or empirical value may enter.

## 3. Frozen shell-copy moments

For signed odd line weights, define

\[
 J_{\rm EM}=J_{\rm SC}+J_{\rm FCC},
 \qquad
 J_{\rm C}=J_{\rm SC}-J_{\rm FCC}.                       \tag{P1}
\]

Let \(r,n\) be orthogonal signed SC axes. Freeze one oriented plane bundle
\({\cal B}(r,n)\) as the following ownership exchange:

- one active SC record on the \(r\)-line changes orientation
  \(-r\mapsto+r\);
- two complete reserve FCC records become active on directions
  \(-(r+n)\) and \(-(r-n)\);
- their identifiers, common C4 phase, source route, and inverse ownership are
  retained.

The inverse returns the two FCC records to reserve and restores the SC
orientation.

The required moment increment is

\[
 \Delta J_{\rm SC}=2r,\qquad
 \Delta J_{\rm FCC}=-2r,                                \tag{P2}
\]

so

\[
 \boxed{\Delta J_{\rm EM}=0,\qquad
 \Delta J_{\rm C}=4r.}                                  \tag{P3}
\]

The family must obey

\[
 g{\cal B}(r,n)={\cal B}(gr,gn)
\qquad(g\in O_h).                                       \tag{P4}
\]

Its geometry is independent of internal charge orientation, so charge
conjugation must preserve equation (P3). Global C4 advance must rotate every
retained payload without changing the spatial moment ledger.

## 4. Frozen relation to the STF divergence

For the normalized event source

\[
 T_r={1\over18}\left(rr^{\mathsf T}-{\mathbf1\over3}\right), \tag{P5}
\]

define

\[
 v(r,q)=3(r\cdot q)r-q.                                 \tag{P6}
\]

Then

\[
 \boxed{216\,T_rq=4v(r,q).}                             \tag{P7}
\]

For a nearest-neighbor derivative direction \(q\):

- if \(q\perp r\), then \(v=-q\), and one bundle
  \({\cal B}(-q,r)\) must realize equation (P7);
- if \(q=r\), then \(v=2r\), so equation (P7) requires
  \(8r\), exactly two \(4r\) bundle increments;
- if \(q=-r\), it requires \(-8r\), again two bundles.

The factor 216 is a blocked chart normalization forced by the registered
\(1/18\) tensor moment and the finite \(4r\) bundle step. It is not a gravity
coupling.

## 5. Exact acceptance gates

### G1 — integrity and blindness

- Every frozen hash matches.
- Every operation is exact finite, integer, or rational.
- No target physical coefficient or continuum observable enters.

### G2 — finite ownership permutation

- Verify equations (P2)--(P3) for every ordered orthogonal pair \((r,n)\).
- Prove the forward and inverse bundle maps are mutual inverses.
- Prove all payload identifiers and C4 phases are retained.
- Prove a missing reserve record, occupied target channel, or inconsistent
  phase fails before mutation.
- Prove disjoint bundles commute and overlapping bundles are rejected
  atomically rather than double spending one line owner.

### G3 — cubic and internal symmetries

- Verify equation (P4) for all 48 signed-cubic transformations.
- Verify charge conjugation leaves \(\Delta J_{\rm C}\) unchanged.
- Verify global C4 phase advance commutes with the bundle map.

### G4 — transverse source realization

- Verify equation (P7) for all six source rays and all six derivative axes.
- For every \(q\perp r\), prove one plane bundle realizes the exact normalized
  divergence load while preserving \(J_{\rm EM}\).

### G5 — axial stabilizer obstruction

For \(q=\pm r\):

- prove one bundle has only half the required \(\Delta J_{\rm C}\);
- compute the signed-cubic stabilizer of the ordered axial data \((r,q)\);
- prove it acts transitively on the two unoriented transverse plane choices;
- prove no context-free equivariant selector chooses one plane;
- prove a C4 scalar phase does not repair the spatial stabilizer obstruction;
  and
- prove the invariant paired repair is the sum of both plane bundles.

### G6 — ownership price of the paired repair

The paired repair must require two independently owned SC records on the same
\(r\)-line, or an explicitly distributed/time-shared equivalent with retained
history. The current one-record-per-directed-channel C18 slice cannot execute
both bundles simultaneously at one site.

The certificate may identify the two physical Maxwell polarization contexts
as a possible source of the two planes, but must not call that a repair unless
independent constraint ownership is actually paid and the result is
polarization independent.

## 6. Outcome classes

- **Outcome A:** one existing C18 record slice realizes both transverse and
  axial source loads equivariantly, with no extra owner or context.
- **Outcome B:** transverse loads have an exact finite bundle, while axial
  loads require a second record, distributed support, or explicit plane
  context.
- **Outcome C:** even transverse incidence requires a new spatial type.
- **Outcome D:** integrity, inverse, or exact moment gates fail.

The expected honest ceiling is Outcome B.

## 7. Interpretation firewall

A pass may establish:

- an explicit charge-even, EM-neutral, phase-complete finite constraint
  bundle for transverse incidence;
- the exact integer normalization connecting it to the STF divergence; and
- the minimum axial ownership/context price.

A pass may not claim:

- the full divergence constraint is realized at one site by the current C18
  slice;
- a second record or time-sharing schedule is already generated;
- scalar/vector constraint dynamics or a static pole exists;
- native spin-2 propagation or lensing is closed;
- equal gravity coupling is derived;
- the physical Born pushforward is closed; or
- a native fine-structure measurement exists.

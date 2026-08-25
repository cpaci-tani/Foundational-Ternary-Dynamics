# V3 charged-candidate measure and pole boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — MULTIPLE INVARIANT CYCLE MEASURES]** +
**[THEOREM — NO ISOLATED-CYCLE COULOMB POLE]** +
**[THEOREM — ACTION NORMALIZATION REMAINS FREE]** +
**[THEOREM — CYCLE-LABEL BORN TRIAL WEIGHTS REMAIN UNSELECTED]** +
**[OPEN — LARGE-NETWORK MIXING, NONINJECTIVE RENEWAL, AND BLOCKED STATIC
POLE]**  
**Scope:** the isolated circulation-frame sector of the candidate charged Phi-v3  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Candidate law:**
[`SPEC_V3_CHARGED_COMMON_ACTION_PHI_v3_CANDIDATE.md`](../../../01_reference/SPEC_V3_CHARGED_COMMON_ACTION_PHI_v3_CANDIDATE.md)  
**Exact certificate:**
[`proof_v3_charged_candidate_measure_pole_boundary.py`](../../../../../scripts/proofs/proof_v3_charged_candidate_measure_pole_boundary.py)

---

## 1. Exact cycle decomposition

Fix a plaquette origin, plane family, and polarity. The circulation-frame
offset advances by

\[
 r\longmapsto r+1\pmod4.
\]

The spatial labels and polarity do not change. Hence every fixed label supports
one four-cycle. Already at one origin there are six disjoint cycles:

\[
 3\ \text{plane families}\times2\ \text{polarities}.
\]

The uniform measure on each individual cycle is invariant. These measures
have disjoint supports. Therefore

> **invariance of the candidate transition map does not select one physical
> history measure.**

Each isolated cycle spends two ticks on each of the two equal-boundary paths,
so a *prepared* frame has the local time average

\[
 \mu(P_A)=\mu(P_B)=\frac12.
\]

This ratio is a theorem of that prepared orbit. It is not a measure over
origins, endpoint networks, source preparations, or detector outcomes.

---

## 2. Laurent-regularity obstruction

The electric cochain of an isolated four-cycle has finite spatial support. Its
Fourier transform is therefore a finite Laurent polynomial

\[
 P(z_x,z_y,z_z).
\]

The cubic lattice Laplacian symbol may be written

\[
 \Lambda(z)=6-\sum_{j=1}^{3}(z_j+z_j^{-1}),
\]

and obeys

\[
 \Lambda(1,1,1)=0.
\]

For every Laurent polynomial regular at `(1,1,1)`,

\[
 [\Lambda P](1,1,1)=0.
\]

It therefore cannot satisfy

\[
 \Lambda P=1,
\]

which is the defining inverse relation for the lattice Coulomb Green function.
Equivalently, a finite-support time average is regular at zero momentum, while
`1/Lambda` has the required massless static pole.

Thus:

> **No isolated finite circulation orbit of the candidate law is the charged
> Coulomb pole.**

This is not a no-go for large finite networks, thermodynamic blocking, or all
finite local laws. A pole can arise from unbounded correlation length in a
controlled family of finite regions. The theorem says the present four-cycle
does not supply that mechanism by itself.

---

## 3. Normalization remains free

Assign a positive price `w` to each plaquette flip. The four-cycle history has

\[
 S_4=4w.
\]

Every `w>0` yields the same state map, the same cycle decomposition, the same
prepared path ratio, and the same Gauss identities. Concrete prices
`w in {1,2,7}` already give three distinct action normalizations with no change
to `Phi`.

Consequently neither the deterministic schedule nor its invariant cycle
measures determine the electromagnetic curvature `chi_EM`.

---

## 4. Consequence for the shared five-sector program

The candidate charged Phi has advanced the kinematics:

- source/current/Gauss records compose in parallel;
- endpoints can move by path extension and withdrawal;
- plaquette deformations have an autonomous conflict-free schedule; and
- the transverse vacuum remains unchanged.

But the next shared object is still missing. To recover a charged pole and a
physical coupling, the same law must generate at least one of:

1. a target-independent noninjective renewal process with a unique attracting
   history measure;
2. a proved transitive/ergodic large-network sector with a preparation theorem;
3. a blocked large-deviation functional whose Hessian has both the transverse
   vacuum residue and the static `1/Lambda` source residue; or
4. a newly priced finite type that supplies the missing selection, declared as
   an adoption rather than a derivation.

The same choice will constrain stable matter and Born statistics. Adding a
separate Coulomb weight would violate the common-action gate.

---

## 5. Born-trial boundary

Along each deterministic four-cycle, the plane-family and polarity label is
constant. Its empirical label frequency is therefore a delta function. At
the same time, every convex combination of the six disjoint uniform cycle
measures is invariant. The certificate explicitly verifies a nonuniform
mixture with six distinct rational weights.

Consequently the candidate transition supplies neither inter-cycle renewal
nor a preferred trial ensemble. Any family/polarity frequency assigned across
these cycles is a preparation weight. This does not invalidate the exact
prepared bright-pair identity $M=|Z|^2$; it proves that the isolated charged
sector does not yet generate the physical trials to which that identity would
apply.

---

## 6. Exact boundary

Closed:

- four-cycle decomposition;
- multiple disjoint invariant measures;
- prepared local `1/2:1/2` path time average;
- Laurent regularity of every isolated-cycle field average;
- persistence of the positive action-scale orbit;
- delta-valued empirical labels on each isolated orbit; and
- invariance of arbitrary convex mixtures across the six cycle supports.

Open:

- formation of a large charged network from admissible seeds;
- a unique physical preparation or renewal measure;
- correlation length growing with finite-region size;
- the charged massless static pole;
- reciprocal source/free-field work;
- `chi_EM` and physical coupling normalization;
- the use of the same measure for stable matter and Born trials; and
- tensor/gravity curvature of the same history object.

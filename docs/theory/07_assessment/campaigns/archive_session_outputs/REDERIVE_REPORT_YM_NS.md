# RE-DERIVE Report: Yang-Mills Mass Gap and Navier-Stokes Papers

**Date:** 2026-04-19
**Author:** Per-paper triage agent (RE-DERIVE phase)
**Inputs:** `CANONICAL_REFRAME.md` v1.0; `AUDIT_INFINITY_REFRAME.md`
**Subjects:**
- `docs/papers/speculative/FTD_Yang_Mills_Mass_Gap.tex`
- `docs/papers/speculative/FTD_Navier_Stokes.tex`

**Status:** Assessment only. No paper has been rewritten. Owner sign-off
required before any of the recommended actions are executed.

---

## Reading conventions used in this report

- "Clay-as-stated": the Clay Mathematics Institute problem, taken at
  face value as a question about a continuum theory on R^4 (Yang-Mills)
  or R^3 (Navier-Stokes).
- "Lattice-first": the FTD position that the discrete substrate is the
  fundamental theory and the continuum is an approximation.
- "Completed-infinity move": any reasoning step that treats Z^3, R^3,
  R^4, or "the L -> infinity limit" as a single completed object whose
  global properties are invoked. Per `CANONICAL_REFRAME.md` such moves
  are proscribed.
- "Finitary version": a restatement that uses only properties that hold
  at every specified finite extent, with explicit rates of approach
  rather than completed limits.

---

## FTD_Yang_Mills_Mass_Gap.tex

### Summary of paper's claim

The paper purports to give a constructive resolution of the Clay
Yang-Mills Existence and Mass Gap problem by (i) constructing a
ternary-state lattice gauge theory whose gauge group SU(3) emerges from
the BCC sublattice of the Moore neighborhood, (ii) proving the path
integral is UV-finite (compact Brillouin zone) and IR-finite
(manifestation threshold K_B > 0), (iii) identifying a "constructive
mass gap" Delta = K_B = m_e via the manifestation threshold, and
(iv) deriving Wilson-loop area law confinement at beta = x_- = 3.024
with string tension sigma = 0.209. The paper itself acknowledges three
"gaps" (Wightman/OS axioms on R^4, generalization to arbitrary G,
Lorentz invariance) but frames them as residual rather than fatal.

### Where the completed-infinity premise enters

The paper's central proof structure depends on completed-infinity moves
in load-bearing places, not just in framing:

1. **Axiom 1 (Discrete space).** "Physical space is the
   three-dimensional cubic lattice Lambda = Z^3." This is the exact
   completed-infinity ontological commitment that Foundational Reframe
   v1.0 retracts. The paper's foundational axiom is no longer admissible
   as stated.

2. **Theorem 3.3 (UV finiteness), proof.** The proof writes
   "all loop integrals integral_BZ d^3 k f(G_L(k)) are bounded" and
   relies on "the lattice propagator on the compact Brillouin zone."
   The Brillouin zone exists as a compact object only if the lattice is
   taken as a completed totality (Z^3 with discrete Fourier transform
   on the dual torus). On a finite undefined-boundary region the
   "compact BZ" is replaced by a finite set of L^3 modes; the integral
   becomes a finite sum. This is recoverable, but the proof as written
   passes through completed infinity.

3. **Theorem 3.4 (IR finiteness).** Stated for "any finite lattice
   |Lambda| = N < infinity", which is actually consistent with the
   reframe. But the paper uses this only as a prelude to the
   "thermodynamic limit" claim in Proposition 5.2.

4. **Proposition 5.2 (Infinite-volume mass gap).** "The mass gap
   Delta = K_B persists in the thermodynamic limit |Lambda| -> infinity."
   This is the load-bearing claim needed to match the Clay statement,
   which is stated on R^4. The proof argues the threshold is a "local
   criterion" so taking the limit "does not change the threshold." This
   is closer to a finitary statement than it looks (the threshold holds
   per voxel for every L), but the paper frames it as a thermodynamic
   limit and the conclusion ("persists in the limit") is a completed-
   infinity statement.

5. **Section 7 Gap 1 ("Wightman / Osterwalder-Schrader axioms").** The
   paper concedes the construction is on Z^3 x N rather than R^4 and
   appeals to "the lattice IS the physical theory." Under the reframe,
   even Z^3 x N is not admissible as a totality; the paper's fallback
   position ("the lattice IS the theory") needs to be restated as "the
   theory is defined pointwise with no global commitment," which is a
   substantively weaker statement than what Clay asks for.

6. **Section 7 Gap 3 (Lorentz invariance).** The argument that
   rotational invariance is recovered "in the scaling limit" with
   corrections O((a/L)^2) invokes a completed limit a -> 0.

7. **Theorem 8.1 (Regularity ladder).** Uses a uniformly convergent
   infinite series sum_{k>=0} a_k psi_k(t). On its own this is a
   constructive Wallis-style claim and survives the reframe. But the
   corollary draws a conclusion about "the emergent continuum" and a
   "C^m function on the emergent continuum," which presupposes the
   continuum as a completed object.

The clean summary is: under the reframe, the paper's "constructive mass
gap" theorem (Section 5) is the part that survives most cleanly,
because it is genuinely a per-voxel local criterion that holds at every
specified L. Everything else - existence as a Wightman/OS theory on
R^4, persistence in the thermodynamic limit, Wilson-loop area law as a
limiting object - is either reframed (to a weaker per-L statement) or
loses its connection to the Clay formulation.

### Three options, ranked by feasibility

**Option 1 - RE-DERIVE finitarily.**

A finitary version of the paper is possible but is a substantively
different paper. The new theorem statements would look like:

- *Existence.* "For every specified finite cubic region Omega subset of
  the lattice with |Omega| = L^3, the partition function Z_Omega is a
  finite sum-integral of a strictly positive Boltzmann weight, hence
  well-defined and finite." This is a one-line theorem and is true.

- *Mass gap.* "At every voxel and every tick, the manifestation
  condition |J(v)| >= K_B is the energy threshold for s(v) != 0;
  consequently, for every specified L, the L-region single-particle
  spectrum has a gap Delta_L >= K_B." This is the strongest part of
  the paper; it survives essentially unchanged because the criterion
  is genuinely local.

- *Confinement.* "At beta = x_- = 3.024, for every specified rectangular
  R x T Wilson loop with R, T finite, the strong-coupling expansion
  gives <W(C)> = u_p^{RT} with u_p = I_1(beta)/I_0(beta) = 0.812 and
  string tension sigma = -ln u_p = 0.209." The Bessel-function ratio is
  algebraic; the area law for finite R, T is a finitary statement; the
  paper's content here essentially survives.

- *Asymptotic freedom.* The one-loop beta function is a parametric
  insertion, the paper already labels it as such. No reframe needed,
  but the claim is also not a derivation.

What does not survive a finitary restatement:

- The framing "we resolve the Clay problem." Clay-as-stated requires a
  Wightman/OS theory on R^4 with a positive mass gap. A theory of
  per-L lattice statements does not satisfy Clay-as-stated and cannot
  be promoted to satisfy it without a completed-infinity step.
- The claim that the theory is "the" quantum Yang-Mills theory. Under
  the reframe, "the" theory is not a well-defined object; the family
  {theory_L} indexed by L is.

**Effort estimate:** DAYS to produce a finitary draft that preserves
all genuine FTD content (mass gap as local criterion, gauge structure
from Moore decomposition, Wilson-loop algebra at finite R x T,
asymptotic-freedom parametric insertion). The resulting paper would no
longer claim to address Clay; it would be titled something like
"Per-voxel mass gap and finite-region confinement in the FTD lattice
gauge theory."

**Option 2 - DEMOTE to conjecture.**

Restate the central claim as:

> [STRONGLY MOTIVATED CONJECTURE] The FTD lattice gauge theory, defined
> pointwise with no global completion, exhibits per-voxel mass gap
> Delta = K_B and finite-region confinement with string tension
> sigma = -ln(I_1(x_-)/I_0(x_-)) at every specified L. The conjecture
> that these per-L properties together constitute a resolution of the
> Clay Yang-Mills problem (which is stated on R^4) is not claimed; the
> Clay problem as literally stated requires a continuum reconstruction
> that FTD does not provide.

Tag changes:
- "Mass gap" stays [THEOREM] (it is a genuine per-voxel theorem).
- "Existence" demotes from [THEOREM] (UV-finite, IR-finite "QFT") to
  [THEOREM, restricted] (well-defined per-L partition function), and
  the claim of existence as a Clay-eligible QFT becomes [CONJECTURE]
  or is dropped.
- "Confinement" stays [THEOREM] in the per-L finite-R-x-T sense;
  drops to [CONJECTURE] for the limiting object claim.
- The overall framing "addresses the Clay problem in substance"
  demotes to [CONJECTURE] or is removed.

Residual content under Option 2: the paper becomes a paper about
local-criterion mass gaps and finite-region confinement on a lattice
gauge theory whose gauge structure is fixed by the Moore decomposition.
That is a real result. It is not a Clay solution.

**Option 3 - RETRACT.**

If the paper's only value-add over standard lattice gauge theory is
(a) the SU(3)-from-Moore-decomposition selection argument and (b) the
"K_B as constructive mass gap" identification, and if the Clay framing
is removed, then the residual content is much smaller and overlaps
with material already in `THEOREM_MOORE_LAYER_DECOMPOSITION.md` and
`docs/theory/03_derivations/DERIV_NC_FROM_TOPOLOGY.md`. In that case
the paper could be moved to `archive/` and a much shorter note in the
theory directory could capture the surviving content (per-voxel mass
gap as a corollary of the manifestation axiom; Wilson-loop algebra at
finite R, T as a finitary computation).

Dependent claims that would need adjustment if retracted:
- Any external citation of "FTD provides a Yang-Mills mass gap" in the
  manuscript, whitepaper, or `docs/theory/01_reference/` references.
- The line in the project README / CLAUDE.md / `META_DOCUMENTATION_MAP.md`
  that lists this paper as a portfolio item, if such a line exists.
- Spot-check any paper or chapter that cites this paper as evidence
  for "FTD addresses Millennium problems"; restate or remove.

**Option 4 (unanticipated) - SPLIT.**

The paper as written conflates two distinct contributions:
(A) the genuine FTD-internal claim that K_B is a per-voxel manifestation
threshold and therefore acts as a local mass gap, and
(B) the contested claim that this constitutes a resolution of Clay.

The paper could be split into:
- A short, finitary, no-Clay-framing paper: "Local manifestation
  threshold as mass gap in FTD lattice gauge theory." This would be
  defensible, would use only finitary statements, and would carry
  appropriate epistemic tags.
- A separate "philosophical position" or "conjecture" note: "On the
  ontological status of the Clay Yang-Mills problem under
  undefined-boundary lattice ontology." This would explicitly state
  that the Clay problem as literally stated may not have an answer
  within FTD, and that the FTD position is that Clay-as-stated is the
  wrong question.

The split is the most epistemically honest disposition.

### Recommended action

**OWNER-JUDGMENT-NEEDED**, with strong default toward Option 4 (SPLIT)
or Option 2 (DEMOTE).

The author needs to decide between:

- (a) Claiming that FTD addresses the Clay Yang-Mills problem, knowing
  that under the foundational reframe this claim cannot be supported by
  a derivation - it must be supported by a *philosophical argument*
  that the Clay problem itself is malformed (Clay assumes R^4
  ontology). This is a defensible position but it is not a proof; it
  is a stance.

- (b) Restricting the paper's claims to per-L finitary statements,
  which are defensible but do not address Clay.

- (c) Retracting the paper as a Clay-addressing paper and folding the
  surviving per-L content into existing FTD documentation.

The owner cannot have both "we have proved the Clay Yang-Mills mass
gap" and "we accept the undefined-boundary reframe." Pick one.

### Specific things the owner needs to decide

- Is the FTD position that Clay-as-stated is the wrong problem? If yes,
  the paper should explicitly say so in the abstract and stop claiming
  to "address the Clay problem in substance."
- Is the per-voxel mass gap result novel enough to stand on its own
  without the Clay framing? (My read: it is interesting but not
  Clay-level; it is essentially a restatement of the manifestation
  axiom.)
- Does the SU(3)-from-BCC selection argument belong in this paper or
  in `THEOREM_MOORE_LAYER_DECOMPOSITION.md`? (Currently duplicated.)
- If the paper is moved to `archive/speculative/`, what shortest note
  should remain in the live tree to preserve the surviving content?
- Should the paper explicitly cite `AUDIT_INFINITY_REFRAME.md` and
  `CANONICAL_REFRAME.md` in its acknowledgement of which steps are no
  longer admissible?
- Is there appetite for the SPLIT (Option 4)? It is more work but it is
  the most honest disposition.

---

## FTD_Navier_Stokes.tex

### Summary of paper's claim

The paper purports to address the Clay Navier-Stokes Existence and
Smoothness problem by constructing a discrete lattice dynamics on
Z^3 x N whose large-scale limit recovers incompressible Navier-Stokes,
proving global existence and uniqueness on the lattice, deriving a
uniform energy bound, and concluding that "finite-time blow-up is an
artifact of the continuum idealization." The paper's central move is
ontological: if physical space is Z^3, the minimum length scale is the
voxel and energy cannot concentrate below it, so the BKM blow-up
scenario is structurally excluded.

### Where the completed-infinity premise enters

This paper is more cleanly entangled with completed-infinity reasoning
than the Yang-Mills paper, and several of its most-load-bearing steps
do not survive the reframe:

1. **Axiom of physical space being Z^3.** Identical issue to the
   Yang-Mills paper's Axiom 1. Under the reframe, the lattice is
   undefined-boundary, not Z^3-as-totality.

2. **Theorem 4.2 (Uniform energy bound), proof.** The proof works "on
   a periodic lattice Lambda = (Z/N Z)^3" via the integration-by-parts
   identity sum_v J . nabla_L^2 J = -sum_v |nabla_L J|^2 <= 0. The
   periodic-boundary-condition step requires the full lattice as a
   completed object so that boundary terms vanish; on an
   undefined-boundary region, integration by parts produces boundary
   terms that are not controlled. **This is the load-bearing technical
   step in the paper.** Without the periodic IBP identity, the energy
   bound does not survive.

3. **Theorem 4.3 (No finite-time blow-up), proof.** Depends on
   Theorem 4.2; same issue. Furthermore, the BKM integral is bounded
   using "max_v |J(v,t)| <= sqrt(2 E(0))" which uses the global energy
   bound, again requiring the full lattice.

4. **Theorem 5.1 (Minimum scale cutoff) and Proposition 5.2 (Energy
   cannot concentrate below the lattice scale).** These are the
   ontological core of the paper: blow-up is impossible because there
   is a minimum length scale. The minimum-scale-cutoff claim itself
   is a local statement and survives. But the *use* of this claim to
   conclude "no blow-up" requires comparing local lattice values to a
   continuum quantity (||u||_{L^infty}), which requires the continuum
   limit as a completed object.

5. **Theorem 6.1 (Lattice wave equation recovers Navier-Stokes).**
   "In the limit where the lattice spacing a -> 0 and the tick duration
   Delta t -> 0 with C = a/Delta t = 1/sqrt(3) fixed, the FTD update
   rule reduces to..." This is a completed-infinity move (continuum
   limit as a completed object). Under the reframe, this is not
   admissible as a derivation; it can only be restated as "for any
   target precision epsilon, there exists a (small) a such that the
   lattice update approximates the continuum equation within epsilon
   on the relevant test functions."

6. **Theorem 6.2 (Regularity ladder).** Uses Sobolev embedding
   H^m(R^3) -> C^{m-2}(R^3), which is a theorem about the continuum
   space R^3. The paper cites this as if it gives smoothness for "the
   corresponding continuum-limit field u(x, t)." Under the reframe,
   the continuum-limit field is not a constructed object - so the
   embedding does not transfer to a per-L statement without
   substantive additional work.

7. **The central thesis** ("blow-up is an artifact of the continuum
   idealization") is itself a *philosophical* claim, not a theorem.
   It says: Clay's problem is malformed because Clay's R^3 substrate
   is not the right physics. This claim does not require completed
   infinity to make - but it also does not constitute a resolution of
   Clay-as-stated.

The honest summary: the paper's central technical work is bounding
energy on a periodic lattice (which uses periodic boundary conditions
to make IBP work), and translating that bound to the continuum (which
requires a completed continuum limit). Both steps invoke completed
infinity. Without them, the paper's "no blow-up" conclusion does not
follow as a *theorem*; it follows only as the *consequence of the
ontological choice* "Z^3 is fundamental, R^3 is approximation."

This is a stronger statement than for the Yang-Mills paper. The
Yang-Mills paper has at least one substantial theorem (per-voxel mass
gap from manifestation threshold) that survives the reframe cleanly.
The Navier-Stokes paper's theorems all depend either on
periodic-boundary IBP or on a continuum-limit comparison.

### Three options, ranked by feasibility

**Option 1 - RE-DERIVE finitarily.**

A finitary version is harder than for the Yang-Mills paper because the
paper's central technical content is the energy bound, and that bound
relies on periodic boundary conditions. Possible finitary statements:

- *Per-tick local energy non-increase.* "At every voxel v and every
  tick t, after the diffusion step, the local energy 1/2 |J(v,t+1)|^2
  is bounded by an explicit linear combination of the energies at v
  and its 6 neighbors at tick t." This is a genuine finitary statement
  but is much weaker than the global energy bound.

- *Bounded energy on a specified L^3 region with explicit boundary
  contribution.* "On a finite region of side L, the total energy
  E_L(t) satisfies E_L(t+1) - E_L(t) <= - epsilon ||nabla_L J||_{L^2}^2
  + B_L(t), where B_L(t) is an explicit boundary flux term." This
  is finitary but the boundary term is the load-bearing piece - it is
  not bounded uniformly without additional assumptions (e.g., that the
  initial data is supported in a smaller region, or that we assume
  periodic boundary conditions, which is a completed-infinity-adjacent
  move).

- *No blow-up at any finite tick.* "For every initial data with
  per-voxel energy bounded by B_0 in a specified finite region, the
  per-voxel energy at any specified subsequent tick t in the region is
  bounded by an explicit constant C(t, B_0, region)." This is
  finitary but the constant grows with t in general, so the
  "no blow-up at any finite time" claim becomes "no blow-up at any
  specified finite tick", which is automatic from the deterministic
  polynomial update rule and is much weaker than the paper claims.

- *Per-voxel vortex-stretching pointwise bound.* The bound
  |(omega . nabla) J| <= 6 sqrt(3) ||omega||_inf ||J||_inf in
  Theorem 7.1 is a per-voxel estimate; it survives if the
  ||.||_inf norms are taken over a specified finite region, but the
  bound is then in terms of the *region* sup, not a global sup.

What does not survive a finitary restatement:

- The global energy bound E(t) <= E(0) in the form claimed.
- The corollary that BKM is finite for all T < infinity.
- The continuum recovery of Navier-Stokes as a derivation (it survives
  only as a per-precision approximation statement, not as a derivation
  of "the" continuum equation).
- The framing "smooth solutions exist for all time" for the
  continuum problem.

**Effort estimate:** WEEKS to produce a finitary draft that is
defensible. The technical work to handle boundary terms in the energy
bound is non-trivial. A finitary regularity-ladder argument that does
not appeal to Sobolev embedding on R^3 needs new infrastructure (likely
some kind of per-L decay-rate bound). The resulting paper would not
address Clay; it would be an internal paper about the FTD lattice
fluid dynamics with explicit per-L bounds.

**Option 2 - DEMOTE to conjecture.**

Restate the central claim as:

> [STRONGLY MOTIVATED CONJECTURE] The FTD lattice fluid dynamics, on
> any specified finite region with periodic boundary conditions on
> that region, exhibits global existence, uniqueness, and an energy
> bound consistent with no finite-tick blow-up. The conjecture that
> this resolves the Clay Navier-Stokes problem (which is stated on
> R^3) is *not* claimed; FTD's position is that Clay-as-stated assumes
> a continuum substrate that FTD does not accept.

Tag changes:
- "Existence and uniqueness on a finite periodic region" stays
  [THEOREM] (this is just iterating a deterministic update rule).
- "Energy bound on a finite periodic region" stays [THEOREM] but
  with explicit periodic-boundary caveat.
- "No finite-time blow-up" demotes from [THEOREM] to [CONJECTURE] for
  the unbounded-region case; remains [THEOREM] only for the strict
  per-tick statement on a periodic region.
- "Smooth solutions of continuum Navier-Stokes" demotes to
  [CONJECTURE] or is dropped; it is not derived, it is assumed via
  the continuum-limit framing.
- The framing "addresses the Clay problem" is dropped.

The residual paper under Option 2 would be a real but modest
contribution: a deterministic, energy-bounded, well-defined lattice
fluid model. That is interesting; it is not a Millennium-problem
paper.

**Option 3 - RETRACT.**

Stronger candidate for retraction than the Yang-Mills paper, because:

- The genuinely novel technical content (uniform energy bound, BKM
  integral bound, regularity ladder for continuum smoothness) all
  depends on completed-infinity moves.
- The surviving claims (deterministic local rules give a unique
  per-tick update; per-voxel energy bounded by deterministic
  polynomial composition) are essentially restatements of the FTD
  postulates and do not need a paper.
- The ontological argument "blow-up is an artifact of the continuum"
  is a one-paragraph philosophical position, not a paper.

If retracted, dependent adjustments needed:
- Any references in the manuscript or whitepaper to "FTD addresses
  Navier-Stokes."
- The line in `META_DOCUMENTATION_MAP.md` or similar that lists this
  paper, if such a line exists.
- Project-level claims about "Millennium problems addressed" anywhere
  in the portfolio (CLAUDE.md, README, summary docs).

**Option 4 (unanticipated) - REFRAME as ontological position paper.**

The paper's most defensible content is *not* its theorems; it is its
philosophical claim that the Navier-Stokes blow-up question is
ill-posed under undefined-boundary ontology. This is a real position
that deserves a careful statement. A reframed paper could be:

- Title: "Navier-Stokes blow-up under undefined-boundary lattice
  ontology: a position paper."
- Content: explicit statement that Clay-as-stated assumes R^3
  ontology; explicit statement that under the FTD reframe, the
  question of blow-up at scales below the lattice spacing is not a
  question the framework recognizes; finitary per-voxel statements
  that do hold and are interesting; an honest accounting of what
  Clay would still ask of FTD that FTD does not provide.

This would be a much shorter, more honest paper. It would not claim
to "address" or "resolve" Clay; it would take a position on Clay.

### Recommended action

**RETRACT or REFRAME (Option 3 or Option 4).**

The Navier-Stokes paper's situation is more severe than the
Yang-Mills paper's. The central technical work either depends on
periodic boundary conditions (a completed-infinity-adjacent move) or
on a continuum limit (a proscribed completed-infinity move). The
paper does not have a strong per-voxel theorem analogous to the
Yang-Mills mass gap that survives the reframe cleanly.

The owner should not let this paper continue to claim it "resolves"
or "addresses in substance" the Clay Navier-Stokes problem. Under the
foundational reframe, that claim cannot be supported.

### Specific things the owner needs to decide

- Is the FTD position genuinely "Clay Navier-Stokes is the wrong
  problem"? If so, that should be the paper's explicit thesis, not a
  consequence buried in caution boxes.
- Does the per-voxel polynomial-update argument count as a
  contribution worth a paper? (My read: no - it is just iterating a
  deterministic rule and is already covered by the engine spec.)
- Are there any external citations of this paper as a Navier-Stokes
  result that need adjustment?
- If retracted, where does the surviving philosophical content
  go - a short note in `docs/theory/02_foundations/`, or the
  manuscript v2, or nowhere?
- Should the paper be moved to `archive/speculative/` with an
  explicit retraction note explaining which steps did not survive
  the reframe?
- Is there appetite for the REFRAME (Option 4)? It would be a
  defensible short paper but its claims would be much weaker than
  the current draft.
- More broadly: does the FTD portfolio's self-description (in
  `CLAUDE.md`, `META_DOCUMENTATION_MAP.md`, project README) currently
  imply that FTD addresses Navier-Stokes? If yes, that
  self-description needs adjustment regardless of which option is
  taken on the paper itself.

---

## Cross-paper observations

### Both papers depend on the same proscribed move

Both papers use the same load-bearing pattern: "Z^3 is the physical
substrate; therefore properties P that are local on Z^3 give us a
global theorem." This pattern is exactly what the foundational reframe
proscribes. Z^3 is no longer admissible as a totalized substrate.

The two papers differ in how badly this hurts them:

- The Yang-Mills paper has *one* theorem (per-voxel mass gap from the
  manifestation threshold) that genuinely is a local property holding
  at every specified L. That theorem survives the reframe. The rest
  of the paper depends on the proscribed move.
- The Navier-Stokes paper does not have an analogous standalone
  per-voxel theorem. Its energy bound, blow-up exclusion, and
  continuum recovery all depend on the proscribed move.

This asymmetry is why the Yang-Mills paper has a viable Option 4
(SPLIT) and the Navier-Stokes paper's most defensible disposition is
RETRACT or REFRAME.

### Both papers acknowledge the gaps but underweight them

Both papers contain caution boxes that essentially admit the central
claims do not address Clay-as-stated. The Yang-Mills paper's Section 7
"Gaps" lists three of them; the Navier-Stokes paper's "What is and is
not proved" caution box says explicitly "the lattice construction
*resolves* the blow-up problem by eliminating the unphysical regime
where it would occur. Whether this constitutes a resolution of the
Clay problem depends on whether one accepts the ontological primacy
of Z^3 over R^3."

This honesty is good. But the abstract, title, and introduction of
both papers continue to claim a Clay resolution. Under the
foundational reframe, the gap between abstract and caution box is no
longer acceptable: the abstract should state the actual epistemic
status from the start.

### Both papers should cite the reframe explicitly

If kept in the portfolio in any form, both papers must cite
`CANONICAL_REFRAME.md` v1.0 and `AUDIT_INFINITY_REFRAME.md` and
explicitly say which steps in the paper are admissible under the
reframe and which are not. Anything else is misleading the reader.

### Portfolio-level question

Are there other portfolio claims (in the manuscript, the whitepaper,
or top-level documents like CLAUDE.md or META_DOCUMENTATION_MAP.md)
that imply FTD addresses Millennium problems? If yes, those claims
also need to be revisited as part of the reframe deployment. Listing
two speculative papers under "Yang-Mills mass gap" and
"Navier-Stokes" in any project summary or README implies a stronger
result than the papers themselves can support post-reframe.

---

## Summary table

| Paper | Surviving theorem | Load-bearing proscribed move | Recommendation |
|---|---|---|---|
| Yang-Mills | Per-voxel mass gap (manifestation threshold) | Z^3 substrate; thermodynamic limit; continuum reconstruction | OWNER-JUDGMENT-NEEDED, default SPLIT (Option 4) or DEMOTE (Option 2) |
| Navier-Stokes | None standalone | Periodic-IBP energy bound; continuum limit; Sobolev embedding on R^3 | RETRACT (Option 3) or REFRAME (Option 4) |

---

## End of report

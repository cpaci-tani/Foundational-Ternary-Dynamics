# V3 dressed SC-source Gauss continuity v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT FOR THE DECLARED FINITE SOURCE-MACRO
EXTENSION]** + **[SELECTION — SOURCE MACRO NOT YET THE V3 REFERENCE LAW]** +
**[OPEN — CHARGED POLE, MOVING SOURCES, WORK, AND PHYSICAL COUPLING]**  
**Carrier price:** none; uses the selected v3 R1-v2 alphabets unchanged  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[`proof_v3_dressed_sc_source_gauss_continuity.py`](../../../../../scripts/proofs/proof_v3_dressed_sc_source_gauss_continuity.py)
passes 15/15 gates and 4,320 exact tick identities across every SC direction,
polarity, local C3 layer, and the complete 24-tick source cycle. It performs no
numerical search and reads no physical coupling or target value.

---

## 1. The carrier already contains the missing bound-state discriminator

The selected site field bank has channels

\[
 (d,n,h,k,\epsilon),
 \qquad 6\cdot4\cdot2\cdot4\cdot2=384,
\]

and every SC relation owns one primary/reserve A9 pair. A context-free electric
edge is the complete eight-channel stabilizer packet

\[
 \mathcal P(d,k,\epsilon)
 =\{(d,n,h,k,\epsilon):n\perp d,\ h=\pm1\}.
\]

No new bound/free flag is required. If the relation token has phase `k` and
the packet-owning site has layer `ell`, the Chinese-remainder conditions

\[
 r\equiv k\pmod4,
 \qquad -r\equiv\ell\pmod3
\]

select one unique `r in Z12`. Advancing the base packet through that many
Hodge/C4 frames gives the unique packet compatible with the already present
relation phase and site layer. Thus the complete bound presentation is
state-identifiable from current finite records; it needs no packet identity,
birth tick, route label, or replay tape.

---

## 2. Selected source-macro extension

Present an SC edge from tail `x` to head `x+d`. Under the v3 incidence
convention, a primary token of polarity `epsilon` contributes `+epsilon` at
the tail and `-epsilon` at the head. Its dressing therefore uses the packet in
direction `-d`, stored at the head site.

The candidate source macro has two valid ownership sectors:

\[
 (\lambda,\rho;\mathcal B)
 =(0,z(k,\epsilon);\mathcal B_0)
\]

and

\[
 (\lambda,\rho;\mathcal B)
 =(z(k,\epsilon),0;
 \mathcal B_0\cup\mathcal P_{k,\ell}(-d,\epsilon)).
\]

At a phase-zero ownership crossing, the first state creates the complete
packet while moving the A9 token to primary ownership; the reverse crossing
removes the packet while returning the token to reserve ownership. Between
crossings the packet undergoes the ordinary Hodge/C4 internal permutation in
place instead of streaming. Partial or occupied targets fail closed.

The macro is a deterministic finite permutation on its valid 24-state
source/C3 orbit. It is local to one SC edge and its endpoint bank. It is an
extension candidate, not yet part of the selected Phi-v2 rule.

The extension is exactly inert on the registered R5 vacuum preparation,
because that preparation has both A9 relation slots occupied. Consequently it
does not alter the already certified transverse vacuum operator or speed
`1/6` at that preparation.

---

## 3. Exact Gauss and continuity identities

The raw eight-channel packet has electric moment `-8 epsilon d`; division by
the already proved Gram normalization gives the canonical edge coefficient

\[
 E_e=-\epsilon.
\]

Writing `partial e=-delta_x+delta_{x+d}`, the primary relation charge and
electric divergence are therefore identical:

\[
 Q_e=-\epsilon\,\partial e,
 \qquad
 \operatorname{div}E_e=-\epsilon\,\partial e,
\]

so every valid source state obeys

\[
 \boxed{\operatorname{div}E=Q.}
\]

For one ownership change `Delta o in {-1,0,+1}`, define the oriented current

\[
 j_e=\epsilon\,\Delta o.
\]

Direct incidence gives, tick by tick,

\[
 \boxed{\Delta Q+\operatorname{div}j=0},
 \qquad
 \boxed{\Delta E=-j},
 \qquad
 \boxed{\Delta(\operatorname{div}E-Q)=0}.
\]

These are chain-complex identities on finite records. No continuum divergence
operator or Gauss projector is inserted.

---

## 4. What is and is not closed

Closed for the declared candidate macro:

- realization inside the existing v3 carrier;
- one state-only bound-dressing discriminator;
- all 12 C4/C3 Hodge frames;
- signed-cubic covariance;
- exact period-24 source creation/withdrawal;
- exact local charge continuity and Gauss preservation; and
- exact inactivity on the R5 transverse-vacuum preparation.

Not closed:

- incorporation into the canonical selected `Phi` and re-verification of its
  complete collision schedule;
- motion of separated charge endpoints;
- divergence-preserving relaxation to a long-range massless static pole;
- the active/reserve work and action curvature `chi_EM`;
- stable matter or a physical detector; and
- any identification with the master-quadratic fine-structure root.

The microscopic occupation count changes by eight when the dressing is
activated. Reversibility proves that no record is secretly required, but it
does not price that change as physical energy. The work ledger and coupling
normalization therefore remain genuine downstream gates.

---

## 5. Next charged gate

The next extension must act on a connected chain of these dressed SC edges and
provide local plaquette/cycle moves satisfying

\[
 \Delta Q=0,
 \qquad
 \operatorname{div}\Delta E=0,
\]

while retaining a positive reciprocal work ledger. A pass must show that
finite deterministic histories relax or block to a charged massless static
kernel, rather than merely preserve one string dressing. Only after that pole
exists can the already registered blind curvature observable `chi_EM` be
evaluated.


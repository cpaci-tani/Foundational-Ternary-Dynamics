# Cotangent single-record STF streaming-lift obstruction v1

**Date:** 2026-08-24

**Status:** **[THEOREM — COMPLETE $O_h$-EQUIVARIANT C18 ROUTE CENSUS]** +
**[SCOPED CLOSED NEGATIVE — CO-LAYER SINGLE-RECORD STF FIRST MOMENT]** +
**[SCOPED CLOSED NEGATIVE — ADJACENT-LAYER SINGLE-RECORD SYMMETRIC-CURL
LIFT]** + **[OPEN — MULTI-RECORD PARITY COLLISION OR LARGER CARRIER]**

**Production status:** unchanged

**Ledger status:** no row minted

**Exact certificate:**
[proof_cotangent_single_record_stf_streaming_lift_obstruction.py](../../../../../scripts/proofs/proof_cotangent_single_record_stf_streaming_lift_obstruction.py)
performs 42,357 exact checks. It exhausts all eighteen C18 route seeds on the
regular 48-flag $O_h$ orbit, all three cotangent layers, all four C4 phases,
both adjacent-layer stagger orientations, and the registered 98 primitive
wavevectors. No floating-point eigensolver or physical target fit is used.

---

## 1. Question inherited from the tensor-curl target

The
[STF parity-price and spin-2 curl theorem](THEOREM_COTANGENT_STF_PARITY_PRICE_AND_SPIN2_CURL_TARGET_v1.md)
proved three facts:

1. the cotangent carrier spans an inversion-even STF tensor $S$;
2. handedness supplies its inversion-odd partner $hS$; and
3. the isotropic symmetric tensor curl is the required first-derivative
   operator for a TT helicity-two cone.

That theorem did not construct a finite streaming/collision lift. The
smallest possible successor would simply assign each existing cotangent
record one C18 route and stream it. The present theorem asks whether any such
deterministic cubic-equivariant assignment can generate the target.

---

## 2. Complete single-record route class

Let $\mathcal F$ be the 48 oriented cotangent flags and let $O_h$ act by the
registered polar/axial/handedness rule. The action is regular:

\[
 |\mathcal F|=|O_h|=48,
 \qquad
 \operatorname{Stab}_{O_h}(f_0)=\{e\}.             \tag{1}
\]

Let $\mathcal R_{18}$ be the six SC plus twelve FCC directions. An equivariant
route map

\[
 r:\mathcal F\longrightarrow \mathcal R_{18},
 \qquad
 r(gf)=g\,r(f),                                    \tag{2}
\]

is therefore fixed uniquely by the seed $r(f_0)$. Consequently there are
exactly eighteen such maps. Allowing an independent seed at each C4 phase
exhausts every phase-dependent deterministic route in this one-record class.

This is a classification statement, not a search for a favorable
coefficient.

---

## 3. Co-layer first-moment zero

At cotangent layer $q$, retain the twenty independent phase/parity STF
coordinates

\[
 (uS_q,vS_q,uhS_q,vhS_q),                          \tag{3}
\]

with five trace-free coordinates per tensor. For route component $r_a$, the
projected first spatial moment is

\[
 M_a
 =R\,\operatorname{diag}(r_a)R^T(RR^T)^{-1}.      \tag{4}
\]

The exact census gives

\[
 \boxed{M_a=0}                                     \tag{5}
\]

for every layer, C4 phase, route seed, and spatial component. Thus neither the
old tangent stream nor any other phase-dependent $O_h$-equivariant C18 stream
can generate a co-layer $O(k)$ STF operator. The correct even/odd
representation types are present, but diagonal single-record streaming does
not couple them.

---

## 4. Adjacent-layer stagger

The next-smallest construction places the even readout on layer $q$ and the
odd readout on $q\pm1$. Its moment rows are

\[
 R_{q,\pm}=(1,S_q,hS_{q\pm1}).                     \tag{6}
\]

These rows do produce nonzero first moments. Collect all three layer choices,
all eighteen route seeds, and all three spatial components into an exact
operator matrix $\mathcal A_\pm$. The certificate finds

\[
 \operatorname{rank}\mathcal A_+
 =\operatorname{rank}\mathcal A_-=3,               \tag{7}
\]

and the combined span still has rank three:

\[
 \operatorname{rank}(\mathcal A_+,\mathcal A_-)=3. \tag{8}
\]

The stagger has created a genuine cubic tensor-gradient family, but not an
arbitrary isotropic tensor derivative.

---

## 5. Symmetric-curl exclusion

Let $\mathcal C_a$ be the five-coordinate restriction of

\[
 (\operatorname{curl}_s T)_{ij}
 ={1\over2}\left(
 \epsilon_{ik\ell}\partial_kT_{\ell j}
 +\epsilon_{jk\ell}\partial_kT_{\ell i}
 \right).                                         \tag{9}
\]

The required real first-moment block on $(S,hS)$ is

\[
 T_a=
 \begin{pmatrix}
 0&-\mathcal C_a\\
 \mathcal C_a&0
 \end{pmatrix}.                                   \tag{10}
\]

Stacking the three axes, the exact augmentation test gives

\[
 \operatorname{rank}\mathcal A=3,
 \qquad
 \operatorname{rank}(\mathcal A,T)=4.              \tag{11}
\]

Therefore

\[
 \boxed{T\notin\operatorname{span}\mathcal A}.      \tag{12}
\]

No weighted schedule, convex mixture, reversal completion, or longer census
of the same single-record adjacent-layer routes can repair this: all remain
inside the rank-three span already exhausted by equation (11).

---

## 6. TT leakage of the canonical stagger

For the most direct witness—stream along the registered tangent and average
the three forward layer staggers—the resulting operator preserves the TT
subspace on only six of the 98 registered primitive wavevectors. Those six
are the signed cubic axes. Generic directions leak into longitudinal STF
components.

This is the expected physical distinction between a cubic tensor-gradient
operator and the isotropic symmetric curl. Axis-only helicity behavior is not
a spin-2 cone.

---

## 7. Exact scope of the closure

### Closed negative

The following route is excluded:

> one existing 192-state cotangent record, one deterministic
> $O_h$-equivariant SC/FCC route per C4 phase, diagonal streaming, and either a
> co-layer or adjacent-layer even/odd STF readout.

It cannot generate the required symmetric tensor curl.

### Not excluded

The theorem does not exclude:

1. a genuine multi-record collision before or after streaming;
2. a rank-twenty on-site phase/parity carrier with a nontrivial local
   permutation;
3. an explicit primal/dual exchange whose collision changes the moment
   readout rather than merely offsetting its layer;
4. a longer-range finite route compiled from several local substeps;
5. constrained tensor dynamics generated by a different spin-2-equivalent
   variable; or
6. a nonlinear collective pole.

It also proves no static gravitational response, universal coupling, lensing,
or nonlinear completion. Production remains class 0.

---

## 8. Consequence for the unified action

The unified action cannot obtain gravity by attaching the tensor readout to
the already-proved Maxwell stream. The minimal surviving macro must contain a
new **parity-changing multi-record collision** $C_{PD}$ such that

\[
 R_{q-1}\,D_a C_{PD}\,R_q^T(R_qR_q^T)^{-1}         \tag{13}
\]

has the symmetric-curl block while retaining the seven-dimensional Maxwell
slow space and an exact inverse.

That collision must be composed with:

1. the two-token A9 actualization/source ledger;
2. the global-clock $O_h$ capacity mixer;
3. the common temporal/spatial permission gate;
4. the Gauss packet and work ledger; and
5. the contextual Born renewal detector.

The next locked gate is therefore no longer “find a stagger.” It is:

> construct or close negative the smallest finite multi-record
> parity-collision whose exact projected first moment is the symmetric tensor
> curl and whose Maxwell block is unchanged.

The
[right-regular collision successor](THEOREM_COTANGENT_RIGHT_REGULAR_COLLISION_SPIN2_SLOW_CLOSURE_OBSTRUCTION_v1.md)
executes the complete one-record collision-centralizer census. The
unrestricted projection can span the curl, but every target-producing
collision leaks the selected tensor variables into fast modes at zero
momentum. The sixteen slow-preserving collisions on each layer have
identically zero first-derivative span. The next gate is therefore genuinely
multi-record; a one-record local permutation cannot pay it.

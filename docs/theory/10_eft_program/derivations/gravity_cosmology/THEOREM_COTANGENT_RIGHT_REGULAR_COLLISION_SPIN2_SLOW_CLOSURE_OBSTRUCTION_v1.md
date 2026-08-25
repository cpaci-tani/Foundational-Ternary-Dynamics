# Cotangent right-regular collision spin-2 slow-closure obstruction v1

**Date:** 2026-08-24

**Status:** **[THEOREM — COMPLETE RIGHT-REGULAR $O_h$ COLLISION CENSUS]** +
**[THEOREM — UNRESTRICTED PROJECTED CURL WITNESS]** +
**[SCOPED CLOSED NEGATIVE — ONE-RECORD SLOW-CLOSED CURL LIFT]** +
**[OPEN — GENUINE MULTI-RECORD PARITY COLLISION]**

**Production status:** unchanged

**Ledger status:** no row minted

**Exact certificate:**
[proof_cotangent_right_regular_collision_spin2_closure_obstruction.py](../../../../../scripts/proofs/proof_cotangent_right_regular_collision_spin2_closure_obstruction.py)
performs 8,809 exact checks. It exhausts the 48-element right-regular
collision centralizer, all eighteen C18 streaming seeds, all three spatial
components, and all three cotangent layers.

---

## 1. Why the diagonal-streaming closure was not the end

The
[single-record streaming-lift obstruction](THEOREM_COTANGENT_SINGLE_RECORD_STF_STREAMING_LIFT_OBSTRUCTION_v1.md)
proved that diagonal cubic-equivariant streaming cannot generate the
symmetric tensor curl. A natural repair is to apply a nontrivial local
permutation to the flag before streaming.

Because the 48 flags carry the regular left action of $O_h$, every local
permutation commuting with that action lies in its 48-element right-regular
centralizer. This theorem exhausts that complete one-record collision class.

---

## 2. Finite collision class

Write every flag uniquely as $f=g f_0$. For fixed $h\in O_h$, define

\[
 C_h(gf_0)=ghf_0.                                  \tag{1}
\]

Then

\[
 C_h(kf)=kC_h(f),                                  \tag{2}
\]

so $C_h$ is a local $O_h$-equivariant permutation. The 48 choices of $h$ are
distinct and exhaust the permutation centralizer.

At cotangent layer $q$, collect the five even and five odd STF coordinates:

\[
 R_q=(S_q,hS_q).                                   \tag{3}
\]

For an C18 route seed $r$ and component $a$, the collision/streaming first
moment is

\[
 M_a(h,r)
 =R_qD_a(r)C_hR_q^T(R_qR_q^T)^{-1}.               \tag{4}
\]

---

## 3. The tempting unrestricted result

If equation (4) is projected without first requiring the ten tensor
observables to form a closed zero-momentum slow space, the complete operator
span has

\[
 \operatorname{rank}\mathcal M_{\rm all}=6.        \tag{5}
\]

The stacked symmetric-curl target belongs to this span:

\[
 \operatorname{rank}(\mathcal M_{\rm all},T)
 =\operatorname{rank}\mathcal M_{\rm all}=6.       \tag{6}
\]

Thus inserting a right-regular collision appears, at first sight, to repair
the rank-three diagonal-streaming obstruction.

Equation (6) is a kinematic projected witness only. It is not yet a
propagating sector.

---

## 4. The zero-momentum closure gate

A candidate collision is admissible only if the selected tensor observables
remain a closed slow space at zero momentum. Define its projected carrier

\[
 K_h=R_qC_hR_q^T(R_qR_q^T)^{-1}.                   \tag{7}
\]

Exact closure requires

\[
 R_qC_h=K_hR_q.                                    \tag{8}
\]

On every cotangent layer, exactly

\[
 16\ \text{of the}\ 48                              \tag{9}
\]

right-regular collisions satisfy equation (8).

For those sixteen collisions and all eighteen route seeds, however,

\[
 \boxed{M_a(h,r)=0}                                \tag{10}
\]

for all three spatial components. Consequently

\[
 \operatorname{rank}\mathcal M_{\rm closed}=0,
 \qquad
 \operatorname{rank}(\mathcal M_{\rm closed},T)=1. \tag{11}
\]

The apparent target in equation (6) is carried entirely by collisions that
send the selected STF observables into unresolved fast modes already at
$k=0$.

---

## 5. Interpretation

This is the difference between:

- obtaining the desired matrix after one projection; and
- possessing a finite invariant sector whose long-wavelength dynamics is
  governed by that matrix.

Only the second can support a pole. The first is a projection artifact unless
the omitted fast modes are retained and shown to close into a larger
invariant carrier.

Therefore:

\[
 \boxed{
 \text{one-record right-regular collision}
 +\text{C18 streaming}
 \not\Rightarrow
 \text{closed spin-2 sector}.}                     \tag{12}
\]

---

## 6. Exact scope

### Closed negative

The theorem excludes a finite spin-2 lift built from:

1. one existing 48-flag cotangent record per C4 phase;
2. any local $O_h$-equivariant permutation of that record;
3. any deterministic SC/FCC streaming seed; and
4. the selected ten-dimensional even/odd STF slow space.

### Open

It does not exclude:

1. a two-record or higher collision with a larger invariant slow space;
2. retaining and constraining the fast copies instead of projecting them
   away;
3. a nonlinear occupation-dependent pair collision;
4. a longer-range reversible compiler; or
5. another spin-2-equivalent variable.

No static pole, universal source law, lensing, or nonlinear gravity is proved.
Production remains class 0.

---

## 7. Next locked gate

The smallest surviving construction must declare at least two locally owned
records and a collision

\[
 C_{PD}:(f_P,f_D)\longmapsto(f'_P,f'_D)             \tag{13}
\]

that:

1. is a total finite permutation with an exact inverse;
2. preserves number, charge, packet payload, and the common work ledger;
3. leaves a declared Maxwell plus tensor slow space invariant at $k=0$;
4. produces the symmetric-curl block at $O(k)$ inside that invariant space;
5. preserves Gauss and TT constraints;
6. accepts the same primal/dual permission bits used by the clock/lensing
   macro; and
7. exposes a sourced static sector rather than only a radiative target.

This is now the precise spin-2 subproblem inside the one-action programme.

The
[rank-twenty closure successor](THEOREM_COTANGENT_RANK20_COLLISION_CLOSURE_AND_TT_LEAKAGE_v1.md)
retains rather than projects away the collision images. It proves that the
minimum leaky closure has rank twenty and is an exact invariant carrier, but
the selected target-containing witness expands a four-dimensional TT seed to
8, 16, or 18 dimensions depending on direction. The surviving gate is
therefore a native constraint-generating multi-record collision, not merely a
larger unconstrained carrier.

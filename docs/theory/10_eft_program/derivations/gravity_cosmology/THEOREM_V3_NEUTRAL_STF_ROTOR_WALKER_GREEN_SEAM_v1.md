# V3 neutral STF rotor/walker Green seam v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT CARRIER-COMPLETE FULL-RANK STF WALKER]** +
**[THEOREM, CONDITIONAL — FIVE-COMPONENT DIRICHLET GREEN LIMIT AND
$1/\Lambda$ STATIC POLE]** + **[BLOCKED-HISTORY SEAM ONLY]** +
**[OPEN — PHI INTEGRATION, ACTION, CONSTRAINTS, UNIVERSAL COUPLING, CONE,
AND LENSING]**  
**Carrier price:** six additional existing field bits at the marked site; no
new primitive type  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Scalar parent:**
[`THEOREM_V3_NEUTRAL_ROTOR_HARMONIC_GREEN_SEAM_v1.md`](../charge_gauss_native_em/THEOREM_V3_NEUTRAL_ROTOR_HARMONIC_GREEN_SEAM_v1.md)  
**Carrier parent:**
[`THEOREM_V3_TWO_RECORD_FULL_STF_TENSOR_CARRIER_BOUNDARY_v1.md`](THEOREM_V3_TWO_RECORD_FULL_STF_TENSOR_CARRIER_BOUNDARY_v1.md)  
**Constraint successor:**
[`THEOREM_V3_NEUTRAL_VECTOR_CONSTRAINT_WALKER_AND_TT_LOCALITY_OBSTRUCTION_v1.md`](THEOREM_V3_NEUTRAL_VECTOR_CONSTRAINT_WALKER_AND_TT_LOCALITY_OBSTRUCTION_v1.md)  
**Joint-bundle successor:**
[`THEOREM_V3_NEUTRAL_SCALAR_VECTOR_STF_BUNDLE_AND_COMMON_GREEN_SEAM_v1.md`](THEOREM_V3_NEUTRAL_SCALAR_VECTOR_STF_BUNDLE_AND_COMMON_GREEN_SEAM_v1.md)  
**Exact certificate:**
[`proof_v3_neutral_stf_rotor_walker_green_seam.py`](../../../../../scripts/proofs/proof_v3_neutral_stf_rotor_walker_green_seam.py)

---

## 1. From tensor readout to transported tensor payload

The parent carrier theorem proves that the symmetric trace-free cross stress
of two existing field controllers,

\[
 \Sigma(a,b)=\operatorname{STF}
 \left(E_aE_b^{\mathsf T}+E_bE_a^{\mathsf T}
      +B_aB_b^{\mathsf T}+B_bB_a^{\mathsf T}\right),       \tag{1}
\]

spans the full cubic spatial tensor representation

\[
 E_g\oplus T_{2g},\qquad \dim=5.                            \tag{2}
\]

That theorem supplied a clock-covariant composite readout but found that the
selected Phi-v2 collision does not protect it. The present theorem asks the
narrower constructive question: can the already certified neutral rotor move
such a payload locally without adding a carrier type?

The 192 field controllers split exactly into sixteen native period-twelve
clock orbits. Choose one orbit for the router and five explicit pairs from
distinct nonrouter orbits. In the integral five-coordinate STF chart their
layer-zero tensors are

\[
\begin{aligned}
 &(4,-2,0,0,3),\quad (4,-2,0,0,-3),\quad (6,-6,0,0,0),\\
 &(0,0,6,0,0),\quad (0,0,3,3,0).
\end{aligned}                                               \tag{3}
\]

Their exact matrix rank is five. The orbit indices and first representatives
are fixed before this rank check; no physical target or fitted number enters.

---

## 2. Eight-record marked site

For controller `q`, let

\[
 U(q)=\{(q,+),(q,-)\}                                      \tag{4}
\]

be one opposite-polarity neutral pair. A tensor-marked router site is

\[
 M(q;a,b)=U(q)\cup U(R^4q)\cup U(a)\cup U(b),              \tag{5}
\]

where `R` is the native period-twelve internal tick. The first two controller
pairs are the rotor and marker. The other two carry equation (1).

Because `a` and `b` lie in nonrouter clock orbits, `q,R^4q` is the unique
`R^4`-related pair among the four controllers throughout the clock. Thus the
instantaneous eight-bit state uniquely identifies the rotor/marker roles and
the unordered STF payload. Every `U` is opposite-polarity paired, so

\[
 E_{\rm additive}=B_{\rm additive}=0                       \tag{6}
\]

on every C3 layer. The payload is a retained composite label without a net
electromagnetic source.

---

## 3. Local transport theorem

Let a marked departure neighbor an unmarked destination `U(p)`. The selected
radius-one transaction is

\[
 \big(M(q;a,b),U(p)\big)
 \longmapsto
 \big(U(Rq),M(p;Ra,Rb)\big),                               \tag{7}
\]

with displacement given by the same served SC direction as the scalar rotor.
It has the following exact properties:

1. ten records occur across the two sites before and after;
2. all additive electric and magnetic fields remain zero;
3. the marked payload moves exactly one SC hop;
4. the combined C4/C3 clock transport leaves `Sigma(a,b)` covariantly
   constant; and
5. transforming the complete transaction by any of the 48 signed-cubic maps
   gives the transaction of the transformed state, with
   `Sigma -> M Sigma M^T`.

The certificate checks 720 selected local transactions and 240 signed-cubic
covariance rows. This is a finite carrier theorem, not yet the canonical
common law.

---

## 4. Tensor Green limit

Route sequential marked payloads from a source through a finite cubic domain
with absorbing exterior. Equation (7) reproduces the scalar rotor path at
every step. If `n_N(x)` is the visit count after `N` injections and `T` is one
of the basis tensors in equation (3), define

\[
 H_N(x)=T\,{n_N(x)\over6N}.                                \tag{8}
\]

The scalar parent theorem gives, componentwise,

\[
 \left\|L_D H_N-T\delta_s\right\|_\infty
 \le {8\over N}\max_A|T_A|.                               \tag{9}
\]

For every fixed finite domain, `H_N` therefore converges to the unique
Dirichlet solution with tensor source `T delta_s`. Since the five registered
`T` values span all spatial STF tensors and

\[
 \Lambda(k)=6-2\sum_{i=1}^3\cos k_i,                       \tag{10}
\]

the controlled large-domain history limit has a full-rank conditional static
kernel

\[
 \boxed{H_A(k)={T_A(k)\over\Lambda(k)}}.                   \tag{11}
\]

This is a deterministic tensor Green seam. It is not a graviton theorem: the
field is a normalized blocked-history readout, not a protected instantaneous
autonomous mode.

---

## 5. Exact remaining boundary

The result closes the gap between “a full STF composite can be named” and “a
full STF payload can be transported through a deterministic Green
construction.” It does **not** supply:

1. integration into the complete state-homogeneous Phi schedule;
2. native tensor-source preparation, renewal, and an owned sink;
3. a tensor-protecting autonomous collision;
4. a common tensor action or its physical normalization;
5. a common local scalar/vector constraint action and TT reduction;
6. coupling to a universally conserved material stress;
7. a shared matter/radiation cone; or
8. lensing and nonlinear self-coupling.

Gravity remains open at those eight debts. In particular, the Deser bootstrap
cannot supply the missing microscopic provenance.

The constraint successor now realizes a full rank-three neutral vector payload
and its componentwise blocked-history `1/Lambda` Green seam in the existing
carrier. It also proves that exact instantaneous finite-range TT projection is
impossible because the required lattice projector has a nonremovable
`1/Lambda` denominator. The remaining debt is therefore a propagated common
constraint action and its composition with the STF sector, not vector carrier
capacity.

The joint-bundle successor closes that last **carrier-composition** wording:
one intrinsically recognized ten-record neutral packet spans scalar plus STF
plus vector rank nine, moves by one reversible radius-one transaction, and
inherits one common componentwise Green history. It still does not generate
the constraint algebra, protect a tensor mode, or turn the history kernel into
an autonomous dynamical pole.

---

## 6. Reproduction

From the repository root:

```bash
python scripts/proofs/proof_v3_neutral_stf_rotor_walker_green_seam.py
```

Expected result: `12/12` exact checks pass.

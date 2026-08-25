# V3 cubic-covariance/transitive-Born-component obstruction v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — CENTRALIZER OF A FINITE TRANSITIVE CYCLE]** +
**[THEOREM — FAITHFUL NONABELIAN SIGNED-CUBIC ACTION ON THE V3 FIELD BANK]** +
**[THEOREM — FULL FAITHFUL CUBIC COVARIANCE OBSTRUCTS ONE TRANSITIVE
PREPARATION COMPONENT]** +
**[BOUNDARY — PHYSICAL CONTEXT, SYMMETRY QUOTIENT, ENVIRONMENT, OR EXPIRY
REQUIRED]** +
**[OPEN — NATIVE BORN PREPARATION AND APPARATUS]**  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Measure parent:**
[`THEOREM_V3_REVERSIBLE_SYNDROME_CONVEYOR_AND_BORN_MEASURE_BOUNDARY_v1.md`](../constituent_complete_matter/THEOREM_V3_REVERSIBLE_SYNDROME_CONVEYOR_AND_BORN_MEASURE_BOUNDARY_v1.md)  
**Prepared readout parent:**
[`THEOREM_V3_FIELD_BANK_GAUSSIAN_BORN_READOUT_v1.md`](THEOREM_V3_FIELD_BANK_GAUSSIAN_BORN_READOUT_v1.md)  
**Exact certificate:**
[`proof_v3_cubic_covariance_transitive_born_obstruction.py`](../../../../../scripts/proofs/proof_v3_cubic_covariance_transitive_born_obstruction.py)

---

## 1. Why transitivity is not a free cure

The finite-measure theorem says that one finite transitive cycle has one
invariant probability measure: uniform time weight on that cycle. It is
tempting to demand that the complete v3 bank simply be one such cycle and call
the Born preparation selected.

That proposal conflicts with exact full cubic covariance whenever the physical
component still carries the faithful cubic action.

Let `X` be a finite set and let `F` be one cycle of length `N=|X|`. If a
permutation `P` commutes with `F`, choose `k` so that

\[
 P(x_0)=F^k(x_0).                                      \tag{1}
\]

Every point is `F^j(x_0)`, so

\[
 P(F^j x_0)=F^jP(x_0)=F^{j+k}x_0.                     \tag{2}
\]

Therefore

\[
 \boxed{P=F^k,
 \qquad C_{\operatorname{Sym}(X)}(F)=\langle F\rangle\cong C_N.} \tag{3}
\]

The centralizer of one transitive cycle is cyclic and hence abelian.

---

## 2. The v3 bank carries a faithful nonabelian action

The selected site field bank has 384 exclusion channels:

\[
 6\ \text{tangents}
 \times4\ \text{axial normals}
 \times2\ \text{hands}
 \times4\ \text{C4 phases}
 \times2\ \text{polarities}
 =384.                                                  \tag{4}
\]

The 48 signed permutation matrices act on tangent and axial-normal labels,
with the required pseudoscalar hand transformation. The certificate proves:

1. all 48 induced channel permutations are distinct, so the action is
   faithful; and
2. two induced permutations fail to commute, so the action is nonabelian.

Singleton occupied-bank states embed the channel action in the full finite
power-set bank. Thus the induced bank action remains faithful and nonabelian
without enumerating `2^384` states.

Suppose one bank component `X` retained this faithful action, the deterministic
successor `F` were one transitive cycle on `X`, and exact cubic covariance held:

\[
 gF=Fg\qquad\text{for every }g\in O_h.                 \tag{5}
\]

Equation (5) places the nonabelian faithful image of `O_h` inside the cyclic
centralizer in equation (3), a contradiction. Hence

\[
 \boxed{
 \text{faithful full-cubic covariance}
 +\text{one transitive finite component}
 \quad\text{is impossible}.}                          \tag{6}
\]

---

## 3. What the theorem does and does not forbid

Equation (6) does **not** forbid a fully covariant microscopic law. It says
that such a law must have multiple symmetry-related cycles, a component on
which the symmetry acts through a smaller quotient, or a preparation process
that carries a physical frame/history.

The charged circulation example makes this concrete. Its 24 exact states are
six separate period-four cycles labeled by plane family and polarity. Within
one selected family/polarity context, the C4 successor is transitive and the
uniform time measure is unique. Across all six cycles, their mixture weights
remain free.

The full law can transform one framed cycle into another covariantly. What it
cannot do is pretend that no frame/sector was physically selected while also
claiming one unique global transitive preparation.

This is precisely the distinction between:

- an **ontic context**: a surviving frame, seed, apparatus record, environment
  history, or symmetry-broken state that selects the physical component; and
- an **epistemic grouping**: an observer later pooling records from distinct
  components. The second does not create the first.

---

## 4. Honest native-preparation branches

The obstruction leaves four structurally honest branches:

1. a physical frame or genesis seed selects a reduced-symmetry transitive
   component;
2. an environment retains which preparation sector was formed;
3. a noninjective expiry/basin rule selects one sector; or
4. the operational component carries only a nonfaithful symmetry quotient.

Each branch requires a concrete finite transaction and formation story. None
is licensed merely by choosing uniform weights after the experiment.

The theorem therefore narrows the Born gate: native preparation cannot be
“full symmetry + reversibility + global ergodicity” without additional physical
context. It does not derive Born probabilities, choose an apparatus, prove
no-signalling, or refute Bell experiments.

---

## 5. Reproduction

```bash
python scripts/proofs/proof_v3_cubic_covariance_transitive_born_obstruction.py
```

Expected result: `12/12` exact checks pass, with 48 faithful signed-cubic bank
actions, 604 centralizer rows, and six framed C4 cycles.

# Pre-registration: C18 uniform-counting joint source metric and Born-measure seam v1

**Date locked:** 2026-08-24  
**Status:** **[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]**  
**Production mutation:** none authorized  
**Ledger disposition:** no FTD claim row may be minted by this pass

## 1. Question

The prepared common-event generator writes electromagnetic current, tensor
stress, capacity depth, recoil, and clock action in one reversible
transaction, but independent canonical rescalings leave their displayed
source coefficients free. The physical Born constructions separately use a
uniform count over ordered residual-history pairs.

This protocol asks one narrower question:

> Does the existing uniform finite C18 counting measure induce one
> nondegenerate, coordinate-invariant quadratic metric on the complete
> electromagnetic/tensor/capacity source increment, and is the prepared Born
> count the pushforward of the corresponding uniform ordered-pair counting
> measure?

A positive answer is only a **bare action/measure seam**. It does not identify
the static Fisher Hessian with the dynamical field action, generate a physical
preparation ensemble, produce a Maxwell or tensor pole, establish lensing, or
measure a coupling.

## 2. Frozen sources

The execution must hash and freeze these sources before any checks:

| Source | SHA-256 |
|---|---|
| `scripts/proofs/proof_c18_uniform_token_blocking.py` | `60443BCD5E02E30E390F077C228B473E0A06D48E679C11822DB883ABF877BF30` |
| `scripts/proofs/proof_c18_common_phase_tensor_doublet.py` | `3D04960ACBEB54CB7D43F8FDDE597482685A058671ED887278F164C98A9ACABA` |
| `scripts/proofs/proof_c18_actualization_moment_source_vertex.py` | `6744132874E7785BB3E1474969AE519B0F319DA19961EE4126659014C603C4A2` |
| `scripts/proofs/proof_c4_coprime_ring_born_pushforward.py` | `59B9AED0F2FAF64609DB42F021B1F2498DB66BAE8B293099BABFE52478F2B802` |
| `THEOREM_C18_UNIFORM_TOKEN_BARE_BLOCKING_v1.md` | `819647BEE0A9043F7142E0022A4AB326677FBA886D7B14B4644204A234804349` |
| `THEOREM_C18_ACTUALIZATION_SHARED_MOMENT_SOURCE_VERTEX_v1.md` | `5EF07BBFEE3D77966279BC7E2A34E18F5B7C913ABF050A37CD9779F5EC52AC06` |
| `THEOREM_C4_COPRIME_RING_BORN_PUSHFORWARD_v1.md` | `0EDA9A309CB7BF02D74699712459527C1E2ABCB3782E6700977CC70A7EE742F1` |
| `THEOREM_HODGE_FRAMED_ALL_AXIS_CONSTRAINT_LIFT_AND_ONE_SIGNED_EVENT_GENERATOR_BOUNDARY_v1.md` | `AE02F7AAB30E5B8582003292B1B4B32621D04566402029DA4299CBC4E43A4322` |

Any mismatch fails before mathematical execution.

## 3. Frozen finite measure and readouts

Use the five-state record alphabet

\[
 \mathcal A_5=\{0,1,i,-1,-i\}
\]

with its uniform counting measure independently on the two directions of each
of the nine antipodal C18 lines. No Gibbs weight, target coupling, empirical
constant, master root, or fitted probability may enter.

Use the already registered block coordinates:

- relative phase currents \((R_u,R_v)\in\mathbb R^3\oplus\mathbb R^3\);
- common tensor quadratures \((Q,P)\in\operatorname{Sym}(3)^2\); and
- capacity tensor \(K\in\operatorname{Sym}(3)\).

In symmetric coordinates \((xx,yy,zz,xy,xz,yz)\), freeze

\[
 A_6=
 \begin{pmatrix}
 4&1&1&0&0&0\\
 1&4&1&0&0&0\\
 1&1&4&0&0&0\\
 0&0&0&1&0&0\\
 0&0&0&0&1&0\\
 0&0&0&0&0&1
 \end{pmatrix}.
\]

The preregistered covariance target is

\[
 \Sigma_R={4\over135}I_6,\qquad
 \Sigma_T=\operatorname{diag}(A_6/810,A_6/810),\qquad
 \Sigma_K=A_6/2025,
\]

and

\[
 \Sigma_{\rm joint}
 =\operatorname{diag}(\Sigma_R,\Sigma_T,\Sigma_K).
\]

All cross-covariances must be derived as zero from the uniform finite alphabet,
not imposed after the calculation.

## 4. Frozen event and invariant cost

For line direction \(d\), dyad \(M=dd^{\mathsf T}\), phase
\(i^k=u+iv\), and orientation \(\epsilon=\pm1\), use the existing event
increment

\[
 \delta R_u={\epsilon u\over9}d,\quad
 \delta R_v={\epsilon v\over9}d,
\]

\[
 \delta Q={u\over18}M,\quad
 \delta P={v\over18}M,\quad
 \delta K=-{1\over18}M.
\]

For each block \(X\in\{R,T,K\}\), define the coordinate-invariant
Mahalanobis half-cost

\[
 C_X={1\over2}\delta X^{\mathsf T}\Sigma_X^{-1}\delta X.
\]

The complete cost is \(C_{\rm joint}=C_R+C_T+C_K\).

## 5. Locked exact targets

The proof must derive, not insert, the following values for every phase and
orientation.

### 5.1 Signed-SC event orbit

\[
 C_R={5\over24},\qquad
 C_T={25\over72},\qquad
 C_K={125\over144},\qquad
 C_{\rm joint}={205\over144}.
\]

The trace/shear split must be

\[
 C_T^{\rm tr}={5\over72},\quad
 C_T^{\rm STF}={5\over18},\qquad
 C_K^{\rm tr}={25\over144},\quad
 C_K^{\rm STF}={25\over36}.
\]

### 5.2 FCC event orbit

\[
 C_R={5\over24},\qquad
 C_T={65\over144},\qquad
 C_K={325\over288},\qquad
 C_{\rm joint}={515\over288}.
\]

The trace costs must equal the SC values, while

\[
 C_T^{\rm STF}={55\over144},\qquad
 C_K^{\rm STF}={275\over288}.
\]

Thus the metric must be constant on each signed-cubic shell but must expose,
not hide, the bare SC/FCC shear anisotropy.

## 6. Reparameterization gate

For independent nonzero block rescalings

\[
 R\mapsto\lambda_RR,qquad
 (Q,P)\mapsto\lambda_T(Q,P),qquad
 K\mapsto\lambda_KK,
\]

the proof must transform both the source vector and covariance and verify

\[
 C_X'=C_X
\]

symbolically. Representative nonsingular rational mixing changes within the
six-component symmetric chart must also preserve the complete Mahalanobis
cost. This gate distinguishes the joint metric from the raw Euclidean norm
ratios already known to be convention dependent.

## 7. Counting-measure/Born gate

For a residual C4 bank with counts

\[
 (n_0,n_1,n_2,n_3),\qquad
 Z=(n_0-n_2)+i(n_1-n_3),
\]

the registered consecutive-ring orbit must visit every ordered address pair
exactly once. The pushforward of uniform counting measure on that orbit to
same-route, same-rail compatible events must give

\[
 M=|Z|^2=(n_0-n_2)^2+(n_1-n_3)^2.
\]

The proof must exhaust the frozen box \(0\le n_p\le4\) and at least four
multi-outcome banks. It must not read a target probability.

This establishes only a common **counting-measure candidate**: the quadratic
block metric is the Hessian of finite multiplicity and the prepared Born map
is an ordered-pair counting pushforward. Native bank formation, ergodicity,
source heralding, multipartite no-signalling, and amplification remain open.

## 8. Action and coupling firewall

The certificate must explicitly reject all of the following inferences:

1. \(C_R\) is the Maxwell action curvature \(\chi_{\rm EM}\);
2. any cost in section 5 is \(\alpha\), \(G_N\), or a lensing coefficient;
3. the static Fisher Hessian is already the interacting dynamical action;
4. uniform invariant counting measure is the physically prepared measure;
5. the SC/FCC difference may be averaged away without a derived blocking law;
6. the result produces a Maxwell pole, a tensor pole, static gravity, or
   stable matter; or
7. the master quadratic may be evaluated or substituted anywhere in the pass.

The next dynamical gate, if this protocol passes, is to derive a local
interacting block kernel whose kinetic Hessian equals or flows from this
metric while preserving the exact Maxwell cone, producing the constrained
two-mode tensor transfer, and generating the source bank/renewal orbit.

## 9. Preregistered outcomes

### Outcome A — exact bare action/measure seam

All covariance, rank, source-cost, reparameterization, shell, and prepared
Born-counting gates pass. The result may be stated as a coordinate-invariant
normalization **candidate** shared by the finite source vertex and the
prepared counting pushforward. Physical action selection remains open.

### Outcome B — partial metric seam

The joint metric is nondegenerate and invariant, but at least one source block
or counting-pushforward gate fails. Report the exact failing sector; do not
repair it after execution.

### Outcome C — no common seam

The full covariance is singular, the event cost depends on phase/orientation
within a cubic orbit, or the uniform ordered-pair pushforward fails. Preserve
the negative result.


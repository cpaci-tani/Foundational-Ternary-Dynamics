# Pre-registration — Local work port versus multicomponent factor discriminator v1

**Identifier:** `FTD-0981`  
**Date locked:** 2026-08-12  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE FIRST EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Question

FTD-0980 left three exact implementations of the oriented relative-field
quarter turn:

1. a modal/nonlocal stiffness root;
2. a site-local scalar root with a canonical work/history reservoir; or
3. added multicomponent factor hardware.

This gate discriminates branches 2 and 3. It asks two separate questions:

- can the production `C18` stiffness be factored by an exact finite-range
  multicomponent incidence/Dirac operator; and
- does that factor by itself make the **one-event** energy-compatible
  quarter turn finite-range, or is a phase-complete local work port still the
  minimum exact completion?

Factorization, a first-order generator, and a finite-time event map are not
to be conflated. No representation is adopted into production by this test.

## 2. Frozen sources

| Source | Frozen SHA-256 |
|---|---|
| `THEOREM_ORIENTED_SQUARE_ROOT_CLUTCH_AND_LOCALITY_ENERGY_TRILEMMA_v1.md` | `6C9082FD7C7E10E5A0767ECCB852B90BB84B5AAEFF2508376A347402E882264B` |
| `THEOREM_C18_FINITE_RANGE_CHARACTERISTIC_AND_RIGID_TRANSLATOR_OBSTRUCTION_v1.md` | `C6424C1AA0DDA2BA57BDE14A1559C76BBB17E279087122FB7121C59350BB4329` |
| `THEOREM_SELF_DUAL_RECIPROCAL_DISCRETE_ACTION_AND_FORMATION_RESERVOIR_BOUNDARY_v1.md` | `A7DC30C90C491976F58CDEAF71FB5ABFCE04952ECE971CA7FF72C65A7B9B90BF` |

The first source supplies the exact local-root work defect. The second
supplies the frozen production `C18` symbol. The third supplies the lower
bound that one scalar account is not a phase-complete reservoir and that at
least one complete canonical pair is required per independently formed
phase plane.

## 3. Frozen `C18` incidence factor

Use one representative from each undirected production bond:

\[
 F_+=\{(1,0,0),(0,1,0),(0,0,1)\},
\]

\[
 E_+=\{(1,1,0),(1,-1,0),(1,0,1),(1,0,-1),
       (0,1,1),(0,1,-1)\}.
\]

Assign

\[
 a_r=\begin{cases}
 1/9,&r\in F_+,\\
 1/18,&r\in E_+.
 \end{cases}
\]

The nine-channel one-sided incidence operator is

\[
 (Bq)_{x,r}=\sqrt{a_r}\,(q_{x+r}-q_x).                 \tag{1}
\]

Its Laurent norm must reproduce the exact positive production stiffness:

\[
 B^*B=K,
\quad
 K(z)=\sum_{r\in F_+\cup E_+}a_r(2-z^r-z^{-r}).        \tag{2}
\]

The certificate must compare equation (2) coefficient by coefficient with
FTD-0943's frozen symbol. It must also prove only the lower bound

\[
 m\geq\operatorname{rank}\operatorname{Hess}K(0)=3     \tag{3}
\]

for any analytic sum-of-squares factor with `m` real channels. The explicit
nine-channel witness is not to be called channel-minimal.

For `K_mu=K+mu^2 I`, register the self-adjoint block factor

\[
 {\cal D}_\mu=
 \begin{pmatrix}\mu I&B^*\\B&-\mu I_9\end{pmatrix},
\quad
 {\cal D}_\mu^2=
 \begin{pmatrix}K_\mu&0\\0&\mu^2I_9+BB^*\end{pmatrix}. \tag{4}
\]

Equation (4) is a selected representation witness, not a derivation of
fermions, spin, a Hilbert space, or a physical Dirac field.

## 4. Factor-versus-event discriminator

The energy-compatible quarter turn built from a self-adjoint factor `D`
has the phase-space form

\[
 J_D=\begin{pmatrix}0&-\sigma D^{-1}\\
                     \sigma D&0\end{pmatrix}.           \tag{5}
\]

The certificate must test whether the finite-range factor (4) removes the
inverse in equation (5).

- For `mu=0`, the vacuum mode makes `D_mu` singular.
- For `mu>0`, if `D_mu^{-1}` were finite-range, then the scalar block of
  `D_mu^{-2}` would make `K_mu^{-1}` a Laurent polynomial. But a nonconstant
  `K_mu` is not a unit in the Laurent ring; its reciprocal is not
  finite-range.

Thus the block factor may localize a first-order **generator**, but it does
not by itself produce the exact finite-range one-event map (5) in the
original scalar coordinate/momentum variables. Evading this conclusion by
declaring factor variables primitive, imposing a constraint subspace, or
replacing the event with a multi-tick first-order propagation is a different
representation/dynamics adoption and must be priced separately.

## 5. Frozen local canonical work-port lift

Let

\[
 z=(q,p)^T,\qquad
 \Omega=\begin{pmatrix}0&I\\-I&0\end{pmatrix},\qquad
 G=\begin{pmatrix}K&0\\0&I\end{pmatrix},                \tag{6}
\]

and use the site-local oriented root

\[
 R_\sigma=
 \begin{pmatrix}0&-\sigma I/\kappa\\
                 \sigma\kappa I&0\end{pmatrix}.         \tag{7}
\]

Define the exact before-minus-after energy matrix

\[
 B_0=G-R_\sigma^TGR_\sigma
 =\begin{pmatrix}K-\kappa^2I&0\\
                  0&I-K/\kappa^2\end{pmatrix}.          \tag{8}
\]

Give one independently gated batch one complete work pair `(theta,I_R)`.
To prove that the work debit is part of a canonical map rather than a scalar
journal entry, define a finite-range symplectic seam family. Put

\[
 B_q=K-\kappa^2I,\qquad B_p=I-K/\kappa^2,
\]

\[
 Q_s=\begin{pmatrix}I&-sB_p\\0&I\end{pmatrix},\qquad
 P_s=\begin{pmatrix}I&0\\sB_q&I\end{pmatrix},
\quad S_s=Q_sP_s,\quad R_s=R_\sigma S_s.                \tag{9}
\]

Every map in (9) is finite-range and symplectic. At the registered crossing
`s=theta-theta_*=0`, `R_s=R_sigma` and

\[
 R_s^T\Omega\,\partial_sR_s\big|_{s=0}=B_0.            \tag{10}
\]

For general `s`, let

\[
 B_s=R_s^T\Omega\,\partial_sR_s.                        \tag{11}
\]

The lifted discrete event is

\[
 z'=R_sz,\qquad
 \theta'=\theta,\qquad
 I_R'=I_R+\frac12z^TB_sz.                               \tag{12}
\]

The certificate must prove that equation (12) preserves

\[
 \Omega+d\theta\wedge dI_R                              \tag{13}

\]
exactly. At the crossing it must reduce to

\[
 z'=R_\sigma z,\qquad
 I_R'=I_R+H(z)-H(R_\sigma z),                            \tag{14}
\]

so

\[
 H(z')+I_R'=H(z)+I_R.                                   \tag{15}
\]

Equation (12) must have a finite-range exact inverse. No passive diagnostic
counter qualifies; the phase dependence in (9)--(12) is the reciprocal
reaction required by symplecticity.

## 6. Four-cycle recovery, positivity, and history

At repeated registered crossings of the same retained orientation,

\[
 z_m=R_\sigma^m z_0,
\qquad
 I_{R,m}=I_{R,0}+H(z_0)-H(z_m).                          \tag{16}
\]

Because `R_sigma^4=I`, equation (16) must give exact four-cycle recovery of
both field and work reserve. At each intermediate stroke the reserve must
remain nonnegative. The ready-domain condition is

\[
 I_{R,0}\geq
 \max_{m=1,2,3,4}\bigl(H(z_m)-H(z_0),0\bigr).           \tag{17}
\]

No finite reserve covers an unbounded-amplitude state space. A physical gate
therefore needs a preregistered compliance shell and must fail closed when
(17) is violated. The work pair does not replace the separate ternary
orientation/history record required by FTD-0980.

## 7. Frozen checks

- **G1:** protocol/source hashes and scope markers;
- **G2:** exact nine-channel incidence identity and Hessian-rank lower bound;
- **G3:** exact block-Dirac square and non-promotion markers;
- **G4:** massless singularity and massive finite-range inverse obstruction;
- **G5:** local seam shears, root, tangent matrix, and finite-range closure;
- **G6:** exact extended symplecticity, inverse, and total-energy identity;
- **G7:** four-cycle telescoping recovery and finite-reserve ready domain;
- **G8:** minimum-one-canonical-pair conclusion conditional on FTD-0928;
- **G9:** no production, `G*`, Born/Bell, Hilbert, fermion, mass, or
  completeness promotion.

No numerical search, coefficient fit, near-miss comparison, or engine
mutation is permitted.

## 8. Frozen classifier

- **Outcome A — factor-only local closure:** the multicomponent factor makes
  the exact one-event energy-compatible root finite-range without an inverse,
  new constraint, work pair, or changed dynamics.
- **Outcome B — minimum canonical work-port closure:** the finite-range
  incidence/Dirac factor exists but does not localize the inverse required by
  the exact one-event root; one complete phase/action work pair per
  independently gated batch gives an exact local canonical energy completion
  on a finite-reserve ready domain and recovers after four strokes.
- **Outcome C — neither branch closes:** the factor fails and no exact local
  canonical work-port lift exists.
- **Outcome D — invalid:** a hash, identity, locality, inverse, or scope gate
  fails.

The expected result is Outcome B. Even Outcome B is a reference-mechanism
theorem only: native formation, replenishment, synchronization, CPU/GPU
realization, stability, and operational hiding remain open.

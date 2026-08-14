# Theorem — Oriented square-root clutch and locality/energy trilemma v1

**Identifiers:** `FTD-0979/0980`  
**Date:** 2026-08-12  
**Status:** `[THEOREM — EXACT MINIMUM ORIENTED ROOTS OF THE PRODUCTION HALF-TURN]` +
`[THEOREM — ENERGY-COMPATIBLE SPECTRAL COMPLEX STRUCTURE]` +
`[THEOREM — NO SCALAR FINITE-RANGE ENERGY-COMPATIBLE C18 ROOT]` +
`[BOUNDARY — LOCALITY/ENERGY/HISTORY TRILEMMA]` +
`[SELECTION — REVERSIBLE TERNARY CLOCK-SEAM CLUTCH]` +
`[OPEN — LOCAL WORK RESERVOIR OR MULTICOMPONENT FACTOR HARDWARE/FORMATION/PRODUCTION]`

## Result

The missing clockwise/counterclockwise operation is not another independent
degree of freedom. It is the oriented symplectic square root of the existing
production left/right swap.

For each complete dual canonical component there are exactly two such roots
within the real orthogonal symplectic class that leaves the common mode fixed:

\[
 R_+^2=R_-^2=S,qquad
 R_- = R_+^{-1},qquad
 R_\pm^4=I.                                             \tag{1}
\]

They are distinguished by one ternary orientation value
`sigma in {-1,+1}`. Time reversal exchanges them. The production half-turn
`S` is their common square and therefore forgets `sigma`.

However, exact energy preservation determines the root from the field
stiffness. For

\[
 H_K={1\over2}(P^TP+D^TKD),
\]

the oriented roots are

\[
 {cal J}_{K,\sigma}=
 \begin{pmatrix}
 0&-\sigma K^{-1/2}\\
 \sigma K^{1/2}&0
 \end{pmatrix}.                                         \tag{2}
\]

On the scalar dispersive `C18` field, `K^{1/2}` has no finite-range
translation-invariant Laurent representation. Thus equation (2) is exact
but modal/nonlocal. A site-local root using one scalar frequency `kappa`
instead has the exact work defect

\[
 \Delta H={1\over2}D^T(\kappa^2I-K)D
          +{1\over2}P^T(K/\kappa^2-I)P.                  \tag{3}
\]

Equation (3) vanishes for every state iff `K=kappa^2 I`. Therefore an exact
production clutch must choose among:

1. a nonlocal/modal root;
2. a local root with a physical canonical reservoir that books equation (3)
   and retains its history; or
3. separately selected multicomponent factor hardware whose square is the
   scalar stiffness.

The current ontology does not yet choose among these branches.

## Certificate of record

- Parent pre-registration:
  [`PREREG_ORIENTED_SQUARE_ROOT_CLUTCH_AND_LOCALITY_ENERGY_TRILEMMA_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_ORIENTED_SQUARE_ROOT_CLUTCH_AND_LOCALITY_ENERGY_TRILEMMA_v1.md),
  SHA-256 `5747E0991BD6984B86B8A9522AD3F9B2927E8AADEDEF0D50C2C826DF7EA185C4`.
- Immutable parent proof:
  [`proof_oriented_square_root_clutch_locality_energy_trilemma.py`](../../../../../scripts/proofs/proof_oriented_square_root_clutch_locality_energy_trilemma.py),
  SHA-256 `814B2B6760E29129BA6616AE1BC6CC047D6DCFD20BCFDCDA8BCC054D9A3D2C92`.
- First execution: 44 displayed substantive passes, then a verifier-only
  Python tuple-attribute exception; no final classifier.
- Repair pre-registration:
  [`PREREG_ORIENTED_SQUARE_ROOT_CLUTCH_CERTIFICATE_REPAIR_v2.md`](../../preregistrations/native_time_carrier_programme/PREREG_ORIENTED_SQUARE_ROOT_CLUTCH_CERTIFICATE_REPAIR_v2.md),
  SHA-256 `D98611D1BB42D3CA61CCE17964C405C7E0832BD16DFF7D47882C1C5D6FE5D985`.
- In-memory wrapper:
  [`proof_oriented_square_root_clutch_locality_energy_trilemma_repaired.py`](../../../../../scripts/proofs/proof_oriented_square_root_clutch_locality_energy_trilemma_repaired.py),
  SHA-256 `7244C1249D2BE592638883E92C1545120A4F515B702921E45B9CBEE328E94A3C`.
- Repaired execution: inherited `58/58` plus repair integrity `16/16`,
  Outcome B.
- Production mutation: none.

## 1. The oriented root in existing variables

For one Cartesian component write

\[
 z=(q_L,q_R,p_L,p_R)^T,                                  \tag{4}
\]

where the `q` variables are dual flux and the `p` variables are their dual
wave velocities. Introduce

\[
 q_C={q_L+q_R\over\sqrt2},\quad q_D={q_L-q_R\over\sqrt2},
 \qquad
 p_C={p_L+p_R\over\sqrt2},\quad p_D={p_L-p_R\over\sqrt2}. \tag{5}
\]

The production swap leaves `(q_C,p_C)` fixed and negates `(q_D,p_D)`.
The positive oriented root leaves the common pair fixed and acts on the
relative pair by

\[
 J_+=\begin{pmatrix}0&-1\\1&0\end{pmatrix}.              \tag{6}
\]

In raw variables this is

\[
 R_+={1\over2}
 \begin{pmatrix}
 1&1&-1&1\\
 1&1&1&-1\\
 1&-1&1&1\\
 -1&1&1&1
 \end{pmatrix}.                                          \tag{7}
\]

Equivalently,

\[
\begin{aligned}
q_L'&=(q_L+q_R-p_L+p_R)/2,\\
q_R'&=(q_L+q_R+p_L-p_R)/2,\\
p_L'&=(p_L+p_R+q_L-q_R)/2,\\
p_R'&=(p_L+p_R-q_L+q_R)/2.
\end{aligned}                                             \tag{8}
\]

The reverse-oriented root is `R_-=R_+^{-1}`. The exact certificate proves

\[
 R_\sigma^T\Omega R_\sigma=\Omega,qquad
 R_\sigma^TR_\sigma=I,qquad
 \det R_\sigma=1.                                        \tag{9}
\]

Thus no new continuous pair is needed merely to represent the root. The root
mixes relative coordinate and conjugate momentum; a left/right label swap
alone cannot do that.

## 2. Minimum and time reversal

Every real `2 x 2` orthogonal symplectic matrix has the form

\[
 A=\begin{pmatrix}a&-b\\b&a\end{pmatrix},qquad
 a^2+b^2=1.                                               \tag{10}
\]

Requiring `A^2=-I` gives

\[
 a^2-b^2=-1,qquad 2ab=0,
\]

hence

\[
 a=0,qquad b=\pm1.                                      \tag{11}
\]

So `+J` and `-J` are the only roots in the registered class. The missing
datum is exactly one orientation sign, not a continuum of new dynamics.

For standard oscillator time reversal

\[
 \Theta(q_C,q_D,p_C,p_D)=(q_C,q_D,-p_C,-p_D),             \tag{12}
\]

one has

\[
 \Theta R_\sigma\Theta=R_{-\sigma}=R_\sigma^{-1},
 \qquad
 \Theta S\Theta=S.                                       \tag{13}
\]

The square is time-reversal even because it has forgotten the oriented root.

## 3. Reversible ternary clutch

Let `h in {-1,0,+1}` be a self-delimiting orientation latch and `r` a blank
receiving record. On the ready domain, the minimum reusable handshake is

\[
 (h,r,z)=(\sigma,0,z)
 \longmapsto
 (0,\sigma,R_\sigma z).                                  \tag{14}
\]

Its inverse is

\[
 (0,\sigma,z')
 \longmapsto
 (\sigma,0,R_{-\sigma}z').                               \tag{15}
\]

It preserves the latch norm `h^2+r^2` and never identifies the two signs.
One retained latch is enough if it is not reset. If the latch must return to
blank, mapping both signs to zero without a receiving record is noninjective.
The orientation must move into a second ternary record or an exported history
rail. This is unactualization as reversible relevance transfer, not deletion.

## 4. Energy selects the complex structure

For one ultralocal oscillator

\[
 H_\kappa={1\over2}(P^2+\kappa^2D^2),qquad \kappa>0,      \tag{16}
\]

the energy-compatible roots are

\[
 J_{\kappa,\sigma}=
 \begin{pmatrix}
 0&-\sigma/\kappa\\
 \sigma\kappa&0
 \end{pmatrix}.                                          \tag{17}
\]

They are symplectic, square to `-I`, and preserve equation (16). The simple
integer matrix `J` appears only after selecting units with `kappa=1`.

For a full positive stiffness `K`, the same calculation uniquely suggests
equation (2). Direct multiplication gives

\[
 {cal J}_{K,\sigma}^2=-I,qquad
 {cal J}_{K,\sigma}^T\Omega{cal J}_{K,\sigma}=\Omega,
 \qquad
 H_K({\cal J}_{K,\sigma}X)=H_K(X).                        \tag{18}
\]

Zero modes lie outside `K^{-1/2}` and need an independently declared zero-mode
sector or regulator. They may not be silently included.

## 5. Why exact energy conflicts with scalar finite range

Restrict the homogeneous massive scalar `C18` stiffness to one coordinate
axis. Its Laurent symbol has the form

\[
 k_\mu(z)=\mu^2+c^2(2-z-z^{-1}),qquad c\ne0.             \tag{19}
\]

If a finite-range scalar `B` represented `K^{1/2}`, its Laurent symbol
`b(z)` would satisfy `b(z)^2=k_mu(z)`. If the greatest and least exponents of
`b` are `m,n`, those of its square are `2m,2n`. Equation (19) instead has
greatest and least exponents `+1,-1`. The equations

\[
 2m=1,qquad2n=-1                                         \tag{20}
\]

have no integer solutions. The mass affects only the exponent-zero
coefficient and cannot repair the obstruction.

This is an all-finite-range proof, not a coefficient search. It extends the
FTD-0943 massless scalar-square obstruction to the massive one-axis slice
needed by the clutch.

## 6. Exact price of insisting on locality

Apply the site-local root (17) to a field with general stiffness `K`:

\[
 D'=-\sigma P/\kappa,qquad P'=\sigma\kappa D.             \tag{21}
\]

Substitution into `H_K` gives equation (3). The coefficients of every
quadratic state variable vanish simultaneously iff

\[
 K=\kappa^2I.                                             \tag{22}
\]

The `C18` stiffness is dispersive, so equation (22) does not hold. A local
quarter-turn is still a legitimate symplectic event, but it is not free. A
physical implementation must debit or credit the exact amount (3) through a
canonical source/reaction reservoir and retain enough state for the inverse.
A scalar diagnostic ledger alone is insufficient.

The alternative is to factor `K` with additional components. FTD-0943 already
identified this as a possible Clifford/Dirac-type branch. Such hardware may
restore first-order locality, but it is an added representation with its own
doubling, positivity, formation, covariance, and production debts; it is not
derived here.

## 7. Clock-seam law

Conditionally on a maintained clock seam of length `L_delta`, the simplest
reference clutch is

\[
 (\delta,\Pi,z,\sigma,0)
 \longmapsto
 (\delta-\sigma L_\delta,\Pi,R_\sigma z,0,\sigma).         \tag{23}
\]

On its ready domain it is symplectic on the continuous variables, exactly
invertible, and time-reversal covariant. Four same-orientation crossings
return the field pair because `R_sigma^4=I`, while four exported ternary
records preserve the history rather than erasing it.

Equation (23) is the minimum coherent reference law found here. It is not yet
a production law. The clock must supply the crossing, the latch must supply
orientation, the field stiffness must supply the correct metric, and any
local approximation must supply the work reservoir.

`G*` can enter only through the separately maintained critical-clock period
and hence the eligibility cadence. It does not select the sign, stiffness
root, normalization, field representation, or work transaction.

## 8. Epistemic disposition

Established:

- **[THEOREM]** the production half-turn has exactly two oriented roots in
  the registered fixed-common orthogonal symplectic class;
- **[THEOREM]** the roots are represented using existing dual coordinate and
  momentum variables and square exactly to the current swap;
- **[THEOREM]** energy compatibility gives the stiffness-dependent complex
  structure (2);
- **[THEOREM]** no scalar finite-range translation-invariant factor realizes
  that root for massive dispersive `C18` stiffness;
- **[THEOREM]** a local scalar root has the exact work defect (3);
- **[BOUNDARY]** exact scalar locality, zero-work full-energy preservation,
  and reset without retained history cannot all be had in this class; and
- **[SELECTION]** equation (23) is a reversible reference clutch.

Still open:

- choose and derive a physical local work reservoir or a multicomponent
  first-order factor representation;
- couple that choice to the single FTD-0977 covariant clock momentum;
- form and maintain the clock, latch, relative mode, and reserve from
  substrate dynamics;
- prove finite-tick energy/current closure, perturbative stability, causal
  CPU/GPU realization, repeated-cycle recovery, and operational hiding; and
- establish any `G*`, Born, Bell, mass, or physical-clock identification.

No Hilbert-space recovery or framework-completeness claim follows.

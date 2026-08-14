# Pre-registration — Oriented square-root clutch and locality/energy trilemma v1

**Identifier:** `FTD-0979`  
**Date locked:** 2026-08-12  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE FIRST EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Question

FTD-0978 proved that the unchanged production left/right exchange is the
order-two relative half-turn

\[
 S_{\rm rel}=-I=J^2=(-J)^2.
\]

What is the minimum oriented symplectic square root of that existing
hardware, and can it be simultaneously:

1. clock-indexed and reversible;
2. strictly finite-range/local on the scalar `C18` relative field;
3. exactly preserving the full dispersive field energy; and
4. resettable without erasing its clockwise/counterclockwise record?

The test constructs the root before testing its locality and energy price.
It does not alter production or assume that mathematical representability is
physical formation.

## 2. Frozen sources

| Source | Frozen SHA-256 |
|---|---|
| `THEOREM_PRODUCTION_CLOCK_INDEXED_C4_TWIST_CENSUS_v1.md` | `3873CEE3BD61C894A99857C0527FBC1082F244CE7E7890FEB3E2F01C6D64E58F` |
| `THEOREM_C18_FINITE_RANGE_CHARACTERISTIC_AND_RIGID_TRANSLATOR_OBSTRUCTION_v1.md` | `C6424C1AA0DDA2BA57BDE14A1559C76BBB17E279087122FB7121C59350BB4329` |
| `THEOREM_KRYLOV_DEGENERACY_TERNARY_LATCH_AND_ORIENTED_C4_TRANSITION_v1.md` | `7DA2366C75D38E0EA1F8012632D71C676C4E6F8D1A7F8D1467EAF4185AE77194` |
| `THEOREM_ONE_CLOCK_C4_COTANGENT_LIFT_AND_CONNECTION_UNDERDETERMINATION_v1.md` | `9D80C133F5D99D0F789C320DC7C2C2A9E41C4DBB56FAECD39054B7BF0DB69E7F` |

No production source is modified. FTD-0978's complete phase-space correction
is controlling: the full swap is symplectic with determinant `+1`; its defect
is lost orientation, not determinant parity.

## 3. Frozen canonical root

For one component use

\[
 z=(q_L,q_R,p_L,p_R)^T
\]

and the orthonormal common/relative chart

\[
 q_C={q_L+q_R\over\sqrt2},\quad q_D={q_L-q_R\over\sqrt2},
 \qquad
 p_C={p_L+p_R\over\sqrt2},\quad p_D={p_L-p_R\over\sqrt2}. \tag{1}
\]

Let

\[
 J_+=\begin{pmatrix}0&-1\\1&0\end{pmatrix},
 \qquad J_-=J_+^{-1}=-J_+.                                \tag{2}
\]

Define `R_sigma`, `sigma in {-1,+1}`, to leave `(q_C,p_C)` fixed and act by
`J_sigma` on `(q_D,p_D)`. The certificate must verify

\[
 R_\sigma^T\Omega R_\sigma=\Omega,quad
 R_\sigma^{-1}=R_{-\sigma},quad
 R_\sigma^2=S,quad R_\sigma^4=I.                         \tag{3}
\]

In raw coordinates the `sigma=+1` root is frozen as

\[
\begin{aligned}
q_L'&=(q_L+q_R-p_L+p_R)/2,\\
q_R'&=(q_L+q_R+p_L-p_R)/2,\\
p_L'&=(p_L+p_R+q_L-q_R)/2,\\
p_R'&=(p_L+p_R-q_L+q_R)/2.                 \tag{4}
\end{aligned}
\]

Within the class of real orthogonal symplectic maps which fix the common pair
pointwise, the two maps `R_+` and `R_-` must be the only square roots of the
production swap. The orientation bit is therefore exactly `sigma`.

## 4. Time reversal and reusable ternary handshake

Use standard oscillator time reversal

\[
 \Theta(q_C,q_D,p_C,p_D)=(q_C,q_D,-p_C,-p_D).              \tag{5}
\]

The root must satisfy

\[
 \Theta R_\sigma\Theta=R_{-\sigma}=R_\sigma^{-1},
 \qquad \Theta S\Theta=S.                                 \tag{6}
\]

For a reusable self-delimiting gate, let `h in {-1,0,+1}` be the incoming
orientation latch and `r in {-1,0,+1}` a blank receiving record. On the ready
domain `(h,r)=(sigma,0)`, register

\[
 (\sigma,0,z)\longmapsto(0,\sigma,R_\sigma z).             \tag{7}
\]

Equation (7) must have the exact inverse

\[
 (0,\sigma,z')\longmapsto
 (\sigma,0,R_{-\sigma}z').                                 \tag{8}
\]

One retained latch is enough if it is never reset. If the ready latch must
return to blank, injectivity requires the orientation to move to another
record or exported history channel; mapping both signs to blank without such
a receiver is forbidden.

## 5. Energy-compatible roots

### 5.1 One ultralocal oscillator

For

\[
 H_\kappa={1\over2}(p_D^2+\kappa^2q_D^2),\qquad\kappa>0,   \tag{9}
\]

the energy-compatible oriented root is

\[
 J_{\kappa,+}=
 \begin{pmatrix}0&-1/\kappa\\\kappa&0\end{pmatrix},
 \qquad J_{\kappa,-}=J_{\kappa,+}^{-1}.                   \tag{10}
\]

It must be symplectic, square to `-I`, preserve equation (9), and reduce to
equation (2) only after a selected unit normalization `kappa=1`.

### 5.2 Full positive stiffness

For a finite or spectrally restricted positive field stiffness `K`, define

\[
 H_K={1\over2}(p^Tp+q^TKq).                                \tag{11}
\]

The exact energy-compatible root is

\[
 {cal J}_{K,+}=
 \begin{pmatrix}0&-K^{-1/2}\\K^{1/2}&0\end{pmatrix},
 \qquad {cal J}_{K,-}={\cal J}_{K,+}^{-1}.                \tag{12}
\]

The certificate must verify symplecticity, square `-I`, and exact preservation
of equation (11). Zero modes are outside the inverse-square-root domain and
must be treated separately rather than silently regularized.

## 6. Locality/energy discriminator

For a homogeneous scalar massive `C18` component, restrict the Laurent symbol
to one axis with the other two wave numbers zero. Up to positive nonzero wave
coefficient `c^2`, the stiffness has the form

\[
 k_\mu(z)=\mu^2+c^2(2-z-z^{-1}).                            \tag{13}
\]

If a scalar finite-range translation-invariant `B` represented `K^{1/2}`, its
Laurent symbol `b(z)` would satisfy `b(z)^2=k_mu(z)`. A nonzero Laurent square
has even highest and lowest exponents, while equation (13) has extremal
exponents `+1` and `-1`. The certificate must prove the contradiction for
every `mu` and every `c != 0`, without coefficient search.

Therefore equation (12) is modal/nonlocal in the registered scalar class.
FTD-0943 already proves the massless rank-three version and the exact
kick--drift characteristic obstruction; this gate supplies the massive
one-axis parity proof needed here.

If a site-local root instead uses one selected scalar `kappa`, its exact full
energy defect is frozen as

\[
 \Delta H={1\over2}q^T(\kappa^2I-K)q
          +{1\over2}p^T(K/\kappa^2-I)p.                    \tag{14}
\]

Equation (14) vanishes for all states iff `K=kappa^2 I`. A dispersive `C18`
field therefore requires either:

1. the nonlocal/modal root (12);
2. a local root plus an explicit work/reaction/history reservoir that books
   equation (14); or
3. additional multicomponent factor hardware, such as a separately selected
   Clifford/Dirac-type representation, subject to its own locality, energy,
   doubling, and production tests.

The protocol does not select among these branches.

## 7. Clock seam

Conditionally on a maintained clock coordinate `delta` with periodic seam
length `L_delta`, an oriented crossing with retained `sigma` may use

\[
 (\delta,\Pi,z,\sigma,0)
 \longmapsto
 (\delta-\sigma L_\delta,\Pi,R_\sigma z,0,\sigma).          \tag{15}
\]

On its ready domain equation (15) is symplectic on continuous variables,
time-reversal covariant, exactly invertible, and repeats after four
same-orientation crossings on the field pair. It is a selected reference
clutch, not a production derivation. `G*` may set the maintained clock's
quarter cadence only after its separate clock-compliance gate; it does not
choose `sigma`, `K^{1/2}`, `kappa`, or the work reservoir.

## 8. Frozen checks

- **G1:** protocol and frozen-source hashes plus scope markers;
- **G2:** exact raw/common-relative roots, symplecticity, order, square, and
  uniqueness in the frozen orthogonal class;
- **G3:** time reversal and exact ternary handshake/inverse/minimum receiver;
- **G4:** ultralocal `J_kappa` symplecticity and energy preservation;
- **G5:** full-stiffness spectral root and zero-mode boundary;
- **G6:** massive scalar finite-range Laurent-square obstruction;
- **G7:** exact local-root energy defect and iff discriminator;
- **G8:** clock-seam symplecticity, inverse, four-cycle, and history retention;
- **G9:** no production, `G*`, Born/Bell, Hilbert, mass, or completeness
  promotion.

No numerical search, fit, near-miss comparison, or engine mutation is
permitted.

## 9. Frozen classifier

- **Outcome A — local zero-work clutch:** one existing-type finite-range root
  is oriented, exactly preserves the full dispersive energy, retains its
  inverse record, and needs no new transaction.
- **Outcome B — exact root / locality-energy-history trilemma:** the two
  oriented roots are exact and minimum, but the energy-compatible scalar root
  is modal/nonlocal; a local root requires booked work/history or additional
  factor hardware. The selected ternary seam clutch is a coherent reference
  mechanism only.
- **Outcome C — no coherent root:** the production half-turn has no exact
  symplectic oriented square root on the registered phase-complete state.
- **Outcome D — invalid:** a hash, exact identity, source marker, or scope gate
  fails.

The expected result is Outcome B.

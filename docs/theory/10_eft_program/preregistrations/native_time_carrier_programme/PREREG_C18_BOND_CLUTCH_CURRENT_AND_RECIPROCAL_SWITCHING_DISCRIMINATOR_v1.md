# FTD-0988 — Preregistration: C18 bond clutch, current, and reciprocal switching discriminator v1

**Identifier:** `FTD-0988`  
**Date locked:** 2026-08-12  
**Status before execution:** `[PREREGISTERED — NOT YET EVIDENCE]`  
**Expected classifier:** **Outcome B — exact local reference law / physical ownership still selected**

## 1. Question

FTD-0987 identifies an existing body-frame longitudinal common canonical pair
but proves that unchanged production does not own or protect it. Its dense
projector is only a selected reference candidate. This discriminator asks:

1. can the exact `C18` incidence factor produce a Moore-local bond clutch;
2. does that clutch give a positive regional Hamiltonian, an antisymmetric
   local energy current, exact boundary isolation, and a reciprocal switching
   ledger;
3. can switching be made naturally work-free at a local zero-strain crossing
   while retaining clockwise/counterclockwise information;
4. does the fixed-clutch production kick--drift have an exact local shadow
   energy and inverse; and
5. what is the correct action normalization of an actually isolated regional
   oscillator?

No engine or production mutation is authorized by this protocol.

## 2. Frozen sources

| source | SHA-256 |
|---|---|
| `THEOREM_NATIVE_COMMON_MODE_WORK_PAIR_AND_PRODUCTION_OWNERSHIP_BOUNDARY_v1.md` | `47C859191CCC1D9E306F82A68B6FC76A128593E6BAA7CC05D871D5DEEEE7EBAC` |
| `THEOREM_LOCAL_CANONICAL_WORK_PORT_AND_C18_FACTOR_EVENT_BOUNDARY_v1.md` | `3BF425E7F826844BDD1F87ACA3B57EE9A26704996CC8A6F7781C683477D3B994` |
| `THEOREM_FLUX_WAVE_VELOCITY_MARKOV_CANONICAL_CARRIER_AND_PRODUCTION_BOUNDARY_v1.md` | `656F51A4E5A533C0436E932B452A33810CD851D63E571621DF81ECB0C9BED622` |
| `THEOREM_C18_FINITE_RANGE_CHARACTERISTIC_AND_RIGID_TRANSLATOR_OBSTRUCTION_v1.md` | `C6424C1AA0DDA2BA57BDE14A1559C76BBB17E279087122FB7121C59350BB4329` |
| `THEOREM_KRYLOV_DEGENERACY_TERNARY_LATCH_AND_ORIENTED_C4_TRANSITION_v1.md` | `7DA2366C75D38E0EA1F8012632D71C676C4E6F8D1A7F8D1467EAF4185AE77194` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |

Any mismatch invalidates execution. A repair must preserve this file and the
first certificate byte-for-byte.

## 3. Registered mathematical class

Choose one orientation for each undirected face/edge bond of the `C18`
graph. With `a_b=1/9` on face representatives and `a_b=1/18` on edge
representatives, let

\[
 (Bq)_b=\sqrt{a_b}(q_y-q_x),\qquad K=B^TB.             \tag{1}
\]

For a ternary bond latch `ell_b in {-1,0,+1}`, define its transmission and
the diagonal channel gate by

\[
 g_b=1-\ell_b^2\in\{0,1\},\qquad G_\ell=\operatorname{diag}(g_b),
 \qquad K_\ell=B^TG_\ell B.                            \tag{2}
\]

Thus `ell_b=0` is the unchanged transmitting bond and `ell_b=+/-1` is a cut
bond whose sign remains an orientation record. This convention must not be
silently reversed after execution.

For fixed latches the candidate scalar common-sector Hamiltonian is

\[
 H_\ell(q,p)=\frac12p^Tp+\frac12q^TK_\ell q.           \tag{3}
\]

The protocol concerns the body-frame longitudinal common scalar
`q_x=e_3 dot(J_L+J_R)/sqrt(2)` and its canonical momentum. It is conditional
on the regular regional frame already proved by FTD-0969/0970. It does not
derive that body's formation or distribute its frame instantaneously.

## 4. Exact gates

### G1 — source and inherited-claim lock

- all eight hashes match;
- the inherited `K=B^*B`, canonical flux/wave-velocity, compact-mode no-go,
  existing common pair, and retained ternary orientation claims are present;
- production still has no bond ownership latch, gate matrix, port reserve,
  switching-work ledger, or inverse transaction.

### G2 — local positive clutch

Prove for every latch assignment:

\[
 K_\ell^T=K_\ell,qquad
 q^TK_\ell q=\|G_\ell^{1/2}Bq\|^2\ge0,                \tag{4}
\]

and `K_ell` has C18 range. For a finite region `Lambda`, cut exactly the bonds
with one endpoint in `Lambda` and one outside. Prove

\[
 K_\ell=K_\Lambda\oplus K_{\Lambda^c}.                 \tag{5}
\]

This is a local boundary cut, not the dense one-mode projector of FTD-0987.

### G3 — local continuous-time current

With `a_xy=a_yx` and `g_xy=g_yx`, assign

\[
 h_x=\frac12p_x^2+rac14\sum_{y\sim x}
 g_{xy}a_{xy}(q_y-q_x)^2                               \tag{6}
\]

and preregister the oriented bond current

\[
 {\cal J}_{x\to y}=
 \frac12g_{xy}a_{xy}(q_x-q_y)(p_x+p_y).                \tag{7}
\]

Prove antisymmetry and

\[
 \dot h_x+\sum_{y\sim x}{\cal J}_{x\to y}=0.          \tag{8}
\]

For a region, the energy change must be the negative boundary flux. A cut
boundary must have identically zero flux.

### G4 — switching work and the zero-strain seam

At fixed `(q,p)`, changing latches has exact work

\[
 W_{\ell\to\ell'}=
 \frac12q^T(K_{\ell'}-K_\ell)q
 =\frac12\sum_b(g_b'-g_b)a_b(q_y-q_x)^2.               \tag{9}
\]

An unbooked off-seam switch must fail closed. If every switched bond obeys

\[
 q_y-q_x=0,                                             \tag{10}
\]

prove both `W=0` and `(K_ell'-K_ell)q=0`; the switch changes neither energy
nor instantaneous force. The local signed crossing velocity

\[
 \sigma_b=\operatorname{sgn}(p_y-p_x)                  \tag{11}
\]

is admissible only when nonzero and must reverse under time reversal. The
reversible orientation handshake uses the already-priced two-slot transfer
`(sigma,0)<->(0,sigma)`; erasing a nonzero sign into blank is forbidden.

### G5 — exact fixed-gate finite-tick ledger

For the kick--drift map inherited from FTD-0876,

\[
 p'=p-hK_\ell q,\qquad q'=q+hp',                       \tag{12}
\]

prove symplecticity, exact invertibility, and exact preservation of

\[
 \widetilde H_{h,\ell}
 =\frac12p^Tp+\frac12q^TK_\ell q
  -\frac h2p^TK_\ell q.                               \tag{13}
\]

Also prove the local decomposition

\[
 \widetilde H_{h,\ell}
 =\frac12\sum_xp_x^2+
 \sum_b g_ba_b
 \left[\frac12(q_y-q_x)^2
 -\frac h2(p_y-p_x)(q_y-q_x)\right].                  \tag{14}
\]

It is nonnegative when `h^2 lambda_max(K_ell)<4`, and positive definite on
the quotient by the massless constant-coordinate null modes. Since
`0<=K_ell<=K`, cutting bonds cannot worsen that stability bound. The exact
finite-tick switching cost is the difference of (13); it must also vanish
under (10).

This gate does not claim the damped, sourced, projected, stochastic, or
boundary production tick preserves (13).

### G6 — oscillator and normalization fork

For any normalized regional eigenmode

\[
 K_\Lambda u=\lambda u,\qquad \lambda>0,
 \qquad Q=u^Tq,\quad P=u^Tp,\quad\omega=\sqrt\lambda, \tag{15}
\]

prove the canonical action-angle chart

\[
 Q=\sqrt{\frac{2I}{\omega}}\cos\theta,qquad
 P=-\sqrt{2\omega I}\sin\theta,qquad
 dQ\wedge dP=d\theta\wedge dI,                        \tag{16}
\]

and

\[
 H_u=\frac12(P^2+\omega^2Q^2)=\omega I.               \tag{17}
\]

The physical seam debit is therefore

\[
 I'=I+\frac{H-H'}{\omega}                              \tag{18}
\]

for this Hamiltonian normalization. FTD-0987's `H+2I` identity remains exact
only for the explicitly non-Hamiltonian observable-amplitude audit. It must
not be promoted to the wave-tick work law. A zero eigenvalue has no regular
oscillator action-angle chart.

The clutch creates finite-region compact eigenmodes by changing the operator;
this does not contradict FTD-0943/0987's no-go for unchanged infinite `C18`.
No unique region, eigenmode, frequency, scale, or `G*` identification is
preregistered.

### G7 — epistemic and production firewalls

The certificate must explicitly reject promotion to:

- a derived production latch or bond actuator;
- a native body/eigenmode formation law;
- a unique work-mode or frequency selection;
- exact conservation by the complete production tick;
- a `G*` gearbox, Born/Bell mechanism, mass, Hilbert-space recovery, or
  whole-framework completeness claim.

No fit, numerical near-miss search, parameter scan, formula substitution, or
engine mutation is permitted.

## 5. Classifier

- **Outcome A — native closure:** every exact gate passes and frozen
  production already contains the local latch, reciprocal ledger, retained
  inverse, and a uniquely formed positive-frequency mode.
- **Outcome B — exact local reference law / physical ownership still
  selected:** G2--G6 pass, but production lacks at least one of the latch,
  formation, mode selection, reciprocal ledger, or complete-tick closure.
- **Outcome C — boundary only:** the incidence clutch is local and positive
  but exact current, zero-strain seam, finite-tick invariant, or oscillator
  normalization fails.
- **Outcome D — invalid:** a source hash or verifier integrity gate fails.

Outcome B is expected. Outcome A is forbidden unless the frozen production
sources themselves satisfy the production gates.

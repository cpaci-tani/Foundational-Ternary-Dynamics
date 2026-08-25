# V3 rotor-Green A2 physical memory and phase-protection boundary v1

**Date:** 2026-08-24  
**Status:** **[SELECTION — FINITE PREPARED EDGE-RECORD APPARATUS]** +
**[THEOREM — EXACT FIXED-OCCUPANCY A2 SIGNED COUNTER]** +
**[THEOREM, CONDITIONAL — PHYSICAL ROTOR-CURRENT WRITEBACK]** +
**[THEOREM, CONDITIONAL — EXACT INITIAL-PHASE RESPONSE BOUND]** +
**[BOUNDARY — OCCUPANCY ACTION IS BLIND TO THE RESPONSE RECORD]** +
**[OPEN — HOMOGENEOUS PHI, RECIPROCAL FORCE, ABSOLUTE NORMALIZATION,
TENSOR POLE, COMMON CONE, LENSING, AND NONLINEAR GRAVITY]**  
**Carrier price at the certified radius-one apparatus:** 108 existing A2
plaquette owners for the measured SC edges plus one distinct Moore-local A2
source counter; no new primitive type  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Rotor/Green parent:**
[`THEOREM_V3_NEUTRAL_ROTOR_HARMONIC_GREEN_SEAM_v1.md`](../charge_gauss_native_em/THEOREM_V3_NEUTRAL_ROTOR_HARMONIC_GREEN_SEAM_v1.md)  
**Triplet scalar parent:**
[`THEOREM_V3_TRIPLET_ISOTROPIC_SCALAR_GRAVITY_GREEN_BRIDGE_AND_TENSOR_SILENCE_v1.md`](THEOREM_V3_TRIPLET_ISOTROPIC_SCALAR_GRAVITY_GREEN_BRIDGE_AND_TENSOR_SILENCE_v1.md)  
**Exact certificate:**
[`proof_v3_rotor_green_a2_physical_memory_phase_protection.py`](../../../../../scripts/proofs/proof_v3_rotor_green_a2_physical_memory_phase_protection.py)

---

## 1. The missing ontological step

The rotor parent proves that sequential deterministic token histories define
the visit potential and current

\[
 G_N(x)={n_N(x)\over6N},\qquad
 J_N(x,d)={m_N(x,d)-m_N(x+d,-d)\over N}.             \tag{1}
\]

It also proves

\[
 \|L_DG_N-\delta_s\|_\infty\le {8\over N},
 \qquad
 |J_N(x,d)-[G_N(x)-G_N(x+d)]|\le {8\over3N}.        \tag{2}
\]

Those are exact finite-history statements, but the integers `n_N` and `m_N`
were computed by the certificate rather than retained in the instantaneous
v3 carrier. That is an ontological gap. A physical response cannot depend on
an analyst remembering events that the substrate itself no longer records.

This theorem closes that narrow gap for a finite prepared apparatus. Every
measured unoriented SC edge receives one finite existing-carrier register, and
one additional register counts completed source injections. Equation (1) is
then read from current physical payloads.

It does **not** follow that the record pushes matter. Physical memory and
mechanical response are different closure gates.

---

## 2. One A2 is an exact constant-occupancy signed counter

The registered A9 alphabet is

\[
 A9=\{\varnothing\}\sqcup(C_4\times\mathbb Z_2).
\]

Restrict each of the four A9 factors in one `A2=A9^4` owner to its eight
occupied phase/polarity states. The resulting constant-occupancy subset has

\[
 8^4=4096                                                   \tag{3}
\]

distinct physical states. Fix a target-blind lexicographic address of those
states. The first 4,095 encode

\[
 -2047,-2046,\ldots,0,\ldots,+2046,+2047,                  \tag{4}
\]

and the final state is the explicit symbol `OVERFLOW`.

Let `P_+` advance the 4,096-cycle by one and `P_-` retreat by one. Then

\[
 \boxed{P_-P_+=P_+P_-=I.}                                  \tag{5}
\]

A crossing in the selected orientation applies `P_+`; a crossing in the
opposite orientation applies `P_-`. Every admitted history whose running
signed count remains in equation (4) is therefore retained exactly. All four
A9 factors remain occupied on every transition:

\[
 \boxed{\Delta N_{A9}=0,\qquad\Delta N_{A2}=0.}             \tag{6}
\]

This is a finite permutation, not a real-valued accumulator and not hidden
unbounded precision. Overflow is an ontic state. A larger declared range
requires additional finite A2 owners and a causal ripple-carry construction;
that multi-owner mechanism remains open.

---

## 3. Local ownership and physical writeback

Each unoriented SC edge is incident on four square plaquettes. The certificate
constructs a deterministic augmenting-path matching from every measured edge
of the radius-one box, including its absorbing boundary edges, to a distinct
incident A2 plaquette. One additional source-adjacent plaquette is reserved
for `N`. The exact apparatus price is

\[
 108\ A2_{\rm edge}+1\ A2_{\rm source}.                    \tag{7}
\]

For each rotor transaction `x -> x+d`, the matched edge register receives
the corresponding signed permutation. The marker, departure rotor,
destination rotor, and served direction are otherwise unchanged. The source
register advances once at each injection.

On three certified histories with `N=7,37,128`, the complete instrumented
walk reproduces the parent visit and traversal dictionaries exactly. The
largest running edge count is only 22, far inside the one-A2 range. For each
edge `e`, the final decoded state is

\[
 C_e=m_N(e_+)-m_N(e_-),                                \tag{8}
\]

and the source register is exactly `N`. Thus

\[
 \boxed{J_N(e)={C_e\over N}}                           \tag{9}
\]

is a ratio of two present finite physical records. The entire counter bank
also obeys exact unit-source continuity:

\[
 \sum_{e\ni x}\operatorname{out}_x C_e
 =N\delta_{x,s}.                                      \tag{10}
\]

Equation (10) is not an analyst's reconstructed ledger. It is true of the
instantaneous A2 payloads.

The owner matching and update are a selected prepared apparatus. They have
not yet been integrated into the homogeneous canonical `Phi` schedule, and
the source/sink work needed to run repeated trials is not supplied here.

---

## 4. Exact protection against the arbitrary rotor phase

Let

\[
 G_D=L_D^{-1}\delta_s                                  \tag{11}
\]

be the unique exact Dirichlet Green function. For a canonically oriented edge
`e=(a,b)`, define the row functional

\[
 q_e^T=e_a^T-e_b^T,                                   \tag{12}
\]

omitting a row when the endpoint lies in the absorbing exterior, and define

\[
 K_e=\|q_e^TL_D^{-1}\|_1.                             \tag{13}
\]

Writing `r_N=L_DG_N-delta_s`, equations (2), (11), and (13) give

\[
\begin{aligned}
|J_N(e)-q_e^TG_D|
&\le |J_N(e)-q_e^TG_N|
  +|q_e^TL_D^{-1}r_N|\\
&\le {8\over3N}+K_e\|r_N\|_\infty\\
&\le \boxed{{8\over3N}+{8K_e\over N}}.              \tag{14}
\end{aligned}
\]

The exact Green function in equation (14) is independent of the initial
rotor phases. Consequently, for any two allowed initial phase assignments
`p` and `p'`, the triangle inequality gives

\[
 \boxed{
 |J_N^{(p)}(e)-J_N^{(p')}(e)|
 \le2\left({8\over3N}+{8K_e\over N}\right).}         \tag{15}
\]

The certificate verifies equation (14) on every edge for all 192 native
uniform initial rotor states at `N=37`, and equation (15) using the exact
phase extrema. The largest running count across those histories is seven.
No stochastic assumption enters. The protection is deterministic and falls
as `1/N` on each fixed finite domain.

This protection is specifically against arbitrary initial rotor phase. It
does not cover malformed router sites, walker collisions, dropped packets,
counter overflow, or apparatus backreaction.

---

## 5. What survives of the gravity bridge

The triplet rest-source theorem supplies the exact scalar coordinate

\[
 \rho_{\rm rest}={1\over12}.                           \tag{16}
\]

Multiplying the physical current readout and equation (14) by `1/12` gives a
finite-carrier scalar response with the same phase protection. The cubic
symbol remains

\[
 \Lambda(k)=6-2(\cos k_x+\cos k_y+\cos k_z),           \tag{17}
\]

with `Lambda(0)=0` and Hessian `2I`. Thus the controlled large-domain
history readout still carries the conditional static form

\[
 {1/12\over\Lambda(k)}.                                \tag{18}
\]

The improvement over the parent theorem is precise:

```text
unretained traversal history:         replaced by finite A2 records
arbitrary uniform initial rotor phase: bounded exactly by O(1/N)
unit source continuity:               present in instantaneous A2 states
mechanical backreaction on matter:    not constructed
absolute response coefficient:        not selected
```

Equation (18) remains a prepared blocked-history pole, not a demonstrated
propagating gravity mode.

---

## 6. The new boundary: memory is not force

Equation (6) has a decisive consequence. The established common relative
occupancy ray prices field, SC-A1, and A2 roles by their occupancies. Because
every counter update is phase-only at fixed A2 occupancy, that ledger assigns

\[
 \boxed{\Delta S_{\rm occupancy}=0}                   \tag{19}
\]

to both signs of every response write.

Therefore the current common-action result can recognize that a memory owner
exists, but it cannot distinguish one stored response value from another.
It supplies neither restoring force nor recoil and cannot determine an
absolute gravitational residue. This is not a defect in the counter: it is
an exact localization of the next missing physics.

A genuine reciprocal gravity mechanism now requires at least one of the
following to be derived from the common law:

1. a nondegenerate phase/clock action for the A2 response orbit;
2. a causal conversion of the phase record into an A2 work excitation or
   matter momentum change, with the conjugate reaction retained; or
3. an equivalent common transaction in which source, response, and probe
   exchange one conserved action quantum.

Simply multiplying equation (18) by a chosen constant would not close this
gate.

The
[`Phi-v9 reciprocal-impulse successor`](../common_action_mechanics_reciprocity/THEOREM_V3_A2_GREEN_PULSE_RECIPROCAL_IMPULSE_ACTION_PHI_v9_CANDIDATE_AND_FORCE_BOUNDARY_v1.md)
constructs one explicit candidate for items 1--2. A finite `12N`-tick
accumulator converts `(C,N)` into average probe impulse `-C/(12N)`, writes the
equal-and-opposite reaction momentum, and conserves the selected quadratic
phase-action plus clock-work record (13/13). The construction makes the
needed extra structure auditable, but it does not derive that phase action
from canonical Phi, return the reaction to source matter, or compose the
momentum record with actual triplet acceleration. The force gate remains
open.

---

## 7. Exact remaining gravity debts

After this theorem, the gravity chain is narrower but still open:

1. integrate the rotor, source renewal, sink, edge memory, and overflow policy
   into one homogeneous canonical `Phi`;
2. construct causal multi-A2 carry/reset if observation ranges exceed one
   register;
3. derive or reject the Phi-v9 phase-action candidate from canonical `Phi`
   and compose its reciprocal records with actual material work;
4. prove a protected scalar constraint mode under traffic and perturbations,
   not only initial-phase protection;
5. derive the moving vector/STF response and an isolated tensor wave pole;
6. establish universal coupling to matter and radiation with an absolute
   normalization;
7. recover the common cone, local clock response, Shapiro delay, and lensing;
   and
8. only then address nonlinear self-coupling and the Einstein bootstrap.

The theorem closes physical **record retention** and one exact protection
class. It does not close physical gravity.

---

## 8. Reproduction

From the repository root:

```bash
python scripts/proofs/proof_v3_rotor_green_a2_physical_memory_phase_protection.py
```

Expected result: `15/15` exact checks pass. The certificate reports 4,096
constant-occupancy A2 states, signed range `-2047..+2047` plus overflow, 108
radius-one edge owners plus one source owner, exact physical unit divergence,
all 192 initial phases inside equation (14), triplet scalar factor `1/12`, and
zero relative occupancy-action delta per memory write.

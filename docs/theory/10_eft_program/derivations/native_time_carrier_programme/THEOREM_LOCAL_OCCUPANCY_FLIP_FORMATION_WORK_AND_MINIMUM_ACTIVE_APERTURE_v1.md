# Theorem — Local occupancy-flip formation work and minimum active aperture v1

**Identifier:** `FTD-0991/0992`  
**Date:** 2026-08-12  
**Status:** `[THEOREM, CONDITIONAL — EXACT CUT-SET FORMATION WORK]` +
`[THEOREM, CONDITIONAL — RECIPROCAL ACTION DEBIT]` +
`[THEOREM, CONDITIONAL — MINIMUM FAIL-CLOSED TERNARY APERTURE]` +
`[CLOSED NEGATIVE — CLOCK SELF-START / PRODUCTION ACTUATOR]`  
**Parent:** `FTD-0990`

## Result

Conditional on the selected FTD-0990 occupancy-controlled common-channel
Hamiltonian, the work of forming, growing, deleting, or moving an occupancy
membrane is no longer arbitrary. It is the exact finite difference of the
common-channel bond potential.

Let

\[
 m_x=s_x^2,\qquad
 g_{xy}=1-(m_x-m_y)^2,
\]

and write `d_b=q_{+,y}-q_{+,x}` for a C18 bond `b=(x,y)` of positive weight
`a_b`. Then

\[
 V_m={1\over2}\sum_b g_ba_bd_b^2,
\]

and every fixed-field occupancy update obeys

\[
 \boxed{W_{m\to m'}
 =V_{m'}-V_m
 ={1\over2}\sum_b(g_b'-g_b)a_bd_b^2.}                   \tag{1}
\]

If a set `S` of occupancy bits is flipped simultaneously, only the cut-set
`\partial S` changes. With `chi_x=1` on `S` and
`c_b=(chi_x-chi_y)^2`, one has

\[
 g_b'-g_b=c_b(1-2g_b),
\]

so

\[
 \boxed{W_S={1\over2}\sum_{b\in\partial S}
 (1-2g_b)a_bd_b^2.}                                    \tag{2}
\]

Equation (2) is local, counts each changed bond once, is independent of any
site ordering at fixed `q_+`, and leaves bonds internal to a simultaneously
flipped cluster unchanged.

For one-site formation `m_x:0->1`, define

\[
 E_{\rm join}={1\over2}\sum_{y:m_y=1}a_{xy}d_{xy}^2,
 \qquad
 E_{\rm cut}={1\over2}\sum_{y:m_y=0}a_{xy}d_{xy}^2.
\]

Then

\[
 \boxed{W_x=E_{\rm join}-E_{\rm cut}.}                 \tag{3}
\]

In particular, forming an occupied cluster in initially uniform void cuts
only its exterior boundary and therefore gives

\[
 \boxed{W_{\rm form}(S)
 =-{1\over2}\sum_{b\in\partial S}a_bd_b^2\le0.}         \tag{4}
\]

Thus topology cutting can release pre-existing common-channel boundary
strain. Reversing the same occupancy update at the same field point costs
exactly the negative of equations (1)--(4). This is an exact local formation
and reversal **ledger**, not a claim that a body forms autonomously.

An already prepared regular action `I>0` of frequency `Omega>0` can book the
work by

\[
 \boxed{I'=I-{W\over\Omega},\qquad
 H'=H+W,qquad H'+\Omega I'=H+\Omega I.}                \tag{5}
\]

Negative switch work charges the action; positive switch work debits it.
The event must fail closed if `I'<0`. Equation (5) does not choose a phase or
create an oscillator from zero action.

Finally, a static boundary needs no controller state, but an **active**
boundary aperture does. Let `(ell_b,r_b)` be two ternary slots and define

\[
 \boxed{\gamma_b=g_b+(1-g_b)r_b^2.}                    \tag{6}
\]

The reversible transfer

\[
 (\sigma,0)\longleftrightarrow(0,\sigma),
 \qquad \sigma\in\{-1,+1\},                            \tag{7}
\]

makes `(sigma,0)` a closed oriented boundary state and `(0,sigma)` an open
oriented state. Blank `(0,0)` is fail-closed. Opening costs one bond's common
strain energy and closing returns it. The sign is time-odd, while the gate is
time-even.

## Certificate of record

- Parent protocol:
  [`PREREG_LOCAL_OCCUPANCY_FLIP_FORMATION_WORK_AND_TERNARY_APERTURE_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_LOCAL_OCCUPANCY_FLIP_FORMATION_WORK_AND_TERNARY_APERTURE_v1.md),
  SHA-256
  `34A71B6E77DBB23FA0D256F0032A5A708405F67CDA63D59AC756A15CA49062E7`.
- Immutable parent proof:
  [`proof_local_occupancy_flip_formation_work_and_ternary_aperture.py`](../../../../../scripts/proofs/proof_local_occupancy_flip_formation_work_and_ternary_aperture.py),
  SHA-256
  `A8F0D61500C5878036E25B4CBEA4148FDD72BC64BDDF94D130EA08BFB38BBA16`.
- First locked execution: `80/83`; every mathematical and source gate passed,
  while three literal verifier predicates failed.
- Repair protocol:
  [`PREREG_LOCAL_OCCUPANCY_FLIP_FORMATION_CERTIFICATE_REPAIR_v2.md`](../../preregistrations/native_time_carrier_programme/PREREG_LOCAL_OCCUPANCY_FLIP_FORMATION_CERTIFICATE_REPAIR_v2.md),
  SHA-256
  `583639F58E4A247F54948D40FD121136A3FEBCD7FB0C7080E2A443C13B1AB59A`.
- Repair wrapper:
  [`proof_local_occupancy_flip_formation_work_and_ternary_aperture_v2.py`](../../../../../scripts/proofs/proof_local_occupancy_flip_formation_work_and_ternary_aperture_v2.py),
  SHA-256
  `00A994F91AF588ABAAA8AD81F66FEB0DF41489530F76E1B1207AD4551A5374F7`.
- Final execution: `94/94` computational, `2` disclosure/scope (cannot fail),
  **Outcome B — exact conditional ledger / selected actuator**.

## 1. Exact cut-set identity

Write the occupancy flip as

\[
 m_x'=m_x\oplus\chi_x.
\]

For a bond `b=(x,y)`, there are only two cases.

If `chi_x=chi_y`, both or neither endpoints are complemented. Equality of
the endpoint bits is unchanged, hence `g_b'=g_b`.

If `chi_x \ne chi_y`, exactly one endpoint is complemented. Equality becomes
inequality and inequality becomes equality, hence

\[
 g_b'=1-g_b.
\]

Since `(chi_x-chi_y)^2` is one exactly in the second case,

\[
 g_b'-g_b=(\chi_x-\chi_y)^2(1-2g_b).                   \tag{8}
\]

Substitution of equation (8) into equation (1) proves equation (2). An edge
whose endpoints both lie in `S` is not in `\partial S`; it contributes neither
work nor a duplicate site term. This is why simultaneous cluster formation
must be evaluated as a bond cut-set rather than as a naive sum of independently
evaluated site costs.

The relative channel contributes no term to equation (1): its stiffness is
the unchanged full `K` on both sides of the FTD-0990 reference update.

## 2. Single-site formation and growth

For a single flipped site,

\[
 \delta_x=m_x'-m_x=1-2m_x.
\]

Direct evaluation of the equality gate gives

\[
 g_{xy}'-g_{xy}=\delta_x(2m_y-1),                       \tag{9}
\]

and therefore

\[
 W_x={\delta_x\over2}\sum_{y\sim x}
 (2m_y-1)a_{xy}d_{xy}^2.                               \tag{10}
\]

For `m_x:0->1`, a bond to existing matter is opened and costs its stored
strain energy; a bond to surrounding void is cut and releases its stored
strain energy. Sorting equation (10) by neighbor occupancy proves equation
(3).

This gives three exact regimes:

1. **isolated formation in uniform void:** `E_join=0`, hence all boundary
   strain is released;
2. **body growth:** both terms occur, and the sign depends on the local field
   geometry rather than an imposed universal formation cost; and
3. **same-point reversal:** every gate difference changes sign, so all work
   is returned exactly.

The third statement is a Hamiltonian endpoint identity. It does not say that
production retains enough history to find the same field point after an
arbitrary intervening evolution.

## 3. Onsite clock support and honest load pricing

The FTD-0990 reference Hamiltonian also contains the imposed onsite support

\[
 V_\omega={\omega_0^2\over2}\sum_xm_x
 (q_{+,x}^2+q_{-,x}^2).
\]

For a simultaneous bit flip its exact fixed-field contribution is

\[
 W_{\omega,S}={\omega_0^2\over2}\sum_{x\in S}
 (1-2m_x)(q_{+,x}^2+q_{-,x}^2).                         \tag{11}
\]

Therefore the available energy of an initially void cluster is not silently
all assigned to matter. Any positive onsite, rest, controller, recoil, or
clock-seed load `L` must satisfy an explicit admission inequality of the
form

\[
 E_{\partial S}+\Omega I_{\rm pre}
 \ge W_{\omega,S}+L,                                   \tag{12}
\]

with equality only when no residual channel is retained. Equation (12)
prices a proposed load; it does not derive `L`, mass, `omega_0`, or a transfer
mechanism.

The membrane sees only `m=s^2`. Consequently its work is identical for
`s=+1` and `s=-1`. Charge polarity, pair neutrality, and outcome frequency
cannot be extracted from this ledger.

## 4. Charging is not self-start

For `Omega>0`, equation (5) is algebraically forced by conservation of
`H+Omega I`. Its inverse is the same endpoint transaction with `W -> -W`.
It follows immediately that

- `W<0` increases `I`;
- `W>0` decreases `I`;
- `I-W/Omega>=0` is the local positive-reserve condition; and
- `W=0` changes no action.

This identifies a natural way for released membrane strain to charge an
**already regular** local clock or controller. It does not solve first
preparation. The action-angle chart

\[
 Q=\sqrt{2I/\Omega}\cos\theta,
 \qquad
 P=-\sqrt{2\Omega I}\sin\theta
\]

has no defined phase at `I=0`. Lifting zero action to positive action requires
a phase-selection or phase-bearing incoming state. `G*` cadence could time
such a gate only after a separate physical identification; it does not supply
the missing phase or energy here.

## 5. Minimum fail-closed active aperture

The static equality membrane is already fail-closed at a matter--void
boundary. Temporarily opening it while keeping the endpoint occupancies fixed
requires a controller variable.

Equation (6) has the exact truth table

| occupancy gate `g` | receiver `r` | active gate `gamma` | meaning |
|---:|---:|---:|---|
| 1 | `-1,0,+1` | 1 | equal occupancy always transmits |
| 0 | 0 | 0 | boundary closed / blank fails closed |
| 0 | `-1,+1` | 1 | boundary open with retained orientation |

The five self-delimiting logical states are

\[
 (0,0),\quad(+1,0),\quad(-1,0),\quad(0,+1),\quad(0,-1). \tag{13}
\]

One ternary slot has only three states and cannot represent equation (13)
injectively. Two ternary slots are sufficient. On the valid oriented states,
the swap `(ell,r)->(r,ell)` realizes equation (7), is its own inverse, and
commutes with time reversal `(ell,r)->(-ell,-r)`.

On a boundary bond, opening changes `gamma:0->1` and costs

\[
 W_{\rm open}={1\over2}a_bd_b^2.                        \tag{14}
\]

Closing returns exactly `-W_open`. At zero strain both vanish, but erasing the
orientation token still remains noninjective. Thus zero energetic work does
not imply zero history cost.

The controller price is incurred only at **active apertures**. No independent
static latch is restored to every ordinary matter boundary.

## 6. Production boundary

The frozen production sources do not implement this reference mechanism.

- `phase_read.cpp` applies the same full C18 stencil to `L` and `R`; it has no
  occupancy-controlled common stiffness.
- `phase_write.cpp` accepts genesis through a random draw, applies a selected
  single-substrate drain which is explicitly not an exact common-action
  latent-heat identity, and clears state on evaporation.
- the event journal is observation-only and cannot supply the inverse;
- no positive local action reserve, fail-closed two-slot aperture, or exact
  formation transaction is present; and
- the dual genesis path has no drain matching the single-substrate path.

No engine change follows from this theorem.

## 7. Epistemic disposition

Established, conditional on the FTD-0990 selected Hamiltonian:

- **[THEOREM, CONDITIONAL]** arbitrary occupancy work is equation (1);
- **[THEOREM, CONDITIONAL]** simultaneous flips reduce exactly to the cut-set
  law (2);
- **[THEOREM, CONDITIONAL]** formation/growth work is equation (3), and a
  uniform-void cluster releases equation (4);
- **[THEOREM, CONDITIONAL]** same-point reversal returns exactly the work;
- **[THEOREM, CONDITIONAL]** a prepared regular action books work through
  equation (5); and
- **[THEOREM, CONDITIONAL]** equations (6)--(7) give the minimum two-ternary-
  slot fail-closed active aperture within the registered self-delimiting
  interface.

Not established:

- the FTD-0990 occupancy-controlled stiffness as a native production law;
- autonomous selection of the flip set `S` or charge sign;
- a zero-action clock self-start or target-blind phase preparation;
- derivation of rest load, recoil, `omega_0`, or `G*`;
- a production actuator, moving-boundary attachment, collision/backpressure,
  repeated stability, CPU/CUDA parity, or operational hiding; or
- Born/Bell, mass, Hilbert-space recovery, Lorentz recovery, or completeness.

The next exact discriminator is therefore narrower: determine whether a
target-blind local phase-bearing input can prepare the connected body's
uniform common mode while satisfying equation (12), or prove that an
independent seed/controller remains irreducible.

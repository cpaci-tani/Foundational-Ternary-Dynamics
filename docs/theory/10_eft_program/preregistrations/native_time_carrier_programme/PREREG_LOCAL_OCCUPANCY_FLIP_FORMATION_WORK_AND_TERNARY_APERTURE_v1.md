# FTD-0991 — Preregistration: local occupancy-flip formation work and ternary aperture v1

**Identifier:** `FTD-0991`  
**Date locked:** 2026-08-12  
**Status before execution:** `[PREREGISTERED — NOT YET EVIDENCE]`  
**Expected classifier:** **Outcome B — exact conditional ledger / selected actuator**

## 1. Question

FTD-0990 derives the static membrane from `m=s^2` but leaves the membrane
transition unpaid. This discriminator asks whether the selected reference
Hamiltonian already fixes, without a fitted coefficient,

1. the exact work of an arbitrary simultaneous occupancy update;
2. the formation/growth/reversal ledger of a connected occupied region;
3. the frequency-normalized debit of an already prepared positive-action
   local clock or controller reserve; and
4. the minimum fail-closed ternary aperture which can temporarily reopen a
   matter--void common-channel bond while retaining time-odd orientation.

The test is conditional on the FTD-0990 occupancy-controlled dual-stiffness
law. It is not permitted to promote that selected Hamiltonian, generate a
clock from zero action, derive charge polarity, or mutate production.

## 2. Frozen sources

| source | SHA-256 |
|---|---|
| `THEOREM_TERNARY_OCCUPANCY_MEMBRANE_AND_SELF_DUAL_BODY_CLOCK_SPLIT_v1.md` | `A19593DACD2CE97A6B785F235AE5048EADC228680E07D2F90F4C4DB7BD15333C` |
| `THEOREM_C18_BOND_CLUTCH_CURRENT_AND_WORK_ACTION_NORMALIZATION_v1.md` | `2A93D9CFF23DFFDFEEC5E1F07CB7C023D95FBACC9B05BEA4E3F77775124D87C8` |
| `THEOREM_KRYLOV_DEGENERACY_TERNARY_LATCH_AND_ORIENTED_C4_TRANSITION_v1.md` | `7DA2366C75D38E0EA1F8012632D71C676C4E6F8D1A7F8D1467EAF4185AE77194` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/src/transmutation_phases.cpp` | `4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043` |

Any mismatch invalidates execution. A repair must preserve this protocol and
the first certificate byte-for-byte.

## 3. Registered class

Let `E` be the C18 bond set, `a_b>0`, `d_b=q_{+,y}-q_{+,x}`, and

\[
 m_x=s_x^2,\qquad
 g_b(m)=1-(m_x-m_y)^2.                                  \tag{1}
\]

The common-channel membrane potential is

\[
 V_m(q_+)={1\over2}\sum_{b\in E}g_b(m)a_bd_b^2.          \tag{2}
\]

For an arbitrary mask update `m -> m'`, register the exact fixed-field work

\[
 \boxed{W_{m\to m'}=V_{m'}-V_m
 ={1\over2}\sum_b\bigl(g_b(m')-g_b(m)\bigr)a_bd_b^2.}    \tag{3}
\]

For a simultaneous bit flip on a set `S`, let `chi_x=1` on `S` and zero
otherwise, and let

\[
 c_b=(\chi_x-\chi_y)^2
\]

be the cut-set indicator. Then the registered reduction is

\[
 \boxed{W_S={1\over2}\sum_{b\in\partial S}
       (1-2g_b)a_bd_b^2.}                               \tag{4}
\]

For a single-site flip `x`, put `delta_x=m'_x-m_x=1-2m_x`. Then

\[
 W_x={\delta_x\over2}\sum_{y\sim x}
       (2m_y-1)a_{xy}(q_{+,y}-q_{+,x})^2.               \tag{5}
\]

For formation `m_x:0->1`, define

\[
 E_{\rm join}={1\over2}\sum_{m_y=1}a_{xy}d_{xy}^2,
 \qquad
 E_{\rm cut}={1\over2}\sum_{m_y=0}a_{xy}d_{xy}^2,
\]

so `W_x=E_join-E_cut`. The reverse update at the same field point must cost
exactly `-W_x`.

If the imposed onsite clock support is included, its fixed-field switch work
is separately

\[
 W_{\omega,S}={\omega_0^2\over2}\sum_{x\in S}
 (1-2m_x)(q_{+,x}^2+q_{-,x}^2).                         \tag{6}
\]

This is a price, not a derivation of `omega_0`.

For an already prepared controller or body-clock action `I>0` of frequency
`Omega>0`, register the work-port debit

\[
 \boxed{I'=I-{W\over\Omega},\qquad H'=H+W,\qquad
 H'+\Omega I'=H+\Omega I.}                              \tag{7}
\]

The transaction is admissible only if `I'>=0`. Equation (7) is a charging
law for an existing regular action chart. It does not choose a phase or
construct a clock from `I=0`.

For an actively controlled bond, use two ternary slots
`(ell,r) in {-1,0,+1}^2` and define the fail-closed common-channel aperture

\[
 \boxed{\gamma_b=g_b+(1-g_b)r_b^2.}                     \tag{8}
\]

The valid orientation-carrying transfer is

\[
 (\sigma,0)\longleftrightarrow(0,\sigma),
 \qquad \sigma\in\{-1,+1\}.                            \tag{9}
\]

At a boundary, `(sigma,0)` is closed and `(0,sigma)` is open; `(0,0)` is
blank and fail-closed. The sign reverses under time reversal but equation
(8) does not. The opening/closing work is equation (3) with `g` replaced by
`gamma`.

## 4. Exact gates

### F1 — source and inherited-claim lock

- all seven hashes match;
- the FTD-0990 membrane, FTD-0989 switching-work/action normalization, and
  inherited reversible two-slot orientation transfer are source present;
- unchanged production still has stochastic genesis, noninvertible
  evaporation, no occupancy membrane, no active aperture, and no exact
  formation ledger.

### F2 — arbitrary and simultaneous occupancy work

Prove:

1. equation (3) is the exact finite difference of equation (2);
2. flipping both or neither endpoint leaves a bond gate unchanged, while
   flipping exactly one endpoint sends `g -> 1-g`;
3. equation (4) follows and counts each changed bond once, so a simultaneous
   cluster update has no site-order or internal-edge double counting;
4. equation (5) is the exact single-site specialization; and
5. the relative `q_-` stiffness contributes no occupancy-switch work because
   it remains `K` on both sides.

### F3 — formation, growth, reversal, and polarity boundary

Prove:

1. forming a cluster `S` in initially uniform void gives

\[
 W_S=-{1\over2}\sum_{b\in\partial S}a_bd_b^2\le0;       \tag{10}
\]

2. single-site growth gives `W=E_join-E_cut`;
3. deletion at the same field point exactly reverses either transaction;
4. equation (6) is the exact onsite-support contribution;
5. any nonnegative rest/clock/controller load must be paid from released
   work or a pre-existing reserve—no latent energy may be silently created;
6. since `m=s^2`, the ledger is blind to `s=+1` versus `s=-1` and cannot
   select charge polarity or a Born weight.

The sign in equation (10) means topology cutting can release stored common
boundary strain. It does not by itself prepare a uniform clock mode or prove
that a physical genesis event occurs.

### F4 — frequency-normalized work port

Prove equation (7), its exact inverse at fixed field endpoints, and the
positive-reserve condition. Establish:

- `W<0` charges the action and `W>0` debits it;
- `Omega=0` is excluded;
- a zero-strain switch has zero work but still requires orientation history;
- an exactly zero-action oscillator has a singular action-angle phase, so
  the ledger is not a self-start or target-blind mode-preparation theorem.

### F5 — minimum active ternary aperture

Prove:

1. equation (8) always transmits equal occupancy;
2. at a boundary it is closed for `r=0`, open for `r=+/-1`, and therefore
   fail-closed in the blank state;
3. sign reversal leaves the gate invariant while retaining orientation;
4. the five required logical states—blank, closed `+/-`, open `+/-`—cannot
   fit in one ternary slot, while the two-slot states in equation (9) realize
   them injectively;
5. equation (9) is exactly invertible and time-reversal covariant; and
6. opening costs one bond's stored strain energy, closing releases the same
   energy, with both zero at zero strain.

This prices controller hardware only on active apertures. Static body
boundaries continue to use endpoint occupancy alone.

### F6 — production and epistemic firewalls

Explicitly reject promotion to:

- a derivation or production implementation of the FTD-0990 dual-stiffness
  law;
- autonomous genesis, evaporation inverse, body-clock preparation, or a
  complete moving-boundary dynamics;
- a free switch, free information erasure, or globally shared instantaneous
  reserve;
- a derivation of `omega_0`, `G*`, charge polarity, Born/Bell, mass, Lorentz
  hiding, Hilbert space, or completeness.

No fit, numerical near-miss search, parameter scan, formula substitution, or
engine mutation is permitted.

## 5. Classifier

- **Outcome A — native reversible formation actuator:** all gates pass and
  frozen production already implements the occupancy coupling, exact
  formation/action transaction, fail-closed aperture/history, and inverse.
- **Outcome B — exact conditional ledger / selected actuator:** F2--F5 pass,
  but the dual-stiffness law, controller/action reserve, phase preparation,
  or reciprocal production transition remains selected or absent.
- **Outcome C — work identity only:** the finite-difference ledger passes but
  the cut-set, reciprocal action, or active-aperture theorem fails.
- **Outcome D — invalid:** a source hash or exact gate fails.

Outcome B is expected. Outcome A is forbidden unless the frozen production
sources themselves contain the complete reciprocal mechanism.

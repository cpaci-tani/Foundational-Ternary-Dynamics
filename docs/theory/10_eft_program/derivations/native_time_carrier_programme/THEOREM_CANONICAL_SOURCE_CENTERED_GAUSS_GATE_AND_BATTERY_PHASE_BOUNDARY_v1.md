# Theorem — Canonical source-centered Gauss gate and battery-phase boundary v1

**Identifier:** `FTD-0885` / repaired execution `FTD-0886`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — POSITIVE SOURCE-CENTERED CANONICAL GAUSS LAYER]` +
`[THEOREM — EXACT RAW-WORK/INTERACTION-ENERGY EXCHANGE]` +
`[THEOREM — COMPLETE-PAIR OPEN HISTORY SHIFT]` +
`[CLOSED NEGATIVE — PHASE-BLIND STATE-DEPENDENT POST-HOC CANONICAL DRAIN]` +
`[REFINED BOUNDARY — FTD-0884 BATTERY IS A LAGRANGIAN-SECTION REFERENCE LAW]` +
`[IMPOSED — CLOCKED HAMILTONIAN INTERPOLATION AND SCALE]` +
`[REFERENCE IMPLEMENTATION — ISOLATED ftd::eft]` +
`[OPEN — AUTONOMOUS PARITY CONTROL, SOURCE FORMATION/RECOIL, PHYSICAL OPEN
HISTORY, PRODUCTION, G*]`

## 1. Verdict

The positive square-root battery introduced by FTD-0884 is not the missing
canonical reservoir. It is an exact one-amplitude energy ledger, but that
amplitude is only one coordinate. Restoring its missing phase/conjugate reveals
two obstructions:

1. the symplectic cotangent lift changes positive oscillator energy by the
   desired `-w` only on the zero-conjugate slice; and
2. an action decrement is locally symplectic for constant work but is not a
   globally Hamiltonian translation on a periodic phase cylinder. If the work
   depends on the system state, a phase-blind triangular drain is not even
   symplectic.

The constructive answer lies one layer earlier. The FTD-0882 residual/port
gate itself has an exact positive canonical lift when its energy is centered
on the fixed source constraint. In that decomposition, the raw field-plus-port
energy gained by the gate is paid exactly by the source-field interaction
energy. No independent post-hoc battery is required.

This closes one local checkerboard Hamiltonian layer and the canonical status
of the FTD-0884 battery. It does not yet provide an autonomous common
Hamiltonian for the alternating parity schedule, dynamical source matter and
recoil, or a finite closed history recycler.

## 2. Normalized source-centered mode

For an active cell, let `d_x` be its matched incidence row with
`||d_x||^2=6`. Define

\[
 y=\frac{d_xJ}{\sqrt6},\qquad
 s=\frac{q_x}{\sqrt6},\qquad
 u=y-s,\qquad
 a=\frac{e_x}{\sqrt6}.                                      \tag{1}
\]

The FTD-0882 update is exactly

\[
 (u,a)\longmapsto(a,-u).                                    \tag{2}
\]

Restore complete canonical modes

\[
 \{u,\pi_u\}=1,\qquad \{a,\pi_a\}=1,                       \tag{3}
\]

and define

\[
 N=\frac12(u^2+a^2+\pi_u^2+\pi_a^2),qquad
 L=a\pi_u-u\pi_a.                                           \tag{4}
\]

The elementary identities

\[
 \{N,L\}=0,                                                  \tag{5}
\]

\[
 2(N-L)=(a-\pi_u)^2+(u+\pi_a)^2,\qquad
 2(N+L)=(a+\pi_u)^2+(u-\pi_a)^2                             \tag{6}
\]

give `|L|<=N` exactly.

## 3. Positive clocked Hamiltonian layer

With an imposed common reference clock `(theta,I)` and `omega>0`, take

\[
 H=\omega I+\omega N
   +\sigma\frac{\omega}{4}(1-\cos\theta)L,qquad
 \sigma\in\{-1,+1\}.                                       \tag{7}
\]

The carrier part is positive:

\[
 \omega N+\sigma\frac{\omega}{4}(1-\cos\theta)L
 \ge \frac{\omega}{2}N\ge0.                                \tag{8}
\]

Since `theta_dot=omega`, one cycle has `T=2*pi/omega`. The `N` flow makes one
complete identity winding, while

\[
 \sigma\frac{\omega}{4}\int_0^T
 (1-\cos\omega t)\,dt=\sigma\frac\pi2.                      \tag{9}
\]

The endpoint is therefore

\[
 \sigma=+1:quad
 (u,a,\pi_u,\pi_a)\mapsto(a,-u,\pi_a,-\pi_u),                \tag{10}
\]

with the opposite quarter-turn for `sigma=-1`. The matrix is orthogonal,
symplectic, determinant `+1`, fourth order, and exactly invertible.

The clock-action ledger is

\[
 I(\theta)=I_0-\sigma\frac14(1-\cos\theta)L,                 \tag{11}
\]

so `I(T)=I_0` and the maximum excursion is `|L|/2`. On the
zero-conjugate section `pi_u=pi_a=0`, `L=0`: the clock has no transient
backreaction, the section is invariant, and (10) reduces exactly to (2).

For one checkerboard color, the normalized incidence rows are orthonormal.
Their generators therefore commute and (7) sums over active cells without
cross-cell conflict. Switching the active color still uses the selected
integer parity schedule; this theorem does not make that switch autonomous.

## 4. Where the source work actually goes

On the zero-conjugate section define

\[
 E_{\rm raw}=\frac12(y^2+a^2),\qquad
 U_{\rm int}=-sy+\frac12s^2.                                 \tag{12}
\]

Then

\[
 E_{\rm raw}+U_{\rm int}=\frac12(u^2+a^2)\ge0.               \tag{13}
\]

Under (2), `y'=a+s` and `a'=-u`. Consequently

\[
 \Delta E_{\rm raw}=s(a-u)
 =\frac{q_x}{6}(e_x-r_x)=w_x,                                \tag{14}
\]

while

\[
 \Delta U_{\rm int}=-s(y'-y)=-w_x.                          \tag{15}
\]

Thus

\[
 \boxed{\Delta(E_{\rm raw}+U_{\rm int})=0.}                 \tag{16}
\]

The interaction term can be negative, but the completed source-centered
energy (13) is positive. This is the standard logical distinction between raw
field energy and total energy in the presence of a fixed source. FTD-0884's
battery was compensating the raw account after omitting this interaction.

Equation (16) does not make source formation free. The offset `s` is fixed in
this theorem. Promoting ternary matter to a moving canonical source, deriving
its interaction, and recovering recoil remain open.

## 5. Why the square-root battery does not canonically survive

### 5.1 Linear conjugate

On one sign branch the FTD-0884 law is

\[
 b'=f_w(b)=\operatorname{sgn}(b)\sqrt{b^2-2w},qquad
 f'_w(b)=\frac b{b'}.                                        \tag{17}
\]

Attach a conjugate `p_b`. The zero-section-preserving cotangent lift is

\[
 p_b'=\frac{p_b}{f'_w(b)}=p_b\frac{b'}b.                     \tag{18}
\]

It is exactly symplectic. But for the positive oscillator energy

\[
 E_{\rm osc}=\frac12(b^2+p_b^2),                             \tag{19}
\]

one obtains

\[
 \Delta E_{\rm osc}
 =-w\left(1+\frac{p_b^2}{b^2}\right).                        \tag{20}
\]

The desired `-w` holds only for `p_b=0` or `w=0`. More generally, let a
triangular symplectic extension have `p_b'=A(b)p_b+g(b)`. Symplecticity fixes
`A=1/f'_w`. Requiring (19) to change by `-w` for every `p_b` forces
`A^2=1`, `g=0`, hence `(f'_w)^2=1`; equation (17) violates that condition for
nonzero work. The failure is structural, not a poor choice of the cotangent
lift.

### 5.2 Action and phase

If positive battery energy is instead the action, the constant-work map

\[
 T_w:(I_b,\phi_b)\mapsto(I_b-w,\phi_b)                       \tag{21}
\]

preserves `dI_b wedge dphi_b` locally. However, for the canonical one-form
`lambda=I_b dphi_b`,

\[
 T_w^*\lambda-\lambda=-w\,d\phi_b.                           \tag{22}
\]

Its integral around the phase circle is `-2*pi*w`. For nonzero `w`, (22) is
closed but not exact, so (21) is not a globally Hamiltonian time map on the
phase cylinder. It would require an unwrapped phase/history coordinate or an
additional channel carrying the flux.

For state-dependent work `w(z)`, the phase-blind triangular map

\[
 (z,I_b,\phi_b)\mapsto(F(z),I_b-w(z),\phi_b)                  \tag{23}
\]

adds

\[
 -dw\wedge d\phi_b                                           \tag{24}
\]

to the pulled-back product symplectic form, even when `F` is symplectic.
Unless `dw=0`, (23) is not symplectic. A physical canonical reservoir must
allow phase backreaction, exchange a complete canonical work mode, or export
an additional conjugate/history degree. It cannot merely read `w` after the
system update.

FTD-0884's amplitude theorem remains mathematically correct in its registered
one-amplitude class. Its physical status is now sharper: it is an imposed
Lagrangian-section reference law, not a phase-complete Hamiltonian reservoir.

## 6. Canonical history export

The outgoing environment is a complete pair `(a,pi_a)`, not just a signed
coordinate or energy scalar. A right shift on an open rail is

\[
 Z'_0=Z_{\rm in},\qquad Z'_{j+1}=Z_j,qquad
 Z_{\rm out}=Z_{N-1}.                                       \tag{25}
\]

Including both boundary pairs, (25) is a permutation of canonical pairs and
is therefore symplectic and exactly invertible. Its positive energy ledger is

\[
 E_{\rm rail}'-E_{\rm rail}
 =E(Z_{\rm in})-E(Z_{\rm out}).                              \tag{26}
\]

A bilateral rail is the closed bijective idealization with a prepared blank
future. A finite cyclic rail still has exactly the FTD-0884 capacity boundary:
when a nonzero pair returns, it is not fresh. Exporting energy alone loses
orientation; exporting only the coordinate is not canonical.

## 7. What closed and what remains open

### Closed at reference level

- existence of a positive clocked Hamiltonian lift for every active local
  residual/port gate;
- exact symplectic inverse on complete canonical modes;
- exact exchange between raw field/port work and source interaction energy;
- the phase-complete obstruction to a post-hoc state-dependent battery drain;
- the Lagrangian-section status of the FTD-0884 square-root law; and
- exact complete-pair transport on an open/bilateral history rail.

### Still open

- one autonomous common Hamiltonian that generates the alternating parity
  schedule rather than accepting the tick-selected layer;
- formation, persistence, motion, and recoil of the ternary source offset;
- a substrate-native physical open environment or justified finite
  compression mechanism;
- three-dimensional routing, congestion, boundaries, and moving sources;
- production migration to matched face variables and the full energy ledger;
- derivation of the clock frequency and interaction scale;
- synchronization with the distinct quartic-`G*` eligibility calendar;
- Born recovery, Bell laboratory recovery, operational Lorentz hiding; and
- whole-framework completeness.

No sixth v2 selected type is added. The construction consumes the already
selected canonical phase/history rail and an imposed harmonic clocked
interpolation. It changes the accounting of the FTD-0884 battery; it does not
promote the interpolation to substrate-native law.

## 8. Verification and provenance

The frozen FTD-0885 protocol SHA-256 is
`70000AF7DA0ACA89F92A593AA4B6A759B9C9D08C65E29E21A2D1EF5B2B2910D7`.
The frozen parent certificate SHA-256 is
`7DC08CF572BF58BC37152F985608EB45A7F11C6308165D8D94F1B0A5B55D248E`.
Its first locked execution returned `60/64`: every mathematical gate passed;
C8, C53, and C62 used mismatched capitalization or raw line-wrapped prose,
and C64 failed dependently. No theorem is booked from that parent run.

FTD-0886 froze only those three marker normalizations. Its repair protocol
SHA-256 is
`428D1C37EF2510235387C1E0D71BD0DDF489CE58AEE1C4E34A10B3E978A26B3C`;
the in-memory wrapper SHA-256 is
`6C35135A3B5B9345E6EA9A6EBFB61B32951EE07DDDB17188362B8B38A10F1816`.
The inherited certificate passes `64/64` with markers:

```text
CANONICAL_SOURCE_CENTERED_GAUSS_GATE=POSITIVE_CLOCKED_LAYER
RAW_SOURCE_WORK=INTERACTION_ENERGY_EXCHANGE
SQUARE_ROOT_BATTERY=EXACT_ONLY_ON_LAGRANGIAN_SECTION
PHASE_BLIND_STATE_DEPENDENT_DRAIN=NOT_SYMPLECTIC
CONSTANT_ACTION_TRANSLATION=SYMPLECTIC_NOT_GLOBAL_HAMILTONIAN
CANONICAL_HISTORY_EXPORT=COMPLETE_PAIR_REQUIRED
FINITE_CYCLIC_FRESHNESS_BOUNDARY=UNCHANGED
AUTONOMOUS_PARITY_AND_SOURCE_DYNAMICS=OPEN
PRODUCTION_COUPLING=NONE
GSTAR_ROLE=SEPARATE_CALENDAR
BORN_BELL_STATUS=UNTOUCHED
```

The isolated implementation is:

- `engine/include/ftd/eft/canonical_source_centered_gauss_gate.h`;
- `engine/src/eft/canonical_source_centered_gauss_gate.cpp`; and
- `engine/tests/test_canonical_source_centered_gauss_gate.cpp`.

The native Release build passes under the pinned MSVC 14.44 toolchain. The
focused CTest passes `1/1`. No production `Voxel`, field, toggle, default,
boundary mode, or tick phase changed.

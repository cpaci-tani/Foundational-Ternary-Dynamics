# Preregistration — Canonical source-centered Gauss gate and battery-phase obstruction v1

**Identifier:** `FTD-0885`  
**Date frozen:** 2026-08-11  
**Status:** `[LOCKED/PRE-RUN]`  
**Programme:** native time carrier / contextual actualization  
**Method guard:** exact symbolic algebra and fixed rational witnesses only; no
numerical search, fitting, near-miss scan, or formula substitution is
permitted.

## 1. Question

FTD-0884 closes the raw field-plus-port work account with an imposed signed
square-root battery, while explicitly leaving a canonical Hamiltonian
reservoir open. This lock asks whether that post-hoc battery is the correct
canonical object.

The registered alternatives are deliberately minimal.

1. Lift one FTD-0882 active-cell residual/port quarter-turn to a positive
   source-centered Hamiltonian flow on complete canonical modes and determine
   where the raw source work resides.
2. Restore the phase/conjugate coordinate omitted by the one-amplitude battery
   and test whether the square-root drain remains both symplectic and exactly
   energy-correct away from its zero-conjugate slice.
3. State the exact history resource required for repeated fresh operation.

The certificate may close a local clocked layer and a battery obstruction. It
may not claim an autonomous common Hamiltonian for the alternating parity
schedule, a finite closed history recycler, a native production reservoir, a
`G*` gearbox, Born recovery, Bell recovery, Lorentz hiding, or framework
completeness.

## 2. Frozen sources

| source | SHA-256 |
|---|---|
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_REVERSIBLE_CHECKERBOARD_GAUSS_RECORD_PREPARATION_AND_SELF_DUAL_ENERGY_SPLIT_v1.md` | `143D897A69B5C6FED8C00402C1840EA9FAEE5BD4BC259C9BDD065DFDC616A814` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_FINITE_PORT_RAIL_POSITIVE_SOURCE_BATTERY_AND_RECYCLING_BOUNDARY_v1.md` | `AF810B73322DE8521C8509792E09D549A10E1D8417C1B283A3630EB8B16D7BFC` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_LOCAL_CANONICAL_HAMILTONIAN_PARITY_RAIL_AND_SCALAR_LOCALITY_BOUNDARY_v1.md` | `982C3B9D00798920A1BDAB96C75EBC9DB3A08111E8900F1D630382B0249B25F6` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CLOCK_GATED_HAMILTONIAN_EXCHANGE_AND_QUARTIC_LOAD_BOUNDARY_v1.md` | `FFC0E39CC2C87FE73DC3C931302FE32EB5493E6AFB426CFA5BF97624DA3917D1` |
| `engine/include/ftd/eft/reversible_checkerboard_gauss_preparation.h` | `7C2AFBFD098268B02C9E58DABAC19ED38DD1FA173385424E111B0FEFAAD79420` |

## 3. Frozen normalized cell mode

For one active cell let `d_x` be its matched incidence row,
`||d_x||^2=6`, and define

\[
 y=\frac{d_xJ}{\sqrt6},\qquad
 s=\frac{q_x}{\sqrt6},\qquad
 u=y-s,\qquad
 a=\frac{e_x}{\sqrt6}.                                      \tag{G1}
\]

The FTD-0882 gate is

\[
 (u,a)\longmapsto(a,-u).                                    \tag{G2}
\]

Restore complete canonical modes

\[
 \{u,\pi_u\}=1,\qquad \{a,\pi_a\}=1,                       \tag{G3}
\]

with all other brackets zero. Set

\[
 N=\frac12(u^2+a^2+\pi_u^2+\pi_a^2),\qquad
 L=a\pi_u-u\pi_a.                                           \tag{G4}
\]

The source offset `s` is frozen during this local-layer theorem. Promoting it
to moving matter with recoil remains outside the lock.

For a common reference clock pair `(theta,I)`, `omega>0`, freeze

\[
 H=\omega I+\omega N
   +\frac{\omega}{4}(1-\cos\theta)L.                         \tag{G5}
\]

The proposed result is that one clock cycle `T=2*pi/omega` gives

\[
 (u,a,\pi_u,\pi_a)\longmapsto
 (a,-u,\pi_a,-\pi_u),                                       \tag{G6}
\]

while the carrier Hamiltonian is bounded below because `|L|<=N`. On the
zero-conjugate section, (G6) is exactly the FTD-0882 configuration gate and
preserves that section.

For a checkerboard half-layer, same-color normalized incidence rows are
orthonormal. The cell generators therefore commute and (G5) sums locally.
The integer parity schedule remains selected external control; no autonomous
parity switch is smuggled into this result.

## 4. Frozen source/inter-action ledger

On the zero-conjugate section define raw normal field-plus-port energy and the
source interaction by

\[
 E_{\rm raw}=\frac12(y^2+a^2),\qquad
 U_{\rm int}=-sy+\frac12s^2.                                 \tag{E1}
\]

Then

\[
 E_{\rm raw}+U_{\rm int}=\frac12(u^2+a^2),                   \tag{E2}
\]

and the registered work is

\[
 w=s(a-u)=\frac{q_x}{6}(e_x-r_x).                            \tag{E3}
\]

The frozen claim is

\[
 \Delta E_{\rm raw}=w,\qquad
 \Delta U_{\rm int}=-w.                                    \tag{E4}
\]

Thus a positive source-centered Hamiltonian accounts the local work without
an independent square-root battery. This does not prove that raw flux energy
alone is conserved, that source formation is free, or that the interaction
term is already present in production.

## 5. Frozen battery-phase tests

### 5.1 Linear canonical completion

For constant nonzero work `w`, one FTD-0884 sign branch has

\[
 b'=f_w(b)=\operatorname{sgn}(b)\sqrt{b^2-2w},\qquad
 f_w'(b)=\frac{b}{b'}.                                      \tag{B1}
\]

Attach a linear conjugate `p_b`. The cotangent lift that maps `p_b=0` to
`p_b'=0` is

\[
 p_b'=\frac{p_b}{f_w'(b)}=p_b\frac{b'}{b}.                   \tag{B2}
\]

It is symplectic, but for positive oscillator energy

\[
 E_{\rm osc}=\frac12(b^2+p_b^2)                              \tag{B3}
\]

the change is frozen for testing as

\[
 \Delta E_{\rm osc}
 =-w\left(1+\frac{p_b^2}{b^2}\right).                        \tag{B4}
\]

Therefore the desired `-w` ledger holds only on `p_b=0` or for `w=0`. More
generally, no continuous triangular symplectic extension with `b'=f_w(b)` can
change (B3) by exactly `-w` for every `p_b` when `w!=0`.

### 5.2 Action/phase completion

If the positive battery energy itself is the action, the constant-work map

\[
 (I_b,\phi_b)\longmapsto(I_b-w,\phi_b)                       \tag{B5}
\]

is locally symplectic under the strict reserve `I_b>w`, but on the phase
cylinder its canonical one-form changes by

\[
 T_w^*(I_b\,d\phi_b)-I_b\,d\phi_b=-w\,d\phi_b.               \tag{B6}
\]

For `w!=0`, (B6) has nonzero integral around the phase circle and is not
exact. Hence (B5) is not the time map of a globally single-valued Hamiltonian
on that cylinder.

For a system state `z`, symplectic map `F`, and nonconstant work `w(z)`, the
phase-blind triangular update

\[
 (z,I_b,\phi_b)\longmapsto
 (F(z),I_b-w(z),\phi_b)                                      \tag{B7}
\]

changes the product symplectic form by `-dw wedge dphi_b`. It is symplectic
only when `dw=0`. A canonical physical reservoir must therefore permit phase
backreaction on the system, transfer a complete canonical work mode, or export
an additional conjugate/history coordinate. It cannot merely read a computed
work scalar after a phase-blind system gate.

## 6. Frozen history completion

Every consumed port is the complete pair `(a,pi_a)`. A fresh port is
`(0,0)`. Exact repeated operation may shift outgoing pairs onto the already
selected canonical phase/history rail:

- a finite cyclic rail again supplies only its declared capacity;
- a bilateral rail is bijective and symplectic but assumes a prepared blank
  future; or
- a unilateral/open rail is completed only by retaining its incoming and
  outgoing boundary pairs.

Energy-only or coordinate-only export is not a canonical completion. The
complete pair is required. No native three-dimensional route or production
boundary is frozen here.

## 7. Frozen certificate gates

The certificate contains exactly **64** checks.

### C1--C8 — provenance and scope

1. all five frozen source hashes match;
2. this protocol hash matches its recorded pre-run value;
3. the normalized residual, port, and source offset are frozen;
4. the full canonical brackets are frozen;
5. the source interaction ledger is frozen;
6. the battery phase tests are frozen;
7. the parity schedule remains external; and
8. production, `G*`, Born, Bell, Lorentz, biology, and completeness are outside
   the result.

### C9--C28 — positive canonical Gauss layer

9. same-color normalized incidence rows are orthonormal;
10. `N` is positive definite;
11. `L` is the registered two-mode angular momentum;
12. `{N,L}=0`;
13. `|L|<=N`;
14. the clock advances uniformly;
15. the base `N` flow completes one full winding;
16. the integrated `L` pulse is `pi/2`;
17. the `L` flow sends `u` to `a`;
18. it sends `a` to `-u`;
19. it sends `pi_u` to `pi_a`;
20. it sends `pi_a` to `-pi_u`;
21. the endpoint matrix is symplectic;
22. the endpoint matrix is orthogonal;
23. the endpoint determinant is `+1`;
24. the exact inverse is the opposite quarter-turn;
25. the zero-conjugate section is invariant;
26. that section reproduces the FTD-0882 field update;
27. disjoint same-color cell generators commute; and
28. the carrier Hamiltonian is bounded below by `omega*N/2`.

### C29--C38 — clock and source-work ledger

29. `N` and `L` are conserved;
30. clock action obeys `I(theta)=I0-(1-cos(theta))*L/4`;
31. clock action returns at the cycle endpoint;
32. the maximum transient action loan is `|L|/2`;
33. the actual zero-conjugate section has `L=0`;
34. `E_raw+U_int` equals the positive centered configuration energy;
35. the raw energy change is exactly `w`;
36. the interaction-energy change is exactly `-w`;
37. raw plus interaction energy is exact; and
38. the fixed source offset is not mislabeled as a moving reservoir.

### C39--C52 — battery obstruction

39. the square-root branch derivative is `b/b'`;
40. the cotangent lift preserves the symplectic form;
41. the lift maps the zero-conjugate slice to itself;
42. the lift reproduces the FTD-0884 amplitude law on that slice;
43. its oscillator-energy change is (B4);
44. the desired `-w` change holds on the zero-conjugate slice;
45. a nonzero conjugate generically changes the booked amount;
46. coefficient comparison excludes every triangular symplectic harmonic-
    energy completion for nonzero `w`;
47. constant action translation preserves `dI_b wedge dphi_b` locally;
48. its canonical-one-form difference is `-w dphi_b`;
49. the phase-circle integral is `-2*pi*w`;
50. nonzero constant action translation is not globally Hamiltonian on the
    cylinder;
51. a state-dependent phase-blind triangular drain adds
    `-dw wedge dphi_b`; and
52. nonconstant work therefore requires phase backreaction or another
    canonical channel.

### C53--C64 — history and interpretation firewall

53. a fresh canonical port is the complete zero pair;
54. the outgoing gate retains the complete canonical pair;
55. a bilateral pair shift is symplectic and bijective;
56. a finite cyclic pair rail retains the FTD-0884 capacity boundary;
57. an open rail requires both boundary pairs in the inverse ledger;
58. scalar energy-only export is insufficient;
59. the post-hoc square-root battery is demoted to a Lagrangian-section
    reference law rather than promoted;
60. the source-centered layer adds no sixth selected v2 type;
61. an autonomous parity controller and native source formation remain open;
62. production and quartic-`G*` synchronization remain separate;
63. Born, Bell, Lorentz hiding, and completeness remain untouched; and
64. the terminal gate executes only if C1--C63 pass.

## 8. Frozen outcomes

- **Outcome A — constructive layer plus obstruction:** all `64/64` pass. Book
  the positive source-centered canonical Gauss-layer lift, exact interaction
  ledger, and the phase-complete obstruction to the standalone square-root
  battery. Retain open parity control, native source/recoil dynamics, open
  history realization, production, and `G*`.
- **Outcome B — partial:** provenance passes but one or more algebraic gates
  fail. Book only independently passing exact statements; do not promote the
  battery or canonical layer.
- **Outcome C — execution invalid:** any source/protocol hash or terminal-count
  gate fails. Book no theorem from this run.

## 9. Frozen terminal markers

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

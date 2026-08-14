# PRE-REGISTRATION — Diagnostic event energy and cubic rail gearbox v1

**Date locked:** 2026-08-10  
**Identifier:** `FTD-0854`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; CERTIFICATE INVALID 29/32]`  
**Parents:** `FTD-0402`, `FTD-0852`, `FTD-0853`

## 1. Question

Does the adopted production diagnostic already define a positive, local,
context-blind energy `B` for each manifested-to-void event? Is FTD-0853's
cubic radial coordinate exactly the normalized FTD-0852 history-rail
coordinate, so that the causal outward shift forms a fresh ready port while
retaining every earlier event?

## 2. Frozen sources

| Source | SHA256 |
|---|---|
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CUBIC_ODD_EVENT_DEPOSIT_v1.md` | `08FBF3361C453DC9E0A99184920883DBC6DE15B5043F7EFC140B0EB740A26474` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md` | `7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7` |
| `engine/include/ftd/causal_kinematics.h` | `705501451985333D64128A0896216A137A2D836673AEB02E9ACE6DE4F2E53AA2` |
| `engine/include/ftd/ontic/particle_masses.h` | `EFE9D68C9ECF6520510519B972D5CDD5925FD86026270AB0E4CAA5BFD6F1B0B1` |
| `engine/src/diagnostics_compute.cpp` | `C3703292F8474EBC119F70024B0F3E4A23921C26EA58F8F6AB5E7581FB654AA6` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/src/energy_ledger_compute.cpp` | `2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B` |

Any mismatch yields Outcome C and no theorem.

## 3. Frozen event-energy provenance

For a valid manifested production record with raw speed squared `v2<C_SPEED^2`,
the adopted causal diagnostic defines

\[
 \gamma(v)=\frac{1}{\sqrt{1-v^2/C_{\rm speed}^2}},
 \qquad
 B_{\rm diag}(v)=E_{\rm rest}+K(v)=\gamma(v)E_{\rm rest}>0. \tag{1}
\]

The evaporation assignment changes the record from nonzero to zero. It leaves
the continuous field and velocity coordinates untouched, but the diagnostic
counts matter rest/kinetic energy only when the record is nonzero. Therefore
the diagnostic matter-energy decrement across that exact assignment is (1).

This is conditional on the already adopted/imposed matter-energy role map. It
is not a P1--P5 derivation of rest energy, a new mass derivation, or proof that
the aggregate production drift ledger conserves this energy.

## 4. Frozen cubic-to-rail gearbox

At history depth `j`, let the selected pure radial cubic mode on the six axial
rays have

\[
 W_{L,j,\nu}=a_j\nu,
 \qquad W_{R,j,\nu}=-a_j\nu,
 \qquad \nu\in\{\pm e_x,\pm e_y,\pm e_z\}. \tag{2}
\]

Define

\[
 Q_j=\sum_\nu \nu\cdot(W_{L,j,\nu}-W_{R,j,\nu})=12a_j,
 \qquad D_j=\frac{Q_j}{\sqrt{12}}=\sqrt{12}\,a_j. \tag{3}
\]

With FTD-0853's selected dual kinetic energy,

\[
 K_j=\frac12\sum_\nu(|W_{L,j,\nu}|^2+|W_{R,j,\nu}|^2)
 =6a_j^2=\frac{D_j^2}{2}=\frac{Q_j^2}{24}. \tag{4}
\]

An event `(s,B_diag)` writes

\[
 a_0'=s\sqrt{B_{\rm diag}/6},
 \qquad D_0'=s\sqrt{2B_{\rm diag}},
 \qquad Q_0'=s\sqrt{24B_{\rm diag}},                 \tag{5}
\]

while the previous rail shifts one cell outward,

\[
 a_{j+1}'=a_j,
 \qquad D_{j+1}'=D_j.                                \tag{6}
\]

Equation (6) vacates the input coordinate before equation (5) occupies it in
the equivalent split-step description. In the synchronous description, every
output at depth `j+1` reads only its inward nearest neighbour and depth zero
reads only the adjacent event site. No future context or outcome target is
read.

## 5. Frozen claims and boundaries

The certificate may prove only:

1. equation (1) is the exact decrement of the currently adopted diagnostic
   matter-energy account for the frozen evaporation assignment;
2. equations (3)--(4) are exact, so FTD-0853's cubic radial coordinate is the
   normalized FTD-0852 rail amplitude;
3. equations (5)--(6) form the next ready port causally, retain prior signed
   history, and increase receiver energy by exactly `B_diag`; and
4. the construction is local, cubic, deterministic, and target blind at
   selected reference scope.

It may not claim:

- that `E_REST` or matter energy emerged from P1--P5;
- that the production aggregate energy ledger counts rest or relative energy;
- that production's bidirectional dual wave stencil implements the directed
  reserved rail;
- that arbitrary ambient relative waves are automatically separated from the
  receiver mode;
- full-state reversal of particle ID, spin, color, remainder, latency, or all
  erased labels;
- a reciprocal protected-record barrier;
- Born, Bell, `G*`, cadence, thermodynamic, or biological recovery.

## 6. Gates

The source-and-algebra certificate has exactly 32 gates:

1. seven frozen source hashes;
2. the causal source defines `E_REST` and flat kinetic energy consistently;
3. production diagnostics count rest and kinetic energy only for nonzero
   records and include them in total particle energy;
4. the evaporation assignment clears the record/labels without a compensating
   continuous-field write;
5. the aggregate drift ledger omits rest energy and separate dual squares;
6. `B_diag=gamma*E_REST` exactly and is positive/sign independent;
7. the diagnostic matter-energy decrement is exactly `B_diag`;
8. the cubic identities `Q=12a`, `D=Q/sqrt(12)`, and
   `K=D^2/2=Q^2/24` hold;
9. the FTD-0853 deposit maps exactly to `D_0'=s*sqrt(2B_diag)`;
10. the combined shift/write is one-cell local, injective on the half-line,
    recursively ready, and energy exact;
11. cubic balance and L/R conjugation covariance hold; and
12. production gaps and all interpretation firewalls remain explicit.

## 7. Outcomes

- **Outcome A:** exact diagnostic `B` plus exact cubic-rail gearbox and current
  production realization.
- **Outcome B:** exact diagnostic `B` plus exact selected cubic-rail gearbox,
  with production ledger/stencil/reserved-channel realization still open.
- **Outcome C:** source mismatch or failure of an exact identity.

Expected honest result: Outcome B. No production code may change in this run.

## 8. Recorded invalid execution

The first locked execution returned `29/32`. C13 sliced the source from the
evaporation heading through end-of-file and therefore saw an unrelated later
maintenance write. C25 and C26 represented `s` as merely nonzero instead of
the registered ternary-event domain `s in {-1,+1}`, so SymPy correctly left
`s^2=1` unproved. All seven source hashes and all other 22 algebra/production
gates passed. The certificate is invalid and books no theorem. A fresh repair
may change only the C13 source-slice boundary and the C25/C26 exact use of the
already registered identity `s^2=1`.

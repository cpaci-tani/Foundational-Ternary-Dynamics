# PRE-REGISTRATION — Native event activation and characteristic boundary v1

**Date locked:** 2026-08-10  
**Identifier:** `FTD-0857`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; EXECUTION INVALID]`  
**Parents:** `FTD-0574`, `FTD-0826`, `FTD-0852`, `FTD-0855`, `FTD-0856`

## 1. Question

Does the frozen dual-substrate production map already supply:

1. a deterministic, target-blind local activation coordinate for record/field
   exchange;
2. an exact incoming/outgoing characteristic chart on the native relative
   field; and
3. protected one-cell characteristic propagation capable of realizing the
   FTD-0855 history rail without a new type?

The run must distinguish an event **acceptance predicate** from an on-shell
reciprocal port. It must not call a source-coded hazard law a derivation from
P1--P5.

## 2. Frozen source surface

| Source | SHA-256 at lock |
|---|---|
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/include/ftd/eft/native_modal_phase_action.h` | `C1E9D5C1944E66D7601D193DC77A39980EBA24B84A41F7D752A3A363910060B6` |
| `THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md` | `7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7` |
| `THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md` | `5D13921555B2289ABC5425F4D2436545C4C3BF0638FC71C98A577E3325D215EA` |
| `THEOREM_DIAGNOSTIC_EVENT_ENERGY_CUBIC_RAIL_GEARBOX_v1.md` | `4A498C6D7C7E65FA685D9F0879157D76713F310A6D025CCAA8756C3F1E0322E6` |

Any source mismatch makes the execution invalid. The theorem, proof, and any
subsequent isolated C++ witness are outputs and are not frozen inputs.

## 3. Registered production predicates

For fixed seed, site `x`, and primitive tick `n`, let `u_g,u_e in [0,1)` be
the stateless production draws for the genesis and evaporation streams.

In the dual path define the common fields

\[
 C=J_L+J_R,\qquad V=W_L+W_R.                         \tag{1}
\]

The registered genesis acceptance bit is

\[
 a_g=
 \mathbf 1_{s=0}
 \mathbf 1_{|C|>K_G}
 \mathbf 1_{u_g<1-\exp[-(|C|-K_G)/K_M]}.             \tag{2}
\]

For the site plus its six face neighbours, let

\[
 E_7=|C_x|^2+|V_x|^2+
 \sum_{y\sim_6x}(|C_y|^2+|V_y|^2).                  \tag{3}
\]

The registered evaporation acceptance bit is

\[
 a_e=
 \mathbf 1_{s\ne0}
 \mathbf 1_{\neg\mathrm{locked}}
 \mathbf 1_{u_e<e^{-E_7/K_M^2}K_E\,d\tau}.          \tag{4}
\]

Equations (2)--(4) are `[ENGINE FACT]` descriptions of selected production
laws. They are deterministic when their complete inputs are fixed, Moore-local,
and do not read a measurement context, selected outcome target, Born weight,
or `G*`. Their thresholds, hazard form, keyed draw, and rate are not thereby
derived physical law.

Genesis is evaluated before evaporation in the same site loop. The proof must
not compress the possible ordered pair `(a_g,a_e)` to one unlabeled bit when a
same-tick genesis-then-evaporation sequence is possible.

## 4. Common/relative kernel gate

Define

\[
 D=J_L-J_R,\qquad P=W_L-W_R.                         \tag{5}
\]

For arbitrary local perturbations `delta_J,delta_W`, the transformation

\[
 (J_L,J_R)\mapsto
 (J_L+\tfrac12\delta_J,J_R-\tfrac12\delta_J),
\quad
 (W_L,W_R)\mapsto
 (W_L+\tfrac12\delta_W,W_R-\tfrac12\delta_W)        \tag{6}
\]

fixes `(C,V)` and shifts `(D,P)` by `(delta_J,delta_W)`.

The certificate must prove that equations (2)--(4) are invariant under (6),
while any relative incoming amplitude may change. Therefore production event
acceptance cannot by itself establish the reciprocal-port conditions

\[
 i=s\sqrt{2B}\quad\text{(absorption)},
 \qquad i=0\quad\text{(ready emission)}.             \tag{7}
\]

The dual-path genesis polarity may read chirality after acceptance. That does
not repair the missing relative amplitude/energy/readiness information and
must not be conflated with the acceptance bit.

## 5. Registered characteristic chart

On one outward-oriented face bond of the relative field, let `p` be the
normalized relative momentum and `g` the oriented relative strain. Define

\[
 i=\frac{p+g}{\sqrt2},\qquad
 o=\frac{p-g}{\sqrt2}.                               \tag{8}
\]

The certificate must prove the inverse, energy, and signed-current identities

\[
 p=\frac{i+o}{\sqrt2},\quad
 g=\frac{i-o}{\sqrt2},\quad
 \frac{p^2+g^2}{2}=\frac{i^2+o^2}{2},\quad
 pg=\frac{i^2-o^2}{2}.                               \tag{9}

Spatial orientation reversal sends `g -> -g` and swaps `(i,o)`. Physical time
reversal sends `p -> -p` and maps `(i,o)->(-o,-i)`. This is an exact local
coordinate chart and energy/current split. It is not yet a theorem that the
production tick propagates the two coordinates independently.

## 6. Frozen axial C18 propagation gate

On the source-free, undamped, unclocked, plane-symmetric axial subspace, the
18-point production Laplacian must reduce exactly to

\[
 (\Delta_1D D)_j=D_{j+1}-2D_j+D_{j-1}.               \tag{10}

\]

With primitive default kick--drift, `c=C_WAVE=1/sqrt(3)`, and a Fourier mode
`e^{ikj}`, define

\[
 a(k)=4c^2\sin^2(k/2),\qquad
 U(k)=\begin{pmatrix}1-a(k)&1\\-a(k)&1\end{pmatrix}. \tag{11}

\]

The certificate must prove

\[
 \det U=1,\qquad
 \operatorname{tr}U=2-a(k),\qquad
 \sin^2\!\frac{\theta(k)}2=c^2\sin^2\!\frac{k}2.    \tag{12}

\]

Two protected one-cell rails have Fourier eigenvalues `e^{+/-ik}` and trace
`2cos(k)=2-4sin^2(k/2)`. Similarity preserves trace and eigenvalues. Equality
with (12) for all `k` requires `c^2=1`, contradicting the selected production
value `c^2=1/3`. Thus the frozen C18 tick cannot be re-labelled as the exact
one-cell FTD-0855 shift. This is scoped to that registered rail and primitive
tick; it is not a no-go on dispersive packets, wider compliance windows,
nonlocal spectral projectors, or added directional storage.

## 7. Signal work and controller boundary

For the FTD-0856 controlled identity/swap `S_g`, the signal account is

\[
 W_{\rm signal}
 =H(S_g(m,i))-H(m,i)=0,
 \qquad H(m,i)=\frac{m^2+i^2}{2}.                    \tag{13}

\]

This does not price the physical controller. The certificate must preserve as
`[OPEN]` the controller state, switching work/dissipation, clock-compliance
coupling, and event-sequence bookkeeping.

## 8. Certificate gates

The exact verifier must pass all of the following:

1. seven source hashes;
2. dual common/relative storage and matched L/R operator source contracts;
3. exact source reconstruction of equations (2)--(4), including fixed
   sequential genesis-before-evaporation order;
4. deterministic/context-blind scope with the selected-law firewall;
5. invertibility of the common/relative transform and nontrivial kernel of the
   common projection;
6. trigger invariance under arbitrary relative perturbations;
7. failure of trigger data to determine equation (7);
8. characteristic-chart inverse, energy, current, orientation, and time-
   reversal identities;
9. exact plane-symmetric C18-to-1D reduction;
10. determinant, trace, and dispersion identities for (11)--(12);
11. exact one-cell-shift obstruction at `c^2=1/3`;
12. zero signal-work identity with controller cost explicitly open; and
13. firewalls against Born, Bell, `G*`, thermodynamic, biological, or
    completeness promotion.

No numerical search, fitted tolerance, empirical frequency, target period,
Born weight, or outcome-coded input is permitted.

## 9. Outcomes

- **Outcome A:** the production acceptance data determine an on-shell relative
  port and the frozen tick protects the two characteristic rails.
- **Outcome B:** target-blind acceptance and an exact local characteristic chart
  exist, but the common/relative kernel and/or C18 dispersion prevent production
  closure.
- **Outcome C:** source mismatch or failure of an exact registered identity.

Expected honest result: **Outcome B**. No production code may change in this
run. A post-certificate isolated `ftd::eft` witness is allowed only after the
locked verdict is recorded and must retain the production/open boundary.

## 10. Recorded invalid execution

**Pre-run protocol SHA-256:**
`6B354D41D8B2324A434758383D6C8B123D17CF813FAAD21CDA84C1A010DA08B1`  
**Verifier SHA-256:**
`6D7B2FC2B6BA432976D359A2C104EAB15FAB175BC5F721B7B6B84BA8D13D17A2`

All seven frozen source hashes passed. The verifier then raised `ValueError`
before C8 because its event-slice end marker, `// ---- Sequential post-pass`,
does not occur in `phase_write.cpp`; the actual function boundary is
`void phase_write_assign_pending_ids`. No algebraic or physics gate executed,
so this run books no theorem.

A read-only diagnostic execution with that marker corrected exposed three
additional verifier-only comparison defects: C32 used structural tuple equality
rather than simplified symbolic equality; C37 required one particular SymPy
factor ordering; and C40's expected wrapped prose contained a spurious literal
`+`. The four repairs are frozen prospectively in the v2 repair protocol. The
parent script and this invalid record remain preserved.

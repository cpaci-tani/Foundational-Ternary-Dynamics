# FTD-0578 — Common Moore Worldline Action and Point-Carrier Obstructions

**Status:** `[THEOREM — EXACT COATED SPACETIME CONTINUITY]` +
`[DERIVED — COMMON ENERGY-COORDINATE ACTION AND RECIPROCAL GATHER]` +
`[THEOREM — DIAGONAL ENERGY-CENTERING MISMATCH]` +
`[THEOREM — NONZERO POINT-CARRIER PEIERLS SELF-FORCE]` +
`[CLOSED NEGATIVE — UNMODIFIED COMPACT POINT ACTION AS FREE MOBILE LAW]`  
**Date:** 2026-07-26  
**Verdict:**
`COMMON_MOORE_WORLDLINE_ACTION_DERIVED_ENERGY_CENTERING_MISMATCH_PEIERLS_PINNED`

## 1. Scope

FTD-0577 derived a noncardinal 27-site coupling coat and an exact local map
from oriented face current to the native central-current complex. FTD-0578
completes that construction in time, derives its common source/probe action,
and asks whether the resulting compact dressed polarity is already a free
mobile matter law.

It is not. The action and reciprocal gather exist, but the unmodified compact
carrier has two independent defects: generic diagonal paths use a different
time-centered source than the exact FTD-0576 endpoint-energy ledger, and the
carrier interacts with its own lattice dressing through a nonzero Peierls
potential.

## 2. Exact coated spacetime current

For the FTD-0478 trilinear density and oriented-face current,

\[
 \dot\rho_{\rm CIC}+d_f k_f=0.
\]

With the FTD-0577 operators

\[
 B_i=\frac{T_i^{-1}+2+T_i}{4},\qquad
 B_M=B_xB_yB_z,\qquad A_i=\frac{1+T_i^{-1}}2,
\]

define

\[
 \rho_M=B_M\rho_{\rm CIC},\qquad
 q_i=A_i\prod_{j\ne i}B_j k_{f,i}.
\]

The Laurent identity `d_c A_i=B_i d_f` gives

\[
 \boxed{\dot\rho_M+D_cq=0.}
\]

For temporal hats `w_0=1-t`, `w_1=t`, set

\[
 T_a=\int_0^1w_a\rho_Mdt,\qquad
 Q_a=\int_0^1w_aqdt,\qquad T=T_0+T_1.
\]

Integration by parts in time yields the exact endpoint splits

\[
 \boxed{D_cQ_0=\rho_0-T,\qquad D_cQ_1=T-\rho_1.}
\]

The aggregate `Q_0+Q_1` is exactly the FTD-0577 current. The temporal
records are derived integrals of one straight segment, not new ontology.

The compiled observer verifies all 26 signed Moore directions, two
polarities, and `L=17,33`: 104 arms total. The worst split-continuity and
aggregate-continuity residual is `1.39e-17`; aggregate-current reconstruction
also closes to `1.39e-17`.

## 3. Common action and reciprocal gather

Use the unique FTD-0576 field-work coordinate `R=J-W/2` and a linear slab
history `R(t)=(1-t)R_0+tR_1`. The coated interaction is

\[
 \boxed{I_M=G_C\int_0^1
 [\langle\rho_M,DR\rangle+\langle q,CR\rangle]dt.}
\]

Equivalently,

\[
 I_M=G_C[\langle T_0,DR_0\rangle+\langle T_1,DR_1\rangle
 +\langle Q_0,CR_0\rangle+\langle Q_1,CR_1\rangle].
\]

Since `D^T=-G`, `C^T=C`, and `B_M` is self-adjoint, its field derivatives
are

\[
 S_a=-G_CGT_a+G_CCQ_a.
\]

The observer evaluates the action independently in two orders. The deposit
route first deposits `T_a,Q_a` and pairs them with `DR_a,CR_a`. The orbit
route first applies the scalar-coat and current-bridge adjoints to those
fields and then samples them along the continuous straight segment. Four
field fixtures agree within `4.32e-18`; endpoint field-adjoint residuals are
below `1.74e-18`.

Therefore source deposition and path gather are derivatives of one selected
coupling functional. Variation with respect to the path produces the Hodge
Lorentz form; its magnetic contribution is orthogonal to velocity and has
zero scalar work. This is a coupling-sidecar result. Production does not
evaluate this action.

## 4. Exact energy-centering mismatch

The FTD-0576 endpoint-energy theorem requires

\[
 \bar\rho=\frac{\rho_0+\rho_1}{2},\qquad
 S_E=-G_CG\bar\rho+G_CC(Q_0+Q_1).
\]

The time-exact action instead produces the aggregate scalar source `-G_CGT`.
They agree exactly only when `T=bar rho`.

For a complete axial CIC hop this equality holds. For complete edge and body
diagonals, exact rational convolution with `B_M` gives

\[
 \boxed{\|T-\bar\rho\|_2^2=0\quad\text{(axial)},}
\]

\[
 \boxed{\|T-\bar\rho\|_2^2=\frac1{1536}\quad\text{(edge)},\qquad
 \frac5{3072}\quad\text{(body)}.}
\]

The compiled values differ from these rationals by at most `2.82e-18`.
Thus one unmodified time-exact action cannot also be the already-derived
endpoint-energy-centered source on every Moore path. A trapezoid correction,
time-varying field treatment, or discrete-gradient transaction is an
additional selected construction.

## 5. Exact point-carrier Peierls self-force

Use the FTD-0575 static Hodge response

\[
 R(k)=\frac{3\sum_i\sin^2k_i}{M(k)},
\]

and displace the coated carrier by `r` along axis `i`:

\[
 \widehat\rho_r(k)=B_M(k)[(1-r)+re^{-ik_i}].
\]

Eliminating the static field from the same action gives

\[
 V_{\rm self}(r)=V_{\rm self}(0)+C_i r(1-r),
\]

\[
 \boxed{C_i=\frac{G_C^2}{L^3}\sum_k
 R(k)B_M(k)^2(1-\cos k_i)>0.}
\]

Hence

\[
 \boxed{\Delta V_{\rm P}=C_i/4,\qquad
 F_{\rm self}=-C_i(1-2r).}
\]

The force is polarity-even and pulls the compact carrier toward integer
sites. On the registered volumes the minimum coefficient is
`0.00026961904613504844` and the minimum half-cell barrier is
`0.00006740476153376211`. The exact quadratic law closes within `2.62e-17`
over 108 volume/axis/polarity/displacement arms. Exact total energy makes the
barrier conservative; it does not remove it.

## 6. Consequence

The noncardinal Moore coat solves the local-current obstruction and admits a
single reciprocal coupling action. It does not, by itself, produce freely
translating point matter. The unmodified compact bare-polarity route is closed
for FTD-0481 because:

1. diagonal time-exact sources do not equal the FTD-0576 energy-centered
   source; and
2. the reciprocal self-field generates a nonzero lattice pinning potential.

This result does not close integer hopping, a selected energy-centered
multistage action, or an extended native excitation whose Peierls barrier per
unit rest energy scales to zero. It licenses no production toggle, scenario,
particle, Coulomb, Lorentz, or unitarity claim.

**Successor status (FTD-0579):** every nonzero finite rigid extension retains
both the diagonal centering mismatch and a positive Peierls coefficient.
Smooth finite envelopes suppress their relative size only as
`O(R_rms^-2)`. Deforming/noncompact carriers, integer hopping, and a selected
energy-centered multistage action remain outside that no-go.

**Successor status (FTD-0580):** the positive endpoint chord plus democratic
shortest-path face routing removes the diagonal centering mismatch exactly.
The common-action Peierls barrier remains positive, so this correction does
not promote gapless mobile matter.

The run of record is `engine/results/ftd_0578/windows_msvc_cpu.json`. The
locked preregistration SHA-256 is
`DE4F20274E679F0C0E39967B985025F85D5D6F56A1D142B86CE6DE603A62019B`.

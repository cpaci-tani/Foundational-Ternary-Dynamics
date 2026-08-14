# Theorem — Zero-action canonical seed and causal clock-growth boundary v1

**Identifier:** `FTD-0993/0994`  
**Date:** 2026-08-12  
**Status:** `[THEOREM, CONDITIONAL — LOCAL CARTESIAN ZERO-ACTION SEED]` +
`[THEOREM, CONDITIONAL — TARGET-BLIND FIRST PHASE]` +
`[THEOREM — EXTENDED UNIFORM-SEED LOCALITY OBSTRUCTION]` +
`[THEOREM — CANONICAL NO-CLONING / EXACT JOIN CHART]` +
`[CLOSED NEGATIVE — UNCHANGED PRODUCTION CAUSAL CLOCK GROWTH]`  
**Parent:** `FTD-0991/0992`

## Result

The singularity of action--angle coordinates at zero action is not a theorem
that a clock cannot start. In Cartesian canonical coordinates, a positive
local membrane-work scalar and the retained time-odd orientation supply an
exact first clock stroke.

Let `x` be local source coordinates with conjugate `pi`, let `(Q,P)` be an
existing Cartesian receiver pair, and let `U(x,Q)>0` be the net local released
work after all registered loads. Define

\[
 \mathcal S_\sigma(x,Q)
 =\sigma\int_0^Q\sqrt{2U(x,\xi)}\,d\xi,
 \qquad \sigma\in\{-1,+1\}.                             \tag{1}
\]

The momentum shear

\[
 \boxed{x'=x,\quad Q'=Q,\quad
 \pi'=\pi+\partial_x\mathcal S_\sigma,
 \quad P'=P+\partial_Q\mathcal S_\sigma}               \tag{2}
\]

is symplectic and exactly inverted by `-mathcal S_sigma=mathcal S_{-sigma}`.
On the zero-action seam `Q=P=0`,

\[
 \boxed{\pi'=\pi,qquad P'=\sigma\sqrt{2U(x,0)}.}       \tag{3}
\]

For receiver frequency `Omega>0`, equation (3) gives

\[
 \boxed{H_{\rm clock}'=U,qquad
 I'={U\over\Omega},qquad
 \theta'=-\sigma{\pi\over2}\pmod{2\pi}.}              \tag{4}
\]

Thus the first action and first phase can be produced without a Born,
measurement-context, setting, outcome, or `G*` target read. The sign is not
created by the shear: it is the previously retained time-odd crossing token.

This local result does not permit an instantaneous exact uniform seed of an
arbitrarily extended body. For `N` sites,

\[
 u_N={\mathbf1\over\sqrt N},\qquad
 Q_N=u_N^Tq,qquad P_N=u_N^Tp.                           \tag{5}
\]

The projector `u_Nu_N^T` is dense. Changing exact `P_N` changes every site
momentum. A bounded event cannot perform that update throughout an arbitrarily
large body in one Moore-local tick. From a seed site `x_0`, exact causal phase
preparation requires at least

\[
 T\ge\max_{y\in S}d(x_0,y),                             \tag{6}
\]

or at least the graph radius after optimizing the seed location.

Growth cannot evade this result by freely copying the clock. The exact join
of an `N`-site uniform pair and one prospective site is

\[
 \begin{pmatrix}Q_{N+1}\\R\end{pmatrix}
 ={1\over\sqrt{N+1}}
 \begin{pmatrix}\sqrt N&1\\1&-\sqrt N\end{pmatrix}
 \begin{pmatrix}Q_N\\q\end{pmatrix},                   \tag{7}
\]

with the same orthogonal transform on momenta. The relative join pair
vanishes exactly when

\[
 \boxed{q={Q_N\over\sqrt N},qquad
 p={P_N\over\sqrt N}.}                                 \tag{8}
\]

A phase-matched new site therefore carries `1/N` of the old uniform-mode
energy before joining. That energy and phase must arrive in the local field,
be paid by formation work through a phase-complete machine, or remain as a
relative excitation. Directly leaving one arbitrary canonical pair unchanged
while assigning an independent blank pair the same values is not canonical.

## Certificate of record

- Parent protocol:
  [`PREREG_ZERO_ACTION_CANONICAL_SHEAR_SEED_AND_CAUSAL_BODY_GROWTH_BOUNDARY_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_ZERO_ACTION_CANONICAL_SHEAR_SEED_AND_CAUSAL_BODY_GROWTH_BOUNDARY_v1.md),
  SHA-256
  `9A25D55B35BC32787E8FCBC513B6225B31ADA2E84249AB8F273992F489662753`.
- Immutable parent proof:
  [`proof_zero_action_canonical_shear_seed_and_causal_body_growth_boundary.py`](../../../../../scripts/proofs/proof_zero_action_canonical_shear_seed_and_causal_body_growth_boundary.py),
  SHA-256
  `4F158B7A8847852D1DEF98E29E30999634FF769B27C56C04E1E39C2048029831`.
- First locked execution: `95/96`; every mathematical, production, and scope
  gate passed; one verifier source-hash literal was mistyped.
- Repair protocol:
  [`PREREG_ZERO_ACTION_CANONICAL_SHEAR_CERTIFICATE_REPAIR_v2.md`](../../preregistrations/native_time_carrier_programme/PREREG_ZERO_ACTION_CANONICAL_SHEAR_CERTIFICATE_REPAIR_v2.md),
  SHA-256
  `0504086A3D106D3A04B90B20467394D6E2F0F3206E126525F29A89B1345851D9`.
- Repair wrapper:
  [`proof_zero_action_canonical_shear_seed_and_causal_body_growth_boundary_v2.py`](../../../../../scripts/proofs/proof_zero_action_canonical_shear_seed_and_causal_body_growth_boundary_v2.py),
  SHA-256
  `19CB58C25C408A56F50D1BB05A99EE9825311269F4FB336C419AAA49F2147CD1`.
- Final execution: inherited `96/96` plus repair integrity `11/11`,
  **Outcome B — local Cartesian seed / extended-body causal boundary**.

## 1. Why the shear is canonical

Order the coordinates as `z=(x,Q,pi,P)`. A coordinate-gradient momentum
shear has Jacobian

\[
 D\Phi=
 \begin{pmatrix}I&0\\D^2\mathcal S&I\end{pmatrix}.      \tag{9}
\]

Since `D^2 mathcal S` is symmetric wherever equation (1) is smooth,

\[
 (D\Phi)^T\Omega(D\Phi)=\Omega,
 \qquad\det D\Phi=1.                                   \tag{10}
\]

No new canonical pair is added. The source reaction
`partial_x mathcal S` is required off the seam and is part of the same
generator. At `Q=0`, the integral in equation (1) vanishes for every `x`, so
its `x` derivative also vanishes. This proves equation (3).

Changing `sigma` to `-sigma` changes `mathcal S` to `-mathcal S`, which is
the exact inverse shear. Resetting or forgetting `sigma` would destroy that
inverse.

## 2. Energy and target-blind first phase

On `Q=P=0`, equation (3) gives

\[
 {P'^2\over2}=U.
\]

If the source/membrane sector loses the same `U`, total energy is exact. The
oscillator convention

\[
 Q=\sqrt{2I/\Omega}\cos\theta,
 \qquad
 P=-\sqrt{2\Omega I}\sin\theta
\]

then yields equation (4). The two orientations select the two opposite
momentum-axis points of the same action circle.

This replaces the overstrong statement “zero action cannot self-start” by a
sharper boundary:

> action--angle coordinates cannot represent the origin, but a Cartesian
> canonical transaction can leave the origin when it receives positive work
> and a retained orientation.

The restriction matters. Away from `P=0`, the kinetic change is

\[
 {1\over2}(P+\sigma\sqrt{2U})^2-{P^2\over2}
 =U+\sigma P\sqrt{2U}.                                  \tag{11}
\]

Hence the simple energy statement is a seam theorem, not a universal
amplitude-reset law. At `U=0`, no energy is available and the square-root
generator is generally nonsmooth. The event must fail closed or use another
regular chart.

The result is target-blind only in the registered sense: the map reads local
work `U`, the existing receiver coordinates, and `sigma`. It does not read a
desired phase, probability, context, setting, outcome, `G*`, or Born weight.
Physical derivation of `U`, the receiver identity, and the seam remains open.

## 3. Exact uniform-mode locality boundary

For any `N>1`, every entry of

\[
 u_Nu_N^T={1\over N}\mathbf1\mathbf1^T                \tag{12}
\]

is nonzero. An exact increment `Delta P_N` corresponds to the site update

\[
 \Delta p_i={\Delta P_N\over\sqrt N}
\]

at every site. Therefore the Jacobian of each distant output momentum with
respect to the local seed is nonzero.

If an event's update radius is one Moore shell per primitive tick, no site at
graph distance greater than `T` can depend on the seed after `T` ticks.
Equation (6) follows. This is the same locality principle that excludes an
instantaneous central reserve bus.

It does not exclude:

- a one-site or bounded one-shell birth;
- a causally propagating clock front;
- gradual phase locking by local interactions; or
- a body formed already carrying a phase-correlated field profile.

Those are dynamical alternatives, not consequences of the fixed-mode
projector.

## 4. Canonical no-cloning and the join chart

Suppose an attempted direct copier takes a source pair `(Q_A,P_A)` and blank
receiver `(Q_B,P_B)` to

\[
 (Q_A',P_A',Q_B',P_B')=(Q_A,P_A,Q_A,P_A).               \tag{13}
\]

Although each displayed output pair separately has bracket one, the cross
bracket is

\[
 \{Q_A',P_B'\}=1,                                      \tag{14}
\]

where independent canonical output pairs require zero. Equation (13) is not
symplectic and is not an invertible phase copier.

The join matrix in equation (7), by contrast, is orthogonal. Applying it to
coordinates and momenta is symplectic. Its relative pair is

\[
 R={Q_N-\sqrt Nq\over\sqrt{N+1}},
 \qquad
 P_R={P_N-\sqrt Np\over\sqrt{N+1}},                    \tag{15}
\]

which proves equation (8).

For equal onsite frequency `Omega`, a compatible prospective site has energy

\[
 {p^2+\Omega^2q^2\over2}
 ={1\over N}{P_N^2+\Omega^2Q_N^2\over2}.               \tag{16}
\]

After joining, the enlarged uniform-mode energy is `(N+1)/N` times the old
mode energy, precisely including the incoming site's `1/N` share. A blank
site instead yields the nonzero relative pair

\[
 (R,P_R)={1\over\sqrt{N+1}}(Q_N,P_N).                  \tag{17}
\]

Thus reversible growth has a clean physical alternative to cloning: the new
site must arrive phase matched, or the mismatch persists as a relative mode
that can subsequently propagate and possibly relax through a separately
derived conservative dynamics.

## 5. Production boundary

The frozen production engine does not implement equations (1)--(8).

- It stores the required dual Cartesian flux/wave-velocity pairs, so the
  receiver type is representable.
- Its scalar `phase` is diagnostic state advanced by the imposed de Broglie
  clock; it does not seed or react back on the dual field pair.
- Genesis remains selected through a random acceptance draw and does not use
  the FTD-0992 membrane-work ledger.
- No source reaction, orientation-controlled generator, positive admission
  gate, causal clock-front law, join transaction, or inverse is present.

No engine or production change follows.

## 6. Epistemic disposition

Established:

- **[THEOREM, CONDITIONAL]** equations (1)--(3) define an exact local
  symplectic and invertible Cartesian seed;
- **[THEOREM, CONDITIONAL]** its zero-action seam produces equation (4) from
  local positive work and retained orientation without reading a target;
- **[THEOREM]** an instantaneous exact uniform seed of an arbitrarily
  extended body violates Moore locality;
- **[THEOREM]** direct canonical phase cloning is impossible in the frozen
  two-pair map; and
- **[THEOREM]** equations (7)--(8) give the exact symplectic join and phase-
  compatibility condition.

Not established:

- a native derivation of the FTD-0990 membrane coupling or the local net work
  `U`;
- autonomous formation of `sigma`, the receiver pair, or a regular seam;
- a causal local propagation/locking law that grows a persistent body clock;
- the physical value of `Omega=omega_0` or any link to `G*`;
- a production genesis/evaporation inverse, controller reserve, moving body,
  collision/backpressure, robustness, or CPU/CUDA parity; or
- Born/Bell, polarity, mass, Lorentz hiding, Hilbert-space recovery, or
  completeness.

The next discriminator is now a genuine dynamics problem: test the minimum
nearest-neighbor conservative clock-growth law that transports the signed
Cartesian seed across a changing occupancy boundary, including mismatch
energy/current, backreaction, finite propagation speed, and exact reversal.

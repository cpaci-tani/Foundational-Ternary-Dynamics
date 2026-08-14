# FTD-0990 — Preregistration: ternary occupancy membrane and self-dual clock split v1

**Identifier:** `FTD-0990`  
**Date locked:** 2026-08-12  
**Status before execution:** `[PREREGISTERED — NOT YET EVIDENCE]`  
**Expected classifier:** **Outcome B — native static mask / selected dynamical coupling**

## 1. Question

FTD-0988/0989 derives a local boundary clutch but leaves one ternary latch per
controlled bond as a possible selected memory cost. This discriminator asks
whether the actual ternary state already contains the minimum static boundary
information and whether the dual substrate then has a unique conditional
division of labor:

- the common `L+R` sector is the body's locally isolated recursive clock/work
  sector; and
- the relative `L-R` sector remains the open interaction sector.

It also tests whether the same occupancy mask that defines the boundary
selects a unique positive uniform body mode once the existing imposed
matter-site Klein--Gordon clock is included.

No engine or production mutation is authorized.

## 2. Frozen sources

| source | SHA-256 |
|---|---|
| `THEOREM_C18_BOND_CLUTCH_CURRENT_AND_WORK_ACTION_NORMALIZATION_v1.md` | `2A93D9CFF23DFFDFEEC5E1F07CB7C023D95FBACC9B05BEA4E3F77775124D87C8` |
| `THEOREM_NATIVE_COMMON_MODE_WORK_PAIR_AND_PRODUCTION_OWNERSHIP_BOUNDARY_v1.md` | `47C859191CCC1D9E306F82A68B6FC76A128593E6BAA7CC05D871D5DEEEE7EBAC` |
| `THEOREM_KRYLOV_DEGENERACY_TERNARY_LATCH_AND_ORIENTED_C4_TRANSITION_v1.md` | `7DA2366C75D38E0EA1F8012632D71C676C4E6F8D1A7F8D1467EAF4185AE77194` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/src/transmutation_phases.cpp` | `4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043` |

Any mismatch invalidates execution. A repair must preserve this protocol and
the first certificate byte-for-byte.

## 3. Registered class

For `s_x in {-1,0,+1}`, define the actual occupancy bit

\[
 m_x=s_x^2\in\{0,1\}.                                    \tag{1}
\]

For an oriented C18 bond `b=(x,y)`, define

\[
 \eta_{xy}=m_x-m_y\in\{-1,0,+1\},
 \qquad g_{xy}=1-\eta_{xy}^2.                            \tag{2}
\]

Thus `g=1` exactly when both endpoints have the same occupancy and `g=0`
exactly on a matter--void boundary. `eta` is a spatially oriented boundary
normal sign; it is time-even and is not the clockwise/counterclockwise event
sign.

Let `B` be the exact C18 incidence factor. Define

\[
 K_m=B^TG_mB,
 \qquad G_m=\operatorname{diag}(g_b).                    \tag{3}
\]

For dual coordinates and momenta, use the canonical common/relative chart

\[
 q_\pm={q_L\pm q_R\over\sqrt2},
 \qquad p_\pm={p_L\pm p_R\over\sqrt2}.                  \tag{4}
\]

The candidate fixed-body Hamiltonian is

\[
 H=\frac12p_+^Tp_++\frac12q_+^TK_mq_+
  +\frac12p_-^Tp_-+\frac12q_-^TKq_-
  +\frac{\omega_0^2}{2}\sum_xm_x(q_{+,x}^2+q_{-,x}^2). \tag{5}
\]

Equation (5) is a reference law to be tested, not an already-derived
production Hamiltonian.

## 4. Exact gates

### O1 — source and inherited-claim lock

- all seven hashes match;
- native ternary state, complete dual canonical storage, the existing common
  pair, the C18 bond-clutch law, and the retained time-odd orientation
  distinction are source present;
- unchanged production still propagates both `L/R` sectors with the same full
  C18 stencil and has no occupancy boundary gate.

### O2 — unique static occupancy membrane

Prove:

1. `m=s^2` is the unique charge-blind function on the ternary alphabet with
   `m(0)=0` and `m(+/-1)=1`. Within even real polynomials of degree at most two
   it is uniquely `s^2`.
2. `g(m_x,m_y)` is the unique symmetric Boolean gate which transmits equal
   occupancy and cuts unequal occupancy; its polynomial is

\[
 g_{xy}=1-(m_x-m_y)^2=1-m_x-m_y+2m_xm_y.                \tag{6}
\]

3. `g` is symmetric, charge-conjugation invariant, and signed-cubic covariant;
   `eta_yx=-eta_xy`, while time reversal leaves `eta` fixed.
4. For every fixed occupancy pattern, equation (3) is positive, Moore-local,
   and splits every connected matter component from the surrounding void.

This gate uses no independent **static** bond-memory variable. It does not
settle reversible dynamic changes of `s`.

### O3 — unique common-storage / relative-interaction split

Conditional on all of the following requirements:

- common and relative coordinates remain canonical;
- the common sector uses `K_m`;
- the relative sector retains the full `K` interaction channel;
- the law is quadratic, real, and invariant under `L<->R`;

prove that the `L/R` stiffness block is uniquely

\[
 {1\over2}
 \begin{pmatrix}
 K_m+K&K_m-K\\
 K_m-K&K_m+K
 \end{pmatrix}.                                         \tag{7}
\]

It is positive because it is orthogonally equivalent to
`diag(K_m,K)`. Away from a matter--void boundary `K_m=K`, so the cross block
vanishes. The `L/R` cross coupling is supported only on the boundary and is
fixed by the desired common/relative division; it is not a fitted constant.

Prove the currents split accordingly: the common boundary current vanishes,
while the relative field retains the ordinary C18 boundary current. This is a
self-dual storage/interaction decomposition, not two disconnected universes.

### O4 — connected-body uniform clock mode

Let `Lambda` be one finite connected occupied component with `N` sites. Prove
the weighted internal Laplacian satisfies

\[
 \ker K_\Lambda=\operatorname{span}\{\mathbf1_\Lambda\}. \tag{8}
\]

For the normalized uniform common mode

\[
 u_\Lambda={\mathbf1_\Lambda\over\sqrt N},
 \qquad Q=u_\Lambda^Tq_+,quad P=u_\Lambda^Tp_+,         \tag{9}
\]

the matter-site clock term in (5) gives

\[
 (K_\Lambda+\omega_0^2I)u_\Lambda
 =\omega_0^2u_\Lambda.                                  \tag{10}
\]

For `omega_0>0`, it is the unique lowest body mode and

\[
 H_u=\frac12(P^2+\omega_0^2Q^2)=\omega_0 I_u.           \tag{11}
\]

The exact seam debit is `Delta I=(H-H')/omega_0`. For `omega_0=0`, the uniform
coordinate is a zero mode and not a regular clock action.

Source-lock that production's imposed de Broglie term already uses precisely
the predicate `state!=0` and acts symmetrically on `L/R`, so equation (1) is
the common support predicate for both the proposed membrane and the existing
onsite clock. This proves representability and a conditional gearbox; it does
not derive `omega_0`, `G*`, or the membrane coupling.

### O5 — static versus dynamic price

Prove all of the following boundaries:

- a fixed occupancy mask supplies the static gate without one stored latch per
  bond;
- changing occupancy changes `K_m` and therefore incurs the exact FTD-0989
  switching work unless every affected bond is at zero strain;
- `eta=m_x-m_y` is time-even and cannot replace the time-odd crossing sign
  `sigma=sgn(p_y-p_x)` or its receiving history record;
- the occupancy-derived gate is always cut on matter--void bonds and cannot
  actively open one boundary bond while retaining the same body. A
  phase-controlled aperture still needs controller state or another derived
  mechanism;
- production genesis is stochastic and its selected drain is explicitly not
  an exact common-action latent-heat identity; evaporation changes `state` to
  zero and the journal is observation-only. Hence production does not close a
  reversible moving membrane or exact formation-energy ledger.

### O6 — epistemic firewalls

Explicitly reject promotion to:

- a forced production coupling or complete-tick Hamiltonian;
- autonomous body, membrane, or mode formation;
- a derived `omega_0`, `G*`, Born/Bell law, mass, Hilbert space, Lorentz
  hiding, or completeness result;
- an active charging aperture without controller state; or
- a claim that the spatial occupancy normal is the temporal orientation sign.

No fit, numerical near-miss search, parameter scan, formula substitution, or
engine mutation is permitted.

## 5. Classifier

- **Outcome A — native dynamical closure:** all exact gates pass and frozen
  production already implements equation (7), reversible boundary switching,
  exact formation work, and mode preparation.
- **Outcome B — native static mask / selected dynamical coupling:** O2--O5
  pass, but equation (7), reversible formation, aperture control, or mode
  preparation is absent from production.
- **Outcome C — static boundary only:** occupancy supplies a boundary mask but
  the positive self-dual split or uniform clock theorem fails.
- **Outcome D — invalid:** a source hash or exact gate fails.

Outcome B is expected. Outcome A is forbidden unless the frozen production
sources themselves contain the required coupling and reversible ledger.


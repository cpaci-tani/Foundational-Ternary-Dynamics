# FTD Alpha Readout Program: Canonical Extraction of $W_U$

**Classification:** Readout Mechanism Specification
**Status:** `[OPEN PROGRAM — LEGACY MASTER-OPERATOR ROUTE UNDERDETERMINED; BLIND DIRECT-RESPONSE ROUTE OPEN]`
**Dependencies:** `SPEC_FTD_PHASE_LAW_V1.md`, `SPEC_ALPHA_READOUT_CONTRACT.md`
**Purpose:** To define the mathematically rigorous sequence $R_\alpha(U)$ required to extract the fine-structure constant $\alpha$ from the explicit transition operator $U$, without importing QED or circular parametrizations.

> **Routing correction (2026-08-23):** FTD-0242 establishes that no tested
> FTD-native route forces the master-quadratic trace/determinant assembly. The
> sequence \(U\to W_U\to\chi_W\) below remains the legacy operator-equality
> program, not the definition of a physical coupling. The one-action program
> now gives priority to an alpha-blind long-distance response measurement from
> [`SCOPE_PHASE_COMPLETE_BOND_TRANSACTION_ACTION_v1.md`](../10_eft_program/scopes_and_specs/SCOPE_PHASE_COMPLETE_BOND_TRANSACTION_ACTION_v1.md)
> §5. Only after that measurement may its inverse be compared with \(x_+\).

---

## 1. The Operational Target

FTD cannot assert $\alpha^{-1}_{\text{FTD}} = \lambda_+ = \frac{1}{\alpha}$ simply by equating the master quadratic's roots to coupling constants. The physics relies entirely on an operational extraction sequence that reads the engine's response mechanically:

$$ U \longrightarrow W_U \longrightarrow \chi_W(\lambda) \longrightarrow \lambda_+(W_U) $$

The central deliverable of this research program is the canonical 2-by-2 response operator $W_U$, extracted from the phase-law $U$. 

If and only if $W_U$ is uniquely forced by the lattice rules, and its characteristic polynomial exactly matches the FTD master quadratic $\lambda^2 - 16G^{*2}\lambda + 16G^{*3} = 0$, can the identification $x_+ = \alpha^{-1}$ transition from `[STRONGLY MOTIVATED CONJECTURE]` to `[THEOREM]`.

That statement concerns a proof that the **master-quadratic root equals the
physical coupling**. It is stronger than, and must not block, measuring the
action's physical coupling without assuming that equality.

### 1.1 Current direct-response target

For a candidate action that forms stable minimal charges, freeze the source,
observable, asymptotic window, and finite-volume rule before evaluation. Define

$$
V_{+-}(r)=-g_{\rm eff}^2G_{\rm vac}(r)+o(G_{\rm vac}(r)),
$$

where $G_{\rm vac}$ is measured from the same vacuum propagation operator.
Measure independently

$$
c_{\rm eff}=\lim_{k\to0}\frac{\omega(k)}{|k|},
\qquad \hbar_{\rm eff}=I_*,
$$

and define

$$
\boxed{\alpha_{\rm native}
=\frac{g_{\rm eff}^2}{4\pi\hbar_{\rm eff}c_{\rm eff}}.}
$$

No $x_+$, physical alpha, CODATA value, master-quadratic coefficient, or
target relaxation eigenvalue may enter the action, preparation, estimator, or
acceptance window. A result is a native coupling measurement whether or not it
matches $x_+$. Equality with $x_+$ remains a separate hypothesis test.

### 1.2 Current strict-discrete vector gate

The one-action branch now has an exact source precursor but also an exact
propagation obstruction.

The
[C18 actualization source vertex](../10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_C18_ACTUALIZATION_SHARED_MOMENT_SOURCE_VERTEX_v1.md)
maps one manifested C4 token to a relative-vector doublet increment with

$$
 \|\Delta R_u\|^2+\|\Delta R_v\|^2={1\over81}.
$$

Contracting this increment with the bare Gaussian vector Hessian gives the
target-free chart cost

$$
 {135\over8}\left(\|\Delta R_u\|^2+\|\Delta R_v\|^2\right)
 ={5\over24}.
$$

This number is **not** a coupling measurement. It is normalization-dependent
and contains no source-source response or propagator.

The exact
[two-record collision kernel](../10_eft_program/derivations/gravity_cosmology/THEOREM_C18_TWO_RECORD_LINEARIZED_KERNEL_AND_TENSOR_BOUNDARY_v1.md)
shows why the readout cannot yet proceed. Its phase-blind momentum vector is
protected, but all nine nontrivial C4 phase-weighted vector currents have
correction eigenvalue $-8$ and relax by

$$
 \mu_{\rm phase\ vector}=1-{8\over5^{17}}<1.
$$

Thus that selected binary collision has no collision-protected
electromagnetic vector mode in the registered product-reference
linearization.

The subsequent
[FCC Gaussian-current collision theorem](../10_eft_program/derivations/charge_gauss_native_em/THEOREM_C18_FCC_GAUSSIAN_CURRENT_COLLISION_AND_MAXWELL_MODE_PRICE_v1.md)
repairs this zero-wavevector obstruction with a different target-free local
permutation. Its complete additive invariant space is record number plus one
complex vector current

$$
 \mathcal C=\sum i^p d=U+iV,
$$

so exactly six vector components are protected. Every other FCC tangent mode
is strictly damped at the registered product reference. On the equal-length
FCC shell, the actualization vertex injects exactly one ninth of a normalized
protected-current quantum.

The readout is nevertheless still inadmissible. The theorem is a $k=0$
collision result; it does not derive the finite-$k$ propagating pole, the two
longitudinal constraint/gauge removals, a static Green kernel, conservative
reserve/source work, a stable charged recurrent source, or $c_{\rm eff}$.
Consequently $G_{\rm vac}$, $g_{\rm eff}$, and $\alpha_{\rm native}$ remain
undefined operationally. No relaxation eigenvalue in either collision may be
relabelled as alpha.

The finite-wavevector test has now been executed. The
[Gaussian-current Bloch theorem](../10_eft_program/derivations/charge_gauss_native_em/THEOREM_C18_FCC_GAUSSIAN_CURRENT_BLOCH_DIFFUSION_BOUNDARY_v1.md)
proves that ordinary phase-independent FCC streaming gives an identically
zero first-order reduced generator. The transverse protected modes obey

$$
 \mu_T(k)=1+\left(
 -{244140599\over104}\pm i{48828125\over104}
 \right)|k|^2+O(|k|^3),
$$

not a linear light cone. This streaming/collision route is therefore scoped
closed negative for the direct-response readout. An oriented Hodge/curl or
cotangent repair is required before section 1.1 becomes operational.

The
[oriented bond--plaquette Hodge target](../10_eft_program/derivations/charge_gauss_native_em/THEOREM_ORIENTED_BOND_PLAQUETTE_HODGE_MAXWELL_TARGET_AND_FINITE_LIFT_BOUNDARY_v1.md)
now supplies an exact candidate cone: its centered-incidence generator has two
divergence constraints and two $\omega=|q|$ polarization pairs. This does not
reopen the readout yet. The operator has no payload-complete finite tick,
reserve/work closure, stable minimal charge, or measured response
normalization. It is the target the action must realize, not a measured
$G_{\rm vac}$ or $c_{\rm eff}$.

### 1.3 Cotangent normalization boundary (2026-08-24)

The strict-discrete chain has now advanced beyond that target. The
[global-C3 cotangent collision](../10_eft_program/derivations/charge_gauss_native_em/THEOREM_GLOBAL_C3_COTANGENT_LAYER_COLLISION_AND_VACUUM_MAXWELL_PASS_v1.md)
supplies a finite reversible selected collision with two transverse vacuum
pairs at

$$
c_{\rm eff}=\frac16.
$$

The
[stabilizer-packet source](../10_eft_program/derivations/charge_gauss_native_em/THEOREM_COTANGENT_STABILIZER_PACKET_REVERSIBLE_GAUSS_SOURCE_v1.md)
supplies one canonically normalized electric edge quantum and exact local Gauss
incidence. These results reopen the kinematic part of the direct-response
program, but not the coupling measurement.

The exact
[cotangent action-scale obstruction](../10_eft_program/derivations/charge_gauss_native_em/THEOREM_COTANGENT_NATIVE_ALPHA_ACTION_SCALE_OBSTRUCTION_v1.md)
shows why. Multiplying the complete quadratic field action by any
$\Gamma>0$ leaves the vacuum equations, speed, packet norm, and
Gauss-constrained field unchanged while multiplying the static interaction
energy. If $I_*$ is the phase-action unit, the conditional family is

$$
\alpha_{\rm native}
=\frac{3}{2\pi}\frac{\Gamma}{I_*}.
$$

Neither packet norm $1$, token ledger $8\to8$, nor the pair tangent weight
$2^{-191}$ fixes $\Gamma/I_*$. The current collision also lacks a charged
wavevector-independent Gauss graph and a relaxed static Coulomb pole.

The corrected blind observable is therefore the canonical packet-direction
curvature

$$
\chi_{\rm EM}
=D^2\!\left(S_{\rm eff}/I_*\right)_0
[e_{\rm packet},e_{\rm packet}],
$$

cross-checked against the static source residue. Only after the common finite
transaction action derives a unique block-stable $\chi_{\rm EM}$ and a charged
massless pole may the program report

$$
\alpha_{\rm native}=\frac{\chi_{\rm EM}}{4\pi c_{\rm eff}}
=\frac{3\chi_{\rm EM}}{2\pi}.
$$

No master-quadratic coefficient may define or normalize
$\chi_{\rm EM}$.

## 2. The Five Extraction Constraints

To prevent circularity or post-hoc parameter fitting, any proposed non-circular extraction program $R_\alpha(U) \mapsto W_U$ must strictly obey the following five constraints:

| Constraint | Requirement |
| :--- | :--- |
| **Canonical preparation** | Use the minimal neutral source configuration $\Omega_{\text{min}}$ allowed by $U$, not a hand-picked geometry. |
| **Canonical sector** | Interrogate the electromagnetic/Gauss response sector exclusively, avoiding gravity, mass, or observer statistical sectors. |
| **Canonical basis** | The 2D basis must be inherently forced by $J$-quadrature / quarter-conjugacy / $\mathbb{Z}[i]$, not selected post-hoc to match 137. |
| **Canonical normalization** | Outputs must be purely dimensionless ratios; no empirical $\alpha$, $e$, $\hbar$, or $c_{\text{phys}}$ values can be inserted. |
| **Canonical limit** | Define $W_U = \lim_{L \to \infty} W_L$ or establish finite-scope convergence with known error scaling. |

## 3. The Pivot to Stochastic Transfer-Operator Readout

> [!WARNING]
> The deterministic tangent map formulation (Floquet readout) is officially **[CLOSED NEGATIVE]**. A rigorous topological obstruction proves that under fixed-itinerary boundary conditions, the continuous Floquet multiplier decays identically to the open-lattice vacuum matrix $A^m$ (see `DERIV_ALPHA_READOUT_CLOSED_NEGATIVE.md`).

The FTD alpha readout must instead be formulated over the **Langevin-stabilized cloud**. Because the stabilization relies on stochastic noise, the $A=14$ soliton is a noise-sustained statistical attractor, not a pristine limit cycle.

Therefore, $W_L$ is no longer defined by the deterministic continuous tangent map $D U$. It must be defined as a **stochastic transfer-operator readout** over the invariant measure of the Langevin bath.

Explicitly:
1. Initialize the minimal neutral charge structure $\Omega_{\text{min}}$ under Langevin stabilization.
2. Construct the stochastic transfer operator $T$ governing the invariant measure of the cloud.
3. The response operator $W_U$ is defined as the resolvent susceptibility of the stationary stochastic cloud ensemble:
   $$ W_U = \Pi_{\mathbb{Z}[i]} (I - T_{\text{red}})^{-1} \Pi_{\mathbb{Z}[i]} $$
   where $T_{\text{red}}$ is the reduced transfer operator restricted to the canonical basis.
4. The coupling $\alpha^{-1}$ emerges from the dominant relaxation eigenvalue of $T_{\text{red}}$ evaluated over the statistical attractor:
   $$ T_{\text{red}} f_+ = \mu_+ f_+ $$
   $$ \alpha_{\text{FTD}} = |1 - \mu_+| $$

The particle is not a perfectly repeating deterministic breather; it is a statistically stabilized oscillatory body whose invariant distribution has a dominant slow relaxation mode. The fine-structure constant $\alpha$ is the small spectral gap of that public cloud response.

### 3.1 Scale-Context Admissibility Gate ($\mathcal{C}_{\rm scale}$)

Before $T_{\rm red}$ is estimated, the trajectory MUST pass the scale-context
readout admissibility gate $\mathcal{C}_{\rm scale}$
(`SPEC_SCALE_CONTEXT_READOUT.md`, implemented **read-only and $\alpha$-blind** in
`engine/src/scale_context.cpp`). A trajectory whose cloud is not scale-separated
($1 \ll R_{\rm eff}/a \ll L$), self-confined, or stationary is **not eligible for
public readout**, regardless of how cleanly the estimator returns a number.

> [!WARNING]
> **Measurement of record + box scan:** the canonical $A=14$ Langevin
> attractor is **`REJECTED_SCALE_CONTEXT`** at every box size tested. It reaches a
> *stationary* steady state at $R_{\rm eff}\approx L/2$ with **$\zeta\approx0.50$
> $L$-invariant** — $(L,R_{\rm eff})=(32,15.9),(64,31.8)$, i.e. $R_{\rm eff}\propto L$.
> The cloud has **no intrinsic confined size**: it is a box-filling / percolated
> state at all $L$, precisely the saturated $R_{\rm cloud}\sim L$ regime declared
> invalid in `SPEC_SCALE_CONTEXT_READOUT §4`. **A larger box does NOT help**
> (contrary to a naive finite-volume expectation). The current $A=14$ Koopman
> trajectory is therefore **inadmissible for $\alpha$ readout at any $L$**; an
> admissible measurement requires a **confining mechanism in the physics** (a
> bath/source/amplitude that produces a self-confined object), not a bigger
> lattice. The gate thresholds were NOT loosened to admit it; no tag was moved
> (MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`). See `SPEC_SCALE_CONTEXT_READOUT §5.4`.

## 4. Stochastic Admissibility Conditions

To ensure the readout is not empirical fitting, the Langevin bath must be canonical. If friction, temperature, or noise amplitude are chosen because they move the spectrum toward $137$, the readout fails.

Any stochastic stabilization must obey the following admissibility conditions:

| Condition                | Requirement                                                      |
| ------------------------ | ---------------------------------------------------------------- |
| **Canonical bath**       | Noise/friction/temperature must be fixed independently of $\alpha$ (e.g., from fluctuation-dissipation balance or fundamental engine rules). |
| **Unique invariant measure** | The stabilized cloud has a reproducible stationary distribution $\pi(\Omega)$. |
| **Projection rule**      | $\Pi_{\mathbb{Z}[i]}$ is fixed before spectral measurement.       |
| **Spectral gap stability**| $\mu_+$ must be stable under lattice size, seed, and sampling length. |
| **No target fitting**    | No parameter sweep over bath variables is allowed to hit $137$.  |

## 5. The Theorem Targets

For the FTD master quadratic to emerge organically from $W_U$, the sequence must prove that:

$$ \lim_{L \to \infty} \operatorname{Tr} W_L = 16G^{*2} $$
$$ \lim_{L \to \infty} \det W_L = 16G^{*3} $$

If both targets are achieved organically via the phase-law's geometry, the characteristic polynomial of the response operator is uniquely constrained:

$$ \chi_W(\lambda) = \lambda^2 - \operatorname{Tr}(W_L)\lambda + \det(W_L) = \lambda^2 - 16G^{*2}\lambda + 16G^{*3} $$

yielding the precise structural coupling value:

$$ \alpha^{-1}_{\text{FTD}} = \lambda_+(W) = 8G^{*2} + 4G^*\sqrt{4G^{*2} - G^*} $$

## 6. The Central Open Subproblem: The Odd $G^*$ Determinant

The trace ($\operatorname{Tr} W_L$) and determinant ($\det W_L$) are algebraically independent quantities. The canon explicitly recognizes that nothing in the raw topology yet forces the determinant slot to carry exactly one odd $G^*$.

**The immediate obstruction is defined as:**
> [!IMPORTANT]
> *Why does $\det W_U$ contain exactly one odd $G^*$ factor?* (Not zero. Not two. Not three. Exactly one.)

**Intuitive Pathway to Proof:**
*   The **trace** measures purely symmetric, self-energy reflection (Watson-like integrals), naturally producing the even quantity $16G^{*2}$.
*   The **determinant** measures the oriented two-channel coupling area.
*   Oriented coupling requires the product of one even channel and one odd quarter-conjugacy bridge.
*   Therefore, the determinant mathematically reduces to an even self-energy times one odd quarter-conjugate bridge:
    $$ \det W = (16G^{*2}) \cdot (G^*) = 16G^{*3} $$

This intuition outlines the shape of the derivation, but it becomes a `[THEOREM]` only if the explicit phase-law operator $U$ definitively forces the choice of response basis and the coupling orientation. The deliverable is not $\alpha$; the deliverable is proving the exact construction of $W_U$.

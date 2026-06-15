# SPEC_SCALE_CONTEXT_READOUT.md

**Title:** Scale Context / Finite-Size Renormalization Layer for FTD
**Status:** `[AXIOM]` / `[ACTIVE SPEC]`
**Depends on:** `SPEC_FTD_PHASE_LAW_V1.md`, `SPEC_ALPHA_READOUT_PROGRAM.md`

---

## 0. Executive Statement

No public physical readout (such as $\alpha$ or $G_N$) is admissible from the FTD engine unless the object being measured is mathematically separated from both the discrete lattice grid and the finite simulation box.

FTD objects operate at a specific operational **phenomenon scale**. Extracting the Koopman operator indiscriminately from a phase-state that has saturated the simulation box constitutes a **scale-context failure**. The phase-law $\mathcal{B}_\Omega$ does not map directly to a reduced physical observable $K_{\rm red}$; it must pass through the **Scale Context Layer** ($\mathcal{C}_{\rm scale}$) which validates self-confinement.

The new canonical readout chain is:
$$ U \rightarrow \mathcal{B}_\Omega \rightarrow \mathcal{C}_{\rm scale} \rightarrow K_{\rm red} \rightarrow W_U \rightarrow \alpha $$

---

## 1. The Scale Context Layer ($\mathcal{C}_{\rm scale}$)

The scale context isolates the physical object from lattice artifacts and finite-volume boundary effects. It is parameterized by six fundamental dimensioned scales:

| Symbol | Meaning |
| :--- | :--- |
| $a$ | Lattice voxel scale (UV cutoff limit) |
| $R_{\rm cloud}$ | Effective cloud radius (phenomenon scale) |
| $\delta_{\rm shell}$ | Active boundary / shell thickness |
| $L$ | Simulation / finite-block size (IR cutoff limit) |
| $\xi$ | Correlation or coherence length |
| $\tau_{\rm cloud}$ | Cloud relaxation / oscillation timescale |

### 1.1 Dimensionless Ratios

From these dimensioned scales, we define the critical dimensionless numbers for admissibility:

- $\kappa = \frac{R_{\rm cloud}}{a}$ (Lattice decoupling)
- $\zeta = \frac{R_{\rm cloud}}{L}$ (Finite-volume decoupling)
- $\beta = \frac{\delta_{\rm shell}}{R_{\rm cloud}}$ (Surface-to-volume ratio)
- $\chi = \frac{\xi}{R_{\rm cloud}}$ (Internal coherence)
- $\Theta = \frac{\tau_{\rm cloud}}{\tau_{\rm bath}}$ (Timescale separation)

### 1.2 Atomic closure-context instance

Atoms are the canonical Scale-2 example of a finite self-confined cloud whose
scale is not a raw function of $Z$ alone. For atoms:

$$
Z \rightarrow \text{source loading},\qquad
n_{\rm shell} \rightarrow \text{scale tier},\qquad
Z_{\rm eff} \rightarrow \text{screened return force}.
$$

The implemented atomic diagnostic vector is:

$$
\mathcal{C}_{\rm atom} =
\left(
Z,\,
n_{\rm shell},\,
Z_{\rm eff},\,
R_{\rm cloud},\,
\delta_{\rm valence},\,
\xi_{\rm orbital},\,
\tau_{\rm electronic}
\right),
$$

with:

$$
R_{\rm cloud} = R_{\rm BOHR}\frac{n_{\rm shell}^2}{Z_{\rm eff}}.
$$

This captures the periodic-table rule: within a shell, increasing screened
nuclear return force generally contracts the cloud; opening a new shell resets
the cloud scale outward. In code this lives in `AtomicClosureContext`
(`engine/include/ftd/atomic_closure_context.h`) and its JS mirror
`computeAtomicClosureContext(...)`. It is diagnostic/readout-only:
`Atom.radius` remains the simulation/LJ interaction radius unless a separate
MD retuning is performed.

---

## 2. Admissibility Rules

### 2.1 The Golden Scale Window
The cloud is physically admissible **if and only if** it occupies the golden scale window:
$$ 1 \ll \frac{R_{\rm cloud}}{a} \ll L $$
In plain English: **The cloud must be much larger than a single voxel, but much smaller than the box.** A cloud that reaches $R_{\rm cloud} \sim L$ is percolating into a phase transition and no longer represents a bounded physical particle.

### 2.2 Empirical Diagnostics
Before any Koopman alpha extraction is run, the cloud must pass the following stability tests:

1. **Volume Fraction:** $0 < f_{\rm active} \ll 1$ (where $f_{\rm active} = \frac{\rho}{L^3}$)
2. **Effective Radius:** $\frac{d}{dt} \langle R_{\rm eff} \rangle \approx 0$ (where $R_{\rm eff}^2 = \frac{\sum_v \rho(v)|v-v_c|^2}{\sum_v\rho(v)}$)
3. **Current Stability:** $\frac{d}{dt} \langle J^2 \rangle \approx 0$
4. **Boundary Susceptibility:** $\langle B(t) \rangle \approx 0$ (where $B(t) = \text{genesis flips} - \text{evaporation flips}$)

---

## 3. The Self-Confinement Condition

The scale must not be an arbitrary tuning knob used to "hit" $\alpha^{-1} = 137.036$. Scale selection must be dynamically self-consistent.

The cloud must satisfy a dynamic scale fixed point driven by flux balance:
$$ \Phi_{\rm outward}(R_*) = \Phi_{\rm return}(R_*) $$
with strict stability against perturbations:
$$ \frac{d}{dR} \left( \Phi_{\rm outward}(R) - \Phi_{\rm return}(R) \right)\Big|_{R=R_*} < 0 $$

If no such stable root $R_*$ exists, the object either evaporates to the vacuum ($R \rightarrow 0$) or undergoes a runaway genesis cascade ($R \rightarrow L$).

### 3.1 Relative Amplitude
Langevin amplitude alone is insufficient. The true forcing object is amplitude relative to the active capacity:
$$ \mathcal{A}_{\rm eff} = \frac{\text{cloud energy}}{\text{active boundary capacity}} \quad \text{or} \quad \mathcal{A}_R = \frac{E_J(R)}{K_B \cdot N_{\rm shell}(R)} $$
This resolves why identical amplitudes behave radically differently depending on lattice size and boundary geometry.

---

## 4. Central Rule

> [!CAUTION]
> **No public physical readout is admissible unless the phenomenon scale is mathematically separated from both the discrete lattice scale and the simulation box scale. Koopman extraction over saturated ($R_{\rm cloud} \sim L$) states is explicitly invalid.**

> **Doctrine:** Persistence is necessary but not sufficient. Public readout requires persistence, **self-confinement**, and **scale separation**.

---

## 5. Operational Definitions (code-level)

> **Status of §5:** `[IMPOSED engineering defaults]` — this section pins the
> *implementation* of §1–§3 in the read-only C++ gate
> `engine/include/ftd/scale_context.{h}` + `engine/src/scale_context.cpp`
> (POD results in `engine/include/ftd/render_bridge_diagnostics.h`). The gate is
> **read-only over `const RenderBridge&`**, never called from `tick()` (golden
> hash `0x56fa28acb5b9fe88` @ L=17 preserved by construction), and **blind to
> $\alpha$ by contract** — it reads only lattice geometry, the flux field
> $|J|^2$, and the observation-only genesis/evaporation counters; it never
> references `ALPHA`, the Koopman eigenvalue, or `137.036`. The thresholds below
> are **engineering defaults, not theorem values**; they are mutually
> self-consistent and the synthetic confining-cloud unit fixture
> (`engine/tests/test_scale_context.cpp`, fixture 7) lands `BoundedAdmissible`
> under them.

### 5.1 Quantities (as implemented)

Let support $= \{v : |J(v)|^2 \ge \texttt{energy\_threshold}\}$ (flux-energy
primary, optionally $\cup\,\{v: s(v)\neq 0\}$). Weight $w_v = |J(v)|^2$.

| Symbol | Code field | Definition |
| :--- | :--- | :--- |
| support / $\rho$ | `support_count` | # voxels in support (flux-energy; catches box-filling fields with sparse manifested state) |
| $f_{\rm active}$ | `active_fraction` | `support_count` $/ L^3$ |
| cloud energy | `cloud_energy` | $\tfrac12\sum_{\rm support}|J|^2$ |
| $v_c$ (center) | `center_{x,y,z}` | PBC **circular mean**: per axis $u$, $C=\sum w\cos(2\pi u/L)$, $S=\sum w\sin(2\pi u/L)$, $v_{c,u}=\operatorname{atan2}(S,C)\,L/2\pi$ |
| center concentration | `center_concentration` | $\min_u \sqrt{C^2+S^2}/\sum w \in[0,1]$; `center_well_defined` $\iff > 0.2$ (else delocalized / box-filling) |
| $R_{\rm cloud}$ | `R_eff` | $\sqrt{\sum w\,r_v^2/\sum w}$ with $r_v=\lVert\,\text{min-image}(v-v_c)\rVert$ (PBC min-image; `min_image_disp`) |
| $\kappa,\ \zeta$ | `kappa`, `zeta` | $R_{\rm eff}/a$, $R_{\rm eff}/L$ ($a\equiv 1$) |
| $\delta_{\rm shell}$ | `delta_shell` | $r_{90}-r_{50}$ from the **radial energy CDF** (own histogram about $v_c$, not `aggregate_profile`'s fixed-center 20-bin one) |
| $\beta$ | `beta` | $\delta_{\rm shell}/R_{\rm eff}$ |
| $\Phi_{\rm out},\ \Phi_{\rm ret}$ | `phi_outward`, `phi_return` | shell sums of $\max(0,\pm\,J\!\cdot\!\hat r)$ at the shell containing $R_{\rm eff}$ |
| $\tfrac{d}{dR}(\Phi_{\rm out}-\Phi_{\rm ret})$ | `dPhi_dR` | central difference of $(\Phi_{\rm out}-\Phi_{\rm ret})$ across adjacent shells |
| $B(t)$ | `B_t` | `genesis_events_this_tick` − `evaporation_events_this_tick` |
| $\dot R,\ \dot{\langle J^2\rangle}$ | `dR_dt`, `dJ2_dt` | least-squares slopes over a rolling window (`ScaleContextTracker`; x-axis = ingest index) |
| $\tau_{\rm cloud},\ \Theta$ | `tau_cloud`, `Theta` | lag-1 autocorrelation relaxation time (advisory, **non-gating**); $\tau_{\rm cloud}/\tau_{\rm bath}$ |

### 5.2 Threshold defaults `[IMPOSED engineering defaults]`

| Config field | Default | Gate |
| :--- | :--- | :--- |
| `energy_threshold` | `1e-4` ($|J|^2$) | support floor |
| `kappa_min` | `3.0` | UV decoupling ($\kappa \ge$) |
| `zeta_max` | `0.25` | IR decoupling ($\zeta \le$) |
| `f_active_max` | `0.10` | volume-fraction ceiling |
| `f_active_evap_min` | `1e-5` | below ⇒ Evaporating |
| `beta_max` | `0.60` | shell-dominance ceiling |
| `phi_balance_tol` | `0.15` | $|\Phi_{\rm out}-\Phi_{\rm ret}|/(\Phi_{\rm out}+\Phi_{\rm ret})$ |
| `dPhi_dR_max` | `0.0` | slope must be $<0$ |
| `dR_dt_tol`, `dJ2_dt_tol` | `0.02`, `0.02` | stationarity |
| `B_t_tol` | `1.0` | windowed $|\langle B\rangle|$ |
| `window`, `tau_bath` | `64`, `50.0` | rolling estimation |
| `n_shells`, `shell_width` | `64`, `1.0` | radial partition |
| `gate_active` | `false` | observe-only ⇒ status `DiagnosticOnly` |

The literal `\kappa_{\min}=6\text{–}10` floated in early planning is **rejected**:
with `zeta_max=0.20` at $L=32$ it makes the golden window $6\le R_{\rm eff}\le6.4$
(or empty), which is incoherent. `kappa_min=3`, `zeta_max=0.25` is the
self-consistent replacement.

### 5.3 Injection convention `[IMPOSED]`

Both `engine/tests/dump_koopman_trajectory.cpp` and
`engine/tests/campaign_alpha_readout_scattering.cpp` inject the cloud as
$|J| = A\cdot K_{\rm GENESIS}$ (genesis-threshold units; $K_{\rm GENESIS}=1.533$).
This unified the two tests (the dumper previously injected raw $A$) and **re-pins
the canonical "A=14" cloud** ($|J|=14\cdot1.533\approx21.46$).

### 5.4 First calibration measurement + box scan (run of record)

Running the dumper (T=0.005, γ=0.02, 2000-tick thermalization + recording) on the
re-pinned A=14 cloud, scanning the box size $L$:

| $L$ | $R_{\rm eff}$ | $\kappa$ | $\zeta=R_{\rm eff}/L$ | $\beta$ | regime |
| :-- | :-- | :-- | :-- | :-- | :-- |
| 32 | 15.89 | 15.89 | 0.4966 | 0.326 | Percolating |
| 64 | 31.82 | 31.82 | 0.4972 | 0.323 | Percolating |

Both states are **stationary** (`dR_dt`$\approx$0), so these are steady states,
not transients.

**Finding — the inadmissibility is intrinsic, not a finite-volume artifact.**
$R_{\rm eff}$ scales **directly with $L$** ($15.89\!\to\!31.82$ when $32\!\to\!64$,
ratio $2.003$) and $\zeta\approx0.50$ is **$L$-invariant** (0.4966 vs 0.4972).
The A=14 Langevin-stabilized cloud therefore fills $\sim$half the box at *every*
$L$: it has **no intrinsic confined size** — it is a box-filling / delocalized
percolated state, exactly the saturated $R_{\rm cloud}\sim L$ regime §4 declares
invalid. (A larger box is predicted to give $\zeta\approx0.5$ again; $L=128$ would
yield $R_{\rm eff}\approx64$, still Percolating.)

**The thresholds were NOT loosened to admit it** — doing so would contradict the
central rule and defeat the gate.

**Consequences:**
1. The current canonical A=14 Koopman trajectory is **not readout-admissible at
   any $L$**; increasing the box does not help (contrary to a naive
   finite-volume expectation).
2. An admissible (bounded, scale-separated) cloud requires a **confining
   mechanism in the physics** — a different bath/source/amplitude that produces a
   self-confined object with $R_{\rm eff}$ set by dynamics rather than the box —
   not merely a bigger lattice.
3. This is consistent with, and sharpens, the standing $\alpha$-readout
   obstruction (MC-T4.3, `[FOUNDATIONAL OBSTRUCTION]`): the gate makes the
   percolation explicit and pre-registers it as a boundary, with no tag moved.

This is precisely the kind of inadmissibility the layer exists to surface before
any $\alpha$ readout is reported.

### 5.5 Confinement scan — can a cloud be made admissible? (run of record)

§5.4 showed the canonical Langevin cloud percolates at every $L$. The natural
follow-up: does *any* variant produce a dynamics-set (bounded, self-confined)
cloud the armed gate accepts? Scan tool:
`engine/tests/campaign_scale_context_confine.cpp` (read-only gate, $L=48$,
1200-tick thermalization + 120-tick recording). Two lever families were swept:
(i) **Langevin** variants (global damping, low $T$, de Broglie mass term); and
(ii) **deterministic** damped clusters (no Langevin) over an amplitude sweep.

**Finding 1 — the Langevin thermal-floor obstruction.** Any Langevin bath with
$T\gtrsim10^{-4}$ drives $f_{\rm active}\to1$: the thermal floor $|J|^2$ exceeds
`energy_threshold` ($10^{-4}$) at *every* voxel, so the box fills and the cloud
is **Percolating** — independent of global damping (which leaves $f_{\rm active}$
unchanged at $0.999$) and of the de Broglie mass term (which masses only
manifested voxels, not the free field). **The stochastic stabilization the
readout program requires is exactly what floods the box.**

**Finding 2 — a deterministic bounded cloud exists, in a narrow window.**
Removing the bath (no Langevin, global damping, $g_c$ coupling on) localizes the
coherent cloud; its size grows with injection amplitude $A$ (in genesis units):

| $A$ | $R_{\rm eff}$ | $\zeta$ | $f_{\rm active}$ | self-confined? | regime / status |
| :-- | :-- | :-- | :-- | :-- | :-- |
| 14 | 1.69 | 0.035 | 0.000 | no | UVLocked / `REJ_SCALE` |
| 26 | 5.31 | 0.111 | 0.021 | **no** | **BoundedAdmissible** / `REJ_CONFINE` |
| 30 | 7.65 | 0.159 | 0.079 | **no** | **BoundedAdmissible** / `REJ_CONFINE` |
| 34 | 22.8 | 0.475 | 0.990 | (n/a) | Percolating / `REJ_SCALE` |
| ≥38 | ~20 | ~0.4 | ~0.95 | no | Percolating / `REJ_SCALE` |

There is a **sharp genesis cascade between $A=30$ and $A=34$** (the cluster
detonates and fills the box). Inside the bounded window ($A\approx26$–$30$) the
gate returns **`BoundedAdmissible`** — a bounded, scale-separated cloud, with
$\zeta\approx0.1$–$0.16$ and $f_{\rm active}\approx0.02$–0.08. This **proves the
gate admits a properly-sized cloud and is not over-rejecting.**

**Finding 3 — self-confinement is the binding barrier; nothing is fully
Admissible.** Across the *entire* bounded window, the self-confinement
flux-balance fixed point ($\Phi_{\rm out}=\Phi_{\rm ret}$ with $d\Phi/dR<0$)
**never** holds: a damped genesis cluster is a **leaky source** (net-outward
flux, no stable return shell), so it lands `RejectedSelfConfinement`, not
`Admissible`. (The $A=34$ `conf=yes` is spurious — a uniform box-filling field
trivially balances and is rejected on scale anyway.) **No config tested —
stochastic or deterministic — reaches full `Admissible`.**

**Conclusion (boundary, no tag moved).** The gate's three layers each bite in
turn: scale-separation (UV/percolation) and then self-confinement. A bounded,
scale-separated cloud is achievable *deterministically* (window $A\approx26$–30);
the missing ingredient for public readout is a **self-confined flux object** — a
genuine field soliton with a stable $\Phi$-balance shell — which neither the
Langevin bath nor genesis+damping produces. This is the same gap the readout
program itself hit (the deterministic breather route is `[CLOSED NEGATIVE]`,
`SPEC_ALPHA_READOUT_PROGRAM §3`), now localized to a concrete mechanical face:
**there is no self-confined cloud to read.** The α readout therefore remains
blocked, consistent with and sharpening **MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`**.
Unlocking it requires new confining physics, not a parameter tweak; thresholds
were **not** loosened, and **no epistemic tag moved**.

> **Flagged limitation (future refinement, not done here):** the
> self-confinement check evaluates $\Phi_{\rm out}/\Phi_{\rm ret}$ **per tick**.
> A *breathing* (oscillating) soliton would be balanced only in the
> window-average, so the per-tick test could under-credit such a cloud. A
> window-averaged $\Phi$-balance is a candidate refinement — but changing it now
> would risk tuning-to-pass, so it is recorded as an open option, not applied.
